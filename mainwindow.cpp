#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QAbstractButton>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDate>
#include <QDateTime>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QPixmap>
#include <QPdfWriter>
#include <QFileDialog>
#include <QPageSize>
#include <QPageLayout>
#include <QDir>
#include <QStandardPaths>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QProgressBar>
#include <QHeaderView>
#include "bateaauuu.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStackedWidget>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <algorithm>
#include <functional>
#include <initializer_list>

// Helpers génériques pour la base de données et le texte UI

static QString weatherDescriptionFr(int weatherCode);
static QString weatherIconEmoji(int weatherCode);

static bool handleCrudDisabled(QWidget* parent)
{
    Q_UNUSED(parent);
    // Point central pour désactiver temporairement les opérations CRUD si besoin.
    return true;
}

static QString normalizeKey(const QString& s)
{
    QString out = s.normalized(QString::NormalizationForm_D);
    static QRegularExpression diacRx(QStringLiteral("\\p{Mn}+"));
    out.remove(diacRx);
    out.replace(QRegularExpression(QStringLiteral("[\\s_]+")), QString());
    return out.toLower();
}

static QStringList dbTableNames(QSqlDatabase db)
{
    if (!db.isValid()) return {};
    QStringList names = db.tables(QSql::Tables);
    names += db.tables(QSql::Views);
    for (QString& n : names) {
        n = n.trimmed();
    }
    return names;
}

static QStringList getColumnNames(QSqlDatabase db, const QString& tableName)
{
    QStringList cols;
    if (!db.isValid() || tableName.isEmpty()) return cols;

    const QSqlRecord rec = db.record(tableName);
    for (int i = 0; i < rec.count(); ++i) {
        cols << rec.fieldName(i);
    }
    return cols;
}

static QString matchColumnBySynonyms(const QStringList& dbCols,
                                     const QStringList& syns)
{
    if (dbCols.isEmpty() || syns.isEmpty()) return QString();

    // D'abord, tentative de correspondance exacte sur la clé normalisée
    for (const QString& syn : syns) {
        const QString key = normalizeKey(syn);
        for (const QString& col : dbCols) {
            if (normalizeKey(col) == key) {
                return col;
            }
        }
    }

    // Ensuite, tentative de correspondance par préfixe
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

static QString resolveTableName(QSqlDatabase db, const QStringList& candidates)
{
    if (!db.isValid()) return QString();

    const QStringList tables = dbTableNames(db);
    if (tables.isEmpty()) return QString();

    // Correspondance exacte sur le nom normalisé
    for (const QString& cand : candidates) {
        const QString key = normalizeKey(cand);
        for (const QString& tbl : tables) {
            if (normalizeKey(tbl) == key) {
                return tbl;
            }
        }
    }

    // Sinon, correspondance par préfixe
    for (const QString& cand : candidates) {
        const QString key = normalizeKey(cand);
        for (const QString& tbl : tables) {
            if (normalizeKey(tbl).startsWith(key)) {
                return tbl;
            }
        }
    }

    return QString();
}

struct RequiredField {
    QString label;
    QWidget* widget;
    std::function<bool()> isEmpty;
};

static bool validateRequiredFields(QWidget* parent,
                                   std::initializer_list<RequiredField> fields)
{
    for (const RequiredField& f : fields) {
        if (f.isEmpty && f.isEmpty()) {
            QMessageBox::warning(parent,
                                 QStringLiteral("Champs requis"),
                                 QStringLiteral("Le champ '%1' est obligatoire.").arg(f.label));
            if (f.widget) {
                f.widget->setFocus();
            }
            return false;
        }
    }
    return true;
}

static bool insertRowByMapping(QWidget* parent,
                               QSqlDatabase db,
                               const QString& tableName,
                               const QHash<QString, QVariant>& values,
                               const QHash<QString, QStringList>& synonyms,
                               QString* errorOut)
{
    if (!db.isValid() || tableName.isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("Base de données ou table invalide");
        return false;
    }

    const QStringList dbCols = getColumnNames(db, tableName);
    if (dbCols.isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("Impossible de récupérer les colonnes de la table %1").arg(tableName);
        return false;
    }

    QStringList columnNames;
    QStringList placeholders;
    QList<QVariant> bindValues;

    for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
        const QString logical = it.key();
        QStringList syns = synonyms.value(logical);
        if (syns.isEmpty()) syns << logical;

        const QString col = matchColumnBySynonyms(dbCols, syns);
        if (col.isEmpty()) {
            // Colonne non trouvée: on ignore ce champ
            continue;
        }

        columnNames << col;
        placeholders << QStringLiteral("?");
        bindValues << it.value();
    }

    if (columnNames.isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("Aucune colonne valide trouvée pour l'insertion dans %1").arg(tableName);
        return false;
    }

    const QString sql = QStringLiteral("INSERT INTO %1 (%2) VALUES (%3)")
                             .arg(tableName,
                                  columnNames.join(QLatin1Char(',')),
                                  placeholders.join(QLatin1Char(',')));

    QSqlQuery query(db);
    query.prepare(sql);
    for (const QVariant& v : std::as_const(bindValues)) {
        query.addBindValue(v);
    }

    if (!query.exec()) {
        if (errorOut) {
            *errorOut = query.lastError().text();
        } else {
            QMessageBox::critical(parent,
                                  QStringLiteral("Insertion"),
                                  QStringLiteral("Erreur SQL: %1").arg(query.lastError().text()));
        }
        return false;
    }

    return true;
}

static void reloadTableWidgetFromDb(QTableWidget* table,
                                     QSqlDatabase db,
                                     const QString& tableName,
                                     const QStringList& logicalColumns,
                                     const QHash<QString, QStringList>& synonyms)
{
    if (!table) return;
    if (!db.isValid()) return;

    table->clearContents();
    table->setRowCount(0);

    const QStringList dbCols = getColumnNames(db, tableName);
    if (dbCols.isEmpty()) return;

    QStringList physicalColumns;
    for (const QString& logical : logicalColumns) {
        QStringList syns = synonyms.value(logical);
        if (syns.isEmpty()) syns << logical;
        QString col = matchColumnBySynonyms(dbCols, syns);
        if (col.isEmpty()) {
            col = logical; // repli
        }
        physicalColumns << col;
    }

    if (physicalColumns.isEmpty()) return;

    const QString sql = QStringLiteral("SELECT %1 FROM %2")
                            .arg(physicalColumns.join(QLatin1Char(',')), tableName);

    QSqlQuery query(db);
    if (!query.exec(sql)) {
        return;
    }

    const int dataColumnCount = physicalColumns.size();
    table->setColumnCount(dataColumnCount + 1); // +1 pour la colonne Actions

    // En-têtes: on affiche les noms logiques plus lisibles
    for (int i = 0; i < logicalColumns.size(); ++i) {
        auto *item = new QTableWidgetItem(logicalColumns.at(i));
        table->setHorizontalHeaderItem(i, item);
    }
    table->setHorizontalHeaderItem(dataColumnCount, new QTableWidgetItem(QStringLiteral("Actions")));

    int row = 0;
    while (query.next()) {
        table->insertRow(row);
        for (int col = 0; col < dataColumnCount; ++col) {
            const QVariant value = query.value(col);
            auto *cellItem = new QTableWidgetItem(value.toString());
            table->setItem(row, col, cellItem);
        }
        ++row;
    }
}



static bool authenticateLoginFromDb(QSqlDatabase db,
                                    const QString& username,
                                    const QString& password,
                                    QString* errorOut)
{
    const QString usersTable = resolveTableName(db, {
        QStringLiteral("UTILISATEUR"), QStringLiteral("UTILISATEURS"),
        QStringLiteral("USER"), QStringLiteral("USERS"),
        QStringLiteral("COMPTE"), QStringLiteral("COMPTES"),
        QStringLiteral("LOGIN")
    });

    if (usersTable.isEmpty()) {
        if (errorOut) {
            *errorOut = QStringLiteral("Table utilisateurs introuvable.");
        }
        return false;
    }

    const QStringList cols = getColumnNames(db, usersTable);
    if (cols.isEmpty()) {
        if (errorOut) {
            *errorOut = QStringLiteral("Colonnes utilisateurs introuvables.");
        }
        return false;
    }

    const QString userCol = matchColumnBySynonyms(cols, {
        QStringLiteral("username"), QStringLiteral("user"), QStringLiteral("login"),
        QStringLiteral("email"), QStringLiteral("nomutilisateur"), QStringLiteral("nom")
    });
    const QString passCol = matchColumnBySynonyms(cols, {
        QStringLiteral("password"), QStringLiteral("motdepasse"), QStringLiteral("mdp"),
        QStringLiteral("pass")
    });

    if (userCol.isEmpty() || passCol.isEmpty()) {
        if (errorOut) {
            *errorOut = QStringLiteral("Colonnes login/mot de passe non trouvées.");
        }
        return false;
    }

    QSqlQuery query(db);
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM %1 WHERE %2 = ? AND %3 = ?")
                      .arg(usersTable, userCol, passCol));
    query.addBindValue(username);
    query.addBindValue(password);

    if (!query.exec() || !query.next()) {
        if (errorOut) {
            *errorOut = query.lastError().text();
        }
        return false;
    }

    return query.value(0).toInt() > 0;
}

// Ajoute une colonne d'actions avec des boutons stylés (sans logique métier spécifique)

static void ensureActionsColumnPopulated(QTableWidget* table, const QString& buttonStyle)
{
    if (!table) return;

    const int actionsCol = table->columnCount() - 1;
    if (actionsCol < 0) return;

    for (int row = 0; row < table->rowCount(); ++row) {
        if (table->cellWidget(row, actionsCol)) {
            continue; // déjà peuplé
        }

        QWidget* container = new QWidget(table);
        container->setAttribute(Qt::WA_TranslucentBackground, true);
        container->setStyleSheet(QStringLiteral("background-color: transparent;"));

        auto *layout = new QHBoxLayout(container);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(4);

        auto *editBtn = new QPushButton(QStringLiteral("📝"), container);
        auto *deleteBtn = new QPushButton(QStringLiteral("❌"), container);

        // Hauteur minimale des boutons (un peu plus petite que précédemment)
        const int buttonMinHeight = 34;
        editBtn->setMinimumHeight(buttonMinHeight);
        deleteBtn->setMinimumHeight(buttonMinHeight);

        if (!buttonStyle.isEmpty()) {
            // On garde la bordure et le rayon définis dans buttonStyle,
            // mais on force juste le fond à être transparent pour
            // tous les états.
            const QString finalStyle = buttonStyle +
                    QStringLiteral(
                        " QPushButton {"
                        "   background-color: transparent;"
                        "   padding: 0 4px;"
                        " }"
                        " QPushButton:hover {"
                        "   background-color: transparent;"
                        " }"
                        " QPushButton:pressed {"
                        "   background-color: transparent;"
                        " }");
            editBtn->setStyleSheet(finalStyle);
            deleteBtn->setStyleSheet(finalStyle);
        }

        layout->addWidget(editBtn);
        layout->addWidget(deleteBtn);
        container->setLayout(layout);

        table->setCellWidget(row, actionsCol, container);
        // Assurer une hauteur de ligne suffisante pour les gros boutons
        table->setRowHeight(row, qMax(table->rowHeight(row), buttonMinHeight + 8));
        // Les connexions spécifiques (édition/suppression) sont gérées ailleurs si nécessaire.
    }
}

// Normalisation du texte de l'UI (stub simple pour l'instant)

void MainWindow::normalizeUiTexts()
{
    // Implémentation simplifiée: la logique détaillée d'emoji/accents
    // pourra être rétablie plus tard si nécessaire.
}

