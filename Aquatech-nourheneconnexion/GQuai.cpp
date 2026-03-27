#include "GQuai.h"
#include "ui_mainwindow.h"
#include <QAbstractButton>
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QDate>
#include <QMap>
#include <QPair>
#include <algorithm>

#include <QDateTime>
#include <QMessageBox>
#include <QMetaType>
#include <QStringList>
#include <QList>
// Database
#include "connection.h"
#include "captures.h"
#include "quotas.h"
#include "actiondelegate.h"
#include "actionswidget.h"
#include <QSqlQuery>
#include <QSqlError>

// Toggle global: activer/désactiver les opérations CRUD.
// Change to true for the application to actually insert/update/delete.
static const bool kCrudEnabled = true;

static inline bool handleCrudDisabled(QWidget* parent)
{
    if (kCrudEnabled) return true;
    QMessageBox::information(parent, QStringLiteral("CRUD désactivé"), QStringLiteral("Les opérations CRUD sont désactivées dans cette build."));
    return false;
}

static bool tryToLongLong(const QVariant& v, qlonglong& out)
{
    if (!v.isValid() || v.isNull()) {
        return false;
    }

    switch (v.typeId()) {
    case QMetaType::Int:
    case QMetaType::LongLong:
    case QMetaType::UInt:
    case QMetaType::ULongLong:
    case QMetaType::Short:
    case QMetaType::UShort:
    case QMetaType::Char:
    case QMetaType::UChar: {
        bool ok = false;
        out = v.toLongLong(&ok);
        return ok;
    }
    case QMetaType::QString: {
        bool ok = false;
        out = v.toString().trimmed().toLongLong(&ok);
        return ok;
    }
    case QMetaType::QByteArray: {
        const QByteArray ba = v.toByteArray().trimmed();
        bool ok = false;
        out = QString::fromLatin1(ba).trimmed().toLongLong(&ok);
        if (ok) return true;
        out = QString::fromUtf8(ba).trimmed().toLongLong(&ok);
        return ok;
    }
    default: {
        bool ok = false;
        out = v.toLongLong(&ok);
        return ok;
    }
    }
}

static void debugBoundValues(const QSqlQuery& q, const QString& context)
{
    const QStringList names = q.boundValueNames();
    if (!names.isEmpty()) {
        qDebug().noquote() << context << "boundValues(named):";
        for (const QString& name : names) {
            const QVariant v = q.boundValue(name);
            qDebug().noquote() << "  " << name
                               << "typeId=" << v.typeId()
                               << "typeName=" << (v.metaType().name() ? v.metaType().name() : "<null>")
                               << "isNull=" << v.isNull()
                               << "value=" << v.toString();
        }
        return;
    }

    const QVariantList values = q.boundValues();
    if (values.isEmpty()) {
        return;
    }

    qDebug().noquote() << context << "boundValues(positional):";
    for (qsizetype i = 0; i < values.size(); ++i) {
        const QVariant& v = values.at(i);
        qDebug().noquote() << "  #" << i
                           << "typeId=" << v.typeId()
                           << "typeName=" << (v.metaType().name() ? v.metaType().name() : "<null>")
                           << "isNull=" << v.isNull()
                           << "value=" << v.toString();
    }
}

static bool loadBoatsIntoComboBox(QComboBox *combo)
{
    if (!combo) {
        return false;
    }

    combo->clear();
    combo->addItem(QStringLiteral("— Aucun bateau —"), QVariant());

    QSqlQuery boatsQuery;
    const QString boatsSqlDefault = QStringLiteral("SELECT ID_BATEAU, NOM_BATEAU FROM BATEAUX ORDER BY ID_BATEAU");
    const QString boatsSqlHr = QStringLiteral("SELECT ID_BATEAU, NOM_BATEAU FROM HR.BATEAUX ORDER BY ID_BATEAU");

    bool ok = false;
    if (boatsQuery.exec(boatsSqlDefault)) {
        ok = true;
    } else {
        qDebug() << "loadBoatsIntoComboBox BATEAUX failed:" << boatsQuery.lastError().text()
                 << "query=" << boatsSqlDefault;
        if (boatsQuery.exec(boatsSqlHr)) {
            ok = true;
        } else {
            qDebug() << "loadBoatsIntoComboBox HR.BATEAUX failed:" << boatsQuery.lastError().text()
                     << "query=" << boatsSqlHr;
        }
    }

    if (!ok) {
        return false;
    }

    while (boatsQuery.next()) {
        const QVariant idBateau = boatsQuery.value(0);
        const QString nomBateau = boatsQuery.value(1).toString().trimmed();
        const QString label = nomBateau.isEmpty()
            ? idBateau.toString()
            : QStringLiteral("%1 (%2)").arg(nomBateau, idBateau.toString());

        // Oracle via ODBC may deliver NUMBER columns as QByteArray (Binary).
        // Store numeric IDs as integer in the combo's userData so later bindValue
        // sends a NUMBER, not BINARY.
        qlonglong idBateauLl = 0;
        const bool idOk = tryToLongLong(idBateau, idBateauLl);
        combo->addItem(label, idOk ? QVariant(idBateauLl) : QVariant());
    }

    return true;
}

static QString normalizeMojibake(QString text)
{
    if (text.isEmpty()) {
        return text;
    }

    // Remove soft hyphen that often appears in broken emoji prefixes.
    text.remove(QChar(0x00AD));

    // Fix common French mojibake (UTF-8 interpreted as CP850/CP437).
    text.replace(QStringLiteral("├®"), QStringLiteral("é"));
    text.replace(QStringLiteral("├¿"), QStringLiteral("è"));
    text.replace(QStringLiteral("├¬"), QStringLiteral("ê"));
    text.replace(QStringLiteral("├á"), QStringLiteral("à"));
    text.replace(QStringLiteral("├┤"), QStringLiteral("ô"));
    text.replace(QStringLiteral("├«"), QStringLiteral("î"));
    text.replace(QStringLiteral("├º"), QStringLiteral("ç"));

    // Fix a few frequent mixed-encoding sequences.
    text.replace(QStringLiteral("âž•"), QStringLiteral("➕"));
    text.replace(QStringLiteral("âš“"), QStringLiteral("⚓"));
    text.replace(QStringLiteral("OCCUP�?S"), QStringLiteral("OCCUPÉS"));
    text.replace(QStringLiteral("R�?PARTITION"), QStringLiteral("RÉPARTITION"));

    // Broken emojis (common sequences seen in the .ui).
    text.replace(QStringLiteral("ÔÜÖ´©Å"), QStringLiteral("⚙️"));
    text.replace(QStringLiteral("ÔØî"), QStringLiteral("❌"));
    text.replace(QStringLiteral("Ô£à"), QStringLiteral("✅"));
    text.replace(QStringLiteral("Ô£Å´©Å"), QStringLiteral("✅"));
    text.replace(QStringLiteral("ÔÅ│"), QStringLiteral("⏳"));
    text.replace(QStringLiteral("ÔùÅ"), QStringLiteral("•"));

    // Another family of broken emoji prefixes used in the .ui.
    text.replace(QStringLiteral("ƒöè"), QStringLiteral("🔊"));
    text.replace(QStringLiteral("ƒîÉ"), QStringLiteral("🌐"));
    text.replace(QStringLiteral("ƒôñ"), QStringLiteral("📄 ")); // Export PDF
    text.replace(QStringLiteral("ƒôê"), QStringLiteral("📊 ")); // Stats
    text.replace(QStringLiteral("ƒôï"), QStringLiteral("📋 ")); // Lists / filters
    text.replace(QStringLiteral("ƒôÀ"), QStringLiteral("📷 ")); // Camera
    text.replace(QStringLiteral("ƒô©"), QStringLiteral("📸 ")); // Capture
    text.replace(QStringLiteral("ƒöÉ"), QStringLiteral("🆔 ")); // FaceID
    text.replace(QStringLiteral("ƒöì"), QStringLiteral("🧬 ")); // Biometrics
    text.replace(QStringLiteral("ƒÆí"), QStringLiteral("💡 ")); // Hint
    text.replace(QStringLiteral("ƒöä"), QStringLiteral("🔁 ")); // Retry
    text.replace(QStringLiteral("ƒæÑ"), QStringLiteral("👥 ")); // All roles
    text.replace(QStringLiteral("ƒÅØ´©Å"), QStringLiteral("🏖️ ")); // Vacation
    text.replace(QStringLiteral("ƒòÁƒÅ╗"), QStringLiteral("✅ ")); // Face detected

    // Replace the common replacement-char based sequences used in some labels/buttons.
    text.replace(QStringLiteral("�Y-�️"), QStringLiteral("🗺️"));
    text.replace(QStringLiteral("�Y-�"), QStringLiteral("🗺️"));
    text.replace(QStringLiteral("�Y\"S"), QStringLiteral("📊"));
    text.replace(QStringLiteral("�Y\"\""), QStringLiteral("🔄"));
    text.replace(QStringLiteral("�YY�"), QStringLiteral("📍"));
    text.replace(QStringLiteral("�Y\"<"), QStringLiteral("📌"));
    text.replace(QStringLiteral("�Y\"^"), QStringLiteral("🧭"));
    text.replace(QStringLiteral("�Y\"�"), QStringLiteral("🤖"));
    text.replace(QStringLiteral("�Ys�"), QStringLiteral("✅"));
    text.replace(QStringLiteral("�.️"), QStringLiteral("⬅️"));
    text.replace(QStringLiteral("�o�️"), QStringLiteral("👁️"));

    // If garbage chars remain at the beginning, strip them.
    while (!text.isEmpty()) {
        const QChar ch = text.at(0);
        // Avoid non-ASCII character literals in single quotes (can trigger -Wmultichar on UTF-8 sources).
        // These are known mojibake prefix characters seen in some UI strings.
        if (ch == QChar(0xFFFD) || ch == QChar(u'\u00D4') /* U+00D4 */ || ch == QChar(u'\u0192') /* U+0192 */) {
            text.remove(0, 1);
            continue;
        }
        break;
    }

    return text;
}

