#include "pecheurs.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QMetaType>
#include <QRegularExpression>
#include <QDebug>
#include <QStringList>
#include <QTime>

static QString s_lastError;

static QVariant nullInt()
{
	return QVariant(QMetaType::fromType<int>());
}

static QVariant nullDate()
{
	return QVariant(QMetaType::fromType<QDate>());
}

static QString normalizeText(QString value)
{
	value = value.trimmed();
	value.replace(QStringLiteral("é"), QStringLiteral("e"));
	value.replace(QStringLiteral("è"), QStringLiteral("e"));
	value.replace(QStringLiteral("ê"), QStringLiteral("e"));
	value.replace(QStringLiteral("à"), QStringLiteral("a"));
	value.replace(QStringLiteral("î"), QStringLiteral("i"));
	value.replace(QStringLiteral("ô"), QStringLiteral("o"));
	value.replace(QStringLiteral("û"), QStringLiteral("u"));
	value.replace(QStringLiteral("ç"), QStringLiteral("c"));
	return value.toLower();
}

static QString canonicalRole(const QString& raw)
{
	const QString n = normalizeText(raw);
	if (n.contains(QStringLiteral("contre"))) return QStringLiteral("Contremaitre");
	if (n.contains(QStringLiteral("maitre"))) return QStringLiteral("Maitre de peche");
	if (n.contains(QStringLiteral("apprenti"))) return QStringLiteral("Apprenti pecheur");
	if (n.contains(QStringLiteral("marin"))) return QStringLiteral("Marin-pecheur");
	return QString();
}

static QString canonicalDisponibilite(const QString& raw)
{
	const QString n = normalizeText(raw);
	if (n.contains(QStringLiteral("indisponible"))) return QStringLiteral("Indisponible");
	if (n.contains(QStringLiteral("conge"))) return QStringLiteral("En conge");
	if (n.contains(QStringLiteral("bientot"))) return QStringLiteral("Disponible bientot");
	if (n.contains(QStringLiteral("disponible"))) return QStringLiteral("Disponible");
	return QString();
}

static QString canonicalSexe(const QString& raw)
{
	const QString n = normalizeText(raw);
	if (n.startsWith(QStringLiteral("m")) || n.contains(QStringLiteral("hom"))) return QStringLiteral("M");
	if (n.startsWith(QStringLiteral("f")) || n.contains(QStringLiteral("fem"))) return QStringLiteral("F");
	return QString();
}

static bool isValidEmail(const QString& email)
{
	static const QRegularExpression pattern(QStringLiteral("^[^\\s@]+@[^\\s@]+\\.[^\\s@]+$"));
	return pattern.match(email.trimmed()).hasMatch();
}

static bool isDigitsOnly(const QString& text)
{
	static const QRegularExpression digitsPattern(QStringLiteral("^\\d+$"));
	return digitsPattern.match(text.trimmed()).hasMatch();
}

static bool idPecheurColumnSupportsText()
{
	QSqlQuery query;
	query.prepare(
		"SELECT DATA_TYPE FROM USER_TAB_COLUMNS "
		"WHERE UPPER(TABLE_NAME) = 'PECHEURS' AND UPPER(COLUMN_NAME) = 'ID_PECHEUR'");

	if (!query.exec()) {
		return true;
	}

	if (!query.next()) {
		return true;
	}

	const QString type = query.value(0).toString().trimmed().toUpper();
	return type.contains(QStringLiteral("CHAR")) || type.contains(QStringLiteral("CLOB"));
}

static bool validateIdColumnCompatibility(const QString& idValue)
{
	if (idValue.trimmed().isEmpty()) {
		return true;
	}

	if (idPecheurColumnSupportsText()) {
		return true;
	}

	if (isDigitsOnly(idValue)) {
		return true;
	}

	s_lastError = QStringLiteral("La colonne ID_Pecheur est num\u00E9rique dans Oracle. "
		"Le format doit contenir uniquement des chiffres (ex: 2610001).");
	return false;
}

