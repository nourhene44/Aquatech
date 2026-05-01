#include "Qarduino.h"

#include <QFileInfo>
#include <QDebug>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>

#include <algorithm>
#include <utility>

#ifdef Q_OS_WIN
#include <regstr.h>
#include <setupapi.h>
#endif

namespace {

const QList<int> kSupportedBaudRates = {9600, 115200, 57600, 38400};

#ifdef Q_OS_WIN
struct SerialPortCandidate
{
    QString portName;
    QString friendlyName;
    QString description;
    QString hardwareId;
    QString manufacturer;
    int score = 0;
};

int serialPortNumber(const QString& portName)
{
    static const QRegularExpression rx(QStringLiteral("^COM(\\d+)$"),
                                       QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = rx.match(portName.trimmed());
    return match.hasMatch() ? match.captured(1).toInt() : -1;
}

QString extractComPortName(const QString& text)
{
    static const QRegularExpression rx(QStringLiteral("\\b(COM\\d+)\\b"),
                                       QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = rx.match(text);
    return match.hasMatch() ? match.captured(1).toUpper() : QString();
}

QString readDeviceProperty(HDEVINFO deviceInfoSet, SP_DEVINFO_DATA& deviceInfo, DWORD property)
{
    DWORD dataType = 0;
    DWORD requiredSize = 0;
    SetupDiGetDeviceRegistryPropertyW(deviceInfoSet,
                                      &deviceInfo,
                                      property,
                                      &dataType,
                                      nullptr,
                                      0,
                                      &requiredSize);

    if (requiredSize == 0) {
        return {};
    }

    QByteArray rawBuffer(static_cast<int>(requiredSize) + static_cast<int>(sizeof(wchar_t)), '\0');
    if (!SetupDiGetDeviceRegistryPropertyW(deviceInfoSet,
                                           &deviceInfo,
                                           property,
                                           &dataType,
                                           reinterpret_cast<PBYTE>(rawBuffer.data()),
                                           static_cast<DWORD>(rawBuffer.size()),
                                           nullptr)) {
        return {};
    }

    return QString::fromWCharArray(reinterpret_cast<const wchar_t*>(rawBuffer.constData())).trimmed();
}

QString readPortNameFromRegistry(HDEVINFO deviceInfoSet, SP_DEVINFO_DATA& deviceInfo)
{
    HKEY key = SetupDiOpenDevRegKey(deviceInfoSet, &deviceInfo, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
    if (key == INVALID_HANDLE_VALUE) {
        return {};
    }

    wchar_t value[256] = {};
    DWORD dataType = 0;
    DWORD valueSize = sizeof(value);
    const LONG status = RegQueryValueExW(key,
                                         L"PortName",
                                         nullptr,
                                         &dataType,
                                         reinterpret_cast<LPBYTE>(value),
                                         &valueSize);
    RegCloseKey(key);

    if (status != ERROR_SUCCESS || dataType != REG_SZ) {
        return {};
    }

    return QString::fromWCharArray(value).trimmed().toUpper();
}

int scoreSerialCandidate(const SerialPortCandidate& candidate)
{
    const QString haystack = QStringLiteral("%1 %2 %3 %4")
                                 .arg(candidate.friendlyName,
                                      candidate.description,
                                      candidate.hardwareId,
                                      candidate.manufacturer)
                                 .toLower();

    int score = 0;

    if (haystack.contains(QStringLiteral("arduino"))) score += 120;
    if (haystack.contains(QStringLiteral("uno"))) score += 40;
    if (haystack.contains(QStringLiteral("nano"))) score += 40;
    if (haystack.contains(QStringLiteral("mega"))) score += 40;
    if (haystack.contains(QStringLiteral("atmega"))) score += 40;
    if (haystack.contains(QStringLiteral("ch340")) || haystack.contains(QStringLiteral("wch"))) score += 35;
    if (haystack.contains(QStringLiteral("cp210"))) score += 35;
    if (haystack.contains(QStringLiteral("ftdi"))) score += 35;
    if (haystack.contains(QStringLiteral("usb serial"))) score += 25;
    if (haystack.contains(QStringLiteral("serial device"))) score += 10;

    if (haystack.contains(QStringLiteral("bluetooth"))) score -= 120;
    if (haystack.contains(QStringLiteral("modem"))) score -= 120;
    if (haystack.contains(QStringLiteral("printer"))) score -= 120;
    if (haystack.contains(QStringLiteral("debug"))) score -= 40;

    const int portNumber = serialPortNumber(candidate.portName);
    if (portNumber > 0) {
        score += std::max(0, 20 - std::min(portNumber, 20));
    }

    return score;
}

QList<SerialPortCandidate> enumerateSerialCandidates()
{
    QList<SerialPortCandidate> candidates;
    QSet<QString> seenPorts;

    GUID portGuids[8];
    DWORD guidCount = 0;
    if (!SetupDiClassGuidsFromNameW(L"Ports", portGuids, static_cast<DWORD>(std::size(portGuids)), &guidCount) ||
        guidCount == 0) {
        return candidates;
    }

    for (DWORD guidIndex = 0; guidIndex < guidCount; ++guidIndex) {
        HDEVINFO deviceInfoSet = SetupDiGetClassDevsW(&portGuids[guidIndex], nullptr, nullptr, DIGCF_PRESENT);
        if (deviceInfoSet == INVALID_HANDLE_VALUE) {
            continue;
        }

        for (DWORD index = 0;; ++index) {
            SP_DEVINFO_DATA deviceInfo;
            ZeroMemory(&deviceInfo, sizeof(deviceInfo));
            deviceInfo.cbSize = sizeof(deviceInfo);

            if (!SetupDiEnumDeviceInfo(deviceInfoSet, index, &deviceInfo)) {
                break;
            }

            SerialPortCandidate candidate;
            candidate.friendlyName = readDeviceProperty(deviceInfoSet, deviceInfo, SPDRP_FRIENDLYNAME);
            candidate.description = readDeviceProperty(deviceInfoSet, deviceInfo, SPDRP_DEVICEDESC);
            candidate.hardwareId = readDeviceProperty(deviceInfoSet, deviceInfo, SPDRP_HARDWAREID);
            candidate.manufacturer = readDeviceProperty(deviceInfoSet, deviceInfo, SPDRP_MFG);
            candidate.portName = extractComPortName(candidate.friendlyName);

            if (candidate.portName.isEmpty()) {
                candidate.portName = readPortNameFromRegistry(deviceInfoSet, deviceInfo);
            }

            if (candidate.portName.isEmpty() || seenPorts.contains(candidate.portName)) {
                continue;
            }

            seenPorts.insert(candidate.portName);
            candidate.score = scoreSerialCandidate(candidate);
            candidates.append(candidate);
        }

        SetupDiDestroyDeviceInfoList(deviceInfoSet);
    }

    std::sort(candidates.begin(), candidates.end(), [](const SerialPortCandidate& left, const SerialPortCandidate& right) {
        if (left.score != right.score) {
            return left.score > right.score;
        }
        return serialPortNumber(left.portName) < serialPortNumber(right.portName);
    });

    return candidates;
}

int countPrintableAscii(const QByteArray& bytes)
{
    int printable = 0;
    for (const unsigned char ch : bytes) {
        if (ch == '\r' || ch == '\n' || ch == '\t' || (ch >= 32 && ch <= 126)) {
            ++printable;
        }
    }
    return printable;
}
#endif

} // namespace

QArduino::QArduino(QObject* parent)
    : QObject(parent)
{
    m_serialReadTimer = new QTimer(this);
    m_serialReadTimer->setInterval(100);
    connect(m_serialReadTimer, &QTimer::timeout, this, &QArduino::readSerial);

    m_serialReconnectTimer = new QTimer(this);
    m_serialReconnectTimer->setInterval(2000);
    connect(m_serialReconnectTimer, &QTimer::timeout, this, &QArduino::setupArduino);
}

QArduino::~QArduino()
{
    stop();
}

void QArduino::start()
{
    setupArduino();
}

void QArduino::stop()
{
#ifdef Q_OS_WIN
    if (m_serialReconnectTimer) {
        m_serialReconnectTimer->stop();
    }
    if (m_serialReadTimer) {
        m_serialReadTimer->stop();
    }
    closeSerialHandle();
#endif

    serialBuffer.clear();
    pendingUid.clear();
    pendingUidMs = 0;
    m_currentBaudRate = 0;
    m_connectedAtMs = 0;
    m_garbageReadCount = 0;
    m_hasSeenUsefulFrame = false;
    m_lastDoorOpenedUid.clear();
    m_lastDoorOpenedMs = 0;
}

bool QArduino::isConnected() const
{
#ifdef Q_OS_WIN
    return arduinoHandle != INVALID_HANDLE_VALUE;
#else
    return false;
#endif
}

bool QArduino::sendAccessDecision(const QString& uid, bool allow)
{
#ifdef Q_OS_WIN
    const QString cleanUid = normalizeUidText(uid);
    QString command = allow ? QStringLiteral("OPEN") : QStringLiteral("DENY");
    if (!cleanUid.isEmpty()) {
        command += QLatin1Char(':');
        command += cleanUid;
    }

    if (!writeLine(command)) {
        emit statusMessage(QStringLiteral("Impossible d'envoyer %1 a l'Arduino.")
                               .arg(allow ? QStringLiteral("OPEN") : QStringLiteral("DENY")));
        return false;
    }

    emit statusMessage(QStringLiteral("Commande %1 envoyee a l'Arduino pour %2.")
                           .arg(allow ? QStringLiteral("OPEN") : QStringLiteral("DENY"),
                                cleanUid.isEmpty() ? QStringLiteral("badge inconnu") : cleanUid));
    return true;
#else
    Q_UNUSED(uid);
    Q_UNUSED(allow);
    emit statusMessage(QStringLiteral("Support serie Win32 uniquement pour le moment."));
    return false;
#endif
}

void QArduino::uploadFirmware(const QString& hexPath, const QString& avrdudePath, const QString& configPath)
{
    const QString portName = !m_lastPortName.isEmpty()
        ? m_lastPortName
        : (m_lastComPort > 0 ? QStringLiteral("COM%1").arg(m_lastComPort) : QString());

    if (portName.isEmpty()) {
        emit uploadFinished(false, QStringLiteral("Erreur: aucun port COM detecte. Connectez l'Arduino d'abord."));
        return;
    }

    stop();
    emit statusMessage(QStringLiteral("Preparation du televersement sur %1...").arg(portName));

    QProcess* process = new QProcess(this);

    QStringList arguments;
    arguments << "-C" << configPath
              << "-v"
              << "-p" << "atmega328p"
              << "-c" << "arduino"
              << "-P" << portName
              << "-b" << "115200"
              << "-D"
              << "-U" << QStringLiteral("flash:w:%1:i").arg(hexPath);

    connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
        const QString output = QString::fromLocal8Bit(process->readAllStandardError()).trimmed();
        if (!output.isEmpty()) {
            emit statusMessage(output);
        }
    });

    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
        const QString output = QString::fromLocal8Bit(process->readAllStandardOutput()).trimmed();
        if (!output.isEmpty()) {
            emit statusMessage(output);
        }
    });

    connect(process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
                if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                    emit uploadFinished(true, QStringLiteral("Televersement reussi."));
                } else {
                    emit uploadFinished(false,
                                        QStringLiteral("Echec du televersement (code %1).").arg(exitCode));
                }

                process->deleteLater();
                QTimer::singleShot(1500, this, &QArduino::start);
            });

    process->start(avrdudePath, arguments);
    if (!process->waitForStarted(5000)) {
        const QString errorText = process->errorString();
        emit uploadFinished(false,
                            QStringLiteral("Impossible de lancer avrdude: %1").arg(errorText));
        process->deleteLater();
        QTimer::singleShot(1500, this, &QArduino::start);
    }
}