static QString addEmojiByKeyword(QString text)
{
    // Don't touch if it already contains a known emoji we set.
    static const QStringList known = {
        QStringLiteral("⚙️"), QStringLiteral("📄"), QStringLiteral("🗺️"), QStringLiteral("📊"),
        QStringLiteral("🔄"), QStringLiteral("🤖"), QStringLiteral("📍"), QStringLiteral("📌"),
        QStringLiteral("✅"), QStringLiteral("❌"), QStringLiteral("👁️"), QStringLiteral("📷"),
        QStringLiteral("📸"), QStringLiteral("🆔"), QStringLiteral("⬅️"), QStringLiteral("⚓")
    };
    for (const auto& k : known) {
        if (text.contains(k)) {
            return text;
        }
    }

    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        return text;
    }

    // Add consistent emojis to common actions/titles.
    if (trimmed.contains(QStringLiteral("Exporter"), Qt::CaseInsensitive) && trimmed.contains(QStringLiteral("PDF"), Qt::CaseInsensitive)) {
        return QStringLiteral("📄 ") + trimmed;
    }
    if (trimmed.contains(QStringLiteral("Voir Map"), Qt::CaseInsensitive) || trimmed.contains(QStringLiteral("CARTE"), Qt::CaseInsensitive)) {
        return QStringLiteral("🗺️ ") + trimmed;
    }
    if (trimmed.contains(QStringLiteral("Statistiques"), Qt::CaseInsensitive)) {
        return QStringLiteral("📊 ") + trimmed;
    }
    if (trimmed.contains(QStringLiteral("Actualiser"), Qt::CaseInsensitive) || trimmed.contains(QStringLiteral("Rafraîchir"), Qt::CaseInsensitive)) {
        return QStringLiteral("🔄 ") + trimmed;
    }
    if (trimmed.contains(QStringLiteral("Analyser"), Qt::CaseInsensitive) && trimmed.contains(QStringLiteral("IA"), Qt::CaseInsensitive)) {
        return QStringLiteral("🤖 ") + trimmed;
    }
    if (trimmed.startsWith(QStringLiteral("Retour"), Qt::CaseInsensitive)) {
        return QStringLiteral("⬅️ ") + trimmed;
    }
    if (trimmed.contains(QStringLiteral("Caméra"), Qt::CaseInsensitive) || trimmed.contains(QStringLiteral("Camera"), Qt::CaseInsensitive)) {
        return QStringLiteral("📷 ") + trimmed;
    }
    if (trimmed.contains(QStringLiteral("Capturer"), Qt::CaseInsensitive) || trimmed.contains(QStringLiteral("Enregistrer Visage"), Qt::CaseInsensitive)) {
        return QStringLiteral("📸 ") + trimmed;
    }
    if (trimmed.contains(QStringLiteral("Reconnaissance Faciale"), Qt::CaseInsensitive) || trimmed.contains(QStringLiteral("FaceID"), Qt::CaseInsensitive)) {
        return QStringLiteral("🆔 ") + trimmed;
    }

    return text;
}

void MainWindow::normalizeUiTexts()
{
    auto fix = [](const QString& s) {
        return addEmojiByKeyword(normalizeMojibake(s));
    };

    setWindowTitle(fix(windowTitle()));

    const auto labels = findChildren<QLabel*>();
    for (auto* w : labels) {
        w->setText(fix(w->text()));
    }

    const auto buttons = findChildren<QAbstractButton*>();
    for (auto* w : buttons) {
        w->setText(fix(w->text()));
    }

    const auto groupBoxes = findChildren<QGroupBox*>();
    for (auto* w : groupBoxes) {
        w->setTitle(fix(w->title()));
    }

    const auto lineEdits = findChildren<QLineEdit*>();
    for (auto* w : lineEdits) {
        w->setPlaceholderText(fix(w->placeholderText()));
        // Only normalize the current text if it's not user-entered yet (common during startup).
        if (!w->text().isEmpty()) {
            w->setText(fix(w->text()));
        }
    }

    const auto comboBoxes = findChildren<QComboBox*>();
    for (auto* w : comboBoxes) {
        for (int i = 0; i < w->count(); ++i) {
            w->setItemText(i, fix(w->itemText(i)));
        }

        const QString current = fix(w->currentText());
        if (w->isEditable()) {
            w->setEditText(current);
        } else {
            w->setCurrentText(current);
        }
    }

    const auto tables = findChildren<QTableWidget*>();
    for (auto* w : tables) {
        for (int c = 0; c < w->columnCount(); ++c) {
            if (auto* item = w->horizontalHeaderItem(c)) {
                item->setText(fix(item->text()));
            }
        }
        for (int r = 0; r < w->rowCount(); ++r) {
            if (auto* item = w->verticalHeaderItem(r)) {
                item->setText(fix(item->text()));
            }
        }
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);



    if (auto btn = this->findChild<QPushButton*>(QStringLiteral("btnai"))) {
        btn->raise();
    }

    if (auto f = this->findChild<QWidget*>(QStringLiteral("frame_2c_2"))) {
        f->hide();
    }

    // Fix accents (é/è/…) + emojis on all pages.
    normalizeUiTexts();

    setupTop5SpeciesMetricControl();

    // Page pêcheurs : frame FaceID masquée par défaut
    if (ui->framefaceidp) {
        ui->framefaceidp->hide();
    }

    // Comportement d'origine GBateau : cacher certains panneaux au démarrage
    if (ui->frame_10b) {
        ui->frame_10b->hide();
    }
    // The alert panel widget may have been renamed in the .ui; look it up at runtime by name
    if (auto *frameAlert = this->findChild<QWidget*>(QStringLiteral("framealertb"))) {
        frameAlert->hide();
    }

     setupFrames();
    
    // Initialize QUOTAS table (create if it doesn't exist)
    initializeQuotasTable();
    
    // Page clients : laisser le panneau chatbot visible (si la page est affichée)

    // Démarrer sur le login GBateau (si présent), sinon sur le menu GBateau
    if (ui->stackedWidget) {
        if (ui->loginb) {
            ui->stackedWidget->setCurrentWidget(ui->loginb);
        } else if (ui->gestionsb) {
            ui->stackedWidget->setCurrentWidget(ui->gestionsb);
        }
    }

    // Connexions (page pêcheurs / FaceID)
    if (ui->brmp) {
        connect(ui->brmp, &QPushButton::clicked, this, &MainWindow::on_brmp_clicked);
    }
if (ui->pushButton_pdfb_5) {
    connect(ui->pushButton_pdfb_5, &QPushButton::clicked, this, &MainWindow::on_pushButton_pdfb_5_clicked);
}
    if (ui->btnFaceIDp) {
        connect(ui->btnFaceIDp, &QPushButton::clicked, this, &MainWindow::on_btnFaceIDp_clicked);
    }
    if (ui->bmi_6p) {
        connect(ui->bmi_6p, &QPushButton::clicked, this, &MainWindow::on_bmi_6p_clicked);
    }
    if (ui->pushButton_11p) {
        connect(ui->pushButton_11p, &QPushButton::clicked, this, &MainWindow::on_pushButton_11p_clicked);
    }

}