// Public helper implementations added to avoid accessing private members from lambdas.

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

    // Remplir automatiquement les tables depuis la base au démarrage
    QSqlDatabase db = Connection::getInstance()->getDatabase();
    if (ui->tableWidgetc) {
        reloadTableWidgetFromDb(ui->tableWidgetc, db, QStringLiteral("clients"),
            {QStringLiteral("id"), QStringLiteral("nom"), QStringLiteral("prenom"), QStringLiteral("statut"), QStringLiteral("profil"), QStringLiteral("date"), QStringLiteral("telephone")},
            {});
        ensureActionsColumnPopulated(ui->tableWidgetc, QStringLiteral("QPushButton { border:2px solid rgb(0, 0, 112); border-radius:8px; background-color: rgba(0, 0, 127,0.7); color: white; padding: 4px 8px; font-family: 'Segoe UI Emoji', 'Segoe UI', 'Arial', sans-serif; font-size: 8px; font-weight: bold;} QPushButton:hover { background-color: rgba(0, 0, 127,0.7); border-color: #59abc8;} QPushButton:pressed { background-color:rgba(0, 0, 127,0.9); }"));
        int actionsCol = ui->tableWidgetc->columnCount() - 1;
        if (actionsCol >= 0) ui->tableWidgetc->setColumnWidth(actionsCol, 140);
    }
    if (ui->tableWidgetee) {
        reloadTableWidgetFromDb(ui->tableWidgetee, db, QStringLiteral("employes"),
            {QStringLiteral("id"), QStringLiteral("nom"), QStringLiteral("prenom"), QStringLiteral("telephone"), QStringLiteral("salaire"), QStringLiteral("equipe"), QStringLiteral("etat"), QStringLiteral("role"), QStringLiteral("zone")},
            {});
        ensureActionsColumnPopulated(ui->tableWidgetee, QStringLiteral("QPushButton { border:2px solid rgb(0, 0, 112); border-radius:6px; background-color: rgba(0, 0, 127,0.7); color: white; padding: 8px 16px; font-family: 'Segoe UI Emoji', 'Segoe UI', 'Arial', sans-serif; font-size: 26px; font-weight: bold;} QPushButton:hover { background-color: rgba(0, 0, 127,0.7); border-color: #59abc8;} QPushButton:pressed { background-color:rgba(0, 0, 127,0.9); }"));
        int actionsCol2 = ui->tableWidgetee->columnCount() - 1;
        if (actionsCol2 >= 0) ui->tableWidgetee->setColumnWidth(actionsCol2, 140);
    }
    loadBateaux();
    // Chargement initial de la table des quais avec actions connectées
    refreshQuaiTable();

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

    // Rafraîchissement automatique des statistiques d'occupation
    // pendant que la page_4 (statistiques des quais) est affichée.
    m_statsTimer = new QTimer(this);
    m_statsTimer->setInterval(5000); // toutes les 5 secondes
    connect(m_statsTimer, &QTimer::timeout, this, [this]() {
        if (!ui || !ui->stackedWidget || !ui->page_4) {
            return;
        }
        if (ui->stackedWidget->currentWidget() == ui->page_4) {
            on_btnRefreshStats_2_clicked();
        }
    });
    m_statsTimer->start();

    // Météo (page_3) : API Open-Meteo sans clé + rafraîchissement périodique
    m_weatherNetwork = new QNetworkAccessManager(this);

    m_weatherTimer = new QTimer(this);
    m_weatherTimer->setInterval(10 * 60 * 1000); // toutes les 10 minutes
    connect(m_weatherTimer, &QTimer::timeout, this, [this]() {
        if (!ui || !ui->stackedWidget || !ui->page_3) {
            return;
        }
        if (ui->stackedWidget->currentWidget() == ui->page_3) {
            refreshWeatherForPage3();
        }
    });
    m_weatherTimer->start();

    if (ui->stackedWidget && ui->page_3) {
        connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, [this](int index) {
            if (!ui || !ui->stackedWidget || !ui->page_3) {
                return;
            }
            if (ui->stackedWidget->widget(index) == ui->page_3) {
                refreshWeatherForPage3();
            }
        });
    }

    // Chargement météo initial
    refreshWeatherForPage3();
}

// Navigation / actions : implémentations uniques plus bas dans le fichier.

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
        if (ui->tableWidgetb) ui->tableWidgetb->setVisible(true);
    }
    // Rafraîchir la table à chaque ouverture
    loadBateaux();
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
    on_pushButton_b_4b_clicked();
}
void MainWindow::on_pushButton_b_4b_clicked()
{
    if (!ui) {
        return;
    }

    const QString username = ui->lineEdit_b ? ui->lineEdit_b->text().trimmed() : QString();
    const QString password = ui->lineEdit_2b ? ui->lineEdit_2b->text() : QString();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this,
                             QStringLiteral("Connexion"),
                             QStringLiteral("Veuillez saisir le nom d'utilisateur et le mot de passe."));
        return;
    }

    Connection *conn = Connection::getInstance();
    if (!conn->ensureOpen()) {
        QMessageBox::critical(this,
                              QStringLiteral("Connexion"),
                              QStringLiteral("Connexion DB échouée: %1").arg(conn->lastErrorText()));
        return;
    }

    bool authenticated = false;
    QString authError;
    authenticated = authenticateLoginFromDb(conn->getDatabase(), username, password, &authError);

    // Fallback minimal si la table utilisateurs n'existe pas encore.
    if (!authenticated && (authError.contains(QStringLiteral("introuvable"), Qt::CaseInsensitive)
                           || authError.contains(QStringLiteral("non trouv"), Qt::CaseInsensitive))) {
        authenticated = (username == QStringLiteral("admin") && password == QStringLiteral("admin"));
    }

    if (!authenticated) {
        QMessageBox::warning(this,
                             QStringLiteral("Connexion"),
                             QStringLiteral("Identifiants invalides."));
        return;
    }

    if (ui->lineEdit_2b) {
        ui->lineEdit_2b->clear();
    }

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

// ----------------------
// CRUD: AJOUTER (toutes pages)
// ----------------------

void MainWindow::on_pushButton_2c_clicked()
{
    if (!handleCrudDisabled(this) || !ui) return;

    if (!validateRequiredFields(this, {
            {QStringLiteral("ID client"), ui->lineEdit_3c, [this]{ return !ui->lineEdit_3c || ui->lineEdit_3c->text().trimmed().isEmpty(); }},
            {QStringLiteral("Nom client"), ui->lineEdit_4c, [this]{ return !ui->lineEdit_4c || ui->lineEdit_4c->text().trimmed().isEmpty(); }},
            {QStringLiteral("Prénom client"), ui->lineEdit_12c, [this]{ return !ui->lineEdit_12c || ui->lineEdit_12c->text().trimmed().isEmpty(); }},
            {QStringLiteral("Téléphone"), ui->lineEdit_14c, [this]{ return !ui->lineEdit_14c || ui->lineEdit_14c->text().trimmed().isEmpty(); }},
        })) {
        return;
    }

    Connection *conn = Connection::getInstance();
    if (!conn->ensureOpen()) {
        QMessageBox::critical(this, QStringLiteral("DB"), QStringLiteral("Connexion DB échouée: %1").arg(conn->lastErrorText()));
        return;
    }
    QSqlDatabase db = conn->getDatabase();

    const QString tableName = resolveTableName(db, {QStringLiteral("CLIENT"), QStringLiteral("CLIENTS"), QStringLiteral("TCLIENT"), QStringLiteral("T_CLIENT")});
    if (tableName.isEmpty()) {
        QMessageBox::critical(this, QStringLiteral("DB"), QStringLiteral("Table clients introuvable (CLIENT/CLIENTS...)."));
        return;
    }

    QHash<QString, QVariant> values;
    values.insert(QStringLiteral("id"), ui->lineEdit_3c ? ui->lineEdit_3c->text().trimmed() : QString());
    values.insert(QStringLiteral("nom"), ui->lineEdit_4c ? ui->lineEdit_4c->text().trimmed() : QString());
    values.insert(QStringLiteral("prenom"), ui->lineEdit_12c ? ui->lineEdit_12c->text().trimmed() : QString());
    values.insert(QStringLiteral("statut"), ui->comboBoxc ? ui->comboBoxc->currentText().trimmed() : QString());
    values.insert(QStringLiteral("profil"), ui->comboBox_2c ? ui->comboBox_2c->currentText().trimmed() : QString());
    values.insert(QStringLiteral("telephone"), ui->lineEdit_14c ? ui->lineEdit_14c->text().trimmed() : QString());
    values.insert(QStringLiteral("date"), ui->dateEdit_c ? ui->dateEdit_c->date() : QDate());

    const QHash<QString, QStringList> syn = {
        {QStringLiteral("id"), {QStringLiteral("id"), QStringLiteral("idclient"), QStringLiteral("clientid"), QStringLiteral("id_client")}},
        {QStringLiteral("nom"), {QStringLiteral("nom"), QStringLiteral("nomclient"), QStringLiteral("name"), QStringLiteral("lastname")}},
        {QStringLiteral("prenom"), {QStringLiteral("prenom"), QStringLiteral("prenomclient"), QStringLiteral("firstname")}},
        {QStringLiteral("statut"), {QStringLiteral("statut"), QStringLiteral("statutclient"), QStringLiteral("status")}},
        {QStringLiteral("profil"), {QStringLiteral("profil"), QStringLiteral("profilclient"), QStringLiteral("profile")}},
        {QStringLiteral("telephone"), {QStringLiteral("telephone"), QStringLiteral("tel"), QStringLiteral("phone")}},
        {QStringLiteral("date"), {QStringLiteral("date"), QStringLiteral("dateinscription"), QStringLiteral("date_inscription"), QStringLiteral("createdat")}},
    };

    QString err;
    if (!insertRowByMapping(this, db, tableName, values, syn, &err)) {
        QMessageBox::critical(this, QStringLiteral("Ajout client"), QStringLiteral("Insertion échouée: %1").arg(err));
        return;
    }

    // Refresh widget
    if (ui->tableWidgetc) {
        reloadTableWidgetFromDb(ui->tableWidgetc, db, tableName,
                                {QStringLiteral("id"), QStringLiteral("nom"), QStringLiteral("prenom"), QStringLiteral("statut"), QStringLiteral("profil"), QStringLiteral("date"), QStringLiteral("telephone")},
                                syn);
        ensureActionsColumnPopulated(ui->tableWidgetc, QStringLiteral("QPushButton { border:2px solid rgb(0, 0, 112); border-radius:8px; background-color: rgba(0, 0, 127,0.7); color: white; padding: 4px 8px; font-family: 'Segoe UI Emoji', 'Segoe UI', 'Arial', sans-serif; font-size: 8px; font-weight: bold;} QPushButton:hover { background-color: rgba(0, 0, 127,0.7); border-color: #59abc8;} QPushButton:pressed { background-color:rgba(0, 0, 127,0.9); }") );
        int actionsCol = ui->tableWidgetc->columnCount() - 1;
        if (actionsCol >= 0) ui->tableWidgetc->setColumnWidth(actionsCol, 140);
    }
}

