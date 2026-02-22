#include "GQuai.h"
#include "ui_mainwindow.h"
#include <QAbstractButton>
#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDate>
#include <QHBoxLayout>
#include <QWidget>

#include <QDateTime>
#include <QMessageBox>
#include <QDebug>
#include <QMetaType>
#include <QStringList>
// Database
#include "connection.h"
#include "pecheurs.h"
#include <QSqlQuery>
#include <QSqlError>

// Toggle global: activer/désactiver les opérations CRUD.
static const bool kCrudEnabled = true;

static int toIntOrZero(const QString& text)
{
    if (text.trimmed().isEmpty()) return 0;
    
    // Essayer d'abord un entier standard
    bool ok = false;
    int value = text.trimmed().toInt(&ok);
    if (ok) return value;
    
    // Si échoue, essayer double (pour notation scientifique comme "2e+07")
    double dvalue = text.trimmed().toDouble(&ok);
    if (ok) return static_cast<int>(dvalue);
    
    return 0;
}

static int bateauIdFromText(const QString& text)
{
    const int direct = toIntOrZero(text);
    if (direct > 0 || text.trimmed().isEmpty()) return direct;
    QSqlQuery q;
    q.prepare("SELECT ID_Bateau FROM BATEAUX WHERE Nom = :nom");
    q.bindValue(":nom", text.trimmed());
    if (q.exec() && q.next()) return q.value(0).toInt();
    return 0;
}

static QString cleanFilterLabel(QString text)
{
    text = text.trimmed();
    text.replace(QStringLiteral("✅"), QString());
    text.replace(QStringLiteral("❌"), QString());
    text.replace(QStringLiteral("⏳"), QString());
    text.replace(QStringLiteral("🏖️"), QString());
    text.replace(QStringLiteral("•"), QString());
    return text.simplified();
}

static bool isAllRolesSelection(const QString& text)
{
    const QString t = text.toLower();
    return t.contains(QStringLiteral("tous"))
        && (t.contains(QStringLiteral("role")) || t.contains(QStringLiteral("rôle")));
}

static bool isAllDisponibiliteSelection(const QString& text)
{
    const QString t = text.toLower();
    return t.contains(QStringLiteral("toutes")) && t.contains(QStringLiteral("dispon"));
}

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
    , m_editingPecheurId(-1)
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

    // Empêcher la saisie libre pour respecter les contraintes CHECK Oracle
    if (ui->comboBoxp) ui->comboBoxp->setEditable(false);
    if (ui->comboBox_2) ui->comboBox_2->setEditable(false);

    // Initialiser les dates avec la date actuelle pour éviter les valeurs invalides
    const QDate today = QDate::currentDate();
    if (ui->dateTimeEdit) ui->dateTimeEdit->setDate(today);
    if (ui->dateTimeEdit_2) ui->dateTimeEdit_2->setDate(today);

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

Pecheurs MainWindow::pecheurFromForm() const
{
    const int id = toIntOrZero(ui->lineEditp ? ui->lineEditp->text() : QString());
    const QString nom = ui->lineEdit_2p ? ui->lineEdit_2p->text() : QString();
    const QString prenom = ui->lineEdit_3p ? ui->lineEdit_3p->text() : QString();
    const QString role = ui->comboBoxp ? ui->comboBoxp->currentText() : QString();
    const QString dispo = ui->comboBox_2 ? ui->comboBox_2->currentText() : QString();
    const QString email = ui->lineEditp_2 ? ui->lineEditp_2->text() : QString();
    const int heures = ui->dateTimeEdit ? ui->dateTimeEdit->time().hour() : 0;
    const QDate dateInscription = ui->dateTimeEdit ? ui->dateTimeEdit->date() : QDate();
    const QDate dateAffectation = ui->dateTimeEdit_2 ? ui->dateTimeEdit_2->date() : QDate();
    const int idBateau = bateauIdFromText(ui->lineEdit_4p_2 ? ui->lineEdit_4p_2->text() : QString());
    return Pecheurs(id, nom, prenom, role, dispo, email, heures, dateInscription, dateAffectation, idBateau);
}