static QString normalizePecheurProfessionalId(QString id)
{
	id = id.trimmed().toUpper();
	// Convert legacy format YY1NNNN / YY2NNNN into YYHNNNN / YYFNNNN.
	static const QRegularExpression legacyPattern(QStringLiteral("^(\\d{2})([12])(\\d{4})$"));
	const auto m = legacyPattern.match(id);
	if (m.hasMatch()) {
		const QString yy = m.captured(1);
		const QString legacySex = m.captured(2);
		const QString seq = m.captured(3);
		const QString sexLetter = (legacySex == QStringLiteral("1")) ? QStringLiteral("H") : QStringLiteral("F");
		return yy + sexLetter + seq;
	}
	return id;
}

static bool isValidPecheurProfessionalId(const QString& id)
{
	// Accept both new and legacy formats.
	static const QRegularExpression anyPattern(QStringLiteral("^\\d{2}([HF]|[12])\\d{4}$"));
	return anyPattern.match(id.trimmed().toUpper()).hasMatch();
}

static bool validatePecheursTemporalConstraints(const QDate& dateInscription,
										const QDate& dateAffectation,
										int heures,
										bool requireCurrentInscription)
{
	if (!dateInscription.isValid()) {
		s_lastError = QStringLiteral("Date d'inscription obligatoire.");
		return false;
	}

	const QDate today = QDate::currentDate();
	if (requireCurrentInscription) {
		if (dateInscription != today) {
			s_lastError = QStringLiteral("Date d'inscription invalide (doit être aujourd'hui).");
			return false;
		}

		const int currentHour = QTime::currentTime().hour();
		if (heures != currentHour) {
			s_lastError = QStringLiteral("Heure d'inscription invalide (doit être l'heure actuelle).");
			return false;
		}
	}

	if (!dateAffectation.isValid()) {
		s_lastError = QStringLiteral("Date d'affectation obligatoire.");
		return false;
	}

	if (dateAffectation < today) {
		s_lastError = QStringLiteral("Date d'affectation invalide (doit être aujourd'hui ou une date future).");
		return false;
	}

	if (dateAffectation < dateInscription) {
		s_lastError = QStringLiteral("Date d'affectation invalide (doit être >= date d'inscription).");
		return false;
	}

	return true;
}

static int toIntOrZeroLocal(const QString& text)
{
	if (text.trimmed().isEmpty()) return 0;

	bool ok = false;
	const int value = text.trimmed().toInt(&ok);
	if (ok) return value;

	const double dvalue = text.trimmed().toDouble(&ok);
	if (ok) return static_cast<int>(dvalue);

	return 0;
}

static bool isAllRolesSelection(const QString& text)
{
	const QString t = normalizeText(text);
	return t.contains(QStringLiteral("tous"))
		&& (t.contains(QStringLiteral("role")) || t.contains(QStringLiteral("rôle")));
}

static bool isAllDisponibiliteSelection(const QString& text)
{
	const QString t = normalizeText(text);
	return t.contains(QStringLiteral("toutes")) && t.contains(QStringLiteral("dispon"));
}

static void appendPecheursFilters(QStringList& whereParts,
							  const QString& recherche,
							  const QString& roleSelection,
							  const QString& dispoSelection)
{
	if (!recherche.isEmpty()) {
		const QStringList termes = recherche.split(' ', Qt::SkipEmptyParts);
		if (termes.size() >= 2) {
			whereParts << "((LOWER(Nom_Pecheur) LIKE LOWER(:nom) AND LOWER(Prenom_Pecheur) LIKE LOWER(:prenom)) "
						 "OR (LOWER(Nom_Pecheur) LIKE LOWER(:prenom) AND LOWER(Prenom_Pecheur) LIKE LOWER(:nom)))";
		} else {
			whereParts << "(LOWER(Nom_Pecheur) LIKE LOWER(:terme) OR LOWER(Prenom_Pecheur) LIKE LOWER(:terme))";
		}
	}

	if (!roleSelection.isEmpty() && !isAllRolesSelection(roleSelection)) {
		whereParts << "LOWER(Role) = LOWER(:role)";
	}

	if (!dispoSelection.isEmpty() && !isAllDisponibiliteSelection(dispoSelection)) {
		whereParts << "LOWER(Disponibilite) = LOWER(:dispo)";
	}
}

