#include "employe.h"
#include "connection.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>
#include <QRegularExpression>
#include <QDate>

static QString normalizeKey(QString text)
{
	text = text.trimmed().toLower();
	text.replace(QStringLiteral("é"), QStringLiteral("e"));
	text.replace(QStringLiteral("è"), QStringLiteral("e"));
	text.replace(QStringLiteral("ê"), QStringLiteral("e"));
	text.replace(QStringLiteral("à"), QStringLiteral("a"));
	text.replace(QStringLiteral("ù"), QStringLiteral("u"));
	text.replace(QStringLiteral("î"), QStringLiteral("i"));
	text.replace(QStringLiteral("ô"), QStringLiteral("o"));
	text.replace(QStringLiteral("ç"), QStringLiteral("c"));
	text.remove(QLatin1Char('_'));
	text.remove(QLatin1Char(' '));
	return text;
}

static QString resolveEmployeTableName(QSqlDatabase db)
{
	const QStringList allTables = db.tables(QSql::Tables) + db.tables(QSql::Views);
	const QStringList candidates = {
		QStringLiteral("EMPLOYE"),
		QStringLiteral("EMPLOYES"),
		QStringLiteral("EMPLOYEE"),
		QStringLiteral("EMPLOYEES"),
		QStringLiteral("T_EMPLOYE")
	};

	for (const QString &cand : candidates) {
		const QString ckey = normalizeKey(cand);
		for (const QString &tbl : allTables) {
			if (normalizeKey(tbl) == ckey) {
				return tbl;
			}
		}
	}

	for (const QString &cand : candidates) {
		const QString ckey = normalizeKey(cand);
		for (const QString &tbl : allTables) {
			if (normalizeKey(tbl).startsWith(ckey)) {
				return tbl;
			}
		}
	}

	return QString();
}

static QStringList getColumns(QSqlDatabase db, const QString &tableName)
{
	QStringList cols;
	const QSqlRecord rec = db.record(tableName);
	for (int i = 0; i < rec.count(); ++i) {
		cols << rec.fieldName(i);
	}
	return cols;
}

static QString matchColumn(const QStringList &dbCols, const QStringList &synonyms)
{
	for (const QString &syn : synonyms) {
		const QString skey = normalizeKey(syn);
		for (const QString &col : dbCols) {
			if (normalizeKey(col) == skey) {
				return col;
			}
		}
	}

	for (const QString &syn : synonyms) {
		const QString skey = normalizeKey(syn);
		for (const QString &col : dbCols) {
			if (normalizeKey(col).startsWith(skey)) {
				return col;
			}
		}
	}

	return QString();
}

static QString canonicalRole(const QString &raw)
{
	const QString key = normalizeKey(raw);
	if (key.contains(QStringLiteral("gard"))) return QStringLiteral("Gardien");
	if (key.contains(QStringLiteral("tech"))) return QStringLiteral("Technicien");
	if (key.contains(QStringLiteral("respons"))) return QStringLiteral("Responsable");
	if (key.contains(QStringLiteral("ouvri"))) return QStringLiteral("Ouvrier");
	if (key.contains(QStringLiteral("pech"))) return QStringLiteral("Pecheur");
	return raw.trimmed();
}

static QString canonicalEtat(const QString &raw)
{
	const QString key = normalizeKey(raw);
	if (key.contains(QStringLiteral("actif"))) return QStringLiteral("Actif");
	if (key.contains(QStringLiteral("banni"))) return QStringLiteral("Banni");
	return raw.trimmed();
}

static QString canonicalStatut(const QString &raw)
{
	const QString key = normalizeKey(raw);
	if (key.contains(QStringLiteral("mission"))) return QStringLiteral("En mission");
	if (key.contains(QStringLiteral("conge"))) return QStringLiteral("Conge");
	if (key.contains(QStringLiteral("dispon"))) return QStringLiteral("Disponible");
	return raw.trimmed();
}

