#include "mainwindow.h"
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
#include <QLayout>

#include <QDateTime>
#include <QMessageBox>
#include <QDebug>
#include <QMetaType>
#include <QStringList>
#include <QPainter>
#include <QPixmap>
#include <QPdfWriter>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QPageSize>
#include <QPageLayout>
#include <QFontMetrics>
#include <QApplication>
// Database
#include "connection.h"
#include "pecheurs.h"

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
    return Pecheurs::bateauIdFromText(text);
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

static QPixmap buildDisponibiliteCirclePixmap(int disponible, int bientot, int indisponible, int enConge, int size)
{
    const int separatorWidth = 2;
    const int outerBorderWidth = 4;

    const int safeSize = qMax(40, size);
    QPixmap pixmap(safeSize, safeSize);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPen separatorPen(QColor(255, 255, 255), separatorWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(separatorPen);

    const qreal inset = static_cast<qreal>(outerBorderWidth) / 2.0;
    const QRectF pieRect(inset, inset, safeSize - 2.0 * inset, safeSize - 2.0 * inset);
    const int total = disponible + bientot + indisponible + enConge;
    const int totalUnits = 360 * 16;

    int sDisponible = 0;
    int sBientot = 0;
    int sIndisponible = 0;
    int sEnConge = 0;

    if (total > 0) {
        sDisponible = qRound((static_cast<double>(disponible) / static_cast<double>(total)) * totalUnits);
        sBientot = qRound((static_cast<double>(bientot) / static_cast<double>(total)) * totalUnits);
        sIndisponible = qRound((static_cast<double>(indisponible) / static_cast<double>(total)) * totalUnits);
        sEnConge = qMax(0, totalUnits - (sDisponible + sBientot + sIndisponible));
    } else {
        sDisponible = totalUnits / 4;
        sBientot = totalUnits / 4;
        sIndisponible = totalUnits / 4;
        sEnConge = totalUnits - (sDisponible + sBientot + sIndisponible);
    }

    QVector<QPair<QColor, int>> slices = {
        { QColor(QStringLiteral("#00C853")), sDisponible },
        { QColor(QStringLiteral("#007BFF")), sBientot },
        { QColor(QStringLiteral("#FF1744")), sIndisponible },
        { QColor(QStringLiteral("#9b59b6")), sEnConge }
    };

    int startAngle = 90 * 16;
    for (const auto& slice : slices) {
        if (slice.second <= 0) {
            continue;
        }
        painter.setBrush(slice.first);
        const int span = -slice.second;
        painter.drawPie(pieRect, startAngle, span);
        startAngle += span;
    }

    painter.setBrush(Qt::NoBrush);
    QPen borderPen(QColor(255, 255, 255), outerBorderWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(borderPen);
    painter.drawEllipse(pieRect);

    return pixmap;
}

static void updateDisponibiliteStats(
    Ui::MainWindow* ui,
    const QString& recherche,
    const QString& roleSelection,
    const QString& dispoSelection)
{
    if (!ui) return;

    const Pecheurs::DisponibiliteStats stats = Pecheurs::calculerDisponibiliteStats(recherche, roleSelection, dispoSelection);
    const int disponible = stats.disponible;
    const int bientot = stats.bientot;
    const int indisponible = stats.indisponible;
    const int enConge = stats.enConge;

    const int total = disponible + bientot + indisponible + enConge;
    const auto pct = [total](int value) {
        if (total <= 0) return QStringLiteral("0.0");
        return QString::number((100.0 * static_cast<double>(value)) / static_cast<double>(total), 'f', 1);
    };

    if (ui->label_legend_chalutierp) {
        ui->label_legend_chalutierp->setText(
            QStringLiteral("• Disponible: %1 (%2%)").arg(disponible).arg(pct(disponible)));
    }
    if (ui->label_legend_palangrierp) {
        ui->label_legend_palangrierp->setText(
            QStringLiteral("• Disponible bientot: %1 (%2%)").arg(bientot).arg(pct(bientot)));
    }
    if (ui->label_legend_caseyeurp) {
        ui->label_legend_caseyeurp->setText(
            QStringLiteral("• Indisponible: %1 (%2%)").arg(indisponible).arg(pct(indisponible)));
    }
    if (ui->label_legend_traditionalp) {
        ui->label_legend_traditionalp->setText(
            QStringLiteral("• En conge: %1 (%2%)").arg(enConge).arg(pct(enConge)));
    }
    if (ui->progressTypeCirclep) {
        const int size = qMin(ui->progressTypeCirclep->width(), ui->progressTypeCirclep->height());
        ui->progressTypeCirclep->setStyleSheet(QStringLiteral("border: none; background: transparent;"));
        ui->progressTypeCirclep->setPixmap(buildDisponibiliteCirclePixmap(disponible, bientot, indisponible, enConge, size));
    }
}

static inline bool handleCrudDisabled(QWidget* parent)
{
    if (kCrudEnabled) return true;
    QMessageBox::information(parent, QStringLiteral("CRUD désactivé"), QStringLiteral("Les opérations CRUD sont désactivées dans cette build."));
    return false;
}

static bool isSaisieConstraintError(const QString& error)
{
    const QString e = error.toLower();
    return e.contains(QStringLiteral("obligatoire"))
        || e.contains(QStringLiteral("invalide"))
        || e.contains(QStringLiteral("déjà utilisé"))
        || e.contains(QStringLiteral("deja utilise"))
        || e.contains(QStringLiteral("veuillez saisir"))
        || e.contains(QStringLiteral("format attendu"))
        || e.contains(QStringLiteral("doit être"))
        || e.contains(QStringLiteral("doit etre"));
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

    const QString recherche = ui->lineEdit_4p ? ui->lineEdit_4p->text().trimmed().simplified() : QString();
    const QString roleSelection = cleanFilterLabel(ui->comboBox_5p ? ui->comboBox_5p->currentText() : QString());
    const QString dispoSelection = cleanFilterLabel(ui->comboBox_6p ? ui->comboBox_6p->currentText() : QString());

    QVector<Pecheurs::TableRowData> rows;
    if (!Pecheurs::chargerTable(recherche, roleSelection, dispoSelection, rows)) {
        qDebug() << "Erreur recherche/loadPecheurs:" << Pecheurs::lastError();
    }

    auto* table = ui->tableWidgetp;
    table->setAlternatingRowColors(true);
    table->setShowGrid(false);
    table->setStyleSheet(
        "QTableWidget {"
        "background-color: #ffffff;"
        "alternate-background-color: #ebebeb;"
        "border: none;"
        "}"
        "QTableWidget::item {"
        "padding: 4px;"
        "border: none;"
        "}"
        "QTableWidget::item:selected {"
        "background-color: rgba(0, 85, 127, 0.20);"
        "color: #0b2d4a;"
        "}");
    table->setRowCount(0);
    qDebug() << "Table vidée, en train de recharger...";

    int row = 0;
    for (const Pecheurs::TableRowData& record : rows) {
        const int currentRow = row;
        table->insertRow(row);
        const QString id = QString::number(record.id);
        const QString nom = record.nom;
        const QString prenom = record.prenom;
        const QString role = record.role;
        
        qDebug() << "Row" << row << "ID:" << id << "Nom:" << nom << "Prenom:" << prenom << "Role:" << role;
        
        table->setItem(row, 0, new QTableWidgetItem(id));
        table->setItem(row, 1, new QTableWidgetItem(nom));
        table->setItem(row, 2, new QTableWidgetItem(prenom));
        table->setItem(row, 3, new QTableWidgetItem(role));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(record.idBateau)));
        table->setItem(row, 5, new QTableWidgetItem(record.disponibilite));
        table->setItem(row, 6, new QTableWidgetItem(record.email));
        const QDate dIns = record.dateInscription;
        const QDate dAff = record.dateAffectation;
        const int heures = record.heures;
        const QString heureTexte = QStringLiteral(" %1:00").arg(qBound(0, heures, 23), 2, 10, QChar('0'));
        table->setItem(row, 7, new QTableWidgetItem(dIns.isValid() ? dIns.toString("dd/MM/yyyy") + heureTexte : QString()));
        table->setItem(row, 8, new QTableWidgetItem(dAff.isValid() ? dAff.toString("dd/MM/yyyy") + heureTexte : QString()));
        table->setItem(row, 9, new QTableWidgetItem(QString::number(heures)));
        table->setItem(row, 10, new QTableWidgetItem(QString()));
        
        // Créer un widget conteneur pour les boutons d'action
        auto* actionWidget = new QWidget();
        actionWidget->setStyleSheet("background-color: transparent;");
        auto* actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(3, 0, 3, 6);
        actionLayout->setSpacing(4);
        
        // Bouton Modifier (style pushButton_6p)
        auto* btnEdit = new QPushButton("📝");
        btnEdit->setFixedSize(40, 24);
        btnEdit->setStyleSheet(
            "QPushButton { "
            "border: 2px solid rgb(0, 0, 112); "
            "border-radius: 4px; "
            "background-color: transparent; "
            "color: rgb(0, 0, 112); "
            "font-size: 16px; "
            "padding: 0px; margin: 0px; } "
            "QPushButton:hover { background-color: rgba(0, 0, 112, 0.08); border-color: #59abc8; } "
            "QPushButton:pressed { background-color: rgba(0, 0, 112, 0.15); }");
        connect(btnEdit, &QPushButton::clicked, this, [this, currentRow]() {
            if (ui && ui->tableWidgetp && currentRow < ui->tableWidgetp->rowCount()) {
                ui->tableWidgetp->selectRow(currentRow);
                on_pushButton_6p_clicked();
            }
        });
        
        // Bouton Supprimer (style pushButton_5p)
        auto* btnDelete = new QPushButton("❌");
        btnDelete->setFixedSize(40, 24);
        btnDelete->setStyleSheet(
            "QPushButton { "
            "border: 2px solid rgb(0, 0, 112); "
            "border-radius: 4px; "
            "background-color: transparent; "
            "color: rgb(0, 0, 112); "
            "font-size: 16px; "
            "padding: 0px; margin: 0px; } "
            "QPushButton:hover { background-color: rgba(0, 0, 112, 0.08); border-color: #59abc8; } "
            "QPushButton:pressed { background-color: rgba(0, 0, 112, 0.15); }");
        connect(btnDelete, &QPushButton::clicked, this, [this, currentRow]() {
            if (ui && ui->tableWidgetp && currentRow < ui->tableWidgetp->rowCount()) {
                ui->tableWidgetp->selectRow(currentRow);
                on_pushButton_5p_clicked();
            }
        });
        
        actionLayout->addWidget(btnEdit, 0, Qt::AlignHCenter | Qt::AlignTop);
        actionLayout->addWidget(btnDelete, 0, Qt::AlignHCenter | Qt::AlignTop);
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
    table->verticalHeader()->setDefaultSectionSize(34);

    // Mettre à jour les statistiques de disponibilité selon les filtres actifs.
    updateDisponibiliteStats(ui, recherche, roleSelection, dispoSelection);
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

static QPixmap captureFullTableWidgetPixmap(QTableWidget* table)
{
    if (!table) return QPixmap();

    int fullWidth = table->frameWidth() * 2 + table->verticalHeader()->width();
    for (int c = 0; c < table->columnCount(); ++c) {
        if (!table->isColumnHidden(c)) {
            fullWidth += table->columnWidth(c);
        }
    }

    int fullHeight = table->frameWidth() * 2 + table->horizontalHeader()->height();
    for (int r = 0; r < table->rowCount(); ++r) {
        if (!table->isRowHidden(r)) {
            fullHeight += table->rowHeight(r);
        }
    }

    fullWidth = qMax(fullWidth, 400);
    fullHeight = qMax(fullHeight, 120);

    const QSize oldSize = table->size();
    const QSize oldMin = table->minimumSize();
    const QSize oldMax = table->maximumSize();

    table->setMinimumSize(fullWidth, fullHeight);
    table->setMaximumSize(fullWidth, fullHeight);
    table->resize(fullWidth, fullHeight);
    qApp->processEvents();

    QPixmap shot = table->grab();

    table->setMinimumSize(oldMin);
    table->setMaximumSize(oldMax);
    table->resize(oldSize);
    qApp->processEvents();

    return shot;
}

static QPixmap captureWidgetForPdf(QWidget* widget,
                                   const QSize& minTargetSize = QSize(900, 520),
                                   int scaleFactor = 2)
{
    if (!widget) return QPixmap();

    QSize targetSize = widget->size();
    targetSize = targetSize.expandedTo(widget->minimumSizeHint());
    if (widget->layout()) {
        targetSize = targetSize.expandedTo(widget->layout()->sizeHint());
    }
    targetSize = targetSize.expandedTo(minTargetSize);
    targetSize.setWidth(qMax(1, targetSize.width()));
    targetSize.setHeight(qMax(1, targetSize.height()));

    const QSize oldSize = widget->size();
    const QSize oldMin = widget->minimumSize();
    const QSize oldMax = widget->maximumSize();

    widget->setMinimumSize(targetSize);
    widget->setMaximumSize(targetSize);
    widget->resize(targetSize);
    qApp->processEvents();

    const int safeScale = qMax(1, scaleFactor);
    QPixmap shot(targetSize.width() * safeScale, targetSize.height() * safeScale);
    shot.setDevicePixelRatio(static_cast<qreal>(safeScale));
    shot.fill(Qt::white);

    QPainter pixPainter(&shot);
    widget->render(&pixPainter, QPoint(), QRegion(), QWidget::DrawWindowBackground | QWidget::DrawChildren);
    pixPainter.end();

    widget->setMinimumSize(oldMin);
    widget->setMaximumSize(oldMax);
    widget->resize(oldSize);
    qApp->processEvents();

    return shot;
}

static int findActionColumnIndex(const QTableWidget* table)
{
    if (!table) return -1;

    for (int c = 0; c < table->columnCount(); ++c) {
        QTableWidgetItem* headerItem = table->horizontalHeaderItem(c);
        if (!headerItem) continue;

        const QString headerText = headerItem->text().trimmed();
        if (headerText.compare(QStringLiteral("Action"), Qt::CaseInsensitive) == 0
            || headerText.contains(QStringLiteral("Action"), Qt::CaseInsensitive)) {
            return c;
        }
    }

    return -1;
}

static void drawPixmapWithPagination(QPdfWriter& writer,
                                     QPainter& painter,
                                     const QPixmap& source,
                                     int margin,
                                     int& y,
                                     int blockSpacing = 18)
{
    if (source.isNull()) return;

    const int pageW = writer.width();
    const int pageH = writer.height();
    const int contentW = pageW - (2 * margin);

    if (contentW <= 0 || source.width() <= 0 || source.height() <= 0) return;

    const double scale = static_cast<double>(contentW) / static_cast<double>(source.width());
    int remainingSrcY = 0;

    while (remainingSrcY < source.height()) {
        int availableH = pageH - margin - y;
        if (availableH <= 40) {
            writer.newPage();
            y = margin;
            availableH = pageH - margin - y;
        }

        int srcChunkH = static_cast<int>(availableH / scale);
        srcChunkH = qMax(1, srcChunkH);
        srcChunkH = qMin(srcChunkH, source.height() - remainingSrcY);

        const int targetChunkH = qMax(1, static_cast<int>(srcChunkH * scale));
        const QRect targetRect(margin, y, contentW, targetChunkH);
        const QRect sourceRect(0, remainingSrcY, source.width(), srcChunkH);
        painter.drawPixmap(targetRect, source, sourceRect);

        y += targetChunkH;
        remainingSrcY += srcChunkH;

        if (remainingSrcY < source.height()) {
            writer.newPage();
            y = margin;
        } else {
            y += blockSpacing;
        }
    }
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

static void exportPecheursPdfReport(MainWindow* parent, Ui::MainWindow* ui)
{
    if (!parent || !ui || !ui->tableWidgetp) return;

    const QString documentsDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString baseDir = documentsDir.isEmpty() ? QDir::homePath() : documentsDir;
    const QString defaultName = QStringLiteral("pecheurs_statistiques_%1.pdf")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")));
    QString filePath = QFileDialog::getSaveFileName(
        parent,
        QStringLiteral("Exporter PDF"),
        QDir(baseDir).filePath(defaultName),
        QStringLiteral("PDF Files (*.pdf)"));

    if (filePath.trimmed().isEmpty()) {
        return;
    }
    if (!filePath.toLower().endsWith(QStringLiteral(".pdf"))) {
        filePath += QStringLiteral(".pdf");
    }

    QPdfWriter writer(filePath);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageOrientation(QPageLayout::Landscape);
    writer.setResolution(300);
    writer.setPageMargins(QMarginsF(8, 8, 8, 8), QPageLayout::Millimeter);

    QPainter painter(&writer);
    if (!painter.isActive()) {
        QMessageBox::critical(parent, QStringLiteral("Erreur"), QStringLiteral("Impossible de créer le fichier PDF."));
        return;
    }

    const int actionColumn = findActionColumnIndex(ui->tableWidgetp);
    const bool actionColumnWasHidden = (actionColumn >= 0) ? ui->tableWidgetp->isColumnHidden(actionColumn) : false;
    if (actionColumn >= 0 && !actionColumnWasHidden) {
        ui->tableWidgetp->setColumnHidden(actionColumn, true);
    }

    auto restoreActionColumn = [ui, actionColumn, actionColumnWasHidden]() {
        if (actionColumn >= 0 && !actionColumnWasHidden) {
            ui->tableWidgetp->setColumnHidden(actionColumn, false);
        }
    };

    const QString exportStamp = QDateTime::currentDateTime().toString(QStringLiteral("dd/MM/yyyy HH:mm"));
    const int pageW = writer.width();
    const int pageH = writer.height();
    const int margin = 54;
    const int contentW = pageW - (2 * margin);
    int y = margin;

    QFont titleFont(QStringLiteral("Arial"), 20, QFont::Bold);
    QFont subtitleFont(QStringLiteral("Arial"), 11, QFont::Normal);
    QFont headerFont(QStringLiteral("Arial"), 9, QFont::Bold);
    QFont cellFont(QStringLiteral("Arial"), 8, QFont::Normal);
    QFont sectionFont(QStringLiteral("Arial"), 19, QFont::Bold);
    QFont totalFont(QStringLiteral("Arial"), 12, QFont::Bold);
    QFont footerFont(QStringLiteral("Arial"), 9, QFont::Normal);

    painter.fillRect(QRect(0, 0, pageW, pageH), QColor(QStringLiteral("#f7f7f7")));

    painter.setPen(QColor(QStringLiteral("#0b1b8f")));
    painter.setFont(titleFont);
    painter.drawText(QRect(margin, y, contentW, 88), Qt::AlignLeft | Qt::AlignVCenter,
                     QStringLiteral("AQUATEC — Liste des Pecheurs"));
    y += 96;

    painter.setPen(QColor(QStringLiteral("#6e6e6e")));
    painter.setFont(subtitleFont);
    painter.drawText(QRect(margin, y, contentW, 28), Qt::AlignHCenter | Qt::AlignVCenter,
                     QStringLiteral("Exporté le %1").arg(exportStamp));
    y += 42;

    QTableWidget* table = ui->tableWidgetp;
    QVector<int> columns;
    QVector<int> sourceWidths;
    int totalSourceW = 0;

    for (int c = 0; c < table->columnCount(); ++c) {
        if (table->isColumnHidden(c)) continue;
        if (c == actionColumn) continue;
        columns.push_back(c);
        const int w = qMax(40, table->columnWidth(c));
        sourceWidths.push_back(w);
        totalSourceW += w;
    }

    if (columns.isEmpty()) {
        restoreActionColumn();
        painter.end();
        QMessageBox::warning(parent, QStringLiteral("Export PDF"),
                             QStringLiteral("Aucune colonne à exporter."));
        return;
    }

    QVector<int> drawWidths;
    drawWidths.reserve(columns.size());
    int usedW = 0;
    for (int i = 0; i < sourceWidths.size(); ++i) {
        int dw = qMax(42, static_cast<int>((static_cast<double>(sourceWidths[i]) / static_cast<double>(qMax(1, totalSourceW))) * contentW));
        drawWidths.push_back(dw);
        usedW += dw;
    }
    if (!drawWidths.isEmpty()) {
        drawWidths[drawWidths.size() - 1] += (contentW - usedW);
    }

    auto drawTableHeader = [&]() {
        const int headerH = 34;
        int x = margin;
        painter.fillRect(QRect(margin, y, contentW, headerH), QColor(QStringLiteral("#0b1b8f")));
        painter.setFont(headerFont);
        painter.setPen(Qt::white);
        for (int i = 0; i < columns.size(); ++i) {
            const int c = columns[i];
            const int w = drawWidths[i];
            const QString text = table->horizontalHeaderItem(c) ? table->horizontalHeaderItem(c)->text().trimmed() : QStringLiteral("Colonne %1").arg(c + 1);
            painter.drawText(QRect(x + 5, y, w - 10, headerH), Qt::AlignLeft | Qt::AlignVCenter, text);
            x += w;
        }
        y += headerH;
    };

    const int footerY = pageH - margin - 20;
    const int statsBlockTop = pageH - margin - 420;
    const int tableTop = y;
    const int tableHeaderH = 30;
    const int tableTotalH = 30;
    const int tableBottomLimit = qMax(tableTop + tableHeaderH + 40, statsBlockTop - 18);
    const int tableRowsAreaH = qMax(40, tableBottomLimit - tableTop - tableHeaderH - tableTotalH);
    const int rowH = 23;
    const int maxRowsInPdf = qMax(1, tableRowsAreaH / rowH);

    drawTableHeader();

    QVector<int> visibleRows;
    visibleRows.reserve(table->rowCount());
    for (int r = 0; r < table->rowCount(); ++r) {
        if (!table->isRowHidden(r)) visibleRows.push_back(r);
    }

    const int totalVisibleRows = visibleRows.size();
    const int exportedRows = qMin(maxRowsInPdf, totalVisibleRows);

    painter.setFont(cellFont);
    for (int index = 0; index < exportedRows; ++index) {
        const int r = visibleRows[index];
        const QColor rowColor = (index % 2 == 0) ? QColor(QStringLiteral("#ffffff")) : QColor(QStringLiteral("#e9edf5"));
        painter.fillRect(QRect(margin, y, contentW, rowH), rowColor);
        painter.setPen(QColor(QStringLiteral("#b9b9b9")));
        painter.drawRect(QRect(margin, y, contentW, rowH));

        int x = margin;
        for (int i = 0; i < columns.size(); ++i) {
            const int c = columns[i];
            const int w = drawWidths[i];
            painter.setPen(QColor(QStringLiteral("#b9b9b9")));
            painter.drawLine(x, y, x, y + rowH);

            painter.setPen(QColor(QStringLiteral("#2c2c2c")));
            const QTableWidgetItem* item = table->item(r, c);
            QString text = item ? item->text() : QString();
            text = text.simplified();

            QFontMetrics fm(cellFont);
            text = fm.elidedText(text, Qt::ElideRight, qMax(10, w - 8));
            painter.drawText(QRect(x + 4, y, w - 8, rowH), Qt::AlignLeft | Qt::AlignVCenter, text);
            x += w;
        }

        painter.setPen(QColor(QStringLiteral("#b9b9b9")));
        painter.drawLine(margin + contentW, y, margin + contentW, y + rowH);
        y += rowH;
    }

    y += 8;
    painter.setPen(QColor(QStringLiteral("#6e6e6e")));
    painter.setFont(totalFont);
    painter.drawText(QRect(margin, y, contentW, 24), Qt::AlignHCenter | Qt::AlignVCenter,
                     QStringLiteral("Total: %1 pecheurs").arg(totalVisibleRows));

    if (totalVisibleRows > exportedRows) {
        painter.setFont(QFont(QStringLiteral("Arial"), 9, QFont::Normal));
        painter.setPen(QColor(QStringLiteral("#8a8a8a")));
        painter.drawText(QRect(margin, y + 24, contentW, 18), Qt::AlignHCenter | Qt::AlignVCenter,
                         QStringLiteral("(%1 lignes affichées sur %2 dans cette page)").arg(exportedRows).arg(totalVisibleRows));
    }

    const QString recherche = ui->lineEdit_4p ? ui->lineEdit_4p->text().trimmed().simplified() : QString();
    const QString roleSelection = cleanFilterLabel(ui->comboBox_5p ? ui->comboBox_5p->currentText() : QString());
    const QString dispoSelection = cleanFilterLabel(ui->comboBox_6p ? ui->comboBox_6p->currentText() : QString());
    const Pecheurs::DisponibiliteStats stats = Pecheurs::calculerDisponibiliteStats(recherche, roleSelection, dispoSelection);
    const int totalStats = stats.disponible + stats.bientot + stats.indisponible + stats.enConge;

    y = statsBlockTop;

    painter.setPen(QColor(QStringLiteral("#0b1b8f")));
    painter.setFont(QFont(QStringLiteral("Arial"), 15, QFont::Bold));
    painter.drawText(QRect(margin, y, contentW, 36), Qt::AlignHCenter | Qt::AlignVCenter,
                     QStringLiteral("Statistiques selon disponibilité"));
    y += 36;

    painter.setPen(QColor(QStringLiteral("#6e6e6e")));
    painter.setFont(QFont(QStringLiteral("Arial"), 10, QFont::Bold));
    painter.drawText(QRect(margin, y, contentW, 22), Qt::AlignHCenter | Qt::AlignVCenter,
                     QStringLiteral("Total: %1 pecheurs").arg(totalStats));
    y += 28;

    const QRect pieRect(margin + 8, y + 4, 170, 170);
    const QVector<QPair<QString, QPair<int, QColor>>> slices = {
        { QStringLiteral("Disponible"), { stats.disponible, QColor(QStringLiteral("#00C853")) } },
        { QStringLiteral("Disponible bientot"), { stats.bientot, QColor(QStringLiteral("#007BFF")) } },
        { QStringLiteral("Indisponible"), { stats.indisponible, QColor(QStringLiteral("#FF1744")) } },
        { QStringLiteral("En conge"), { stats.enConge, QColor(QStringLiteral("#AA00FF")) } }
    };

    painter.setRenderHint(QPainter::Antialiasing, true);
    if (totalStats > 0) {
        int startAngle = 90 * 16;
        for (const auto& slice : slices) {
            const int value = slice.second.first;
            if (value <= 0) continue;
            const int span = -qRound((static_cast<double>(value) / static_cast<double>(totalStats)) * 360.0 * 16.0);
            painter.setBrush(slice.second.second);
            painter.setPen(Qt::white);
            painter.drawPie(pieRect, startAngle, span);
            startAngle += span;
        }
    } else {
        painter.setBrush(QColor(QStringLiteral("#dadada")));
        painter.setPen(Qt::white);
        painter.drawEllipse(pieRect);
    }

    painter.setRenderHint(QPainter::Antialiasing, false);

    const int legendX = pieRect.right() + 28;
    int legendY = y + 4;
    painter.setFont(QFont(QStringLiteral("Arial"), 10, QFont::Normal));
    for (const auto& slice : slices) {
        const int value = slice.second.first;
        const double pct = (totalStats > 0) ? (100.0 * static_cast<double>(value) / static_cast<double>(totalStats)) : 0.0;
        painter.fillRect(QRect(legendX, legendY + 6, 10, 10), slice.second.second);
        painter.setPen(QColor(QStringLiteral("#202020")));
        painter.drawText(QRect(legendX + 18, legendY - 2, contentW - (legendX - margin) - 20, 22),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         QStringLiteral("%1: %2 (%3%)")
                            .arg(slice.first)
                            .arg(value)
                            .arg(QString::number(pct, 'f', 0)));
        legendY += 26;
    }

    painter.setPen(QColor(QStringLiteral("#8a8a8a")));
    painter.setFont(footerFont);
    painter.drawText(QRect(margin, footerY, contentW, 20), Qt::AlignHCenter | Qt::AlignVCenter,
                     QStringLiteral("AQUATEC - Rapport généré le %1").arg(exportStamp));

    restoreActionColumn();

    painter.end();
    QMessageBox::information(parent, QStringLiteral("Export PDF"),
                             QStringLiteral("Fichier PDF exporté avec succès:\n%1").arg(filePath));
}

void MainWindow::on_pushButton_pdfb_5_clicked()
{
    // Bouton retour menu (comportement d'origine)
    if (!ui) return;

    if (ui->framefaceidp) ui->framefaceidp->hide();
    setPecheurMainWidgetsVisible(ui, true);

    if (ui->stackedWidget && ui->gestionsb) {
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
    }
}

void MainWindow::on_bep_clicked()
{
    exportPecheursPdfReport(this, ui);
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
    
    const QString nom = ui && ui->lineEdit_2p ? ui->lineEdit_2p->text().trimmed() : QString();
    const QString prenom = ui && ui->lineEdit_3p ? ui->lineEdit_3p->text().trimmed() : QString();
    const QString email = ui && ui->lineEditp_2 ? ui->lineEditp_2->text().trimmed() : QString();

    if (id <= 0) {
        QMessageBox::warning(this, "Champs", "Veuillez saisir ID");
        if (ui && ui->lineEditp) ui->lineEditp->setFocus();
        return;
    }

    if (nom.isEmpty()) {
        QMessageBox::warning(this, "Champs", "Veuillez saisir Nom");
        if (ui && ui->lineEdit_2p) ui->lineEdit_2p->setFocus();
        return;
    }

    if (prenom.isEmpty()) {
        QMessageBox::warning(this, "Champs", "Veuillez saisir Prenom");
        if (ui && ui->lineEdit_3p) ui->lineEdit_3p->setFocus();
        return;
    }

    if (email.isEmpty()) {
        QMessageBox::warning(this, "Champs", "Veuillez saisir Email");
        if (ui && ui->lineEditp_2) ui->lineEditp_2->setFocus();
        return;
    }
    
    // Vérifier si on est en mode édition (modification)
    if (m_editingPecheurId > 0) {
        // Mode MODIFICATION
        qDebug() << ">>> Mode MODIFICATION ACTIVÉ <<<";
        qDebug() << "Mode MODIFICATION: ID en édition:" << m_editingPecheurId << "ID du formulaire:" << id;
        // Utiliser l'ID original uniquement pour cibler la ligne en base,
        // et l'ID du formulaire comme nouvelle valeur à enregistrer.
        qDebug() << "Pêcheur à modifier: ancien ID=" << m_editingPecheurId << "nouvel ID=" << id;
        if (p.modifierAvecAncienId(m_editingPecheurId)) {
            qDebug() << "p.modifier() a réussi!";
            loadPecheurs();
            resetAjouterButton();
            QMessageBox::information(this, "OK", "Modification reussie");
        } else {
            const QString err = Pecheurs::lastError();
            qDebug() << "p.modifierAvecAncienId() a échoué!" << err;
            if (isSaisieConstraintError(err)) {
                QMessageBox::warning(this, "Champs", "Modification echouee: " + err);
            } else {
                QMessageBox::critical(this, "Erreur", "Modification echouee: " + err);
            }
        }
    } else {
        // Mode AJOUT
        qDebug() << ">>> Mode AJOUT ACTIVÉ <<<";
        qDebug() << "Mode AJOUT: ID en édition:" << m_editingPecheurId;
        if (Pecheurs::idExiste(id)) {
            QMessageBox::warning(this, "Doublon", "Cet ID existe déjà. Veuillez saisir un ID différent.");
            return;
        }

        if (p.ajouter()) {
            qDebug() << "p.ajouter() a réussi!";
            loadPecheurs();
            resetAjouterButton();
            QMessageBox::information(this, "OK", "Ajout reussi");
        } else {
            const QString err = Pecheurs::lastError();
            qDebug() << "p.ajouter() a échoué!" << err;
            if (isSaisieConstraintError(err)) {
                QMessageBox::warning(this, "Champs", "Ajout echoue: " + err);
            } else {
                QMessageBox::critical(this, "Erreur", "Ajout echoue: " + err);
            }
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
    
    // Garder le champ ID éditable pour permettre la modification de l'ID
    if (ui && ui->lineEditp) {
        ui->lineEditp->setReadOnly(false);
        qDebug() << "Champ ID éditable en mode modification";
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
        if (p.modifierAvecAncienId(m_editingPecheurId)) {
            loadPecheurs();
            resetAjouterButton();
            QMessageBox::information(this, "OK", "Modification reussie");
        } else {
            const QString err = Pecheurs::lastError();
            if (isSaisieConstraintError(err)) {
                QMessageBox::warning(this, "Champs", "Modification echouee: " + err);
            } else {
                QMessageBox::critical(this, "Erreur", "Modification echouee: " + err);
            }
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