QString QArduino::normalizeKey(QString text) const
{
    QString out = text.normalized(QString::NormalizationForm_D);
    static const QRegularExpression diacriticRx(QStringLiteral("\\p{Mn}+"));
    out.remove(diacriticRx);
    out.replace(QRegularExpression(QStringLiteral("[\\s_]+")), QString());
    return out.toLower();
}

#ifdef Q_OS_WIN
void QArduino::closeSerialHandle()
{
    if (arduinoHandle == INVALID_HANDLE_VALUE) {
        return;
    }

    PurgeComm(arduinoHandle, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
    CloseHandle(arduinoHandle);
    arduinoHandle = INVALID_HANDLE_VALUE;
}

void QArduino::scheduleReconnect(int intervalMs)
{
    if (!m_serialReconnectTimer) {
        return;
    }

    m_serialReconnectTimer->start(intervalMs);
}

void QArduino::handleSerialFailure(const QString& reason, bool rotateProfile)
{
    if (!reason.trimmed().isEmpty()) {
        emit statusMessage(reason.trimmed());
    }

    if (rotateProfile) {
        ++m_openAttemptIndex;
    }

    if (m_serialReadTimer) {
        m_serialReadTimer->stop();
    }

    closeSerialHandle();
    serialBuffer.clear();
    pendingUid.clear();
    pendingUidMs = 0;
    m_currentBaudRate = 0;
    m_connectedAtMs = 0;
    m_garbageReadCount = 0;
    m_hasSeenUsefulFrame = false;
    m_lastDoorOpenedUid.clear();
    m_lastDoorOpenedMs = 0;

    scheduleReconnect(1000);
}

QString QArduino::normalizeUidText(QString uid) const
{
    QString cleaned = uid.trimmed();
    cleaned.remove(QRegularExpression(QStringLiteral("(?i)\\b(?:uid|rfid|badge|card)\\b\\s*[:=#-]*")));
    cleaned.replace(QRegularExpression(QStringLiteral("[^0-9A-Fa-f]+")), QStringLiteral(" "));
    cleaned = cleaned.simplified().toUpper();

    if (cleaned.isEmpty()) {
        return {};
    }

    if (cleaned.contains(' ')) {
        return cleaned;
    }

    if (cleaned.size() >= 8 && cleaned.size() % 2 == 0) {
        QStringList parts;
        for (int i = 0; i < cleaned.size(); i += 2) {
            parts << cleaned.mid(i, 2);
        }
        return parts.join(' ');
    }

    return cleaned;
}

bool QArduino::writeLine(const QString& line)
{
    if (arduinoHandle == INVALID_HANDLE_VALUE) {
        return false;
    }

    QByteArray payload = line.toUtf8();
    payload.append('\n');

    DWORD bytesWritten = 0;
    if (!WriteFile(arduinoHandle, payload.constData(), static_cast<DWORD>(payload.size()), &bytesWritten, nullptr)) {
        const DWORD errorCode = GetLastError();
        if (errorCode == ERROR_ACCESS_DENIED ||
            errorCode == ERROR_GEN_FAILURE ||
            errorCode == ERROR_INVALID_HANDLE ||
            errorCode == ERROR_OPERATION_ABORTED) {
            handleSerialFailure(QStringLiteral("Connexion Arduino perdue pendant l'envoi. Reconnexion..."));
        }
        return false;
    }

    FlushFileBuffers(arduinoHandle);
    return bytesWritten == static_cast<DWORD>(payload.size());
}

QString QArduino::extractUid(const QString& line) const
{
    const QString trimmed = line.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }

    // 1. Détection d'un code hexadécimal espacé (ex: "C3 FC 05 12")
    // On le fait AVANT la détection par libellé pour éviter de capturer des bouts de mots (ex: "de" dans "detecte")
    static const QRegularExpression spacedHexRx(
        QStringLiteral("\\b([0-9A-Fa-f]{2}(?:[\\s:-]+[0-9A-Fa-f]{2}){1,9})\\b"));
    if (const QRegularExpressionMatch match = spacedHexRx.match(trimmed); match.hasMatch()) {
        return normalizeUidText(match.captured(1));
    }

    // 2. Détection avec libellé (UID:, RFID:, Badge:, etc.)
    static const QRegularExpression labeledRx(
        QStringLiteral("(?i)\\b(?:uid|rfid|badge|card\\s*uid|id|code)\\b\\s*[:=#-]*\\s*([0-9A-Fa-f\\s:-]{3,})"));
    if (const QRegularExpressionMatch match = labeledRx.match(trimmed); match.hasMatch()) {
        return normalizeUidText(match.captured(1));
    }

    // 3. Détection d'un code compact (ex: "C3FC0512" ou "150")
    // On accepte les codes de 3 à 24 caractères si la ligne semble être un ID
    static const QRegularExpression compactHexRx(QStringLiteral("\\b([0-9A-Fa-f]{3,24})\\b"));
    if (const QRegularExpressionMatch match = compactHexRx.match(trimmed); match.hasMatch()) {
        // Pour les codes courts, on vérifie que la ligne ne contient pas trop d'autres choses
        if (trimmed.length() < 30) {
            return normalizeUidText(match.captured(1));
        }
    }

    return {};
}
#endif