static QString canonicalEquipe(const QString &raw)
{
	const QString t = raw.trimmed();
	for (QChar ch : t) {
		if (!ch.isLetter()) continue;
		const QChar up = ch.toUpper();
		if (up == QLatin1Char('A') || up == QLatin1Char('B') || up == QLatin1Char('C')) {
			return QString(up);
		}
		break;
	}

	const QString key = normalizeKey(t);
	if (key.startsWith(QStringLiteral("a"))) return QStringLiteral("A");
	if (key.startsWith(QStringLiteral("b"))) return QStringLiteral("B");
	if (key.startsWith(QStringLiteral("c"))) return QStringLiteral("C");
	return QString();
}

static int employeRoleCodeForId(const QString& roleRaw)
{
	const QString role = canonicalRole(roleRaw);
	if (role == QStringLiteral("Gardien")) return 3;
	if (role == QStringLiteral("Technicien")) return 4;
	if (role == QStringLiteral("Responsable")) return 5;
	if (role == QStringLiteral("Ouvrier")) return 6;
	return 0;
}

static bool isValidPersonName(const QString& text)
{
	static const QRegularExpression pattern(QStringLiteral("^[A-Za-zÀ-ÿ\\s'-]+$"));
	return pattern.match(text.trimmed()).hasMatch();
}

static bool isValidTelephone(const QString& text)
{
	static const QRegularExpression pattern(QStringLiteral("^\\d{8,15}$"));
	return pattern.match(text.trimmed()).hasMatch();
}

struct EmployeDbMap
{
	QString table;
	QString id;
	QString nom;
	QString prenom;
	QString telephone;
	QString role;
	QString equipe;
	QString etat;
	QString statut;
	QString salaire;
};

static bool resolveEmployeDbMap(QSqlDatabase db, EmployeDbMap &map, QString &err)
{
	map = EmployeDbMap{};
	map.table = resolveEmployeTableName(db);
	if (map.table.isEmpty()) {
		err = QStringLiteral("Table employés introuvable (EMPLOYE/EMPLOYES/EMPLOYEE...).");
		return false;
	}

	const QStringList cols = getColumns(db, map.table);
	if (cols.isEmpty()) {
		err = QStringLiteral("Impossible de lire les colonnes de la table %1.").arg(map.table);
		return false;
	}

	map.id = matchColumn(cols, {QStringLiteral("ID_EMPLOYE"), QStringLiteral("ID"), QStringLiteral("IDEMPLOYE"), QStringLiteral("EMPLOYEEID")});
	map.nom = matchColumn(cols, {QStringLiteral("NOM_EMPLOYE"), QStringLiteral("NOM"), QStringLiteral("NAME"), QStringLiteral("LASTNAME")});
	map.prenom = matchColumn(cols, {QStringLiteral("PRENOM_EMPLOYE"), QStringLiteral("PRENOM"), QStringLiteral("FIRSTNAME")});
	map.telephone = matchColumn(cols, {QStringLiteral("TELEPHONE"), QStringLiteral("TEL"), QStringLiteral("PHONE"), QStringLiteral("NUMERO")});
	map.role = matchColumn(cols, {QStringLiteral("ROLE"), QStringLiteral("POSTE"), QStringLiteral("FONCTION")});
	map.equipe = matchColumn(cols, {QStringLiteral("EQUIPE"), QStringLiteral("TEAM")});
	map.etat = matchColumn(cols, {QStringLiteral("ETAT"), QStringLiteral("STATE"), QStringLiteral("ETAT_EMPLOYE")});
	map.statut = matchColumn(cols, {QStringLiteral("STATUT"), QStringLiteral("STATUS"), QStringLiteral("ZONE")});
	map.salaire = matchColumn(cols, {QStringLiteral("SALAIRE"), QStringLiteral("SALARY")});

	if (map.id.isEmpty() || map.nom.isEmpty() || map.salaire.isEmpty()) {
		err = QStringLiteral("Colonnes obligatoires employé introuvables dans %1.").arg(map.table);
		return false;
	}

	return true;
}