void MainWindow::setupTop5SpeciesMetricControl()
{
    if (!ui || !ui->label_zonesb_2) {
        return;
    }

    if (m_top5MetricCombo) {
        return;
    }

    QWidget* parent = ui->label_zonesb_2->parentWidget();
    if (!parent || !parent->layout()) {
        return;
    }

    auto* vbox = qobject_cast<QVBoxLayout*>(parent->layout());
    if (!vbox) {
        return;
    }

    const int idx = vbox->indexOf(ui->label_zonesb_2);
    if (idx < 0) {
        return;
    }

    vbox->removeWidget(ui->label_zonesb_2);

    QWidget* headerRow = new QWidget(parent);
    auto* hbox = new QHBoxLayout(headerRow);
    hbox->setContentsMargins(0, 0, 0, 0);

    m_top5MetricCombo = new QComboBox(headerRow);
    m_top5MetricCombo->addItem(QStringLiteral("Quantité"), QStringLiteral("qty"));
    m_top5MetricCombo->addItem(QStringLiteral("Poids"), QStringLiteral("poids"));
    m_top5MetricCombo->setMaximumWidth(130);

    hbox->addStretch(1);
    hbox->addWidget(ui->label_zonesb_2);
    hbox->addStretch(1);
    hbox->addWidget(m_top5MetricCombo);

    vbox->insertWidget(idx, headerRow);

    connect(m_top5MetricCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        updateTop5SpeciesStats(m_lastCaptureRows);
    });
}
// Fonction pour afficher une frame et cacher l'autre
void MainWindow::showFrame(QWidget* frameToShow)
{
    if (!ui) return;

    // Frames à gérer
    QWidget* frameb_3b = ui->frame_3b;
    QWidget* frameb = ui->frameb;

    if (frameToShow == frameb_3b) {
        frameb_3b->show();
        frameb_3b->raise();
        frameb->hide();
    } else if (frameToShow == frameb) {
        frameb->show();
        frameb->raise();
        frameb_3b->hide();
    }

    // Always keep the small overlay frame above both login/QR frames.

}


// Au démarrage, cacher le frame QR
void MainWindow::setupFrames()
{
    if (ui->frame_3b) ui->frame_3b->hide();        // Frame QR caché
    ui->frameb->show(); // Frame Connexion visible si tu veux
}

void MainWindow::initializeQuotasTable()
{
    // Create QUOTAS table if it doesn't exist
    // This function is called at startup to ensure the table exists and has data
    
    QSqlQuery query;
    
    // Step 1: Try to create the QUOTAS table
    // Using CREATE TABLE IF NOT EXISTS (Oracle 11g+)
    query.prepare(
        "BEGIN "
        "  BEGIN "
        "    CREATE TABLE \"QUOTAS\" ( "
        "      \"TYPE_POISSON\" VARCHAR2(50) PRIMARY KEY, "
        "      \"QUOTA\" NUMBER(10,2) NOT NULL "
        "    ); "
        "  EXCEPTION "
        "    WHEN OTHERS THEN "
        "      NULL; "
        "  END; "
        "END; "
    );
    
    if (!query.exec()) {
        qWarning() << "Could not create QUOTAS table:" << query.lastError().text();
        // Continue anyway - table might already exist
    }
    
    QSqlDatabase::database().commit();
    
    // Step 2: Check if table has data
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM \"QUOTAS\"");
    
    bool hasData = false;
    if (checkQuery.exec() && checkQuery.next()) {
        int count = checkQuery.value(0).toInt();
        hasData = (count > 0);
    }
    
    // Step 3: Insert default data if table is empty
    if (!hasData) {
        QSqlQuery insertQuery;
        
        // Define default quotas for all fish types
        QList<QPair<int, QPair<QString, double>>> defaultQuotas;
        defaultQuotas.append(qMakePair(1, qMakePair(QString("Sardine"), 200.0)));
        defaultQuotas.append(qMakePair(2, qMakePair(QString("Maquereau"), 150.0)));
        defaultQuotas.append(qMakePair(3, qMakePair(QString("Merlu"), 180.0)));
        defaultQuotas.append(qMakePair(4, qMakePair(QString("Thon"), 300.0)));
        defaultQuotas.append(qMakePair(5, qMakePair(QString("Loup"), 120.0)));
        defaultQuotas.append(qMakePair(6, qMakePair(QString("Calamar"), 100.0)));
        defaultQuotas.append(qMakePair(7, qMakePair(QString("Crevette"), 80.0)));
        defaultQuotas.append(qMakePair(8, qMakePair(QString("Rouget"), 90.0)));
        defaultQuotas.append(qMakePair(9, qMakePair(QString("Poulpes"), 110.0)));
        
        // Insert default data
        for (const auto& quota : defaultQuotas) {
            QString fishType = quota.second.first;
            double value = quota.second.second;
            
            insertQuery.prepare(
                "INSERT INTO \"QUOTAS\" (\"TYPE_POISSON\", \"QUOTA\") "
                "VALUES (:fishType, :value)"
            );
            insertQuery.bindValue(":fishType", fishType);
            insertQuery.bindValue(":value", value);
            
            if (!insertQuery.exec()) {
                qWarning() << "Could not insert quota for" << fishType 
                          << ":" << insertQuery.lastError().text();
            }
        }
        
        QSqlDatabase::database().commit();
        qDebug() << "QUOTAS table initialized with default data";
    } else {
        qDebug() << "QUOTAS table already has data";
    }
}

// Bouton QR Code : afficher frame QR, cacher frame Connexion
void MainWindow::on_pushButton_b_3b_clicked()
{
    showFrame(ui->frame_3b);
}

// Bouton Connexion : afficher frame Connexion, cacher frame QR
void MainWindow::on_pushButton_b_2b_clicked()
{
    showFrame(ui->frameb);
}
void MainWindow::on_p4b_clicked()
{
    // Depuis le menu GBateau : ouvrir la gestion des quais
    if (ui->stackedWidget && ui->page_3) {
        ui->stackedWidget->setCurrentWidget(ui->page_3);
    }

    // loadQuais(); (removed)
}

void MainWindow::on_p6b_clicked()
{
    // Depuis le menu GBateau : ouvrir la gestion des employés
    if (ui->stackedWidget && ui->pagee) {
        ui->stackedWidget->setCurrentWidget(ui->pagee);
    }

    // loadEmployes(); (removed)
}

void MainWindow::on_p5b_clicked()
{
    if (!ui || !ui->stackedWidget || !ui->cap_pagecaptures) {
        return;
    }

    ui->stackedWidget->setCurrentWidget(ui->cap_pagecaptures);
    
    // Load captures and quotas tables when navigating to captures page
    loadCapturesTable();
    loadQuotasTable();
}
void MainWindow::on_pushButton_3b_50_clicked(){
    if (ui && ui->stackedWidget && ui->cap_pagecaptures) {
        ui->stackedWidget->setCurrentWidget(ui->cap_pagecaptures);
    }
}
void MainWindow::on_pushButton_3b_7_clicked(){
    if (ui && ui->stackedWidget && ui->cap_pagecaptures) {
        ui->stackedWidget->setCurrentWidget(ui->cap_pagecaptures);
    }
}
    void MainWindow::on_pushButton_3b_19_clicked(){
    if (ui && ui->stackedWidget && ui->cap_pagecaptures) {
        ui->stackedWidget->setCurrentWidget(ui->cap_pagecaptures);
    }
}
    void MainWindow::on_pushButton_3b_56_clicked(){
        if (ui && ui->stackedWidget && ui->cap_pagecaptures) {
            ui->stackedWidget->setCurrentWidget(ui->cap_pagecaptures);
        }
    }
    void MainWindow::on_pushButton_3b_13_clicked(){
        if (ui && ui->stackedWidget && ui->cap_pagecaptures) {
            ui->stackedWidget->setCurrentWidget(ui->cap_pagecaptures);
        }
    }
    void MainWindow::on_pushButton_3b_44_clicked(){
        if (ui && ui->stackedWidget && ui->cap_pagecaptures) {
            ui->stackedWidget->setCurrentWidget(ui->cap_pagecaptures);
        }
    }
    void MainWindow::on_pushButton_3b_55_clicked(){
    if (ui && ui->stackedWidget && ui->pageclients) {
        ui->stackedWidget->setCurrentWidget(ui->pageclients);
    }
}
    void MainWindow::on_pushButton_3b_18_clicked(){
        if (ui && ui->stackedWidget && ui->pageclients) {
            ui->stackedWidget->setCurrentWidget(ui->pageclients);
        }
    }
    void MainWindow::on_pushButton_3b_12_clicked(){
        if (ui && ui->stackedWidget && ui->pageclients) {
            ui->stackedWidget->setCurrentWidget(ui->pageclients);
        }
    }
    void MainWindow::on_pushButton_3b_49_clicked(){
        if (ui && ui->stackedWidget && ui->pageclients) {
            ui->stackedWidget->setCurrentWidget(ui->pageclients);
        }
    }
    void MainWindow::on_pushButton_3b_6_clicked(){
        if (ui && ui->stackedWidget && ui->pageclients) {
            ui->stackedWidget->setCurrentWidget(ui->pageclients);
        }
    }
    void MainWindow::on_pushButton_3b_43_clicked(){
        if (ui && ui->stackedWidget && ui->pageclients) {
            ui->stackedWidget->setCurrentWidget(ui->pageclients);
        }
    }
