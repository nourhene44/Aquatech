#include "arduinoserial.h"

#include <QDateTime>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>
#include <QtGlobal>
#include <QFile>

#include <algorithm>

#ifdef Q_OS_WIN
#include <regstr.h>
#include <setupapi.h>
#elif defined(HAVE_SERIALPORT)
#include <QSerialPortInfo>
#endif

namespace {

void appendArduinoDebugLog(const QString& msg)
{
    QFile f(QStringLiteral("arduino_debug.log"));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) return;
    const QString line = QDateTime::currentDateTime().toString(Qt::ISODate) + QStringLiteral(" - ") + msg + QLatin1Char('\n');
    const QByteArray bytes = line.toUtf8();
    f.write(bytes);
    f.close();
}


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
    if (!SetupDiClassGuidsFromNameW(L"Ports", portGuids, 8, &guidCount) || guidCount == 0) {
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

QString formatWindowsError(const QString& prefix, DWORD errorCode)
{
    LPWSTR buffer = nullptr;
    const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS;
    const DWORD len = FormatMessageW(flags,
                                     nullptr,
                                     errorCode,
                                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                     reinterpret_cast<LPWSTR>(&buffer),
                                     0,
                                     nullptr);

    QString suffix;
    if (len > 0 && buffer) {
        suffix = QString::fromWCharArray(buffer, static_cast<int>(len)).trimmed();
        LocalFree(buffer);
    }

    if (suffix.isEmpty()) {
        return QStringLiteral("%1 (code %2)").arg(prefix).arg(errorCode);
    }

    return QStringLiteral("%1: %2 (code %3)").arg(prefix, suffix).arg(errorCode);
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

bool isSensorTelemetryLine(const QString& line)
{
    const QString trimmed = line.trimmed().toLower();
    return trimmed.startsWith(QStringLiteral("distance:")) ||
           trimmed.startsWith(QStringLiteral("pulse:")) ||
           trimmed.startsWith(QStringLiteral("temp:")) ||
           trimmed.startsWith(QStringLiteral("temperature:")) ||
           trimmed.startsWith(QStringLiteral("[arduino]"));
}

QString compactUidText(QString uid)
{
    uid = uid.trimmed().toUpper();
    uid.remove(QRegularExpression(QStringLiteral("[^0-9A-Z]+")));
    return uid;
}
#endif

} // namespace

ArduinoSerial::ArduinoSerial(QObject* parent)
    : QObject(parent)
{
#ifdef Q_OS_WIN
    m_winPollTimer = new QTimer(this);
    m_winPollTimer->setInterval(100);
    connect(m_winPollTimer, &QTimer::timeout, this, &ArduinoSerial::pollWinSerial);
#elif defined(HAVE_SERIALPORT)
    m_port.setReadBufferSize(64 * 1024);

    connect(&m_port, &QSerialPort::readyRead, this, [this]() {
        handleIncomingBytes(m_port.readAll());
    });

    connect(&m_port,
            &QSerialPort::errorOccurred,
            this,
            [this](QSerialPort::SerialPortError error) {
                if (error == QSerialPort::NoError) return;

                const bool shouldClose =
                    error == QSerialPort::ResourceError ||
                    error == QSerialPort::DeviceNotFoundError ||
                    error == QSerialPort::PermissionError;

                const QString msg = m_port.errorString().isEmpty()
                    ? QStringLiteral("Erreur serie (code %1)").arg(static_cast<int>(error))
                    : m_port.errorString();

                if (shouldClose && m_port.isOpen()) {
                    m_port.close();
                    resetRuntimeState();
                    emit disconnected();
                }

                emit errorOccurred(msg);
            });
#endif
}

ArduinoSerial::~ArduinoSerial()
{
    disconnectFromPort();
}

bool ArduinoSerial::serialPortAvailable() const
{
#ifdef Q_OS_WIN
    return true;
#elif defined(HAVE_SERIALPORT)
    return true;
#else
    return false;
#endif
}

QString ArduinoSerial::connectedPortName() const
{
#ifdef Q_OS_WIN
    return (m_winHandle != INVALID_HANDLE_VALUE) ? m_selectedPortName : QString();
#elif defined(HAVE_SERIALPORT)
    return m_port.isOpen() ? m_port.portName() : QString();
#else
    return QString();
#endif
}

QStringList ArduinoSerial::availablePortNames() const
{
    QStringList names;

#ifdef Q_OS_WIN
    const QList<SerialPortCandidate> candidates = enumerateSerialCandidates();
    names.reserve(candidates.size());
    for (const SerialPortCandidate& candidate : candidates) {
        names.push_back(candidate.portName);
    }
#elif defined(HAVE_SERIALPORT)
    const auto ports = QSerialPortInfo::availablePorts();
    names.reserve(ports.size());
    for (const QSerialPortInfo& info : ports) {
        names.push_back(info.portName());
    }
#endif

    return names;
}

bool ArduinoSerial::connectToPort(const QString& portName, int baudRate, QString* errorOut)
{
    const QString cleanPort = normalizedPortName(portName);
    if (cleanPort.isEmpty()) {
        const QString msg = QStringLiteral("Nom du port serie invalide.");
        if (errorOut) *errorOut = msg;
        emit errorOccurred(msg);
        return false;
    }

#ifdef Q_OS_WIN
    m_selectedPortName = cleanPort;
    m_baudCandidates.clear();
    m_baudCandidates.append(baudRate);
    for (int candidate : kSupportedBaudRates) {
        if (!m_baudCandidates.contains(candidate)) {
            m_baudCandidates.append(candidate);
        }
    }
    m_baudCandidateIndex = 0;

    disconnectFromPort();

    QString localError;
    if (!openWinPort(cleanPort, m_baudCandidates.first(), &localError)) {
        if (errorOut) *errorOut = localError;
        emit errorOccurred(localError);
        return false;
    }

    emit connected(cleanPort);
    emit statusMessage(QStringLiteral("Arduino connecte sur %1 (%2 bauds).")
                           .arg(cleanPort)
                           .arg(m_currentBaudRate));
    return true;
#elif defined(HAVE_SERIALPORT)
    if (isConnected()) {
        if (m_port.portName() == cleanPort) {
            return true;
        }
        disconnectFromPort();
    }

    m_port.setPortName(cleanPort);
    m_port.setBaudRate(baudRate);
    m_port.setDataBits(QSerialPort::Data8);
    m_port.setParity(QSerialPort::NoParity);
    m_port.setStopBits(QSerialPort::OneStop);
    m_port.setFlowControl(QSerialPort::NoFlowControl);
    m_port.setDataTerminalReady(true);
    m_port.setRequestToSend(true);

    if (!m_port.open(QIODevice::ReadWrite)) {
        const QString msg = m_port.errorString();
        if (errorOut) *errorOut = msg;
        emit errorOccurred(msg);
        return false;
    }

    resetRuntimeState();
    appendArduinoDebugLog(QStringLiteral("Opened QSerialPort %1 at %2 bauds").arg(cleanPort).arg(baudRate));
    emit connected(cleanPort);
    emit statusMessage(QStringLiteral("Arduino connecte sur %1 (%2 bauds).")
                           .arg(cleanPort)
                           .arg(baudRate));
    return true;
#else
    Q_UNUSED(cleanPort);
    Q_UNUSED(baudRate);
    const QString msg = QStringLiteral("Aucun backend serie disponible.");
    if (errorOut) *errorOut = msg;
    emit errorOccurred(msg);
    return false;
#endif
}

void ArduinoSerial::disconnectFromPort()
{
#ifdef Q_OS_WIN
    const bool wasConnected = (m_winHandle != INVALID_HANDLE_VALUE);
    if (m_winPollTimer) {
        m_winPollTimer->stop();
    }
    closeWinHandle();
    resetRuntimeState();
    m_currentBaudRate = 0;
    if (wasConnected) {
        emit disconnected();
    }
#elif defined(HAVE_SERIALPORT)
    if (!m_port.isOpen()) return;

    m_port.close();
    resetRuntimeState();
    emit disconnected();
#endif
}

bool ArduinoSerial::isConnected() const
{
#ifdef Q_OS_WIN
    return m_winHandle != INVALID_HANDLE_VALUE;
#elif defined(HAVE_SERIALPORT)
    return m_port.isOpen();
#else
    return false;
#endif
}

bool ArduinoSerial::writeLine(const QString& line, QString* errorOut)
{
#ifdef Q_OS_WIN
    if (m_winHandle == INVALID_HANDLE_VALUE) {
        const QString msg = QStringLiteral("Port serie non connecte.");
        if (errorOut) *errorOut = msg;
        emit errorOccurred(msg);
        return false;
    }

    QByteArray payload = line.toUtf8();
    payload.append("\r\n");

    DWORD bytesWritten = 0;
    if (!WriteFile(m_winHandle,
                   payload.constData(),
                   static_cast<DWORD>(payload.size()),
                   &bytesWritten,
                   nullptr)) {
        const DWORD errorCode = GetLastError();
        const QString msg = formatWindowsError(QStringLiteral("Ecriture serie echouee"), errorCode);
        appendArduinoDebugLog(QStringLiteral("Serial write failed on %1: %2").arg(m_selectedPortName, msg));
        if (errorOut) *errorOut = msg;
        emit errorOccurred(msg);
        return false;
    }

    FlushFileBuffers(m_winHandle);
    const bool ok = bytesWritten == static_cast<DWORD>(payload.size());
    appendArduinoDebugLog(QStringLiteral("Serial write to %1: %2 bytes, data='%3'")
                          .arg(m_selectedPortName)
                          .arg(static_cast<qint64>(bytesWritten))
                          .arg(QString::fromUtf8(payload).trimmed()));
    return ok;
#elif defined(HAVE_SERIALPORT)
    if (!m_port.isOpen()) {
        const QString msg = QStringLiteral("Port serie non connecte.");
        appendArduinoDebugLog(QStringLiteral("Serial write attempted but port not open: %1").arg(msg));
        if (errorOut) *errorOut = msg;
        emit errorOccurred(msg);
        return false;
    }

    QByteArray data = line.toUtf8();
    data.append("\r\n");

    const qint64 written = m_port.write(data);
    if (written < 0) {
        const QString msg = m_port.errorString();
        appendArduinoDebugLog(QStringLiteral("Serial write error on %1: %2").arg(m_port.portName(), msg));
        if (errorOut) *errorOut = msg;
        emit errorOccurred(msg);
        return false;
    }

    if (!m_port.waitForBytesWritten(500)) {
        const QString msg = m_port.errorString().isEmpty()
            ? QStringLiteral("Timeout ecriture serie.")
            : m_port.errorString();
        appendArduinoDebugLog(QStringLiteral("Serial write timeout on %1: %2").arg(m_port.portName(), msg));

        if (errorOut) *errorOut = msg;
        emit errorOccurred(msg);
        return false;
    }

    appendArduinoDebugLog(QStringLiteral("Serial write to %1: %2 bytes, data='%3'")
                          .arg(m_port.portName())
                          .arg(written)
                          .arg(QString::fromUtf8(data).trimmed()));

    return true;
#else
    Q_UNUSED(line);
    const QString msg = QStringLiteral("Aucun backend serie disponible.");
    if (errorOut) *errorOut = msg;
    emit errorOccurred(msg);
    return false;
#endif
}

bool ArduinoSerial::sendAccessDecision(const QString& uid, bool allow)
{
    const QString cleanUid = normalizeUidText(uid);
    QString command = allow ? QStringLiteral("OPEN") : QStringLiteral("DENY");
    if (!cleanUid.isEmpty()) {
        command += QLatin1Char(':');
        command += cleanUid;
    }

    QString error;
    if (!writeLine(command, &error)) {
        emit statusMessage(QStringLiteral("Impossible d'envoyer %1 a l'Arduino.")
                               .arg(allow ? QStringLiteral("OPEN") : QStringLiteral("DENY")));
        return false;
    }

    if (allow && !cleanUid.isEmpty()) {
        m_pendingUid = cleanUid;
        m_pendingUidMs = QDateTime::currentMSecsSinceEpoch();
        m_lastAllowedUid = cleanUid;
        m_lastAllowedUidMs = m_pendingUidMs;
    }

    emit statusMessage(QStringLiteral("Commande %1 envoyee a l'Arduino pour %2.")
                           .arg(allow ? QStringLiteral("OPEN") : QStringLiteral("DENY"),
                                cleanUid.isEmpty() ? QStringLiteral("badge inconnu") : cleanUid));
    return true;
}

void ArduinoSerial::resetRuntimeState()
{
    m_rxBuffer.clear();
    m_pendingUid.clear();
    m_pendingUidMs = 0;
    m_lastAllowedUid.clear();
    m_lastAllowedUidMs = 0;
    m_lastDoorOpenedUid.clear();
    m_lastDoorOpenedMs = 0;
#ifdef Q_OS_WIN
    m_garbageReadCount = 0;
    m_hasSeenUsefulFrame = false;
    m_connectedAtMs = 0;
    m_lastDataReceivedMs = 0;
#endif
}

QString ArduinoSerial::normalizeKey(QString text)
{
    text = text.normalized(QString::NormalizationForm_D);
    static const QRegularExpression diacriticRx(QStringLiteral("\\p{Mn}+"));
    text.remove(diacriticRx);
    text.replace(QRegularExpression(QStringLiteral("[\\s_]+")), QString());
    return text.toLower();
}

QString ArduinoSerial::normalizeUidText(QString uid) const
{
    uid = uid.trimmed();
    uid.remove(QRegularExpression(QStringLiteral("(?i)\\b(?:uid|rfid|badge|card)\\b\\s*[:=#-]*")));
    uid.replace(QRegularExpression(QStringLiteral("[^0-9A-Fa-f]+")), QStringLiteral(" "));
    uid = uid.simplified().toUpper();

    if (uid.isEmpty()) {
        return QString();
    }

    if (uid.contains(QLatin1Char(' '))) {
        return uid;
    }

    if (uid.size() >= 8 && uid.size() % 2 == 0) {
        QStringList parts;
        for (int i = 0; i < uid.size(); i += 2) {
            parts << uid.mid(i, 2);
        }
        return parts.join(QLatin1Char(' '));
    }

    return uid;
}

QString ArduinoSerial::extractUid(const QString& line) const
{
    const QString trimmed = line.trimmed();
    if (trimmed.isEmpty()) {
        return QString();
    }

    static const QRegularExpression spacedHexRx(
        QStringLiteral("\\b([0-9A-Fa-f]{2}(?:[\\s:-]+[0-9A-Fa-f]{2}){1,9})\\b"));
    if (const QRegularExpressionMatch match = spacedHexRx.match(trimmed); match.hasMatch()) {
        return normalizeUidText(match.captured(1));
    }

    static const QRegularExpression labeledRx(
        QStringLiteral("(?i)\\b(?:uid|rfid|badge|card\\s*uid|id|code)\\b\\s*[:=#-]*\\s*([0-9A-Fa-f\\s:-]{3,})"));
    if (const QRegularExpressionMatch match = labeledRx.match(trimmed); match.hasMatch()) {
        return normalizeUidText(match.captured(1));
    }

    static const QRegularExpression compactHexRx(QStringLiteral("\\b([0-9A-Fa-f]{3,24})\\b"));
    if (const QRegularExpressionMatch match = compactHexRx.match(trimmed); match.hasMatch()) {
        if (trimmed.length() < 30) {
            return normalizeUidText(match.captured(1));
        }
    }

    return QString();
}

void ArduinoSerial::handleIncomingBytes(const QByteArray& bytes)
{
    if (bytes.isEmpty()) return;

    m_rxBuffer += QString::fromLatin1(bytes);
    m_rxBuffer.replace(QLatin1Char('\r'), QLatin1Char('\n'));

    if (m_rxBuffer.size() > 512) {
        m_rxBuffer.append(QLatin1Char('\n'));
    }

    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();

    while (true) {
        const int newlineIndex = m_rxBuffer.indexOf(QLatin1Char('\n'));
        QString line;
        bool hasNewline = false;

        if (newlineIndex >= 0) {
            line = m_rxBuffer.left(newlineIndex).trimmed();
            m_rxBuffer.remove(0, newlineIndex + 1);
            hasNewline = true;
        } else if (!m_rxBuffer.isEmpty() && m_rxBuffer.size() < 50) {
            line = m_rxBuffer.trimmed();
        } else {
            break;
        }

        if (line.isEmpty()) {
            if (hasNewline) {
                continue;
            }
            break;
        }

        const bool telemetryLine = isSensorTelemetryLine(line);
        const QString uidFromLine = telemetryLine ? QString() : extractUid(line);
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

        if (hasNewline && uidFromLine.isEmpty() && !doorOpenedFlag && !accessDeniedFlag) {
            emit lineReceived(line);
        }

        if (!uidFromLine.isEmpty()) {
            if (!hasNewline) {
                m_rxBuffer.clear();
            }

            if (!doorOpenedFlag && !accessDeniedFlag) {
                m_pendingUid = uidFromLine;
                m_pendingUidMs = nowMs;
#ifdef Q_OS_WIN
                m_hasSeenUsefulFrame = true;
#endif
                appendArduinoDebugLog(QStringLiteral("UID received: %1 (line: %2)").arg(uidFromLine, line));
                emit uidReceived(uidFromLine);
            }

            if (!hasNewline) {
                break;
            }
        } else if (!hasNewline) {
            break;
        }

        if (doorOpenedFlag) {
#ifdef Q_OS_WIN
            m_hasSeenUsefulFrame = true;
#endif
            QString uidToApply = uidFromLine;
            constexpr qint64 kUidAssociationMaxAgeMs = 10000;
            if (uidToApply.isEmpty() &&
                !m_pendingUid.isEmpty() &&
                m_pendingUidMs > 0 &&
                (nowMs - m_pendingUidMs) < kUidAssociationMaxAgeMs) {
                uidToApply = m_pendingUid;
            }
            if (uidToApply.isEmpty() &&
                !m_lastAllowedUid.isEmpty() &&
                m_lastAllowedUidMs > 0 &&
                (nowMs - m_lastAllowedUidMs) < kUidAssociationMaxAgeMs) {
                uidToApply = m_lastAllowedUid;
            }

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
            } else {
                emit statusMessage(QStringLiteral("Ouverture de porte detectee, mais aucun UID n'a pu etre associe."));
            }
            m_pendingUid.clear();
            m_pendingUidMs = 0;
            m_lastAllowedUid.clear();
            m_lastAllowedUidMs = 0;
        } else if (accessDeniedFlag) {
#ifdef Q_OS_WIN
            m_hasSeenUsefulFrame = true;
#endif
            QString deniedUid = uidFromLine;
            if (deniedUid.isEmpty() &&
                !m_pendingUid.isEmpty() &&
                m_pendingUidMs > 0 &&
                (nowMs - m_pendingUidMs) < 10000) {
                deniedUid = m_pendingUid;
            }
            emit statusMessage(
                deniedUid.isEmpty()
                    ? QStringLiteral("Acces refuse par l'Arduino.")
                    : QStringLiteral("Acces refuse pour le badge %1.").arg(deniedUid));
            m_pendingUid.clear();
            m_pendingUidMs = 0;
        }
    }
}

