#include <QCoreApplication>
#include <QDebug>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStringList>

static QString normalizeKey(QString text)
{
    text = text.normalized(QString::NormalizationForm_D).toLower();
    static const QRegularExpression diacRx(QStringLiteral("\\p{Mn}+"));
    text.remove(diacRx);
    text.remove(QLatin1Char('_'));
    text.remove(QLatin1Char(' '));
    return text;
}

static QString normalizeRfidKey(QString text)
{
    text = text.trimmed().toUpper();
    text.remove(QRegularExpression(QStringLiteral("[^0-9A-Z]+")));
    return text;
}

static QString matchColumn(const QStringList& dbCols, const QStringList& synonyms)
{
    for (const QString& syn : synonyms) {
        const QString skey = normalizeKey(syn);
        for (const QString& col : dbCols) {
            if (normalizeKey(col) == skey) {
                return col;
            }
        }
    }

    for (const QString& syn : synonyms) {
        const QString skey = normalizeKey(syn);
        for (const QString& col : dbCols) {
            if (normalizeKey(col).startsWith(skey)) {
                return col;
            }
        }
    }

    return {};
}

static QString sqlNormalizedRfidExpr(const QString& column)
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

    for (const QString& ch : charsToStrip) {
        expr = QStringLiteral("REPLACE(%1, '%2', '')").arg(expr, ch);
    }

    return expr;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QString inputUid = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QStringLiteral("C3 FC 05 12");
    const QString rfidKey = normalizeRfidKey(inputUid);

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QODBC"));
    db.setDatabaseName(QStringLiteral("DRIVER={Oracle in XE};DBQ=localhost:1521/XE;UID=nour1;PWD=nour123;"));

    if (!db.open()) {
        qCritical().noquote() << "OPEN_FAIL" << db.lastError().text();
        return 1;
    }

    qDebug().noquote() << "OPEN_OK driver=" << db.driverName();
    const QStringList tables = db.tables(QSql::Tables);
    qDebug().noquote() << "TABLES" << tables.join(", ");

    QString tableName;
    const QStringList candidates = {
        QStringLiteral("EMPLOYES"),
        QStringLiteral("EMPLOYE"),
        QStringLiteral("EMPLOYEE"),
        QStringLiteral("EMPLOYEES"),
        QStringLiteral("T_EMPLOYE")
    };
    for (const QString& cand : candidates) {
        const QString ckey = normalizeKey(cand);
        for (const QString& tbl : tables) {
            if (normalizeKey(tbl) == ckey) {
                tableName = tbl;
                break;
            }
        }
        if (!tableName.isEmpty()) break;
    }

    if (tableName.isEmpty()) {
        qCritical() << "NO_EMPLOYE_TABLE";
        return 2;
    }

    const QSqlRecord rec = db.record(tableName);
    QStringList cols;
    for (int i = 0; i < rec.count(); ++i) {
        cols << rec.fieldName(i);
    }
    qDebug().noquote() << "EMP_TABLE" << tableName;
    qDebug().noquote() << "COLS" << cols.join(", ");

    const QString idCol = matchColumn(cols, {QStringLiteral("ID_EMPLOYE"), QStringLiteral("ID"), QStringLiteral("IDEMPLOYE"), QStringLiteral("EMPLOYEEID")});
    const QString statutCol = matchColumn(cols, {QStringLiteral("STATUT"), QStringLiteral("STATUS"), QStringLiteral("DISPONIBILITE"), QStringLiteral("DISPO"), QStringLiteral("STATUT_EMPLOYE")});
    const QString etatCol = matchColumn(cols, {QStringLiteral("ETAT"), QStringLiteral("STATE"), QStringLiteral("ETAT_EMPLOYE")});
    const QString rfidCol = matchColumn(cols, {QStringLiteral("RFID_ID"), QStringLiteral("RFID"), QStringLiteral("CARD_ID")});
    const QString nomCol = matchColumn(cols, {QStringLiteral("NOM_EMPLOYE"), QStringLiteral("NOM"), QStringLiteral("NAME"), QStringLiteral("LASTNAME")});
    const QString prenomCol = matchColumn(cols, {QStringLiteral("PRENOM_EMPLOYE"), QStringLiteral("PRENOM"), QStringLiteral("FIRSTNAME")});

    qDebug().noquote() << "MATCH id=" << idCol << "statut=" << statutCol << "etat=" << etatCol << "rfid=" << rfidCol;

    if (idCol.isEmpty() || statutCol.isEmpty() || rfidCol.isEmpty()) {
        qCritical() << "MISSING_REQUIRED_COLUMNS";
        return 3;
    }

    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT %1, %2, %3, %4, %5, %6 "
        "FROM %7 WHERE %8 = :rfid")
                      .arg(idCol,
                           nomCol.isEmpty() ? QStringLiteral("''") : nomCol,
                           prenomCol.isEmpty() ? QStringLiteral("''") : prenomCol,
                           statutCol,
                           etatCol.isEmpty() ? QStringLiteral("''") : etatCol,
                           rfidCol,
                           tableName,
                           sqlNormalizedRfidExpr(rfidCol)));
    query.bindValue(QStringLiteral(":rfid"), rfidKey);

    if (!query.exec()) {
        qCritical().noquote() << "SELECT_FAIL" << query.lastError().text();
        return 4;
    }

    if (!query.next()) {
        qCritical().noquote() << "RFID_NOT_FOUND" << rfidKey;
        return 5;
    }

    const QVariant empId = query.value(0);
    const QString nom = query.value(1).toString();
    const QString prenom = query.value(2).toString();
    const QString statutBefore = query.value(3).toString();
    const QString etat = query.value(4).toString();
    const QString rfidDb = query.value(5).toString();

    qDebug().noquote() << "FOUND id=" << empId.toString()
                       << "nom=" << nom
                       << "prenom=" << prenom
                       << "statut=" << statutBefore
                       << "etat=" << etat
                       << "rfid=" << rfidDb;

    const QString beforeKey = normalizeKey(statutBefore);
    const QString statutNext = beforeKey.contains(QStringLiteral("indispon"))
        ? QStringLiteral("Disponible")
        : QStringLiteral("Indisponible");

    QSqlQuery update(db);
    update.prepare(QStringLiteral("UPDATE %1 SET %2 = :statut WHERE %3 = :id")
                       .arg(tableName, statutCol, idCol));
    update.bindValue(QStringLiteral(":statut"), statutNext);
    update.bindValue(QStringLiteral(":id"), empId);

    if (!update.exec()) {
        qCritical().noquote() << "UPDATE_FAIL" << update.lastError().text();
        return 6;
    }

    qDebug().noquote() << "UPDATED rows=" << update.numRowsAffected() << "target=" << statutNext;

    QSqlQuery commit(db);
    if (!commit.exec(QStringLiteral("COMMIT"))) {
        qCritical().noquote() << "COMMIT_FAIL" << commit.lastError().text();
        return 7;
    }

    QSqlQuery verify(db);
    verify.prepare(QStringLiteral("SELECT %1, %2 FROM %3 WHERE %4 = :id")
                       .arg(statutCol, etatCol.isEmpty() ? QStringLiteral("''") : etatCol, tableName, idCol));
    verify.bindValue(QStringLiteral(":id"), empId);
    if (!verify.exec() || !verify.next()) {
        qCritical().noquote() << "VERIFY_FAIL" << verify.lastError().text();
        return 8;
    }

    qDebug().noquote() << "AFTER statut=" << verify.value(0).toString() << "etat=" << verify.value(1).toString();
    return 0;
}
