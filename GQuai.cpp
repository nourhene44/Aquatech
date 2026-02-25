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
#include <QRegularExpression>

#include <QDateTime>
#include <QMessageBox>
#include <QMetaType>
#include <QStringList>
// Database
#include "connection.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include "bateaauuu.h"
#include <QPixmap>
#include <QPainter>
#include <QColor>
#include <QFont>
#include <cmath>
#include <QPainterPath>
// PDF / dialogs
#include <QFileDialog>
#include <QPdfWriter>
#include <QPageSize>
#include <QPageLayout>
#include <QImage>
#include <QStandardPaths>

// Toggle global: activer/désactiver les opérations CRUD.
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
{
    ui->setupUi(this);

    // Ensure password fields show masked characters (dots).
    // Explicit known field: login password `lineEdit_2b`.
    if (ui->lineEdit_2b) ui->lineEdit_2b->setEchoMode(QLineEdit::Password);

    // Also auto-detect likely password fields by object name or placeholder text.
    const auto allEdits = this->findChildren<QLineEdit*>();
    for (QLineEdit* e : allEdits) {
        if (!e) continue;
        QString on = e->objectName().toLower();
        QString ph = e->placeholderText().toLower();
        if (on.contains("mdp") || on.contains("pass") || ph.contains(QStringLiteral("mot de passe")) || ph.contains(QStringLiteral("password"))) {
            e->setEchoMode(QLineEdit::Password);
        }
    }




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

    // Recherche bateaux : connecter le bouton loupe et le champ texte
    if (ui->pushButton_9e_2b) {
        connect(ui->pushButton_9e_2b, &QPushButton::clicked, this, &MainWindow::filterBateaux);
    }
    if (ui->lineEdit_4p_2) {
        connect(ui->lineEdit_4p_2, &QLineEdit::textChanged, this, &MainWindow::filterBateaux);
    }
    if (ui->doubleSpinBox_largeur) {
        connect(ui->doubleSpinBox_largeur, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, [this](double) { filterBateaux(); });
    }
    if (ui->spinBoxb_2b) {
        connect(ui->spinBoxb_2b, QOverload<int>::of(&QSpinBox::valueChanged),
                this, [this](int) { filterBateaux(); });
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

    // Load bateaux when opening the gestion page so the table is current
    loadBateaux();
}

void MainWindow::on_pushButton_2b_clicked()
{
    // Ajouter / Enregistrer
    ajouterBateauFromForm();
}

void MainWindow::ajouterBateauFromForm()
{
    if (!handleCrudDisabled(this)) return;

    // Read form fields
    QString id          = ui->lineEdit_3b->text().trimmed();
    QString nom         = ui->lineEdit_4b->text().trimmed();
    QString proprietaire= ui->lineEdit_5b->text().trimmed();
    QString type        = ui->comboBoxb->currentText().trimmed();
    QString statut      = ui->comboBox_2b->currentText().trimmed();
    int     capacite    = ui->spinBoxb->value();
    double  largeur     = ui->doubleSpinBox->value();
    QDate   dateEntree  = ui->dateEdit->date();
    QDate   dateMaint   = ui->dateEdit_2->date();
    int     freqMaint   = ui->spinBox->value();

    // ── Contrôles de saisie ──────────────────────────────────────────────
    QStringList erreurs;

    // ID obligatoire (sauf en mode modification) et unicité (clé primaire)
    if (currentEditingId.isEmpty()) {
        if (id.isEmpty()) {
            erreurs << "L'ID du bateau est obligatoire.";
        } else {
            QSqlQuery checkId;
            checkId.prepare("SELECT COUNT(*) FROM BATEAUX WHERE ID_BATEAU = :id");
            checkId.bindValue(":id", id);
            if (checkId.exec() && checkId.next() && checkId.value(0).toInt() > 0) {
                erreurs << "Cet ID existe déjà. Veuillez saisir un ID unique.";
            }
        }
    }

    // Nom obligatoire et lettres/espaces uniquement
    if (nom.isEmpty()) {
        erreurs << "Le nom du bateau est obligatoire.";
    } else if (!QRegularExpression("^[A-Za-zÀ-ÿ\\s\\-']+$").match(nom).hasMatch()) {
        erreurs << "Le nom du bateau ne doit contenir que des lettres, espaces ou tirets.";
    }

    // Propriétaire obligatoire et lettres/espaces uniquement
    if (proprietaire.isEmpty()) {
        erreurs << "Le propriétaire est obligatoire.";
    } else if (!QRegularExpression("^[A-Za-zÀ-ÿ\\s\\-']+$").match(proprietaire).hasMatch()) {
        erreurs << "Le propriétaire ne doit contenir que des lettres, espaces ou tirets.";
    }

    // Type obligatoire
    if (type.isEmpty()) {
        erreurs << "Le type du bateau est obligatoire.";
    }

    // Statut obligatoire
    if (statut.isEmpty()) {
        erreurs << "Le statut du bateau est obligatoire.";
    }

    // Largeur > 0 (vérifiée avant capacité pour afficher ce message en priorité)
    if (largeur <= 0.0) {
        erreurs << "La largeur doit être supérieure à 0.";
    }

    // Capacité > 0
    if (capacite <= 0) {
        erreurs << "La capacité doit être supérieure à 0.";
    }

    // Fréquence de maintenance > 0 (vérifiée après capacité)
    if (freqMaint <= 0) {
        erreurs << "La fréquence de maintenance doit être supérieure à 0.";
    }

    // Date d'entrée valide et pas dans le futur
    if (!dateEntree.isValid()) {
        erreurs << "La date d'entrée n'est pas valide.";
    } else if (dateEntree > QDate::currentDate()) {
        erreurs << "La date d'entrée ne peut pas être dans le futur.";
    }

    // Date de maintenance <= aujourd'hui si renseignée
    if (dateMaint.isValid() && dateMaint > QDate::currentDate()) {
        erreurs << "La date de dernière maintenance ne peut pas être dans le futur.";
    }

    // Date de maintenance >= date d'entrée
    if (dateEntree.isValid() && dateMaint.isValid() && dateMaint < dateEntree) {
        erreurs << "La date de maintenance ne peut pas être antérieure à la date d'entrée.";
    }

    // Si erreurs, afficher la première erreur et arrêter (affichage séquentiel souhaité)
    if (!erreurs.isEmpty()) {
        QMessageBox::warning(this, "Erreur de saisie", erreurs.first());
        return;
    }
    // ─────────────────────────────────────────────────────────────────────

    bateaauuu b;
    bool ok = false;

    if (!currentEditingId.isEmpty()) {
        // Update existing
        ok = b.updateBateau(currentEditingId, nom, type, capacite, proprietaire,
                            statut, largeur, dateEntree, dateMaint, freqMaint);
        if (ok) {
            QMessageBox::information(this, "Succès", "Bateau modifié avec succès.");
            currentEditingId.clear();
        } else {
            QMessageBox::critical(this, "Erreur", "Échec de la modification:\n" + b.lastError());
        }
    } else {
        // Add new
        ok = b.addBateau(id, nom, type, capacite, proprietaire,
                         statut, largeur, dateEntree, dateMaint, freqMaint);
        if (ok) {
            QMessageBox::information(this, "Succès", "Bateau ajouté avec succès.");
        } else {
            QMessageBox::critical(this, "Erreur", "Échec de l'ajout:\n" + b.lastError());
        }
    }

    if (ok) {
        // Clear form
        ui->lineEdit_3b->clear();
        ui->lineEdit_4b->clear();
        ui->lineEdit_5b->clear();
        ui->comboBoxb->setCurrentIndex(0);
        ui->comboBox_2b->setCurrentIndex(0);
        ui->spinBoxb->setValue(0);
        ui->doubleSpinBox->setValue(0.0);
        ui->spinBox->setValue(0);

        loadBateaux();
    }
}

// ── Destructor ───────────────────────────────────────────────────────────────
MainWindow::~MainWindow()
{
    delete ui;
}

// ── Database helpers ─────────────────────────────────────────────────────────
Connection* MainWindow::getOracleConnection()
{
    static Connection conn;
    conn.createconnect();
    return &conn;
}

void MainWindow::closeOracleConnection(Connection* /*conn*/)
{
    // Connection lifetime is managed by the static object / QSqlDatabase
}

void MainWindow::ajouterBateau(const QString& nom, const QString& type, int capacite,
                               const QString& proprietaire, const QString& statut,
                               double largeur)
{
    if (!handleCrudDisabled(this)) return;
    bateaauuu b;
    if (!b.addBateau(QString(), nom, type, capacite, proprietaire, statut, largeur,
                     QDate::currentDate(), QDate(), 0)) {
        QMessageBox::critical(this, "Erreur", "Échec ajout:\n" + b.lastError());
    }
}

// ── Load bateaux table ──────────────────────────────────────────────────────
void MainWindow::loadBateaux()
{
    if (!ui->tableWidgetb) return;

    // Build SQL with optional WHERE clauses from search filters
    QString sql = "SELECT ID_BATEAU, NOM, PROPRIETAIRE, LARGEUR, TYPE, STATUT, "
                  "CAPACITE, DATE_ENTREE, DATE_DERNIERE_MAINTENANCE, FREQUENCE_MAINTENANCE "
                  "FROM BATEAUX";

    QStringList conditions;
    QString searchText = ui->lineEdit_4p_2 ? ui->lineEdit_4p_2->text().trimmed() : QString();
    double  filterLargeur  = ui->doubleSpinBox_largeur ? ui->doubleSpinBox_largeur->value() : 0.0;
    int     filterCapacite = ui->spinBoxb_2b ? ui->spinBoxb_2b->value() : 0;

    if (!searchText.isEmpty()) {
        QString escaped = searchText;
        escaped.replace("'", "''");
        conditions << QString("(UPPER(NOM) LIKE UPPER('%%1%') OR "
                              "UPPER(PROPRIETAIRE) LIKE UPPER('%%1%') OR "
                              "UPPER(TYPE) LIKE UPPER('%%1%') OR "
                              "UPPER(STATUT) LIKE UPPER('%%1%') OR "
                              "UPPER(TO_CHAR(ID_BATEAU)) LIKE UPPER('%%1%'))").arg(escaped);
    }
    if (filterLargeur > 0.001) {
        conditions << QString("LARGEUR = %1").arg(filterLargeur);
    }
    if (filterCapacite > 0) {
        conditions << QString("CAPACITE = %1").arg(filterCapacite);
    }

    if (!conditions.isEmpty()) {
        sql += " WHERE " + conditions.join(" AND ");
    }

    QSqlQuery query(sql);
    ui->tableWidgetb->setRowCount(0);

    int row = 0;
    while (query.next()) {
        ui->tableWidgetb->insertRow(row);
        for (int col = 0; col < 10; ++col) {
            QString cellText;
            // Format date columns (7=DATE_ENTREE, 8=DATE_DERNIERE_MAINTENANCE) as dd/MM/yyyy
            if (col == 7 || col == 8) {
                QDateTime dt = query.value(col).toDateTime();
                if (dt.isValid()) {
                    cellText = dt.date().toString("dd/MM/yyyy");
                } else {
                    // Fallback: strip any "T..." time suffix from raw string
                    cellText = query.value(col).toString();
                    int tIdx = cellText.indexOf('T');
                    if (tIdx > 0)
                        cellText = cellText.left(tIdx);
                }
            } else {
                cellText = query.value(col).toString();
            }
            ui->tableWidgetb->setItem(row, col,
                new QTableWidgetItem(cellText));
        }

        // ── Colonne Actions : boutons icônes Modifier + Supprimer ──
        QWidget *actionWidget = new QWidget();
        actionWidget->setStyleSheet("background: transparent; border: none;");
        QHBoxLayout *layout = new QHBoxLayout(actionWidget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(2);

        QPushButton *btnModifier = new QPushButton(QStringLiteral("📝"));
        btnModifier->setFixedSize(28, 28);
        btnModifier->setToolTip("Modifier");
        btnModifier->setStyleSheet(
            "QPushButton { background: transparent; "
            "border: 2px solid rgb(0, 0, 112); border-radius: 6px; font-size: 14px; padding: 0px; } "
            "QPushButton:hover { background-color: rgba(0, 0, 112, 0.1); }");

        QPushButton *btnSupprimer = new QPushButton(QStringLiteral("❌"));
        btnSupprimer->setFixedSize(28, 28);
        btnSupprimer->setToolTip("Supprimer");
        btnSupprimer->setStyleSheet(
            "QPushButton { background: transparent; "
            "border: 2px solid rgb(0, 0, 112); border-radius: 6px; font-size: 14px; padding: 0px; } "
            "QPushButton:hover { background-color: rgba(0, 0, 112, 0.1); }");

        layout->addWidget(btnModifier);
        layout->addWidget(btnSupprimer);
        actionWidget->setLayout(layout);

        // Connect with captured row index
        int currentRow = row;
        connect(btnModifier, &QPushButton::clicked, this, [this, currentRow]() {
            modifierBateauFromRow(currentRow);
        });
        connect(btnSupprimer, &QPushButton::clicked, this, [this, currentRow]() {
            supprimerBateauFromRow(currentRow);
        });

        // Column 10 = "Actions"
        ui->tableWidgetb->setCellWidget(row, 10, actionWidget);

        ++row;
    }

    // Hide the unused column 11
    ui->tableWidgetb->setColumnHidden(11, true);

    // Auto-resize all data columns so text is fully visible
    ui->tableWidgetb->resizeColumnsToContents();
    // Stretch data columns to fill available space
    ui->tableWidgetb->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    // Ensure Proprietaire column (2) is wide enough for the full header text
    ui->tableWidgetb->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->tableWidgetb->setColumnWidth(2, 110);
    // Fix Dern. Maint. and Fréq. Maint. columns to proper width
    ui->tableWidgetb->horizontalHeader()->setSectionResizeMode(8, QHeaderView::Fixed);
    ui->tableWidgetb->setColumnWidth(8, 105);
    ui->tableWidgetb->horizontalHeader()->setSectionResizeMode(9, QHeaderView::Fixed);
    ui->tableWidgetb->setColumnWidth(9, 100);
    // Fix the Actions column to a compact width
    ui->tableWidgetb->horizontalHeader()->setSectionResizeMode(10, QHeaderView::Fixed);
    ui->tableWidgetb->setColumnWidth(10, 72);

    updateStatsBateaux();
}

// ── Modifier : remplir le formulaire avec les données de la ligne ────────────
void MainWindow::modifierBateauFromRow(int row)
{
    if (!ui->tableWidgetb) return;

    auto cellText = [&](int col) -> QString {
        QTableWidgetItem *item = ui->tableWidgetb->item(row, col);
        return item ? item->text() : QString();
    };

    currentEditingId = cellText(0); // ID_BATEAU

    ui->lineEdit_3b->setText(cellText(0));   // ID
    ui->lineEdit_4b->setText(cellText(1));   // Nom
    ui->lineEdit_5b->setText(cellText(2));   // Proprietaire
    ui->doubleSpinBox->setValue(cellText(3).toDouble());  // Largeur
    ui->comboBoxb->setCurrentText(cellText(4));           // Type
    ui->comboBox_2b->setCurrentText(cellText(5));         // Statut
    ui->spinBoxb->setValue(cellText(6).toInt());          // Capacite

    // Dates
    QDate de = QDate::fromString(cellText(7), "yyyy-MM-dd");
    if (!de.isValid()) de = QDate::fromString(cellText(7), "dd/MM/yyyy");
    if (de.isValid()) ui->dateEdit->setDate(de);

    QDate dm = QDate::fromString(cellText(8), "yyyy-MM-dd");
    if (!dm.isValid()) dm = QDate::fromString(cellText(8), "dd/MM/yyyy");
    if (dm.isValid()) ui->dateEdit_2->setDate(dm);

    ui->spinBox->setValue(cellText(9).toInt()); // Frequence maintenance

    QMessageBox::information(this, "Modifier",
        QString("Bateau \"%1\" chargé dans le formulaire.\nModifiez les champs puis cliquez Ajouter.").arg(cellText(1)));
}

// ── Supprimer : supprimer le bateau de la ligne ─────────────────────────────
void MainWindow::supprimerBateauFromRow(int row)
{
    if (!ui->tableWidgetb) return;

    QTableWidgetItem *idItem = ui->tableWidgetb->item(row, 0);
    if (!idItem) return;

    QString id  = idItem->text();
    QString nom = ui->tableWidgetb->item(row, 1) ? ui->tableWidgetb->item(row, 1)->text() : id;

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirmer la suppression",
        QString("Voulez-vous vraiment supprimer le bateau \"%1\" (ID: %2) ?").arg(nom, id),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    bateaauuu b;
    if (b.deleteBateau(id)) {
        QMessageBox::information(this, "Succès", "Bateau supprimé avec succès.");
        loadBateaux();
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la suppression:\n" + b.lastError());
    }
}

// ── Filtrer la table bateaux ─────────────────────────────────────────────────
void MainWindow::filterBateaux()
{
    loadBateaux();
}

// ── Update statistics widgets ───────────────────────────────────────────────
void MainWindow::updateStatsBateaux()
{
    // ── Build the same WHERE clause used by loadBateaux() ──
    QStringList conditions;
    QString searchText = ui->lineEdit_4p_2 ? ui->lineEdit_4p_2->text().trimmed() : QString();
    double  filterLargeur  = ui->doubleSpinBox_largeur ? ui->doubleSpinBox_largeur->value() : 0.0;
    int     filterCapacite = ui->spinBoxb_2b ? ui->spinBoxb_2b->value() : 0;

    if (!searchText.isEmpty()) {
        QString escaped = searchText;
        escaped.replace("'", "''");
        conditions << QString("(UPPER(NOM) LIKE UPPER('%%1%') OR "
                              "UPPER(PROPRIETAIRE) LIKE UPPER('%%1%') OR "
                              "UPPER(TYPE) LIKE UPPER('%%1%') OR "
                              "UPPER(STATUT) LIKE UPPER('%%1%') OR "
                              "UPPER(TO_CHAR(ID_BATEAU)) LIKE UPPER('%%1%'))").arg(escaped);
    }
    if (filterLargeur > 0.001) {
        conditions << QString("LARGEUR = %1").arg(filterLargeur);
    }
    if (filterCapacite > 0) {
        conditions << QString("CAPACITE = %1").arg(filterCapacite);
    }

    QString whereClause;
    if (!conditions.isEmpty())
        whereClause = " WHERE " + conditions.join(" AND ");

    // Count by statut (filtered)
    QSqlQuery q;
    int total = 0, enMer = 0, enMaint = 0, auPort = 0, disponible = 0;
    q.exec("SELECT STATUT, COUNT(*) FROM BATEAUX" + whereClause + " GROUP BY STATUT");
    while (q.next()) {
        QString s = q.value(0).toString().trimmed().toLower();
        int     c = q.value(1).toInt();
        total += c;
        if (s.contains("mer"))          enMer      = c;
        else if (s.contains("maint"))   enMaint    = c;
        else if (s.contains("port"))    auPort     = c;
        else if (s.contains("dispo"))   disponible = c;
    }

    auto setBar = [&](QProgressBar* bar, QLabel* lbl, int val) {
        if (!bar || !lbl) return;
        int pct = total > 0 ? qRound(100.0 * val / total) : 0;
        bar->setValue(pct);
        lbl->setText(QString("%1/%2").arg(val).arg(total));
    };

    setBar(ui->progressZoneNordb,  ui->value_zoneNordb,  enMer);
    setBar(ui->progressZoneSudb,   ui->value_zoneSudb,   enMaint);
    setBar(ui->progressZoneEstb,   ui->value_zoneEstb,   auPort);
    setBar(ui->progressZoneOuestb, ui->value_zoneOuestb, disponible);

    // Count by type (filtered)
    int nChalutier = 0, nPalangrier = 0, nCaseyeur = 0, nTraditional = 0, nAutres = 0;
    q.exec("SELECT TYPE, COUNT(*) FROM BATEAUX" + whereClause + " GROUP BY TYPE");
    while (q.next()) {
        QString t = q.value(0).toString().trimmed().toLower();
        int     c = q.value(1).toInt();
        if (t.contains("chalut"))        nChalutier   = c;
        else if (t.contains("palang"))   nPalangrier  = c;
        else if (t.contains("casey"))    nCaseyeur    = c;
        else if (t.contains("tradition"))nTraditional = c;
        else                             nAutres     += c;
    }

    auto pctStr = [&](int v) {
        return total > 0 ? QString::number(qRound(100.0 * v / total)) : QStringLiteral("0");
    };

    if (ui->label_total_typesb)
        ui->label_total_typesb->setText(QString("Total: %1 bateaux").arg(total));
    if (ui->label_legend_chalutierb)
        ui->label_legend_chalutierb->setText(QString("• Chalutier: %1 (%2%)").arg(nChalutier).arg(pctStr(nChalutier)));
    if (ui->label_legend_palangrierb)
        ui->label_legend_palangrierb->setText(QString("• Palangrier: %1 (%2%)").arg(nPalangrier).arg(pctStr(nPalangrier)));
    if (ui->label_legend_caseyeurb)
        ui->label_legend_caseyeurb->setText(QString("• Caseyeur: %1 (%2%)").arg(nCaseyeur).arg(pctStr(nCaseyeur)));
    if (ui->label_legend_traditionalb)
        ui->label_legend_traditionalb->setText(QString("• Traditional: %1 (%2%)").arg(nTraditional).arg(pctStr(nTraditional)));
    if (ui->label_legend_otherb)
        ui->label_legend_otherb->setText(QString("• Autres: %1 (%2%)").arg(nAutres).arg(pctStr(nAutres)));

    // ── Draw solid pie chart on progressTypeCircleb ──
    if (ui->progressTypeCircleb && total > 0) {
        int sz = qMin(ui->progressTypeCircleb->width(), ui->progressTypeCircleb->height());
        if (sz < 50) sz = 200;

        QPixmap pix(sz, sz);
        pix.fill(Qt::transparent);

        QPainter painter(&pix);
        painter.setRenderHint(QPainter::Antialiasing, true);

        struct Slice { int count; QColor color; };
        QVector<Slice> slices = {
            { nChalutier,   QColor("#4A90D9") },   // blue
            { nPalangrier,  QColor("#27ae60") },   // green
            { nCaseyeur,    QColor("#F5A623") },   // orange
            { nTraditional, QColor("#9b59b6") },   // purple
            { nAutres,      QColor("#D0021B") }    // red
        };

        int margin = 6;
        QRectF pieRect(margin, margin, sz - 2 * margin, sz - 2 * margin);

        // Draw slices
        int startAngle = 90 * 16; // start from top (12 o'clock)
        for (const auto &s : slices) {
            if (s.count <= 0) continue;
            int spanAngle = qRound(360.0 * s.count / total * 16);

            painter.setPen(QPen(Qt::white, 2));
            painter.setBrush(s.color);
            painter.drawPie(pieRect, startAngle, -spanAngle);

            startAngle -= spanAngle;
        }

        painter.end();

        ui->progressTypeCircleb->setPixmap(pix);
        ui->progressTypeCircleb->setAlignment(Qt::AlignCenter);
    } else if (ui->progressTypeCircleb) {
        // No data: show empty circle
        int sz = 200;
        QPixmap pix(sz, sz);
        pix.fill(Qt::transparent);
        QPainter painter(&pix);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(QColor(0, 0, 112), 2));
        painter.setBrush(QColor(224, 238, 255));
        painter.drawEllipse(6, 6, sz - 12, sz - 12);
        painter.setPen(QColor(0, 0, 90));
        QFont f = painter.font();
        f.setPixelSize(24);
        f.setBold(true);
        painter.setFont(f);
        painter.drawText(QRectF(0, 0, sz, sz), Qt::AlignCenter, "0");
        painter.end();
        ui->progressTypeCircleb->setPixmap(pix);
        ui->progressTypeCircleb->setAlignment(Qt::AlignCenter);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  EXPORT PDF
// ═══════════════════════════════════════════════════════════════════════════

void MainWindow::on_pushButton_pdfb_clicked()
{
    if (!ui->tableWidgetb || ui->tableWidgetb->rowCount() == 0) {
        QMessageBox::warning(this, "Export PDF", "Le tableau est vide, rien à exporter.");
        return;
    }

    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
                          + "/Bateaux_" + QDate::currentDate().toString("yyyy-MM-dd") + ".pdf";

    QString filePath = QFileDialog::getSaveFileName(this, "Enregistrer le PDF", defaultPath, "PDF (*.pdf)");
    if (filePath.isEmpty()) return;

    QPdfWriter writer(filePath);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageOrientation(QPageLayout::Landscape);
    writer.setResolution(300);

    QPainter painter(&writer);
    if (!painter.isActive()) {
        QMessageBox::critical(this, "Erreur", "Impossible de créer le fichier PDF.");
        return;
    }

    const int pageW = writer.width();
    const int pageH = writer.height();

    // Page margins / content area
    const int leftMargin = 50;
    const int rightMargin = 50;
    const int topMargin = 100;
    const int bottomMargin = 80;
    const int contentW = pageW - leftMargin - rightMargin;

    // ── Title ──
    QFont titleFont("Arial", 20, QFont::Bold);
    titleFont.setPointSize(20);
    painter.setFont(titleFont);
    painter.setPen(QColor(0, 82, 155));
    painter.drawText(QRect(leftMargin, topMargin - 40, contentW, 60), Qt::AlignCenter, "AQUATEC – Liste des Bateaux");

    // ── Date ──
    QFont dateFont("Arial", 9);
    dateFont.setPointSize(9);
    painter.setFont(dateFont);
    painter.setPen(Qt::darkGray);
    painter.drawText(QRect(leftMargin, topMargin + 20, contentW, 30), Qt::AlignCenter,
                     "Exporté le " + QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm"));

    int yStart = topMargin + 80;

    // Column headers (0-9, skip 10 and 11=Actions)
    QStringList headers = { "ID", "Nom", "Propriétaire", "Largeur", "Type",
                            "Statut", "Capacité", "Date entrée", "Dern. maint.", "Fréq. maint." };
    const int colCount = headers.size();

    // Calculate column widths
    QVector<int> colWidths(colCount);
    int totalW = contentW; // use content width inside margins
    // Proportional widths
    QVector<double> proportions = { 0.06, 0.12, 0.13, 0.07, 0.10, 0.10, 0.08, 0.12, 0.12, 0.10 };
    for (int i = 0; i < colCount; ++i)
        colWidths[i] = static_cast<int>(totalW * proportions[i]);

    int rowHeight = 36;
    int headerHeight = 40;
    int xMargin = leftMargin;

    // ── Draw header row ──
    QFont headerFont("Arial", 10, QFont::Bold);
    headerFont.setPointSize(10);
    painter.setFont(headerFont);

    int x = xMargin;
    for (int c = 0; c < colCount; ++c) {
        QRect cellRect(x, yStart, colWidths[c], headerHeight);
        painter.fillRect(cellRect, QColor(0, 82, 155));
        painter.setPen(Qt::white);
        painter.drawText(cellRect.adjusted(8, 0, -8, 0), Qt::AlignCenter | Qt::TextWordWrap, headers[c]);
        painter.setPen(QPen(QColor(220, 220, 220), 1));
        painter.drawRect(cellRect);
        x += colWidths[c];
    }

    // ── Draw data rows ──
    QFont cellFont("Arial", 9);
    cellFont.setPointSize(9);
    painter.setFont(cellFont);

    int y = yStart + headerHeight;
    int rowCount = ui->tableWidgetb->rowCount();

    for (int r = 0; r < rowCount; ++r) {
        // Check if we need a new page
        if (y + rowHeight > pageH - 100) {
            writer.newPage();
            y = 100;

            // Re-draw header on new page
            painter.setFont(headerFont);
            x = xMargin;
            for (int c = 0; c < colCount; ++c) {
                QRect cellRect(x, y, colWidths[c], headerHeight);
                painter.fillRect(cellRect, QColor(0, 0, 112));
                painter.setPen(Qt::white);
                painter.drawText(cellRect.adjusted(10, 0, -10, 0), Qt::AlignCenter | Qt::TextWordWrap, headers[c]);
                painter.setPen(QColor(0, 0, 112));
                painter.drawRect(cellRect);
                x += colWidths[c];
            }
            y += headerHeight;
            painter.setFont(cellFont);
        }

        // Alternate row background
        QColor bg = (r % 2 == 0) ? QColor(250, 250, 252) : QColor(255, 255, 255);

        x = xMargin;
        for (int c = 0; c < colCount; ++c) {
            QRect cellRect(x, y, colWidths[c], rowHeight);
            painter.fillRect(cellRect, bg);
            painter.setPen(QPen(QColor(230, 230, 230), 1));
            painter.drawRect(cellRect);

            QString text;
            QTableWidgetItem *item = ui->tableWidgetb->item(r, c);
            if (item) text = item->text();

            painter.setPen(Qt::black);
            painter.drawText(cellRect.adjusted(8, 0, -8, 0), Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap, text);
            x += colWidths[c];
        }
        y += rowHeight;
    }

    // ── Footer on table page ──
    painter.setPen(Qt::darkGray);
    QFont footerFont("Arial", 9);
    footerFont.setPointSize(9);
    painter.setFont(footerFont);
    painter.drawText(QRect(leftMargin, pageH - bottomMargin + 10, contentW, bottomMargin - 10), Qt::AlignCenter,
                     QString("Total: %1 bateaux").arg(rowCount));

    // ══════════════════════════════════════════════════════════════════
    //  PAGE 2 : STATISTIQUES
    // ══════════════════════════════════════════════════════════════════
    writer.newPage();

    // ── Gather stats data (same logic as updateStatsBateaux) ──
    QStringList statConditions;
    QString sText = ui->lineEdit_4p_2 ? ui->lineEdit_4p_2->text().trimmed() : QString();
    double  sLarg = ui->doubleSpinBox_largeur ? ui->doubleSpinBox_largeur->value() : 0.0;
    int     sCap  = ui->spinBoxb_2b ? ui->spinBoxb_2b->value() : 0;

    if (!sText.isEmpty()) {
        QString escaped = sText; escaped.replace("'", "''");
        statConditions << QString("(UPPER(NOM) LIKE UPPER('%%1%') OR "
                              "UPPER(PROPRIETAIRE) LIKE UPPER('%%1%') OR "
                              "UPPER(TYPE) LIKE UPPER('%%1%') OR "
                              "UPPER(STATUT) LIKE UPPER('%%1%') OR "
                              "UPPER(TO_CHAR(ID_BATEAU)) LIKE UPPER('%%1%'))").arg(escaped);
    }
    if (sLarg > 0.001) statConditions << QString("LARGEUR = %1").arg(sLarg);
    if (sCap  > 0)     statConditions << QString("CAPACITE = %1").arg(sCap);

    QString wc;
    if (!statConditions.isEmpty()) wc = " WHERE " + statConditions.join(" AND ");

    // Statut counts
    QSqlQuery sq;
    int sTotal = 0, sEnMer = 0, sEnMaint = 0, sAuPort = 0, sDisponible = 0;
    sq.exec("SELECT STATUT, COUNT(*) FROM BATEAUX" + wc + " GROUP BY STATUT");
    while (sq.next()) {
        QString st = sq.value(0).toString().trimmed().toLower();
        int     cv = sq.value(1).toInt();
        sTotal += cv;
        if (st.contains("mer"))        sEnMer      = cv;
        else if (st.contains("maint")) sEnMaint    = cv;
        else if (st.contains("port"))  sAuPort     = cv;
        else if (st.contains("dispo")) sDisponible = cv;
    }

    // Type counts
    int nCh = 0, nPa = 0, nCa = 0, nTr = 0, nAu = 0;
    sq.exec("SELECT TYPE, COUNT(*) FROM BATEAUX" + wc + " GROUP BY TYPE");
    while (sq.next()) {
        QString t = sq.value(0).toString().trimmed().toLower();
        int     cv = sq.value(1).toInt();
        if (t.contains("chalut"))        nCh = cv;
        else if (t.contains("palang"))   nPa = cv;
        else if (t.contains("casey"))    nCa = cv;
        else if (t.contains("tradition"))nTr = cv;
        else                             nAu += cv;
    }

    // ── Stats page title ──
    painter.setPen(QColor(0, 82, 155));
    titleFont.setPointSize(18);
    painter.setFont(titleFont);
    painter.drawText(QRect(leftMargin, 60, contentW, 60), Qt::AlignCenter, "Statistiques des Bateaux");

    painter.setPen(Qt::darkGray);
    dateFont.setPointSize(10);
    painter.setFont(dateFont);
    painter.drawText(QRect(leftMargin, 140, contentW, 30), Qt::AlignCenter,
                     QString("Total: %1 bateaux").arg(sTotal));

    // ── Decorative line under title ──
    painter.setPen(QPen(QColor(0, 0, 112), 4));
    painter.drawLine(200, 380, pageW - 200, 380);

    // ══════════════════════════════════════════════════════════════════
    //  TOP SECTION: Répartition par Statut (horizontal bars)
    // ══════════════════════════════════════════════════════════════════
    int statsY = 440;
    int contentX = 200;
    int statsContentW = pageW - 400;

    QFont sectionFont("Arial", 12, QFont::Bold);
    sectionFont.setPointSize(12);
    painter.setFont(sectionFont);
    painter.setPen(QColor(0, 82, 155));
    painter.drawText(QRect(contentX, statsY, statsContentW, 40), Qt::AlignLeft | Qt::AlignVCenter,
                     QString::fromUtf8("📊 Répartition par Statut"));
    statsY += 160;

    struct StatutBar { QString label; int count; QColor color; };
    QVector<StatutBar> bars = {
        { "En Mer",           sEnMer,      QColor("#3498db") },
        { "En Maintenance",   sEnMaint,    QColor("#e67e22") },
        { "Au Port",          sAuPort,     QColor("#2ecc71") },
        { "Disponible",       sDisponible, QColor("#9b59b6") }
    };

    int labelW = 700;
    int barMaxW = statsContentW - labelW - 600;
    int barH = 110;
    int barSpacing = 150;

    QFont barLabelFont("Arial", 10);
    barLabelFont.setPixelSize(70);
    QFont barValueFont("Arial", 9, QFont::Bold);
    barValueFont.setPixelSize(65);

    for (const auto &b : bars) {
        // Label on the left
        painter.setFont(barLabelFont);
        painter.setPen(Qt::black);
        painter.drawText(QRect(contentX, statsY, labelW, barH), Qt::AlignVCenter | Qt::AlignRight, b.label + "  ");

        // Bar background
        int barX = contentX + labelW + 30;
        QRect barBg(barX, statsY + 15, barMaxW, barH - 30);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(230, 235, 245));
        painter.drawRoundedRect(barBg, 15, 15);

        // Bar fill
        if (sTotal > 0 && b.count > 0) {
            int fillW = qMax(30, qRound(static_cast<double>(b.count) / sTotal * barMaxW));
            QRect barFill(barX, statsY + 15, fillW, barH - 30);
            painter.setBrush(b.color);
            painter.drawRoundedRect(barFill, 15, 15);
        }

        // Count + percentage on the right of the bar
        int pct = sTotal > 0 ? qRound(100.0 * b.count / sTotal) : 0;
        painter.setFont(barValueFont);
        painter.setPen(Qt::black);
        painter.drawText(QRect(barX + barMaxW + 30, statsY, 500, barH),
                         Qt::AlignVCenter | Qt::AlignLeft,
                         QString("%1 (%2%)").arg(b.count).arg(pct));

        statsY += barSpacing;
    }

    // ── Separator line ──
    statsY += 40;
    painter.setPen(QPen(QColor(200, 200, 220), 3));
    painter.drawLine(contentX, statsY, contentX + statsContentW, statsY);
    statsY += 60;

    // ══════════════════════════════════════════════════════════════════
    //  BOTTOM SECTION: Répartition par Type (pie chart + legend side by side)
    // ══════════════════════════════════════════════════════════════════
    painter.setFont(sectionFont);
    painter.setPen(QColor(0, 0, 112));
    painter.drawText(QRect(contentX, statsY, statsContentW, 130), Qt::AlignLeft | Qt::AlignVCenter,
                     QString::fromUtf8("📊 Répartition par Type"));
    statsY += 160;

    struct TypeSlice { QString label; int count; QColor color; };
    QVector<TypeSlice> typeSlices = {
        { "Chalutier",    nCh, QColor("#4A90D9") },
        { "Palangrier",   nPa, QColor("#27ae60") },
        { "Caseyeur",     nCa, QColor("#F5A623") },
        { "Traditional",  nTr, QColor("#9b59b6") },
        { "Autres",       nAu, QColor("#D0021B") }
    };

    // Pie on the left, legend on the right — compute sane sizes to avoid overlap
    int maxPieByHeight = pageH - statsY - bottomMargin - 60;
    int maxPieByWidth = statsContentW / 2 - 40;
    int pieSz = qMin(maxPieByHeight, maxPieByWidth);
    if (pieSz < 200) pieSz = qMin(200, maxPieByWidth);
    int pieX = contentX + 20;
    QRectF pieR(pieX, statsY, pieSz, pieSz);

    if (sTotal > 0) {
        int startA = 90 * 16;
        for (const auto &sl : typeSlices) {
            if (sl.count <= 0) continue;
            int spanA = qRound(360.0 * sl.count / sTotal * 16);
            painter.setPen(QPen(Qt::white, 5));
            painter.setBrush(sl.color);
            painter.drawPie(pieR, startA, -spanA);
            startA -= spanA;
        }
    } else {
        painter.setPen(QPen(QColor(200, 200, 200), 4));
        painter.setBrush(QColor(240, 240, 240));
        painter.drawEllipse(pieR);
    }

    // Legend to the right of the pie
    int legendX = pieX + pieSz + 30;
    int legendMaxW = statsContentW - (legendX - contentX) - 20;
    int legendY = statsY + 10; // start a bit below the pie top
    QFont legendFont("Arial", 10);
    legendFont.setPointSize(10);
    painter.setFont(legendFont);

    int itemH = 28;
    int colorBoxSize = 18;
    int gap = 10;

    for (const auto &sl : typeSlices) {
        int pct = sTotal > 0 ? qRound(100.0 * sl.count / sTotal) : 0;

        // Color square
        painter.setBrush(sl.color);
        painter.setPen(Qt::NoPen);
        QRect colorRect(legendX, legendY + (itemH - colorBoxSize) / 2, colorBoxSize, colorBoxSize);
        painter.drawRoundedRect(colorRect, 4, 4);

        // Text
        painter.setPen(Qt::black);
        QRect textRect(legendX + colorBoxSize + gap, legendY, legendMaxW - colorBoxSize - gap, itemH);
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft,
                         QString("%1: %2 (%3%)").arg(sl.label).arg(sl.count).arg(pct));

        legendY += itemH + 8;
    }

    // ── Final footer ──
    painter.setPen(Qt::darkGray);
    painter.setFont(footerFont);
    painter.drawText(QRect(leftMargin, pageH - bottomMargin + 10, contentW, bottomMargin - 10), Qt::AlignCenter,
                     "AQUATEC - Rapport généré le " + QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm"));

    painter.end();

    QMessageBox::information(this, "Export PDF",
        QString("PDF exporté avec succès !\n%1").arg(filePath));
}

// ═══════════════════════════════════════════════════════════════════════════
//  LOGIN  /  RECOVERY  NAVIGATION
// ═══════════════════════════════════════════════════════════════════════════

void MainWindow::on_pushButton_b_clicked()
{
    // Connexion button on login page — hardcoded admin/admin check (no DB)
    QString username = ui->lineEdit_b  ? ui->lineEdit_b->text().trimmed()  : QString();
    QString password = ui->lineEdit_2b ? ui->lineEdit_2b->text().trimmed() : QString();

    if (username == "admin" && password == "admin") {
        if (ui->stackedWidget && ui->gestionsb)
            ui->stackedWidget->setCurrentWidget(ui->gestionsb);
    } else {
        QMessageBox::warning(this, "Erreur de connexion",
            "Nom d'utilisateur ou mot de passe incorrect.");
    }
}

void MainWindow::on_pushButton_b_4b_clicked()
{
    // Second connexion button — same logic
    on_pushButton_b_clicked();
}

void MainWindow::on_pushButton_20b_clicked()
{
    // Mot de passe oublié? -> page Rmdpb
    if (auto* page = findChild<QWidget*>(QStringLiteral("Rmdpb")))
        ui->stackedWidget->setCurrentWidget(page);
}

void MainWindow::on_pushButton_13b_clicked()
{
    // Envoyer (email recovery) -> page Remailb
    if (auto* page = findChild<QWidget*>(QStringLiteral("Remailb")))
        ui->stackedWidget->setCurrentWidget(page);
}

void MainWindow::on_pushButton_14b_clicked()
{
    // Annuler -> retour loginb
    if (ui->stackedWidget && ui->loginb)
        ui->stackedWidget->setCurrentWidget(ui->loginb);
}

void MainWindow::on_pushButton_15b_clicked()
{
    // Vérifier -> page Nmdpb (nouveau mot de passe)
    if (auto* page = findChild<QWidget*>(QStringLiteral("Nmdpb")))
        ui->stackedWidget->setCurrentWidget(page);
}

void MainWindow::on_pushButton_16b_clicked()
{
    // Valider (nouveau mdp) -> loginb
    if (ui->stackedWidget && ui->loginb)
        ui->stackedWidget->setCurrentWidget(ui->loginb);
}

void MainWindow::on_pushButton_17b_clicked()
{
    // Retour -> loginb
    if (ui->stackedWidget && ui->loginb)
        ui->stackedWidget->setCurrentWidget(ui->loginb);
}

void MainWindow::on_pushButton_18b_clicked()
{
    // Retour -> loginb
    if (ui->stackedWidget && ui->loginb)
        ui->stackedWidget->setCurrentWidget(ui->loginb);
}

void MainWindow::on_pushButton_19b_clicked()
{
    // Retour -> loginb
    if (ui->stackedWidget && ui->loginb)
        ui->stackedWidget->setCurrentWidget(ui->loginb);
}

// ═══════════════════════════════════════════════════════════════════════════
//  MENU  NAVIGATION
// ═══════════════════════════════════════════════════════════════════════════

void MainWindow::on_p2b_clicked()
{
    // Menu GBateau -> pagepecheur
    if (ui->stackedWidget && ui->pagepecheur)
        ui->stackedWidget->setCurrentWidget(ui->pagepecheur);
}

void MainWindow::on_p3b_clicked()
{
    // Menu GBateau -> pageclients
    if (ui->stackedWidget && ui->pageclients)
        ui->stackedWidget->setCurrentWidget(ui->pageclients);
}

void MainWindow::on_pushButton_3b_clicked()
{
    // Retour (gestionbateaub) -> menu GBateau
    if (ui->stackedWidget && ui->gestionsb)
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
}

void MainWindow::on_p1_2b_clicked()
{
    // Toggle frame_10b settings panel
    if (ui->frame_10b)
        ui->frame_10b->setVisible(!ui->frame_10b->isVisible());
}

void MainWindow::on_pushButton_12b_clicked()
{
    // Fermer frame_10b
    if (ui->frame_10b)
        ui->frame_10b->hide();
}

void MainWindow::on_pushButton_9b_clicked()
{
    // Déconnexion -> loginb
    if (ui->stackedWidget && ui->loginb)
        ui->stackedWidget->setCurrentWidget(ui->loginb);
}

// ═══════════════════════════════════════════════════════════════════════════
//  PAGE PÊCHEURS  –  FaceID
// ═══════════════════════════════════════════════════════════════════════════

void MainWindow::on_btnFaceIDp_clicked()
{
    // Afficher le panneau FaceID
    if (ui->framefaceidp)
        ui->framefaceidp->show();
}

void MainWindow::on_bmi_6p_clicked()
{
    // Valider FaceID -> fermer panneau
    if (ui->framefaceidp)
        ui->framefaceidp->hide();
}

void MainWindow::on_pushButton_11p_clicked()
{
    // Annuler FaceID -> fermer panneau
    if (ui->framefaceidp)
        ui->framefaceidp->hide();
}

void MainWindow::on_brmp_clicked()
{
    // Page pêcheurs -> retour menu (gestionsb)
    if (ui->stackedWidget && ui->gestionsb)
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
}

// ═══════════════════════════════════════════════════════════════════════════
//  GESTION  DES  QUAIS
// ═══════════════════════════════════════════════════════════════════════════

void MainWindow::on_pushButton_clicked()
{
    // Retour Menu (page_3) -> Menu GBateau
    if (ui->stackedWidget && ui->gestionsb)
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
}

// ═══════════════════════════════════════════════════════════════════════════
//  GESTION  DES  EMPLOYÉS
// ═══════════════════════════════════════════════════════════════════════════

void MainWindow::on_pushButton_10e_clicked()
{
    // Retour Menu (pagee) -> Menu GBateau
    if (ui->stackedWidget && ui->gestionsb)
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
}

void MainWindow::on_pushButton_pdfb_5_clicked()
{
    // Retour menu depuis sidebar
    if (ui->stackedWidget && ui->gestionsb)
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
}

void MainWindow::on_pushButton_7c_5_clicked()
{
    // Placeholder — employés action
}

void MainWindow::on_pushButton_8c_4_clicked()
{
    // Placeholder — employés action
}

void MainWindow::on_pushButton_8c_3_clicked()
{
    // Placeholder — employés action
}

// ═══════════════════════════════════════════════════════════════════════════
//  ALERTES
// ═══════════════════════════════════════════════════════════════════════════

void MainWindow::on_pushButton_7b_clicked()
{
    // Afficher le panneau des alertes (framealertb)
    if (auto *f = findChild<QWidget*>(QStringLiteral("framealertb")))
        f->setVisible(!f->isVisible());
}

// ═══════════════════════════════════════════════════════════════════════════
//  PAGE  CLIENTS  /  CHAT  IA
// ═══════════════════════════════════════════════════════════════════════════

void MainWindow::on_btnai_clicked()
{
    // Toggle panneau chat IA
    if (auto *f = findChild<QWidget*>(QStringLiteral("frame_2c_2")))
        f->setVisible(!f->isVisible());
}

void MainWindow::on_btnSend_2_clicked()
{
    // Fermer panneau chat IA
    if (auto *f = findChild<QWidget*>(QStringLiteral("frame_2c_2")))
        f->hide();
}

void MainWindow::on_pushButton_7c_clicked()
{
    // Retour Menu depuis clients
    if (ui->stackedWidget && ui->gestionsb)
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
}

// ═══════════════════════════════════════════════════════════════════════════
//  MAP  ET  STATISTIQUES
// ═══════════════════════════════════════════════════════════════════════════

void MainWindow::on_pushButton_6_clicked()
{
    // Voir Map
    if (auto *page = findChild<QWidget*>(QStringLiteral("pagemap")))
        ui->stackedWidget->setCurrentWidget(page);
}

void MainWindow::on_pushButton_2_clicked()
{
    // Retour depuis map/stats
    if (ui->stackedWidget && ui->gestionsb)
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
}

void MainWindow::on_pushButton_9_clicked()
{
    // Voir les statistiques
    if (auto *page = findChild<QWidget*>(QStringLiteral("pagestats")))
        ui->stackedWidget->setCurrentWidget(page);
}
