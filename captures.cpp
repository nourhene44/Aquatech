#include "captures.h"
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

namespace {

QString s_lastError;

QString friendlyOracleError(const QString& raw)
{
	const QString e = raw.toUpper();
	if (e.contains(QStringLiteral("ORA-02291"))) {
		return QStringLiteral("ID bateau introuvable dans BATEAUX (contrainte FK_CAPTURE_BATEAU). Ajoutez d'abord ce bateau.");
	}
	if (e.contains(QStringLiteral("ORA-00001"))) {
		return QStringLiteral("ID capture deja existant (cle primaire). Utilisez un ID capture unique.");
	}
	if (e.contains(QStringLiteral("ORA-01722"))) {
		return QStringLiteral("Valeur numerique invalide. ID_CAPTURE et ID_BATEAU doivent etre numeriques.");
	}
	if (e.contains(QStringLiteral("ORA-02290")) || e.contains(QStringLiteral("TYPE_POISSON"))) {
		return QStringLiteral("Type de poisson invalide (valeurs autorisees: Sardine, Thon, Merlu, Dorade, Loup, Rouget, Calamar, Poulpes, Crevette, Merou, Maquereau).");
	}
	return raw;
}

bool parsePositiveInt(const QString& text, int* out)
{
	bool ok = false;
	const int v = text.trimmed().toInt(&ok);
	if (!ok || v <= 0) {
		return false;
	}
	if (out) *out = v;
	return true;
}

QString normalizeKey(const QString& s)
{
	QString out = s.normalized(QString::NormalizationForm_D);
	static QRegularExpression diacRx(QStringLiteral("\\p{Mn}+"));
	out.remove(diacRx);
	out.replace(QRegularExpression(QStringLiteral("[\\s_]+")), QString());
	return out.toLower();
}

QString matchColumnBySynonyms(const QStringList& dbCols, const QStringList& syns)
{
	for (const QString& syn : syns) {
		const QString key = normalizeKey(syn);
		for (const QString& col : dbCols) {
			if (normalizeKey(col) == key) {
				return col;
			}
		}
	}
	for (const QString& syn : syns) {
		const QString key = normalizeKey(syn);
		for (const QString& col : dbCols) {
			if (normalizeKey(col).startsWith(key)) {
				return col;
			}
		}
	}
	return QString();
}

bool canSelectColumn(QSqlDatabase db, const QString& tableName, const QString& columnName)
{
	if (!db.isValid() || !db.isOpen()) return false;
	if (tableName.trimmed().isEmpty() || columnName.trimmed().isEmpty()) return false;

	QSqlQuery q(db);
	q.prepare(QStringLiteral("SELECT %1 FROM %2 WHERE 1=0").arg(columnName, tableName));
	return q.exec();
}

QString probeTemperatureColumn(QSqlDatabase db, const QString& tableName)
{
	static const QStringList candidates = {
		QStringLiteral("TEMPERATURE"),
		QStringLiteral("TEMP"),
		QStringLiteral("TEMP_C"),
		QStringLiteral("TEMPERATURE_C")
	};
	for (const QString& c : candidates) {
		if (canSelectColumn(db, tableName, c)) {
			return c;
		}
	}
	return QString();
}

QString findCapturesTable(QSqlDatabase db)
{
	const QStringList tables = db.tables(QSql::Tables) + db.tables(QSql::Views);
	const QStringList candidates = {
		QStringLiteral("CAPTURES"),
		QStringLiteral("CAPTURE"),
		QStringLiteral("TCAPTURES"),
		QStringLiteral("T_CAPTURES")
	};

	for (const QString& cand : candidates) {
		const QString key = normalizeKey(cand);
		for (const QString& t : tables) {
			if (normalizeKey(t) == key || normalizeKey(t).startsWith(key)) {
				return t;
			}
		}
	}

	return QString();
}

QString findBateauxTable(QSqlDatabase db)
{
	const QStringList tables = db.tables(QSql::Tables) + db.tables(QSql::Views);
	const QStringList candidates = {
		QStringLiteral("BATEAUX"),
		QStringLiteral("BATEAU"),
		QStringLiteral("TBATEAUX"),
		QStringLiteral("T_BATEAUX")
	};

	for (const QString& cand : candidates) {
		const QString key = normalizeKey(cand);
		for (const QString& t : tables) {
			if (normalizeKey(t) == key || normalizeKey(t).startsWith(key)) {
				return t;
			}
		}
	}

	return QString();
}

bool bateauExiste(QSqlDatabase db, int idBateau)
{
	if (idBateau <= 0) {
		return false;
	}

	const QString table = findBateauxTable(db);
	if (table.isEmpty()) {
		s_lastError = QStringLiteral("Table BATEAUX introuvable.");
		return false;
	}

	const QSqlRecord rec = db.record(table);
	QStringList cols;
	for (int i = 0; i < rec.count(); ++i) {
		cols << rec.fieldName(i);
	}

	const QString idCol = matchColumnBySynonyms(cols, {
		QStringLiteral("id_bateau"), QStringLiteral("idbateau"), QStringLiteral("id")
	});

	if (idCol.isEmpty()) {
		s_lastError = QStringLiteral("Colonne ID_BATEAU introuvable dans BATEAUX.");
		return false;
	}

	QSqlQuery query(db);
	query.prepare(QStringLiteral("SELECT 1 FROM %1 WHERE %2 = ?").arg(table, idCol));
	query.addBindValue(idBateau);
	if (!query.exec()) {
		s_lastError = friendlyOracleError(query.lastError().text());
		return false;
	}

	return query.next();
}

bool resolveColumns(QSqlDatabase db,
					QString* tableName,
					QString* idCol,
					QString* idBateauCol,
					QString* typeCol,
					QString* quantiteCol,
					QString* poidsCol,
					QString* dateCol,
					QString* temperatureCol)
{
	if (!db.isValid() || !db.isOpen()) {
		s_lastError = QStringLiteral("Connexion base de donnees indisponible.");
		return false;
	}

	const QString table = findCapturesTable(db);
	if (table.isEmpty()) {
		s_lastError = QStringLiteral("Table CAPTURES introuvable.");
		return false;
	}

	const QSqlRecord rec = db.record(table);
	QStringList cols;
	for (int i = 0; i < rec.count(); ++i) {
		cols << rec.fieldName(i);
	}

	const QString cId = matchColumnBySynonyms(cols, {
		QStringLiteral("id_capture"), QStringLiteral("idcapture"), QStringLiteral("id")
	});
	const QString cIdBateau = matchColumnBySynonyms(cols, {
		QStringLiteral("id_bateau"), QStringLiteral("idbateau"), QStringLiteral("bateau_id")
	});
	const QString cType = matchColumnBySynonyms(cols, {
		QStringLiteral("type_poisson"), QStringLiteral("typepoisson"), QStringLiteral("poisson"), QStringLiteral("espece")
	});
	const QString cQuantite = matchColumnBySynonyms(cols, {
		QStringLiteral("quantite"), QStringLiteral("qte")
	});
	const QString cPoids = matchColumnBySynonyms(cols, {
		QStringLiteral("poids"), QStringLiteral("poids_kg")
	});
	const QString cDate = matchColumnBySynonyms(cols, {
		QStringLiteral("date_capture"), QStringLiteral("datecapture"), QStringLiteral("date")
	});
	const QString cTemperature = matchColumnBySynonyms(cols, {
		QStringLiteral("temperature"), QStringLiteral("temp"), QStringLiteral("temp_c"), QStringLiteral("temperature_c")
	});

	if (cId.isEmpty() || cIdBateau.isEmpty() || cType.isEmpty() || cQuantite.isEmpty() || cPoids.isEmpty() || cDate.isEmpty()) {
		s_lastError = QStringLiteral("Colonnes CAPTURES manquantes (ID/ID_BATEAU/TYPE/QUANTITE/POIDS/DATE).");
		return false;
	}

	*tableName = table;
	*idCol = cId;
	*idBateauCol = cIdBateau;
	*typeCol = cType;
	*quantiteCol = cQuantite;
	*poidsCol = cPoids;
	*dateCol = cDate;
	if (temperatureCol) {
		*temperatureCol = cTemperature;
		if (temperatureCol->isEmpty()) {
			*temperatureCol = probeTemperatureColumn(db, table);
		}
	}
	return true;
}

bool resolveColumns(QSqlDatabase db,
					QString* tableName,
					QString* idCol,
					QString* idBateauCol,
					QString* typeCol,
					QString* quantiteCol,
					QString* poidsCol,
					QString* dateCol)
{
	QString tempCol;
	return resolveColumns(db, tableName, idCol, idBateauCol, typeCol, quantiteCol, poidsCol, dateCol, &tempCol);
}

} // namespace

