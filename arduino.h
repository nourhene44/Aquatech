#ifndef ARDUINO_H
#define ARDUINO_H

#include <QByteArray>
#include <QString>

class QSerialPort;

class arduino
{
public:
    static const int ARDUINO_AVAILABLE = 0;
    static const int ARDUINO_NOT_AVAILABLE = -1;

    arduino();
    ~arduino();

    int connect_arduino();
    int close_arduino();

    QByteArray read_from_arduino();
    bool write_to_arduino(const QByteArray &data);

    QString getarduino_port_name() const;
    QSerialPort* serial() const;

private:
    QSerialPort* m_serial = nullptr;
    QString m_portName;
};

#endif // ARDUINO_H