static bool employeValueExists(QSqlDatabase db,
						   const EmployeDbMap &map,
						   const QString &column,
						   const QVariant &value,
						   int excludeId,
						   bool &exists,
						   QString &err)
{
	exists = false;
	err.clear();

	if (column.trimmed().isEmpty()) {
		return true;
	}

	QSqlQuery query(db);
	QString sql = QStringLiteral("SELECT COUNT(1) FROM %1 WHERE %2 = :value")
					 .arg(map.table, column);
	if (excludeId > 0) {
		sql += QStringLiteral(" AND %1 <> :excludeId").arg(map.id);
	}

	query.prepare(sql);
	query.bindValue(QStringLiteral(":value"), value);
	if (excludeId > 0) {
		query.bindValue(QStringLiteral(":excludeId"), excludeId);
	}

	if (!query.exec()) {
		err = query.lastError().text();
		return false;
	}

	if (query.next()) {
		exists = query.value(0).toInt() > 0;
	}

	return true;
}

QString Employe::s_lastError;

Employe::Employe() = default;

Employe::Employe(int id,
				 const QString &nom,
				 const QString &prenom,
				 const QString &role,
				 const QString &equipe,
				 const QString &etat,
				 const QString &statut,
				 double salaire,
				 const QString &telephoneUi)
	: m_id(id)
	, m_nom(nom)
	, m_prenom(prenom)
	, m_role(role)
	, m_equipe(equipe)
	, m_etat(etat)
	, m_statut(statut)
	, m_salaire(salaire)
	, m_telephoneUi(telephoneUi)
{
}

QString Employe::lastError()
{
	return s_lastError;
}

bool Employe::ajouter() const
{
	if (m_id <= 0) {
		s_lastError = QStringLiteral("ID invalide (doit être > 0).");
		return false;
	}
	if (m_nom.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("Nom obligatoire.");
		return false;
	}
	if (m_prenom.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("Prénom obligatoire.");
		return false;
	}
	if (!isValidPersonName(m_nom)) {
		s_lastError = QStringLiteral("Nom invalide (lettres uniquement).");
		return false;
	}
	if (!isValidPersonName(m_prenom)) {
		s_lastError = QStringLiteral("Prénom invalide (lettres uniquement).");
		return false;
	}
	if (!isValidTelephone(m_telephoneUi)) {
		s_lastError = QStringLiteral("Téléphone invalide (8 à 15 chiffres).");
		return false;
	}
	if (m_salaire < 0.0) {
		s_lastError = QStringLiteral("Salaire invalide (doit être >= 0).");
		return false;
	}

	const QString roleValue = canonicalRole(m_role);
	const QString equipeValue = canonicalEquipe(m_equipe);
	const QString etatValue = canonicalEtat(m_etat);
	const QString statutValue = canonicalStatut(m_statut);

	if (equipeValue.isEmpty()) {
		s_lastError = QStringLiteral("Équipe invalide (valeurs autorisées: A, B, C).");
		return false;
	}

	Connection* conn = Connection::getInstance();
	if (!conn->ensureOpen()) {
		s_lastError = QStringLiteral("Connexion DB échouée: %1").arg(conn->lastErrorText());
		return false;
	}

	QSqlDatabase db = conn->getDatabase();
	EmployeDbMap map;
	QString mapErr;
	if (!resolveEmployeDbMap(db, map, mapErr)) {
		s_lastError = mapErr;
		return false;
	}

	bool exists = false;
	QString existsErr;
	if (!employeValueExists(db, map, map.id, m_id, -1, exists, existsErr)) {
		s_lastError = existsErr;
		return false;
	}
	if (exists) {
		s_lastError = QStringLiteral("ID déjà existant.");
		return false;
	}

	if (!map.telephone.isEmpty()) {
		if (!employeValueExists(db, map, map.telephone, m_telephoneUi.trimmed(), -1, exists, existsErr)) {
			s_lastError = existsErr;
			return false;
		}
		if (exists) {
			s_lastError = QStringLiteral("Numéro de téléphone déjà existant.");
			return false;
		}
	}

	QStringList cols;
	QList<QVariant> bindValues;

	cols << map.id << map.nom << map.salaire;
	bindValues << m_id << m_nom.trimmed() << m_salaire;

	if (!map.prenom.isEmpty()) { cols << map.prenom; bindValues << m_prenom.trimmed(); }
	if (!map.telephone.isEmpty()) { cols << map.telephone; bindValues << m_telephoneUi.trimmed(); }
	if (!map.role.isEmpty()) { cols << map.role; bindValues << roleValue; }
	if (!map.statut.isEmpty()) { cols << map.statut; bindValues << statutValue; }
	if (!map.equipe.isEmpty()) { cols << map.equipe; bindValues << equipeValue; }
	if (!map.etat.isEmpty()) { cols << map.etat; bindValues << etatValue; }

	QStringList placeholders;
	for (int i = 0; i < cols.size(); ++i) placeholders << QStringLiteral("?");

	QSqlQuery query(db);
	query.prepare(QStringLiteral("INSERT INTO %1 (%2) VALUES (%3)")
					  .arg(map.table,
						   cols.join(QLatin1Char(',')),
						   placeholders.join(QLatin1Char(','))));
	for (const QVariant &v : bindValues) {
		query.addBindValue(v);
	}

	if (!query.exec()) {
		s_lastError = query.lastError().text();
		return false;
	}

	s_lastError.clear();
	return true;
}

