#ifndef ARDUINOSERIAL_H
#define ARDUINOSERIAL_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QStringList>

#ifdef HAVE_SERIALPORT
#include <QSerialPort>
#endif

class ArduinoSerial final : public QObject
{
    Q_OBJECT
public:
    explicit ArduinoSerial(QObject* parent = nullptr);

    bool serialPortAvailable() const;

    QString connectedPortName() const;

    QStringList availablePortNames() const;

    bool connectToPort(const QString& portName,
                       int baudRate = 9600,
                       QString* errorOut = nullptr);

    void disconnectFromPort();

    bool isConnected() const;

    bool writeLine(const QString& line, QString* errorOut = nullptr);

signals:
    void connected(const QString& portName);
    void disconnected();
    void lineReceived(const QString& line);
    void errorOccurred(const QString& message);

private:
    void handleIncomingBytes(const QByteArray& bytes);

#ifdef HAVE_SERIALPORT
    QSerialPort m_port;
#endif
    QByteArray m_rxBuffer;
};

#endif // ARDUINOSERIAL_H
