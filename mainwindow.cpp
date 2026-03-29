#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QAbstractButton>
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDate>
#include <QMap>
#include <QPair>

#include <QDateTime>
#include <QMessageBox>
#include <QMetaType>
#include <QStringList>
#include <QList>
#include "connection.h"
#include "captures.h"
#include "quotas.h"
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
    , isEditingCapture(false)
    , currentEditCaptureId(-1)
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
    
    // Step 1: Try to create the QUOTAS table (schema compatible avec QUOTAS_SQL_SETUP.sql)
    // Using PL/SQL block to ignore "table already exists" errors
    query.prepare(
        "BEGIN "
        "  BEGIN "
        "    CREATE TABLE QUOTAS ( "
        "      ID_QUOTA NUMBER PRIMARY KEY, "
        "      TYPE_POISSON VARCHAR2(50) NOT NULL, "
        "      VALEUR_QUOTA NUMBER(10,2) NOT NULL "
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
                "INSERT INTO QUOTAS (ID_QUOTA, TYPE_POISSON, VALEUR_QUOTA) "
                "VALUES (:idQuota, :fishType, :value)"
            );
            insertQuery.bindValue(":idQuota", quota.first);
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
    updateTop5FishStats();  // Update top 5 fish species statistics
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
    
    // Update statistics
    updateSurpecheStats();

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
    
    // Update statistics
    updateTendanceStats();

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

    // On n'utilise pas les numéros de lignes -> cacher la marge de gauche
    if (tbl->verticalHeader()) {
        tbl->verticalHeader()->setVisible(false);
    }

    // Setup columns with Actions column
    if (tbl->columnCount() == 0) {
        tbl->setColumnCount(7); // ID, Bateau, Type, Qty, Poids, Date, Actions
        QStringList headers;
        headers << "ID Capture" << "ID Bateau" << "Type Poisson" << "Quantité" << "Poids" << "Date" << "Actions";
        tbl->setHorizontalHeaderLabels(headers);
        tbl->setColumnWidth(6, 75); // Actions column pour 2 boutons (moins d'espace perdu)
    }

    // Load data from database
    QList<QStringList> rows = Captures::getAllCapturesAsRows();
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
        // Create a container widget for the buttons
        QWidget *buttonsWidget = new QWidget(tbl);
        // Couleur de fond identique aux autres cellules du tableau
        buttonsWidget->setStyleSheet("background-color: rgb(224, 238, 255);");
        QHBoxLayout *layout = new QHBoxLayout(buttonsWidget);
        layout->setContentsMargins(4, 1, 4, 1);
        layout->setSpacing(6);
        
        // Edit button - emoji only, transparent background, same border style
        QPushButton *editBtn = new QPushButton(QStringLiteral("📝"));
        // un peu plus large, un peu moins haut
        editBtn->setFixedSize(40, 30);
        editBtn->setStyleSheet(
            "QPushButton {"
            "  background-color: transparent;"
            "  border: 2px solid rgb(0, 0, 112);"  // même couleur que les autres boutons
            "  border-radius: 5px;"
            "  font-size: 16px;"
            "  padding: 3px;"
            "}"
        );
        
        // Delete button - emoji only, transparent background, même bordure
        QPushButton *deleteBtn = new QPushButton(QStringLiteral("❌"));
        // un peu plus large, un peu moins haut
        deleteBtn->setFixedSize(40, 30);
        deleteBtn->setStyleSheet(
            "QPushButton {"
            "  background-color: transparent;"
            "  border: 2px solid rgb(0, 0, 112);"  // même couleur que les autres boutons
            "  border-radius: 5px;"
            "  font-size: 16px;"
            "  padding: 3px;"
            "}"
        );
        
        layout->addWidget(editBtn);
        layout->addWidget(deleteBtn);
        layout->addStretch();
        
        tbl->setCellWidget(row, 6, buttonsWidget);
        
        // Connect buttons to slots using captured row index
        connect(editBtn, &QPushButton::clicked, [this, row]() {
            onCaptureEditClicked(row);
        });
        connect(deleteBtn, &QPushButton::clicked, [this, row]() {
            onCaptureDeleteClicked(row);
        });
    }

    // Set column widths for better visibility (table plus large et lisible)
    tbl->setColumnWidth(0, 100);   // ID Capture
    tbl->setColumnWidth(1, 110);   // ID Bateau
    tbl->setColumnWidth(2, 160);   // Type Poisson
    tbl->setColumnWidth(3, 100);   // Quantité
    tbl->setColumnWidth(4, 100);   // Poids
    tbl->setColumnWidth(5, 160);   // Date

    // Éviter que la colonne Actions soit étirée inutilement
    if (tbl->horizontalHeader()) {
        QHeaderView *header = tbl->horizontalHeader();
        header->setStretchLastSection(false);
        header->setSectionResizeMode(6, QHeaderView::Fixed);
        tbl->setColumnWidth(6, 100); // un peu plus large pour mieux présenter les boutons
    }

    // Ajuster la largeur totale du tableau pour qu'il s'arrête après "Actions"
    int totalWidth = 0;
    for (int c = 0; c < tbl->columnCount(); ++c) {
        totalWidth += tbl->columnWidth(c);
    }
    totalWidth += 2 * tbl->frameWidth();
    tbl->setMinimumWidth(totalWidth);
    tbl->setMaximumWidth(totalWidth);
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

    const int idBat = ui->cap_lineEdit_9->text().toInt();
    if (idBat <= 0) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("L'ID bateau doit être un entier positif."));
        ui->cap_lineEdit_9->setFocus();
        return;
    }
    
    // Vérifier que l'ID_BATEAU existe dans la table BATEAUX
    QSqlQuery checkBateauQuery;
    checkBateauQuery.prepare("SELECT COUNT(*) FROM BATEAUX WHERE ID_BATEAU = :idBateau");
    checkBateauQuery.bindValue(":idBateau", idBat);
    if (!checkBateauQuery.exec()) {
        QMessageBox::critical(this, QStringLiteral("Erreur"), 
                            QStringLiteral("Erreur lors de la vérification du bateau:\n") + checkBateauQuery.lastError().text());
        return;
    }
    if (checkBateauQuery.next() && checkBateauQuery.value(0).toInt() == 0) {
        QMessageBox::warning(this, QStringLiteral("Bateau introuvable"), 
                           QStringLiteral("Le bateau avec l'ID %1 n'existe pas dans la base de données.\n"
                                        "Veuillez saisir un ID de bateau valide.").arg(idBat));
        ui->cap_lineEdit_9->setFocus();
        ui->cap_lineEdit_9->selectAll();
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

    // Check if we are in edit mode
    if (isEditingCapture && currentEditCaptureId > 0) {
        // MODE MODIFICATION
        QSqlQuery updateQuery;
        updateQuery.prepare(
            "UPDATE CAPTURES "
            "SET ID_BATEAU=:idBateau, TYPE_POISSON=:type, QUANTITE=:qte, POIDS=:poids, DATE_CAPTURE=:date "
            "WHERE ID_CAPTURE=:idCapture"
        );
        
        updateQuery.bindValue(QStringLiteral(":idBateau"), idBat);
        updateQuery.bindValue(QStringLiteral(":type"), type);
        updateQuery.bindValue(QStringLiteral(":qte"), qty);
        updateQuery.bindValue(QStringLiteral(":poids"), w);
        updateQuery.bindValue(QStringLiteral(":date"), d);
        updateQuery.bindValue(QStringLiteral(":idCapture"), currentEditCaptureId);

        if (updateQuery.exec()) {
            QSqlDatabase::database().commit();
            QMessageBox::information(this, QStringLiteral("Succès"), QStringLiteral("Capture modifiée avec succès!"));
            clearCaptureForm();
            loadCapturesTable();
            updateTop5FishStats();
        } else {
            QString errMsg = updateQuery.lastError().text();
            QMessageBox::warning(this, QStringLiteral("Erreur Base de Données"), 
                                QStringLiteral("Impossible de modifier la capture: ") + errMsg);
        }
    } else {
        // MODE AJOUT
        Captures c;
        c.idCapture = idCap;
        c.idBateau = idBat;
        c.typePoisson = type;
        c.quantite = qty;
        c.poids = w;
        c.dateCapture = d;

        if (c.ajouter()) {
            QMessageBox::information(this, QStringLiteral("Succès"), QStringLiteral("Capture ajoutée avec succès."));
            clearCaptureForm();
            loadCapturesTable();
            updateTop5FishStats();
        } else {
            QString errText = Captures::lastError();
            if (errText.isEmpty()) {
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
    
    // Configurer les colonnes: Col 0 = Espèce, Col 1 = Quotas
    if (tbl->columnCount() < 2) {
        tbl->setColumnCount(2);
        QStringList headers;
        headers << QStringLiteral("Espèce") << QStringLiteral("Quotas");
        tbl->setHorizontalHeaderLabels(headers);
    }
    
    // Charger les quotas depuis la BD
    // Query en cherchant par TYPE_POISSON pour matcher avec les row headers
    QSqlQuery query;
    query.prepare("SELECT TYPE_POISSON, VALEUR_QUOTA FROM QUOTAS ORDER BY TYPE_POISSON");
    
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
        QString fishType;
        if (tbl->verticalHeaderItem(i)) {
            fishType = tbl->verticalHeaderItem(i)->text();
        }
        
        // Colonne 0: afficher le nom du poisson (non éditable)
        QTableWidgetItem *fishItem = tbl->item(i, 0);
        if (!fishItem) {
            fishItem = new QTableWidgetItem();
        }
        fishItem->setText(fishType);
        fishItem->setFlags(fishItem->flags() & ~Qt::ItemIsEditable);
        tbl->setItem(i, 0, fishItem);
        
        // Chercher le quota correspondant dans la BD
        double quotaValue = 0.0;
        if (quotasMap.contains(fishType)) {
            quotaValue = quotasMap[fishType];
        }
        
        // Mettre à jour le tableau avec la valeur du quota (colonne 1)
        QTableWidgetItem *item = new QTableWidgetItem(QString::number(quotaValue, 'f', 2));
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(item->flags() | Qt::ItemIsEditable);  // Éditable
        tbl->setItem(i, 1, item);
    }

    // Présentation: cacher les en-têtes de lignes et ajuster les largeurs
    tbl->verticalHeader()->setVisible(false);
    tbl->resizeRowsToContents();
    if (tbl->horizontalHeader()) {
        tbl->horizontalHeader()->setStretchLastSection(false);
        tbl->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        tbl->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    }
}

void MainWindow::modifyQuotaRow(int row)
{
    if (!ui || !ui->cap_tableWidget_2) {
        return;
    }

    QTableWidget *tbl = ui->cap_tableWidget_2;
    
    // Get fish type depuis la colonne 0
    QTableWidgetItem *fishItem = tbl->item(row, 0);
    if (!fishItem) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("Impossible de trouver le type de poisson."));
        return;
    }
    QString fishType = fishItem->text().trimmed();
    
    // Get the quota value from the table (colonne 1)
    QTableWidgetItem *item = tbl->item(row, 1);
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
    // First, check if the quota exists dans la table QUOTAS
    bool quotaExists = false;
    {
        QSqlQuery query;
        query.prepare("SELECT COUNT(*) FROM QUOTAS WHERE TYPE_POISSON = :fishType");
        query.bindValue(":fishType", fishType);

        // Essayer une première fois
        if (!query.exec()) {
            QString errMsg = query.lastError().text();

            // Si la table QUOTAS n'existe pas encore, tenter de la créer puis réessayer une fois
            if (errMsg.contains("ORA-00942")) {
                initializeQuotasTable();

                query.finish();
                query.clear();
                query.prepare("SELECT COUNT(*) FROM QUOTAS WHERE TYPE_POISSON = :fishType");
                query.bindValue(":fishType", fishType);

                if (!query.exec()) {
                    errMsg = query.lastError().text();
                    QMessageBox::warning(this, QStringLiteral("Erreur Base de Données"),
                                         QStringLiteral("Erreur après création de la table QUOTAS: ") + errMsg);
                    return;
                }
            } else {
                QMessageBox::warning(this, QStringLiteral("Erreur Base de Données"),
                                     QStringLiteral("Erreur lors de la vérification du quota: ") + errMsg);
                return;
            }
        }

        if (query.next()) {
            quotaExists = (query.value(0).toInt() > 0);
        }
        query.finish(); // Finaliser la requête avant d'en créer une nouvelle
    }
    
    if (quotaExists) {
        // Update existing quota (colonne VALEUR_QUOTA)
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE \"QUOTAS\" SET \"VALEUR_QUOTA\" = :quota WHERE \"TYPE_POISSON\" = :fishType");
        updateQuery.bindValue(":quota", quota);
        updateQuery.bindValue(":fishType", fishType);
        
        if (!updateQuery.exec()) {
            QString errMsg = updateQuery.lastError().text();
            QMessageBox::warning(this, QStringLiteral("Erreur Base de Données"), 
                                QStringLiteral("Impossible de modifier le quota: ") + errMsg);
            return;
        }
        updateQuery.finish();
        QSqlDatabase::database().commit();
        QMessageBox::information(this, QStringLiteral("Succès"), 
                                QStringLiteral("Quota modifié avec succès!"));
    } else {
        // Create new quota entry (générer un nouvel ID_QUOTA) en deux étapes
        int nextId = 1;
        {
            QSqlQuery idQuery;
            idQuery.prepare("SELECT NVL(MAX(\"ID_QUOTA\"),0) + 1 FROM \"QUOTAS\"");
            if (!idQuery.exec() || !idQuery.next()) {
                QString errMsg = idQuery.lastError().text();
                QMessageBox::warning(this, QStringLiteral("Erreur Base de Données"),
                                     QStringLiteral("Impossible de récupérer un nouvel ID_QUOTA: ") + errMsg);
                return;
            }
            nextId = idQuery.value(0).toInt();
            idQuery.finish();
        }

        QSqlQuery insertQuery;
        insertQuery.prepare(
            "INSERT INTO \"QUOTAS\" (\"ID_QUOTA\", \"TYPE_POISSON\", \"VALEUR_QUOTA\") "
            "VALUES (:idQuota, :fishType, :quota)"
        );
        insertQuery.bindValue(":idQuota", nextId);
        insertQuery.bindValue(":fishType", fishType);
        insertQuery.bindValue(":quota", quota);
        
        if (!insertQuery.exec()) {
            QString errMsg = insertQuery.lastError().text();
            QMessageBox::warning(this, QStringLiteral("Erreur Base de Données"), 
                                QStringLiteral("Impossible d'ajouter le quota: ") + errMsg);
            return;
        }
        insertQuery.finish();
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

void MainWindow::clearCaptureForm()
{
    if (!ui) return;
    
    // Reset mode to add
    isEditingCapture = false;
    currentEditCaptureId = -1;
    
    // Clear all form fields
    if (ui->cap_lineEdit_11) {
        ui->cap_lineEdit_11->clear();
        ui->cap_lineEdit_11->setReadOnly(false);
    }
    if (ui->cap_lineEdit_9) ui->cap_lineEdit_9->clear();
    if (ui->cap_comboBox_4) ui->cap_comboBox_4->setCurrentIndex(0);
    if (ui->cap_sbQuantite_4) ui->cap_sbQuantite_4->setValue(0);
    if (ui->cap_dsPoids_3) ui->cap_dsPoids_3->setValue(0.0);
    if (ui->cap_deDateCapture_3) ui->cap_deDateCapture_3->setDate(QDate::currentDate());
    
    // Change button text back to "Ajouter"
    if (ui->cap_btnValiider_3) {
        ui->cap_btnValiider_3->setText(QStringLiteral("Ajouter"));
    }
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

    // Fill form with data from selected row
    isEditingCapture = true;
    currentEditCaptureId = idCapture;
    
    if (ui->cap_lineEdit_11) {
        ui->cap_lineEdit_11->setText(QString::number(idCapture));
        ui->cap_lineEdit_11->setReadOnly(true);
    }
    if (ui->cap_lineEdit_9) {
        ui->cap_lineEdit_9->setText(idBateauText);
    }
    if (ui->cap_comboBox_4) {
        int typeIndex = ui->cap_comboBox_4->findText(typePoisson);
        if (typeIndex >= 0) {
            ui->cap_comboBox_4->setCurrentIndex(typeIndex);
        } else {
            ui->cap_comboBox_4->setCurrentText(typePoisson);
        }
    }
    if (ui->cap_sbQuantite_4) {
        ui->cap_sbQuantite_4->setValue(quantite);
    }
    if (ui->cap_dsPoids_3) {
        ui->cap_dsPoids_3->setValue(poids);
    }
    if (ui->cap_deDateCapture_3) {
        ui->cap_deDateCapture_3->setDate(captureDate);
    }
    
    // Change button text to "Modifier"
    if (ui->cap_btnValiider_3) {
        ui->cap_btnValiider_3->setText(QStringLiteral("Modifier"));
    }
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
        updateTop5FishStats();
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

void MainWindow::updateSurpecheStats()
{
    if (!ui) return;
    
    QTableWidget *statsTable = this->findChild<QTableWidget*>(QStringLiteral("cap_tableWidget_4"));
    if (!statsTable) {
        qDebug() << "cap_tableWidget_4 not found";
        return;
    }
    
    // Clear existing data
    statsTable->setRowCount(0);
    
    // Query to get total captures by fish type with quotas
    QSqlQuery query;
    query.prepare(
        "SELECT c.TYPE_POISSON, "
        "       SUM(c.QUANTITE) as total_quantite, "
        "       SUM(c.POIDS) as total_poids, "
        "       q.VALEUR_QUOTA "
        "FROM CAPTURES c "
        "LEFT JOIN QUOTAS q ON c.TYPE_POISSON = q.TYPE_POISSON "
        "GROUP BY c.TYPE_POISSON, q.VALEUR_QUOTA "
        "ORDER BY c.TYPE_POISSON"
    );
    
    if (!query.exec()) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), 
                           QStringLiteral("Erreur lors du chargement des statistiques: ") + query.lastError().text());
        return;
    }
    
    int row = 0;
    while (query.next()) {
        QString fishType = query.value(0).toString();
        double totalQuantite = query.value(1).toDouble();
        double totalPoids = query.value(2).toDouble();
        QVariant quotaVariant = query.value(3);
        
        double quota = quotaVariant.isNull() ? 0.0 : quotaVariant.toDouble();
        double percentUsed = (quota > 0) ? (totalPoids / quota * 100.0) : 0.0;
        
        QString etat;
        if (quota == 0.0) {
            etat = QStringLiteral("⚠️ Pas de quota");
        } else if (percentUsed < 70.0) {
            etat = QStringLiteral("✅ OK");
        } else if (percentUsed < 100.0) {
            etat = QStringLiteral("⚠️ Attention");
        } else {
            etat = QStringLiteral("❌ Surpêche");
        }
        
        statsTable->insertRow(row);
        statsTable->setItem(row, 0, new QTableWidgetItem(fishType));
        statsTable->setItem(row, 1, new QTableWidgetItem(QString::number(totalQuantite, 'f', 0)));
        statsTable->setItem(row, 2, new QTableWidgetItem(QString::number(totalPoids, 'f', 2) + " kg"));
        statsTable->setItem(row, 3, new QTableWidgetItem(quota > 0 ? QString::number(quota, 'f', 2) + " kg" : "N/A"));
        statsTable->setItem(row, 4, new QTableWidgetItem(quota > 0 ? QString::number(percentUsed, 'f', 1) + "%" : "N/A"));
        statsTable->setItem(row, 5, new QTableWidgetItem(etat));
        
        // Color code the state column
        QTableWidgetItem *etatItem = statsTable->item(row, 5);
        if (etat.contains(QStringLiteral("OK"))) {
            etatItem->setBackground(QColor(144, 238, 144)); // Light green
        } else if (etat.contains(QStringLiteral("Attention"))) {
            etatItem->setBackground(QColor(255, 215, 0)); // Gold
        } else if (etat.contains(QStringLiteral("Surpêche"))) {
            etatItem->setBackground(QColor(255, 99, 71)); // Tomato red
        }
        
        row++;
    }
    
    // Resize columns to content
    statsTable->resizeColumnsToContents();
}