void MainWindow::on_pushButton_3b_17_clicked(){
    if (ui && ui->stackedWidget && ui->pagee) {
        ui->stackedWidget->setCurrentWidget(ui->pagee);
    }
}
void MainWindow::on_pushButton_3b_54_clicked(){
    if (ui && ui->stackedWidget && ui->pagee) {
        ui->stackedWidget->setCurrentWidget(ui->pagee);
    }
}
void MainWindow::on_pushButton_3b_48_clicked(){
    if (ui && ui->stackedWidget && ui->pagee) {
        ui->stackedWidget->setCurrentWidget(ui->pagee);
    }
}
void MainWindow::on_pushButton_3b_5_clicked(){
    if (ui && ui->stackedWidget && ui->pagee) {
        ui->stackedWidget->setCurrentWidget(ui->pagee);
    }
}
void MainWindow::on_pushButton_3b_11_clicked(){
    if (ui && ui->stackedWidget && ui->pagee) {
        ui->stackedWidget->setCurrentWidget(ui->pagee);
    }
}
void MainWindow::on_pushButton_3b_42_clicked(){
    if (ui && ui->stackedWidget && ui->pagee) {
        ui->stackedWidget->setCurrentWidget(ui->pagee);
    }
}
void MainWindow::on_pushButton_3b_10_clicked(){
    if (ui && ui->stackedWidget && ui->page_3) {
        ui->stackedWidget->setCurrentWidget(ui->page_3);
    }
}
void MainWindow::on_pushButton_3b_16_clicked(){
    if (ui && ui->stackedWidget && ui->page_3) {
        ui->stackedWidget->setCurrentWidget(ui->page_3);
    }
}
void MainWindow::on_pushButton_3b_53_clicked(){
    if (ui && ui->stackedWidget && ui->page_3) {
        ui->stackedWidget->setCurrentWidget(ui->page_3);
    }
}
void MainWindow::on_pushButton_3b_41_clicked(){
    if (ui && ui->stackedWidget && ui->page_3) {
        ui->stackedWidget->setCurrentWidget(ui->page_3);
    }
}
void MainWindow::on_pushButton_3b_47_clicked(){
    if (ui && ui->stackedWidget && ui->page_3) {
        ui->stackedWidget->setCurrentWidget(ui->page_3);
    }
}
void MainWindow::on_pushButton_3b_4_clicked(){
    if (ui && ui->stackedWidget && ui->page_3) {
        ui->stackedWidget->setCurrentWidget(ui->page_3);
    }
}
void MainWindow::on_pushButton_3b_46_clicked(){
    if (ui && ui->stackedWidget && ui->pagepecheur) {
        ui->stackedWidget->setCurrentWidget(ui->pagepecheur);
    }
}
void MainWindow::on_pushButton_3b_3_clicked(){
    if (ui && ui->stackedWidget && ui->pagepecheur) {
        ui->stackedWidget->setCurrentWidget(ui->pagepecheur);
    }
}
void MainWindow::on_pushButton_3b_9_clicked(){
    if (ui && ui->stackedWidget && ui->pagepecheur) {
        ui->stackedWidget->setCurrentWidget(ui->pagepecheur);
    }
}
void MainWindow::on_pushButton_3b_15_clicked(){
    if (ui && ui->stackedWidget && ui->pagepecheur) {
        ui->stackedWidget->setCurrentWidget(ui->pagepecheur);
    }
}
void MainWindow::on_pushButton_3b_52_clicked(){
    if (ui && ui->stackedWidget && ui->pagepecheur) {
        ui->stackedWidget->setCurrentWidget(ui->pagepecheur);
    }
}
void MainWindow::on_pushButton_3b_14_clicked(){
    if (ui && ui->stackedWidget && ui->gestionbateaub) {
        ui->stackedWidget->setCurrentWidget(ui->gestionbateaub);
    }
}
    void MainWindow::on_pushButton_3b_40_clicked(){
    if (ui && ui->stackedWidget && ui->pagepecheur) {
        ui->stackedWidget->setCurrentWidget(ui->pagepecheur);
    }
}
    void MainWindow::on_pushButton_3b_39_clicked(){
    if (ui && ui->stackedWidget && ui->gestionbateaub) {
        ui->stackedWidget->setCurrentWidget(ui->gestionbateaub);
    }
}
    void MainWindow::on_pushButton_3b_51_clicked(){
    if (ui && ui->stackedWidget && ui->gestionbateaub) {
        ui->stackedWidget->setCurrentWidget(ui->gestionbateaub);
    }
}
    void MainWindow::on_pushButton_3b_8_clicked(){
    if (ui && ui->stackedWidget && ui->gestionbateaub) {
        ui->stackedWidget->setCurrentWidget(ui->gestionbateaub);
    }
}
    void MainWindow::on_pushButton_3b_2_clicked(){
    if (ui && ui->stackedWidget && ui->gestionbateaub) {
        ui->stackedWidget->setCurrentWidget(ui->gestionbateaub);
    }
}
    void MainWindow::on_pushButton_3b_45_clicked(){
        if (ui && ui->stackedWidget && ui->gestionbateaub) {
            ui->stackedWidget->setCurrentWidget(ui->gestionbateaub);
        }
    }
void MainWindow::on_cap_btnBackMenu_clicked()
{
    if (ui && ui->stackedWidget && ui->gestionsb) {
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
    }
}

void MainWindow::on_cap_btnShowSurpeche_clicked()
{
    if (!ui || !ui->stackedWidget) {
        return;
    }

    // The Surpeche page was renamed in the .ui at some point; support both names.
    QWidget *surpechePage = ui->stackedWidget->findChild<QWidget*>(QStringLiteral("cap_btnShowSurpeche_2"));
    if (!surpechePage) {
        surpechePage = ui->stackedWidget->findChild<QWidget*>(QStringLiteral("cap_pagecaptures_surpeche"));
    }
    if (!surpechePage) {
        return;
    }

    ui->stackedWidget->setCurrentWidget(surpechePage);

    // Put focus on a meaningful control on the destination page.
    if (auto *fallback = surpechePage->findChild<QPushButton*>(QStringLiteral("cap_btnBackToCapturesMainSurpeche"))) {
        fallback->setFocus(Qt::FocusReason::OtherFocusReason);
    }
}

void MainWindow::on_cap_btnShowTendance_clicked()
{
    if (!ui || !ui->stackedWidget) {
        return;
    }

    QWidget *tendancePage = ui->stackedWidget->findChild<QWidget*>(QStringLiteral("cap_pagecaptures_tendance"));
    if (!tendancePage) {
        return;
    }

    ui->stackedWidget->setCurrentWidget(tendancePage);

    if (auto *fallback = tendancePage->findChild<QPushButton*>(QStringLiteral("cap_btnBackToCapturesMainTendance"))) {
        fallback->setFocus(Qt::FocusReason::OtherFocusReason);
    }
}

void MainWindow::on_cap_btnBackToCapturesMainSurpeche_clicked()
{
    if (ui && ui->stackedWidget && ui->cap_pagecaptures) {
        ui->stackedWidget->setCurrentWidget(ui->cap_pagecaptures);
    }
}

void MainWindow::on_cap_btnBackToCapturesMainTendance_clicked()
{
    if (ui && ui->stackedWidget && ui->cap_pagecaptures) {
        ui->stackedWidget->setCurrentWidget(ui->cap_pagecaptures);
    }
}

void MainWindow::loadCapturesTable()
{
    if (!ui || !ui->cap_tableWidget) {
        qWarning() << "loadCapturesTable: UI or cap_tableWidget is null";
        return;
    }

    QTableWidget *tbl = ui->cap_tableWidget;
    tbl->setRowCount(0);

    // Setup columns with Actions column
    if (tbl->columnCount() == 0) {
        tbl->setColumnCount(7); // ID, Bateau, Type, Qty, Poids, Date, Actions
        QStringList headers;
        headers << "ID Capture" << "ID Bateau" << "Type Poisson" << "Quantité" << "Poids" << "Date" << "Actions";
        tbl->setHorizontalHeaderLabels(headers);
        tbl->setColumnWidth(6, 280); // Wider actions column for 3 buttons
    }

    // Load data from database
    const QList<QStringList> rows = Captures::getAllCapturesAsRows();
    for (int i = 0; i < rows.size(); ++i) {
        tbl->insertRow(i);
        const QStringList &row = rows.at(i);
        
        // Add data columns (ID, Bateau, Type, Qty, Poids, Date)
        for (int col = 0; col < row.size() && col < 6; ++col) {
            QTableWidgetItem *item = new QTableWidgetItem(row.at(col));
            item->setFlags(item->flags() & ~Qt::ItemIsEditable); // Make read-only
            tbl->setItem(i, col, item);
        }
    }

    // Add action buttons for each row using setCellWidget (buttons always visible)
    for (int row = 0; row < tbl->rowCount(); ++row) {
        ActionsButtonWidget *buttonsWidget = new ActionsButtonWidget(row, tbl);
        tbl->setCellWidget(row, 6, buttonsWidget);
        
        // Connect button signals to CRUD slot handlers
        connect(buttonsWidget, &ActionsButtonWidget::editClicked, 
                this, &MainWindow::onCaptureEditClicked);
        connect(buttonsWidget, &ActionsButtonWidget::deleteClicked, 
                this, &MainWindow::onCaptureDeleteClicked);
        connect(buttonsWidget, &ActionsButtonWidget::refreshClicked, 
                this, &MainWindow::onCaptureRefreshClicked);
    }

    // Set column widths for better visibility
    tbl->setColumnWidth(0, 80);   // ID
    tbl->setColumnWidth(1, 80);   // ID Bateau
    tbl->setColumnWidth(2, 100);  // Type
    tbl->setColumnWidth(3, 80);   // Quantité
    tbl->setColumnWidth(4, 80);   // Poids
    tbl->setColumnWidth(5, 100);  // Date

    m_lastCaptureRows = rows;
    updateTop5SpeciesStats(m_lastCaptureRows);
}