static void bindPecheursFilters(QSqlQuery& query,
						const QString& recherche,
						const QString& roleSelection,
						const QString& dispoSelection)
{
	if (!recherche.isEmpty()) {
		const QStringList termes = recherche.split(' ', Qt::SkipEmptyParts);
		if (termes.size() >= 2) {
			const QString nomTerme = termes.at(0);
			const QString prenomTerme = termes.mid(1).join(" ");
			query.bindValue(QStringLiteral(":nom"), "%" + nomTerme + "%");
			query.bindValue(QStringLiteral(":prenom"), "%" + prenomTerme + "%");
		} else {
			query.bindValue(QStringLiteral(":terme"), "%" + recherche + "%");
		}
	}

	if (!roleSelection.isEmpty() && !isAllRolesSelection(roleSelection)) {
		const QString roleCanonical = canonicalRole(roleSelection);
		query.bindValue(QStringLiteral(":role"), roleCanonical.isEmpty() ? roleSelection.trimmed() : roleCanonical);
	}
	if (!dispoSelection.isEmpty() && !isAllDisponibiliteSelection(dispoSelection)) {
		const QString dispoCanonical = canonicalDisponibilite(dispoSelection);
		query.bindValue(QStringLiteral(":dispo"), dispoCanonical.isEmpty() ? dispoSelection.trimmed() : dispoCanonical);
	}
}

Pecheurs::Pecheurs()
	: heures_(0), idBateau_(0)
{
}

Pecheurs::Pecheurs(const QString& id, const QString& nom, const QString& prenom,
				   const QString& sexe,
				   const QString& role, const QString& disponibilite,
				   const QString& email, int heures,
				   const QDate& dateInscription, const QDate& dateAffectation,
				   int idBateau)
	: id_(id),
	  nom_(nom),
	  prenom_(prenom),
	  sexe_(sexe),
	  role_(role),
	  disponibilite_(disponibilite),
	  email_(email),
	  heures_(heures),
	  dateInscription_(dateInscription),
	  dateAffectation_(dateAffectation),
	  idBateau_(idBateau)
{
}

