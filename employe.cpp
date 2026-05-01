#include "employe.h"
#include "connection.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlDriver>
#include <QVariant>
#include <QRegularExpression>
#include <QDate>
#include <QtGlobal>
#include <utility>

static QString normalizeKey(QString text)
{
	// On utilise simplified() pour gérer les espaces multiples, tabs, newlines
	// et on passe en minuscule pour la comparaison.
	text = text.simplified().toLower();
	
	// Remplacement des caractères accentués courants
	text.replace(QStringLiteral("é"), QStringLiteral("e"));
	text.replace(QStringLiteral("è"), QStringLiteral("e"));
	text.replace(QStringLiteral("ê"), QStringLiteral("e"));
	text.replace(QStringLiteral("à"), QStringLiteral("a"));
	text.replace(QStringLiteral("â"), QStringLiteral("a"));
	text.replace(QStringLiteral("î"), QStringLiteral("i"));
	text.replace(QStringLiteral("ï"), QStringLiteral("i"));
	text.replace(QStringLiteral("ô"), QStringLiteral("o"));
	text.replace(QStringLiteral("û"), QStringLiteral("u"));
	
	// Suppression de tout ce qui n'est pas alphanumérique pour une comparaison brute
	QString result;
	for (const QChar &c : text) {
		if (c.isLetterOrNumber()) {
			result.append(c);
		}
	}
	return result;
}

static QString normalizeRfidKey(QString text)
{
	text = text.trimmed().toUpper();
	text.remove(QRegularExpression(QStringLiteral("[^0-9A-Z]+")));
	return text;
}

static QString sqlNormalizedRfidExpr(const QString &column)
{
	QString expr = QStringLiteral("UPPER(TRIM(%1))").arg(column);
	const QStringList charsToStrip = {
		QStringLiteral(" "),
		QStringLiteral("-"),
		QStringLiteral(":"),
		QStringLiteral("."),
		QStringLiteral("/"),
		QStringLiteral("\\")
	};

	for (const QString &ch : charsToStrip) {
		expr = QStringLiteral("REPLACE(%1, '%2', '')").arg(expr, ch);
	}

	return expr;
}