void MainWindow::updateTop5SpeciesStats(const QList<QStringList>& rows)
{
    if (!ui) {
        return;
    }

    QLabel* nameLabels[5] = {
        ui->label_zoneNordb_2,
        ui->label_zoneSudb_2,
        ui->label_zoneEstb_2,
        ui->label_zoneOuestb_3,
        ui->label_zoneOuestb_2,
    };

    QProgressBar* bars[5] = {
        ui->progressZoneNordb_2,
        ui->progressZoneSudb_2,
        ui->progressZoneEstb_2,
        ui->progressZoneOuestb_3,
        ui->progressZoneOuestb_2,
    };

    QLabel* valueLabels[5] = {
        ui->value_zoneNordb_2,
        ui->value_zoneSudb_2,
        ui->value_zoneEstb_2,
        ui->value_zoneOuestb_3,
        ui->value_zoneOuestb_2,
    };

    for (int i = 0; i < 5; ++i) {
        if (bars[i]) {
            bars[i]->setRange(0, 100);
            bars[i]->setValue(0);
        }
        if (nameLabels[i]) {
            nameLabels[i]->setText(QString());
        }
        if (valueLabels[i]) {
            valueLabels[i]->setText(QStringLiteral("0/0"));
        }
    }

    const QString metric = (m_top5MetricCombo ? m_top5MetricCombo->currentData().toString() : QStringLiteral("qty"));
    const bool useWeight = (metric == QStringLiteral("poids"));

    auto parseQty = [](const QString& s) -> qlonglong {
        const QString t = s.trimmed();
        if (t.isEmpty()) return 0;
        bool ok = false;
        const qlonglong v = t.toLongLong(&ok);
        if (ok) return v;
        const double d = t.toDouble(&ok);
        return ok ? static_cast<qlonglong>(d) : 0;
    };

    auto parseDouble = [](const QString& s) -> double {
        const QString t = s.trimmed();
        if (t.isEmpty()) return 0.0;
        bool ok = false;
        const double v = t.toDouble(&ok);
        return ok ? v : 0.0;
    };

    struct Item { QString type; double value; };
    QList<Item> items;
    items.reserve(32);

    double total = 0.0;
    QMap<QString, double> sumByType;
    for (const QStringList& row : rows) {
        if (row.size() < 5) {
            continue;
        }
        const QString type = row.at(2).trimmed();
        if (type.isEmpty()) {
            continue;
        }

        const double v = useWeight ? parseDouble(row.at(4)) : static_cast<double>(parseQty(row.at(3)));
        total += v;
        sumByType[type] = sumByType.value(type) + v;
    }

    for (auto it = sumByType.constBegin(); it != sumByType.constEnd(); ++it) {
        items.append(Item{it.key(), it.value()});
    }
    std::sort(items.begin(), items.end(), [](const Item& a, const Item& b) {
        return a.value > b.value;
    });
    if (items.size() > 5) {
        items = items.mid(0, 5);
    }

    for (int i = 0; i < 5; ++i) {
        if (!bars[i] || !nameLabels[i] || !valueLabels[i]) {
            continue;
        }

        if (i < items.size()) {
            const Item& it = items.at(i);
            nameLabels[i]->setText(it.type);
            const int percent = (total > 0.0)
                ? qBound(0, static_cast<int>(qRound((it.value * 100.0) / total)), 100)
                : 0;
            bars[i]->setValue(percent);
            if (useWeight) {
                valueLabels[i]->setText(QStringLiteral("%1/%2")
                                            .arg(QString::number(it.value, 'f', 2),
                                                 QString::number(total, 'f', 2)));
            } else {
                valueLabels[i]->setText(QStringLiteral("%1/%2")
                                            .arg(QString::number(static_cast<qlonglong>(it.value)),
                                                 QString::number(static_cast<qlonglong>(total))));
            }
        } else {
            nameLabels[i]->setText(QString());
            bars[i]->setValue(0);
            if (useWeight) {
                valueLabels[i]->setText(QStringLiteral("0/%1").arg(QString::number(total, 'f', 2)));
            } else {
                valueLabels[i]->setText(QStringLiteral("0/%1").arg(QString::number(static_cast<qlonglong>(total))));
            }
        }
    }
}

void MainWindow::on_cap_btnValiider_3_clicked()
{
    if (!handleCrudDisabled(this))
        return;

    if (!ui) return;

    // basic validation for all form fields
    const int idCap = ui->cap_lineEdit_11->text().toInt();
    if (idCap <= 0) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("L'ID capture doit être un entier positif."));
        ui->cap_lineEdit_11->setFocus();
        return;
    }

    const QString idBatText = ui->cap_lineEdit_9->text().trimmed();
    bool idBatOk = false;
    const int idBat = idBatText.toInt(&idBatOk);
    qDebug().noquote() << "Capture add: idBatText='" + idBatText + "' idBatOk=" << idBatOk << "idBat=" << idBat;
    if (!idBatOk || idBat <= 0) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("L'ID bateau doit être un entier positif."));
        ui->cap_lineEdit_9->setFocus();
        return;
    }

    const QString type = ui->cap_comboBox_4->currentText().trimmed();
    if (type.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("Veuillez sélectionner un type de poisson."));
        ui->cap_comboBox_4->setFocus();
        return;
    }
    if (type.length() > 50) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("Le type de poisson est trop long."));
        ui->cap_comboBox_4->setFocus();
        return;
    }

    const QDate d = ui->cap_deDateCapture_3->date();
    if (!d.isValid()) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("Veuillez saisir une date valide."));
        ui->cap_deDateCapture_3->setFocus();
        return;
    }
    if (d > QDate::currentDate()) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("La date ne peut pas être dans le futur."));
        ui->cap_deDateCapture_3->setFocus();
        return;
    }

    const int qty = ui->cap_sbQuantite_4->value();
    qDebug() << "validation: qty=" << qty;
    if (qty <= 0) {
        QMessageBox::warning(this, QStringLiteral("Erreur"),
                             QStringLiteral("La quantité doit être strictement positive. (actuelle=%1)").arg(qty));
        ui->cap_sbQuantite_4->setFocus();
        return;
    }
    // optionally limit to realistic maximum (e.g. 1 million)
    if (qty > 1000000) {
        QMessageBox::warning(this, QStringLiteral("Erreur"),
                             QStringLiteral("La quantité est trop élevée. (actuelle=%1)").arg(qty));
        ui->cap_sbQuantite_4->setFocus();
        return;
    }

    const double w = ui->cap_dsPoids_3->value();
    qDebug() << "validation: weight=" << w;
    if (w <= 0.0) {
        QMessageBox::warning(this, QStringLiteral("Erreur"),
                             QStringLiteral("Le poids doit être strictement positif. (actuel=%1)").arg(w));
        ui->cap_dsPoids_3->setFocus();
        return;
    }
    if (w > 100000.0) {
        QMessageBox::warning(this, QStringLiteral("Erreur"),
                             QStringLiteral("Le poids est trop élevé. (actuel=%1)").arg(w));
        ui->cap_dsPoids_3->setFocus();
        return;
    }

    Captures c;
    c.idCapture = idCap;
    c.idBateau = idBat;
    c.typePoisson = type;
    c.quantite = qty;
    c.poids = w;
    c.dateCapture = ui->cap_deDateCapture_3->date();

    if (c.ajouter()) {
        QMessageBox::information(this, QStringLiteral("Succès"), QStringLiteral("Capture ajoutée avec succès."));
        // refresh table view
        loadCapturesTable();
    } else {
        QString errText = Captures::lastError();
        if (errText.isEmpty()) {
            // sometimes driver puts info in database().lastError()
            errText = QSqlDatabase::database().lastError().text();
        }
        QString queryText = Captures::lastQuery();
        QString message = QStringLiteral("Impossible d'ajouter la capture (%1)\n%2")
                          .arg(c.idCapture)
                          .arg(errText);
        if (!queryText.isEmpty()) {
            message += QStringLiteral("\nSQL: %1").arg(queryText);
        }
        QMessageBox::warning(this, QStringLiteral("Erreur"), message);
    }
}