bool Employe::modifier() const
{
	return modifierAvecAncienId(m_id);
}

bool Employe::modifierAvecAncienId(int ancienId) const
{
	if (m_id <= 0) {
		s_lastError = QStringLiteral("ID invalide (doit être > 0).");
		return false;
	}
	if (ancienId <= 0) {
		s_lastError = QStringLiteral("ID original invalide (doit être > 0).");
		return false;
	}
	if (m_nom.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("Nom obligatoire.");
		return false;
	}
	if (m_prenom.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("Prénom obligatoire.");
		return false;
	}
	if (!isValidPersonName(m_nom)) {
		s_lastError = QStringLiteral("Nom invalide (lettres uniquement).");
		return false;
	}
	if (!isValidPersonName(m_prenom)) {
		s_lastError = QStringLiteral("Prénom invalide (lettres uniquement).");
		return false;
	}
	if (!isValidTelephone(m_telephoneUi)) {
		s_lastError = QStringLiteral("Téléphone invalide (8 à 15 chiffres).");
		return false;
	}
	if (m_salaire < 0.0) {
		s_lastError = QStringLiteral("Salaire invalide (doit être >= 0).");
		return false;
	}

	const QString roleValue = canonicalRole(m_role);
	const QString equipeValue = canonicalEquipe(m_equipe);
	const QString etatValue = canonicalEtat(m_etat);
	const QString statutValue = canonicalStatut(m_statut);

	if (equipeValue.isEmpty()) {
		s_lastError = QStringLiteral("Équipe invalide (valeurs autorisées: A, B, C).");
		return false;
	}

	Connection* conn = Connection::getInstance();
	if (!conn->ensureOpen()) {
		s_lastError = QStringLiteral("Connexion DB échouée: %1").arg(conn->lastErrorText());
		return false;
	}

	QSqlDatabase db = conn->getDatabase();
	EmployeDbMap map;
	QString mapErr;
	if (!resolveEmployeDbMap(db, map, mapErr)) {
		s_lastError = mapErr;
		return false;
	}

	bool exists = false;
	QString existsErr;
	if (!employeValueExists(db, map, map.id, m_id, ancienId, exists, existsErr)) {
		s_lastError = existsErr;
		return false;
	}
	if (exists) {
		s_lastError = QStringLiteral("ID déjà existant.");
		return false;
	}

	if (!map.telephone.isEmpty()) {
		if (!employeValueExists(db, map, map.telephone, m_telephoneUi.trimmed(), ancienId, exists, existsErr)) {
			s_lastError = existsErr;
			return false;
		}
		if (exists) {
			s_lastError = QStringLiteral("Numéro de téléphone déjà existant.");
			return false;
		}
	}

	QStringList sets;
	QList<QVariant> bindValues;

	sets << QStringLiteral("%1 = ?").arg(map.id);
	bindValues << m_id;
	sets << QStringLiteral("%1 = ?").arg(map.nom);
	bindValues << m_nom.trimmed();
	sets << QStringLiteral("%1 = ?").arg(map.salaire);
	bindValues << m_salaire;

	if (!map.prenom.isEmpty()) { sets << QStringLiteral("%1 = ?").arg(map.prenom); bindValues << m_prenom.trimmed(); }
	if (!map.telephone.isEmpty()) { sets << QStringLiteral("%1 = ?").arg(map.telephone); bindValues << m_telephoneUi.trimmed(); }
	if (!map.role.isEmpty()) { sets << QStringLiteral("%1 = ?").arg(map.role); bindValues << roleValue; }
	if (!map.statut.isEmpty()) { sets << QStringLiteral("%1 = ?").arg(map.statut); bindValues << statutValue; }
	if (!map.equipe.isEmpty()) { sets << QStringLiteral("%1 = ?").arg(map.equipe); bindValues << equipeValue; }
	if (!map.etat.isEmpty()) { sets << QStringLiteral("%1 = ?").arg(map.etat); bindValues << etatValue; }

	QSqlQuery query(db);
	query.prepare(QStringLiteral("UPDATE %1 SET %2 WHERE %3 = ?")
					  .arg(map.table,
						   sets.join(QLatin1Char(',')),
						   map.id));
	for (const QVariant &v : bindValues) {
		query.addBindValue(v);
	}
	query.addBindValue(ancienId);

	if (!query.exec()) {
		s_lastError = query.lastError().text();
		return false;
	}

	if (query.numRowsAffected() <= 0) {
		s_lastError = QStringLiteral("Aucune ligne modifiée. Vérifier l'ID de l'employé.");
		return false;
	}

	s_lastError.clear();
	return true;
}