captures::captures() {}

captures::captures(const QString& idCapture,
				   int idBateau,
				   const QString& typePoisson,
				   int quantite,
				   double poids,
				   const QDate& dateCapture)
	: idCapture_(idCapture)
	, idBateau_(idBateau)
	, typePoisson_(typePoisson)
	, quantite_(quantite)
	, poids_(poids)
	, dateCapture_(dateCapture)
{
}

captures::captures(const QString& idCapture,
				   int idBateau,
				   const QString& typePoisson,
				   int quantite,
				   double poids,
				   const QDate& dateCapture,
				   double temperatureC)
	: idCapture_(idCapture)
	, idBateau_(idBateau)
	, typePoisson_(typePoisson)
	, quantite_(quantite)
	, poids_(poids)
	, temperatureC_(temperatureC)
	, hasTemperature_(true)
	, dateCapture_(dateCapture)
{
}

bool captures::ajouter() const
{
	if (idCapture_.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("ID capture obligatoire.");
		return false;
	}
	int idCaptureNum = 0;
	if (!parsePositiveInt(idCapture_, &idCaptureNum)) {
		s_lastError = QStringLiteral("ID capture invalide (doit etre un nombre > 0).");
		return false;
	}
	if (idBateau_ <= 0) {
		s_lastError = QStringLiteral("ID bateau invalide.");
		return false;
	}
	if (typePoisson_.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("Type poisson obligatoire.");
		return false;
	}
	if (quantite_ <= 0) {
		s_lastError = QStringLiteral("Quantite invalide (doit etre > 0).");
		return false;
	}
	if (poids_ <= 0.0) {
		s_lastError = QStringLiteral("Poids invalide (doit etre > 0).");
		return false;
	}

	QSqlDatabase db = QSqlDatabase::database();
	QString tableName;
	QString idCol;
	QString idBateauCol;
	QString typeCol;
	QString quantiteCol;
	QString poidsCol;
	QString dateCol;
	QString temperatureCol;
	if (!resolveColumns(db, &tableName, &idCol, &idBateauCol, &typeCol, &quantiteCol, &poidsCol, &dateCol, &temperatureCol)) {
		return false;
	}
	if (hasTemperature_ && temperatureCol.isEmpty()) {
		s_lastError = QStringLiteral("Colonne TEMPERATURE introuvable dans %1. Ajoutez-la (ALTER TABLE %1 ADD TEMPERATURE NUMBER).")
					  .arg(tableName);
		return false;
	}
	if (!bateauExiste(db, idBateau_)) {
		if (s_lastError.isEmpty()) {
			s_lastError = QStringLiteral("ID bateau introuvable dans BATEAUX.");
		}
		return false;
	}

	QStringList cols = { idCol, idBateauCol, typeCol, quantiteCol, poidsCol, dateCol };
	QStringList placeholders = { QStringLiteral("?"), QStringLiteral("?"), QStringLiteral("?"), QStringLiteral("?"), QStringLiteral("?"), QStringLiteral("?") };
	QList<QVariant> binds;
	binds << idCaptureNum
		  << idBateau_
		  << typePoisson_.trimmed()
		  << quantite_
		  << poids_
		  << (dateCapture_.isValid() ? QVariant(dateCapture_) : QVariant(QDate::currentDate()));

	if (!temperatureCol.isEmpty() && hasTemperature_) {
		cols.insert(cols.size() - 1, temperatureCol);
		placeholders.insert(placeholders.size() - 1, QStringLiteral("?"));
		binds.insert(binds.size() - 1, temperatureC_);
	}

	QSqlQuery query(db);
	query.prepare(QStringLiteral("INSERT INTO %1 (%2) VALUES (%3)")
				  .arg(tableName, cols.join(QStringLiteral(", ")), placeholders.join(QStringLiteral(", "))));
	for (const QVariant& v : binds) {
		query.addBindValue(v);
	}

	if (!query.exec()) {
		s_lastError = friendlyOracleError(query.lastError().text());
		return false;
	}

	s_lastError.clear();
	return true;
}

