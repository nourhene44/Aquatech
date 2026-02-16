#include "GQuai.h"
#include "ui_mainwindow.h"
#include <QAbstractButton>
#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDate>

#include <QDateTime>
#include <QMessageBox>
#include <QMetaType>
#include <QStringList>
// Database
#include "connection.h"
#include <QSqlQuery>
#include <QSqlError>

// Toggle global: activer/désactiver les opérations CRUD.
static const bool kCrudEnabled = false;

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

    // loadCaptures(); (removed)
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
Connection* MainWindow::getOracleConnection()
{
    return Connection::getInstance();
}

// Close is a no-op wrapper (Connection singleton handles lifecycle)
void MainWindow::closeOracleConnection(Connection* conn)
{
    Q_UNUSED(conn);
    // Nothing to do: Connection singleton will be closed on app exit
}

