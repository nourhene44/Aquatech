#ifndef ARDUINOSERIAL_H
#define ARDUINOSERIAL_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QtGlobal>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(HAVE_SERIALPORT)
#include <QSerialPort>
#endif

class ArduinoSerial final : public QObject
{
    Q_OBJECT
public:
    explicit ArduinoSerial(QObject* parent = nullptr);
    ~ArduinoSerial() override;

    bool serialPortAvailable() const;

    QString connectedPortName() const;

    QStringList availablePortNames() const;

    bool connectToPort(const QString& portName,
                       int baudRate = 9600,
                       QString* errorOut = nullptr);

    void disconnectFromPort();

    bool isConnected() const;

    bool writeLine(const QString& line, QString* errorOut = nullptr);
    bool sendAccessDecision(const QString& uid, bool allow);

signals:
    void connected(const QString& portName);
    void disconnected();
    void lineReceived(const QString& line);
    void uidReceived(const QString& uid);
    void doorOpened(const QString& uid);
    void statusMessage(const QString& message);
    void errorOccurred(const QString& message);

private:
    void handleIncomingBytes(const QByteArray& bytes);
    void resetRuntimeState();
    static QString normalizeKey(QString text);
    QString normalizeUidText(QString uid) const;
    QString extractUid(const QString& line) const;

#ifdef Q_OS_WIN
    void pollWinSerial();
    void closeWinHandle();
    bool openWinPort(const QString& portName, int baudRate, QString* errorOut);
    void tryNextBaudProfile(const QString& reason);
    QString normalizedPortName(const QString& portName) const;

    HANDLE m_winHandle = INVALID_HANDLE_VALUE;
    QTimer* m_winPollTimer = nullptr;
    QString m_selectedPortName;
    QList<int> m_baudCandidates;
    int m_baudCandidateIndex = 0;
    int m_currentBaudRate = 0;
    qint64 m_connectedAtMs = 0;
    qint64 m_lastDataReceivedMs = 0;
    int m_garbageReadCount = 0;
    bool m_hasSeenUsefulFrame = false;
#elif defined(HAVE_SERIALPORT)
    QSerialPort m_port;
#endif

    QString m_rxBuffer;
    QString m_pendingUid;
    qint64 m_pendingUidMs = 0;
    QString m_lastAllowedUid;
    qint64 m_lastAllowedUidMs = 0;
    QString m_lastDoorOpenedUid;
    qint64 m_lastDoorOpenedMs = 0;
};

#endif // ARDUINOSERIAL_H