bool captures::modifier() const
{
	return modifierAvecAncienId(idCapture_);
}

bool captures::modifierAvecAncienId(const QString& ancienId) const
{
	if (ancienId.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("ID original invalide.");
		return false;
	}
	if (idCapture_.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("ID capture obligatoire.");
		return false;
	}
	int newIdNum = 0;
	if (!parsePositiveInt(idCapture_, &newIdNum)) {
		s_lastError = QStringLiteral("ID capture invalide (doit etre un nombre > 0).");
		return false;
	}
	int oldIdNum = 0;
	if (!parsePositiveInt(ancienId, &oldIdNum)) {
		s_lastError = QStringLiteral("ID original invalide (doit etre numerique).");
		return false;
	}
	if (idBateau_ <= 0) {
		s_lastError = QStringLiteral("ID bateau invalide.");
		return false;
	}

	QSqlDatabase db = QSqlDatabase::database();
	QString tableName;
	QString idCol;
	QString idBateauCol;
	QString typeCol;
	QString quantiteCol;
	QString poidsCol;
	QString dateCol;
	QString temperatureCol;
	if (!resolveColumns(db, &tableName, &idCol, &idBateauCol, &typeCol, &quantiteCol, &poidsCol, &dateCol, &temperatureCol)) {
		return false;
	}
	if (hasTemperature_ && temperatureCol.isEmpty()) {
		s_lastError = QStringLiteral("Colonne TEMPERATURE introuvable dans %1. Ajoutez-la (ALTER TABLE %1 ADD TEMPERATURE NUMBER).")
					  .arg(tableName);
		return false;
	}
	if (!bateauExiste(db, idBateau_)) {
		if (s_lastError.isEmpty()) {
			s_lastError = QStringLiteral("ID bateau introuvable dans BATEAUX.");
		}
		return false;
	}

	QStringList sets;
	QList<QVariant> binds;
	sets << QStringLiteral("%1 = ?").arg(idCol);
	binds << newIdNum;
	sets << QStringLiteral("%1 = ?").arg(idBateauCol);
	binds << idBateau_;
	sets << QStringLiteral("%1 = ?").arg(typeCol);
	binds << typePoisson_.trimmed();
	sets << QStringLiteral("%1 = ?").arg(quantiteCol);
	binds << quantite_;
	sets << QStringLiteral("%1 = ?").arg(poidsCol);
	binds << poids_;
	if (!temperatureCol.isEmpty() && hasTemperature_) {
		sets << QStringLiteral("%1 = ?").arg(temperatureCol);
		binds << temperatureC_;
	}
	sets << QStringLiteral("%1 = ?").arg(dateCol);
	binds << (dateCapture_.isValid() ? QVariant(dateCapture_) : QVariant(QDate::currentDate()));

	QSqlQuery query(db);
	query.prepare(QStringLiteral("UPDATE %1 SET %2 WHERE %3 = ?")
				  .arg(tableName, sets.join(QStringLiteral(", ")), idCol));
	for (const QVariant& v : binds) {
		query.addBindValue(v);
	}
	query.addBindValue(oldIdNum);

	if (!query.exec()) {
		s_lastError = friendlyOracleError(query.lastError().text());
		return false;
	}
	if (query.numRowsAffected() <= 0) {
		s_lastError = QStringLiteral("Aucune capture mise a jour.");
		return false;
	}

	s_lastError.clear();
	return true;
}