bool Employe::supprimer(int id)
{
	if (id <= 0) {
		s_lastError = QStringLiteral("ID invalide (doit être > 0).");
		return false;
	}

	Connection* conn = Connection::getInstance();
	if (!conn->ensureOpen()) {
		s_lastError = QStringLiteral("Connexion DB échouée: %1").arg(conn->lastErrorText());
		return false;
	}

	QSqlDatabase db = conn->getDatabase();
	EmployeDbMap map;
	QString mapErr;
	if (!resolveEmployeDbMap(db, map, mapErr)) {
		s_lastError = mapErr;
		return false;
	}

	QSqlQuery query(db);
	query.prepare(QStringLiteral("DELETE FROM %1 WHERE %2 = ?").arg(map.table, map.id));
	query.addBindValue(id);

	if (!query.exec()) {
		s_lastError = query.lastError().text();
		return false;
	}

	s_lastError.clear();
	return true;
}

QSqlQueryModel *Employe::afficher()
{
	auto *model = new QSqlQueryModel();

	Connection* conn = Connection::getInstance();
	if (!conn->ensureOpen()) {
		s_lastError = QStringLiteral("Connexion DB échouée: %1").arg(conn->lastErrorText());
		return model;
	}

	QSqlDatabase db = conn->getDatabase();
	EmployeDbMap map;
	QString mapErr;
	if (!resolveEmployeDbMap(db, map, mapErr)) {
		s_lastError = mapErr;
		return model;
	}

	QStringList selectCols;
	selectCols << map.id << map.nom;
	if (!map.prenom.isEmpty()) selectCols << map.prenom;
	if (!map.telephone.isEmpty()) selectCols << map.telephone;
	if (!map.role.isEmpty()) selectCols << map.role;
	if (!map.statut.isEmpty()) selectCols << map.statut;
	if (!map.equipe.isEmpty()) selectCols << map.equipe;
	if (!map.etat.isEmpty()) selectCols << map.etat;
	selectCols << map.salaire;

	const QString sql = QStringLiteral("SELECT %1 FROM %2 ORDER BY %3")
							.arg(selectCols.join(QLatin1Char(',')), map.table, map.id);
	model->setQuery(sql, db);

	if (model->lastError().isValid()) {
		s_lastError = model->lastError().text();
		return model;
	}

	model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
	model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
	int headerIdx = 2;
	if (!map.prenom.isEmpty()) model->setHeaderData(headerIdx++, Qt::Horizontal, QObject::tr("Prénom"));
	if (!map.telephone.isEmpty()) model->setHeaderData(headerIdx++, Qt::Horizontal, QObject::tr("Téléphone"));
	if (!map.role.isEmpty()) model->setHeaderData(headerIdx++, Qt::Horizontal, QObject::tr("Rôle"));
	if (!map.statut.isEmpty()) model->setHeaderData(headerIdx++, Qt::Horizontal, QObject::tr("Statut"));
	if (!map.equipe.isEmpty()) model->setHeaderData(headerIdx++, Qt::Horizontal, QObject::tr("Équipe"));
	if (!map.etat.isEmpty()) model->setHeaderData(headerIdx++, Qt::Horizontal, QObject::tr("État"));
	model->setHeaderData(headerIdx, Qt::Horizontal, QObject::tr("Salaire"));

	s_lastError.clear();

	return model;
}