void MainWindow::on_p1b_clicked()
{
    // Depuis le menu GBateau : ouvrir la gestion des bateaux
    if (ui->stackedWidget && ui->gestionbateaub) {
        ui->stackedWidget->setCurrentWidget(ui->gestionbateaub);
    }

    // loadBateaux(); (removed)
}

void MainWindow::on_p2b_clicked()
{
    // Depuis le menu GBateau : ouvrir la gestion des pêcheurs
    if (ui->stackedWidget && ui->pagepecheur) {
        ui->stackedWidget->setCurrentWidget(ui->pagepecheur);
    }

    // Par défaut, garder FaceID masqué à l'entrée de la page
    if (ui && ui->framefaceidp) {
        ui->framefaceidp->hide();
    }

    // Réafficher les widgets principaux (au cas où on revient depuis FaceID)
    if (ui) {
        if (ui->tableWidgetp) ui->tableWidgetp->setVisible(true);
        if (ui->label_3p) ui->label_3p->setVisible(true);
        if (ui->lineEdit_4p) ui->lineEdit_4p->setVisible(true);
        if (ui->label_2p) ui->label_2p->setVisible(true);
        if (ui->comboBox_5p) ui->comboBox_5p->setVisible(true);
        if (ui->label_4p) ui->label_4p->setVisible(true);
        if (ui->comboBox_6p) ui->comboBox_6p->setVisible(true);
        if (ui->frame_typesp) ui->frame_typesp->setVisible(true);
        if (ui->pushButton_4p) ui->pushButton_4p->setVisible(true);
        if (ui->pushButton_5p) ui->pushButton_5p->setVisible(true);
        if (ui->pushButton_6p) ui->pushButton_6p->setVisible(true);
    }

    // loadPecheurs(); (removed)
}

void MainWindow::on_p3b_clicked()
{
    // Depuis le menu GBateau : ouvrir la gestion des clients (intégrée dans mainwindow.ui)
    if (ui && ui->stackedWidget && ui->pageclients) {
        ui->stackedWidget->setCurrentWidget(ui->pageclients);

        // Garder le bouton IA visible au-dessus
        if (auto btn = this->findChild<QPushButton*>(QStringLiteral("btnai"))) {
            btn->raise();
        }
    }

    // loadClients(); (removed)
}

void MainWindow::on_btnai_clicked()
{
    if (!ui) return;
    if (auto f = this->findChild<QWidget*>(QStringLiteral("frame_2c_2"))) {
        f->setVisible(true);
        f->raise();
    }

    if (auto btn = this->findChild<QPushButton*>(QStringLiteral("btnai"))) {
        btn->raise();
    }
}

void MainWindow::on_btnSend_2_clicked()
{
    if (!ui) return;
    if (auto f = this->findChild<QWidget*>(QStringLiteral("frame_2c_2"))) {
        f->setVisible(false);
    }

    if (auto btn = this->findChild<QPushButton*>(QStringLiteral("btnai"))) {
        btn->raise();
    }
}
void MainWindow::on_pushButton_8c_3_clicked()
{
    // Retour Menu depuis la page clients -> menu GBateau
    if (ui && ui->stackedWidget && ui->page_7) {
        ui->stackedWidget->setCurrentWidget(ui->page_7);
    }
}
void MainWindow::on_pushButton_8c_4_clicked()
{
    // Retour Menu depuis la page clients -> menu GBateau
    if (ui && ui->stackedWidget && ui->page_7) {
        ui->stackedWidget->setCurrentWidget(ui->page_7);
    }
}
    void MainWindow::on_pushButton_7c_5_clicked()
{
    // Retour Menu depuis la page clients -> menu GBateau
    if (ui && ui->stackedWidget && ui->pageclients) {
        ui->stackedWidget->setCurrentWidget(ui->pageclients);
    }
}
void MainWindow::on_pushButton_7c_clicked()
{
    // Retour Menu depuis la page clients -> menu GBateau
    if (ui && ui->stackedWidget && ui->gestionsb) {
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
    }
}

static void setPecheurMainWidgetsVisible(Ui::MainWindow* ui, bool visible)
{
    if (!ui) return;
    if (ui->tableWidgetp) ui->tableWidgetp->setVisible(visible);
    if (ui->label_3p) ui->label_3p->setVisible(visible);
    if (ui->lineEdit_4p) ui->lineEdit_4p->setVisible(visible);
    if (ui->label_2p) ui->label_2p->setVisible(visible);
    if (ui->comboBox_5p) ui->comboBox_5p->setVisible(visible);
    if (ui->label_4p) ui->label_4p->setVisible(visible);
    if (ui->comboBox_6p) ui->comboBox_6p->setVisible(visible);
    if (ui->frame_typesp) ui->frame_typesp->setVisible(visible);
    if (ui->pushButton_4p) ui->pushButton_4p->setVisible(visible);
    if (ui->pushButton_5p) ui->pushButton_5p->setVisible(visible);
    if (ui->pushButton_6p) ui->pushButton_6p->setVisible(visible);
}

void MainWindow::on_btnFaceIDp_clicked()
{
    // Afficher FaceID, masquer les widgets principaux
    if (!ui) return;
    setPecheurMainWidgetsVisible(ui, false);
    if (ui->framefaceidp) {
        ui->framefaceidp->setVisible(true);
        ui->framefaceidp->raise();
    }
}

void MainWindow::on_brmp_clicked()
{
    // Retour Menu depuis pagepecheur -> menu GBateau
    if (!ui) return;

    // Remettre l'état normal côté pêcheurs
    if (ui->framefaceidp) ui->framefaceidp->hide();
    setPecheurMainWidgetsVisible(ui, true);

    if (ui->stackedWidget && ui->gestionsb) {
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
    }
}
void MainWindow::on_pushButton_pdfb_5_clicked()
{
    // Retour Menu depuis pagepecheur -> menu GBateau
    if (!ui) return;

    // Remettre l'état normal côté pêcheurs
    if (ui->framefaceidp) ui->framefaceidp->hide();
    setPecheurMainWidgetsVisible(ui, true);

    if (ui->stackedWidget && ui->gestionsb) {
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
    }
}

void MainWindow::on_bmi_6p_clicked()
{
    // Fermer FaceID, réafficher les widgets principaux
    if (!ui) return;
    if (ui->framefaceidp) ui->framefaceidp->setVisible(false);
    setPecheurMainWidgetsVisible(ui, true);
}

void MainWindow::on_pushButton_11p_clicked()
{
    // Annuler FaceID, réafficher les widgets principaux
    if (!ui) return;
    if (ui->framefaceidp) ui->framefaceidp->setVisible(false);
    setPecheurMainWidgetsVisible(ui, true);
}

void MainWindow::on_pushButton_b_clicked()
{
    // Connexion -> aller au menu GBateau
    if (ui->stackedWidget && ui->gestionsb) {
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
    }
}
void MainWindow::on_pushButton_b_4b_clicked()
{
    // Connexion -> aller au menu GBateau
    if (ui->stackedWidget && ui->gestionsb) {
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
    }
}

void MainWindow::on_pushButton_20b_clicked()
{
    // Mot de passe oublié ? -> page récupération (Rmdpb)
    if (ui->stackedWidget && ui->Rmdpb) {
        ui->stackedWidget->setCurrentWidget(ui->Rmdpb);
    }
}

void MainWindow::on_pushButton_13b_clicked()
{
    // Envoyer (récupération) -> page code email
    if (ui->stackedWidget && ui->Remailb) {
        ui->stackedWidget->setCurrentWidget(ui->Remailb);
    }
}

void MainWindow::on_pushButton_14b_clicked()
{
    // Annuler -> retour login
    if (ui->stackedWidget && ui->loginb) {
        ui->stackedWidget->setCurrentWidget(ui->loginb);
    }
}

void MainWindow::on_pushButton_15b_clicked()
{
    // Verifier code -> nouveau mot de passe
    if (ui->stackedWidget && ui->Nmdpb) {
        ui->stackedWidget->setCurrentWidget(ui->Nmdpb);
    }
}

void MainWindow::on_pushButton_16b_clicked()
{
    // Valider nouveau mot de passe -> retour login
    if (ui->stackedWidget && ui->loginb) {
        ui->stackedWidget->setCurrentWidget(ui->loginb);
    }
}

void MainWindow::on_pushButton_17b_clicked()
{
    // Retour -> login
    if (ui->stackedWidget && ui->loginb) {
        ui->stackedWidget->setCurrentWidget(ui->loginb);
    }
}