bool captures::supprimer(const QString& idCapture)
{
	if (idCapture.trimmed().isEmpty()) {
		s_lastError = QStringLiteral("ID capture invalide.");
		return false;
	}
	int idCaptureNum = 0;
	if (!parsePositiveInt(idCapture, &idCaptureNum)) {
		s_lastError = QStringLiteral("ID capture invalide (doit etre numerique).");
		return false;
	}

	QSqlDatabase db = QSqlDatabase::database();
	QString tableName;
	QString idCol;
	QString idBateauCol;
	QString typeCol;
	QString quantiteCol;
	QString poidsCol;
	QString dateCol;
	if (!resolveColumns(db, &tableName, &idCol, &idBateauCol, &typeCol, &quantiteCol, &poidsCol, &dateCol)) {
		return false;
	}

	QSqlQuery query(db);
	query.prepare(QStringLiteral("DELETE FROM %1 WHERE %2 = ?").arg(tableName, idCol));
	query.addBindValue(idCaptureNum);

	if (!query.exec()) {
		s_lastError = friendlyOracleError(query.lastError().text());
		return false;
	}

	s_lastError.clear();
	return true;
}

bool captures::idExiste(const QString& idCapture)
{
	if (idCapture.trimmed().isEmpty()) {
		return false;
	}
	int idCaptureNum = 0;
	if (!parsePositiveInt(idCapture, &idCaptureNum)) {
		s_lastError = QStringLiteral("ID capture invalide (doit etre numerique).");
		return false;
	}

	QSqlDatabase db = QSqlDatabase::database();
	QString tableName;
	QString idCol;
	QString idBateauCol;
	QString typeCol;
	QString quantiteCol;
	QString poidsCol;
	QString dateCol;
	if (!resolveColumns(db, &tableName, &idCol, &idBateauCol, &typeCol, &quantiteCol, &poidsCol, &dateCol)) {
		return false;
	}

	QSqlQuery query(db);
	query.prepare(QStringLiteral("SELECT 1 FROM %1 WHERE %2 = ?").arg(tableName, idCol));
	query.addBindValue(idCaptureNum);
	if (!query.exec()) {
		s_lastError = friendlyOracleError(query.lastError().text());
		return false;
	}

	s_lastError.clear();
	return query.next();
}