#ifdef Q_OS_WIN
QString ArduinoSerial::normalizedPortName(const QString& portName) const
{
    return portName.trimmed().toUpper();
}

void ArduinoSerial::closeWinHandle()
{
    if (m_winHandle == INVALID_HANDLE_VALUE) {
        return;
    }

    PurgeComm(m_winHandle, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
    CloseHandle(m_winHandle);
    m_winHandle = INVALID_HANDLE_VALUE;
}

bool ArduinoSerial::openWinPort(const QString& portName, int baudRate, QString* errorOut)
{
    const QString cleanPort = normalizedPortName(portName);
    const QString devicePath = QStringLiteral("\\\\.\\%1").arg(cleanPort);

    HANDLE handle = CreateFileW(reinterpret_cast<LPCWSTR>(devicePath.utf16()),
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                nullptr,
                                OPEN_EXISTING,
                                0,
                                nullptr);

    if (handle == INVALID_HANDLE_VALUE) {
        const DWORD errorCode = GetLastError();
        const QString msg = formatWindowsError(QStringLiteral("Impossible d'ouvrir %1").arg(cleanPort), errorCode);
        if (errorOut) *errorOut = msg;
        return false;
    }

    DCB serialParams;
    ZeroMemory(&serialParams, sizeof(serialParams));
    serialParams.DCBlength = sizeof(serialParams);

    if (!GetCommState(handle, &serialParams)) {
        const QString msg = formatWindowsError(QStringLiteral("Lecture de la configuration serie echouee"), GetLastError());
        CloseHandle(handle);
        if (errorOut) *errorOut = msg;
        return false;
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
        const QString msg = formatWindowsError(QStringLiteral("Configuration du port serie echouee"), GetLastError());
        CloseHandle(handle);
        if (errorOut) *errorOut = msg;
        return false;
    }

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
        const QString msg = formatWindowsError(QStringLiteral("Configuration des timeouts serie echouee"), GetLastError());
        CloseHandle(handle);
        if (errorOut) *errorOut = msg;
        return false;
    }

    PurgeComm(handle, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);

    m_winHandle = handle;
    resetRuntimeState();
    m_selectedPortName = cleanPort;
    m_currentBaudRate = baudRate;
    m_connectedAtMs = QDateTime::currentMSecsSinceEpoch();
    if (m_winPollTimer && !m_winPollTimer->isActive()) {
        m_winPollTimer->start();
    }
    appendArduinoDebugLog(QStringLiteral("Opened Windows serial port %1 at %2 bauds").arg(cleanPort).arg(baudRate));
    return true;
}