void MainWindow::loadPecheurs()
{
    qDebug() << "=== loadPecheurs() APPELÉE ===";
    if (!ui || !ui->tableWidgetp) return;

    QSqlQuery query;
    const QString recherche = ui->lineEdit_4p ? ui->lineEdit_4p->text().trimmed().simplified() : QString();
    const QString roleSelection = cleanFilterLabel(ui->comboBox_5p ? ui->comboBox_5p->currentText() : QString());
    const QString dispoSelection = cleanFilterLabel(ui->comboBox_6p ? ui->comboBox_6p->currentText() : QString());

    QStringList whereParts;

    if (!recherche.isEmpty()) {
        const QStringList termes = recherche.split(' ', Qt::SkipEmptyParts);
        if (termes.size() >= 2) {
            const QString nomTerme = termes.at(0);
            const QString prenomTerme = termes.mid(1).join(" ");
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

    QString sql =
        "SELECT ID_Pecheur, Nom_Pecheur, Prenom_Pecheur, Role, ID_Bateau, "
        "Disponibilite, Email, Date_Inscription, Date_Affectation, Heures, Photo "
        "FROM PECHEURS";
    if (!whereParts.isEmpty()) {
        sql += " WHERE " + whereParts.join(" AND ");
    }
    sql += " ORDER BY ID_Pecheur";
    query.prepare(sql);

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
        query.bindValue(QStringLiteral(":role"), roleSelection);
    }
    if (!dispoSelection.isEmpty() && !isAllDisponibiliteSelection(dispoSelection)) {
        query.bindValue(QStringLiteral(":dispo"), dispoSelection);
    }

    if (!query.exec()) {
        qDebug() << "Erreur recherche/loadPecheurs:" << query.lastError().text();
    }

    auto* table = ui->tableWidgetp;
    table->setRowCount(0);
    qDebug() << "Table vidée, en train de recharger...";

    int row = 0;
    while (query.next()) {
        const int currentRow = row;
        table->insertRow(row);
        const QString id = query.value(0).toString();
        const QString nom = query.value(1).toString();
        const QString prenom = query.value(2).toString();
        const QString role = query.value(3).toString();
        
        qDebug() << "Row" << row << "ID:" << id << "Nom:" << nom << "Prenom:" << prenom << "Role:" << role;
        
        table->setItem(row, 0, new QTableWidgetItem(id));
        table->setItem(row, 1, new QTableWidgetItem(nom));
        table->setItem(row, 2, new QTableWidgetItem(prenom));
        table->setItem(row, 3, new QTableWidgetItem(role));
        table->setItem(row, 4, new QTableWidgetItem(query.value(4).toString()));
        table->setItem(row, 5, new QTableWidgetItem(query.value(5).toString()));
        table->setItem(row, 6, new QTableWidgetItem(query.value(6).toString()));
        const QDate dIns = query.value(7).toDate();
        const QDate dAff = query.value(8).toDate();
        const int heures = query.value(9).toInt();
        const QString heureTexte = QStringLiteral(" %1:00").arg(qBound(0, heures, 23), 2, 10, QChar('0'));
        table->setItem(row, 7, new QTableWidgetItem(dIns.isValid() ? dIns.toString("dd/MM/yyyy") + heureTexte : QString()));
        table->setItem(row, 8, new QTableWidgetItem(dAff.isValid() ? dAff.toString("dd/MM/yyyy") + heureTexte : QString()));
        table->setItem(row, 9, new QTableWidgetItem(QString::number(heures)));
        table->setItem(row, 10, new QTableWidgetItem(QString()));
        
        // Créer un widget conteneur pour les boutons d'action
        auto* actionWidget = new QWidget();
        actionWidget->setStyleSheet("background-color: transparent;");
        auto* actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(2, 0, 2, 12);
        actionLayout->setSpacing(5);
        
        // Bouton Modifier (style pushButton_6p)
        auto* btnEdit = new QPushButton("✏️");
        btnEdit->setMinimumWidth(35);
        btnEdit->setMinimumHeight(35);
        btnEdit->setMaximumWidth(35);
        btnEdit->setMaximumHeight(35);
        btnEdit->setStyleSheet("QPushButton { border: none; background-color: transparent; font-size: 18px; padding: 0px; margin: 0px; } QPushButton:hover { background-color: rgba(52, 152, 219, 0.05); }");
        connect(btnEdit, &QPushButton::clicked, this, [this, currentRow]() {
            if (ui && ui->tableWidgetp && currentRow < ui->tableWidgetp->rowCount()) {
                ui->tableWidgetp->selectRow(currentRow);
                on_pushButton_6p_clicked();
            }
        });
        
        // Bouton Supprimer (style pushButton_5p)
        auto* btnDelete = new QPushButton("❌");
        btnDelete->setMinimumWidth(35);
        btnDelete->setMinimumHeight(35);
        btnDelete->setMaximumWidth(35);
        btnDelete->setMaximumHeight(35);
        btnDelete->setStyleSheet("QPushButton { border: none; background-color: transparent; font-size: 18px; padding: 0px; margin: 0px; } QPushButton:hover { background-color: rgba(231, 76, 60, 0.05); }");
        connect(btnDelete, &QPushButton::clicked, this, [this, currentRow]() {
            if (ui && ui->tableWidgetp && currentRow < ui->tableWidgetp->rowCount()) {
                ui->tableWidgetp->selectRow(currentRow);
                on_pushButton_5p_clicked();
            }
        });
        
        actionLayout->addStretch();
        actionLayout->addWidget(btnEdit, 0, Qt::AlignCenter);
        actionLayout->addWidget(btnDelete, 0, Qt::AlignCenter);
        actionLayout->addStretch();
        
        table->setCellWidget(row, 11, actionWidget);
        ++row;
    }
    
    qDebug() << "TOTAL de lignes chargées:" << row;
    
    // Définir des largeurs spécifiques pour chaque colonne (total ~1290px)
    table->setColumnWidth(0, 60);   // ID
    table->setColumnWidth(1, 100);  // Nom
    table->setColumnWidth(2, 100);  // Prenom
    table->setColumnWidth(3, 140);  // Rôle
    table->setColumnWidth(4, 70);   // Bateaux
    table->setColumnWidth(5, 150);  // Disponibilité
    table->setColumnWidth(6, 180);  // Email
    table->setColumnWidth(7, 130);  // Date Inscription
    table->setColumnWidth(8, 130);  // Date Affectation
    table->setColumnWidth(9, 70);   // Heures
    table->setColumnWidth(10, 60);  // Photo
    table->setColumnWidth(11, 100); // Action
}

void MainWindow::on_lineEdit_4p_textChanged(const QString &text)
{
    Q_UNUSED(text);
    loadPecheurs();
}

void MainWindow::on_comboBox_5p_currentTextChanged(const QString &text)
{
    Q_UNUSED(text);
    loadPecheurs();
}

void MainWindow::on_comboBox_6p_currentTextChanged(const QString &text)
{
    Q_UNUSED(text);
    loadPecheurs();
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
void MainWindow::showQrFrameLegacy()
{
    showFrame(ui->frame_3b);
}

// Bouton Connexion : afficher frame Connexion, cacher frame QR
void MainWindow::showLoginFrameLegacy()
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
    }

    loadPecheurs();
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

void MainWindow::openAiPanelLegacy()
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

void MainWindow::closeAiPanelLegacy()
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

void MainWindow::on_bap_clicked()
{
    qDebug() << "=== on_bap_clicked() APPELÉE ===";
    qDebug() << "m_editingPecheurId ACTUEL:" << m_editingPecheurId;
    qDebug() << "Texte du bouton bap:" << (ui && ui->bap ? ui->bap->text() : "nullptr");
    
    if (!handleCrudDisabled(this)) return;
    const Pecheurs p = pecheurFromForm();
    const int id = toIntOrZero(ui && ui->lineEditp ? ui->lineEditp->text() : QString());
    qDebug() << "ID du formulaire:" << id;
    
    if (id <= 0 || (ui && ui->lineEdit_2p && ui->lineEdit_2p->text().trimmed().isEmpty())) {
        QMessageBox::warning(this, "Champs", "ID et Nom obligatoires");
        return;
    }
    
    // Vérifier si on est en mode édition (modification)
    if (m_editingPecheurId > 0) {
        // Mode MODIFICATION
        qDebug() << ">>> Mode MODIFICATION ACTIVÉ <<<";
        qDebug() << "Mode MODIFICATION: ID en édition:" << m_editingPecheurId << "ID du formulaire:" << id;
        // IMPORTANT: Utiliser l'ID original du pêcheur sélectionné (m_editingPecheurId), pas celui du formulaire
        // Le champ ID est maintenant en lecture seule, mais on force la valeur correcte par sécurité
        Pecheurs pForModify = p;
        pForModify.setId(m_editingPecheurId);
        qDebug() << "Pêcheur à modifier: ID=" << m_editingPecheurId << "(au lieu de " << id << " du formulaire)";
        if (pForModify.modifier()) {
            qDebug() << "p.modifier() a réussi!";
            loadPecheurs();
            resetAjouterButton();
            QMessageBox::information(this, "OK", "Modification reussie");
        } else {
            qDebug() << "p.modifier() a échoué!" << Pecheurs::lastError();
            QMessageBox::critical(this, "Erreur", "Modification echouee: " + Pecheurs::lastError());
        }
    } else {
        // Mode AJOUT
        qDebug() << ">>> Mode AJOUT ACTIVÉ <<<";
        qDebug() << "Mode AJOUT: ID en édition:" << m_editingPecheurId;
        QSqlQuery existsQuery;
        existsQuery.prepare("SELECT 1 FROM PECHEURS WHERE ID_Pecheur = :id");
        existsQuery.bindValue(":id", id);
        if (existsQuery.exec() && existsQuery.next()) {
            QMessageBox::warning(this, "Doublon", "Cet ID existe déjà. Veuillez saisir un ID différent.");
            return;
        }

        if (p.ajouter()) {
            qDebug() << "p.ajouter() a réussi!";
            loadPecheurs();
            resetAjouterButton();
            QMessageBox::information(this, "OK", "Ajout reussi");
        } else {
            qDebug() << "p.ajouter() a échoué!" << Pecheurs::lastError();
            QMessageBox::critical(this, "Erreur", "Ajout echoue: " + Pecheurs::lastError());
        }
    }
}

void MainWindow::loadPecheurFromTable()
{
    if (!ui || !ui->tableWidgetp) return;
    
    // Obtenir la ligne sélectionnée
    const int currentRow = ui->tableWidgetp->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Selection", "Veuillez sélectionner une ligne");
        return;
    }
    
    // Extraire les données de chaque colonne
    const QString id = ui->tableWidgetp->item(currentRow, 0) ? ui->tableWidgetp->item(currentRow, 0)->text() : QString();
    const QString nom = ui->tableWidgetp->item(currentRow, 1) ? ui->tableWidgetp->item(currentRow, 1)->text() : QString();
    const QString prenom = ui->tableWidgetp->item(currentRow, 2) ? ui->tableWidgetp->item(currentRow, 2)->text() : QString();
    const QString role = ui->tableWidgetp->item(currentRow, 3) ? ui->tableWidgetp->item(currentRow, 3)->text() : QString();
    const QString idBateau = ui->tableWidgetp->item(currentRow, 4) ? ui->tableWidgetp->item(currentRow, 4)->text() : QString();
    const QString dispo = ui->tableWidgetp->item(currentRow, 5) ? ui->tableWidgetp->item(currentRow, 5)->text() : QString();
    const QString email = ui->tableWidgetp->item(currentRow, 6) ? ui->tableWidgetp->item(currentRow, 6)->text() : QString();
    const QString dateInsStr = ui->tableWidgetp->item(currentRow, 7) ? ui->tableWidgetp->item(currentRow, 7)->text() : QString();
    const QString dateAffStr = ui->tableWidgetp->item(currentRow, 8) ? ui->tableWidgetp->item(currentRow, 8)->text() : QString();
    const int heures = toIntOrZero(ui->tableWidgetp->item(currentRow, 9) ? ui->tableWidgetp->item(currentRow, 9)->text() : QString());
    
    // Remplir les champs du formulaire
    if (ui->lineEditp) ui->lineEditp->setText(id);
    if (ui->lineEdit_2p) ui->lineEdit_2p->setText(nom);
    if (ui->lineEdit_3p) ui->lineEdit_3p->setText(prenom);
    
    // Mettre à jour le comboBox Role
    if (ui->comboBoxp) {
        const int roleIndex = ui->comboBoxp->findText(role);
        if (roleIndex >= 0) {
            ui->comboBoxp->setCurrentIndex(roleIndex);
        }
    }
    
    // Mettre à jour le comboBox Disponibilité
    if (ui->comboBox_2) {
        const int dispoIndex = ui->comboBox_2->findText(dispo);
        if (dispoIndex >= 0) {
            ui->comboBox_2->setCurrentIndex(dispoIndex);
        }
    }
    
    if (ui->lineEditp_2) ui->lineEditp_2->setText(email);
    if (ui->lineEdit_4p_2) ui->lineEdit_4p_2->setText(idBateau);
    
    // Remplir les dates
    if (ui->dateTimeEdit && !dateInsStr.isEmpty()) {
        QDateTime dtIns = QDateTime::fromString(dateInsStr, "dd/MM/yyyy HH:mm");
        if (!dtIns.isValid()) {
            const QDate dIns = QDate::fromString(dateInsStr, "dd/MM/yyyy");
            if (dIns.isValid()) {
                dtIns = QDateTime(dIns, QTime(qBound(0, heures, 23), 0, 0));
            }
        }
        if (dtIns.isValid()) {
            ui->dateTimeEdit->setDateTime(dtIns);
        }
    }
    if (ui->dateTimeEdit_2 && !dateAffStr.isEmpty()) {
        QDateTime dtAff = QDateTime::fromString(dateAffStr, "dd/MM/yyyy HH:mm");
        if (!dtAff.isValid()) {
            const QDate dAff = QDate::fromString(dateAffStr, "dd/MM/yyyy");
            if (dAff.isValid()) {
                dtAff = QDateTime(dAff, QTime(qBound(0, heures, 23), 0, 0));
            }
        }
        if (dtAff.isValid()) {
            ui->dateTimeEdit_2->setDateTime(dtAff);
        }
    }
    
    // Mémoriser l'ID du pêcheur en édition (l'ID original sélectionné)
    m_editingPecheurId = toIntOrZero(id);
    qDebug() << "loadPecheurFromTable: m_editingPecheurId =" << m_editingPecheurId << "ID extrait =" << id;
    
    // Rendre le champ ID en lecture seule (ne pas modifier une clé primaire)
    if (ui && ui->lineEditp) {
        ui->lineEditp->setReadOnly(true);
        qDebug() << "Champ ID mis en lecture seule";
    }
    
    // Changer le bouton "Ajouter" en "Modifier"
    if (ui && ui->bap) {
        ui->bap->setText("Modifier");
        qDebug() << "Bouton changé à: Modifier";
    }
}