int Employe::genererNouvelId(const QString &role)
{
	const int roleCode = employeRoleCodeForId(role);
	if (roleCode == 0) {
		s_lastError = QStringLiteral("Rôle invalide pour génération ID (Gardien=3, Technicien=4, Responsable=5, Ouvrier=6).");
		return 0;
	}

	Connection* conn = Connection::getInstance();
	if (!conn->ensureOpen()) {
		s_lastError = QStringLiteral("Connexion DB échouée: %1").arg(conn->lastErrorText());
		return 0;
	}

	QSqlDatabase db = conn->getDatabase();
	EmployeDbMap map;
	QString mapErr;
	if (!resolveEmployeDbMap(db, map, mapErr)) {
		s_lastError = mapErr;
		return 0;
	}

	const int yy = QDate::currentDate().year() % 100;
	const int prefix = (yy * 10) + roleCode;
	const int minId = prefix * 10000;
	const int maxId = minId + 9999;

	QSqlQuery query(db);
	query.prepare(QStringLiteral("SELECT NVL(MAX(%1), 0) FROM %2 WHERE %1 BETWEEN :minId AND :maxId")
				  .arg(map.id, map.table));
	query.bindValue(QStringLiteral(":minId"), minId);
	query.bindValue(QStringLiteral(":maxId"), maxId);

	if (!query.exec()) {
		s_lastError = query.lastError().text();
		return 0;
	}

	int currentMax = 0;
	if (query.next()) {
		currentMax = query.value(0).toInt();
	}

	const int nextId = (currentMax > 0) ? (currentMax + 1) : (minId + 1);
	if (nextId > maxId) {
		s_lastError = QStringLiteral("Limite atteinte pour ce préfixe ID (YYRNNNN). Changer d'année ou rôle.");
		return 0;
	}

	s_lastError.clear();
	return nextId;
}

