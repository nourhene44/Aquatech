#include "client.h"
#include "connection.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QTime>

static QString g_clientLastError;

client::client() {}

static QString escapeString(const QString& str) {
	QString escaped = str;
	return escaped.replace("'", "''");
}

static QString formatId(const QString& id) {
	bool isNum;
	id.toLongLong(&isNum);
	return isNum ? id : "'" + escapeString(id) + "'";
}

static QString formatDate(const QDate& date) {
	return date.isValid()
	? QString("TO_DATE('%1','YYYY-MM-DD')").arg(date.toString("yyyy-MM-dd"))
	: "NULL";
}

static bool executeQueryClient(const QString& sql) {
	QSqlQuery query;
	if (query.exec(sql)) {
		g_clientLastError.clear();
		qDebug() << "Query succeeded:" << sql;
		return true;
	}

	QSqlError e = query.lastError();
	g_clientLastError = QString("%1 | driver: %2 | sql: %3").arg(e.text(), e.driverText(), sql);
	qWarning() << "Query failed:" << g_clientLastError;
	return false;
}

static bool checkConnectionClient() {
	Connection* conn = Connection::getInstance();
	if (!conn->createconnect()) {
		g_clientLastError = "DB connect failed";
		qWarning() << g_clientLastError;
		return false;
	}
	return true;
}

QString client::genererNouvelId()
{
	Connection* conn = Connection::getInstance();
	if (!conn->ensureOpen()) {
		g_clientLastError = QStringLiteral("Connexion DB échouée: %1").arg(conn->lastErrorText());
		return QString();
	}

	QSqlDatabase db = conn->getDatabase();
	if (!db.isValid() || !db.isOpen()) {
		g_clientLastError = QStringLiteral("Connexion DB indisponible.");
		return QString();
	}

	// Schéma demandé: 265NNNN
	const int prefix = 265;
	const int minId = prefix * 10000;
	const int maxId = minId + 9999;

	QSqlQuery query(db);
	query.prepare(QStringLiteral(
		"SELECT NVL(MAX(TO_NUMBER(ID_Client)), 0) FROM CLIENTS "
		"WHERE ID_Client IS NOT NULL "
		"AND REGEXP_LIKE(TRIM(TO_CHAR(ID_Client)), '^[0-9]+$') "
		"AND TO_NUMBER(ID_Client) BETWEEN :minId AND :maxId"));
	query.bindValue(QStringLiteral(":minId"), minId);
	query.bindValue(QStringLiteral(":maxId"), maxId);

	if (!query.exec()) {
		g_clientLastError = query.lastError().text();
		return QString();
	}

	int currentMax = 0;
	if (query.next()) {
		currentMax = query.value(0).toInt();
	}

	const int nextId = (currentMax > 0) ? (currentMax + 1) : (minId + 1);
	if (nextId > maxId) {
		g_clientLastError = QStringLiteral("Limite atteinte pour ce préfixe ID (265NNNN).");
		return QString();
	}

	g_clientLastError.clear();
	return QString::number(nextId);
}

bool client::addClient(const QString& id, const QString& nom, const QString& prenom,
					   const QString& profil, const QString& statut,
					   const QDate& date_inscription, const QString& telephone) {

	if (!checkConnectionClient()) return false;

	QString finalId = id.trimmed();
	if (finalId.isEmpty()) {
		finalId = genererNouvelId();
		if (finalId.isEmpty()) {
			return false;
		}
	}

	QStringList cols = {"ID_Client", "Nom_Client", "Prenom_Client", "Profil_Client",
						"Statut_Conformite", "Telephone"};
	QStringList vals = {formatId(finalId),
						"'" + escapeString(nom) + "'",
						"'" + escapeString(prenom) + "'",
						"'" + escapeString(profil) + "'",
						"'" + escapeString(statut) + "'",
						"'" + escapeString(telephone) + "'"};

	if (date_inscription.isValid()) {
		cols << "Date_Inscription";
		vals << formatDate(date_inscription);
	}

	QString sql = QString("INSERT INTO CLIENTS (%1) VALUES (%2)")
					  .arg(cols.join(", "), vals.join(", "));

	return executeQueryClient(sql);
}

static QString buildSetClauseClient(const QString& nom, const QString& prenom,
									const QString& profil, const QString& statut,
									const QDate& date_inscription, const QString& telephone) {
	QStringList set;
	set << QString("Nom_Client = '%1'").arg(escapeString(nom));
	set << QString("Prenom_Client = '%1'").arg(escapeString(prenom));
	set << QString("Profil_Client = '%1'").arg(escapeString(profil));
	set << QString("Statut_Conformite = '%1'").arg(escapeString(statut));
	set << QString("Telephone = '%1'").arg(escapeString(telephone));
	set << QString("Date_Inscription = %1").arg(formatDate(date_inscription));
	return set.join(", ");
}

bool client::updateClient(const QString& id, const QString& nom, const QString& prenom,
						  const QString& profil, const QString& statut,
						  const QDate& date_inscription, const QString& telephone) {

	if (id.isEmpty() || !checkConnectionClient()) return false;

	QString setClause = buildSetClauseClient(nom, prenom, profil, statut, date_inscription, telephone);

	QString sql = QString("UPDATE CLIENTS SET %1 WHERE ID_Client = %2")
					  .arg(setClause, formatId(id));

	return executeQueryClient(sql);
}

bool client::deleteClient(const QString& id) {
	if (id.isEmpty() || !checkConnectionClient()) return false;

	QString sql = QString("DELETE FROM CLIENTS WHERE ID_Client = %1").arg(formatId(id));

	return executeQueryClient(sql);
}

QString client::lastError() {
	return g_clientLastError;
}
