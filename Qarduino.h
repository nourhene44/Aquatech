#ifndef QARDUINO_H
#define QARDUINO_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QDateTime>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class QArduino : public QObject
{
    Q_OBJECT
public:
    explicit QArduino(QObject *parent = nullptr);
    ~QArduino();

    void start();
    void stop();
    bool isConnected() const;
    bool sendAccessDecision(const QString& uid, bool allow);
    void uploadFirmware(const QString& hexPath, const QString& avrdudePath, const QString& configPath);

signals:
    void uidReceived(const QString &uid);
    void doorOpened(const QString &uid);
    void statusMessage(const QString &msg);
    void uploadProgress(int progress);
    void uploadFinished(bool success, const QString& message);

private slots:
    void setupArduino();
    void readSerial();

private:
    QString normalizeKey(QString text) const;

#ifdef Q_OS_WIN
    void closeSerialHandle();
    void scheduleReconnect(int intervalMs = 2000);
    void handleSerialFailure(const QString& reason, bool rotateProfile = false);
    QString extractUid(const QString& line) const;
    QString normalizeUidText(QString uid) const;
    bool writeLine(const QString& line);

    HANDLE arduinoHandle = INVALID_HANDLE_VALUE;
#endif

    QString serialBuffer;
    QTimer* m_serialReadTimer = nullptr;
    QTimer* m_serialReconnectTimer = nullptr;
    int m_lastComPort = -1;
    QString m_lastPortName;
    int m_currentBaudRate = 0;
    int m_openAttemptIndex = 0;
    qint64 m_connectedAtMs = 0;
    qint64 m_lastDataReceivedMs = 0;
    int m_garbageReadCount = 0;
    bool m_hasSeenUsefulFrame = false;
    QString m_lastDoorOpenedUid;
    qint64 m_lastDoorOpenedMs = 0;

    QString pendingUid;
    qint64 pendingUidMs = 0;
};

#endif // QARDUINO_H