bool Pecheurs::ajouter() const
{
	if (!validateIdColumnCompatibility(id_)) {
		return false;
	}

	if (id_.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("ID obligatoire.");
		return false;
	}

	const bool textIdSupported = idPecheurColumnSupportsText();
	QString normalizedId = id_.trimmed().toUpper();
	if (textIdSupported) {
		normalizedId = normalizePecheurProfessionalId(normalizedId);
	}

	if (textIdSupported) {
		if (!isValidPecheurProfessionalId(id_)) {
			s_lastError = QStringLiteral("ID invalide (format attendu: YYHxxxx ou YYFxxxx, ex: 26H0001. "
				"Ancien format accept\u00E9: YY1xxxx / YY2xxxx).");
			return false;
		}
	} else {
		static const QRegularExpression numericPattern(QStringLiteral("^\\d{2}[12]\\d{4}$"));
		if (!numericPattern.match(normalizedId).hasMatch()) {
			s_lastError = QStringLiteral("ID invalide (format attendu: YY1xxxx ou YY2xxxx, ex: 2610001). ");
			return false;
		}
	}

	if (textIdSupported && normalizedId.isEmpty()) {
		s_lastError = QStringLiteral("ID invalide.");
		return false;
	}

	if (nom_.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("Nom obligatoire.");
		return false;
	}
	if (prenom_.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("Prénom obligatoire.");
		return false;
	}
	if (email_.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("Email obligatoire.");
		return false;
	}

	const QString sexeCanonical = canonicalSexe(sexe_);
	if (sexeCanonical.isEmpty()) {
		s_lastError = QStringLiteral("Sexe invalide. Valeurs autorisées: M ou F.");
		return false;
	}

	const QString roleCanonical = canonicalRole(role_);
	if (roleCanonical.isEmpty()) {
		s_lastError = QStringLiteral("Rôle invalide. Valeurs autorisées: Maitre de peche, Apprenti pecheur, Marin-pecheur, Contremaitre.");
		return false;
	}

	const QString dispoCanonical = canonicalDisponibilite(disponibilite_);
	if (dispoCanonical.isEmpty()) {
		s_lastError = QStringLiteral("Disponibilité invalide. Valeurs autorisées: Disponible, Disponible bientot, Indisponible, En conge.");
		return false;
	}

	if (!isValidEmail(email_)) {
		s_lastError = QStringLiteral("Email invalide (format attendu: nom@domaine.tld).");
		return false;
	}

	if (heures_ < 0) {
		s_lastError = QStringLiteral("Heures invalides (doit être >= 0).");
		return false;
	}

	if (!validatePecheursTemporalConstraints(dateInscription_, dateAffectation_, heures_, true)) {
		return false;
	}

	QSqlQuery query;
	query.prepare(
		"INSERT INTO PECHEURS "
		"(ID_Pecheur, Nom_Pecheur, Prenom_Pecheur, Sexe, Role, Disponibilite, Email, Heures, "
		"Date_Inscription, Date_Affectation, ID_Bateau) "
		"VALUES (:id, :nom, :prenom, :sexe, :role, :disp, :email, :heures, :dins, :daff, :idb)");
	query.bindValue(":id", normalizedId);
	query.bindValue(":nom", nom_.trimmed());
	query.bindValue(":prenom", prenom_.trimmed());
	query.bindValue(":sexe", sexeCanonical);
	query.bindValue(":role", roleCanonical);
	query.bindValue(":disp", dispoCanonical);
	query.bindValue(":email", email_.trimmed());
	query.bindValue(":heures", heures_);
	query.bindValue(":dins", dateInscription_.isValid() ? QVariant(dateInscription_) : nullDate());
	query.bindValue(":daff", dateAffectation_.isValid() ? QVariant(dateAffectation_) : nullDate());
	query.bindValue(":idb", idBateau_ > 0 ? QVariant(idBateau_) : nullInt());
	if (!query.exec()) {
		QString debugInfo = QString("Valeurs: ID=%1, Nom='%2', Prenom='%3', Role='%4', Dispo='%5', Email='%6', Heures=%7, ID_Bateau=%8")
			.arg(id_)
			.arg(nom_.trimmed())
			.arg(prenom_.trimmed())
			.arg(roleCanonical)
			.arg(dispoCanonical)
			.arg(email_.trimmed())
			.arg(heures_)
			.arg(idBateau_);
		s_lastError = query.lastError().text() + "\n" + debugInfo;
		return false;
	}
	s_lastError.clear();
	return true;
}

bool Pecheurs::supprimer(const QString& id)
{
	if (id.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("ID invalide.");
		return false;
	}

	QSqlQuery query;
	query.prepare("DELETE FROM PECHEURS WHERE ID_Pecheur = :id");
	query.bindValue(":id", id.trimmed().toUpper());
	if (!query.exec()) {
		s_lastError = query.lastError().text();
		return false;
	}
	s_lastError.clear();
	return true;
}

bool Pecheurs::modifier() const
{
	return modifierAvecAncienId(id_);
}