QString captures::genererNouvelId()
{
	QSqlDatabase db = QSqlDatabase::database();
	QString tableName;
	QString idCol;
	QString idBateauCol;
	QString typeCol;
	QString quantiteCol;
	QString poidsCol;
	QString dateCol;
	if (!resolveColumns(db, &tableName, &idCol, &idBateauCol, &typeCol, &quantiteCol, &poidsCol, &dateCol)) {
		return QString();
	}

	const int prefix = 266;
	const int minId = prefix * 10000;
	const int maxId = minId + 9999;

	QSqlQuery query(db);
	query.prepare(QStringLiteral(
		"SELECT NVL(MAX(idnum), 0) FROM ("
		"  SELECT TO_NUMBER(REGEXP_SUBSTR(TRIM(TO_CHAR(%1)), '^[0-9]+$')) AS idnum "
		"  FROM %2"
		") WHERE idnum BETWEEN :minId AND :maxId")
				  .arg(idCol, tableName));
	query.bindValue(QStringLiteral(":minId"), minId);
	query.bindValue(QStringLiteral(":maxId"), maxId);

	if (!query.exec()) {
		s_lastError = friendlyOracleError(query.lastError().text());
		return QString();
	}

	int currentMax = 0;
	if (query.next()) {
		currentMax = query.value(0).toInt();
	}

	const int nextId = (currentMax > 0) ? (currentMax + 1) : (minId + 1);
	if (nextId > maxId) {
		s_lastError = QStringLiteral("Limite atteinte pour ce préfixe ID (266NNNN).");
		return QString();
	}

	s_lastError.clear();
	return QString::number(nextId);
}