void QArduino::setupArduino()
{
#ifdef Q_OS_WIN
    if (arduinoHandle != INVALID_HANDLE_VALUE) {
        if (m_serialReconnectTimer && m_serialReconnectTimer->isActive()) {
            m_serialReconnectTimer->stop();
        }
        return;
    }

    const QList<SerialPortCandidate> candidates = enumerateSerialCandidates();
    if (candidates.isEmpty()) {
        scheduleReconnect(2000);
        return;
    }

    const int baudCount = kSupportedBaudRates.size();
    const int combinationCount = candidates.size() * baudCount;
    const int startIndex = combinationCount > 0
        ? ((m_openAttemptIndex % combinationCount) + combinationCount) % combinationCount
        : 0;

    bool busyPortDetected = false;

    for (int offset = 0; offset < combinationCount; ++offset) {
        const int combinationIndex = (startIndex + offset) % combinationCount;
        const SerialPortCandidate& candidate = candidates.at(combinationIndex / baudCount);
        const int baudRate = kSupportedBaudRates.at(combinationIndex % baudCount);
        const QString devicePath = QStringLiteral("\\\\.\\%1").arg(candidate.portName);

        HANDLE handle = CreateFileW(reinterpret_cast<LPCWSTR>(devicePath.utf16()),
                                    GENERIC_READ | GENERIC_WRITE,
                                    0,
                                    nullptr,
                                    OPEN_EXISTING,
                                    0,
                                    nullptr);

        if (handle == INVALID_HANDLE_VALUE) {
            const DWORD errorCode = GetLastError();
            if (errorCode == ERROR_ACCESS_DENIED) {
                busyPortDetected = true;
            }
            continue;
        }

        DCB serialParams;
        ZeroMemory(&serialParams, sizeof(serialParams));
        serialParams.DCBlength = sizeof(serialParams);

        if (!GetCommState(handle, &serialParams)) {
            CloseHandle(handle);
            continue;
        }

        serialParams.BaudRate = static_cast<DWORD>(baudRate);
        serialParams.ByteSize = 8;
        serialParams.StopBits = ONESTOPBIT;
        serialParams.Parity = NOPARITY;
        serialParams.fBinary = TRUE;
        serialParams.fDtrControl = DTR_CONTROL_ENABLE;
        serialParams.fRtsControl = RTS_CONTROL_ENABLE;
        serialParams.fOutxCtsFlow = FALSE;
        serialParams.fOutxDsrFlow = FALSE;
        serialParams.fDsrSensitivity = FALSE;
        serialParams.fOutX = FALSE;
        serialParams.fInX = FALSE;
        serialParams.fErrorChar = FALSE;
        serialParams.fNull = FALSE;
        serialParams.fAbortOnError = FALSE;

        if (!SetCommState(handle, &serialParams)) {
            CloseHandle(handle);
            continue;
        }

        // Forcer les signaux physiques pour réveiller certains clones d'Arduino
        EscapeCommFunction(handle, SETDTR);
        EscapeCommFunction(handle, SETRTS);

        COMMTIMEOUTS timeouts;
        ZeroMemory(&timeouts, sizeof(timeouts));
        timeouts.ReadIntervalTimeout = 50;
        timeouts.ReadTotalTimeoutConstant = 50;
        timeouts.ReadTotalTimeoutMultiplier = 10;
        timeouts.WriteTotalTimeoutConstant = 50;
        timeouts.WriteTotalTimeoutMultiplier = 10;

        if (!SetCommTimeouts(handle, &timeouts)) {
            CloseHandle(handle);
            continue;
        }

        PurgeComm(handle, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);

        arduinoHandle = handle;
        serialBuffer.clear();
        pendingUid.clear();
        pendingUidMs = 0;
        m_lastPortName = candidate.portName;
        m_lastComPort = serialPortNumber(candidate.portName);
        m_currentBaudRate = baudRate;
        m_connectedAtMs = QDateTime::currentMSecsSinceEpoch();
        m_lastDataReceivedMs = 0;
        m_garbageReadCount = 0;
        m_hasSeenUsefulFrame = false;
        m_lastDoorOpenedUid.clear();
        m_lastDoorOpenedMs = 0;
        m_openAttemptIndex = combinationIndex;

        if (m_serialReconnectTimer) {
            m_serialReconnectTimer->stop();
        }
        if (m_serialReadTimer && !m_serialReadTimer->isActive()) {
            m_serialReadTimer->start();
        }

        const QString deviceLabel = !candidate.friendlyName.isEmpty()
            ? QStringLiteral(" - %1").arg(candidate.friendlyName)
            : QString();
        emit statusMessage(QStringLiteral("Arduino connecte sur %1 (%2 bauds)%3")
                               .arg(candidate.portName)
                               .arg(baudRate)
                               .arg(deviceLabel));
        return;
    }

    if (busyPortDetected) {
        emit statusMessage(QStringLiteral("Port serie detecte mais occupe. Fermez l'IDE Arduino puis reessayez."));
    }

    scheduleReconnect(2000);
#else
    emit statusMessage(QStringLiteral("Support serie Win32 uniquement pour le moment."));
#endif
}