bool Pecheurs::modifierAvecAncienId(const QString& ancienId) const
{
	if (!validateIdColumnCompatibility(id_) || !validateIdColumnCompatibility(ancienId)) {
		return false;
	}

	if (id_.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("ID invalide.");
		return false;
	}

	const bool textIdSupported = idPecheurColumnSupportsText();
	QString normalizedNewId = id_.trimmed().toUpper();
	if (textIdSupported) {
		normalizedNewId = normalizePecheurProfessionalId(normalizedNewId);
		if (!isValidPecheurProfessionalId(id_)) {
			s_lastError = QStringLiteral("ID invalide (format attendu: YYHxxxx ou YYFxxxx, ex: 26H0001. "
				"Ancien format accept\u00E9: YY1xxxx / YY2xxxx).");
			return false;
		}
	} else {
		static const QRegularExpression numericPattern(QStringLiteral("^\\d{2}[12]\\d{4}$"));
		if (!numericPattern.match(normalizedNewId).hasMatch()) {
			s_lastError = QStringLiteral("ID invalide (format attendu: YY1xxxx ou YY2xxxx, ex: 2610001). ");
			return false;
		}
	}

	if (ancienId.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("ID original invalide.");
		return false;
	}
	const QString sexeCanonical = canonicalSexe(sexe_);
	if (sexeCanonical.isEmpty()) {
		s_lastError = QStringLiteral("Sexe invalide. Valeurs autorisées: M ou F.");
		return false;
	}


	if (nom_.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("Nom obligatoire.");
		return false;
	}
	if (prenom_.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("Prénom obligatoire.");
		return false;
	}
	if (email_.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("Email obligatoire.");
		return false;
	}

	const QString roleCanonical = canonicalRole(role_);
	if (roleCanonical.isEmpty()) {
		s_lastError = QStringLiteral("Rôle invalide. Valeurs autorisées: Maitre de peche, Apprenti pecheur, Marin-pecheur, Contremaitre.");
		return false;
	}

	const QString dispoCanonical = canonicalDisponibilite(disponibilite_);
	if (dispoCanonical.isEmpty()) {
		s_lastError = QStringLiteral("Disponibilité invalide. Valeurs autorisées: Disponible, Disponible bientot, Indisponible, En conge.");
		return false;
	}

	if (!isValidEmail(email_)) {
		s_lastError = QStringLiteral("Email invalide (format attendu: nom@domaine.tld).");
		return false;
	}

	if (heures_ < 0) {
		s_lastError = QStringLiteral("Heures invalides (doit être >= 0).");
		return false;
	}

	if (!validatePecheursTemporalConstraints(dateInscription_, dateAffectation_, heures_, false)) {
		return false;
	}

	const QString normalizedOldId = ancienId.trimmed().toUpper();

	if (normalizedNewId != normalizedOldId) {
		QSqlQuery checkIdQuery;
		checkIdQuery.prepare(
			"SELECT 1 FROM PECHEURS "
			"WHERE ID_Pecheur = :newId AND ID_Pecheur <> :oldId");
		checkIdQuery.bindValue(":newId", normalizedNewId);
		checkIdQuery.bindValue(":oldId", normalizedOldId);

		if (!checkIdQuery.exec()) {
			s_lastError = checkIdQuery.lastError().text();
			return false;
		}

		if (checkIdQuery.next()) {
			s_lastError = QStringLiteral("Cet ID est déjà utilisé. Veuillez saisir un ID différent.");
			return false;
		}
	}

	QSqlQuery query;
	query.prepare(
		"UPDATE PECHEURS SET "
		"ID_Pecheur = :newId, Nom_Pecheur = :nom, Prenom_Pecheur = :prenom, Sexe = :sexe, Role = :role, "
		"Disponibilite = :disp, Email = :email, Heures = :heures, "
		"Date_Inscription = :dins, Date_Affectation = :daff, ID_Bateau = :idb "
		"WHERE ID_Pecheur = :oldId");
	query.bindValue(":newId", normalizedNewId);
	query.bindValue(":nom", nom_.trimmed());
	query.bindValue(":prenom", prenom_.trimmed());
	query.bindValue(":sexe", sexeCanonical);
	query.bindValue(":role", roleCanonical);
	query.bindValue(":disp", dispoCanonical);
	query.bindValue(":email", email_.trimmed());
	query.bindValue(":heures", heures_);
	query.bindValue(":dins", dateInscription_.isValid() ? QVariant(dateInscription_) : nullDate());
	query.bindValue(":daff", dateAffectation_.isValid() ? QVariant(dateAffectation_) : nullDate());
	query.bindValue(":idb", idBateau_ > 0 ? QVariant(idBateau_) : nullInt());
	query.bindValue(":oldId", normalizedOldId);

	if (!query.exec()) {
		const QString dbError = query.lastError().text();
		if (dbError.contains(QStringLiteral("ORA-00001"), Qt::CaseInsensitive)) {
			s_lastError = QStringLiteral("Cet ID est déjà utilisé. Veuillez saisir un ID différent.");
		} else {
			s_lastError = dbError;
		}
		return false;
	}
	const int rowsAffected = query.numRowsAffected();
	if (rowsAffected <= 0) {
		s_lastError = QStringLiteral("Aucune ligne mise à jour. Vérifier l'ID original du pêcheur.");
		return false;
	}
	s_lastError.clear();
	return true;
}