void MainWindow::on_pushButton_5e_clicked()
{
    if (!handleCrudDisabled(this) || !ui) return;

    if (!validateRequiredFields(this, {
            {QStringLiteral("ID Employé"), ui->lineEdit_12e, [this]{ return !ui->lineEdit_12e || ui->lineEdit_12e->text().trimmed().isEmpty(); }},
            {QStringLiteral("Nom"), ui->lineEdit_13e, [this]{ return !ui->lineEdit_13e || ui->lineEdit_13e->text().trimmed().isEmpty(); }},
            {QStringLiteral("Prénom"), ui->lineEdit_16e, [this]{ return !ui->lineEdit_16e || ui->lineEdit_16e->text().trimmed().isEmpty(); }},
            {QStringLiteral("Téléphone"), ui->lineEdit_14e, [this]{ return !ui->lineEdit_14e || ui->lineEdit_14e->text().trimmed().isEmpty(); }},
            {QStringLiteral("Salaire"), ui->lineEdit_14e_2, [this]{ return !ui->lineEdit_14e_2 || ui->lineEdit_14e_2->text().trimmed().isEmpty(); }},
        })) {
        return;
    }

    Connection *conn = Connection::getInstance();
    if (!conn->ensureOpen()) {
        QMessageBox::critical(this, QStringLiteral("DB"), QStringLiteral("Connexion DB échouée: %1").arg(conn->lastErrorText()));
        return;
    }
    QSqlDatabase db = conn->getDatabase();

    const QString tableName = resolveTableName(db, {QStringLiteral("EMPLOYE"), QStringLiteral("EMPLOYES"), QStringLiteral("EMPLOYEE"), QStringLiteral("EMPLOYEES"), QStringLiteral("T_EMPLOYE")});
    if (tableName.isEmpty()) {
        QMessageBox::critical(this, QStringLiteral("DB"), QStringLiteral("Table employés introuvable (EMPLOYE/EMPLOYES...)."));
        return;
    }

    QHash<QString, QVariant> values;
    values.insert(QStringLiteral("id"), ui->lineEdit_12e ? ui->lineEdit_12e->text().trimmed() : QString());
    values.insert(QStringLiteral("nom"), ui->lineEdit_13e ? ui->lineEdit_13e->text().trimmed() : QString());
    values.insert(QStringLiteral("prenom"), ui->lineEdit_16e ? ui->lineEdit_16e->text().trimmed() : QString());
    values.insert(QStringLiteral("telephone"), ui->lineEdit_14e ? ui->lineEdit_14e->text().trimmed() : QString());
    values.insert(QStringLiteral("salaire"), ui->lineEdit_14e_2 ? ui->lineEdit_14e_2->text().trimmed() : QString());
    values.insert(QStringLiteral("equipe"), ui->comboBox_15e ? ui->comboBox_15e->currentText().trimmed() : QString());
    values.insert(QStringLiteral("etat"), ui->comboBox_16e ? ui->comboBox_16e->currentText().trimmed() : QString());
    values.insert(QStringLiteral("role"), ui->comboBox_11e ? ui->comboBox_11e->currentText().trimmed() : QString());
    values.insert(QStringLiteral("zone"), ui->comboBox_14e ? ui->comboBox_14e->currentText().trimmed() : QString());

    const QHash<QString, QStringList> syn = {
        {QStringLiteral("id"), {QStringLiteral("id"), QStringLiteral("idemploye"), QStringLiteral("employeeid"), QStringLiteral("id_employe")}},
        {QStringLiteral("nom"), {QStringLiteral("nom"), QStringLiteral("name"), QStringLiteral("lastname")}},
        {QStringLiteral("prenom"), {QStringLiteral("prenom"), QStringLiteral("firstname")}},
        {QStringLiteral("telephone"), {QStringLiteral("telephone"), QStringLiteral("tel"), QStringLiteral("phone")}},
        {QStringLiteral("salaire"), {QStringLiteral("salaire"), QStringLiteral("salary")}},
        {QStringLiteral("equipe"), {QStringLiteral("equipe"), QStringLiteral("team")}},
        {QStringLiteral("etat"), {QStringLiteral("etat"), QStringLiteral("etat_employe"), QStringLiteral("state")}},
        {QStringLiteral("role"), {QStringLiteral("role"), QStringLiteral("poste"), QStringLiteral("fonction")}},
        {QStringLiteral("zone"), {QStringLiteral("zone"), QStringLiteral("statut"), QStringLiteral("status")}},
    };

    QString err;
    if (!insertRowByMapping(this, db, tableName, values, syn, &err)) {
        QMessageBox::critical(this, QStringLiteral("Ajout employé"), QStringLiteral("Insertion échouée: %1").arg(err));
        return;
    }

    if (ui->tableWidgetee) {
        reloadTableWidgetFromDb(ui->tableWidgetee, db, tableName,
                                {QStringLiteral("id"), QStringLiteral("nom"), QStringLiteral("prenom"), QStringLiteral("telephone"), QStringLiteral("salaire"), QStringLiteral("equipe"), QStringLiteral("etat"), QStringLiteral("role"), QStringLiteral("zone")},
                                syn);
        ensureActionsColumnPopulated(ui->tableWidgetee, QStringLiteral("QPushButton { border:2px solid rgb(0, 0, 112); border-radius:6px; background-color: rgba(0, 0, 127,0.7); color: white; padding: 8px 16px; font-family: 'Segoe UI Emoji', 'Segoe UI', 'Arial', sans-serif; font-size: 26px; font-weight: bold;} QPushButton:hover { background-color: rgba(0, 0, 127,0.7); border-color: #59abc8;} QPushButton:pressed { background-color:rgba(0, 0, 127,0.9); }") );
        int actionsCol2 = ui->tableWidgetee->columnCount() - 1;
        if (actionsCol2 >= 0) ui->tableWidgetee->setColumnWidth(actionsCol2, 140);
        ensureActionsColumnPopulated(ui->tableWidgetee, QStringLiteral("QPushButton { border:2px solid rgb(0, 0, 112); border-radius:12px; background-color: rgba(0, 0, 127,0.7); color: white; padding: 16px 24px; font-family: 'Segoe UI Emoji', 'Segoe UI', 'Arial', sans-serif; font-size: 38px; font-weight: bold;} QPushButton:hover { background-color: rgba(0, 0, 127,0.7); border-color: #59abc8;} QPushButton:pressed { background-color:rgba(0, 0, 127,0.9); }") );
        ensureActionsColumnPopulated(ui->tableWidgetee, QStringLiteral("QPushButton { border:2px solid rgb(0, 0, 112); border-radius:12px; background-color: rgba(0, 0, 127,0.7); color: white; padding: 10px 18px; font-family: 'Segoe UI Emoji', 'Segoe UI', 'Arial', sans-serif; font-size: 26px; font-weight: bold;} QPushButton:hover { background-color: rgba(0, 0, 127,0.7); border-color: #59abc8;} QPushButton:pressed { background-color:rgba(0, 0, 127,0.9); }") );
        ensureActionsColumnPopulated(ui->tableWidgetee, QStringLiteral("QPushButton { border:2px solid rgb(0, 0, 112); border-radius:8px; background-color: rgba(0, 0, 127,0.7); color: white; padding: 4px 8px; font-family: 'Segoe UI Emoji', 'Segoe UI', 'Arial', sans-serif; font-size: 18px; font-weight: bold;} QPushButton:hover { background-color: rgba(0, 0, 127,0.7); border-color: #59abc8;} QPushButton:pressed { background-color:rgba(0, 0, 127,0.9); }") );
    }
}