bool captures::chargerTable(const QString& rechercheId,
							const QDate& dateDebut,
							QVector<TableRowData>& rows)
{
	rows.clear();

	QSqlDatabase db = QSqlDatabase::database();
	QString tableName;
	QString idCol;
	QString idBateauCol;
	QString typeCol;
	QString quantiteCol;
	QString poidsCol;
	QString dateCol;
	QString temperatureCol;
	if (!resolveColumns(db, &tableName, &idCol, &idBateauCol, &typeCol, &quantiteCol, &poidsCol, &dateCol, &temperatureCol)) {
		return false;
	}

	const bool hasTempCol = !temperatureCol.isEmpty();
	QStringList selectCols = { idCol, idBateauCol, typeCol, quantiteCol, poidsCol };
	if (hasTempCol) {
		selectCols << temperatureCol;
	}
	selectCols << dateCol;

	QString sql = QStringLiteral("SELECT %1 FROM %2")
				  .arg(selectCols.join(QStringLiteral(", ")), tableName);

	QStringList where;
	QList<QVariant> binds;
	if (!rechercheId.trimmed().isEmpty()) {
		where << QStringLiteral("UPPER(%1) LIKE ?").arg(idCol);
		binds << QStringLiteral("%") + rechercheId.trimmed().toUpper() + QStringLiteral("%");
	}

	const QDate sentinel(2000, 1, 1);
	if (dateDebut.isValid() && dateDebut > sentinel) {
		where << QStringLiteral("TRUNC(%1) >= TO_DATE(?, 'YYYY-MM-DD')").arg(dateCol);
		binds << dateDebut.toString(QStringLiteral("yyyy-MM-dd"));
	}

	if (!where.isEmpty()) {
		sql += QStringLiteral(" WHERE ") + where.join(QStringLiteral(" AND "));
	}
	sql += QStringLiteral(" ORDER BY %1 DESC").arg(dateCol);

	QSqlQuery query(db);
	query.prepare(sql);
	for (const QVariant& v : binds) {
		query.addBindValue(v);
	}

	if (!query.exec()) {
		s_lastError = query.lastError().text();
		return false;
	}

	while (query.next()) {
		TableRowData row;
		row.idCapture = query.value(0).toString();
		row.idBateau = query.value(1).toInt();
		row.typePoisson = query.value(2).toString();
		row.quantite = query.value(3).toInt();
		row.poids = query.value(4).toDouble();
		const int dateIndex = hasTempCol ? 6 : 5;
		if (hasTempCol) {
			const QVariant tempV = query.value(5);
			if (!tempV.isNull()) {
				row.temperatureC = tempV.toDouble();
				row.hasTemperature = true;
			}
		}
		row.dateCapture = query.value(dateIndex).toDate();
		rows.push_back(row);
	}

	s_lastError.clear();
	return true;
}

bool captures::chargerTableAvancee(const QString& rechercheId,
								  int quantiteFiltre,
								  const QDate& dateFiltre,
								  QVector<TableRowData>& rows)
{
	rows.clear();

	QSqlDatabase db = QSqlDatabase::database();
	QString tableName;
	QString idCol;
	QString idBateauCol;
	QString typeCol;
	QString quantiteCol;
	QString poidsCol;
	QString dateCol;
	QString temperatureCol;
	if (!resolveColumns(db, &tableName, &idCol, &idBateauCol, &typeCol, &quantiteCol, &poidsCol, &dateCol, &temperatureCol)) {
		return false;
	}

	const bool hasTempCol = !temperatureCol.isEmpty();
	QStringList selectCols = { idCol, idBateauCol, typeCol, quantiteCol, poidsCol };
	if (hasTempCol) {
		selectCols << temperatureCol;
	}
	selectCols << dateCol;

	QString sql = QStringLiteral("SELECT %1 FROM %2")
				  .arg(selectCols.join(QStringLiteral(", ")), tableName);

	QStringList where;
	QList<QVariant> binds;
	
	// Filtre par ID
	if (!rechercheId.trimmed().isEmpty()) {
		where << QStringLiteral("UPPER(%1) LIKE ?").arg(idCol);
		binds << QStringLiteral("%") + rechercheId.trimmed().toUpper() + QStringLiteral("%");
	}

	// Filtre par quantité exacte
	if (quantiteFiltre > 0) {
		where << QStringLiteral("%1 = ?").arg(quantiteCol);
		binds << quantiteFiltre;
	}

	// Filtre par date exacte
	if (dateFiltre.isValid()) {
		where << QStringLiteral("TRUNC(%1) = TO_DATE(?, 'YYYY-MM-DD')").arg(dateCol);
		binds << dateFiltre.toString(QStringLiteral("yyyy-MM-dd"));
	}

	if (!where.isEmpty()) {
		sql += QStringLiteral(" WHERE ") + where.join(QStringLiteral(" AND "));
	}
	sql += QStringLiteral(" ORDER BY %1 DESC").arg(dateCol);

	QSqlQuery query(db);
	query.prepare(sql);
	for (const QVariant& v : binds) {
		query.addBindValue(v);
	}

	if (!query.exec()) {
		s_lastError = query.lastError().text();
		return false;
	}

	while (query.next()) {
		TableRowData row;
		row.idCapture = query.value(0).toString();
		row.idBateau = query.value(1).toInt();
		row.typePoisson = query.value(2).toString();
		row.quantite = query.value(3).toInt();
		row.poids = query.value(4).toDouble();
		const int dateIndex = hasTempCol ? 6 : 5;
		if (hasTempCol) {
			const QVariant tempV = query.value(5);
			if (!tempV.isNull()) {
				row.temperatureC = tempV.toDouble();
				row.hasTemperature = true;
			}
		}
		row.dateCapture = query.value(dateIndex).toDate();
		rows.push_back(row);
	}

	s_lastError.clear();
	return true;
}