void MainWindow::on_pushButton_18b_clicked()
{
    // Retour (GBateau) -> Rmdpb
    if (ui->stackedWidget && ui->Rmdpb) {
        ui->stackedWidget->setCurrentWidget(ui->Rmdpb);
    }
}

void MainWindow::on_pushButton_19b_clicked()
{
    // Retour (GBateau) -> Remailb
    if (ui->stackedWidget && ui->Remailb) {
        ui->stackedWidget->setCurrentWidget(ui->Remailb);
    }
}

void MainWindow::on_p1_2b_clicked()
{
    // Bouton "p1_2b" : afficher/masquer le frame_10b (comme dans GBateau)
    if (ui->frame_10b) {
        ui->frame_10b->setVisible(!ui->frame_10b->isVisible());
    }
}

void MainWindow::on_pushButton_12b_clicked()
{
    // Bouton "Fermer" dans le panneau paramètres : fermer frame_10b
    if (ui && ui->frame_10b) {
        ui->frame_10b->hide();
    }
}

void MainWindow::on_pushButton_9b_clicked()
{
    // Déconnexion : revenir à la page login
    if (ui && ui->stackedWidget && ui->loginb) {
        ui->stackedWidget->setCurrentWidget(ui->loginb);
    }
    if (ui && ui->frame_10b) {
        ui->frame_10b->hide();
    }
    if (auto *frameAlert = this->findChild<QWidget*>(QStringLiteral("framealertb"))) {
        frameAlert->hide();
    }
}

void MainWindow::on_pushButton_clicked()
{
    // Bouton "⬅️ Retour Menu" sur la page gestion des quais : revenir au menu GBateau
    if (ui->stackedWidget && ui->gestionsb) {
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
    }
}

void MainWindow::on_pushButton_10e_clicked()
{
    // Bouton "Retour Menu" sur la page gestion des employés : revenir au menu GBateau
    if (ui->stackedWidget && ui->gestionsb) {
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
    }
}

void MainWindow::on_pushButton_3b_clicked()
{
    // Bouton "Retour" sur la page gestionbateaub : revenir au menu GBateau
    if (ui->stackedWidget && ui->gestionsb) {
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
    }
}

void MainWindow::loadQuotasTable()
{
    if (!ui || !ui->cap_tableWidget_2) {
        qWarning() << "loadQuotasTable: UI or cap_tableWidget_2 is null";
        return;
    }

    QTableWidget *tbl = ui->cap_tableWidget_2;
    
    // IMPORTANT: NE PAS appeler setRowCount(0) - cela efface les noms des poissons!
    // Les noms des poissons sont définis dans l'UI (Sardine, Maquereau, etc.)
    // Nous garderons le nombre de rows tel quel, et remplissons seulement les cellules
    
    // Charger les quotas depuis la BD
    // Query en cherchant par TYPE_POISSON pour matcher avec les row headers
    QSqlQuery query;
    query.prepare("SELECT \"TYPE_POISSON\", \"QUOTA\" FROM QUOTAS ORDER BY \"TYPE_POISSON\"");
    
    // Créer une map pour accès rapide des quotas par type de poisson
    QMap<QString, double> quotasMap;
    
    if (query.exec()) {
        while (query.next()) {
            QString typePoisson = query.value(0).toString();
            double valeur = query.value(1).toDouble();
            quotasMap[typePoisson] = valeur;
        }
    }
    
    // Remplir le tableau avec les quotas
    // Itérer sur les rows qui existent déjà (définis dans l'UI)
    int rowCount = tbl->rowCount();
    
    for (int i = 0; i < rowCount; ++i) {
        // Récupérer le nom du poisson depuis le vertical header (row label)
        QString fishType = tbl->verticalHeaderItem(i)->text();
        
        // Chercher le quota correspondant dans la BD
        double quotaValue = 0.0;
        if (quotasMap.contains(fishType)) {
            quotaValue = quotasMap[fishType];
        }
        
        // Mettre à jour le tableau avec la valeur du quota
        QTableWidgetItem *item = new QTableWidgetItem(QString::number(quotaValue, 'f', 2));
        item->setFlags(item->flags() | Qt::ItemIsEditable);  // Éditable
        tbl->setItem(i, 0, item);
    }
}

void MainWindow::modifyQuotaRow(int row)
{
    if (!ui || !ui->cap_tableWidget_2) {
        return;
    }

    QTableWidget *tbl = ui->cap_tableWidget_2;
    
    // GetRowHeader (le nom du poisson)
    if (!tbl->verticalHeaderItem(row)) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("Impossible de trouver le type de poisson."));
        return;
    }
    
    QString fishType = tbl->verticalHeaderItem(row)->text();
    
    // Get the quota value from the table (colonne 0)
    QTableWidgetItem *item = tbl->item(row, 0);
    if (!item) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("Impossible de trouver la valeur de quota."));
        return;
    }

    QString quotaStr = item->text().trimmed();
    
    // Validation: vérifier que la chaîne n'est pas vide
    if (quotaStr.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), 
                            QStringLiteral("La valeur de quota ne peut pas être vide."));
        return;
    }
    
    // Gérer les séparateurs décimaux (vigule ou point)
    quotaStr.replace(QStringLiteral(","), QStringLiteral("."));
    
    // Convertir en double
    bool ok;
    double quota = quotaStr.toDouble(&ok);
    
    // Validation: vérifier que la conversion a fonctionné
    if (!ok) {
        QMessageBox::warning(this, QStringLiteral("Erreur de Saisie"), 
                            QStringLiteral("La valeur doit être un nombre (ex: 200 ou 200.50)"));
        return;
    }
    
    // Validation: vérifier que la valeur est positive
    if (quota < 0) {
        QMessageBox::warning(this, QStringLiteral("Erreur de Validation"), 
                            QStringLiteral("La valeur de quota doit être positive (supérieur à 0)."));
        return;
    }
    
    // Validation: vérifier que la valeur n'est pas trop grande (max 1 million kg par exemple)
    if (quota > 1000000) {
        QMessageBox::warning(this, QStringLiteral("Erreur de Validation"), 
                            QStringLiteral("La valeur du quota est trop élevée (max: 1000000)."));
        return;
    }
    
    // Update database
    // First, check if the quota exists
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM QUOTAS WHERE \"TYPE_POISSON\" = :fishType");
    query.bindValue(":fishType", fishType);
    
    bool quotaExists = false;
    if (query.exec() && query.next()) {
        quotaExists = (query.value(0).toInt() > 0);
    }
    
    if (quotaExists) {
        // Update existing quota
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE QUOTAS SET \"QUOTA\" = :quota WHERE \"TYPE_POISSON\" = :fishType");
        updateQuery.bindValue(":quota", quota);
        updateQuery.bindValue(":fishType", fishType);
        
        if (!updateQuery.exec()) {
            QString errMsg = updateQuery.lastError().text();
            QMessageBox::warning(this, QStringLiteral("Erreur Base de Données"), 
                                QStringLiteral("Impossible de modifier le quota: ") + errMsg);
            return;
        }
        QMessageBox::information(this, QStringLiteral("Succès"), 
                                QStringLiteral("Quota modifié avec succès!"));
    } else {
        // Create new quota entry
        QSqlQuery insertQuery;
        insertQuery.prepare("INSERT INTO QUOTAS (\"TYPE_POISSON\", \"QUOTA\") "
                           "VALUES (:fishType, :quota)");
        insertQuery.bindValue(":fishType", fishType);
        insertQuery.bindValue(":quota", quota);
        
        if (!insertQuery.exec()) {
            QString errMsg = insertQuery.lastError().text();
            QMessageBox::warning(this, QStringLiteral("Erreur Base de Données"), 
                                QStringLiteral("Impossible d'ajouter le quota: ") + errMsg);
            return;
        }
        QSqlDatabase::database().commit();
        QMessageBox::information(this, QStringLiteral("Succès"), 
                            QStringLiteral("Quota pour ") + fishType + QStringLiteral(" mis à jour avec succès."));
    }
}

void MainWindow::on_cap_btnModifyQuota_clicked()
{
    if (!ui || !ui->cap_tableWidget_2) {
        return;
    }

    QTableWidget *tbl = ui->cap_tableWidget_2;
    int currentRow = tbl->currentRow();
    
    if (currentRow < 0) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("Veuillez sélectionner une ligne à modifier."));
        return;
    }

    modifyQuotaRow(currentRow);
    // Reload the table to refresh
    loadQuotasTable();
}

void MainWindow::on_pushButton_7b_clicked()
{
    // Bouton "Paramètres des alertes" : afficher le panneau framealertb (lookup by name)
    if (auto *frameAlert = this->findChild<QWidget*>(QStringLiteral("framealertb"))) {
        if (frameAlert->isVisible()) {
            frameAlert->hide();
            return;
        }
        frameAlert->show();
        frameAlert->raise();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_6_clicked()
{
    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentWidget(ui->page_4);
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentWidget(ui->page_3);
    }
}

void MainWindow::on_pushButton_9_clicked()
{
    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentWidget(ui->page_4);
    }
}

