#include "arduino.h"

#include <QSerialPort>
#include <QSerialPortInfo>

arduino::arduino()
	: m_serial(new QSerialPort)
{
}

arduino::~arduino()
{
	close_arduino();
	delete m_serial;
	m_serial = nullptr;
}

int arduino::connect_arduino()
{
	if (!m_serial) {
		return ARDUINO_NOT_AVAILABLE;
	}

	if (m_serial->isOpen()) {
		return ARDUINO_AVAILABLE;
	}

	QString selectedPort;
	const QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
	for (const QSerialPortInfo& info : ports) {
		const bool looksLikeArduino =
			info.description().contains(QStringLiteral("arduino"), Qt::CaseInsensitive)
			|| info.description().contains(QStringLiteral("ch340"), Qt::CaseInsensitive)
			|| info.description().contains(QStringLiteral("cp210"), Qt::CaseInsensitive)
			|| info.manufacturer().contains(QStringLiteral("arduino"), Qt::CaseInsensitive)
			|| (info.hasVendorIdentifier()
				&& (info.vendorIdentifier() == 0x2341
					|| info.vendorIdentifier() == 0x1A86
					|| info.vendorIdentifier() == 0x10C4
					|| info.vendorIdentifier() == 0x0403));

		if (looksLikeArduino) {
			selectedPort = info.portName();
			break;
		}
	}

	if (selectedPort.isEmpty() && !ports.isEmpty()) {
		selectedPort = ports.first().portName();
	}

	if (selectedPort.isEmpty()) {
		return ARDUINO_NOT_AVAILABLE;
	}

	m_serial->setPortName(selectedPort);
	m_serial->setBaudRate(QSerialPort::Baud9600);
	m_serial->setDataBits(QSerialPort::Data8);
	m_serial->setParity(QSerialPort::NoParity);
	m_serial->setStopBits(QSerialPort::OneStop);
	m_serial->setFlowControl(QSerialPort::NoFlowControl);

	if (!m_serial->open(QIODevice::ReadWrite)) {
		return ARDUINO_NOT_AVAILABLE;
	}

	m_portName = selectedPort;
	return ARDUINO_AVAILABLE;
}

int arduino::close_arduino()
{
	if (m_serial && m_serial->isOpen()) {
		m_serial->close();
	}
	return ARDUINO_AVAILABLE;
}

QByteArray arduino::read_from_arduino()
{
	if (!m_serial || !m_serial->isOpen()) {
		return QByteArray();
	}
	return m_serial->readAll();
}

bool arduino::write_to_arduino(const QByteArray &data)
{
	if (!m_serial || !m_serial->isOpen()) {
		return false;
	}

	const qint64 bytesWritten = m_serial->write(data);
	return bytesWritten == data.size();
}

QString arduino::getarduino_port_name() const
{
	return m_portName;
}

QSerialPort* arduino::serial() const
{
	return m_serial;
}