QSqlQueryModel* Pecheurs::afficher()
{
	auto* model = new QSqlQueryModel();
	model->setQuery("SELECT * FROM PECHEURS ORDER BY ID_Pecheur");
	return model;
}

bool Pecheurs::chargerTable(const QString& recherche,
						   const QString& roleSelection,
						   const QString& dispoSelection,
						   QVector<TableRowData>& rows)
{
	rows.clear();

	QSqlQuery query;
	QStringList whereParts;
	appendPecheursFilters(whereParts, recherche, roleSelection, dispoSelection);

	QString sql =
		"SELECT ID_Pecheur, Nom_Pecheur, Prenom_Pecheur, Sexe, Role, ID_Bateau, "
		"Disponibilite, Email, Date_Inscription, Date_Affectation, Heures, Photo "
		"FROM PECHEURS";
	if (!whereParts.isEmpty()) {
		sql += " WHERE " + whereParts.join(" AND ");
	}
	sql += " ORDER BY ID_Pecheur";
	query.prepare(sql);
	bindPecheursFilters(query, recherche, roleSelection, dispoSelection);

	if (!query.exec()) {
		s_lastError = query.lastError().text();
		return false;
	}

	while (query.next()) {
		TableRowData row;
		row.id = query.value(0).toString();
		row.nom = query.value(1).toString();
		row.prenom = query.value(2).toString();
		row.sexe = query.value(3).toString();
		row.role = query.value(4).toString();
		row.idBateau = query.value(5).toInt();
		row.disponibilite = query.value(6).toString();
		row.email = query.value(7).toString();
		row.dateInscription = query.value(8).toDate();
		row.dateAffectation = query.value(9).toDate();
		row.heures = query.value(10).toInt();
		row.photo = query.value(11).toString();
		rows.push_back(row);
	}

	s_lastError.clear();
	return true;
}

Pecheurs::DisponibiliteStats Pecheurs::calculerDisponibiliteStats(const QString& recherche,
										  const QString& roleSelection,
										  const QString& dispoSelection)
{
	DisponibiliteStats stats;

	QSqlQuery query;
	QStringList whereParts;
	appendPecheursFilters(whereParts, recherche, roleSelection, dispoSelection);

	QString sql = QStringLiteral("SELECT Disponibilite FROM PECHEURS");
	if (!whereParts.isEmpty()) {
		sql += " WHERE " + whereParts.join(" AND ");
	}

	query.prepare(sql);
	bindPecheursFilters(query, recherche, roleSelection, dispoSelection);

	if (!query.exec()) {
		s_lastError = query.lastError().text();
		return stats;
	}

	while (query.next()) {
		const QString d = normalizeText(query.value(0).toString());
		if (d.contains(QStringLiteral("indisponible"))) {
			++stats.indisponible;
		} else if (d.contains(QStringLiteral("bientot"))) {
			++stats.bientot;
		} else if (d.contains(QStringLiteral("conge"))) {
			++stats.enConge;
		} else if (d.contains(QStringLiteral("disponible"))) {
			++stats.disponible;
		}
	}

	s_lastError.clear();
	return stats;
}