// Return the shared Connection instance


// Close is a no-op wrapper (Connection singleton handles lifecycle)
void MainWindow::closeOracleConnection(Connection* conn)
{
    Q_UNUSED(conn);
    // Nothing to do: Connection singleton will be closed on app exit
}

// ========================================
// CRUD Action Button Slot Handlers
// ========================================

void MainWindow::onCaptureEditClicked(int row)
{
    editCaptureRow(row);
}

void MainWindow::onCaptureDeleteClicked(int row)
{
    deleteCaptureRow(row);
}

void MainWindow::onCaptureRefreshClicked(int row)
{
    refreshSingleCapture(row);
}

void MainWindow::editCaptureRow(int row)
{
    if (!ui || !ui->cap_tableWidget) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("Table not found"));
        return;
    }

    QTableWidget *tbl = ui->cap_tableWidget;
    if (row < 0 || row >= tbl->rowCount()) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("Invalid row"));
        return;
    }

    // Get ID_CAPTURE from first column
    QTableWidgetItem *idItem = tbl->item(row, 0);
    if (!idItem) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("Cannot find ID_CAPTURE"));
        return;
    }

    const int idCapture = idItem->text().toInt();

    // Read current values from selected row
    const QString idBateauText = (tbl->item(row, 1) ? tbl->item(row, 1)->text().trimmed() : QString());
    const QString typePoisson = (tbl->item(row, 2) ? tbl->item(row, 2)->text().trimmed() : QString());
    const int quantite = (tbl->item(row, 3) ? tbl->item(row, 3)->text().toInt() : 0);
    const double poids = (tbl->item(row, 4) ? tbl->item(row, 4)->text().toDouble() : 0.0);
    const QString dateText = (tbl->item(row, 5) ? tbl->item(row, 5)->text().trimmed() : QString());
    QDate captureDate = QDate::fromString(dateText, Qt::ISODate);
    if (!captureDate.isValid()) {
        captureDate = QDate::currentDate();
    }

    // Build small edit dialog
    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("Modifier la Capture"));
    dialog.setModal(true);

    QFormLayout *formLayout = new QFormLayout(&dialog);

    QLineEdit *idCaptureEdit = new QLineEdit(QString::number(idCapture), &dialog);
    idCaptureEdit->setReadOnly(true);

    QComboBox *boatCombo = new QComboBox(&dialog);
    loadBoatsIntoComboBox(boatCombo);

    // Pre-select boat by ID_BATEAU using findData(userData)
    if (!idBateauText.isEmpty()) {
        bool idOk = false;
        const int idBateau = idBateauText.toInt(&idOk);
        if (idOk) {
            const int boatIndex = boatCombo->findData(QVariant(static_cast<qlonglong>(idBateau)));
            if (boatIndex >= 0) {
                boatCombo->setCurrentIndex(boatIndex);
            }
        }
    } else {
        boatCombo->setCurrentIndex(0);
    }

    QComboBox *typeCombo = new QComboBox(&dialog);
    typeCombo->setEditable(true);
    if (ui && ui->cap_comboBox_4) {
        for (int i = 0; i < ui->cap_comboBox_4->count(); ++i) {
            typeCombo->addItem(ui->cap_comboBox_4->itemText(i));
        }
    }
    if (!typePoisson.isEmpty()) {
        int typeIndex = typeCombo->findText(typePoisson);
        if (typeIndex >= 0) {
            typeCombo->setCurrentIndex(typeIndex);
        } else {
            typeCombo->setCurrentText(typePoisson);
        }
    }

    QSpinBox *quantiteSpin = new QSpinBox(&dialog);
    quantiteSpin->setRange(0, 1000000);
    quantiteSpin->setValue(quantite);

    QDoubleSpinBox *poidsSpin = new QDoubleSpinBox(&dialog);
    poidsSpin->setRange(0.0, 1000000.0);
    poidsSpin->setDecimals(2);
    poidsSpin->setValue(poids);

    QDateEdit *dateEdit = new QDateEdit(captureDate, &dialog);
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);

    formLayout->addRow(QStringLiteral("ID Capture:"), idCaptureEdit);
    formLayout->addRow(QStringLiteral("ID Bateau:"), boatCombo);
    formLayout->addRow(QStringLiteral("Type Poisson:"), typeCombo);
    formLayout->addRow(QStringLiteral("Quantité:"), quantiteSpin);
    formLayout->addRow(QStringLiteral("Poids:"), poidsSpin);
    formLayout->addRow(QStringLiteral("Date Capture:"), dateEdit);
    formLayout->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    if (!handleCrudDisabled(this)) {
        return;
    }

    // Save modifications
    QSqlQuery updateQuery;

    // IMPORTANT (Oracle ODBC): binding a NULL QVariant for a NUMBER column can be
    // interpreted as BINARY, triggering ORA-00932. Avoid binding NULL for ID_BATEAU;
    // set it to NULL in SQL when no boat is selected.
    const QVariant selectedBoatId = boatCombo->currentData();
    qlonglong idBateauLl = 0;
    const bool hasBoatId = tryToLongLong(selectedBoatId, idBateauLl);

    if (hasBoatId) {
        updateQuery.prepare(
            "UPDATE CAPTURES "
            "SET ID_BATEAU=:idBateau, TYPE_POISSON=:type, QUANTITE=:qte, POIDS=:poids, DATE_CAPTURE=:date "
            "WHERE ID_CAPTURE=:idCapture"
        );
        updateQuery.bindValue(QStringLiteral(":idBateau"), QVariant(idBateauLl));
    } else {
        updateQuery.prepare(
            "UPDATE CAPTURES "
            "SET ID_BATEAU=NULL, TYPE_POISSON=:type, QUANTITE=:qte, POIDS=:poids, DATE_CAPTURE=:date "
            "WHERE ID_CAPTURE=:idCapture"
        );
    }
    updateQuery.bindValue(QStringLiteral(":type"), typeCombo->currentText().trimmed());
    updateQuery.bindValue(QStringLiteral(":qte"), quantiteSpin->value());
    updateQuery.bindValue(QStringLiteral(":poids"), poidsSpin->value());
    updateQuery.bindValue(QStringLiteral(":date"), dateEdit->date());
    updateQuery.bindValue(QStringLiteral(":idCapture"), idCapture);

    debugBoundValues(updateQuery, QStringLiteral("Capture UPDATE"));

    if (!updateQuery.exec()) {
        QMessageBox::warning(
            this,
            QStringLiteral("Erreur Base de Données"),
            QStringLiteral("Impossible de modifier la capture: ") + updateQuery.lastError().text());
        return;
    }

    QSqlDatabase::database().commit();
    loadCapturesTable();
    QMessageBox::information(this, QStringLiteral("Succès"), QStringLiteral("Capture modifiée avec succès!"));
}

void MainWindow::deleteCaptureRow(int row)
{
    if (!ui || !ui->cap_tableWidget) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("Table not found"));
        return;
    }

    QTableWidget *tbl = ui->cap_tableWidget;
    if (row < 0 || row >= tbl->rowCount()) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("Invalid row"));
        return;
    }

    // Get ID_CAPTURE
    QTableWidgetItem *idItem = tbl->item(row, 0);
    if (!idItem) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("Cannot find ID_CAPTURE"));
        return;
    }

    int idCapture = idItem->text().toInt();

    // Confirm deletion
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        QStringLiteral("Confirmation"),
        QStringLiteral("Êtes-vous sûr de vouloir supprimer la capture #%1 ?").arg(idCapture),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply != QMessageBox::Yes) {
        return;
    }

    // Execute DELETE query
    QSqlQuery deleteQuery;
    deleteQuery.prepare("DELETE FROM CAPTURES WHERE \"ID_CAPTURE\" = :idCapture");
    deleteQuery.bindValue(":idCapture", idCapture);

    if (deleteQuery.exec()) {
        QSqlDatabase::database().commit();
        QMessageBox::information(this, QStringLiteral("Succès"), 
                                QStringLiteral("Capture supprimée avec succès!"));
        
        // Refresh the table
        loadCapturesTable();
    } else {
        QString errMsg = deleteQuery.lastError().text();
        QMessageBox::warning(this, QStringLiteral("Erreur Base de Données"), 
                            QStringLiteral("Impossible de supprimer la capture: ") + errMsg);
    }
}

void MainWindow::refreshSingleCapture(int row)
{
    Q_UNUSED(row);
    if (!ui || !ui->cap_tableWidget) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("Table not found"));
        return;
    }

    // Simply reload the entire table - this refreshes the single row
    loadCapturesTable();
    
    QMessageBox::information(this, QStringLiteral("Actualisation"), 
                            QStringLiteral("Données rechargées depuis la base de données."));
}