void MainWindow::updateTendanceStats()
{
    if (!ui) return;
    
    QComboBox *fishTypeCombo = this->findChild<QComboBox*>(QStringLiteral("cap_comboBox_3"));
    if (!fishTypeCombo) {
        qDebug() << "cap_comboBox_3 not found";
        return;
    }
    
    QString selectedFishType = fishTypeCombo->currentText();
    
    // Query to get monthly statistics
    QSqlQuery query;
    if (selectedFishType == QStringLiteral("Tous") || selectedFishType.isEmpty()) {
        query.prepare(
            "SELECT EXTRACT(MONTH FROM DATE_CAPTURE) as mois, "
            "       SUM(QUANTITE) as total_quantite, "
            "       SUM(POIDS) as total_poids, "
            "       COUNT(*) as nb_captures "
            "FROM CAPTURES "
            "WHERE EXTRACT(YEAR FROM DATE_CAPTURE) = EXTRACT(YEAR FROM SYSDATE) "
            "GROUP BY EXTRACT(MONTH FROM DATE_CAPTURE) "
            "ORDER BY mois"
        );
    } else {
        query.prepare(
            "SELECT EXTRACT(MONTH FROM DATE_CAPTURE) as mois, "
            "       SUM(QUANTITE) as total_quantite, "
            "       SUM(POIDS) as total_poids, "
            "       COUNT(*) as nb_captures "
            "FROM CAPTURES "
            "WHERE TYPE_POISSON = :fishType "
            "  AND EXTRACT(YEAR FROM DATE_CAPTURE) = EXTRACT(YEAR FROM SYSDATE) "
            "GROUP BY EXTRACT(MONTH FROM DATE_CAPTURE) "
            "ORDER BY mois"
        );
        query.bindValue(":fishType", selectedFishType);
    }
    
    if (!query.exec()) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), 
                           QStringLiteral("Erreur lors du chargement des tendances: ") + query.lastError().text());
        return;
    }
    
    // Create a map of month data
    QMap<int, QPair<double, int>> monthlyData;
    while (query.next()) {
        int month = query.value(0).toInt();
        double totalPoids = query.value(2).toDouble();
        int nbCaptures = query.value(3).toInt();
        monthlyData[month] = QPair<double, int>(totalPoids, nbCaptures);
    }
    
    // Display results in a message box (since we don't have a chart widget)
    QString statsText = QStringLiteral("📊 Statistiques Saisonnières ");
    if (selectedFishType != QStringLiteral("Tous")) {
        statsText += QStringLiteral("pour ") + selectedFishType;
    }
    statsText += QStringLiteral("\n\n");
    
    QStringList monthNames = {
        "Janvier", "Février", "Mars", "Avril", "Mai", "Juin",
        "Juillet", "Août", "Septembre", "Octobre", "Novembre", "Décembre"
    };
    
    if (monthlyData.isEmpty()) {
        statsText += QStringLiteral("Aucune donnée disponible pour cette année.");
    } else {
        for (int month = 1; month <= 12; ++month) {
            if (monthlyData.contains(month)) {
                double poids = monthlyData[month].first;
                int nb = monthlyData[month].second;
                statsText += QString("%1: %2 kg (%3 captures)\n")
                    .arg(monthNames[month - 1])
                    .arg(poids, 0, 'f', 2)
                    .arg(nb);
            }
        }
    }
    
    QMessageBox::information(this, QStringLiteral("Tendance Saisonnière"), statsText);
}