void MainWindow::on_bap_clicked()
{
    if (!handleCrudDisabled(this) || !ui) return;

    if (!validateRequiredFields(this, {
            {QStringLiteral("ID Pêcheur"), ui->lineEditp, [this]{ return !ui->lineEditp || ui->lineEditp->text().trimmed().isEmpty(); }},
            {QStringLiteral("Nom Pêcheur"), ui->lineEdit_2p, [this]{ return !ui->lineEdit_2p || ui->lineEdit_2p->text().trimmed().isEmpty(); }},
            {QStringLiteral("Prénom Pêcheur"), ui->lineEdit_3p, [this]{ return !ui->lineEdit_3p || ui->lineEdit_3p->text().trimmed().isEmpty(); }},
            {QStringLiteral("Email"), ui->lineEditp_2, [this]{ return !ui->lineEditp_2 || ui->lineEditp_2->text().trimmed().isEmpty(); }},
        })) {
        return;
    }

    Connection *conn = Connection::getInstance();
    if (!conn->ensureOpen()) {
        QMessageBox::critical(this, QStringLiteral("DB"), QStringLiteral("Connexion DB échouée: %1").arg(conn->lastErrorText()));
        return;
    }
    QSqlDatabase db = conn->getDatabase();

    const QString tableName = resolveTableName(db, {QStringLiteral("PECHEUR"), QStringLiteral("PECHEURS"), QStringLiteral("PECHEURs"), QStringLiteral("T_PECHEUR")});
    if (tableName.isEmpty()) {
        QMessageBox::critical(this, QStringLiteral("DB"), QStringLiteral("Table pêcheurs introuvable (PECHEUR/PECHEURS...)."));
        return;
    }

    const QDateTime inscription = ui->dateTimeEdit ? ui->dateTimeEdit->dateTime() : QDateTime();
    QHash<QString, QVariant> values;
    values.insert(QStringLiteral("id"), ui->lineEditp ? ui->lineEditp->text().trimmed() : QString());
    values.insert(QStringLiteral("nom"), ui->lineEdit_2p ? ui->lineEdit_2p->text().trimmed() : QString());
    values.insert(QStringLiteral("prenom"), ui->lineEdit_3p ? ui->lineEdit_3p->text().trimmed() : QString());
    values.insert(QStringLiteral("role"), ui->comboBoxp ? ui->comboBoxp->currentText().trimmed() : QString());
    values.insert(QStringLiteral("bateau"), ui->lineEdit_2p_2 ? ui->lineEdit_2p_2->text().trimmed() : QString());
    values.insert(QStringLiteral("disponibilite"), ui->comboBox_2 ? ui->comboBox_2->currentText().trimmed() : QString());
    values.insert(QStringLiteral("email"), ui->lineEditp_2 ? ui->lineEditp_2->text().trimmed() : QString());
    values.insert(QStringLiteral("date"), inscription.isValid() ? inscription.date() : QDate());
    values.insert(QStringLiteral("heure"), inscription.isValid() ? inscription.time().toString(QStringLiteral("HH:mm:ss")) : QString());

    const QHash<QString, QStringList> syn = {
        {QStringLiteral("id"), {QStringLiteral("id"), QStringLiteral("idpecheur"), QStringLiteral("fisherid"), QStringLiteral("id_pecheur")}},
        {QStringLiteral("nom"), {QStringLiteral("nom"), QStringLiteral("name"), QStringLiteral("lastname")}},
        {QStringLiteral("prenom"), {QStringLiteral("prenom"), QStringLiteral("firstname")}},
        {QStringLiteral("role"), {QStringLiteral("role"), QStringLiteral("fonction")}},
        {QStringLiteral("bateau"), {QStringLiteral("bateau"), QStringLiteral("bateaux"), QStringLiteral("idbateau"), QStringLiteral("boat")}},
        {QStringLiteral("disponibilite"), {QStringLiteral("disponibilite"), QStringLiteral("availability"), QStringLiteral("etat")}},
        {QStringLiteral("email"), {QStringLiteral("email"), QStringLiteral("mail")}},
        {QStringLiteral("date"), {QStringLiteral("date"), QStringLiteral("dateinscription"), QStringLiteral("date_inscription")}},
        {QStringLiteral("heure"), {QStringLiteral("heure"), QStringLiteral("time")}},
    };

    QString err;
    if (!insertRowByMapping(this, db, tableName, values, syn, &err)) {
        QMessageBox::critical(this, QStringLiteral("Ajout pêcheur"), QStringLiteral("Insertion échouée: %1").arg(err));
        return;
    }

    if (ui->tableWidgetp) {
        reloadTableWidgetFromDb(ui->tableWidgetp, db, tableName,
                                {QStringLiteral("id"), QStringLiteral("nom"), QStringLiteral("prenom"), QStringLiteral("role"), QStringLiteral("bateau"), QStringLiteral("disponibilite"), QStringLiteral("email"), QStringLiteral("date"), QStringLiteral("heure")},
                                syn);
    }
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

    // Largeur > 0
    if (largeur <= 0.0) {
        erreurs << "La largeur doit être supérieure à 0.";
    }

    // Capacité > 0
    if (capacite <= 0) {
        erreurs << "La capacité doit être supérieure à 0.";
    }

    // Fréquence de maintenance > 0
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

    // Si erreurs, afficher la première erreur et arrêter
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

// ── Database helpers ─────────────────────────────────────────────────────────
Connection* MainWindow::getOracleConnection()
{
    Connection* conn = Connection::getInstance();
    conn->ensureOpen();
    return conn;
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
    // Synchronise le nombre de colonnes avec les données SQL + Actions
    ui->tableWidgetb->setColumnCount(12); // 11 champs + Actions
    ui->tableWidgetb->setRowCount(0);

    int row = 0;
    while (query.next()) {
        ui->tableWidgetb->insertRow(row);
        for (int col = 0; col < 11; ++col) {
            QString cellText;
            if (col == 7 || col == 8) {
                QDateTime dt = query.value(col).toDateTime();
                if (dt.isValid()) {
                    cellText = dt.date().toString("dd/MM/yyyy");
                } else {
                    cellText = query.value(col).toString();
                    int tIdx = cellText.indexOf('T');
                    if (tIdx > 0)
                        cellText = cellText.left(tIdx);
                }
            } else {
                cellText = query.value(col).toString();
            }
            ui->tableWidgetb->setItem(row, col, new QTableWidgetItem(cellText));
        }
        // Colonne Actions
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
        int currentRow = row;
        connect(btnModifier, &QPushButton::clicked, this, [this, currentRow]() {
            modifierBateauFromRow(currentRow);
        });
        connect(btnSupprimer, &QPushButton::clicked, this, [this, currentRow]() {
            supprimerBateauFromRow(currentRow);
        });
        ui->tableWidgetb->setCellWidget(row, 11, actionWidget);
        ++row;
    }

    // Afficher la colonne Actions
    ui->tableWidgetb->setColumnHidden(11, false);
    // Hide the unused column 12
    ui->tableWidgetb->setColumnHidden(12, true);

    ui->tableWidgetb->resizeColumnsToContents();
    ui->tableWidgetb->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidgetb->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->tableWidgetb->setColumnWidth(2, 110);
    ui->tableWidgetb->horizontalHeader()->setSectionResizeMode(8, QHeaderView::Fixed);
    ui->tableWidgetb->setColumnWidth(8, 105);
    ui->tableWidgetb->horizontalHeader()->setSectionResizeMode(9, QHeaderView::Fixed);
    ui->tableWidgetb->setColumnWidth(9, 100);
    ui->tableWidgetb->horizontalHeader()->setSectionResizeMode(10, QHeaderView::Fixed);
    ui->tableWidgetb->setColumnWidth(10, 100);
    ui->tableWidgetb->horizontalHeader()->setSectionResizeMode(11, QHeaderView::Fixed);
    ui->tableWidgetb->setColumnWidth(11, 72);

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

void MainWindow::on_pushButton_11_clicked()
{
    if (!handleCrudDisabled(this) || !ui) return;

    // Validation des champs obligatoires
    if (!validateRequiredFields(this, {
            {QStringLiteral("ID quai"), ui->lineEdit_4, [this]{ return !ui->lineEdit_4 || ui->lineEdit_4->text().trimmed().isEmpty(); }},
            {QStringLiteral("Nom du quai"), ui->lineEdit_6, [this]{ return !ui->lineEdit_6 || ui->lineEdit_6->text().trimmed().isEmpty(); }},
            {QStringLiteral("Longueur maximale"), ui->doubleSpinBox_2, [this]{ return !ui->doubleSpinBox_2 || ui->doubleSpinBox_2->value() <= 0.0; }},
            {QStringLiteral("Capacité maximale"), ui->spinBox_2, [this]{ return !ui->spinBox_2 || ui->spinBox_2->value() <= 0; }},
        })) {
        return;
    }

    // Récupérer les données du formulaire
    QString zonePort = ui->comboBox_6->currentText().trimmed();
    QString zoneCouverte = ui->comboBox_8->currentText().trimmed();
    QString statutUi = ui->comboBox_7->currentText().trimmed();

    // Adapter les valeurs aux contraintes CHECK Oracle
    // Zone_Port: 'Nord','Sud','Est','Ouest' -> déjà cohérent avec les valeurs de la combo
    // Zone_Couverte: 'Oui','Non' -> déjà cohérent
    // Statut: 'Libre','Occupe','Maintenance' (sans accent sur Occupe)
    QString statutDb = statutUi;
    if (statutUi.compare(QStringLiteral("Occupé"), Qt::CaseInsensitive) == 0 ||
        statutUi.startsWith(QStringLiteral("Occu"), Qt::CaseInsensitive)) {
        statutDb = QStringLiteral("Occupe");
    }

    QVariantMap donnees;
    donnees["ID_QUAI"] = ui->lineEdit_4->text().trimmed().toInt();
    donnees["NOM_QUAI"] = ui->lineEdit_6->text().trimmed();
    donnees["ZONE_PORT"] = zonePort;
    donnees["ZONE_COUVERTE"] = zoneCouverte;
    donnees["LONGUEUR"] = ui->doubleSpinBox_2->value();
    donnees["CAPACITE_QUAIS"] = ui->spinBox_2->value();
    donnees["STATUT"] = statutDb;

    bool succes = false;
    if (m_quai.isModeModification()) {
        // Mode modification
        succes = m_quai.modifier(m_quai.idEnCours(), donnees);
        if (succes) {
            QMessageBox::information(this, "Modification", "Quai modifié avec succès.");
            m_quai.setModeModification(false);
            ui->pushButton_11->setText("Ajouter");
        } else {
            const QString err = m_quai.lastError();
            QMessageBox::critical(this,
                                  QStringLiteral("Erreur"),
                                  err.isEmpty()
                                      ? QStringLiteral("Échec de la modification.")
                                      : QStringLiteral("Échec de la modification : %1").arg(err));
        }
    } else {
        // Mode ajout
        succes = m_quai.ajouter(donnees);
        if (succes) {
            QMessageBox::information(this, "Ajout", "Quai ajouté avec succès.");
        } else {
            const QString err = m_quai.lastError();
            QMessageBox::critical(this,
                                  QStringLiteral("Erreur"),
                                  err.isEmpty()
                                      ? QStringLiteral("Échec de l'ajout.")
                                      : QStringLiteral("Échec de l'ajout : %1").arg(err));
        }
    }

    if (succes) {
        refreshQuaiTable();
        // Vider le formulaire
        ui->lineEdit_4->clear();
        ui->lineEdit_6->clear();
        ui->comboBox_6->setCurrentIndex(0);
        ui->comboBox_8->setCurrentIndex(0);
        ui->doubleSpinBox_2->setValue(0.0);
        ui->spinBox_2->setValue(0);
        ui->comboBox_7->setCurrentIndex(0);
    }
}

// ----------------------
// Gestion des quais (helpers publics)
// ----------------------

void MainWindow::refreshQuaiTable()
{
    if (!ui || !ui->tableWidgetQuai) {
        return;
    }

    Connection *conn = Connection::getInstance();
    if (!conn->ensureOpen()) {
        QMessageBox::critical(this, QStringLiteral("DB"),
                              QStringLiteral("Connexion DB échouée: %1").arg(conn->lastErrorText()));
        return;
    }

    QSqlDatabase db = conn->getDatabase();

    const QHash<QString, QStringList> syn = {
        {QStringLiteral("ID_QUAI"), {QStringLiteral("ID_QUAI"), QStringLiteral("id_quai"), QStringLiteral("id")}},
        {QStringLiteral("NOM_QUAI"), {QStringLiteral("NOM_QUAI"), QStringLiteral("nom_quai"), QStringLiteral("nom")}},
        {QStringLiteral("ZONE_PORT"), {QStringLiteral("ZONE_PORT"), QStringLiteral("zone_port"), QStringLiteral("zoneport")}},
        {QStringLiteral("ZONE_COUVERTE"), {QStringLiteral("ZONE_COUVERTE"), QStringLiteral("zone_couverte"), QStringLiteral("zonecouverte")}},
        {QStringLiteral("LONGUEUR"), {QStringLiteral("LONGUEUR"), QStringLiteral("longueur")}},
        {QStringLiteral("CAPACITE_QUAIS"), {QStringLiteral("CAPACITE_QUAIS"), QStringLiteral("capacite_quais"), QStringLiteral("capacitemax")}},
        {QStringLiteral("STATUT"), {QStringLiteral("STATUT"), QStringLiteral("statut")}}
    };

    const QString tableName = QStringLiteral("QUAIS");

    reloadTableWidgetFromDb(ui->tableWidgetQuai, db, tableName,
                            {QStringLiteral("ID Quai"), QStringLiteral("Nom Quai"), QStringLiteral("Zone Port"),
                             QStringLiteral("Zone Couverte"), QStringLiteral("Longueur Max"),
                             QStringLiteral("Capacité Quais"), QStringLiteral("Statut")},
                            syn);

    ensureQuaiActionsColumn(QStringLiteral(
        "QPushButton {"
        " border: 2px solid rgb(0, 0, 112);"
        " border-radius: 7px;"
        " background-color: rgba(0, 0, 127,0.7);"
        " color: white;"
        " padding: 7px 20px;"
        " font-family: 'Segoe UI Emoji', 'Segoe UI', 'Arial', sans-serif;"
        " font-size: 18px;"
        " font-weight: bold;"
        " }"
        " QPushButton:hover { background-color: rgba(0, 0, 127,0.7); border-color: #59abc8; }"
        " QPushButton:pressed { background-color:rgba(0, 0, 127,0.9); }"));

    // Mettre à jour les statistiques d'occupation quand les quais changent
    on_btnRefreshStats_2_clicked();

    // Réappliquer les filtres (recherche, statut, zone, capacité)
    // après rechargement de la table.
    applyQuaiFilters();
}

void MainWindow::ensureQuaiActionsColumn(const QString &buttonStyle)
{
    if (!ui || !ui->tableWidgetQuai) {
        return;
    }

    QTableWidget *table = ui->tableWidgetQuai;

    ensureActionsColumnPopulated(table, buttonStyle);

    const int actionsCol = table->columnCount() - 1;
    if (actionsCol < 0) {
        return;
    }

    table->setColumnWidth(actionsCol, 140);

    for (int row = 0; row < table->rowCount(); ++row) {
        QWidget *container = table->cellWidget(row, actionsCol);
        if (!container) {
            continue;
        }

        const auto buttons = container->findChildren<QPushButton*>();
        if (buttons.size() < 2) {
            continue;
        }

        QPushButton *editBtn = buttons.at(0);
        QPushButton *deleteBtn = buttons.at(1);

        if (!editBtn || !deleteBtn) {
            continue;
        }

        editBtn->disconnect();
        deleteBtn->disconnect();

        QObject::connect(editBtn, &QPushButton::clicked, this, [this, row]() {
            editQuaiFromTable(row);
        });

        QObject::connect(deleteBtn, &QPushButton::clicked, this, [this, table, row]() {
            if (!table->item(row, 0)) {
                return;
            }
            bool ok = false;
            int id = table->item(row, 0)->text().toInt(&ok);
            if (!ok) {
                return;
            }
            deleteQuaiById(id, QString());
        });
    }
}

void MainWindow::showQuaiFormPage()
{
    if (ui && ui->stackedWidget && ui->page_3) {
        ui->stackedWidget->setCurrentWidget(ui->page_3);
    }
}

void MainWindow::setAddButtonText(const QString &text)
{
    if (ui && ui->pushButton_11) {
        ui->pushButton_11->setText(text);
    }
}

void MainWindow::deleteQuaiById(int id, const QString &)
{
    if (id <= 0) {
        return;
    }

    const auto reply = QMessageBox::question(this,
                                             QStringLiteral("Suppression"),
                                             QStringLiteral("Supprimer le quai %1 ?").arg(id),
                                             QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }

    if (!m_quai.supprimer(id)) {
        QMessageBox::critical(this,
                              QStringLiteral("Suppression"),
                              QStringLiteral("Échec de la suppression du quai."));
        return;
    }

    refreshQuaiTable();
}

void MainWindow::editQuaiFromTable(int row)
{
    if (!ui || !ui->tableWidgetQuai) {
        return;
    }

    QTableWidget *table = ui->tableWidgetQuai;

    if (row < 0 || row >= table->rowCount()) {
        return;
    }

    QTableWidgetItem *idItem = table->item(row, 0);
    if (!idItem) {
        return;
    }

    bool ok = false;
    int id = idItem->text().toInt(&ok);
    if (!ok) {
        return;
    }

    const QVariantMap infos = m_quai.chargerInfos(id);
    if (infos.isEmpty()) {
        QMessageBox::warning(this,
                             QStringLiteral("Édition quai"),
                             QStringLiteral("Impossible de charger les informations de ce quai."));
        return;
    }

    if (ui->lineEdit_4) ui->lineEdit_4->setText(QString::number(infos.value("ID_QUAI").toInt()));
    if (ui->lineEdit_6) ui->lineEdit_6->setText(infos.value("NOM_QUAI").toString());

    if (ui->comboBox_6) {
        const QString zonePort = infos.value("ZONE_PORT").toString();
        int idx = ui->comboBox_6->findText(zonePort);
        if (idx >= 0) ui->comboBox_6->setCurrentIndex(idx);
    }

    if (ui->comboBox_8) {
        const QString zoneCouverte = infos.value("ZONE_COUVERTE").toString();
        int idx = ui->comboBox_8->findText(zoneCouverte);
        if (idx >= 0) ui->comboBox_8->setCurrentIndex(idx);
    }

    if (ui->doubleSpinBox_2) ui->doubleSpinBox_2->setValue(infos.value("LONGUEUR").toDouble());
    if (ui->spinBox_2) ui->spinBox_2->setValue(infos.value("CAPACITE_QUAIS").toInt());

    if (ui->comboBox_7) {
        const QString statut = infos.value("STATUT").toString();
        int idx = ui->comboBox_7->findText(statut);
        if (idx >= 0) ui->comboBox_7->setCurrentIndex(idx);
    }

    m_quai.setModeModification(true, id);
    setAddButtonText(QStringLiteral("Modifier"));
    showQuaiFormPage();
}

// --- Filtres de recherche / statut / zone / capacité sur la page_3 (quais) ---

void MainWindow::applyQuaiFilters()
{
    if (!ui || !ui->tableWidgetQuai) {
        return;
    }

    QTableWidget *table = ui->tableWidgetQuai;

    const QString searchText = ui->lineEdit_3
            ? ui->lineEdit_3->text().trimmed().toLower()
            : QString();

    const QString statutFilter = ui->comboBox_3
            ? ui->comboBox_3->currentText()
            : QStringLiteral("Tous");

    const QString zoneFilter = ui->comboBox_4
            ? ui->comboBox_4->currentText()
            : QStringLiteral("Toutes");

    const QString capaciteFilter = ui->comboBox_5
            ? ui->comboBox_5->currentText()
            : QStringLiteral("Toutes");

    auto parseCapacity = [](const QString &text) -> int {
        const QString trimmed = text.trimmed();
        if (trimmed.isEmpty()) return 0;
        bool ok = false;
        // On essaie de lire le premier "mot" numérique
        const QString firstToken = trimmed.split(' ', Qt::SkipEmptyParts).value(0);
        int value = firstToken.toInt(&ok);
        if (!ok) {
            value = trimmed.toInt(&ok);
        }
        return ok ? value : 0;
    };

    const int rowCount = table->rowCount();
    const int colCount = table->columnCount();
    const int statutCol = 6; // colonne "Statut" dans tableWidgetQuai
    const int zoneCol   = 2; // colonne "Z du port" / Zone_Port
    const int capaCol   = 5; // colonne "C maximale" / capacité quais

    for (int row = 0; row < rowCount; ++row) {
        bool match = true;

        // 1) Filtre texte (recherche globale sur les colonnes de données)
        if (!searchText.isEmpty()) {
            bool found = false;
            for (int col = 0; col < colCount - 1; ++col) { // on ignore la colonne Actions (widget)
                QTableWidgetItem *item = table->item(row, col);
                if (item && item->text().toLower().contains(searchText)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                match = false;
            }
        }

        // 2) Filtre Statut
        if (match && !statutFilter.isEmpty() && statutFilter.compare(QStringLiteral("Tous"), Qt::CaseInsensitive) != 0) {
            QTableWidgetItem *statutItem = (statutCol >= 0 && statutCol < colCount)
                                           ? table->item(row, statutCol)
                                           : nullptr;
            const QString cellStatut = statutItem ? statutItem->text().trimmed() : QString();
            if (cellStatut.compare(statutFilter, Qt::CaseInsensitive) != 0) {
                match = false;
            }
        }

        // 3) Filtre Zone du port
        if (match && !zoneFilter.isEmpty() && zoneFilter.compare(QStringLiteral("Toutes"), Qt::CaseInsensitive) != 0) {
            QTableWidgetItem *zoneItem = (zoneCol >= 0 && zoneCol < colCount)
                                         ? table->item(row, zoneCol)
                                         : nullptr;
            const QString cellZone = zoneItem ? zoneItem->text().trimmed() : QString();
            if (cellZone.compare(zoneFilter, Qt::CaseInsensitive) != 0) {
                match = false;
            }
        }

        // 4) Filtre Capacité
        if (match && !capaciteFilter.isEmpty() && capaciteFilter.compare(QStringLiteral("Toutes"), Qt::CaseInsensitive) != 0) {
            QTableWidgetItem *capaItem = (capaCol >= 0 && capaCol < colCount)
                                         ? table->item(row, capaCol)
                                         : nullptr;
            const int capa = parseCapacity(capaItem ? capaItem->text() : QString());

            if (capaciteFilter.startsWith(QStringLiteral("<="))) {
                // "<= 5 bateaux"
                if (capa > 5) {
                    match = false;
                }
            } else if (capaciteFilter.startsWith(QStringLiteral("6-10"))) {
                if (capa < 6 || capa > 10) {
                    match = false;
                }
            } else if (capaciteFilter.startsWith(QStringLiteral(">"))) {
                if (capa <= 10) {
                    match = false;
                }
            }
        }

        table->setRowHidden(row, !match);
    }
}

void MainWindow::on_lineEdit_3_textChanged(const QString &)
{
    applyQuaiFilters();
}

void MainWindow::on_comboBox_3_currentIndexChanged(int)
{
    applyQuaiFilters();
}

void MainWindow::on_comboBox_4_currentIndexChanged(int)
{
    applyQuaiFilters();
}

void MainWindow::on_comboBox_5_currentIndexChanged(int)
{
    applyQuaiFilters();
}

void MainWindow::on_comboBoxCity_currentIndexChanged(int index)
{
    if (!ui || index < 0) return;
    m_selectedCity = ui->comboBoxCity->itemText(index);
    refreshWeatherForPage3();
}

// Boutons supplémentaires non encore utilisés explicitement dans la logique

void MainWindow::on_pushButton_6_clicked()
{
    // Depuis la page_3 (gestion des quais) : ouvrir la page_4 (carte + statistiques)
    if (ui && ui->stackedWidget && ui->page_4) {
        ui->stackedWidget->setCurrentWidget(ui->page_4);
        // Actualiser les statistiques à l'ouverture
        on_btnRefreshStats_2_clicked();
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    // Bouton retour sur la page_4 : revenir à la gestion des quais (page_3)
    if (ui && ui->stackedWidget && ui->page_3) {
        ui->stackedWidget->setCurrentWidget(ui->page_3);
    }
}

void MainWindow::on_pushButton_9_clicked()
{
    // Bouton "statistiques" sur la page_3 : même comportement que le bouton carte,
    // ouvre la page_4 avec les stats d'occupation.
    on_pushButton_6_clicked();
}

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
    int totalW = contentW;
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

    // Pie on the left, legend on the right
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
    int legendY = statsY + 10;
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

void MainWindow::on_pushButton_pdfb_2_clicked()
{
    // Exporter la table des quais (page_3) en PDF
    if (!ui || !ui->tableWidgetQuai) {
        return;
    }

    exportWidgetToPdf(ui->tableWidgetQuai,
                      QStringLiteral("quais.pdf"),
                      QStringLiteral("Exporter les quais en PDF"));
}

void MainWindow::on_btnExportStatsPDF_2_clicked()
{
    // Exporter le bloc complet des statistiques d'occupation (page_4) en PDF
    if (!ui || !ui->statsFrame_2) {
        return;
    }

    exportWidgetToPdf(ui->statsFrame_2,
                      QStringLiteral("statistiques_quais.pdf"),
                      QStringLiteral("Exporter les statistiques d'occupation"));
}

void MainWindow::on_comboChartType_2_currentIndexChanged(int)
{
    // Pour l'instant, le filtrage temporel fin n'est pas disponible.
    // On relance simplement le calcul des statistiques pour rester cohérent
    // et permettre une évolution future (24h / 7j / 30j).
    on_btnRefreshStats_2_clicked();
}

// --- Export PDF générique pour un widget donné ---

bool MainWindow::exportWidgetToPdf(QWidget *widget,
                                   const QString &defaultFileName,
                                   const QString &dialogTitle)
{
    if (!widget) {
        return false;
    }

    const QString filePath = QFileDialog::getSaveFileName(
                this,
                dialogTitle,
                defaultFileName,
                QStringLiteral("Fichiers PDF (*.pdf)"));

    if (filePath.isEmpty()) {
        return false; // annulé par l'utilisateur
    }

    QPdfWriter writer(filePath);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageOrientation(QPageLayout::Portrait);
    writer.setTitle(dialogTitle);

    QPainter painter(&writer);
    if (!painter.isActive()) {
        QMessageBox::warning(this,
                             QStringLiteral("Export PDF"),
                             QStringLiteral("Impossible de créer le fichier PDF."));
        return false;
    }

    const QRect pageRect = writer.pageLayout().paintRectPixels(writer.resolution());
    const QSize widgetSize = widget->size();

    if (widgetSize.isEmpty()) {
        return false;
    }

    const double xScale = static_cast<double>(pageRect.width())  / widgetSize.width();
    const double yScale = static_cast<double>(pageRect.height()) / widgetSize.height();
    const double scale  = std::min(xScale, yScale);

    painter.translate(pageRect.x(), pageRect.y());
    painter.scale(scale, scale);
    widget->render(&painter);
    painter.end();

    QMessageBox::information(this,
                             QStringLiteral("Export PDF"),
                             QStringLiteral("Exportation terminée :\n%1")
                                 .arg(QDir::toNativeSeparators(filePath)));

    return true;
}

// Actualisation des statistiques d'occupation des quais (page_4)

void MainWindow::on_btnRefreshStats_2_clicked()
{
    if (!ui) return;

    QMap<QString, QPair<int, int>> stats; // zone -> (occupés, total en base)

    // Normalise les libellés de zone pour correspondre exactement
    // aux clés attendues ("Nord", "Sud", "Est", "Ouest"),
    // quel que soit le texte stocké en base.
    auto normalizeZone = [](const QString &raw) -> QString {
        const QString lower = raw.trimmed().toLower();
        if (lower == QStringLiteral("nord"))  return QStringLiteral("Nord");
        if (lower == QStringLiteral("sud"))   return QStringLiteral("Sud");
        if (lower == QStringLiteral("est"))   return QStringLiteral("Est");
        if (lower == QStringLiteral("ouest")) return QStringLiteral("Ouest");
        return raw.trimmed();
    };

    // Calcul des statistiques *uniquement* à partir de la base QUAIS
    // pour garantir que les valeurs affichées correspondent aux données réelles.
    Connection *conn = Connection::getInstance();
    if (!conn->ensureOpen()) {
        QMessageBox::critical(this,
                              QStringLiteral("DB"),
                              QStringLiteral("Connexion DB échouée: %1").arg(conn->lastErrorText()));
        return;
    }

    QSqlDatabase db = conn->getDatabase();

    QSqlQuery query(db);
    if (!query.exec(QStringLiteral(
            "SELECT Zone_Port, COUNT(*) AS total, "
            "SUM(CASE WHEN UPPER(Statut) <> 'LIBRE' THEN 1 ELSE 0 END) AS occupe "
            "FROM QUAIS GROUP BY Zone_Port"))) {
        QMessageBox::warning(this,
                             QStringLiteral("Statistiques"),
                             QStringLiteral("Impossible de calculer les statistiques : %1")
                                 .arg(query.lastError().text()));
        return;
    }

    while (query.next()) {
        const QString zone = normalizeZone(query.value(0).toString());
        const int total = query.value(1).toInt();
        const int occupe = query.value(2).toInt();
        stats.insert(zone, qMakePair(occupe, total));
    }

    // Pourcentage d'occupation à l'intérieur d'une zone
    auto computePct = [&stats](const QString &zone) -> int {
        const auto it = stats.constFind(zone);
        if (it == stats.constEnd() || it->second == 0)
            return 0;
        const int occupe = it->first;
        const int total = it->second;
        if (total <= 0)
            return 0;
        return (occupe * 100) / total;
    };

    auto getZoneCounts = [&stats](const QString &zone) -> QPair<int, int> {
        const auto it = stats.constFind(zone);
        if (it == stats.constEnd())
            return qMakePair(0, 0);
        return qMakePair(it->first, it->second);
    };

    const int pctNord  = computePct(QStringLiteral("Nord"));
    const int pctSud   = computePct(QStringLiteral("Sud"));
    const int pctEst   = computePct(QStringLiteral("Est"));
    const int pctOuest = computePct(QStringLiteral("Ouest"));

    // Stat globale d'occupation (tous les quais confondus),
    // basée sur les mêmes données "stats" (table ou base).
    int totalOccupe = 0;
    int totalQuais  = 0;
    for (auto it = stats.constBegin(); it != stats.constEnd(); ++it) {
        totalOccupe += it->first;
        totalQuais  += it->second;
    }

    const int pctGlobal = (totalQuais > 0) ? (totalOccupe * 100) / totalQuais : 0;

    // Répartition des quais par zone (pour le bloc "RÉPARTITION PAR ZONE") :
    // pourcentage du nombre total de quais appartenant à chaque zone.
    auto computeShare = [&stats, totalQuais](const QString &zone) -> int {
        if (totalQuais <= 0)
            return 0;
        const auto it = stats.constFind(zone);
        if (it == stats.constEnd())
            return 0;
        const int total = it->second;
        if (total <= 0)
            return 0;
        return (total * 100) / totalQuais;
    };

    const int shareNord  = computeShare(QStringLiteral("Nord"));
    const int shareSud   = computeShare(QStringLiteral("Sud"));
    const int shareEst   = computeShare(QStringLiteral("Est"));
    const int shareOuest = computeShare(QStringLiteral("Ouest"));

    // On utilise ces pourcentages de répartition pour les barres verticales
    // "RÉPARTITION PAR ZONE".
    if (ui->progressZoneNord_2) {
        ui->progressZoneNord_2->setRange(0, 100);
        ui->progressZoneNord_2->setValue(shareNord);
    }
    if (ui->progressZoneSud_2) {
        ui->progressZoneSud_2->setRange(0, 100);
        ui->progressZoneSud_2->setValue(shareSud);
    }
    if (ui->progressZoneEst_2) {
        ui->progressZoneEst_2->setRange(0, 100);
        ui->progressZoneEst_2->setValue(shareEst);
    }
    if (ui->progressZoneOuest_2) {
        ui->progressZoneOuest_2->setRange(0, 100);
        ui->progressZoneOuest_2->setValue(shareOuest);
    }

    if (ui->label_chartTitle_2) {
        ui->label_chartTitle_2->setText(
            QStringLiteral("Évolution du taux d'occupation (%1 %)").arg(pctGlobal));
    }

    // Tooltips sur les points de la courbe pour donner les stats
    // d'occupation par zone directement depuis la base.
    const auto nordCounts  = getZoneCounts(QStringLiteral("Nord"));
    const auto sudCounts   = getZoneCounts(QStringLiteral("Sud"));
    const auto estCounts   = getZoneCounts(QStringLiteral("Est"));
    const auto ouestCounts = getZoneCounts(QStringLiteral("Ouest"));

    const QString ttNordOcc  = QStringLiteral("Zone Nord : %1 % occupé (%2 / %3 quais)")
                                   .arg(pctNord)
                                   .arg(nordCounts.first)
                                   .arg(nordCounts.second);
    const QString ttSudOcc   = QStringLiteral("Zone Sud : %1 % occupé (%2 / %3 quais)")
                                   .arg(pctSud)
                                   .arg(sudCounts.first)
                                   .arg(sudCounts.second);
    const QString ttEstOcc   = QStringLiteral("Zone Est : %1 % occupé (%2 / %3 quais)")
                                   .arg(pctEst)
                                   .arg(estCounts.first)
                                   .arg(estCounts.second);
    const QString ttOuestOcc = QStringLiteral("Zone Ouest : %1 % occupé (%2 / %3 quais)")
                                   .arg(pctOuest)
                                   .arg(ouestCounts.first)
                                   .arg(ouestCounts.second);

    // Occupation par zone pour la courbe
    if (ui->curvePoint1_2) ui->curvePoint1_2->setToolTip(ttNordOcc);
    if (ui->curvePoint3_2) ui->curvePoint3_2->setToolTip(ttSudOcc);
    if (ui->curvePoint5_2) ui->curvePoint5_2->setToolTip(ttEstOcc);
    if (ui->curvePoint7_2) ui->curvePoint7_2->setToolTip(ttOuestOcc);

    // Répartition du nombre de quais pour les barres verticales
    const QString ttNordShare  = QStringLiteral("Zone Nord : %1 % des quais (%2 / %3, %4 % occupés)")
                                     .arg(shareNord)
                                     .arg(nordCounts.second)
                                     .arg(totalQuais)
                                     .arg(pctNord);
    const QString ttSudShare   = QStringLiteral("Zone Sud : %1 % des quais (%2 / %3, %4 % occupés)")
                                     .arg(shareSud)
                                     .arg(sudCounts.second)
                                     .arg(totalQuais)
                                     .arg(pctSud);
    const QString ttEstShare   = QStringLiteral("Zone Est : %1 % des quais (%2 / %3, %4 % occupés)")
                                     .arg(shareEst)
                                     .arg(estCounts.second)
                                     .arg(totalQuais)
                                     .arg(pctEst);
    const QString ttOuestShare = QStringLiteral("Zone Ouest : %1 % des quais (%2 / %3, %4 % occupés)")
                                     .arg(shareOuest)
                                     .arg(ouestCounts.second)
                                     .arg(totalQuais)
                                     .arg(pctOuest);

    if (ui->progressZoneNord_2)  ui->progressZoneNord_2->setToolTip(ttNordShare);
    if (ui->progressZoneSud_2)   ui->progressZoneSud_2->setToolTip(ttSudShare);
    if (ui->progressZoneEst_2)   ui->progressZoneEst_2->setToolTip(ttEstShare);
    if (ui->progressZoneOuest_2) ui->progressZoneOuest_2->setToolTip(ttOuestShare);

    // -------------------------------------------------------------
    // Mise à jour de la carte (zoneNord_map, zoneSud_map, etc.)
    // -------------------------------------------------------------
    auto updateZoneMap = [&](const QString &zoneName,
                             QFrame *zoneFrame,
                             QLabel *valueLabel,
                             int occupe, int total) {
        Q_UNUSED(zoneName);
        Q_UNUSED(occupe);
        Q_UNUSED(total);
        if (!zoneFrame) return;

        // Pas de grand carreau coloré : fond transparent, pas de bordure
        zoneFrame->setStyleSheet(QStringLiteral("background-color: transparent; border: none;"));
        // Retirer les contraintes de taille fixe pour que le frame s'adapte
        zoneFrame->setMinimumSize(0, 0);
        zoneFrame->setMaximumSize(16777215, 16777215);

        // Cacher le label "X / Y occupés"
        if (valueLabel) valueLabel->hide();
    };

    updateZoneMap(QStringLiteral("Nord"),  ui->zoneNord_map,  ui->value_zoneNord_map,  nordCounts.first,  nordCounts.second);
    updateZoneMap(QStringLiteral("Sud"),   ui->zoneSud_map,   ui->value_zoneSud_map,   sudCounts.first,   sudCounts.second);
    updateZoneMap(QStringLiteral("Est"),   ui->zoneEst_map,   ui->value_zoneEst_map,   estCounts.first,   estCounts.second);
    updateZoneMap(QStringLiteral("Ouest"), ui->zoneOuest_map, ui->value_zoneOuest_map, ouestCounts.first, ouestCounts.second);

    // Cacher aussi les labels de titre de zone (label_zone*_map)
    if (ui->label_zoneNord_map)  ui->label_zoneNord_map->hide();
    if (ui->label_zoneSud_map)   ui->label_zoneSud_map->hide();
    if (ui->label_zoneEst_map)   ui->label_zoneEst_map->hide();
    if (ui->label_zoneOuest_map) ui->label_zoneOuest_map->hide();

    // Animation simple de la courbe : on positionne verticalement
    // les points en fonction des pourcentages calculés.
    if (ui->curveCanvas_2) {
        const int yTop = 30;   // proche de 100 %
        const int yBot = 145;  // proche de 0 %
        auto yFromPct = [yTop, yBot](int pct) {
            if (pct < 0) pct = 0;
            if (pct > 100) pct = 100;
            return yBot - (pct * (yBot - yTop)) / 100;
        };

        // Points principaux par zone
        if (ui->curvePoint1_2) {
            const QPoint p = ui->curvePoint1_2->pos();
            ui->curvePoint1_2->move(p.x(), yFromPct(pctNord));
        }
        if (ui->curvePoint3_2) {
            const QPoint p = ui->curvePoint3_2->pos();
            ui->curvePoint3_2->move(p.x(), yFromPct(pctSud));
        }
        if (ui->curvePoint5_2) {
            const QPoint p = ui->curvePoint5_2->pos();
            ui->curvePoint5_2->move(p.x(), yFromPct(pctEst));
        }
        if (ui->curvePoint7_2) {
            const QPoint p = ui->curvePoint7_2->pos();
            ui->curvePoint7_2->move(p.x(), yFromPct(pctOuest));
        }

        // Points intermédiaires pour lisser la courbe : moyenne entre zones voisines
        if (ui->curvePoint2_2 && ui->curvePoint1_2 && ui->curvePoint3_2) {
            const int pct12 = (pctNord + pctSud) / 2;
            const QPoint p = ui->curvePoint2_2->pos();
            ui->curvePoint2_2->move(p.x(), yFromPct(pct12));
        }
        if (ui->curvePoint4_2 && ui->curvePoint3_2 && ui->curvePoint5_2) {
            const int pct34 = (pctSud + pctEst) / 2;
            const QPoint p = ui->curvePoint4_2->pos();
            ui->curvePoint4_2->move(p.x(), yFromPct(pct34));
        }
        if (ui->curvePoint6_2 && ui->curvePoint5_2 && ui->curvePoint7_2) {
            const int pct56 = (pctEst + pctOuest) / 2;
            const QPoint p = ui->curvePoint6_2->pos();
            ui->curvePoint6_2->move(p.x(), yFromPct(pct56));
        }

        // Si tu ajoutes plus tard un point dédié au taux global
        // (par exemple curvePoint8_2 dans le .ui), tu pourras ici
        // le positionner en fonction de pctGlobal.

        // Dessine une ligne colorée reliant tous les points
        if (!m_curveLineLabelQuaiStats) {
            m_curveLineLabelQuaiStats = new QLabel(ui->curveCanvas_2);
            m_curveLineLabelQuaiStats->setObjectName("curveLineLabelQuaiStats");
            m_curveLineLabelQuaiStats->setAttribute(Qt::WA_TransparentForMouseEvents);
            m_curveLineLabelQuaiStats->setGeometry(ui->curveCanvas_2->rect());
        } else {
            m_curveLineLabelQuaiStats->setGeometry(ui->curveCanvas_2->rect());
        }

        QPixmap pix(ui->curveCanvas_2->size());
        pix.fill(Qt::transparent);
        QPainter painter(&pix);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QVector<QPoint> curvePoints;

        auto centerInCanvas = [this](QWidget* w) -> QPoint {
            if (!w) return QPoint();
            const QPoint topLeft = w->mapTo(this->ui->curveCanvas_2, QPoint(0, 0));
            return QPoint(topLeft.x() + w->width() / 2, topLeft.y() + w->height() / 2);
        };

        curvePoints << centerInCanvas(ui->curvePoint1_2)
                << centerInCanvas(ui->curvePoint2_2)
                << centerInCanvas(ui->curvePoint3_2)
                << centerInCanvas(ui->curvePoint4_2)
                << centerInCanvas(ui->curvePoint5_2)
                << centerInCanvas(ui->curvePoint6_2)
                << centerInCanvas(ui->curvePoint7_2);

        // Supprime les points nuls éventuels
        QVector<QPoint> validPoints;
        validPoints.reserve(curvePoints.size());
        for (const QPoint &pt : curvePoints) {
            if (!pt.isNull())
                validPoints.append(pt);
        }

        // Tracer des lignes de grille horizontales pour les niveaux clés (0,25,50,75,100 %)
        {
            QPen gridPen(QColor(0, 0, 128, 40));
            gridPen.setWidth(1);
            painter.setPen(gridPen);
            const int levels[] = {0, 25, 50, 75, 100};
            for (int lvl : levels) {
                const int y = yFromPct(lvl);
                painter.drawLine(10, y, pix.width() - 10, y);
            }
        }

        if (validPoints.size() >= 2) {
            // Courbe passant exactement par tous les points
            QPainterPath path(validPoints.first());
            for (int i = 1; i < validPoints.size(); ++i) {
                path.lineTo(validPoints[i]);
            }

            // Zone remplie sous la courbe
            QPainterPath fillPath(path);
            const qreal bottomY = pix.height() - 10;
            fillPath.lineTo(validPoints.last().x(), bottomY);
            fillPath.lineTo(validPoints.first().x(), bottomY);
            fillPath.closeSubpath();

            QLinearGradient grad(0, yTop, 0, bottomY);
            grad.setColorAt(0.0, QColor(52, 152, 219, 90));
            grad.setColorAt(1.0, QColor(52, 152, 219, 10));
            painter.fillPath(fillPath, grad);

            // Tracer la courbe principale par-dessus
            QPen mainPen(QColor(52, 152, 219));
            mainPen.setWidth(4);
            mainPen.setCapStyle(Qt::RoundCap);
            mainPen.setJoinStyle(Qt::RoundJoin);
            painter.setPen(mainPen);
            painter.drawPath(path);
        }

        painter.end();
        m_curveLineLabelQuaiStats->setPixmap(pix);
        m_curveLineLabelQuaiStats->lower();
        // On s'assure que les points restent au-dessus de la ligne
        if (ui->curvePoint1_2) ui->curvePoint1_2->raise();
        if (ui->curvePoint2_2) ui->curvePoint2_2->raise();
        if (ui->curvePoint3_2) ui->curvePoint3_2->raise();
        if (ui->curvePoint4_2) ui->curvePoint4_2->raise();
        if (ui->curvePoint5_2) ui->curvePoint5_2->raise();
        if (ui->curvePoint6_2) ui->curvePoint6_2->raise();
        if (ui->curvePoint7_2) ui->curvePoint7_2->raise();
    }

    // -------------------------------------------------------------
    // Affichage détaillé des places de quai par zone (carte)
    // -------------------------------------------------------------
    if (db.isOpen()) {
        QSqlQuery queryPlaces(db);
        if (queryPlaces.exec(QStringLiteral("SELECT ID_QUAI, ZONE_PORT, STATUT FROM QUAIS"))) {
            // Regrouper les quais par zone
            QMap<QString, QList<QPair<int, QString>>> quaisParZone;
            while (queryPlaces.next()) {
                int id = queryPlaces.value(0).toInt();
                QString zone = queryPlaces.value(1).toString().trimmed();
                QString statut = queryPlaces.value(2).toString().trimmed();

                // Normaliser le nom de la zone
                if (zone.compare(QStringLiteral("Nord"), Qt::CaseInsensitive) == 0) zone = QStringLiteral("Nord");
                else if (zone.compare(QStringLiteral("Sud"), Qt::CaseInsensitive) == 0) zone = QStringLiteral("Sud");
                else if (zone.compare(QStringLiteral("Est"), Qt::CaseInsensitive) == 0) zone = QStringLiteral("Est");
                else if (zone.compare(QStringLiteral("Ouest"), Qt::CaseInsensitive) == 0) zone = QStringLiteral("Ouest");
                else continue; // ignorer les zones inconnues

                quaisParZone[zone].append({id, statut});
            }

            // Fonction pour mettre à jour l'affichage d'une zone donnée
            auto updateZonePlaces = [&](const QString &zone, QVBoxLayout *layout) {
                if (!layout) return;

                // Supprimer les anciens widgets "places" s'ils existent
                QList<QWidget*> toDelete;
                for (int i = 0; i < layout->count(); ++i) {
                    QWidget *w = layout->itemAt(i) ? layout->itemAt(i)->widget() : nullptr;
                    if (w && w->objectName().startsWith(QStringLiteral("placesZone"))) {
                        toDelete.append(w);
                    }
                }
                for (QWidget *w : toDelete) delete w;

                // Conteneur principal de la zone
                QWidget *placesWidget = new QWidget();
                placesWidget->setObjectName(QStringLiteral("placesZone") + zone);
                placesWidget->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
                QVBoxLayout *mainVLayout = new QVBoxLayout(placesWidget);
                mainVLayout->setSpacing(5);
                mainVLayout->setContentsMargins(3, 3, 3, 3);

                // Emoji de direction
                QString zoneIcon;
                QString zoneBadgeColor;
                if (zone == QStringLiteral("Nord")) {
                    zoneIcon = QStringLiteral("⬆️");
                    zoneBadgeColor = QStringLiteral("#1565c0");
                } else if (zone == QStringLiteral("Sud")) {
                    zoneIcon = QStringLiteral("⬇️");
                    zoneBadgeColor = QStringLiteral("#2e7d32");
                } else if (zone == QStringLiteral("Est")) {
                    zoneIcon = QStringLiteral("➡️");
                    zoneBadgeColor = QStringLiteral("#f57f17");
                } else if (zone == QStringLiteral("Ouest")) {
                    zoneIcon = QStringLiteral("⬅️");
                    zoneBadgeColor = QStringLiteral("#6a1b9a");
                }

                // Titre de la zone : badge coloré avec ombre
                const auto &list = quaisParZone.value(zone);
                QLabel *zoneTitle = new QLabel(QStringLiteral("  %1 %2  ").arg(zoneIcon).arg(zone));
                zoneTitle->setAlignment(Qt::AlignCenter);
                zoneTitle->setStyleSheet(QStringLiteral(
                    "color: white; font-weight: bold; font-size: 11px; "
                    "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
                    "stop:0 %1, stop:1 %2); border: none; "
                    "border-radius: 10px; padding: 4px 14px; "
                    "letter-spacing: 1px;")
                    .arg(zoneBadgeColor)
                    .arg(QColor(zoneBadgeColor).lighter(130).name()));
                mainVLayout->addWidget(zoneTitle, 0, Qt::AlignCenter);

                // Grille pour les carreaux
                QWidget *gridWidget = new QWidget();
                gridWidget->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
                QGridLayout *grid = new QGridLayout(gridWidget);
                grid->setSpacing(4);
                grid->setContentsMargins(0, 2, 0, 0);

                const int maxCols = 3;
                for (int i = 0; i < list.size(); ++i) {
                    int qid = list[i].first;
                    QString statut = list[i].second;
                    int row = i / maxCols;
                    int col = i % maxCols;

                    // Couleurs selon le statut avec dégradé
                    QString bgStart, bgEnd, borderColor, statusEmoji;
                    if (statut.compare(QStringLiteral("Libre"), Qt::CaseInsensitive) == 0) {
                        bgStart = QStringLiteral("#66bb6a");
                        bgEnd = QStringLiteral("#43a047");
                        borderColor = QStringLiteral("#2e7d32");
                        statusEmoji = QStringLiteral("✅");
                    } else if (statut.compare(QStringLiteral("Occupe"), Qt::CaseInsensitive) == 0) {
                        bgStart = QStringLiteral("#ef5350");
                        bgEnd = QStringLiteral("#e53935");
                        borderColor = QStringLiteral("#c62828");
                        statusEmoji = QStringLiteral("🚫");
                    } else if (statut.compare(QStringLiteral("Maintenance"), Qt::CaseInsensitive) == 0) {
                        bgStart = QStringLiteral("#ffa726");
                        bgEnd = QStringLiteral("#fb8c00");
                        borderColor = QStringLiteral("#ef6c00");
                        statusEmoji = QStringLiteral("🛠️");
                    } else {
                        bgStart = QStringLiteral("#90a4ae");
                        bgEnd = QStringLiteral("#78909c");
                        borderColor = QStringLiteral("#546e7a");
                        statusEmoji = QStringLiteral("❓");
                    }

                    // Carreau avec dégradé, ID + emoji statut
                    QWidget *tileWidget = new QWidget();
                    tileWidget->setFixedSize(46, 50);
                    tileWidget->setToolTip(QStringLiteral(
                        "🚢 Quai %1\n📍 Zone : %2\n📊 Statut : %3")
                        .arg(qid).arg(zone).arg(statut));
                    tileWidget->setStyleSheet(QStringLiteral(
                        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
                        "stop:0 %1, stop:1 %2); "
                        "border: 2px solid %3; border-radius: 12px;")
                        .arg(bgStart).arg(bgEnd).arg(borderColor));

                    QVBoxLayout *tileLay = new QVBoxLayout(tileWidget);
                    tileLay->setSpacing(0);
                    tileLay->setContentsMargins(2, 3, 2, 3);

                    // ID du quai
                    QLabel *idLbl = new QLabel(QString::number(qid));
                    idLbl->setAlignment(Qt::AlignCenter);
                    idLbl->setStyleSheet(QStringLiteral(
                        "color: white; font-weight: bold; font-size: 11px; "
                        "background: transparent; border: none; "
                        "text-shadow: 1px 1px 2px rgba(0,0,0,0.3);"));
                    tileLay->addWidget(idLbl);

                    // Emoji statut
                    QLabel *iconLbl = new QLabel(statusEmoji);
                    iconLbl->setAlignment(Qt::AlignCenter);
                    iconLbl->setStyleSheet(QStringLiteral(
                        "font-size: 12px; background: transparent; border: none;"));
                    tileLay->addWidget(iconLbl);

                    grid->addWidget(tileWidget, row, col, Qt::AlignCenter);
                }

                mainVLayout->addWidget(gridWidget, 0, Qt::AlignCenter);

                // Si aucun quai dans cette zone
                if (list.isEmpty()) {
                    QLabel *emptyLabel = new QLabel(QStringLiteral("🚧 Aucun quai"));
                    emptyLabel->setAlignment(Qt::AlignCenter);
                    emptyLabel->setStyleSheet(QStringLiteral(
                        "color: #90a4ae; font-size: 9px; font-style: italic; "
                        "background: transparent; border: none;"));
                    mainVLayout->addWidget(emptyLabel);
                }

                layout->addWidget(placesWidget, 0, Qt::AlignCenter);
            };

            // Mettre à jour chaque zone
            updateZonePlaces(QStringLiteral("Nord"),  ui->verticalLayout_zoneNord);
            updateZonePlaces(QStringLiteral("Sud"),   ui->verticalLayout_zoneSud);
            updateZonePlaces(QStringLiteral("Est"),   ui->verticalLayout_zoneEst);
            updateZonePlaces(QStringLiteral("Ouest"), ui->verticalLayout_zoneOuest);
        }
    }
}

void MainWindow::on_btnExportMapPdf_clicked()
{
    if (!ui || !ui->mapContainer) return;
    exportWidgetToPdf(ui->mapContainer,
                      QStringLiteral("carte_port.pdf"),
                      QStringLiteral("Exporter la carte du port en PDF"));
}

void MainWindow::updateWeatherLabels(const QString& icon,
                                     const QString& temperatureText,
                                     const QString& descriptionText,
                                     const QString& windText,
                                     const QString& humidityText)
{
    if (!ui) return;

    if (ui->labelMeteoIcon) {
        ui->labelMeteoIcon->setStyleSheet(QStringLiteral(
            "qproperty-alignment: AlignCenter;"
            "font-size: 28px;"
            "color: rgb(0, 0, 90);"
            "image: none;"));
        ui->labelMeteoIcon->setText(icon);
    }
    if (ui->labelMeteoTemp) {
        ui->labelMeteoTemp->setText(temperatureText);
    }
    if (ui->labelMeteoDesc) {
        ui->labelMeteoDesc->setText(descriptionText);
    }
    if (ui->labelMeteoWind) {
        ui->labelMeteoWind->setText(windText);
    }
    if (ui->labelMeteoHumidity) {
        ui->labelMeteoHumidity->setText(humidityText);
    }
}

void MainWindow::refreshWeatherForPage3()
{
    if (!ui || !m_weatherNetwork) return;

    updateWeatherLabels(QStringLiteral("⏳"),
                        QStringLiteral("--°C"),
                        QStringLiteral("Chargement météo..."),
                        QStringLiteral("🌬️ Vent: -- km/h"),
                        QStringLiteral("💧 Humidité: --%"));

    QUrl geoUrl(QStringLiteral("https://geocoding-api.open-meteo.com/v1/search"));
    QUrlQuery geoQuery;
    geoQuery.addQueryItem(QStringLiteral("name"), m_selectedCity);
    geoQuery.addQueryItem(QStringLiteral("count"), QStringLiteral("1"));
    geoQuery.addQueryItem(QStringLiteral("language"), QStringLiteral("fr"));
    geoQuery.addQueryItem(QStringLiteral("format"), QStringLiteral("json"));
    geoUrl.setQuery(geoQuery);

    QNetworkReply* geoReply = m_weatherNetwork->get(QNetworkRequest(geoUrl));
    connect(geoReply, &QNetworkReply::finished, this, [this, geoReply]() {
        const QByteArray geoPayload = geoReply->readAll();
        const QNetworkReply::NetworkError geoError = geoReply->error();
        geoReply->deleteLater();

        if (geoError != QNetworkReply::NoError) {
            updateWeatherLabels(QStringLiteral("⚠️"),
                                QStringLiteral("--°C"),
                                QStringLiteral("Météo indisponible"),
                                QStringLiteral("🌬️ Vent: -- km/h"),
                                QStringLiteral("💧 Humidité: --%"));
            return;
        }

        QJsonParseError parseGeoError;
        const QJsonDocument geoDoc = QJsonDocument::fromJson(geoPayload, &parseGeoError);
        if (parseGeoError.error != QJsonParseError::NoError || !geoDoc.isObject()) {
            updateWeatherLabels(QStringLiteral("⚠️"),
                                QStringLiteral("--°C"),
                                QStringLiteral("Réponse météo invalide"),
                                QStringLiteral("🌬️ Vent: -- km/h"),
                                QStringLiteral("💧 Humidité: --%"));
            return;
        }

        const QJsonObject geoObj = geoDoc.object();
        const QJsonArray results = geoObj.value(QStringLiteral("results")).toArray();
        if (results.isEmpty() || !results.first().isObject()) {
            updateWeatherLabels(QStringLiteral("⚠️"),
                                QStringLiteral("--°C"),
                                QStringLiteral("Localisation météo introuvable"),
                                QStringLiteral("🌬️ Vent: -- km/h"),
                                QStringLiteral("💧 Humidité: --%"));
            return;
        }

        const QJsonObject firstResult = results.first().toObject();
        const double latitude = firstResult.value(QStringLiteral("latitude")).toDouble();
        const double longitude = firstResult.value(QStringLiteral("longitude")).toDouble();
        const QString cityName = firstResult.value(QStringLiteral("name")).toString(m_selectedCity);

        QUrl weatherUrl(QStringLiteral("https://api.open-meteo.com/v1/forecast"));
        QUrlQuery weatherQuery;
        weatherQuery.addQueryItem(QStringLiteral("latitude"), QString::number(latitude, 'f', 6));
        weatherQuery.addQueryItem(QStringLiteral("longitude"), QString::number(longitude, 'f', 6));
        weatherQuery.addQueryItem(QStringLiteral("current"),
                                  QStringLiteral("temperature_2m,relative_humidity_2m,wind_speed_10m,weather_code"));
        weatherQuery.addQueryItem(QStringLiteral("timezone"), QStringLiteral("auto"));
        weatherUrl.setQuery(weatherQuery);

        QNetworkReply* weatherReply = m_weatherNetwork->get(QNetworkRequest(weatherUrl));
        connect(weatherReply, &QNetworkReply::finished, this, [this, weatherReply, cityName]() {
            const QByteArray weatherPayload = weatherReply->readAll();
            const QNetworkReply::NetworkError weatherError = weatherReply->error();
            weatherReply->deleteLater();

            if (weatherError != QNetworkReply::NoError) {
                updateWeatherLabels(QStringLiteral("⚠️"),
                                    QStringLiteral("--°C"),
                                    QStringLiteral("Météo indisponible (%1)").arg(cityName),
                                    QStringLiteral("🌬️ Vent: -- km/h"),
                                    QStringLiteral("💧 Humidité: --%"));
                return;
            }

            QJsonParseError parseWeatherError;
            const QJsonDocument weatherDoc = QJsonDocument::fromJson(weatherPayload, &parseWeatherError);
            if (parseWeatherError.error != QJsonParseError::NoError || !weatherDoc.isObject()) {
                updateWeatherLabels(QStringLiteral("⚠️"),
                                    QStringLiteral("--°C"),
                                    QStringLiteral("Données météo invalides (%1)").arg(cityName),
                                    QStringLiteral("🌬️ Vent: -- km/h"),
                                    QStringLiteral("💧 Humidité: --%"));
                return;
            }

            const QJsonObject root = weatherDoc.object();
            const QJsonObject current = root.value(QStringLiteral("current")).toObject();
            if (current.isEmpty()) {
                updateWeatherLabels(QStringLiteral("⚠️"),
                                    QStringLiteral("--°C"),
                                    QStringLiteral("Aucune météo courante (%1)").arg(cityName),
                                    QStringLiteral("🌬️ Vent: -- km/h"),
                                    QStringLiteral("💧 Humidité: --%"));
                return;
            }

            const double temp = current.value(QStringLiteral("temperature_2m")).toDouble();
            const int humidity = current.value(QStringLiteral("relative_humidity_2m")).toInt();
            const double wind = current.value(QStringLiteral("wind_speed_10m")).toDouble();
            const int code = current.value(QStringLiteral("weather_code")).toInt();

            const QString icon = weatherIconEmoji(code);
            const QString desc = weatherDescriptionFr(code);
            const QString tempText = QStringLiteral("%1°C").arg(QString::number(temp, 'f', 1));
            const QString descText = QStringLiteral("%1 • %2").arg(cityName, desc);
            const QString windText = QStringLiteral("🌬️ Vent: %1 km/h")
                                         .arg(QString::number(wind, 'f', 1));
            const QString humidityText = QStringLiteral("💧 Humidité: %1%")
                                             .arg(QString::number(humidity));

            updateWeatherLabels(icon, tempText, descText, windText, humidityText);
        });
    });
}

static QString weatherDescriptionFr(int weatherCode)
{
    if (weatherCode == 0) return QStringLiteral("Ensoleillé");
    if (weatherCode >= 1 && weatherCode <= 3) return QStringLiteral("Partiellement nuageux");
    if (weatherCode == 45 || weatherCode == 48) return QStringLiteral("Brouillard");
    if ((weatherCode >= 51 && weatherCode <= 57) ||
        (weatherCode >= 61 && weatherCode <= 67) ||
        (weatherCode >= 80 && weatherCode <= 82)) {
        return QStringLiteral("Pluie");
    }
    if (weatherCode >= 71 && weatherCode <= 77) return QStringLiteral("Neige");
    if (weatherCode >= 95 && weatherCode <= 99) return QStringLiteral("Orage");
    return QStringLiteral("Nuageux");
}

static QString weatherIconEmoji(int weatherCode)
{
    if (weatherCode == 0) return QStringLiteral("☀️");
    if (weatherCode == 1 || weatherCode == 2) return QStringLiteral("⛅");
    if (weatherCode == 3) return QStringLiteral("☁️");
    if (weatherCode == 45 || weatherCode == 48) return QStringLiteral("🌫️");
    if ((weatherCode >= 51 && weatherCode <= 57) ||
        (weatherCode >= 61 && weatherCode <= 67) ||
        (weatherCode >= 80 && weatherCode <= 82)) {
        return QStringLiteral("🌧️");
    }
    if (weatherCode >= 71 && weatherCode <= 77) return QStringLiteral("❄️");
    if (weatherCode >= 95 && weatherCode <= 99) return QStringLiteral("⛈️");
    return QStringLiteral("☁️");
}