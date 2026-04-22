#include "arduino.h"

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>

static bool looksLikeArduino(const QSerialPortInfo& info)
{
	return info.description().contains(QStringLiteral("arduino"), Qt::CaseInsensitive)
		|| info.description().contains(QStringLiteral("ch340"), Qt::CaseInsensitive)
		|| info.description().contains(QStringLiteral("cp210"), Qt::CaseInsensitive)
		|| info.manufacturer().contains(QStringLiteral("arduino"), Qt::CaseInsensitive)
		|| (info.hasVendorIdentifier()
			&& (info.vendorIdentifier() == 0x2341
				|| info.vendorIdentifier() == 0x1A86
				|| info.vendorIdentifier() == 0x10C4
				|| info.vendorIdentifier() == 0x0403));
}

static QList<QString> orderedPortsToTry()
{
	QList<QString> preferredPorts;
	QList<QString> otherPorts;
	QSet<QString> seen;

	for (const QSerialPortInfo& info : QSerialPortInfo::availablePorts()) {
		const QString portName = info.portName();
		if (seen.contains(portName)) continue;
		seen.insert(portName);

		if (looksLikeArduino(info)) preferredPorts.append(portName);
		else otherPorts.append(portName);
	}

	return preferredPorts + otherPorts;
}

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

	const QList<QString> portsToTry = orderedPortsToTry();
	if (portsToTry.isEmpty()) {
		qWarning() << "Aucun port serie disponible.";
		return ARDUINO_NOT_AVAILABLE;
	}

	for (const QString& portName : portsToTry) {
		if (m_serial->isOpen()) {
			m_serial->close();
		}

		m_serial->setPortName(portName);
		m_serial->setBaudRate(QSerialPort::Baud9600);
		m_serial->setDataBits(QSerialPort::Data8);
		m_serial->setParity(QSerialPort::NoParity);
		m_serial->setStopBits(QSerialPort::OneStop);
		m_serial->setFlowControl(QSerialPort::NoFlowControl);

		if (m_serial->open(QIODevice::ReadWrite)) {
			m_portName = portName;
			qInfo() << "Connexion Arduino reussie sur" << portName;
			return ARDUINO_AVAILABLE;
		}

		qWarning() << "Echec ouverture port" << portName << ":" << m_serial->errorString();
	}

	qWarning() << "Impossible de connecter Arduino sur les ports detectes.";
	return ARDUINO_NOT_AVAILABLE;
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
	if (bytesWritten != data.size()) {
		return false;
	}

	if (!m_serial->waitForBytesWritten(200)) {
		return false;
	}

	return true;
}

QString arduino::getarduino_port_name() const
{
	return m_portName;
}

QSerialPort* arduino::serial() const
{
	return m_serial;
}