void MainWindow::resetAjouterButton()
{
    qDebug() << "=== resetAjouterButton() appelée ===";
    qDebug() << "m_editingPecheurId AVANT reset:" << m_editingPecheurId;
    // Réinitialiser le bouton à "Ajouter"
    if (ui && ui->bap) {
        ui->bap->setText("Ajouter");
        qDebug() << "Bouton bap remis à: Ajouter";
    }
    // Rendre le champ ID à nouveau éditable
    if (ui && ui->lineEditp) {
        ui->lineEditp->setReadOnly(false);
        qDebug() << "Champ ID remis éditable";
    }
    m_editingPecheurId = -1;
    qDebug() << "m_editingPecheurId APRÈS reset:" << m_editingPecheurId;
}

void MainWindow::on_pushButton_6p_clicked()
{
    if (!handleCrudDisabled(this)) return;
    
    // Si aucun pêcheur n'est en édition ET une ligne est sélectionnée = Mode CHARGEMENT
    if (m_editingPecheurId <= 0 && ui && ui->tableWidgetp && ui->tableWidgetp->currentRow() >= 0) {
        // Charger les données du tableau dans le formulaire
        loadPecheurFromTable();
        QMessageBox::information(this, "Édition", "Formulaire rempli - modifiez les champs et cliquez sur Modifier pour sauvegarder");
    } 
    // Sinon = Mode SAUVEGARDE
    else {
        const Pecheurs p = pecheurFromForm();
        if (p.modifier()) {
            loadPecheurs();
            m_editingPecheurId = -1;  // Réinitialiser l'état
            QMessageBox::information(this, "OK", "Modification reussie");
        } else {
            QMessageBox::critical(this, "Erreur", "Modification echouee: " + Pecheurs::lastError());
        }
    }
}

