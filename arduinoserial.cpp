#include "arduinoserial.h"

#include <QtGlobal>

#ifdef HAVE_SERIALPORT
#include <QSerialPortInfo>
#endif

ArduinoSerial::ArduinoSerial(QObject* parent)
	: QObject(parent)
{
#ifdef HAVE_SERIALPORT
	m_port.setReadBufferSize(64 * 1024);

	connect(&m_port, &QSerialPort::readyRead, this, [this]() {
		handleIncomingBytes(m_port.readAll());
	});

	connect(&m_port,
			&QSerialPort::errorOccurred,
			this,
			[this](QSerialPort::SerialPortError error) {
				if (error == QSerialPort::NoError) return;

				const QString msg = m_port.errorString().isEmpty()
					? QStringLiteral("Erreur serie (code %1)").arg(static_cast<int>(error))
					: m_port.errorString();

				emit errorOccurred(msg);
			});
#endif
}

QString ArduinoSerial::connectedPortName() const
{
#ifdef HAVE_SERIALPORT
	return m_port.isOpen() ? m_port.portName() : QString();
#else
	return QString();
#endif
}

bool ArduinoSerial::serialPortAvailable() const
{
#ifdef HAVE_SERIALPORT
	return true;
#else
	return false;
#endif
}

QStringList ArduinoSerial::availablePortNames() const
{
	QStringList names;
#ifdef HAVE_SERIALPORT
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
#ifdef HAVE_SERIALPORT
	if (isConnected()) {
		if (m_port.portName() == portName) {
			return true;
		}
		disconnectFromPort();
	}

	m_port.setPortName(portName);
	m_port.setBaudRate(baudRate);
	m_port.setDataBits(QSerialPort::Data8);
	m_port.setParity(QSerialPort::NoParity);
	m_port.setStopBits(QSerialPort::OneStop);
	m_port.setFlowControl(QSerialPort::NoFlowControl);

	if (!m_port.open(QIODevice::ReadWrite)) {
		const QString msg = m_port.errorString();
		if (errorOut) *errorOut = msg;
		emit errorOccurred(msg);
		return false;
	}

	emit connected(portName);
	return true;
#else
	Q_UNUSED(portName);
	Q_UNUSED(baudRate);
	const QString msg = QStringLiteral("Qt SerialPort indisponible (module non installe).");
	if (errorOut) *errorOut = msg;
	emit errorOccurred(msg);
	return false;
#endif
}

void ArduinoSerial::disconnectFromPort()
{
#ifdef HAVE_SERIALPORT
	if (!m_port.isOpen()) return;

	m_port.close();
	m_rxBuffer.clear();
	emit disconnected();
#endif
}

bool ArduinoSerial::isConnected() const
{
#ifdef HAVE_SERIALPORT
	return m_port.isOpen();
#else
	return false;
#endif
}

bool ArduinoSerial::writeLine(const QString& line, QString* errorOut)
{
#ifdef HAVE_SERIALPORT
	if (!m_port.isOpen()) {
		const QString msg = QStringLiteral("Port serie non connecte.");
		if (errorOut) *errorOut = msg;
		emit errorOccurred(msg);
		return false;
	}

	QByteArray data = line.toUtf8();
	data.append('\n');

	const qint64 written = m_port.write(data);
	if (written < 0) {
		const QString msg = m_port.errorString();
		if (errorOut) *errorOut = msg;
		emit errorOccurred(msg);
		return false;
	}

	if (!m_port.waitForBytesWritten(500)) {
		const QString msg = m_port.errorString().isEmpty()
			? QStringLiteral("Timeout ecriture serie.")
			: m_port.errorString();

		if (errorOut) *errorOut = msg;
		emit errorOccurred(msg);
		return false;
	}

	return true;
#else
	Q_UNUSED(line);
	const QString msg = QStringLiteral("Qt SerialPort indisponible (module non installe).");
	if (errorOut) *errorOut = msg;
	emit errorOccurred(msg);
	return false;
#endif
}

void ArduinoSerial::handleIncomingBytes(const QByteArray& bytes)
{
	if (bytes.isEmpty()) return;

	m_rxBuffer += bytes;

	while (true) {
		const int newlineIndex = m_rxBuffer.indexOf('\n');
		if (newlineIndex < 0) break;

		QByteArray lineBytes = m_rxBuffer.left(newlineIndex);
		m_rxBuffer.remove(0, newlineIndex + 1);

		if (!lineBytes.isEmpty() && lineBytes.endsWith('\r')) {
			lineBytes.chop(1);
		}

		const QString line = QString::fromUtf8(lineBytes).trimmed();
		if (!line.isEmpty()) {
			emit lineReceived(line);
		}
	}
}