void ArduinoSerial::tryNextBaudProfile(const QString& reason)
{
    if (m_selectedPortName.isEmpty() || m_baudCandidates.isEmpty()) {
        if (!reason.trimmed().isEmpty()) {
            emit statusMessage(reason.trimmed());
        }
        return;
    }

    if (m_baudCandidateIndex + 1 >= m_baudCandidates.size()) {
        if (!reason.trimmed().isEmpty()) {
            emit statusMessage(reason.trimmed());
        }
        return;
    }

    const QString targetPort = m_selectedPortName;
    disconnectFromPort();

    ++m_baudCandidateIndex;
    QString error;
    if (!openWinPort(targetPort, m_baudCandidates.at(m_baudCandidateIndex), &error)) {
        emit errorOccurred(error);
        emit statusMessage(error);
        return;
    }

    emit connected(targetPort);
    emit statusMessage(QStringLiteral("%1 Nouvel essai sur %2 (%3 bauds).")
                           .arg(reason.trimmed().isEmpty()
                                    ? QStringLiteral("Reconfiguration serie.")
                                    : reason.trimmed(),
                                targetPort)
                           .arg(m_currentBaudRate));
}

void ArduinoSerial::pollWinSerial()
{
    if (m_winHandle == INVALID_HANDLE_VALUE) {
        return;
    }

    DWORD bytesRead = 0;
    char buffer[256];

    if (!ReadFile(m_winHandle, buffer, sizeof(buffer), &bytesRead, nullptr)) {
        const QString msg = formatWindowsError(QStringLiteral("Lecture serie echouee"), GetLastError());
        emit errorOccurred(msg);
        disconnectFromPort();
        return;
    }

    if (bytesRead == 0) {
        const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
        if (!m_rxBuffer.trimmed().isEmpty() &&
            m_lastDataReceivedMs > 0 &&
            (nowMs - m_lastDataReceivedMs) >= 200) {
            handleIncomingBytes(QByteArray("\n"));
        }
        return;
    }

    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    m_lastDataReceivedMs = nowMs;

    const QByteArray incoming(buffer, static_cast<int>(bytesRead));
    if (!m_hasSeenUsefulFrame && incoming.size() >= 6) {
        const int printableRatio = (countPrintableAscii(incoming) * 100) / incoming.size();
        if (printableRatio < 60) {
            ++m_garbageReadCount;
            if (m_garbageReadCount >= 3 && m_connectedAtMs > 0 && (nowMs - m_connectedAtMs) <= 10000) {
                tryNextBaudProfile(QStringLiteral("Donnees serie illisibles detectees."));
                return;
            }
        }
    }

    handleIncomingBytes(incoming);
}
#endif