bool Pecheurs::idExiste(const QString& id)
{
	if (!validateIdColumnCompatibility(id)) {
		return false;
	}

	if (id.trimmed().isEmpty()) return false;

	QSqlQuery query;
	query.prepare("SELECT 1 FROM PECHEURS WHERE ID_Pecheur = :id");
	query.bindValue(":id", id.trimmed().toUpper());

	if (!query.exec()) {
		s_lastError = query.lastError().text();
		return false;
	}

	s_lastError.clear();
	return query.next();
}

QString Pecheurs::genererNouvelId(const QString& sexe)
{
	const QString canonical = canonicalSexe(sexe);
	if (canonical.isEmpty()) {
		s_lastError = QStringLiteral("Sexe invalide. Choisir Homme ou Femme.");
		return QString();
	}

	const bool textIdSupported = idPecheurColumnSupportsText();

	const QString yy = QString::number(QDate::currentDate().year() % 100).rightJustified(2, QLatin1Char('0'));
	const QString newSexCode = (canonical == QStringLiteral("M"))
		? (textIdSupported ? QStringLiteral("H") : QStringLiteral("1"))
		: (textIdSupported ? QStringLiteral("F") : QStringLiteral("2"));
	const QString prefix = yy + newSexCode;

	QSqlQuery query;
	if (textIdSupported) {
		const QString legacySexCode = (canonical == QStringLiteral("M")) ? QStringLiteral("1") : QStringLiteral("2");
		const QString legacyPrefix = yy + legacySexCode;
		query.prepare(
			"SELECT NVL(MAX(TO_NUMBER(SUBSTR(ID_Pecheur, 4, 4))), 0) "
			"FROM PECHEURS WHERE ID_Pecheur LIKE :p1 OR ID_Pecheur LIKE :p2");
		query.bindValue(":p1", prefix + QStringLiteral("%"));
		query.bindValue(":p2", legacyPrefix + QStringLiteral("%"));
	} else {
		query.prepare(
			"SELECT NVL(MAX(TO_NUMBER(SUBSTR(ID_Pecheur, 4, 4))), 0) "
			"FROM PECHEURS WHERE ID_Pecheur LIKE :prefix");
		query.bindValue(":prefix", prefix + QStringLiteral("%"));
	}

	if (!query.exec()) {
		s_lastError = query.lastError().text();
		return QString();
	}

	int maxSeq = 0;
	if (query.next()) {
		maxSeq = query.value(0).toInt();
	}

	const QString nextSeq = QString::number(maxSeq + 1).rightJustified(4, QLatin1Char('0'));
	s_lastError.clear();
	return prefix + nextSeq;
}

int Pecheurs::bateauIdFromText(const QString& text)
{
	const int direct = toIntOrZeroLocal(text);
	if (direct > 0 || text.trimmed().isEmpty()) return direct;

	QSqlQuery query;
	query.prepare("SELECT ID_Bateau FROM BATEAUX WHERE Nom = :nom");
	query.bindValue(":nom", text.trimmed());

	if (query.exec() && query.next()) {
		s_lastError.clear();
		return query.value(0).toInt();
	}

	if (!query.lastError().text().isEmpty()) {
		s_lastError = query.lastError().text();
	}
	return 0;
}

QString Pecheurs::lastError()
{
	return s_lastError;
}
