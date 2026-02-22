#include "pecheurs.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QMetaType>
#include <QRegularExpression>
#include <QDebug>

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

static bool isValidEmail(const QString& email)
{
	if (email.trimmed().isEmpty()) {
		return true;
	}
	static const QRegularExpression pattern(QStringLiteral("^[^\\s@]+@[^\\s@]+\\.[^\\s@]+$"));
	return pattern.match(email.trimmed()).hasMatch();
}

Pecheurs::Pecheurs()
	: id_(0), heures_(0), idBateau_(0)
{
}

Pecheurs::Pecheurs(int id, const QString& nom, const QString& prenom,
				   const QString& role, const QString& disponibilite,
				   const QString& email, int heures,
				   const QDate& dateInscription, const QDate& dateAffectation,
				   int idBateau)
	: id_(id),
	  nom_(nom),
	  prenom_(prenom),
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
	// Validation ID
	if (id_ <= 0) {
		s_lastError = QStringLiteral("ID invalide (doit être > 0).");
		return false;
	}

	// Validation Nom et Prénom obligatoires
	if (nom_.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("Nom obligatoire.");
		return false;
	}
	if (prenom_.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("Prénom obligatoire.");
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

	// Validation heures
	if (heures_ < 0) {
		s_lastError = QStringLiteral("Heures invalides (doit être >= 0).");
		return false;
	}

	if (dateInscription_.isValid() && dateAffectation_.isValid() && dateAffectation_ < dateInscription_) {
		s_lastError = QStringLiteral("Date d'affectation invalide (doit être >= date d'inscription).");
		return false;
	}

	QSqlQuery query;
	query.prepare(
		"INSERT INTO PECHEURS "
		"(ID_Pecheur, Nom_Pecheur, Prenom_Pecheur, Role, Disponibilite, Email, Heures, "
		"Date_Inscription, Date_Affectation, ID_Bateau) "
		"VALUES (:id, :nom, :prenom, :role, :disp, :email, :heures, :dins, :daff, :idb)");
	query.bindValue(":id", id_);
	query.bindValue(":nom", nom_.trimmed());
	query.bindValue(":prenom", prenom_.trimmed());
	query.bindValue(":role", roleCanonical);
	query.bindValue(":disp", dispoCanonical);
	query.bindValue(":email", email_.trimmed().isEmpty() ? QVariant(QMetaType::fromType<QString>()) : QVariant(email_.trimmed()));
	query.bindValue(":heures", heures_);  // Envoyer 0 si c'est 0, pas NULL
	query.bindValue(":dins", dateInscription_.isValid() ? QVariant(dateInscription_) : nullDate());
	query.bindValue(":daff", dateAffectation_.isValid() ? QVariant(dateAffectation_) : nullDate());
	query.bindValue(":idb", idBateau_ > 0 ? QVariant(idBateau_) : nullInt());
	if (!query.exec()) {
		// Créer un message d'erreur détaillé avec les valeurs envoyées
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

bool Pecheurs::supprimer(int id)
{
	QSqlQuery query;
	query.prepare("DELETE FROM PECHEURS WHERE ID_Pecheur = :id");
	query.bindValue(":id", id);
	if (!query.exec()) {
		s_lastError = query.lastError().text();
		return false;
	}
	s_lastError.clear();
	return true;
}

bool Pecheurs::modifier() const
{
	// Validation ID
	if (id_ <= 0) {
		s_lastError = QStringLiteral("ID invalide (doit être > 0).");
		return false;
	}

	// Validation Nom et Prénom obligatoires
	if (nom_.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("Nom obligatoire.");
		return false;
	}
	if (prenom_.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("Prénom obligatoire.");
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

	// Validation heures
	if (heures_ < 0) {
		s_lastError = QStringLiteral("Heures invalides (doit être >= 0).");
		return false;
	}

	if (dateInscription_.isValid() && dateAffectation_.isValid() && dateAffectation_ < dateInscription_) {
		s_lastError = QStringLiteral("Date d'affectation invalide (doit être >= date d'inscription).");
		return false;
	}

	QSqlQuery query;
	query.prepare(
		"UPDATE PECHEURS SET "
		"Nom_Pecheur = :nom, Prenom_Pecheur = :prenom, Role = :role, "
		"Disponibilite = :disp, Email = :email, Heures = :heures, "
		"Date_Inscription = :dins, Date_Affectation = :daff, ID_Bateau = :idb "
		"WHERE ID_Pecheur = :id");
	query.bindValue(":nom", nom_.trimmed());
	query.bindValue(":prenom", prenom_.trimmed());
	query.bindValue(":role", roleCanonical);
	query.bindValue(":disp", dispoCanonical);
	query.bindValue(":email", email_.trimmed().isEmpty() ? QVariant(QMetaType::fromType<QString>()) : QVariant(email_.trimmed()));
	query.bindValue(":heures", heures_);
	query.bindValue(":dins", dateInscription_.isValid() ? QVariant(dateInscription_) : nullDate());
	query.bindValue(":daff", dateAffectation_.isValid() ? QVariant(dateAffectation_) : nullDate());
	query.bindValue(":idb", idBateau_ > 0 ? QVariant(idBateau_) : nullInt());
	query.bindValue(":id", id_);
	
	qDebug() << "=== Pecheurs::modifier() UPDATE QUERY ===";
	qDebug() << "ID:" << id_ << "Nom:" << nom_.trimmed() << "Prenom:" << prenom_.trimmed() 
	         << "Role:" << roleCanonical << "Dispo:" << dispoCanonical << "Email:" << email_.trimmed();
	
	if (!query.exec()) {
		s_lastError = query.lastError().text();
		qDebug() << "query.exec() FAILED!" << s_lastError;
		return false;
	}
	const int rowsAffected = query.numRowsAffected();
	qDebug() << "query.exec() SUCCESS - Rows affected:" << rowsAffected;
	if (rowsAffected <= 0) {
		s_lastError = QStringLiteral("Aucune ligne mise à jour. Vérifier l'ID du pêcheur.");
		qDebug() << "ERREUR: " << s_lastError;
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

QString Pecheurs::lastError()
{
	return s_lastError;
}