static QString resolveEmployeTableName(QSqlDatabase db)
{
	const QStringList allTables = db.tables(QSql::Tables) + db.tables(QSql::Views);
	const QStringList candidates = {
		QStringLiteral("EMPLOYES"),
		QStringLiteral("EMPLOYE"),
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
	
	// Détection par mots-clés pour gérer les troncatures ou variations
	if (key.contains(QStringLiteral("indispo")) || 
		key.contains(QStringLiteral("absent")) || 
		key.contains(QStringLiteral("nonpo"))) 
		return QStringLiteral("Indisponible");
		
	if (key.contains(QStringLiteral("mission")) || 
		key.contains(QStringLiteral("deplacement"))) 
		return QStringLiteral("En mission");
		
	if (key.contains(QStringLiteral("conge")) || 
		key.contains(QStringLiteral("vacance"))) 
		return QStringLiteral("Congé");
		
	if (key.contains(QStringLiteral("dispo")) || 
		key.contains(QStringLiteral("present")) || 
		key.contains(QStringLiteral("libre")) ||
		key.contains(QStringLiteral("actif"))) 
		return QStringLiteral("Disponible");
		
	// Valeur par défaut si rien n'est reconnu
	return raw.trimmed();
}

static QString toggledStatutForRfid(const QString &raw)
{
	// On normalise le statut actuel
	const QString current = canonicalStatut(raw);
	
	// Basculement strict : Disponible <-> Indisponible
	// Si l'employé est "Disponible", il devient "Indisponible"
	if (current == QStringLiteral("Disponible")) {
		return QStringLiteral("Indisponible");
	}
	
	// Dans tous les autres cas (Indisponible, En mission, Congé), on le remet en "Disponible"
	return QStringLiteral("Disponible");
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
	QString rfid;
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
	map.statut = matchColumn(cols, {QStringLiteral("STATUT"), QStringLiteral("STATUS"), QStringLiteral("DISPONIBILITE"), QStringLiteral("DISPO"), QStringLiteral("STATUT_EMPLOYE")});
	map.salaire = matchColumn(cols, {QStringLiteral("SALAIRE"), QStringLiteral("SALARY")});
	map.rfid = matchColumn(cols, {QStringLiteral("RFID_ID"), QStringLiteral("RFID"), QStringLiteral("CARD_ID")});

	if (map.id.isEmpty() || map.nom.isEmpty() || map.salaire.isEmpty()) {
		err = QStringLiteral("Colonnes obligatoires employé introuvables dans %1.").arg(map.table);
		return false;
	}

	// Si RFID_ID manque, on tente de l'ajouter automatiquement (Oracle syntax)
	if (map.rfid.isEmpty()) {
		QSqlQuery alter(db);
		if (alter.exec(QStringLiteral("ALTER TABLE %1 ADD RFID_ID VARCHAR2(50)").arg(map.table))) {
			map.rfid = QStringLiteral("RFID_ID");
		}
	}

	// Suppression de la colonne PRESENCE si elle existe
	QSqlQuery dropPresence(db);
	dropPresence.exec(QStringLiteral("ALTER TABLE %1 DROP COLUMN PRESENCE").arg(map.table));

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

static bool employeRfidExists(QSqlDatabase db,
						  const EmployeDbMap &map,
						  const QString &rfidId,
						  int excludeId,
						  bool &exists,
						  QString &err)
{
	exists = false;
	err.clear();

	if (map.rfid.trimmed().isEmpty()) {
		return true;
	}

	const QString rfidKey = normalizeRfidKey(rfidId);
	if (rfidKey.isEmpty()) {
		return true;
	}

	QSqlQuery query(db);
	const QString rfidExpr = sqlNormalizedRfidExpr(map.rfid);
	QString sql = QStringLiteral(
		"SELECT COUNT(1) FROM %1 "
		"WHERE %2 = :rfid")
				  .arg(map.table, rfidExpr);
	if (excludeId > 0) {
		sql += QStringLiteral(" AND %1 <> :excludeId").arg(map.id);
	}

	query.prepare(sql);
	query.bindValue(QStringLiteral(":rfid"), rfidKey);
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
static bool findEmployeByRfid(QSqlDatabase db,
						  const EmployeDbMap &map,
						  const QString &rfidId,
						  QVariant &employeIdOut,
						  QString &statutOut,
						  QString &nomOut,
						  QString &prenomOut,
						  QString &err,
						  QString *etatOut = nullptr)
{
	employeIdOut.clear();
	statutOut.clear();
	nomOut.clear();
	prenomOut.clear();
	err.clear();
	if (etatOut) {
		etatOut->clear();
	}

	if (map.id.isEmpty() || map.rfid.isEmpty() || map.statut.isEmpty()) {
		err = QStringLiteral("Configuration RFID/statut invalide.");
		return false;
	}

	const QString rfidKey = normalizeRfidKey(rfidId);
	if (rfidKey.isEmpty()) {
		err = QStringLiteral("UID RFID vide ou invalide.");
		return false;
	}

	QSqlQuery query(db);
	QStringList selectCols;
	selectCols << map.id << map.rfid << map.statut << map.nom;
	if (!map.prenom.isEmpty()) {
		selectCols << map.prenom;
	} else {
		selectCols << QStringLiteral("''"); // Fallback
	}
	if (!map.etat.isEmpty()) {
		selectCols << map.etat;
	} else {
		selectCols << QStringLiteral("''");
	}

	query.prepare(QStringLiteral("SELECT %1 FROM %2")
						.arg(selectCols.join(QStringLiteral(", ")), map.table));
	if (!query.exec()) {
		err = query.lastError().text();
		return false;
	}

	while (query.next()) {
		const QString dbRfid = normalizeRfidKey(query.value(1).toString());
		if (dbRfid.isEmpty()) {
			continue;
		}

		if (dbRfid == rfidKey) {
			employeIdOut = query.value(0);
			statutOut = query.value(2).toString().trimmed();
			nomOut = query.value(3).toString().trimmed();
			prenomOut = query.value(4).toString().trimmed();
			if (etatOut) {
				*etatOut = query.value(5).toString().trimmed();
			}
			return true;
		}
	}

	err = QStringLiteral("Aucun employé trouvé avec ce RFID: %1").arg(rfidId);
	return false;
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
				 const QString &telephoneUi,
				 const QString &rfidId)
	: m_id(id)
	, m_nom(nom)
	, m_prenom(prenom)
	, m_role(role)
	, m_equipe(equipe)
	, m_etat(etat)
	, m_statut(statut)
	, m_salaire(salaire)
	, m_telephoneUi(telephoneUi)
	, m_rfidId(rfidId)
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
	const QString rfidValue = normalizeRfidKey(m_rfidId);

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

	if (!employeRfidExists(db, map, rfidValue, -1, exists, existsErr)) {
		s_lastError = existsErr;
		return false;
	}
	if (exists) {
		s_lastError = QStringLiteral("UID RFID déjà attribué à un autre employé.");
		return false;
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
	if (!map.rfid.isEmpty()) { cols << map.rfid; bindValues << rfidValue; }

	QStringList placeholders;
	for (int i = 0; i < cols.size(); ++i) placeholders << QStringLiteral("?");

	QSqlQuery query(db);
	query.prepare(QStringLiteral("INSERT INTO %1 (%2) VALUES (%3)")
					  .arg(map.table,
						   cols.join(QLatin1Char(',')),
						   placeholders.join(QLatin1Char(','))));
	for (const QVariant &v : std::as_const(bindValues)) {
		query.addBindValue(v);
	}

	if (!query.exec()) {
		s_lastError = query.lastError().text();
		return false;
	}

	// Commit explicite pour Oracle
	QSqlQuery commitQuery(db);
	commitQuery.exec(QStringLiteral("COMMIT"));

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
	const QString rfidValue = normalizeRfidKey(m_rfidId);

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

	if (!employeRfidExists(db, map, rfidValue, ancienId, exists, existsErr)) {
		s_lastError = existsErr;
		return false;
	}
	if (exists) {
		s_lastError = QStringLiteral("UID RFID déjà attribué à un autre employé.");
		return false;
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
	if (!map.rfid.isEmpty()) { sets << QStringLiteral("%1 = ?").arg(map.rfid); bindValues << rfidValue; }

	QSqlQuery query(db);
	query.prepare(QStringLiteral("UPDATE %1 SET %2 WHERE %3 = ?")
					  .arg(map.table,
						   sets.join(QLatin1Char(',')),
						   map.id));
	for (const QVariant &v : std::as_const(bindValues)) {
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
	if (!map.rfid.isEmpty()) selectCols << map.rfid;
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
	if (!map.rfid.isEmpty()) model->setHeaderData(headerIdx++, Qt::Horizontal, QObject::tr("RFID"));
	model->setHeaderData(headerIdx, Qt::Horizontal, QObject::tr("Salaire"));

	s_lastError.clear();

	return model;
}

int Employe::genererNouvelId(const QString &role)
{
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

	// Schéma demandé (par rôle):
	// - Gardien:      2643NNN
	// - Technicien:   2644NNN
	// - Responsable:  2645NNN
	// - Ouvrier:      2546NNN
	// Repli (rôle inconnu): 264NNNN
	const QString canonRole = canonicalRole(role);
	int prefix = 264;
	int multiplier = 10000; // NNNN
	if (canonRole == QStringLiteral("Gardien")) {
		prefix = 2643;
		multiplier = 1000; // NNN
	} else if (canonRole == QStringLiteral("Technicien")) {
		prefix = 2644;
		multiplier = 1000;
	} else if (canonRole == QStringLiteral("Responsable")) {
		prefix = 2645;
		multiplier = 1000;
	} else if (canonRole == QStringLiteral("Ouvrier")) {
		prefix = 2546;
		multiplier = 1000;
	}

	const int minId = prefix * multiplier;
	const int maxId = minId + (multiplier - 1);

	QSqlQuery query(db);
	query.prepare(QStringLiteral(
		"SELECT NVL(MAX(TO_NUMBER(%1)), 0) FROM %2 "
		"WHERE %1 IS NOT NULL "
		"AND REGEXP_LIKE(TRIM(TO_CHAR(%1)), '^[0-9]+$') "
		"AND TO_NUMBER(%1) BETWEEN :minId AND :maxId")
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
		s_lastError = QStringLiteral("Limite atteinte pour ce préfixe ID (%1...).").arg(prefix);
		return 0;
	}

	s_lastError.clear();
	return nextId;
}

bool Employe::canAccessByRfid(const QString &rfidId, QString *nomEmployeOut, QString *reasonOut)
{
	if (nomEmployeOut) {
		nomEmployeOut->clear();
	}
	if (reasonOut) {
		reasonOut->clear();
	}

	Connection* conn = Connection::getInstance();
	if (!conn->ensureOpen()) {
		s_lastError = QStringLiteral("Connexion DB echouee: %1").arg(conn->lastErrorText());
		if (reasonOut) {
			*reasonOut = s_lastError;
		}
		return false;
	}

	QSqlDatabase db = conn->getDatabase();
	EmployeDbMap map;
	QString mapErr;
	if (!resolveEmployeDbMap(db, map, mapErr)) {
		s_lastError = mapErr;
		if (reasonOut) {
			*reasonOut = s_lastError;
		}
		return false;
	}

	QVariant employeId;
	QString statutActuel;
	QString nomEmp;
	QString prenomEmp;
	QString err;
	QString etatEmp;
	if (!findEmployeByRfid(db, map, rfidId, employeId, statutActuel, nomEmp, prenomEmp, err, &etatEmp)) {
		s_lastError = err;
		if (reasonOut) {
			*reasonOut = s_lastError;
		}
		return false;
	}

	Q_UNUSED(employeId);
	Q_UNUSED(statutActuel);

	if (nomEmployeOut) {
		*nomEmployeOut = QStringLiteral("%1 %2").arg(prenomEmp, nomEmp).trimmed();
	}

	if (canonicalEtat(etatEmp) == QStringLiteral("Banni")) {
		s_lastError = QStringLiteral("Acces refuse: employe banni.");
		if (reasonOut) {
			*reasonOut = s_lastError;
		}
		return false;
	}

	s_lastError.clear();
	return true;
}

bool Employe::updateStatutByRfid(const QString &rfidId, const QString &forcedStatut, QString *nouveauStatutOut, QString *nomEmployeOut)
{
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

	const QString rfidClean = normalizeRfidKey(rfidId);
	qDebug() << "--- DÉBUT RFID POINTAGE ---";
	qDebug() << "Badge scanné (nettoyé) :" << rfidClean;

	if (rfidClean.isEmpty()) {
		s_lastError = QStringLiteral("UID RFID vide ou invalide.");
		return false;
	}

	// 1. Rechercher l'employé pour avoir ses infos actuelles
	QVariant employeId;
	QString actuelRaw, nomEmp, prenomEmp, findErr;
	if (!findEmployeByRfid(db, map, rfidClean, employeId, actuelRaw, nomEmp, prenomEmp, findErr)) {
		qDebug() << "❌ Employé non trouvé pour RFID:" << rfidClean;
		s_lastError = QStringLiteral("Employé non trouvé pour RFID: %1").arg(rfidClean);
		return false;
	}

	qDebug() << "👤 Employé trouvé :" << prenomEmp << nomEmp << "(ID:" << employeId.toString() << ")";
	qDebug() << "📊 Statut actuel en base :" << actuelRaw;

	// Remplissage immédiat des infos de sortie pour l'UI (même si l'update échoue plus tard)
	if (nomEmployeOut) {
		*nomEmployeOut = QStringLiteral("%1 %2").arg(prenomEmp, nomEmp).trimmed();
	}

	// 2. Déterminer le nouveau statut
	QString nouveauStatut;
	if (!forcedStatut.isEmpty()) {
		nouveauStatut = forcedStatut;
	} else {
		nouveauStatut = toggledStatutForRfid(actuelRaw);
	}

	if (nouveauStatutOut) {
		*nouveauStatutOut = nouveauStatut;
	}
	
	qDebug() << "******************************************";
	qDebug() << "[RFID] MISE À JOUR DE STATUT DETECTE";
	qDebug() << "[RFID] Employe :" << prenomEmp << nomEmp;
	qDebug() << "[RFID] Ancien Statut :" << actuelRaw;
	qDebug() << "[RFID] NOUVEAU STATUT :" << nouveauStatut;
	qDebug() << "******************************************";

	// 3. Exécuter l'UPDATE par ID
	QSqlQuery query(db);
	// Utilisation de placeholders positionnels (?) pour une meilleure compatibilité ODBC/Oracle
	query.prepare(QStringLiteral("UPDATE %1 SET %2 = ? WHERE %3 = ?")
					  .arg(map.table, map.statut, map.id));
	query.addBindValue(nouveauStatut);
	query.addBindValue(employeId); // Utilisation directe du QVariant original

	if (!query.exec()) {
		qDebug() << "❌ Erreur SQL UPDATE (pos) :" << query.lastError().text();
		
		// Tentative de secours : augmenter la taille de la colonne statut au cas où (ex: "Indisponible" > 10 chars)
		QSqlQuery alter(db);
		alter.exec(QStringLiteral("ALTER TABLE %1 MODIFY %2 VARCHAR2(50)").arg(map.table, map.statut));
		
		// Ré-essai après l'éventuel ALTER
		query.prepare(QStringLiteral("UPDATE %1 SET %2 = ? WHERE %3 = ?")
						  .arg(map.table, map.statut, map.id));
		query.addBindValue(nouveauStatut);
		query.addBindValue(employeId);
		
		if (!query.exec()) {
			qDebug() << "❌ Erreur SQL UPDATE (retry) :" << query.lastError().text();
			s_lastError = QStringLiteral("Erreur SQL UPDATE : %1").arg(query.lastError().text());
			return false;
		}
	}

	int rows = query.numRowsAffected();
	qDebug() << "📝 Lignes affectées :" << rows;

	if (rows == 0) {
		qDebug() << "⚠️ Aucune ligne modifiée par ID. Tentative par RFID...";
		query.prepare(QStringLiteral("UPDATE %1 SET %2 = ? WHERE UPPER(REPLACE(%3, ' ', '')) = ?")
						  .arg(map.table, map.statut, map.rfid));
		query.addBindValue(nouveauStatut);
		query.addBindValue(rfidClean);
		if (!query.exec() || query.numRowsAffected() == 0) {
			qDebug() << "❌ Échec total de la mise à jour.";
			s_lastError = QStringLiteral("Échec de la mise à jour (aucune ligne affectée).");
			return false;
		}
		qDebug() << "✅ Mise à jour réussie par RFID.";
	}

	// 4. COMMIT EXPLICITE
	if (!QSqlQuery(db).exec(QStringLiteral("COMMIT"))) {
		qDebug() << "❌ Erreur COMMIT :" << db.lastError().text();
	}

	// 5. Fin
	qDebug() << "✅ POINTAGE TERMINÉ AVEC SUCCÈS";
	qDebug() << "---------------------------";
	
	s_lastError.clear();
	return true;
}