QVector<captures::SpeciesStatData> captures::calculerTop5Species()
{
	QVector<SpeciesStatData> result;

	QSqlDatabase db = QSqlDatabase::database();
	QString tableName;
	QString idCol;
	QString idBateauCol;
	QString typeCol;
	QString quantiteCol;
	QString poidsCol;
	QString dateCol;
	if (!resolveColumns(db, &tableName, &idCol, &idBateauCol, &typeCol, &quantiteCol, &poidsCol, &dateCol)) {
		s_lastError = QStringLiteral("Impossible de résoudre les colonnes.");
		return result;
	}

	// Requête groupée par type de poisson pour obtenir les totaux
	QString sql = QStringLiteral("SELECT %1, SUM(%2) as quantite_totale FROM %3 GROUP BY %1 ORDER BY quantite_totale DESC")
					  .arg(typeCol, quantiteCol, tableName);

	QSqlQuery query(db);
	if (!query.exec(sql)) {
		s_lastError = query.lastError().text();
		return result;
	}

	QVector<QPair<QString, int>> allSpecies;
	int totalQuantite = 0;

	while (query.next()) {
		const QString species = query.value(0).toString().trimmed();
		const int quantite = query.value(1).toInt();
		if (!species.isEmpty()) {
			allSpecies.push_back({ species, quantite });
			totalQuantite += quantite;
		}
	}

	// Garder seulement les 5 premiers
	const int top5Count = qMin(5, allSpecies.size());
	for (int i = 0; i < top5Count; ++i) {
		SpeciesStatData stat;
		stat.species = allSpecies[i].first;
		stat.quantite = allSpecies[i].second;
		stat.pourcentage = (totalQuantite > 0)
			? (100.0 * static_cast<double>(allSpecies[i].second) / static_cast<double>(totalQuantite))
			: 0.0;
		result.push_back(stat);
	}

	s_lastError.clear();
	return result;
}

QSqlQueryModel* captures::afficher()
{
	auto* model = new QSqlQueryModel();

	QSqlDatabase db = QSqlDatabase::database();
	QString tableName;
	QString idCol;
	QString idBateauCol;
	QString typeCol;
	QString quantiteCol;
	QString poidsCol;
	QString dateCol;
	if (!resolveColumns(db, &tableName, &idCol, &idBateauCol, &typeCol, &quantiteCol, &poidsCol, &dateCol)) {
		return model;
	}

	model->setQuery(QStringLiteral("SELECT * FROM %1 ORDER BY %2 DESC").arg(tableName, dateCol), db);
	return model;
}

QString captures::lastError()
{
	return s_lastError;
}