void MainWindow::updateTop5FishStats()
{
    if (!ui) return;
    
    // Find the progress bars and labels
    QProgressBar* progressBars[5] = {
        this->findChild<QProgressBar*>(QStringLiteral("progressZoneNordb_2")),
        this->findChild<QProgressBar*>(QStringLiteral("progressZoneSudb_2")),
        this->findChild<QProgressBar*>(QStringLiteral("progressZoneEstb_2")),
        this->findChild<QProgressBar*>(QStringLiteral("progressZoneOuestb_3")),
        this->findChild<QProgressBar*>(QStringLiteral("progressZoneOuestb_2"))
    };
    
    QLabel* labels[5] = {
        this->findChild<QLabel*>(QStringLiteral("label_zoneNordb_2")),
        this->findChild<QLabel*>(QStringLiteral("label_zoneSudb_2")),
        this->findChild<QLabel*>(QStringLiteral("label_zoneEstb_2")),
        this->findChild<QLabel*>(QStringLiteral("label_zoneOuestb_3")),
        this->findChild<QLabel*>(QStringLiteral("label_zoneOuestb_2"))
    };
    
    QLabel* valueLabels[5] = {
        this->findChild<QLabel*>(QStringLiteral("value_zoneNordb_2")),
        this->findChild<QLabel*>(QStringLiteral("value_zoneSudb_2")),
        this->findChild<QLabel*>(QStringLiteral("value_zoneEstb_2")),
        this->findChild<QLabel*>(QStringLiteral("value_zoneOuestb_3")),
        this->findChild<QLabel*>(QStringLiteral("value_zoneOuestb_2"))
    };
    
    // Query to get TOP 5 fish species by total weight
    QSqlQuery query;
    
    // Try HR.CAPTURES first, then CAPTURES if it fails
    QString sql = 
        "SELECT * FROM ( "
        "  SELECT TYPE_POISSON, "
        "         SUM(POIDS) as total_poids, "
        "         COUNT(*) as nb_captures "
        "  FROM HR.CAPTURES "
        "  GROUP BY TYPE_POISSON "
        "  ORDER BY SUM(POIDS) DESC "
        ") WHERE ROWNUM <= 5";
    
    query.prepare(sql);
    if (!query.exec()) {
        // Try without HR schema
        sql = 
            "SELECT * FROM ( "
            "  SELECT TYPE_POISSON, "
            "         SUM(POIDS) as total_poids, "
            "         COUNT(*) as nb_captures "
            "  FROM CAPTURES "
            "  GROUP BY TYPE_POISSON "
            "  ORDER BY SUM(POIDS) DESC "
            ") WHERE ROWNUM <= 5";
        query.prepare(sql);
        
        if (!query.exec()) {
            QMessageBox::warning(this, QStringLiteral("Erreur"), 
                               QStringLiteral("Erreur Top 5: ") + query.lastError().text());
            return;
        }
    }
    
    // Collect results
    struct FishData {
        QString name;
        double weight;
        int count;
    };
    QList<FishData> fishList;
    double totalWeight = 0.0;
    
    while (query.next()) {
        FishData fish;
        fish.name = query.value(0).toString();
        fish.weight = query.value(1).toDouble();
        fish.count = query.value(2).toInt();
        fishList.append(fish);
        totalWeight += fish.weight;
    }
    
    query.finish(); // Finaliser la requête pour éviter l'erreur S1010
    
    // Debug: afficher le nombre de résultats
    if (fishList.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("Info"), 
                               QStringLiteral("Aucune donnée de capture trouvée pour les statistiques."));
    }
    
    // Update widgets
    for (int i = 0; i < 5; ++i) {
        if (i < fishList.size()) {
            // Calculate percentage
            int percentage = (totalWeight > 0) ? static_cast<int>((fishList[i].weight / totalWeight) * 100.0) : 0;
            
            // Update progress bar
            if (progressBars[i]) {
                progressBars[i]->setValue(percentage);
            }
            
            // Update fish name label
            if (labels[i]) {
                labels[i]->setText(fishList[i].name);
            }
            
            // Update value label (weight / count)
            if (valueLabels[i]) {
                valueLabels[i]->setText(QString("%1 kg / %2")
                    .arg(fishList[i].weight, 0, 'f', 1)
                    .arg(fishList[i].count));
            }
        } else {
            // Reset empty slots
            if (progressBars[i]) {
                progressBars[i]->setValue(0);
            }
            if (labels[i]) {
                labels[i]->setText("");
            }
            if (valueLabels[i]) {
                valueLabels[i]->setText("0/0");
            }
        }
    }
}