void MainWindow::on_pushButton_5p_clicked()
{
    if (!handleCrudDisabled(this)) return;
    
    // Vérifier qu'une ligne est sélectionnée dans le tableau
    if (!ui || !ui->tableWidgetp) return;
    
    const int currentRow = ui->tableWidgetp->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Sélection", "Veuillez sélectionner un pêcheur à supprimer");
        return;
    }
    
    // Récupérer l'ID de la ligne sélectionnée
    QTableWidgetItem* idItem = ui->tableWidgetp->item(currentRow, 0);
    if (!idItem) {
        QMessageBox::warning(this, "Erreur", "Impossible de récupérer l'ID du pêcheur");
        return;
    }
    
    const int id = toIntOrZero(idItem->text());
    if (id <= 0) {
        QMessageBox::warning(this, "ID", "ID invalide");
        return;
    }
    
    // Récupérer le nom et prénom pour la confirmation
    const QString nom = ui->tableWidgetp->item(currentRow, 1) ? ui->tableWidgetp->item(currentRow, 1)->text() : QString();
    const QString prenom = ui->tableWidgetp->item(currentRow, 2) ? ui->tableWidgetp->item(currentRow, 2)->text() : QString();
    
    // Demander confirmation
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        "Confirmation", 
        QString("Voulez-vous vraiment supprimer le pêcheur :\n%1 %2 (ID: %3) ?").arg(nom).arg(prenom).arg(id),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply != QMessageBox::Yes) {
        return;  // Annulation
    }
    
    // Supprimer de la base de données
    if (Pecheurs::supprimer(id)) {
        qDebug() << "Pêcheur supprimé avec succès - ID:" << id;
        loadPecheurs();  // Recharger le tableau
        m_editingPecheurId = -1;  // Réinitialiser l'état
        QMessageBox::information(this, "OK", "Suppression réussie");
    } else {
        qDebug() << "Échec de suppression - ID:" << id << "Erreur:" << Pecheurs::lastError();
        QMessageBox::critical(this, "Erreur", "Suppression échouée: " + Pecheurs::lastError());
    }
}

void MainWindow::on_pushButton_b_clicked()
{
    // Connexion -> aller au menu GBateau
    if (ui->stackedWidget && ui->gestionsb) {
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
    }
}
void MainWindow::goToMenuFromLegacyLogin()
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

void MainWindow::goToMenuFromLegacyGestionBateau()
{
    // Bouton "Retour" sur la page gestionbateaub : revenir au menu GBateau
    if (ui->stackedWidget && ui->gestionsb) {
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
    }
}

void MainWindow::toggleAlertsPanelLegacy()
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