void QArduino::readSerial()
{
#ifdef Q_OS_WIN
    if (arduinoHandle == INVALID_HANDLE_VALUE) {
        return;
    }

    DWORD bytesRead = 0;
    char buffer[256];

    if (!ReadFile(arduinoHandle, buffer, sizeof(buffer), &bytesRead, nullptr)) {
        const DWORD errorCode = GetLastError();
        if (errorCode == ERROR_ACCESS_DENIED ||
            errorCode == ERROR_GEN_FAILURE ||
            errorCode == ERROR_INVALID_HANDLE ||
            errorCode == ERROR_OPERATION_ABORTED) {
            handleSerialFailure(QStringLiteral("Connexion Arduino perdue. Reconnexion..."));
        }
        return;
    }

    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();

    // Log de pulsation toutes les 10 secondes pour confirmer que readSerial tourne
        static qint64 lastHeartbeat = 0;
        if (nowMs - lastHeartbeat > 10000) {
            lastHeartbeat = nowMs;
            qDebug() << "[SERIAL] Heartbeat - En attente de donnees sur" << m_lastPortName << "a" << m_currentBaudRate << "bauds";
        }

    if (bytesRead > 0) {
        m_lastDataReceivedMs = nowMs; // On a reçu quelque chose !
        const QByteArray incoming(buffer, static_cast<int>(bytesRead));
        qDebug() << "[SERIAL] Recu :" << incoming << "(" << bytesRead << "octets)";
        
        const int printableCount = countPrintableAscii(incoming);

        if (!m_hasSeenUsefulFrame && incoming.size() >= 6) {
            const int printableRatio = (printableCount * 100) / incoming.size();
            if (printableRatio < 60) {
                ++m_garbageReadCount;
                if (m_garbageReadCount >= 3 && (nowMs - m_connectedAtMs) <= 10000) {
                    handleSerialFailure(QStringLiteral("Donnees serie illisibles sur %1 a %2 bauds. Nouvel essai...")
                                            .arg(m_lastPortName.isEmpty() ? QStringLiteral("Arduino") : m_lastPortName)
                                            .arg(m_currentBaudRate),
                                        true);
                    return;
                }
            }
        }

        serialBuffer += QString::fromLatin1(incoming);
        serialBuffer.replace('\r', '\n');

        // Si le buffer devient trop gros sans retour à la ligne, on force un traitement
        if (serialBuffer.size() > 512) {
            serialBuffer.append('\n');
        }

        while (true) {
            const int newlineIndex = serialBuffer.indexOf('\n');
            QString line;
            bool hasNewline = false;

            if (newlineIndex >= 0) {
                line = serialBuffer.left(newlineIndex).trimmed();
                serialBuffer.remove(0, newlineIndex + 1);
                hasNewline = true;
            } else if (serialBuffer.size() > 0 && serialBuffer.size() < 50) {
                // Si pas de newline mais petit buffer (ex: juste un ID "150"), 
                // on tente de l'extraire si le buffer n'a pas bougé depuis un moment.
                line = serialBuffer.trimmed();
                // On ne vide pas le buffer ici car on attend peut-être encore la suite,
                // SAUF si extractUid réussit.
            } else {
                break;
            }

            if (line.isEmpty()) {
                if (hasNewline) continue;
                else break;
            }

            qDebug() << "[SERIAL] Analyse du buffer :" << line;

            QString uidFromLine = extractUid(line);
            const QString key = normalizeKey(line);
            const bool doorOpenedFlag =
                key.contains(QStringLiteral("porteouverte")) ||
                key.contains(QStringLiteral("dooropen")) ||
                key.contains(QStringLiteral("accesautorise")) ||
                key.contains(QStringLiteral("accesaccepte")) ||
                key.contains(QStringLiteral("accessgranted")) ||
                key.contains(QStringLiteral("authorized")) ||
                key.contains(QStringLiteral("unlock")) ||
                key.contains(QStringLiteral("serrureouverte")) ||
                key == QStringLiteral("open");
            const bool accessDeniedFlag =
                key.contains(QStringLiteral("accessdenied")) ||
                key.contains(QStringLiteral("accesrefuse")) ||
                key.contains(QStringLiteral("accesdenied")) ||
                key.contains(QStringLiteral("accessrefused")) ||
                key == QStringLiteral("deny");
            if (!uidFromLine.isEmpty()) {
                qDebug() << "[SERIAL] UID EXTRAIT :" << uidFromLine;
                
                // Si on a extrait un UID d'un buffer sans newline, on vide le buffer pour éviter de le relire
                if (!hasNewline) {
                    serialBuffer.clear();
                }

                if (!doorOpenedFlag && !accessDeniedFlag) {
                    pendingUid = uidFromLine;
                    pendingUidMs = nowMs;
                    m_hasSeenUsefulFrame = true;
                    emit uidReceived(uidFromLine);
                }
                if (!hasNewline) break; // On a fini avec ce petit morceau
            } else if (hasNewline) {
                qDebug() << "⚠️ Aucun UID détecté dans cette ligne.";
            } else {
                // Pas d'UID et pas de newline, on attend la suite.
                break;
            }

            if (doorOpenedFlag) {
                m_hasSeenUsefulFrame = true;
                const QString uidToApply = !uidFromLine.isEmpty() ? uidFromLine : pendingUid;
                if (!uidToApply.isEmpty()) {
                    const QString uidKey = normalizeUidText(uidToApply);
                    constexpr qint64 kDoorOpenedCooldownMs = 2500;
                    if (!(uidKey == m_lastDoorOpenedUid &&
                          m_lastDoorOpenedMs > 0 &&
                          (nowMs - m_lastDoorOpenedMs) < kDoorOpenedCooldownMs)) {
                        emit doorOpened(uidToApply);
                        m_lastDoorOpenedUid = uidKey;
                        m_lastDoorOpenedMs = nowMs;
                    }
                    pendingUid.clear();
                    pendingUidMs = 0;
                }
            } else if (accessDeniedFlag) {
                m_hasSeenUsefulFrame = true;
                const QString deniedUid = !uidFromLine.isEmpty() ? uidFromLine : pendingUid;
                emit statusMessage(
                    deniedUid.isEmpty()
                        ? QStringLiteral("Acces refuse par l'Arduino.")
                        : QStringLiteral("Acces refuse pour le badge %1.").arg(deniedUid));
                pendingUid.clear();
                pendingUidMs = 0;
            }
        }
    }

    // En mode Qt pur, on ne force pas de refus automatique si aucune
    // confirmation Arduino n'arrive pas dans les 5 secondes.
    // Le badge reste simplement en attente jusqu'à un nouvel événement.
#endif
}
