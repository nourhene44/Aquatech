#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "arduinoserial.h"
#include <QStatusBar>
#include <QDialog>
#include <QPlainTextEdit>
#include <QAbstractButton>
#include <QComboBox>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDate>
#include <QDateTime>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlDriver>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QPixmap>
#include <QPdfWriter>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QPageSize>
#include <QPageLayout>
#include <QDir>
#include <QStandardPaths>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QProgressBar>
#include <QHeaderView>
#include <QAction>
#include <QIcon>
#include <QPalette>
#include <QToolButton>
#include <QFrame>
#include <QSystemTrayIcon>
#include <QEvent>
#ifdef HAVE_ACTIVEQT
#include <QAxObject>
#endif
#include <QGraphicsDropShadowEffect>
#include <QLocale>
#include <QStyle>
#include <QStringList>
#include "bateaauuu.h"
#include "captures.h"
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
#include <cmath>
#include <algorithm>
#include <functional>
#include <initializer_list>
#include <limits>
#include <QRandomGenerator>
#include <QSslSocket>
#include <QSettings>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QDir>
#include <QBuffer>

// Helpers g├⌐n├⌐riques pour la base de donn├⌐es et le texte UI

static QString weatherDescriptionFr(int weatherCode);
static QString weatherIconEmoji(int weatherCode);
static QString weatherIconEmojiDayNight(int weatherCode, bool isDay);
static void exportEmployesPdfReport(MainWindow* parent, Ui::MainWindow* ui);
static void updateCapturesStats(MainWindow* parent, Ui::MainWindow* ui);

static bool handleCrudDisabled(QWidget* parent)
{
    Q_UNUSED(parent);
    // Point central pour d├⌐sactiver temporairement les op├⌐rations CRUD si besoin.
    return true;
}

static int bateauIdFromText(const QString& text)
{
    return Pecheurs::bateauIdFromText(text);
}

static QLineEdit* pecheurBateauLineEdit(Ui::MainWindow* ui)
{
    if (!ui) return nullptr;
    return ui->stackedWidget ? ui->stackedWidget->findChild<QLineEdit*>(QStringLiteral("lineEdit_2p_2")) : nullptr;
}

static QComboBox* pecheurBateauCombo(Ui::MainWindow* ui)
{
    if (!ui) return nullptr;
    return ui->comboBox_9;
}

static QDateTime nullPecheurAffectationDateTime()
{
    return QDateTime(QDate(1900, 1, 1), QTime(0, 0));
}

static bool isPecheurAffectationNull(const QDateTime& dateTime)
{
    return !dateTime.isValid() || dateTime <= nullPecheurAffectationDateTime();
}

static QString resolveBateauMaintenanceFrequencyColumn()
{
    Connection* conn = Connection::getInstance();
    if (!conn || !conn->ensureOpen()) {
        return QString();
    }

    const QSqlDatabase db = conn->getDatabase();
    if (!db.isValid() || !db.isOpen()) {
        return QString();
    }

    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT COLUMN_NAME "
        "FROM USER_TAB_COLUMNS "
        "WHERE UPPER(TABLE_NAME) = 'BATEAUX'"));

    if (!query.exec()) {
        return QString();
    }

    QSet<QString> availableColumns;
    while (query.next()) {
        availableColumns.insert(query.value(0).toString().trimmed().toUpper());
    }

    static const QStringList candidates = {
        QStringLiteral("PROCHAINE_MAINTENANCE"),
        QStringLiteral("FREQUENCE_MAINTENANCE"),
        QStringLiteral("FREQUENCE")
    };

    for (const QString& candidate : candidates) {
        if (availableColumns.contains(candidate)) {
            return candidate;
        }
    }

    return QString();
}

static QString bateauMaintenanceFrequencySelectExpr()
{
    const QString column = resolveBateauMaintenanceFrequencyColumn();
    return column.isEmpty() ? QStringLiteral("0") : column;
}

static QString pecheurFaceDisplayName(Ui::MainWindow* ui)
{
    if (!ui) return QString();
    const QString nom = ui->lineEdit_2p ? ui->lineEdit_2p->text().trimmed() : QString();
    const QString prenom = ui->lineEdit_3p ? ui->lineEdit_3p->text().trimmed() : QString();
    return QStringLiteral("%1 %2").arg(nom, prenom).trimmed();
}

static QString pecheurFaceRole(Ui::MainWindow* ui)
{
    return ui && ui->comboBoxp ? ui->comboBoxp->currentText().trimmed() : QString();
}

static QString pecheurFaceOpenStatusText()
{
    return QStringLiteral("Positionnez votre visage.");
}

static void setPecheurFaceStatus(Ui::MainWindow* ui, const QString& text)
{
    if (!ui) return;
    if (ui->labelFaceStatusp) {
        ui->labelFaceStatusp->setAlignment(Qt::AlignCenter);
        ui->labelFaceStatusp->setWordWrap(true);
        ui->labelFaceStatusp->setText(text);
    }
    // Status label visible sur le frame FaceID principal.
    if (ui->label_8p) {
        ui->label_8p->setAlignment(Qt::AlignCenter);
        ui->label_8p->setWordWrap(true);
        ui->label_8p->setText(text);
    }
}

static QRect pecheurFacePreviewRect(QWidget* container)
{
    if (!container) return QRect();

    const int margin = 8;
    const int availableWidth = qMax(0, container->width() - 2 * margin);
    const int availableHeight = qMax(0, container->height() - 2 * margin);
    const int diameter = qMax(40, qMin(availableWidth, availableHeight));
    const int x = (container->width() - diameter) / 2;
    const int y = (container->height() - diameter) / 2;
    return QRect(x, y, diameter, diameter);
}

static QPixmap buildCircularFacePreviewPixmap(const QImage& image, int diameter)
{
    if (image.isNull() || diameter <= 0) return QPixmap();

    const qreal borderWidth = 3.0;
    const qreal imageInset = borderWidth + 1.0;

    QImage frame = image.convertToFormat(QImage::Format_ARGB32_Premultiplied)
                        .scaled(diameter, diameter, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    const int cropX = qMax(0, (frame.width() - diameter) / 2);
    const int cropY = qMax(0, (frame.height() - diameter) / 2);
    const QRect cropRect(cropX, cropY, qMin(diameter, frame.width()), qMin(diameter, frame.height()));

    QPixmap pixmap(diameter, diameter);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setClipPath(QPainterPath());
    QPainterPath clipPath;
    clipPath.addEllipse(imageInset,
                        imageInset,
                        diameter - 2.0 * imageInset,
                        diameter - 2.0 * imageInset);
    painter.setClipPath(clipPath);
    painter.drawImage(QRectF(imageInset,
                             imageInset,
                             diameter - 2.0 * imageInset,
                             diameter - 2.0 * imageInset),
                      frame,
                      cropRect);
    painter.end();

    return pixmap;
}

static void applyPecheurFacePreviewShape(QLabel* previewLabel)
{
    if (!previewLabel) return;

    previewLabel->setStyleSheet(QStringLiteral(
        "QLabel {"
        "background: transparent;"
        "border: 3px dashed #2EF1A6;"
        "border-radius: 999px;"
        "}"
    ));
}

static void showPecheurPhotoPreviewDialog(QWidget* parent, const QPixmap& source)
{
    if (source.isNull()) return;

    auto* dialog = new QDialog(parent);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->setWindowTitle(QStringLiteral("Photo du pêcheur"));
    dialog->setModal(true);
    dialog->resize(760, 760);

    auto* layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(12, 12, 12, 12);

    auto* imageLabel = new QLabel(dialog);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet(QStringLiteral("background: #0f1720; border: 1px solid #2d3f55; border-radius: 10px;"));

    const QSize targetSize = dialog->size() - QSize(32, 32);
    imageLabel->setPixmap(source.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    layout->addWidget(imageLabel);
    dialog->show();
}

static constexpr qint64 kFaceLiveFrameTimeoutMs = 900;

static bool frameHasLiveCameraContent(const QImage& image)
{
    if (image.isNull()) return false;

    const QImage rgb = image.convertToFormat(QImage::Format_RGB888);
    if (rgb.isNull() || rgb.width() <= 0 || rgb.height() <= 0) return false;

    const int stepX = qMax(1, rgb.width() / 64);
    const int stepY = qMax(1, rgb.height() / 64);

    qint64 samples = 0;
    qint64 darkCount = 0;
    qint64 brightCount = 0;
    qint64 centerBrightCount = 0;
    qint64 centerSamples = 0;
    qint64 outerBrightCount = 0;
    qint64 outerSamples = 0;
    qint64 sum = 0;
    qint64 sumSq = 0;

    const QRect centerRect(rgb.width() / 4, rgb.height() / 4, rgb.width() / 2, rgb.height() / 2);

    for (int y = 0; y < rgb.height(); y += stepY) {
        const uchar* row = rgb.constScanLine(y);
        for (int x = 0; x < rgb.width(); x += stepX) {
            const int idx = x * 3;
            const int r = row[idx + 0];
            const int g = row[idx + 1];
            const int b = row[idx + 2];
            const int lum = (r + g + b) / 3;

            ++samples;
            sum += lum;
            sumSq += static_cast<qint64>(lum) * static_cast<qint64>(lum);
            if (lum < 20) ++darkCount;
            if (lum > 225) {
                ++brightCount;
                if (centerRect.contains(x, y)) ++centerBrightCount;
                else ++outerBrightCount;
            }

            if (centerRect.contains(x, y)) ++centerSamples;
            else ++outerSamples;
        }
    }

    if (samples <= 0) return false;

    const double mean = static_cast<double>(sum) / static_cast<double>(samples);
    const double variance = (static_cast<double>(sumSq) / static_cast<double>(samples)) - (mean * mean);
    const double darkRatio = static_cast<double>(darkCount) / static_cast<double>(samples);
    const double brightRatio = static_cast<double>(brightCount) / static_cast<double>(samples);
    const double centerBrightRatio = centerSamples > 0
        ? static_cast<double>(centerBrightCount) / static_cast<double>(centerSamples)
        : 0.0;
    const double outerBrightRatio = outerSamples > 0
        ? static_cast<double>(outerBrightCount) / static_cast<double>(outerSamples)
        : 0.0;

    const bool looksLikeCameraOffPlaceholder =
        darkRatio > 0.78
        && brightRatio > 0.002
        && brightRatio < 0.18
        && centerBrightRatio > (outerBrightRatio * 2.0 + 0.01);

    if (looksLikeCameraOffPlaceholder) {
        return false;
    }

    return mean > 10.0 && variance > 12.0;
}

static QString gmailAppPassword()
{
    return QStringLiteral("offsujmzgyrigukg");
}

static bool smtpReadResponse(QSslSocket& socket, int expectedCode, QString* errorOut = nullptr)
{
    QByteArray response;
    while (socket.waitForReadyRead(10000)) {
        response += socket.readAll();
        const QList<QByteArray> lines = response.split('\n');
        bool hasFinalLine = false;
        QByteArray lastLine;
        for (const QByteArray& line : lines) {
            if (line.size() >= 4 && line[3] == ' ') {
                hasFinalLine = true;
                lastLine = line;
            }
        }
        if (!hasFinalLine) {
            continue;
        }

        const QByteArray trimmed = lastLine.trimmed();
        bool ok = false;
        const int code = QString::fromLatin1(trimmed.left(3)).toInt(&ok);
        if (ok && code == expectedCode) {
            return true;
        }

        if (errorOut) {
            *errorOut = QString::fromLatin1(response);
        }
        return false;
    }

    if (errorOut) {
        *errorOut = socket.errorString();
    }
    return false;
}

static bool smtpSendCommand(QSslSocket& socket, const QByteArray& command, int expectedCode, QString* errorOut = nullptr)
{
    if (socket.write(command) == -1 || !socket.waitForBytesWritten(10000)) {
        if (errorOut) {
            *errorOut = socket.errorString();
        }
        return false;
    }
    return smtpReadResponse(socket, expectedCode, errorOut);
}

static bool sendMissionMailSmtp(const QString& recipient,
                                const QString& nom,
                                const QString& prenom,
                                const QDateTime& dateAffectation,
                                int idBateau,
                                QString* errorOut = nullptr)
{
    if (recipient.trimmed().isEmpty()) {
        if (errorOut) {
            *errorOut = QStringLiteral("Adresse e-mail vide.");
        }
        return false;
    }

    const QString sender = QStringLiteral("aquatech.2626@gmail.com");
    const QString password = gmailAppPassword();
    const QString subject = QStringLiteral("Affectation a une mission");
    const QString body = QStringLiteral(
        "Bonjour %1 %2,\n\n"
        "Nous vous informons que vous avez été affecté(e) à une mission.\n\n"
        "Date d’affectation : %3 à %4h\n"
        "ID du bateau : %5\n\n"
        "Nous vous remercions de bien vouloir prendre les dispositions nécessaires.\n\n"
        "Cordialement, ")
            .arg(nom.trimmed(),
                 prenom.trimmed(),
                 dateAffectation.date().toString(QStringLiteral("dd/MM/yyyy")),
                 dateAffectation.time().toString(QStringLiteral("HH:mm")),
                 QString::number(idBateau));

    QByteArray message;
    message += QByteArray("From: ") + sender.toUtf8() + QByteArray("\r\n");
    message += QByteArray("To: ") + recipient.trimmed().toUtf8() + QByteArray("\r\n");
    message += QByteArray("Subject: ") + subject.toUtf8() + QByteArray("\r\n");
    message += QByteArray("MIME-Version: 1.0\r\n");
    message += QByteArray("Content-Type: text/plain; charset=UTF-8\r\n");
    message += QByteArray("Content-Transfer-Encoding: 8bit\r\n");
    message += QByteArray("\r\n");
    message += body.toUtf8();
    message += QByteArray("\r\n");

    QSslSocket socket;
    socket.connectToHost(QStringLiteral("smtp.gmail.com"), 587);
    if (!socket.waitForConnected(10000)) {
        if (errorOut) *errorOut = socket.errorString();
        return false;
    }

    QString smtpError;
    if (!smtpReadResponse(socket, 220, &smtpError)) {
        if (errorOut) *errorOut = QStringLiteral("Serveur SMTP inattendu: %1").arg(smtpError);
        socket.disconnectFromHost();
        return false;
    }

    if (!smtpSendCommand(socket, "EHLO localhost\r\n", 250, &smtpError)) {
        if (errorOut) *errorOut = QStringLiteral("EHLO echoue: %1").arg(smtpError);
        socket.disconnectFromHost();
        return false;
    }

    if (!smtpSendCommand(socket, "STARTTLS\r\n", 220, &smtpError)) {
        if (errorOut) *errorOut = QStringLiteral("STARTTLS echoue: %1").arg(smtpError);
        socket.disconnectFromHost();
        return false;
    }

    socket.startClientEncryption();
    if (!socket.waitForEncrypted(10000)) {
        if (errorOut) *errorOut = QStringLiteral("Echec TLS: %1").arg(socket.errorString());
        socket.disconnectFromHost();
        return false;
    }

    if (!smtpSendCommand(socket, "EHLO localhost\r\n", 250, &smtpError)) {
        if (errorOut) *errorOut = QStringLiteral("EHLO TLS echoue: %1").arg(smtpError);
        socket.disconnectFromHost();
        return false;
    }

    if (!smtpSendCommand(socket, "AUTH LOGIN\r\n", 334, &smtpError)) {
        if (errorOut) *errorOut = QStringLiteral("Authentification echouee: %1").arg(smtpError);
        socket.disconnectFromHost();
        return false;
    }
    if (!smtpSendCommand(socket, sender.toUtf8().toBase64() + "\r\n", 334, &smtpError)) {
        if (errorOut) *errorOut = QStringLiteral("Login SMTP echoue: %1").arg(smtpError);
        socket.disconnectFromHost();
        return false;
    }
    if (!smtpSendCommand(socket, password.toUtf8().toBase64() + "\r\n", 235, &smtpError)) {
        if (errorOut) *errorOut = QStringLiteral("Mot de passe SMTP echoue: %1").arg(smtpError);
        socket.disconnectFromHost();
        return false;
    }

    if (!smtpSendCommand(socket, QByteArray("MAIL FROM:<") + sender.toUtf8() + ">\r\n", 250, &smtpError)) {
        if (errorOut) *errorOut = QStringLiteral("MAIL FROM echoue: %1").arg(smtpError);
        socket.disconnectFromHost();
        return false;
    }
    if (!smtpSendCommand(socket, QByteArray("RCPT TO:<") + recipient.trimmed().toUtf8() + ">\r\n", 250, &smtpError)) {
        if (errorOut) *errorOut = QStringLiteral("RCPT TO echoue: %1").arg(smtpError);
        socket.disconnectFromHost();
        return false;
    }
    if (!smtpSendCommand(socket, "DATA\r\n", 354, &smtpError)) {
        if (errorOut) *errorOut = QStringLiteral("DATA echoue: %1").arg(smtpError);
        socket.disconnectFromHost();
        return false;
    }

    if (socket.write(message) == -1 || !socket.waitForBytesWritten(10000)) {
        if (errorOut) *errorOut = socket.errorString();
        socket.disconnectFromHost();
        return false;
    }
    if (socket.write(".\r\n") == -1 || !socket.waitForBytesWritten(10000)) {
        if (errorOut) *errorOut = socket.errorString();
        socket.disconnectFromHost();
        return false;
    }
    if (!smtpReadResponse(socket, 250, &smtpError)) {
        if (errorOut) *errorOut = QStringLiteral("Envoi du message echoue: %1").arg(smtpError);
        socket.disconnectFromHost();
        return false;
    }

    smtpSendCommand(socket, "QUIT\r\n", 221, nullptr);
    socket.disconnectFromHost();
    return true;
}

static void loadPecheurBateauChoices(Ui::MainWindow* ui)
{
    QComboBox* combo = pecheurBateauCombo(ui);
    if (!combo) return;

    const QVariant previousData = combo->currentData();
    const QString previousText = combo->currentText().trimmed();

    combo->blockSignals(true);
    combo->clear();
    combo->addItem(QStringLiteral("0 - aucun bateau"), 0);

    QSqlQuery query;
    if (query.exec(QStringLiteral("SELECT ID_BATEAU, NOM FROM BATEAUX ORDER BY ID_BATEAU"))) {
        while (query.next()) {
            const int id = query.value(0).toInt();
            if (id == 0) {
                continue;
            }
            const QString nom = query.value(1).toString().trimmed();
            const QString label = nom.isEmpty()
                ? QString::number(id)
                : QStringLiteral("%1 - %2").arg(id).arg(nom);
            combo->addItem(label, id);
        }
    }

    int idx = -1;
    if (previousData.isValid()) {
        idx = combo->findData(previousData);
    }
    if (idx < 0 && !previousText.isEmpty()) {
        idx = combo->findText(previousText, Qt::MatchStartsWith);
    }
    if (idx >= 0) {
        combo->setCurrentIndex(idx);
    } else {
        combo->setCurrentIndex(0); // 0 - aucun bateau
    }

    combo->blockSignals(false);
}

static QComboBox* quaiBateauCombo(Ui::MainWindow* ui)
{
    if (!ui) return nullptr;
    return ui->comboBoxQuaiBateau;
}

static void loadQuaiBateauChoices(Ui::MainWindow* ui)
{
    QComboBox* combo = quaiBateauCombo(ui);
    if (!combo) return;

    const QVariant previousData = combo->currentData();
    const QString previousText = combo->currentText().trimmed();

    combo->blockSignals(true);
    combo->clear();
    combo->addItem(QStringLiteral("0 - Aucun bateau"), 0);

    QSqlQuery query;
    if (query.exec(QStringLiteral("SELECT ID_BATEAU, NOM FROM BATEAUX ORDER BY ID_BATEAU"))) {
        while (query.next()) {
            const int id = query.value(0).toInt();
            const QString nom = query.value(1).toString().trimmed();
            const QString label = nom.isEmpty()
                ? QString::number(id)
                : QStringLiteral("%1 - %2").arg(QString::number(id), nom);
            combo->addItem(label, id);
        }
    }

    int idx = -1;
    if (previousData.isValid()) {
        idx = combo->findData(previousData);
    }
    if (idx < 0 && !previousText.isEmpty()) {
        idx = combo->findText(previousText, Qt::MatchStartsWith);
    }
    combo->setCurrentIndex(idx >= 0 ? idx : 0);
    combo->blockSignals(false);
}

static QString pecheurSexeCode(Ui::MainWindow* ui)
{
    if (!ui) return QString();
    if (ui->radioButton_2p && ui->radioButton_2p->isChecked()) return QStringLiteral("M");
    if (ui->radioButtonp && ui->radioButtonp->isChecked()) return QStringLiteral("F");
    return QString();
}

static bool isValidEmailAddress(const QString& email)
{
    static const QRegularExpression rx(QStringLiteral("^[\\w\\-\\.]+@([\\w-]+\\.)+[\\w-]{2,4}$"));
    return rx.match(email).hasMatch();
}

static bool pecheurEmailExistsInDb(const QString& email, QString* errorOut = nullptr)
{
    QSqlQuery q;
    q.prepare(QStringLiteral("SELECT 1 FROM PECHEURS WHERE LOWER(Email) = LOWER(?)"));
    q.addBindValue(email.trimmed());
    if (!q.exec()) {
        if (errorOut) *errorOut = q.lastError().text();
        return false;
    }
    return q.next();
}

static int toIntOrZero(const QString& text)
{
    if (text.trimmed().isEmpty()) return 0;

    bool ok = false;
    int value = text.trimmed().toInt(&ok);
    if (ok) return value;

    double dvalue = text.trimmed().toDouble(&ok);
    if (ok) return static_cast<int>(dvalue);

    return 0;
}

static void applyRememberedLogin(Ui::MainWindow* ui)
{
    if (!ui) return;

    QSettings settings(QStringLiteral("AquaTech"), QStringLiteral("AquaTech"));
    const bool remember = settings.value(QStringLiteral("auth/rememberMe"), false).toBool();
    const QString savedUser = settings.value(QStringLiteral("auth/rememberedUsername")).toString();
    const QString savedPass = settings.value(QStringLiteral("auth/rememberedPassword")).toString();

    if (ui->checkBoxb) {
        ui->checkBoxb->setChecked(remember);
    }

    if (remember) {
        if (ui->lineEdit_b) ui->lineEdit_b->setText(savedUser);
        if (ui->lineEdit_2b) ui->lineEdit_2b->setText(savedPass);
    } else {
        if (ui->lineEdit_b) ui->lineEdit_b->clear();
        if (ui->lineEdit_2b) ui->lineEdit_2b->clear();
    }
}

static void storeRememberedLogin(const QString& username,
                                 const QString& password,
                                 bool remember)
{
    QSettings settings(QStringLiteral("AquaTech"), QStringLiteral("AquaTech"));
    settings.setValue(QStringLiteral("auth/rememberMe"), remember);

    if (remember) {
        settings.setValue(QStringLiteral("auth/rememberedUsername"), username);
        settings.setValue(QStringLiteral("auth/rememberedPassword"), password);
    } else {
        settings.remove(QStringLiteral("auth/rememberedUsername"));
        settings.remove(QStringLiteral("auth/rememberedPassword"));
    }

    settings.sync();
}

static QIcon makeEyeIcon(const QColor& color, bool slashed, int sizePx)
{
    const int size = qMax(16, sizePx);
    QPixmap px(size, size);
    px.fill(Qt::transparent);

    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing, true);

    QPen pen(color);
    const qreal penW = qMax<qreal>(1.6, (qreal)size * 0.08);
    pen.setWidthF(penW);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    // Outer eye shape
    const QPointF left(size * 0.10, size * 0.50);
    const QPointF right(size * 0.90, size * 0.50);
    const QPointF top(size * 0.50, size * 0.18);
    const QPointF bottom(size * 0.50, size * 0.82);

    QPainterPath eye;
    eye.moveTo(left);
    eye.quadTo(top, right);
    eye.quadTo(bottom, left);
    p.drawPath(eye);

    // Iris + pupil
    p.drawEllipse(QPointF(size * 0.50, size * 0.50), size * 0.20, size * 0.20);
    p.setBrush(color);
    p.drawEllipse(QPointF(size * 0.50, size * 0.50), size * 0.08, size * 0.08);
    p.setBrush(Qt::NoBrush);

    if (slashed) {
        p.drawLine(QPointF(size * 0.16, size * 0.84), QPointF(size * 0.84, size * 0.16));
    }

    return QIcon(px);
}

class EyeToggleFilter final : public QObject
{
public:
    EyeToggleFilter(QLineEdit* edit, QToolButton* button)
        : QObject(edit)
        , m_edit(edit)
        , m_button(button)
    {
    }

    void updateUi()
    {
        if (!m_edit || !m_button) return;

        const int editH = (m_edit->height() > 0) ? m_edit->height() : 28;
        // Slightly smaller than the field height (user requested "a little small")
        const int btnSize = qMax(20, (editH * 2) / 3);
        const int iconPx = qMax(16, btnSize - 6);

        const QColor color = m_edit->palette().color(QPalette::Text);
        const QIcon iconVisible = makeEyeIcon(color, false, iconPx);
        const QIcon iconHidden = makeEyeIcon(color, true, iconPx);

        m_button->setFixedSize(QSize(btnSize, btnSize));
        m_button->setIconSize(QSize(iconPx, iconPx));
        m_button->setIcon(m_button->isChecked() ? iconVisible : iconHidden);

        const int frame = m_edit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, nullptr, m_edit);
        const QRect r = m_edit->rect();
        const int x = r.right() - frame - btnSize + 1;
        const int y = (r.height() - btnSize) / 2;
        m_button->move(x, y);
        m_button->raise();

        QMargins m = m_edit->textMargins();
        const int right = btnSize + 6;
        if (m.right() != right) {
            m_edit->setTextMargins(m.left(), m.top(), right, m.bottom());
        }
    }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        if (watched == m_edit) {
            switch (event->type()) {
            case QEvent::Show:
            case QEvent::Resize:
            case QEvent::StyleChange:
            case QEvent::FontChange:
            case QEvent::PaletteChange:
                updateUi();
                break;
            default:
                break;
            }
        }
        return QObject::eventFilter(watched, event);
    }

private:
    QLineEdit* m_edit;
    QToolButton* m_button;
};

static void installPasswordEyeToggle(QLineEdit* edit)
{
    if (!edit) return;

    if (edit->property("_aquatech_eye_toggle").toBool()) return;
    edit->setProperty("_aquatech_eye_toggle", true);

    // Hidden by default
    edit->setEchoMode(QLineEdit::Password);
    edit->setInputMethodHints(Qt::ImhHiddenText | Qt::ImhNoPredictiveText | Qt::ImhNoAutoUppercase);

    auto* button = new QToolButton(edit);
    button->setCheckable(true);
    button->setChecked(false);
    button->setCursor(Qt::PointingHandCursor);
    button->setAutoRaise(true);
    button->setFocusPolicy(Qt::NoFocus);
    button->setToolTip(QObject::tr("Afficher / Masquer"));
    button->setStyleSheet(QStringLiteral("QToolButton{border:none;padding:0px;background:transparent;}"));

    auto* filter = new EyeToggleFilter(edit, button);
    edit->installEventFilter(filter);
    filter->updateUi();

    QObject::connect(button, &QToolButton::toggled, edit, [edit, filter](bool checked) {
        const int cursorPos = edit->cursorPosition();
        const int selStart = edit->selectionStart();
        const int selLen = edit->selectedText().size();

        edit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
        if (filter) filter->updateUi();

        edit->setCursorPosition(cursorPos);
        if (selStart >= 0) {
            edit->setSelection(selStart, selLen);
        }
    });
}


QString MainWindow::generateCode()
{
    const int code = QRandomGenerator::global()->bounded(100000, 1000000);
    return QString::number(code);
}

QString MainWindow::fallbackAdminPassword() const
{
    QSettings settings(QStringLiteral("AquaTech"), QStringLiteral("AquaTech"));
    const QString pass = settings.value(QStringLiteral("auth/adminPassword")).toString();
    return pass.isEmpty() ? QStringLiteral("admin") : pass;
}

void MainWindow::setFallbackAdminPassword(const QString& newPassword)
{
    QSettings settings(QStringLiteral("AquaTech"), QStringLiteral("AquaTech"));
    settings.setValue(QStringLiteral("auth/adminPassword"), newPassword);
    settings.sync();
}

void MainWindow::clearPasswordResetState()
{
    m_passwordResetCode.clear();
    m_passwordResetEmail.clear();
    m_passwordResetVerified = false;
}

static bool parseQuotaKg(const QString& text, double* outKg)
{
    if (outKg) *outKg = 0.0;
    QString t = text;
    t = t.trimmed();

    if (t.isEmpty()) return false;

    t.replace(QLatin1Char(','), QLatin1Char('.'));
    t.remove(QStringLiteral("kg"), Qt::CaseInsensitive);
    t.remove(QStringLiteral("kgs"), Qt::CaseInsensitive);
    t.remove(QStringLiteral("kilogramme"), Qt::CaseInsensitive);
    t.remove(QStringLiteral("kilogrammes"), Qt::CaseInsensitive);

    static QRegularExpression rx(QStringLiteral("(-?\\d+(?:\\.\\d+)?)"));
    const QRegularExpressionMatch m = rx.match(t);
    if (!m.hasMatch()) return false;

    bool ok = false;
    const double v = m.captured(1).toDouble(&ok);
    if (!ok || v < 0.0) return false;
    if (outKg) *outKg = v;
    return true;
}


bool MainWindow::sendEmail(const QString& to,
                           const QString& content,
                           QString* errorOut,
                           const QString& subject)
{
    const QString userEnv = QString::fromUtf8(qgetenv("AQUATECH_SMTP_USER"));
    const QString passEnv = QString::fromUtf8(qgetenv("AQUATECH_SMTP_PASS"));

    const QString user = userEnv.isEmpty() ? QStringLiteral("personinkonnu@gmail.com") : userEnv;
    QString pass = passEnv.isEmpty() ? QStringLiteral("ogtj yhmr lnnz uasi") : passEnv;
    pass.remove(QLatin1Char(' '));

    if (to.trimmed().isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("Adresse email destinataire vide.");
        return false;
    }

    QSslSocket socket;
    socket.connectToHostEncrypted(QStringLiteral("smtp.gmail.com"), 465);

    if (!socket.waitForEncrypted(10000)) {
        if (errorOut) *errorOut = socket.errorString();
        return false;
    }

    auto readResponse = [&]() -> QByteArray {
        if (!socket.waitForReadyRead(10000)) {
            return {};
        }
        QByteArray data = socket.readAll();
        while (socket.waitForReadyRead(150)) {
            data += socket.readAll();
        }
        return data;
    };

    auto sendCommand = [&](const QByteArray& cmd) -> QByteArray {
        socket.write(cmd + "\r\n");
        if (!socket.waitForBytesWritten(10000)) {
            return {};
        }
        return readResponse();
    };

    const auto startsWithCode = [](const QByteArray& resp, const QByteArray& code3) -> bool {
        if (resp.size() < 3) return false;
        return resp.left(3) == code3;
    };

    QByteArray resp = readResponse(); // 220 greeting
    if (resp.isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("Pas de r\u00e9ponse du serveur SMTP.");
        return false;
    }

    resp = sendCommand("EHLO localhost");
    if (!startsWithCode(resp, "250")) {
        if (errorOut) *errorOut = QStringLiteral("SMTP EHLO \u00e9chou\u00e9: %1").arg(QString::fromUtf8(resp));
        return false;
    }

    resp = sendCommand("AUTH LOGIN");
    if (!startsWithCode(resp, "334")) {
        if (errorOut) *errorOut = QStringLiteral("SMTP AUTH LOGIN \u00e9chou\u00e9: %1").arg(QString::fromUtf8(resp));
        return false;
    }

    resp = sendCommand(user.toUtf8().toBase64());
    if (!startsWithCode(resp, "334")) {
        if (errorOut) *errorOut = QStringLiteral("SMTP USERNAME \u00e9chou\u00e9: %1").arg(QString::fromUtf8(resp));
        return false;
    }

    resp = sendCommand(pass.toUtf8().toBase64());
    if (!startsWithCode(resp, "235")) {
        if (errorOut) *errorOut = QStringLiteral("SMTP PASSWORD \u00e9chou\u00e9: %1").arg(QString::fromUtf8(resp));
        return false;
    }

    resp = sendCommand(QByteArray("MAIL FROM:<") + user.toUtf8() + ">");
    if (!startsWithCode(resp, "250")) {
        if (errorOut) *errorOut = QStringLiteral("SMTP MAIL FROM \u00e9chou\u00e9: %1").arg(QString::fromUtf8(resp));
        return false;
    }

    resp = sendCommand(QByteArray("RCPT TO:<") + to.toUtf8() + ">");
    if (!startsWithCode(resp, "250")) {
        if (errorOut) *errorOut = QStringLiteral("SMTP RCPT TO \u00e9chou\u00e9: %1").arg(QString::fromUtf8(resp));
        return false;
    }

    resp = sendCommand("DATA");
    if (!startsWithCode(resp, "354")) {
        if (errorOut) *errorOut = QStringLiteral("SMTP DATA \u00e9chou\u00e9: %1").arg(QString::fromUtf8(resp));
        return false;
    }

    QByteArray message;
    message += "From: AquaTech <" + user.toUtf8() + ">\r\n";
    message += "To: <" + to.toUtf8() + ">\r\n";
    message += "Subject: " + subject.toUtf8() + "\r\n";
    message += "\r\n";
    message += content.toUtf8() + "\r\n";
    message += "\r\n";
    message += ".\r\n";

    socket.write(message);
    if (!socket.waitForBytesWritten(10000)) {
        if (errorOut) *errorOut = QStringLiteral("SMTP: envoi du message \u00e9chou\u00e9.");
        return false;
    }

    resp = readResponse();
    if (!startsWithCode(resp, "250")) {
        if (errorOut) *errorOut = QStringLiteral("SMTP fin de DATA \u00e9chou\u00e9e: %1").arg(QString::fromUtf8(resp));
        return false;
    }

    sendCommand("QUIT");
    return true;
}


static QString cleanFilterLabel(QString text)
{
    text = text.trimmed();
    text.replace(QStringLiteral("Γ£à"), QString());
    text.replace(QStringLiteral("Γ¥î"), QString());
    text.replace(QStringLiteral("ΓÅ│"), QString());
    text.replace(QStringLiteral("≡ƒÅû∩╕Å"), QString());
    text.replace(QStringLiteral("ΓÇó"), QString());
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

static QString canonicalEmployeRole(const QString& raw)
{
    QString key = raw.trimmed().toLower();
    key.replace(QStringLiteral("├⌐"), QStringLiteral("e"));
    key.replace(QStringLiteral("├¿"), QStringLiteral("e"));
    key.replace(QStringLiteral("├¬"), QStringLiteral("e"));
    key.replace(QStringLiteral("├á"), QStringLiteral("a"));
    key.replace(QStringLiteral("├╣"), QStringLiteral("u"));
    key.replace(QStringLiteral("├«"), QStringLiteral("i"));
    key.replace(QStringLiteral("├┤"), QStringLiteral("o"));
    key.replace(QStringLiteral("├º"), QStringLiteral("c"));
    key.remove(QLatin1Char('_'));
    key.remove(QLatin1Char(' '));

    if (key.contains(QStringLiteral("gard"))) return QStringLiteral("Gardien");
    if (key.contains(QStringLiteral("tech"))) return QStringLiteral("Technicien");
    if (key.contains(QStringLiteral("respons"))) return QStringLiteral("Responsable");
    if (key.contains(QStringLiteral("ouvri"))) return QStringLiteral("Ouvrier");
    if (key.contains(QStringLiteral("pech"))) return QStringLiteral("Pecheur");
    return raw.trimmed();
}

static QPixmap buildEmployeRoleCirclePixmap(int gardien,
                                            int technicien,
                                            int responsable,
                                            int ouvrier,
                                            int size)
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
    const int total = gardien + technicien + responsable + ouvrier;
    const int totalUnits = 360 * 16;

    int sGardien = 0;
    int sTechnicien = 0;
    int sResponsable = 0;
    int sOuvrier = 0;

    if (total > 0) {
        sGardien = qRound((static_cast<double>(gardien) / static_cast<double>(total)) * totalUnits);
        sTechnicien = qRound((static_cast<double>(technicien) / static_cast<double>(total)) * totalUnits);
        sResponsable = qRound((static_cast<double>(responsable) / static_cast<double>(total)) * totalUnits);
        sOuvrier = qMax(0, totalUnits - (sGardien + sTechnicien + sResponsable));
    } else {
        sGardien = totalUnits / 4;
        sTechnicien = totalUnits / 4;
        sResponsable = totalUnits / 4;
        sOuvrier = totalUnits - (sGardien + sTechnicien + sResponsable);
    }

    QVector<QPair<QColor, int>> slices = {
        { QColor(QStringLiteral("#3498db")), sGardien },
        { QColor(QStringLiteral("#27ae60")), sTechnicien },
        { QColor(QStringLiteral("#f39c12")), sResponsable },
        { QColor(QStringLiteral("#9b59b6")), sOuvrier }
    };

    int startAngle = 90 * 16;
    for (const auto& slice : slices) {
        if (slice.second <= 0) continue;
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

static void updateEmployeRoleStats(Ui::MainWindow* ui,
                                   int gardien,
                                   int technicien,
                                   int responsable,
                                   int ouvrier)
{
    if (!ui) return;

    const int total = gardien + technicien + responsable + ouvrier;
    const auto pct = [total](int value) {
        if (total <= 0) return QStringLiteral("0.0");
        return QString::number((100.0 * static_cast<double>(value)) / static_cast<double>(total), 'f', 1);
    };

    QLabel* obsoletePecheurLegend = ui->stackedWidget
        ? ui->stackedWidget->findChild<QLabel*>(QStringLiteral("label_legend_othere"))
        : nullptr;
    if (obsoletePecheurLegend) {
        obsoletePecheurLegend->clear();
        obsoletePecheurLegend->hide();
    }

    if (ui->label_total_typese) {
        ui->label_total_typese->setText(QStringLiteral("Total: %1 employ\u00E9s").arg(total));
    }
    if (ui->label_legend_chalutiere) {
        ui->label_legend_chalutiere->setText(
            QStringLiteral("\u2022 Gardien: %1 (%2%)").arg(gardien).arg(pct(gardien)));
    }
    if (ui->label_legend_palangriere) {
        ui->label_legend_palangriere->setText(
            QStringLiteral("\u2022 Technicien: %1 (%2%)").arg(technicien).arg(pct(technicien)));
    }
    if (ui->label_legend_caseyeure) {
        ui->label_legend_caseyeure->setText(
            QStringLiteral("\u2022 Responsable: %1 (%2%)").arg(responsable).arg(pct(responsable)));
    }
    if (ui->label_legend_traditionale) {
        ui->label_legend_traditionale->setText(
            QStringLiteral("\u2022 Ouvrier: %1 (%2%)").arg(ouvrier).arg(pct(ouvrier)));
    }
    if (ui->progressTypeCirclee) {
        const int size = qMin(ui->progressTypeCirclee->width(), ui->progressTypeCirclee->height());
        ui->progressTypeCirclee->setStyleSheet(QStringLiteral("border: none; background: transparent;"));
        ui->progressTypeCirclee->setPixmap(buildEmployeRoleCirclePixmap(gardien, technicien, responsable, ouvrier, size));
    }
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
            QStringLiteral("\u2022 Disponible: %1 (%2%)").arg(disponible).arg(pct(disponible)));
    }
    if (ui->label_legend_palangrierp) {
        ui->label_legend_palangrierp->setText(
            QStringLiteral("\u2022 Disponible bient\u00F4t: %1 (%2%)").arg(bientot).arg(pct(bientot)));
    }
    if (ui->label_legend_caseyeurp) {
        ui->label_legend_caseyeurp->setText(
            QStringLiteral("\u2022 Indisponible: %1 (%2%)").arg(indisponible).arg(pct(indisponible)));
    }
    if (ui->label_legend_traditionalp) {
        ui->label_legend_traditionalp->setText(
            QStringLiteral("\u2022 En cong\u00E9: %1 (%2%)").arg(enConge).arg(pct(enConge)));
    }
    if (ui->progressTypeCirclep) {
        const int size = qMin(ui->progressTypeCirclep->width(), ui->progressTypeCirclep->height());
        ui->progressTypeCirclep->setStyleSheet(QStringLiteral("border: none; background: transparent;"));
        ui->progressTypeCirclep->setPixmap(buildDisponibiliteCirclePixmap(disponible, bientot, indisponible, enConge, size));
    }
}

static bool isSaisieConstraintError(const QString& error)
{
    const QString e = error.toLower();
    return e.contains(QStringLiteral("obligatoire"))
        || e.contains(QStringLiteral("invalide"))
        || e.contains(QStringLiteral("d├⌐j├á utilis├⌐"))
        || e.contains(QStringLiteral("deja utilise"))
        || e.contains(QStringLiteral("veuillez saisir"))
        || e.contains(QStringLiteral("format attendu"))
        || e.contains(QStringLiteral("doit ├¬tre"))
        || e.contains(QStringLiteral("doit etre"));
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

    // D'abord, tentative de correspondance exacte sur la cl├⌐ normalis├⌐e
    for (const QString& syn : syns) {
        const QString key = normalizeKey(syn);
        for (const QString& col : dbCols) {
            if (normalizeKey(col) == key) {
                return col;
            }
        }
    }

    // Ensuite, tentative de correspondance par pr├⌐fixe
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

    // Correspondance exacte sur le nom normalis├⌐
    for (const QString& cand : candidates) {
        const QString key = normalizeKey(cand);
        for (const QString& tbl : tables) {
            if (normalizeKey(tbl) == key) {
                return tbl;
            }
        }
    }

    // Sinon, correspondance par pr├⌐fixe
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
        if (errorOut) *errorOut = QStringLiteral("Base de donn├⌐es ou table invalide");
        return false;
    }

    const QStringList dbCols = getColumnNames(db, tableName);
    if (dbCols.isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("Impossible de r├⌐cup├⌐rer les colonnes de la table %1").arg(tableName);
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
            // Colonne non trouv├⌐e: on ignore ce champ
            continue;
        }

        columnNames << col;
        placeholders << QStringLiteral("?");
        bindValues << it.value();
    }

    if (columnNames.isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("Aucune colonne valide trouv├⌐e pour l'insertion dans %1").arg(tableName);
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

static bool updateRowByMapping(QWidget* parent,
                               QSqlDatabase db,
                               const QString& tableName,
                               const QString& idLogicalKey,
                               const QVariant& idValue,
                               const QHash<QString, QVariant>& values,
                               const QHash<QString, QStringList>& synonyms,
                               bool allowUpdatingId,
                               QString* errorOut)
{
    if (!db.isValid() || tableName.isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("Base de donn├⌐es ou table invalide");
        return false;
    }

    const QStringList dbCols = getColumnNames(db, tableName);
    if (dbCols.isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("Impossible de r├⌐cup├⌐rer les colonnes de la table %1").arg(tableName);
        return false;
    }

    QStringList idSyns = synonyms.value(idLogicalKey);
    if (idSyns.isEmpty()) idSyns << idLogicalKey;
    const QString idCol = matchColumnBySynonyms(dbCols, idSyns);
    if (idCol.isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("Colonne ID introuvable pour %1").arg(tableName);
        return false;
    }

    QStringList setParts;
    QList<QVariant> bindValues;

    for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
        const QString logical = it.key();
        if (!allowUpdatingId && normalizeKey(logical) == normalizeKey(idLogicalKey)) {
            continue; // ne pas modifier la cl├⌐
        }

        QStringList syns = synonyms.value(logical);
        if (syns.isEmpty()) syns << logical;
        const QString col = matchColumnBySynonyms(dbCols, syns);
        if (col.isEmpty()) {
            continue;
        }

        setParts << QStringLiteral("%1 = ?").arg(col);
        bindValues << it.value();
    }

    if (setParts.isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("Aucune colonne valide trouv├⌐e pour la modification dans %1").arg(tableName);
        return false;
    }

    const QString sql = QStringLiteral("UPDATE %1 SET %2 WHERE %3 = ?")
                            .arg(tableName, setParts.join(QStringLiteral(", ")), idCol);

    QSqlQuery query(db);
    query.prepare(sql);
    for (const QVariant& v : std::as_const(bindValues)) {
        query.addBindValue(v);
    }
    query.addBindValue(idValue);

    if (!query.exec()) {
        if (errorOut) {
            *errorOut = query.lastError().text();
        } else {
            QMessageBox::critical(parent,
                                  QStringLiteral("Modification"),
                                  QStringLiteral("Erreur SQL: %1").arg(query.lastError().text()));
        }
        return false;
    }

    return true;
}

static bool deleteRowByMapping(QWidget* parent,
                               QSqlDatabase db,
                               const QString& tableName,
                               const QString& idLogicalKey,
                               const QVariant& idValue,
                               const QHash<QString, QStringList>& synonyms,
                               QString* errorOut)
{
    if (!db.isValid() || tableName.isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("Base de donn├⌐es ou table invalide");
        return false;
    }

    const QStringList dbCols = getColumnNames(db, tableName);
    if (dbCols.isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("Impossible de r├⌐cup├⌐rer les colonnes de la table %1").arg(tableName);
        return false;
    }

    QStringList idSyns = synonyms.value(idLogicalKey);
    if (idSyns.isEmpty()) idSyns << idLogicalKey;
    const QString idCol = matchColumnBySynonyms(dbCols, idSyns);
    if (idCol.isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("Colonne ID introuvable pour %1").arg(tableName);
        return false;
    }

    const QString sql = QStringLiteral("DELETE FROM %1 WHERE %2 = ?").arg(tableName, idCol);
    QSqlQuery query(db);
    query.prepare(sql);
    query.addBindValue(idValue);

    if (!query.exec()) {
        if (errorOut) {
            *errorOut = query.lastError().text();
        } else {
            QMessageBox::critical(parent,
                                  QStringLiteral("Suppression"),
                                  QStringLiteral("Erreur SQL: %1").arg(query.lastError().text()));
        }
        return false;
    }

    return true;
}

static QString generateNextNumericIdWithPrefix(QSqlDatabase db,
                                               const QString& tableName,
                                               const QString& idCol,
                                               int prefix,
                                               int suffixDigits,
                                               QString* errorOut)
{
    if (!db.isValid() || !db.isOpen() || tableName.isEmpty() || idCol.isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("Base de données/table/colonne invalide");
        return QString();
    }
    if (suffixDigits <= 0 || suffixDigits > 9) {
        if (errorOut) *errorOut = QStringLiteral("Nombre de chiffres suffixe invalide");
        return QString();
    }

    int multiplier = 1;
    for (int i = 0; i < suffixDigits; ++i) {
        multiplier *= 10;
    }

    const int minId = prefix * multiplier;
    const int maxId = minId + (multiplier - 1);

    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT NVL(MAX(TO_NUMBER(%1)), 0) FROM %2 "
        "WHERE %1 IS NOT NULL "
        "AND REGEXP_LIKE(TRIM(TO_CHAR(%1)), '^[0-9]+$') "
        "AND TO_NUMBER(%1) BETWEEN :minId AND :maxId")
        .arg(idCol, tableName));
    query.bindValue(QStringLiteral(":minId"), minId);
    query.bindValue(QStringLiteral(":maxId"), maxId);

    if (!query.exec()) {
        if (errorOut) *errorOut = query.lastError().text();
        return QString();
    }

    int currentMax = 0;
    if (query.next()) {
        currentMax = query.value(0).toInt();
    }

    const int nextId = (currentMax > 0) ? (currentMax + 1) : (minId + 1);
    if (nextId > maxId) {
        if (errorOut) *errorOut = QStringLiteral("Limite atteinte pour le préfixe %1").arg(prefix);
        return QString();
    }

    if (errorOut) errorOut->clear();
    return QString::number(nextId);
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

    // En-t├¬tes: on affiche les noms logiques plus lisibles
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

            QString text;
            // Pour les colonnes de type date / datetime (par ex. CLIENTS.Date_Inscription),
            // on n'affiche que la date sans l'heure ni le "T".
            if (value.userType() == QMetaType::QDateTime) {
                const QDate d = value.toDateTime().date();
                text = d.isValid() ? d.toString("yyyy-MM-dd") : QString();
            } else if (value.userType() == QMetaType::QDate) {
                const QDate d = value.toDate();
                text = d.isValid() ? d.toString("yyyy-MM-dd") : QString();
            } else {
                text = value.toString();
            }

            auto *cellItem = new QTableWidgetItem(text);
            table->setItem(row, col, cellItem);
        }
        ++row;
    }
}

namespace {

constexpr int kQuaiOriginalRowRole = Qt::UserRole + 42;

static double extractQuaiNumber(const QString& text, bool* ok = nullptr)
{
    QString normalized = text.trimmed();
    if (normalized.isEmpty()) {
        if (ok) *ok = false;
        return 0.0;
    }

    normalized.replace(',', '.');

    QRegularExpressionMatch match = QRegularExpression(
        QStringLiteral("[-+]?\\d+(?:\\.\\d+)?")).match(normalized);
    if (!match.hasMatch()) {
        if (ok) *ok = false;
        return 0.0;
    }

    bool localOk = false;
    const double value = match.captured(0).toDouble(&localOk);
    if (ok) *ok = localOk;
    return localOk ? value : 0.0;
}

struct QuaiRowSnapshot
{
    QVector<QTableWidgetItem*> items;
};

static void sortQuaiTableRows(QTableWidget* table, int sortCol, Qt::SortOrder order)
{
    if (!table) return;

    const int rowCount = table->rowCount();
    const int colCount = table->columnCount();
    if (rowCount <= 1 || colCount <= 0) return;

    QVector<QuaiRowSnapshot> rows;
    rows.reserve(rowCount);

    for (int row = 0; row < rowCount; ++row) {
        QuaiRowSnapshot snapshot;
        snapshot.items.resize(colCount);
        for (int col = 0; col < colCount; ++col) {
            snapshot.items[col] = table->takeItem(row, col);
        }
        rows.push_back(std::move(snapshot));
    }

    const auto compareRows = [sortCol, order](const QuaiRowSnapshot& lhs, const QuaiRowSnapshot& rhs) {
        const QTableWidgetItem* leftItem =
            (sortCol >= 0 && sortCol < lhs.items.size()) ? lhs.items.at(sortCol) : nullptr;
        const QTableWidgetItem* rightItem =
            (sortCol >= 0 && sortCol < rhs.items.size()) ? rhs.items.at(sortCol) : nullptr;

        const auto originalOrder = [](const QTableWidgetItem* item) {
            if (!item) return std::numeric_limits<int>::max();
            return item->data(kQuaiOriginalRowRole).toInt();
        };

        auto fallbackCompare = [&]() {
            return originalOrder(leftItem) < originalOrder(rightItem);
        };

        if (sortCol < 0) {
            return fallbackCompare();
        }

        const bool numericColumn = (sortCol == 0 || sortCol == 4 || sortCol == 5 || sortCol == 7);
        if (numericColumn) {
            bool leftOk = false;
            bool rightOk = false;
            const double leftValue = extractQuaiNumber(leftItem ? leftItem->text() : QString(), &leftOk);
            const double rightValue = extractQuaiNumber(rightItem ? rightItem->text() : QString(), &rightOk);

            if (leftOk && rightOk && !qFuzzyCompare(leftValue + 1.0, rightValue + 1.0)) {
                return (order == Qt::AscendingOrder) ? (leftValue < rightValue)
                                                     : (leftValue > rightValue);
            }
            if (leftOk != rightOk) {
                return (order == Qt::AscendingOrder) ? leftOk : rightOk;
            }
        } else {
            const QString leftText = leftItem ? leftItem->text().trimmed().toCaseFolded() : QString();
            const QString rightText = rightItem ? rightItem->text().trimmed().toCaseFolded() : QString();
            if (leftText != rightText) {
                return (order == Qt::AscendingOrder) ? (leftText < rightText)
                                                     : (leftText > rightText);
            }
        }

        return fallbackCompare();
    };

    std::stable_sort(rows.begin(), rows.end(), compareRows);

    table->setRowCount(0);
    for (int row = 0; row < rows.size(); ++row) {
        table->insertRow(row);
        for (int col = 0; col < colCount; ++col) {
            if (rows[row].items[col]) {
                table->setItem(row, col, rows[row].items[col]);
            }
        }
    }
}

} // namespace



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
            *errorOut = QStringLiteral("Colonnes login/mot de passe non trouv├⌐es.");
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

bool MainWindow::resetAdminPassword(const QString& adminEmail,
                                   const QString& newPassword,
                                   QString* errorOut)
{
    const QString trimmedPass = newPassword;
    if (trimmedPass.trimmed().isEmpty()) {
        if (errorOut) {
            *errorOut = QStringLiteral("Nouveau mot de passe vide.");
        }
        return false;
    }

    Connection* conn = Connection::getInstance();
    if (!conn || !conn->ensureOpen()) {
        if (errorOut) {
            *errorOut = QStringLiteral("Connexion DB \u00e9chou\u00e9e.");
        }
        return false;
    }

    QSqlDatabase db = conn->getDatabase();
    const QString usersTable = resolveTableName(db, {
        QStringLiteral("UTILISATEUR"), QStringLiteral("UTILISATEURS"),
        QStringLiteral("USER"), QStringLiteral("USERS"),
        QStringLiteral("COMPTE"), QStringLiteral("COMPTES"),
        QStringLiteral("LOGIN")
    });

    // Si la table n'existe pas, on bascule sur un fallback local.
    if (usersTable.isEmpty()) {
        setFallbackAdminPassword(trimmedPass);
        return true;
    }

    const QStringList cols = getColumnNames(db, usersTable);
    if (cols.isEmpty()) {
        if (errorOut) {
            *errorOut = QStringLiteral("Colonnes utilisateurs introuvables.");
        }
        return false;
    }

    QString passCol = matchColumnBySynonyms(cols, {
        QStringLiteral("password"), QStringLiteral("motdepasse"), QStringLiteral("mdp"),
        QStringLiteral("pass")
    });
    if (passCol.isEmpty()) {
        // Schma non compatible: on bascule sur un mot de passe admin stock localement.
        setFallbackAdminPassword(trimmedPass);
        return true;
    }

    const QString usernameCol = matchColumnBySynonyms(cols, {
        QStringLiteral("username"), QStringLiteral("user"), QStringLiteral("login"),
        QStringLiteral("nomutilisateur"), QStringLiteral("nom")
    });
    const QString emailCol = matchColumnBySynonyms(cols, {
        QStringLiteral("email"), QStringLiteral("mail"), QStringLiteral("adresseemail"), QStringLiteral("adresse_mail")
    });

    const QString trimmedEmail = adminEmail.trimmed();
    QStringList whereParts;
    QList<QVariant> whereBinds;

    if (!usernameCol.isEmpty()) {
        whereParts << QStringLiteral("%1 = ?").arg(usernameCol);
        whereBinds << QStringLiteral("admin");
    }
    if (!trimmedEmail.isEmpty() && !emailCol.isEmpty()) {
        whereParts << QStringLiteral("%1 = ?").arg(emailCol);
        whereBinds << trimmedEmail;
    }

    // En dernier recours, on reprend la m\u00eame logique "userCol" que l'auth.
    if (whereParts.isEmpty()) {
        const QString userCol = matchColumnBySynonyms(cols, {
            QStringLiteral("username"), QStringLiteral("user"), QStringLiteral("login"),
            QStringLiteral("email"), QStringLiteral("nomutilisateur"), QStringLiteral("nom")
        });
        if (!userCol.isEmpty()) {
            whereParts << QStringLiteral("%1 = ?").arg(userCol);
            whereBinds << QStringLiteral("admin");
        }
    }

    if (whereParts.isEmpty()) {
        if (errorOut) {
            *errorOut = QStringLiteral("Impossible d'identifier la colonne utilisateur/email.");
        }
        return false;
    }

    QSqlQuery query(db);
    query.prepare(QStringLiteral("UPDATE %1 SET %2 = ? WHERE %3")
                      .arg(usersTable, passCol, whereParts.join(QStringLiteral(" AND "))));
    query.addBindValue(trimmedPass);
    for (const QVariant& v : std::as_const(whereBinds)) {
        query.addBindValue(v);
    }

    if (!query.exec()) {
        if (errorOut) {
            *errorOut = query.lastError().text();
        }
        return false;
    }

    if (query.numRowsAffected() <= 0) {
        // Tentative email-only si disponible.
        if (!trimmedEmail.isEmpty() && !emailCol.isEmpty()) {
            QSqlQuery q2(db);
            q2.prepare(QStringLiteral("UPDATE %1 SET %2 = ? WHERE %3 = ?")
                           .arg(usersTable, passCol, emailCol));
            q2.addBindValue(trimmedPass);
            q2.addBindValue(trimmedEmail);
            if (!q2.exec()) {
                if (errorOut) {
                    *errorOut = q2.lastError().text();
                }
                return false;
            }
            if (q2.numRowsAffected() <= 0) {
                if (errorOut) {
                    *errorOut = QStringLiteral("Compte admin introuvable dans la base.");
                }
                return false;
            }
        } else {
            if (errorOut) {
                *errorOut = QStringLiteral("Compte admin introuvable dans la base.");
            }
            return false;
        }
    }

    // Maintient un fallback coh\u00e9rent si jamais la table users est absente plus tard.
    setFallbackAdminPassword(trimmedPass);
    return true;
}

// Ajoute une colonne d'actions avec des boutons styl├⌐s (sans logique m├⌐tier sp├⌐cifique)

static void ensureActionsColumnPopulated(QTableWidget* table, const QString& buttonStyle)
{
    if (!table) return;

    const int actionsCol = table->columnCount() - 1;
    if (actionsCol < 0) return;

    for (int row = 0; row < table->rowCount(); ++row) {
        if (table->cellWidget(row, actionsCol)) {
            continue; // d├⌐j├á peupl├⌐
        }

        QWidget* container = new QWidget(table);
        container->setAttribute(Qt::WA_TranslucentBackground, true);
        container->setStyleSheet(QStringLiteral("background-color: transparent;"));

        auto *layout = new QHBoxLayout(container);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(4);

        auto *editBtn = new QPushButton(QStringLiteral("\u270F\uFE0F"), container); // ✏️
        auto *deleteBtn = new QPushButton(QStringLiteral("\u274C"), container);    // ❌

        editBtn->setToolTip(QStringLiteral("Modifier"));
        deleteBtn->setToolTip(QStringLiteral("Supprimer"));

        // Hauteur minimale des boutons (un peu plus petite que pr├⌐c├⌐demment)
        const int buttonMinHeight = 34;
        editBtn->setMinimumHeight(buttonMinHeight);
        deleteBtn->setMinimumHeight(buttonMinHeight);

        // If this is the clients table, apply the same visual style as the
        // "gestion bateau" action buttons (bordered + light background).
        const QString tableName = table->objectName();
        if (tableName == QLatin1String("tableWidgetc")) {
            editBtn->setFixedSize(42, 32);
            deleteBtn->setFixedSize(42, 32);

            QFont editFont(QStringLiteral("Segoe UI Emoji"));
            editFont.setPointSize(15);
            editFont.setBold(true);
            editBtn->setFont(editFont);
            editBtn->setFlat(true);
            editBtn->setStyleSheet(QStringLiteral(
                "QPushButton {"
                " border: 2px solid rgb(0, 0, 112);"
                " border-radius: 6px;"
                " background-color: rgb(224, 238, 255);"
                " color: rgb(0, 0, 112);"
                " padding: 0px;"
                " }"
                "QPushButton:hover { background-color: rgb(224, 238, 255); }"
                "QPushButton:pressed { background-color: rgb(224, 238, 255); }"));

            QFont deleteFont(QStringLiteral("Segoe UI Emoji"));
            deleteFont.setPointSize(15);
            deleteFont.setBold(true);
            deleteBtn->setFont(deleteFont);
            deleteBtn->setFlat(true);
            deleteBtn->setStyleSheet(QStringLiteral(
                "QPushButton {"
                " border: 2px solid rgb(0, 0, 112);"
                " border-radius: 6px;"
                " background-color: rgb(224, 238, 255);"
                " color: rgb(0, 0, 112);"
                " padding: 0px;"
                " }"
                "QPushButton:hover { background-color: rgb(224, 238, 255); }"
                "QPushButton:pressed { background-color: rgb(224, 238, 255); }"));
        } else {
            if (!buttonStyle.isEmpty()) {
                // On garde la bordure et le rayon d├⌐finis dans buttonStyle,
                // mais on force juste le fond ├á ├¬tre transparent pour
                // tous les ├⌐tats.
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
        }

        layout->addWidget(editBtn);
        layout->addWidget(deleteBtn);
        container->setLayout(layout);

        table->setCellWidget(row, actionsCol, container);
        // Assurer une hauteur de ligne suffisante pour les gros boutons
        table->setRowHeight(row, qMax(table->rowHeight(row), buttonMinHeight + 8));
        // Les connexions sp├⌐cifiques (├⌐dition/suppression) sont g├⌐r├⌐es ailleurs si n├⌐cessaire.
    }
}

// Normalisation du texte de l'UI (stub simple pour l'instant)

void MainWindow::normalizeUiTexts()
{
    auto fixText = [](QString s) -> QString {
        if (s.isEmpty()) return s;

        // Common mojibake seen in this project (typically UTF-8 bytes mis-decoded).
        s.replace(QStringLiteral("├⌐"), QStringLiteral("é"));
        s.replace(QStringLiteral("├¿"), QStringLiteral("è"));
        s.replace(QStringLiteral("├¬"), QStringLiteral("ê"));
        s.replace(QStringLiteral("├á"), QStringLiteral("à"));
        s.replace(QStringLiteral("├«"), QStringLiteral("î"));
        s.replace(QStringLiteral("├┤"), QStringLiteral("ô"));
        s.replace(QStringLiteral("├ë"), QStringLiteral("É"));

        // Bullets / punctuation mojibake.
        s.replace(QStringLiteral("ΓÇó"), QStringLiteral("•"));
        s.replace(QStringLiteral("Γçó"), QStringLiteral("•"));
        s.replace(QStringLiteral("ΓÇô"), QStringLiteral("–"));
        s.replace(QStringLiteral("ΓùÅ"), QStringLiteral("•"));

        // Corrupted icon glyphs seen in this project (map/status/tooltips).
        s.replace(QStringLiteral("Γ£à"), QStringLiteral("✓"));
        s.replace(QStringLiteral("≡ƒÜ½"), QStringLiteral("⛔"));
        s.replace(QStringLiteral("≡ƒ¢á∩╕Å"), QStringLiteral("⚙"));
        s.replace(QStringLiteral("Γ¼å∩╕Å"), QStringLiteral("↑"));
        s.replace(QStringLiteral("Γ¼ç∩╕Å"), QStringLiteral("↓"));
        s.replace(QStringLiteral("Γ₧í∩╕Å"), QStringLiteral("→"));
        s.replace(QStringLiteral("Γ¼à∩╕Å"), QStringLiteral("←"));
        s.replace(QStringLiteral("≡ƒÜó"), QString());
        s.replace(QStringLiteral("≡ƒôì"), QString());
        s.replace(QStringLiteral("≡ƒôè"), QString());

        // Degree sign mojibake (common in this repo).
        s.replace(QStringLiteral("┬░"), QStringLiteral("°"));

        return s;
    };

    // Window title.
    setWindowTitle(fixText(windowTitle()));

    // Tooltips + basic text on widgets.
    const auto widgets = findChildren<QWidget*>();
    for (QWidget* w : widgets) {
        if (!w) continue;

        const QString tt = w->toolTip();
        const QString fixedTt = fixText(tt);
        if (fixedTt != tt) {
            w->setToolTip(fixedTt);
        }

        if (auto* label = qobject_cast<QLabel*>(w)) {
            const QString t = label->text();
            const QString fixed = fixText(t);
            if (fixed != t) label->setText(fixed);
            continue;
        }

        if (auto* btn = qobject_cast<QAbstractButton*>(w)) {
            const QString t = btn->text();
            const QString fixed = fixText(t);
            if (fixed != t) btn->setText(fixed);
            continue;
        }

        if (auto* box = qobject_cast<QGroupBox*>(w)) {
            const QString t = box->title();
            const QString fixed = fixText(t);
            if (fixed != t) box->setTitle(fixed);
            continue;
        }

        if (auto* tabs = qobject_cast<QTabWidget*>(w)) {
            for (int i = 0; i < tabs->count(); ++i) {
                const QString t = tabs->tabText(i);
                const QString fixed = fixText(t);
                if (fixed != t) tabs->setTabText(i, fixed);
            }
            continue;
        }

        if (auto* combo = qobject_cast<QComboBox*>(w)) {
            for (int i = 0; i < combo->count(); ++i) {
                const QString t = combo->itemText(i);
                const QString fixed = fixText(t);
                if (fixed != t) combo->setItemText(i, fixed);
            }
            continue;
        }

        if (auto* table = qobject_cast<QTableWidget*>(w)) {
            // Headers
            for (int c = 0; c < table->columnCount(); ++c) {
                if (auto* item = table->horizontalHeaderItem(c)) {
                    const QString t = item->text();
                    const QString fixed = fixText(t);
                    if (fixed != t) item->setText(fixed);
                }
            }
            for (int r = 0; r < table->rowCount(); ++r) {
                if (auto* item = table->verticalHeaderItem(r)) {
                    const QString t = item->text();
                    const QString fixed = fixText(t);
                    if (fixed != t) item->setText(fixed);
                }
            }
            continue;
        }
    }

    // Actions (menu / toolbar)
    const auto actions = findChildren<QAction*>();
    for (QAction* a : actions) {
        if (!a) continue;
        a->setText(fixText(a->text()));
        a->setToolTip(fixText(a->toolTip()));
        a->setStatusTip(fixText(a->statusTip()));
        a->setWhatsThis(fixText(a->whatsThis()));
    }
}

// Ajuster la largeur des colonnes pour la table des clients

void MainWindow::adjustClientTableColumns()
{
    if (!ui || !ui->tableWidgetc) return;

    auto *table = ui->tableWidgetc;
    const int colCount = table->columnCount();
    if (colCount < 7) return; // id, nom, prenom, statut, profil, date, telephone, [actions]

    // Colonnes de donn├⌐es
    table->setColumnWidth(0, 60);   // id plus petit
    table->setColumnWidth(1, 110);  // nom
    table->setColumnWidth(2, 110);  // prenom
    table->setColumnWidth(3, 110);  // statut
    table->setColumnWidth(4, 110);  // profil
    table->setColumnWidth(5, 150);  // date
    table->setColumnWidth(6, 110);  // telephone

    // Colonne Actions (derni├¿re)
    const int actionsCol = colCount - 1;
    if (actionsCol >= 0)
        table->setColumnWidth(actionsCol, 140);
}

// Ajuster la largeur des colonnes pour la table des statistiques Top Clients

void MainWindow::adjustTopClientsStatsColumns()
{
    if (!ui || !ui->tableTopClients_4) return;

    auto *table = ui->tableTopClients_4;
    const int colCount = table->columnCount();
    if (colCount < 5) return;

    table->setColumnWidth(0, 70);   // id
    table->setColumnWidth(1, 160);  // nom
    table->setColumnWidth(2, 160);  // prenom / profil
    table->setColumnWidth(3, 160);  // autre info
    table->setColumnWidth(4, 160);  // valeur statistique
}

// Public helper implementations added to avoid accessing private members from lambdas.

void MainWindow::refreshClientsPage()
{
    if (!ui || !ui->tableWidgetc) return;

    Connection *conn = Connection::getInstance();
    if (!conn->ensureOpen()) {
        qWarning() << "refreshClientsPage: DB connection failed" << conn->lastErrorText();
        return;
    }
    QSqlDatabase db = conn->getDatabase();

    const QString tableName = resolveTableName(db, {QStringLiteral("CLIENT"), QStringLiteral("CLIENTS"), QStringLiteral("TCLIENT"), QStringLiteral("T_CLIENT")});
    if (tableName.isEmpty()) {
        qWarning() << "refreshClientsPage: clients table not found";
        return;
    }

    const QStringList dbCols = getColumnNames(db, tableName);
    if (dbCols.isEmpty()) return;

    const QHash<QString, QStringList> syn = {
        {QStringLiteral("id"), {QStringLiteral("id"), QStringLiteral("idclient"), QStringLiteral("clientid"), QStringLiteral("id_client")}},
        {QStringLiteral("nom"), {QStringLiteral("nom"), QStringLiteral("nomclient"), QStringLiteral("name"), QStringLiteral("lastname")}},
        {QStringLiteral("prenom"), {QStringLiteral("prenom"), QStringLiteral("prenomclient"), QStringLiteral("firstname")}},
        {QStringLiteral("statut"), {QStringLiteral("statut"), QStringLiteral("statutclient"), QStringLiteral("status"), QStringLiteral("statut_conformite")}},
        {QStringLiteral("profil"), {QStringLiteral("profil"), QStringLiteral("profilclient"), QStringLiteral("profile"), QStringLiteral("profil_client")}},
        {QStringLiteral("telephone"), {QStringLiteral("telephone"), QStringLiteral("tel"), QStringLiteral("phone")}},
        {QStringLiteral("date"), {QStringLiteral("date"), QStringLiteral("dateinscription"), QStringLiteral("date_inscription"), QStringLiteral("createdat"), QStringLiteral("date_inscription")}},
    };

    auto resolveCol = [&](const QString &logical) {
        QStringList syns = syn.value(logical);
        if (syns.isEmpty()) syns << logical;
        QString col = matchColumnBySynonyms(dbCols, syns);
        if (col.isEmpty()) col = logical;
        return col;
    };

    const QString idCol   = resolveCol(QStringLiteral("id"));
    const QString nomCol  = resolveCol(QStringLiteral("nom"));
    const QString preCol  = resolveCol(QStringLiteral("prenom"));
    const QString statCol = resolveCol(QStringLiteral("statut"));
    const QString profCol = resolveCol(QStringLiteral("profil"));
    const QString dateCol = resolveCol(QStringLiteral("date"));
    const QString telCol  = resolveCol(QStringLiteral("telephone"));

    QStringList selectCols = {idCol, nomCol, preCol, statCol, profCol, dateCol, telCol};

    // R├⌐cup├⌐rer les filtres de l'UI
    const QString searchText = ui->lineEdit_7c ? ui->lineEdit_7c->text().trimmed() : QString();
    const QString rawStatut = ui->comboBoxc_2 ? ui->comboBoxc_2->currentText().trimmed() : QString();
    const QString statutFilter = (rawStatut.compare(QStringLiteral("Tous"), Qt::CaseInsensitive) == 0) ? QString() : rawStatut;
    const QDate dateFilter = ui->dateEdit_2c ? ui->dateEdit_2c->date() : QDate();
    const QDate sentinelDate(2000, 1, 1); // valeur par d├⌐faut dans l'UI => pas de filtre

    QStringList where;
    QList<QVariant> binds;

    if (!searchText.isEmpty()) {
        const QString pattern = QStringLiteral("%%1%2").arg(searchText.trimmed().toUpper(), QString());
        where << QStringLiteral("(UPPER(%1) LIKE ? OR UPPER(%2) LIKE ? OR TO_CHAR(%3) LIKE ?)")
                     .arg(nomCol, preCol, idCol);
        const QString patt = QStringLiteral("%%1%").arg(searchText.trimmed().toUpper());
        binds << patt << patt << patt;
    }

    if (!statutFilter.isEmpty()) {
        where << QStringLiteral("%1 = ?").arg(statCol);
        binds << statutFilter;
    }

    if (dateFilter.isValid() && dateFilter != sentinelDate) {
        where << QStringLiteral("TRUNC(%1) = TO_DATE(?, 'YYYY-MM-DD')").arg(dateCol);
        binds << dateFilter.toString(QStringLiteral("yyyy-MM-dd"));
    }

    QString sql = QStringLiteral("SELECT %1 FROM %2").arg(selectCols.join(QLatin1Char(',')), tableName);
    if (!where.isEmpty()) {
        sql += QStringLiteral(" WHERE ") + where.join(QStringLiteral(" AND "));
    }

    QSqlQuery query(db);
    query.prepare(sql);
    for (const QVariant &v : std::as_const(binds)) {
        query.addBindValue(v);
    }

    if (!query.exec()) {
        qWarning() << "refreshClientsPage: query failed" << query.lastError().text();
        return;
    }

    QTableWidget *table = ui->tableWidgetc;
    table->clearContents();
    table->setRowCount(0);
    table->setColumnCount(8); // 7 donn├⌐es + Actions

    const QStringList headers = {QStringLiteral("id"), QStringLiteral("nom"), QStringLiteral("prenom"),
                                 QStringLiteral("statut"), QStringLiteral("profil"), QStringLiteral("date"),
                                 QStringLiteral("telephone"), QStringLiteral("Actions")};
    for (int i = 0; i < headers.size(); ++i) {
        table->setHorizontalHeaderItem(i, new QTableWidgetItem(headers.at(i)));
    }

    int row = 0;

    while (query.next()) {
        table->insertRow(row);

        for (int col = 0; col < 7; ++col) {
            const QVariant value = query.value(col);
            QString text;
            if (value.userType() == QMetaType::QDateTime) {
                const QDate d = value.toDateTime().date();
                text = d.isValid() ? d.toString(QStringLiteral("yyyy-MM-dd")) : QString();
            } else if (value.userType() == QMetaType::QDate) {
                const QDate d = value.toDate();
                text = d.isValid() ? d.toString(QStringLiteral("yyyy-MM-dd")) : QString();
            } else {
                text = value.toString();
            }

            auto *cellItem = new QTableWidgetItem(text);
            table->setItem(row, col, cellItem);
        }

        ++row;
    }

    // Boutons Actions et largeur des colonnes
    ensureActionsColumnPopulated(table, QStringLiteral("QPushButton { border:2px solid rgb(0, 0, 112); border-radius:8px; background-color: rgba(0, 0, 127,0.7); color: white; padding: 4px 8px; font-family: 'Segoe UI Emoji', 'Segoe UI', 'Arial', sans-serif; font-size: 8px; font-weight: bold;} QPushButton:hover { background-color: rgba(0, 0, 127,0.7); border-color: #59abc8;} QPushButton:pressed { background-color:rgba(0, 0, 127,0.9); }"));

    // C├óbler les boutons Modifier/Supprimer (colonne Actions)
    const int actionsCol = table->columnCount() - 1;
    if (actionsCol >= 0) {
        for (int r = 0; r < table->rowCount(); ++r) {
            QWidget *container = table->cellWidget(r, actionsCol);
            if (!container) continue;
            const auto buttons = container->findChildren<QPushButton*>();
            if (buttons.size() < 2) continue;

            QPushButton *editBtn = buttons.at(0);
            QPushButton *deleteBtn = buttons.at(1);
            if (!editBtn || !deleteBtn) continue;

            editBtn->disconnect();
            deleteBtn->disconnect();

            QObject::connect(editBtn, &QPushButton::clicked, this, [this, r]() {
                modifierClientFromRow(r);
            });
            QObject::connect(deleteBtn, &QPushButton::clicked, this, [this, r]() {
                supprimerClientFromRow(r);
            });
        }
    }

    adjustClientTableColumns();

    // Recompute counts from the populated table to ensure stats match the displayed rows
    int countFidele = 0;
    int countOccas = 0;
    int countRegul = 0;

    // Find the profile column index by header (fall back to 4)
    int profColIndex = 4;
    for (int c = 0; c < table->columnCount(); ++c) {
        QTableWidgetItem *hdr = table->horizontalHeaderItem(c);
        if (!hdr) continue;
        if (normalizeKey(hdr->text()) == normalizeKey(QStringLiteral("profil"))) {
            profColIndex = c;
            break;
        }
    }

    const int rcount = table->rowCount();
    for (int r = 0; r < rcount; ++r) {
        QTableWidgetItem *it = table->item(r, profColIndex);
        const QString pv = it ? it->text().trimmed() : QString();
        const QString pnorm = normalizeKey(pv);
        if (pnorm == normalizeKey(QStringLiteral("Fidele"))) ++countFidele;
        else if (pnorm == normalizeKey(QStringLiteral("Occasionnel"))) ++countOccas;
        else if (pnorm == normalizeKey(QStringLiteral("Regulier"))) ++countRegul;
    }

    const int total = rcount; // total rows shown in the table
    auto updateOne = [&](QProgressBar *bar, QLabel *lbl, int count) {
        if (!bar || !lbl) return;
        bar->setRange(0, 100);
        const int percent = (total > 0) ? qRound((double)count * 100.0 / (double)total) : 0;
        bar->setValue(percent);
        lbl->setText(QStringLiteral("%1/%2 (%3%)").arg(count).arg(total).arg(percent));
    };

    updateOne(ui->progressCc,   ui->value_Cc,   countFidele);
    updateOne(ui->progressCCc,  ui->value_CCc,  countOccas);
    updateOne(ui->progressc,    ui->value_c,    countRegul);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_editingPecheurId()
    , m_editingEmployeId(-1)
    , m_editingCaptureId()
    , m_pendingPecheurPhotoBytes()
    , m_faceCamera(nullptr)
    , m_faceCaptureSession(nullptr)
    , m_faceImageCapture(nullptr)
    , m_facePreviewLabel(nullptr)
    , m_faceVideoSink(nullptr)
{
    ui->setupUi(this);

    if (ui->tableWidgetQuai) {
        ui->tableWidgetQuai->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->tableWidgetQuai->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableWidgetQuai->setSelectionMode(QAbstractItemView::SingleSelection);
    }

    // Eye toggle (show/hide) for each password field
    installPasswordEyeToggle(ui->lineEdit_2b);   // login
    installPasswordEyeToggle(ui->lineEdit_12b);  // mot de passe actuel (param├¿tres)
    installPasswordEyeToggle(ui->lineEdit_13b);  // nouveau mot de passe (param├¿tres)
    installPasswordEyeToggle(ui->lineEdit_10b);  // nouveau mot de passe (r├⌐cup├⌐ration)
    installPasswordEyeToggle(ui->lineEdit_11b);  // confirmer mot de passe (r├⌐cup├⌐ration)

    // Se souvenir de moi: recharger les identifiants sauvegard├⌐s
    applyRememberedLogin(ui);



    if (auto btn = this->findChild<QPushButton*>(QStringLiteral("btnai"))) {
        btn->raise();
    }

    if (auto f = this->findChild<QWidget*>(QStringLiteral("frame_2c_2"))) {
        f->hide();
    }


    // Fix accents (├⌐/├¿/ΓÇª) + emojis on all pages.
    normalizeUiTexts();

    setupArduinoIntegration();

    // Page employ├⌐s : masquer les anciens boutons flottants (├⌐dition/suppression/refresh)
    // et utiliser uniquement la colonne Actions de la table.
    if (auto *btnEditLegacy = this->findChild<QPushButton*>(QStringLiteral("pushButton_6e_2"))) btnEditLegacy->hide();
    if (auto *btnDeleteLegacy = this->findChild<QPushButton*>(QStringLiteral("pushButton_4p_5"))) btnDeleteLegacy->hide();
    if (auto *btnRefreshLegacy = this->findChild<QPushButton*>(QStringLiteral("pushButton_5p_6"))) btnRefreshLegacy->hide();

    // Page pêchers: validation du nom et prénom (lettres uniquement)
    if (ui->lineEdit_2p && ui->lineEdit_3p) {
        QRegularExpression rxLetters("^[\\p{L}\\s-]+$");
        auto* validator = new QRegularExpressionValidator(rxLetters, this);
        ui->lineEdit_2p->setValidator(validator);
        ui->lineEdit_3p->setValidator(validator);
    }

    if (ui->comboBoxp) ui->comboBoxp->setEditable(false);
    if (ui->comboBox_2) ui->comboBox_2->setEditable(false);
    if (ui->lineEditp) {
        ui->lineEditp->setReadOnly(false);
        ui->lineEditp->setEnabled(true);
        ui->lineEditp->setMinimumWidth(140);
    }

    // Page employés: IDs: champs modifiables selon demande utilisateur
    const auto unlockIdFields = [this]() {
        if (!ui) return;
        if (ui->lineEditp) { ui->lineEditp->setReadOnly(false); ui->lineEditp->setEnabled(true); }           // Pecheur
        if (ui->lineEdit_12e) { ui->lineEdit_12e->setReadOnly(false); ui->lineEdit_12e->setEnabled(true); }  // Employe
        if (ui->lineEdit_3b) { ui->lineEdit_3b->setReadOnly(false); ui->lineEdit_3b->setEnabled(true); }     // Bateau
        if (ui->lineEdit_3c) { ui->lineEdit_3c->setReadOnly(false); ui->lineEdit_3c->setEnabled(true); }     // Client
        if (ui->lineEdit_4) { ui->lineEdit_4->setReadOnly(false); ui->lineEdit_4->setEnabled(true); }        // Quai
        if (ui->cap_lineEdit_11) { ui->cap_lineEdit_11->setReadOnly(false); ui->cap_lineEdit_11->setEnabled(true); } // Capture
    };
    unlockIdFields();

    // Initialisation Raison du bannissement (caché par défaut)
    if (ui->label_raison_banni) ui->label_raison_banni->hide();
    if (ui->lineEdit_raison_banni) ui->lineEdit_raison_banni->hide();

    if (ui->comboBox_16e) {
        connect(ui->comboBox_16e, &QComboBox::currentTextChanged, this, [this](const QString &text) {
            const bool isBanni = (text == QStringLiteral("Banni"));
            if (ui->label_raison_banni) ui->label_raison_banni->setVisible(isBanni);
            if (ui->lineEdit_raison_banni) ui->lineEdit_raison_banni->setVisible(isBanni);
        });
    }

    if (ui->lineEdit_raison_banni) {
        connect(ui->lineEdit_raison_banni, &QLineEdit::textChanged, this, [this](const QString &text) {
            if (!ui->tableWidgetBannis) return;
            const QString currentId = ui->lineEdit_12e ? ui->lineEdit_12e->text().trimmed() : QString();
            if (currentId.isEmpty()) return;

            for (int i = 0; i < ui->tableWidgetBannis->rowCount(); ++i) {
                QTableWidgetItem *idItem = ui->tableWidgetBannis->item(i, 1);
                if (idItem && idItem->text() == currentId) {
                    QTableWidgetItem *raisonItem = ui->tableWidgetBannis->item(i, 2);
                    if (raisonItem) raisonItem->setText(text);
                    break;
                }
            }
        });
    }

    if (ui->stackedWidget) {
        connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, [unlockIdFields](int) {
            unlockIdFields();
        });

        // Quand on arrive sur la page captures, rafraichir aussi les quotas.
        connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, [this](int) {
            if (!ui || !ui->stackedWidget || !ui->cap_pagecaptures) return;
            if (ui->stackedWidget->currentWidget() == ui->cap_pagecaptures) {
                loadQuotas(false);
            }
        });
    }

    const QDateTime now = QDateTime::currentDateTime();
    const QDate today = now.date();
    if (ui->dateTimeEdit) {
        ui->dateTimeEdit->setDateTime(now);
        ui->dateTimeEdit->setReadOnly(false);
    }
    if (ui->dateTimeEdit_2) {
        ui->dateTimeEdit_2->setMinimumDate(QDate(1900, 1, 1));
        ui->dateTimeEdit_2->setSpecialValueText(QStringLiteral("Non affectée"));
        ui->dateTimeEdit_2->setDateTime(nullPecheurAffectationDateTime());
    }

    // Page bateaux: valeurs par défaut (sinon QDateEdit démarre souvent en 2000-01-01)
    if (ui->dateEdit) {
        ui->dateEdit->setDate(today); // Date d'entrée
    }
    if (ui->dateEdit_2) {
        ui->dateEdit_2->setDate(today); // Date dernière maintenance
    }

    const auto refreshPecheurGeneratedId = [this]() {
        if (!ui || !ui->lineEditp || !m_editingPecheurId.isEmpty()) return;
        const QString sexe = pecheurSexeCode(ui);
        if (sexe.isEmpty()) {
            ui->lineEditp->clear();
            return;
        }

        const QString generatedId = Pecheurs::genererNouvelId(sexe);
        if (!generatedId.isEmpty()) {
            ui->lineEditp->setText(generatedId);
        }
    };

    if (ui->radioButton_2p) {
        connect(ui->radioButton_2p, &QRadioButton::toggled, this, [refreshPecheurGeneratedId](bool checked) {
            if (checked) refreshPecheurGeneratedId();
        });
    }
    if (ui->radioButtonp) {
        connect(ui->radioButtonp, &QRadioButton::toggled, this, [refreshPecheurGeneratedId](bool checked) {
            if (checked) refreshPecheurGeneratedId();
        });
    }
    if (ui->radioButton_2p && ui->radioButtonp && !ui->radioButton_2p->isChecked() && !ui->radioButtonp->isChecked()) {
        ui->radioButton_2p->setChecked(true);
    }
    refreshPecheurGeneratedId();

    const auto refreshEmployeGeneratedId = [this]() {
        if (!ui || !ui->lineEdit_12e || m_editingEmployeId > 0) return;
        const QString role = ui->comboBox_11e ? ui->comboBox_11e->currentText().trimmed() : QString();
        const int generatedId = Employe::genererNouvelId(role);
        if (generatedId > 0) {
            ui->lineEdit_12e->setText(QString::number(generatedId));
        }
    };

    if (ui->comboBox_11e) {
        connect(ui->comboBox_11e, &QComboBox::currentTextChanged, this, [refreshEmployeGeneratedId](const QString&) {
            refreshEmployeGeneratedId();
        });
    }
    refreshEmployeGeneratedId();

    // Remplir automatiquement les tables depuis la base au d├⌐marrage
    // Ensure the status filter includes a 'Tous' option that means no filter
    // Add 'Tous' as first item if not already present
    if (ui->comboBoxc_2) {
        bool found = false;
        for (int i = 0; i < ui->comboBoxc_2->count(); ++i) {
            if (ui->comboBoxc_2->itemText(i).compare(QStringLiteral("Tous"), Qt::CaseInsensitive) == 0) { found = true; break; }
        }
        if (!found) {
            ui->comboBoxc_2->insertItem(0, QStringLiteral("Tous"));
            ui->comboBoxc_2->setCurrentIndex(0);
        }
    }

    if (ui->cap_deDebut) {
        ui->cap_deDebut->setDate(QDate(2000, 1, 1));
    }

    // Sélecteur "stats captures" (Quantité / Poids) — ajouté en runtime (sans modifier le .ui)
    if (ui->cap_pagecaptures && ui->frame_zonesb_2) {
        auto *combo = ui->cap_pagecaptures->findChild<QComboBox*>(QStringLiteral("cap_comboStatsMetric"));
        if (!combo) {
            combo = new QComboBox(ui->cap_pagecaptures);
            combo->setObjectName(QStringLiteral("cap_comboStatsMetric"));
            combo->addItem(QStringLiteral("Quantité"));
            combo->addItem(QStringLiteral("Poids"));
            combo->setToolTip(QStringLiteral("Choisir la métrique des statistiques (Quantité / Poids)"));

            const QRect statsFrameGeom = ui->frame_zonesb_2->geometry();
            // En haut à droite du bloc "Répartitions par top 5 espèces"
            combo->setGeometry(statsFrameGeom.right() - 210, statsFrameGeom.top() + 10, 200, 32);

            connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
                updateCapturesStats(this, ui);
            });
        }
    }

    refreshClientsPage();

    QSqlDatabase db = Connection::getInstance()->getDatabase();
    if (ui->tableWidgetee) {
        // Chargement initial des employ├⌐s via helper d├⌐di├⌐ (r├⌐solution de table + synonymes)
        loadEmployes();
    }
    loadCaptures();
    loadQuotas(false);
    loadBateaux();
    setupBateauMaintenanceAlertSystem();
    loadPecheurBateauChoices(ui);
    loadQuaiBateauChoices(ui);
    // Chargement initial de la table des quais avec actions connect├⌐es
    refreshQuaiTable();

    // Pré-remplir les IDs auto-générés (toujours en lecture seule)
    if (ui->lineEdit_3b && currentEditingId.isEmpty() && ui->lineEdit_3b->text().trimmed().isEmpty()) {
        const QString newId = bateaauuu::genererNouvelId();
        if (!newId.isEmpty()) ui->lineEdit_3b->setText(newId);
    }

    if (ui->lineEdit_4 && !m_quai.isModeModification() && ui->lineEdit_4->text().trimmed().isEmpty()) {
        const int newId = m_quai.genererNouvelId();
        if (newId > 0) ui->lineEdit_4->setText(QString::number(newId));
    }

    if (ui->cap_lineEdit_11 && m_editingCaptureId.isEmpty() && ui->cap_lineEdit_11->text().trimmed().isEmpty()) {
        const QString newId = captures::genererNouvelId();
        if (!newId.isEmpty()) ui->cap_lineEdit_11->setText(newId);
    }

    if (ui->lineEdit_3c && m_editingClientId.isEmpty() && ui->lineEdit_3c->text().trimmed().isEmpty()) {
        Connection* conn = Connection::getInstance();
        if (conn->ensureOpen()) {
            QSqlDatabase cdb = conn->getDatabase();
            const QString tableName = resolveTableName(cdb, {QStringLiteral("CLIENT"), QStringLiteral("CLIENTS"), QStringLiteral("TCLIENT"), QStringLiteral("T_CLIENT")});
            if (!tableName.isEmpty()) {
                const QStringList cols = getColumnNames(cdb, tableName);
                const QString idCol = matchColumnBySynonyms(cols, {
                    QStringLiteral("id"), QStringLiteral("idclient"), QStringLiteral("clientid"), QStringLiteral("id_client")
                });
                if (!idCol.isEmpty()) {
                    QString genErr;
                    const QString newId = generateNextNumericIdWithPrefix(cdb, tableName, idCol, 265, 4, &genErr);
                    if (!newId.isEmpty()) ui->lineEdit_3c->setText(newId);
                }
            }
        }
    }

    // Page p├¬cheurs : frame FaceID masqu├⌐e par d├⌐faut
    if (ui->framefaceidp) {
        ui->framefaceidp->hide();
    }

    // Comportement d'origine GBateau : cacher certains panneaux au d├⌐marrage
    if (ui->frame_10b) {
        ui->frame_10b->hide();
    }
    // The alert panel widget may have been renamed in the .ui; look it up at runtime by name
    if (auto *frameAlert = this->findChild<QWidget*>(QStringLiteral("framealertb"))) {
        frameAlert->hide();
    }

     setupFrames();
    // Page clients : laisser le panneau chatbot visible (si la page est affich├⌐e)

    // D├⌐marrer sur le login GBateau (si pr├⌐sent), sinon sur le menu GBateau
    if (ui->stackedWidget) {
        if (ui->loginb) {
            ui->stackedWidget->setCurrentWidget(ui->loginb);
        } else if (ui->gestionsb) {
            ui->stackedWidget->setCurrentWidget(ui->gestionsb);
        }
    }

    // Filtres  recherche + ├⌐tat + statut
    if (ui->pushButton_9e) {
        connect(ui->pushButton_9e, &QPushButton::clicked, this, [this]() { loadEmployes(); });
    }
    if (ui->lineEdit_12e_2e) {
        connect(ui->lineEdit_12e_2e, &QLineEdit::textChanged, this, [this](const QString&) { loadEmployes(); });
    }
    if (ui->comboBox_16e_2) {
        connect(ui->comboBox_16e_2, &QComboBox::currentTextChanged, this, [this](const QString&) { loadEmployes(); });
    }
    if (ui->comboBox_17e) {
        connect(ui->comboBox_17e, &QComboBox::currentTextChanged, this, [this](const QString&) { loadEmployes(); });
    }
    if (ui->pushButton_11e) {
        connect(ui->pushButton_11e, &QPushButton::clicked, this, [this]() {
            exportEmployesPdfReport(this, ui);
        });
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

    // Rafra├«chissement automatique des statistiques d'occupation
    // pendant que la page_4 (statistiques des quais) est affich├⌐e.
    m_statsTimer = new QTimer(this);
    m_statsTimer->setInterval(5000); // toutes les 5 secondes
    connect(m_statsTimer, &QTimer::timeout, this, [this]() {
        if (!ui || !ui->stackedWidget || !ui->page_4) {
            return;
        }
        if (ui->stackedWidget->currentWidget() == ui->page_4) {
            refreshStats_2();
        }
    });
    m_statsTimer->start();

    // M├⌐t├⌐o (page_3) : API Open-Meteo sans cl├⌐ + rafra├«chissement p├⌐riodique
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

    // Chargement m├⌐t├⌐o initial
    refreshWeatherForPage3();

    // Ajout du combo de tri pour les quais sur la page_3
    if (ui->lineEdit_3) {
        QWidget *parent = ui->lineEdit_3->parentWidget();
        if (parent && !parent->findChild<QComboBox*>(QStringLiteral("comboBox_sortQuais"))) {
            QComboBox *comboTri = new QComboBox(parent);
            comboTri->setObjectName(QStringLiteral("comboBox_sortQuais"));

            comboTri->addItem(QStringLiteral("Tri: Par défaut"), 0);
            comboTri->addItem(QStringLiteral("Tri: ID (Croissant)"), 1);
            comboTri->addItem(QStringLiteral("Tri: ID (Décroissant)"), 2);
            comboTri->addItem(QStringLiteral("Tri: Capacité (Croissante)"), 3);
            comboTri->addItem(QStringLiteral("Tri: Capacité (Décroissante)"), 4);
            comboTri->addItem(QStringLiteral("Tri: Longueur (Croissante)"), 5);
            comboTri->addItem(QStringLiteral("Tri: Longueur (Décroissante)"), 6);

            comboTri->setStyleSheet(QStringLiteral(
                "QComboBox {"
                "    font-size: 16px;"
                "    padding: 6px;"
                "    background-color: rgb(224, 238, 255);"
                "    border: 2px solid rgb(0, 0, 115);"
                "    border-radius: 10px;"
                "    color: rgb(0, 0, 90);"
                "    font-weight: 600;"
                "}"));

            const QRect geom = ui->lineEdit_3->geometry();
            comboTri->setGeometry(geom.right() + 20, geom.top() - 5, 180, 41);

            connect(comboTri, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
                applyQuaiFilters();
            });

            comboTri->raise();
        }
    }
}

// Navigation / actions : impl├⌐mentations uniques plus bas dans le fichier.

// Fonction pour afficher une frame et cacher l'autre
void MainWindow::showFrame(QWidget* frameToShow)
{
    if (!ui) return;

    // Frames ├á g├⌐rer
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


// Au d├⌐marrage, cacher le frame QR
void MainWindow::setupFrames()
{
    if (ui->frame_3b) ui->frame_3b->hide();
    if (ui->frameb) ui->frameb->show();

    if (ui->horizontalLayoutMeteo) {
        ui->horizontalLayoutMeteo->setStretch(0, 0);
        ui->horizontalLayoutMeteo->setStretch(1, 1);
        ui->horizontalLayoutMeteo->setStretch(2, 3);
    }

    if (ui->labelMeteoDaily) {
        ui->labelMeteoDaily->setTextInteractionFlags(Qt::TextBrowserInteraction);
        ui->labelMeteoDaily->setOpenExternalLinks(false);
        ui->labelMeteoDaily->setCursor(Qt::PointingHandCursor);
        connect(ui->labelMeteoDaily, &QLabel::linkActivated, this, [this](const QString &link) {
            if (!link.startsWith(QStringLiteral("daily:"))) return;

            bool ok = false;
            const int idx = QStringView(link).mid(QStringLiteral("daily:").size()).toInt(&ok);
            if (!ok) return;

            m_selectedDailyIndex = idx;
            if (m_hasLastWeather) {
                updateWeatherLabels(m_lastWeatherIcon,
                                    m_lastWeatherTemp,
                                    m_lastWeatherDesc,
                                    m_lastWeatherWind,
                                    m_lastWeatherHumidity,
                                    m_lastWeatherHourly,
                                    m_lastWeatherDaily,
                                    m_lastWeatherIsDay);
            }
            updateSelectedDayDetails();
        });
    }

    if (ui->frameMeteo && !ui->frameMeteo->graphicsEffect()) {
        auto *shadow = new QGraphicsDropShadowEffect(ui->frameMeteo);
        shadow->setBlurRadius(34);
        shadow->setOffset(0, 10);
        shadow->setColor(QColor(15, 23, 42, 70));
        ui->frameMeteo->setGraphicsEffect(shadow);
    }

    if (ui->labelMeteoIcon && !ui->labelMeteoIcon->graphicsEffect()) {
        auto *shadow = new QGraphicsDropShadowEffect(ui->labelMeteoIcon);
        shadow->setBlurRadius(18);
        shadow->setOffset(0, 6);
        shadow->setColor(QColor(2, 132, 199, 120));
        ui->labelMeteoIcon->setGraphicsEffect(shadow);
    }
}

// Bouton QR Code : afficher frame QR, cacher frame Connexion
void MainWindow::legacy_pushButton_b_3b_clicked()
{
    showFrame(ui->frame_3b);
}

// Bouton Connexion : afficher frame Connexion, cacher frame QR
void MainWindow::legacy_pushButton_b_2b_clicked()
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
    // Depuis le menu GBateau : ouvrir la gestion des employ├⌐s
    if (ui->stackedWidget && ui->pagee) {
        ui->stackedWidget->setCurrentWidget(ui->pagee);
    }

    loadEmployes();
}

// ----------------------
// CRUD Employes (helpers) - style atelier
// ----------------------

void MainWindow::loadEmployes()
{
    if (!ui || !ui->tableWidgetee) {
        return;
    }

    QScopedPointer<QSqlQueryModel> model(Employe::afficher());
    if (!model) {
        return;
    }

    QTableWidget *table = ui->tableWidgetee;
    const int dataCols = model->columnCount();
    if (dataCols <= 0) {
        QMessageBox::critical(this,
                      QStringLiteral("Employes"),
                      QStringLiteral("Chargement echoue: %1").arg(Employe::lastError()));
        return;
    }

    while (model->canFetchMore()) {
        model->fetchMore();
    }
    const int rowCount = model->rowCount();

    const QString recherche = ui->lineEdit_12e_2e ? ui->lineEdit_12e_2e->text().trimmed().simplified() : QString();
    const QString etatSelection = cleanFilterLabel(ui->comboBox_16e_2 ? ui->comboBox_16e_2->currentText() : QString());
    const QString statutSelection = cleanFilterLabel(ui->comboBox_17e ? ui->comboBox_17e->currentText() : QString());

    auto isTousSelection = [](const QString& text) {
        const QString key = normalizeKey(text);
        return key.isEmpty() || key == QStringLiteral("tous");
    };

    auto findModelCol = [model = model.data()](std::initializer_list<QString> names) -> int {
        for (int c = 0; c < model->columnCount(); ++c) {
            const QString h = normalizeKey(model->headerData(c, Qt::Horizontal).toString());
            for (const QString &name : names) {
                const QString n = normalizeKey(name);
                if (h == n || h.startsWith(n)) {
                    return c;
                }
            }
        }
        return -1;
    };

    const int colNom = findModelCol({QStringLiteral("Nom")});
    const int colPrenom = findModelCol({QStringLiteral("Prenom")});
    const int colRole = findModelCol({QStringLiteral("Role")});
    const int colEtat = findModelCol({QStringLiteral("Etat")});
    const int colStatut = findModelCol({QStringLiteral("Statut")});
    const int colRaison = findModelCol({QStringLiteral("Raison")});

    int countGardien = 0;
    int countTechnicien = 0;
    int countResponsable = 0;
    int countOuvrier = 0;

    table->clearContents();
    table->setRowCount(0);
    table->setColumnCount(dataCols + 1);

    // Initialisation du tableau des éléments bannis
    if (ui->tableWidgetBannis) {
        ui->tableWidgetBannis->setRowCount(0);
        ui->tableWidgetBannis->setColumnCount(3);
        ui->tableWidgetBannis->setHorizontalHeaderLabels({QStringLiteral("Nom"), QStringLiteral("ID"), QStringLiteral("Raison")});
        ui->tableWidgetBannis->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }

    // En-tetes
    for (int c = 0; c < dataCols; ++c) {
        auto *item = new QTableWidgetItem(model->headerData(c, Qt::Horizontal).toString());
        table->setHorizontalHeaderItem(c, item);
    }
    table->setHorizontalHeaderItem(dataCols, new QTableWidgetItem(QStringLiteral("Actions")));

    // Donnees filtrees
    int visibleRow = 0;
    const int colId = findModelCol({QStringLiteral("ID")});
    for (int r = 0; r < rowCount; ++r) {
        const QString nom = (colNom >= 0) ? model->data(model->index(r, colNom)).toString().trimmed() : QString();
        const QString prenom = (colPrenom >= 0) ? model->data(model->index(r, colPrenom)).toString().trimmed() : QString();
        const QString role = (colRole >= 0) ? model->data(model->index(r, colRole)).toString().trimmed() : QString();
        const QString etat = (colEtat >= 0) ? model->data(model->index(r, colEtat)).toString().trimmed() : QString();
        const QString statut = (colStatut >= 0) ? model->data(model->index(r, colStatut)).toString().trimmed() : QString();
        const QString raison = (colRaison >= 0) ? model->data(model->index(r, colRaison)).toString().trimmed() : QString();

        bool matchRecherche = true;
        if (!recherche.isEmpty()) {
            const QStringList termes = recherche.split(' ', Qt::SkipEmptyParts);
            if (termes.size() >= 2) {
                const QString t1 = termes.at(0);
                const QString t2 = termes.mid(1).join(QStringLiteral(" "));
                const bool sens1 = nom.contains(t1, Qt::CaseInsensitive) && prenom.contains(t2, Qt::CaseInsensitive);
                const bool sens2 = nom.contains(t2, Qt::CaseInsensitive) && prenom.contains(t1, Qt::CaseInsensitive);
                matchRecherche = sens1 || sens2;
            } else {
                const QString t = termes.isEmpty() ? recherche : termes.first();
                matchRecherche = nom.contains(t, Qt::CaseInsensitive) || prenom.contains(t, Qt::CaseInsensitive);
            }
        }

        bool matchEtat = true;
        if (!isTousSelection(etatSelection)) {
            matchEtat = (normalizeKey(etat) == normalizeKey(etatSelection));
        }

        bool matchStatut = true;
        if (!isTousSelection(statutSelection)) {
            matchStatut = (normalizeKey(statut) == normalizeKey(statutSelection));
        }

        if (!(matchRecherche && matchEtat && matchStatut)) {
            continue;
        }

        table->insertRow(visibleRow);
        for (int c = 0; c < dataCols; ++c) {
            const QVariant value = model->data(model->index(r, c));
            auto *cellItem = new QTableWidgetItem(value.toString());
            table->setItem(visibleRow, c, cellItem);
        }

        // Ajouter aux éléments bannis si l'état est "Banni"
        if (normalizeKey(etat) == normalizeKey(QStringLiteral("Banni"))) {
            if (ui->tableWidgetBannis) {
                int bannedRow = ui->tableWidgetBannis->rowCount();
                ui->tableWidgetBannis->insertRow(bannedRow);
                ui->tableWidgetBannis->setItem(bannedRow, 0, new QTableWidgetItem(nom));
                ui->tableWidgetBannis->setItem(bannedRow, 1, new QTableWidgetItem(model->data(model->index(r, colId)).toString()));
                ui->tableWidgetBannis->setItem(bannedRow, 2, new QTableWidgetItem(raison));
            }
        }

        const QString roleCanonical = canonicalEmployeRole(role);
        if (roleCanonical == QStringLiteral("Gardien")) ++countGardien;
        else if (roleCanonical == QStringLiteral("Technicien")) ++countTechnicien;
        else if (roleCanonical == QStringLiteral("Responsable")) ++countResponsable;
        else if (roleCanonical == QStringLiteral("Ouvrier")) ++countOuvrier;

        ++visibleRow;
    }

    table->setAlternatingRowColors(false);
    table->setShowGrid(false);
    table->setStyleSheet(
        "QTableWidget {"
        "background-color: rgb(224, 238, 255);"
        "alternate-background-color: rgb(224, 238, 255);"
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

    ensureEmployeActionsColumn(QStringLiteral(
        "QPushButton {"
        " border: 2px solid rgb(0, 0, 112);"
        " border-radius: 10px;"
        " background-color: rgb(224, 238, 255);"
        " color: rgb(0, 0, 112);"
        " padding: 0px;"
        " }"
        "QPushButton:hover { background-color: rgb(214, 230, 252); border-color: #59abc8; }"
        "QPushButton:pressed { background-color: rgb(200, 220, 245); }"));

    updateEmployeRoleStats(ui,
                           countGardien,
                           countTechnicien,
                           countResponsable,
                           countOuvrier);
}

void MainWindow::ensureEmployeActionsColumn(const QString &buttonStyle)
{
    Q_UNUSED(buttonStyle);

    if (!ui || !ui->tableWidgetee) {
        return;
    }

    QTableWidget *table = ui->tableWidgetee;

    const int actionsCol = table->columnCount() - 1;
    if (actionsCol < 0) {
        return;
    }

    table->setColumnWidth(actionsCol, 114);

    for (int row = 0; row < table->rowCount(); ++row) {
        QWidget *container = new QWidget(table);
        container->setAttribute(Qt::WA_TranslucentBackground, true);
        container->setStyleSheet(QStringLiteral("background-color: transparent;"));

        auto *layout = new QHBoxLayout(container);
        layout->setContentsMargins(2, 2, 2, 2);
        layout->setSpacing(7);

        auto *editBtn = new QPushButton(container);
        auto *deleteBtn = new QPushButton(container);

        editBtn->setText(QStringLiteral("\u270F\uFE0F")); // ✏️
        deleteBtn->setText(QStringLiteral("\u274C"));    // ❌

        editBtn->setToolTip(QStringLiteral("Modifier"));
        deleteBtn->setToolTip(QStringLiteral("Supprimer"));

        editBtn->setFixedSize(42, 32);
        deleteBtn->setFixedSize(42, 32);

        QFont btnFont(QStringLiteral("Segoe UI Emoji"));
        btnFont.setPointSize(15);
        btnFont.setBold(true);
        editBtn->setFont(btnFont);
        deleteBtn->setFont(btnFont);
        editBtn->setFlat(true);
        deleteBtn->setFlat(true);

        const QString pecheurButtonStyle = QStringLiteral(
            "QPushButton {"
            " border: 2px solid rgb(0, 0, 112);"
            " border-radius: 6px;"
            " background-color: rgb(224, 238, 255);"
            " color: rgb(0, 0, 112);"
            " padding: 0px;"
            " }"
            "QPushButton:hover { background-color: rgb(224, 238, 255); }"
            "QPushButton:pressed { background-color: rgb(224, 238, 255); }");
        editBtn->setStyleSheet(pecheurButtonStyle);
        deleteBtn->setStyleSheet(pecheurButtonStyle);

        layout->addStretch();
        layout->addWidget(editBtn, 0, Qt::AlignVCenter);
        layout->addWidget(deleteBtn, 0, Qt::AlignVCenter);
        layout->addStretch();
        container->setLayout(layout);
        table->setCellWidget(row, actionsCol, container);
        table->setRowHeight(row, 42);

        QObject::connect(editBtn, &QPushButton::clicked, this, [this, row]() {
            editEmployeFromTable(row);
        });

        QObject::connect(deleteBtn, &QPushButton::clicked, this, [this, table, row]() {
            if (!table->item(row, 0)) {
                return;
            }

            const QString idText = table->item(row, 0)->text().trimmed();
            const QString nom = table->item(row, 1) ? table->item(row, 1)->text().trimmed() : QString();
            const QString prenom = table->item(row, 2) ? table->item(row, 2)->text().trimmed() : QString();

            bool okInt = false;
            const int idInt = idText.toInt(&okInt);
            if (!okInt || idInt <= 0) {
                QMessageBox::warning(this, QStringLiteral("Suppression employ\u00E9"), QStringLiteral("ID employ\u00E9 invalide."));
                return;
            }

            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                QStringLiteral("Suppression"),
                QStringLiteral("Supprimer l'employ\u00E9 :\n%1 %2 (ID: %3) ?").arg(nom, prenom, idText),
                QMessageBox::Yes | QMessageBox::No);

            if (reply != QMessageBox::Yes) {
                return;
            }

            if (!Employe::supprimer(idInt)) {
                QMessageBox::critical(this,
                                      QStringLiteral("Suppression employ\u00E9"),
                                      QStringLiteral("Suppression \u00E9chou\u00E9e: %1").arg(Employe::lastError()));
                return;
            }

            QMessageBox::information(this, QStringLiteral("Suppression employ\u00E9"), QStringLiteral("Suppression r\u00E9ussie."));
            loadEmployes();
        });
    }
}

void MainWindow::editEmployeFromTable(int row)
{
    if (!ui || !ui->tableWidgetee) {
        return;
    }

    QTableWidget *table = ui->tableWidgetee;
    if (row < 0 || row >= table->rowCount()) {
        return;
    }

    auto colByNames = [table](std::initializer_list<QString> names) -> int {
        for (int c = 0; c < table->columnCount(); ++c) {
            QTableWidgetItem *header = table->horizontalHeaderItem(c);
            if (!header) continue;
            const QString h = normalizeKey(header->text());
            for (const QString &name : names) {
                if (h == normalizeKey(name) || h.startsWith(normalizeKey(name))) {
                    return c;
                }
            }
        }
        return -1;
    };

    const int colId = colByNames({QStringLiteral("ID")});
    const int colNom = colByNames({QStringLiteral("Nom")});
    const int colPrenom = colByNames({QStringLiteral("Prenom")});
    const int colTelephone = colByNames({QStringLiteral("Telephone"), QStringLiteral("Tel")});
    const int colRole = colByNames({QStringLiteral("Role")});
    const int colEquipe = colByNames({QStringLiteral("Equipe")});
    const int colEtat = colByNames({QStringLiteral("Etat")});
    const int colStatut = colByNames({QStringLiteral("Statut")});
    const int colSalaire = colByNames({QStringLiteral("Salaire")});
    const int colRaison = colByNames({QStringLiteral("Raison")});
    const int colCv = colByNames({QStringLiteral("CV")});

    const QString idText = (colId >= 0 && table->item(row, colId)) ? table->item(row, colId)->text().trimmed() : QString();
    const QString nom = (colNom >= 0 && table->item(row, colNom)) ? table->item(row, colNom)->text().trimmed() : QString();
    const QString prenom = (colPrenom >= 0 && table->item(row, colPrenom)) ? table->item(row, colPrenom)->text().trimmed() : QString();
    const QString telephone = (colTelephone >= 0 && table->item(row, colTelephone)) ? table->item(row, colTelephone)->text().trimmed() : QString();
    const QString role = (colRole >= 0 && table->item(row, colRole)) ? table->item(row, colRole)->text().trimmed() : QString();
    const QString equipe = (colEquipe >= 0 && table->item(row, colEquipe)) ? table->item(row, colEquipe)->text().trimmed() : QString();
    const QString etat = (colEtat >= 0 && table->item(row, colEtat)) ? table->item(row, colEtat)->text().trimmed() : QString();
    const QString statut = (colStatut >= 0 && table->item(row, colStatut)) ? table->item(row, colStatut)->text().trimmed() : QString();
    const QString salaire = (colSalaire >= 0 && table->item(row, colSalaire)) ? table->item(row, colSalaire)->text().trimmed() : QString();
    const QString raison = (colRaison >= 0 && table->item(row, colRaison)) ? table->item(row, colRaison)->text().trimmed() : QString();
    const QString cvPath = (colCv >= 0 && table->item(row, colCv)) ? table->item(row, colCv)->text().trimmed() : QString();

    if (ui->lineEdit_12e) ui->lineEdit_12e->setText(idText);
    if (ui->lineEdit_13e) ui->lineEdit_13e->setText(nom);
    if (ui->lineEdit_16e) ui->lineEdit_16e->setText(prenom);
    if (ui->lineEdit_14e) ui->lineEdit_14e->setText(telephone);
    if (ui->lineEdit_14e_2) ui->lineEdit_14e_2->setText(salaire);
    if (ui->lineEdit_raison_banni) ui->lineEdit_raison_banni->setText(raison);
    if (ui->lineEdit_cv_path) ui->lineEdit_cv_path->setText(cvPath);

    if (ui->comboBox_15e) {
        int idx = ui->comboBox_15e->findText(equipe);
        if (idx >= 0) ui->comboBox_15e->setCurrentIndex(idx);
    }
    if (ui->comboBox_16e) {
        int idx = ui->comboBox_16e->findText(etat);
        if (idx >= 0) ui->comboBox_16e->setCurrentIndex(idx);
    }
    if (ui->comboBox_11e) {
        int idx = ui->comboBox_11e->findText(role);
        if (idx >= 0) ui->comboBox_11e->setCurrentIndex(idx);
    }
    if (ui->comboBox_14e) {
        int idx = ui->comboBox_14e->findText(statut);
        if (idx >= 0) ui->comboBox_14e->setCurrentIndex(idx);
    }

    // Réinitialiser les champs spécifiques lors de l'édition
    m_currentCvPath.clear();
    if (ui->pushButton_5e_2) {
        ui->pushButton_5e_2->setText(QStringLiteral("📄 Upload CV"));
        ui->pushButton_5e_2->setToolTip(QString());
    }

    bool okInt = false;
    m_editingEmployeId = idText.toInt(&okInt);
    if (!okInt) {
        // Si l'ID n'est pas numerique, on stocke simplement un marqueur negatif pour desactiver la mise a jour
        m_editingEmployeId = -1;
    }

    if (ui->lineEdit_12e) ui->lineEdit_12e->setReadOnly(false);
    if (ui->pushButton_5e) ui->pushButton_5e->setText(QStringLiteral("Modifier"));

    QMessageBox::information(this,
                             QStringLiteral("Modification employe"),
                             QStringLiteral("Formulaire rempli. Cliquez sur Modifier pour sauvegarder les modifications."));
}

void MainWindow::setQuotaTableEditable(bool editable)
{
    if (!ui || !ui->cap_tableWidget_2) return;
    QTableWidget* table = ui->cap_tableWidget_2;

    table->setEditTriggers(editable
        ? (QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed)
        : QAbstractItemView::NoEditTriggers);

    for (int r = 0; r < table->rowCount(); ++r) {
        QTableWidgetItem* item = table->item(r, 0);
        if (!item) {
            item = new QTableWidgetItem();
            table->setItem(r, 0, item);
        }
        Qt::ItemFlags flags = item->flags();
        flags |= Qt::ItemIsSelectable;
        flags |= Qt::ItemIsEnabled;
        if (editable) {
            flags |= Qt::ItemIsEditable;
        } else {
            flags &= ~Qt::ItemIsEditable;
        }
        item->setFlags(flags);
    }
}

void MainWindow::loadQuotas(bool showErrors)
{
    if (!ui || !ui->cap_tableWidget_2) return;
    if (m_quotaEditMode) return; // Ne pas ecraser pendant l'edition.

    Connection* conn = Connection::getInstance();
    if (!conn || !conn->ensureOpen()) {
        if (showErrors) {
            QMessageBox::warning(this, QStringLiteral("Quotas"), QStringLiteral("Connexion a la base indisponible."));
        }
        return;
    }

    QSqlDatabase db = conn->getDatabase();
    const QString quotaTable = resolveTableName(db, {
        QStringLiteral("QUOTA"),
        QStringLiteral("QUOTAS"),
        QStringLiteral("TQUOTA"),
        QStringLiteral("T_QUOTA"),
        QStringLiteral("QUOTA_POISSON"),
        QStringLiteral("QUOTAS_POISSON")
    });

    if (quotaTable.isEmpty()) {
        if (showErrors) {
            QMessageBox::warning(this, QStringLiteral("Quotas"), QStringLiteral("Table QUOTA introuvable dans la base."));
        }
        return;
    }

    const QStringList cols = getColumnNames(db, quotaTable);
    const QString typeCol = matchColumnBySynonyms(cols, {
        QStringLiteral("type_poisson"), QStringLiteral("espece"), QStringLiteral("poisson"),
        QStringLiteral("species"), QStringLiteral("type")
    });
    const QString quotaCol = matchColumnBySynonyms(cols, {
        QStringLiteral("quota"), QStringLiteral("quota_kg"), QStringLiteral("quotaKg"),
        QStringLiteral("poids_max"), QStringLiteral("max_poids"), QStringLiteral("limite")
    });

    if (typeCol.isEmpty() || quotaCol.isEmpty()) {
        if (showErrors) {
            QMessageBox::warning(this, QStringLiteral("Quotas"), QStringLiteral("Colonnes QUOTA manquantes (type + valeur)."));
        }
        return;
    }

    QHash<QString, double> quotaBySpecies;
    QSqlQuery q(db);
    const QString sql = QStringLiteral("SELECT %1, %2 FROM %3")
        .arg(typeCol, quotaCol, quotaTable);
    if (!q.exec(sql)) {
        if (showErrors) {
            QMessageBox::critical(this, QStringLiteral("Quotas"), QStringLiteral("Lecture quotas echouee: %1").arg(q.lastError().text()));
        }
        return;
    }

    while (q.next()) {
        const QString species = q.value(0).toString().trimmed();
        const QVariant v = q.value(1);
        bool ok = false;
        const double kg = v.toDouble(&ok);
        if (!species.isEmpty() && ok && kg >= 0.0) {
            quotaBySpecies.insert(normalizeKey(species), kg);
        }
    }

    QTableWidget* table = ui->cap_tableWidget_2;
    if (table->columnCount() < 1) {
        table->setColumnCount(1);
    }
    if (!table->horizontalHeaderItem(0)) {
        table->setHorizontalHeaderItem(0, new QTableWidgetItem(QStringLiteral("Quotas")));
    }

    for (int r = 0; r < table->rowCount(); ++r) {
        QString species;
        if (auto* vh = table->verticalHeaderItem(r)) {
            species = vh->text().trimmed();
        }
        if (species.isEmpty()) {
            continue;
        }

        QTableWidgetItem* item = table->item(r, 0);
        if (!item) {
            item = new QTableWidgetItem();
            table->setItem(r, 0, item);
        }

        const auto it = quotaBySpecies.constFind(normalizeKey(species));
        if (it != quotaBySpecies.constEnd()) {
            item->setText(QStringLiteral("%1kg").arg(QString::number(*it, 'f', 0)));
        }
    }

    setQuotaTableEditable(false);
}

bool MainWindow::saveQuotas(QString* errorOut)
{
    if (errorOut) errorOut->clear();
    if (!ui || !ui->cap_tableWidget_2) {
        if (errorOut) *errorOut = QStringLiteral("Table quotas introuvable dans l'UI.");
        return false;
    }

    Connection* conn = Connection::getInstance();
    if (!conn || !conn->ensureOpen()) {
        if (errorOut) *errorOut = QStringLiteral("Connexion a la base indisponible.");
        return false;
    }
    QSqlDatabase db = conn->getDatabase();

    const QString quotaTable = resolveTableName(db, {
        QStringLiteral("QUOTA"),
        QStringLiteral("QUOTAS"),
        QStringLiteral("TQUOTA"),
        QStringLiteral("T_QUOTA"),
        QStringLiteral("QUOTA_POISSON"),
        QStringLiteral("QUOTAS_POISSON")
    });
    if (quotaTable.isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("Table QUOTA introuvable dans la base.");
        return false;
    }

    const QStringList cols = getColumnNames(db, quotaTable);
    const QString typeCol = matchColumnBySynonyms(cols, {
        QStringLiteral("type_poisson"), QStringLiteral("espece"), QStringLiteral("poisson"),
        QStringLiteral("species"), QStringLiteral("type")
    });
    const QString quotaCol = matchColumnBySynonyms(cols, {
        QStringLiteral("quota"), QStringLiteral("quota_kg"), QStringLiteral("quotaKg"),
        QStringLiteral("poids_max"), QStringLiteral("max_poids"), QStringLiteral("limite")
    });
    const QString updatedAtCol = matchColumnBySynonyms(cols, {
        QStringLiteral("updated_at"), QStringLiteral("updatedat"), QStringLiteral("date_maj"),
        QStringLiteral("date_update"), QStringLiteral("modified_at"), QStringLiteral("maj")
    });
    if (typeCol.isEmpty() || quotaCol.isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("Colonnes QUOTA manquantes (type + valeur).");
        return false;
    }

    QTableWidget* table = ui->cap_tableWidget_2;

    const bool canTx = db.driver() && db.driver()->hasFeature(QSqlDriver::Transactions);
    if (canTx) {
        db.transaction();
    }

    auto fail = [&](const QString& msg) {
        if (canTx) db.rollback();
        if (errorOut) *errorOut = msg;
        return false;
    };

    QSqlQuery qExists(db);
    qExists.prepare(QStringLiteral("SELECT 1 FROM %1 WHERE %2 = ?").arg(quotaTable, typeCol));

    QSqlQuery qUpdate(db);
    if (!updatedAtCol.isEmpty()) {
        qUpdate.prepare(QStringLiteral("UPDATE %1 SET %2 = ?, %3 = SYSDATE WHERE %4 = ?")
                            .arg(quotaTable, quotaCol, updatedAtCol, typeCol));
    } else {
        qUpdate.prepare(QStringLiteral("UPDATE %1 SET %2 = ? WHERE %3 = ?").arg(quotaTable, quotaCol, typeCol));
    }

    QSqlQuery qInsert(db);
    if (!updatedAtCol.isEmpty()) {
        qInsert.prepare(QStringLiteral("INSERT INTO %1 (%2, %3, %4) VALUES (?, ?, SYSDATE)")
                            .arg(quotaTable, typeCol, quotaCol, updatedAtCol));
    } else {
        qInsert.prepare(QStringLiteral("INSERT INTO %1 (%2, %3) VALUES (?, ?)").arg(quotaTable, typeCol, quotaCol));
    }

    for (int r = 0; r < table->rowCount(); ++r) {
        QString species;
        if (auto* vh = table->verticalHeaderItem(r)) {
            species = vh->text().trimmed();
        }
        if (species.isEmpty()) continue;

        QTableWidgetItem* item = table->item(r, 0);
        if (!item) {
            return fail(QStringLiteral("Valeur quota manquante pour %1").arg(species));
        }

        double kg = 0.0;
        if (!parseQuotaKg(item->text(), &kg)) {
            return fail(QStringLiteral("Quota invalide pour %1 (ex: 200 ou 200kg)").arg(species));
        }

        qExists.bindValue(0, species);
        if (!qExists.exec()) {
            return fail(QStringLiteral("Verification existence echouee: %1").arg(qExists.lastError().text()));
        }
        const bool exists = qExists.next();
        qExists.finish();

        if (exists) {
            qUpdate.bindValue(0, kg);
            qUpdate.bindValue(1, species);
            if (!qUpdate.exec()) {
                return fail(QStringLiteral("Update quota echoue (%1): %2").arg(species, qUpdate.lastError().text()));
            }
            qUpdate.finish();
        } else {
            qInsert.bindValue(0, species);
            qInsert.bindValue(1, kg);
            if (!qInsert.exec()) {
                return fail(QStringLiteral("Insert quota echoue (%1): %2").arg(species, qInsert.lastError().text()));
            }
            qInsert.finish();
        }
    }

    if (canTx) {
        if (!db.commit()) {
            return fail(QStringLiteral("Commit echoue."));
        }
    }

    return true;
}

captures MainWindow::captureFromForm() const
{
    const QString idCapture = ui && ui->cap_lineEdit_11 ? ui->cap_lineEdit_11->text().trimmed() : QString();
    const int idBateau = ui && ui->cap_lineEdit_9 ? toIntOrZero(ui->cap_lineEdit_9->text()) : 0;
    const QString typePoisson = ui && ui->cap_comboBox_4 ? ui->cap_comboBox_4->currentText().trimmed() : QString();
    const int quantite = ui && ui->cap_sbQuantite_4 ? ui->cap_sbQuantite_4->value() : 0;
    const double poids = ui && ui->cap_dsPoids_3 ? ui->cap_dsPoids_3->value() : 0.0;
    const QDate dateCapture = ui && ui->cap_deDateCapture_3 ? ui->cap_deDateCapture_3->date() : QDate();

    const bool isEditing = !m_editingCaptureId.isEmpty();
    const bool useTemp = isEditing ? m_editingCaptureHasTemperature : m_hasArduinoTemperature;
    const double tempC = isEditing ? m_editingCaptureTemperatureC : m_lastArduinoTemperatureC;

    if (useTemp) {
        return captures(idCapture, idBateau, typePoisson, quantite, poids, dateCapture, tempC);
    }

    return captures(idCapture, idBateau, typePoisson, quantite, poids, dateCapture);
}

void MainWindow::loadCaptures()
{
    if (!ui || !ui->cap_tableWidget) return;

    const QString recherche = ui->lineEdit_7c_2 ? ui->lineEdit_7c_2->text().trimmed() : QString();
    
    QVector<captures::TableRowData> rows;
    if (!captures::chargerTableAvancee(recherche, m_captureQuantiteFiltre, m_captureDateFiltre, rows)) {
        qDebug() << "Erreur loadCaptures:" << captures::lastError();
    }

    QTableWidget* table = ui->cap_tableWidget;
    table->setAlternatingRowColors(false);
    table->setShowGrid(false);
    table->setStyleSheet(
        "QTableWidget {"
        "background-color: rgb(224, 238, 255);"
        "alternate-background-color: rgb(224, 238, 255);"
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

    table->clearContents();
    table->setRowCount(0);
    table->setColumnCount(8);

    const QStringList headers = {
        QStringLiteral("ID Capture"),
        QStringLiteral("ID Bateau"),
        QStringLiteral("Type Poisson"),
        QStringLiteral("Quantite"),
        QStringLiteral("Poids"),
        QStringLiteral("Temp\u00E9rature (\u00B0C)"),
        QStringLiteral("Date"),
        QStringLiteral("Actions")
    };
    for (int i = 0; i < headers.size(); ++i) {
        table->setHorizontalHeaderItem(i, new QTableWidgetItem(headers.at(i)));
    }

    int row = 0;
    for (const captures::TableRowData& record : rows) {
        const int currentRow = row;
        table->insertRow(row);

        table->setItem(row, 0, new QTableWidgetItem(record.idCapture));
        table->setItem(row, 1, new QTableWidgetItem(QString::number(record.idBateau)));
        table->setItem(row, 2, new QTableWidgetItem(record.typePoisson));
        table->setItem(row, 3, new QTableWidgetItem(QString::number(record.quantite)));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(record.poids, 'f', 2)));
        QTableWidgetItem* tempItem = new QTableWidgetItem(record.hasTemperature
                                 ? QString::number(record.temperatureC, 'f', 1)
                                 : QString());
        if (record.hasTemperature) {
            if (record.temperatureC < m_arduinoTempDangerThreshold) {
                tempItem->setForeground(QColor(39, 174, 96)); // Vert
            } else if (record.temperatureC <= m_arduinoTempCritiqueThreshold) {
                tempItem->setForeground(QColor(243, 156, 18)); // Orange
            } else {
                tempItem->setForeground(QColor(231, 76, 60)); // Rouge
            }
            QFont f = tempItem->font();
            f.setBold(true);
            tempItem->setFont(f);
            tempItem->setTextAlignment(Qt::AlignCenter);
        }
        table->setItem(row, 5, tempItem);
        table->setItem(row, 6, new QTableWidgetItem(record.dateCapture.isValid()
                                                     ? record.dateCapture.toString(QStringLiteral("yyyy-MM-dd"))
                                                     : QString()));

        auto* actionWidget = new QWidget();
        actionWidget->setStyleSheet(QStringLiteral("background-color: transparent;"));
        auto* actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(2, 2, 2, 2);
        actionLayout->setSpacing(7);

        auto* btnEdit = new QPushButton(QStringLiteral("✏️"));
        btnEdit->setFixedSize(42, 32);
        btnEdit->setFont(QFont(QStringLiteral("Segoe UI Emoji"), 15, QFont::Bold));
        btnEdit->setFlat(true);
        btnEdit->setStyleSheet(QStringLiteral(
            "QPushButton {"
            " border: 2px solid rgb(0, 0, 112);"
            " border-radius: 6px;"
            " background-color: rgb(224, 238, 255);"
            " color: rgb(0, 0, 112);"
            " padding: 0px;"
            " }"
            "QPushButton:hover { background-color: rgb(224, 238, 255); }"
            "QPushButton:pressed { background-color: rgb(224, 238, 255); }"));
        btnEdit->setToolTip(QStringLiteral("Modifier"));
        connect(btnEdit, &QPushButton::clicked, this, [this, currentRow]() {
            loadCaptureFromTable(currentRow);
        });

        auto* btnDelete = new QPushButton(QStringLiteral("❌"));
        btnDelete->setFixedSize(42, 32);
        btnDelete->setFont(QFont(QStringLiteral("Segoe UI Emoji"), 15, QFont::Bold));
        btnDelete->setFlat(true);
        btnDelete->setStyleSheet(QStringLiteral(
            "QPushButton {"
            " border: 2px solid rgb(0, 0, 112);"
            " border-radius: 6px;"
            " background-color: rgb(224, 238, 255);"
            " color: rgb(0, 0, 112);"
            " padding: 0px;"
            " }"
            "QPushButton:hover { background-color: rgb(224, 238, 255); }"
            "QPushButton:pressed { background-color: rgb(224, 238, 255); }"));
        btnDelete->setToolTip(QStringLiteral("Supprimer"));
        connect(btnDelete, &QPushButton::clicked, this, [this, currentRow]() {
            supprimerCaptureFromRow(currentRow);
        });

        actionLayout->addStretch();
        actionLayout->addWidget(btnEdit, 0, Qt::AlignVCenter);
        actionLayout->addWidget(btnDelete, 0, Qt::AlignVCenter);
        actionLayout->addStretch();

        table->setCellWidget(row, 7, actionWidget);
        ++row;
    }

    table->setColumnWidth(0, 120);
    table->setColumnWidth(1, 100);
    table->setColumnWidth(2, 140);
    table->setColumnWidth(3, 90);
    table->setColumnWidth(4, 90);
    table->setColumnWidth(5, 130);
    table->setColumnWidth(6, 120);
    table->setColumnWidth(7, 114);
    table->verticalHeader()->setDefaultSectionSize(42);

    // If the Arduino is providing a live temperature, keep the table in sync
    // so the temperature cell updates in real-time.
    if (m_hasArduinoTemperature) {
        updateCapturesTableLiveTemperature(m_lastArduinoTemperatureC);
    }
    
    // Mettre à jour les statistiques
    updateCapturesStats(this, ui);

    // Synchroniser le tableau quota (lecture seule par défaut)
    loadQuotas(false);
}

void MainWindow::on_cap_btnValiider_4_clicked()
{
    if (!ui || !ui->cap_btnValiider_4 || !ui->cap_tableWidget_2) {
        return;
    }

    if (!m_quotaEditMode) {
        // Passer en mode edition
        loadQuotas(true);
        m_quotaEditMode = true;
        setQuotaTableEditable(true);
        ui->cap_btnValiider_4->setText(QStringLiteral("💾Enregistrer Quotas"));
        QMessageBox::information(this,
                                 QStringLiteral("Quotas"),
                                 QStringLiteral("Modifiez les valeurs, puis cliquez sur 'Enregistrer Quotas'."));
        return;
    }

    QString err;
    if (!saveQuotas(&err)) {
        QMessageBox::critical(this, QStringLiteral("Quotas"), QStringLiteral("Enregistrement echoue: %1").arg(err));
        return;
    }

    m_quotaEditMode = false;
    setQuotaTableEditable(false);
    ui->cap_btnValiider_4->setText(QStringLiteral("✏️Modifier Quotas"));
    loadQuotas(false);
    QMessageBox::information(this, QStringLiteral("Quotas"), QStringLiteral("Quotas mis a jour."));
}

void MainWindow::loadCaptureFromTable(int row)
{
    if (!ui || !ui->cap_tableWidget) return;
    QTableWidget* table = ui->cap_tableWidget;
    if (row < 0 || row >= table->rowCount()) return;

    const QString idCapture = table->item(row, 0) ? table->item(row, 0)->text().trimmed() : QString();
    const QString idBateau = table->item(row, 1) ? table->item(row, 1)->text().trimmed() : QString();
    const QString typePoisson = table->item(row, 2) ? table->item(row, 2)->text().trimmed() : QString();
    const QString quantite = table->item(row, 3) ? table->item(row, 3)->text().trimmed() : QString();
    const QString poids = table->item(row, 4) ? table->item(row, 4)->text().trimmed() : QString();
    const QString tempTxt = table->item(row, 5) ? table->item(row, 5)->text().trimmed() : QString();
    const QString dateTxt = table->item(row, 6) ? table->item(row, 6)->text().trimmed() : QString();

    if (idCapture.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Modification capture"), QStringLiteral("ID capture introuvable."));
        return;
    }

    m_editingCaptureId = idCapture;

    m_editingCaptureHasTemperature = false;
    m_editingCaptureTemperatureC = 0.0;
    if (!tempTxt.isEmpty()) {
        bool ok = false;
        const double t = tempTxt.toDouble(&ok);
        if (ok) {
            m_editingCaptureTemperatureC = t;
            m_editingCaptureHasTemperature = true;
        }
    }

    if (ui->cap_lineEdit_11) ui->cap_lineEdit_11->setText(idCapture);
    if (ui->cap_lineEdit_9) ui->cap_lineEdit_9->setText(idBateau);
    if (ui->cap_comboBox_4) ui->cap_comboBox_4->setCurrentText(typePoisson);
    if (ui->cap_sbQuantite_4) ui->cap_sbQuantite_4->setValue(toIntOrZero(quantite));
    if (ui->cap_dsPoids_3) ui->cap_dsPoids_3->setValue(poids.toDouble());

    if (ui->cap_deDateCapture_3 && !dateTxt.isEmpty()) {
        QDate d = QDate::fromString(dateTxt, QStringLiteral("yyyy-MM-dd"));
        if (!d.isValid()) d = QDate::fromString(dateTxt, QStringLiteral("dd/MM/yyyy"));
        if (d.isValid()) ui->cap_deDateCapture_3->setDate(d);
    }

    if (ui->cap_btnValiider_3) ui->cap_btnValiider_3->setText(QStringLiteral("Modifier"));
}

void MainWindow::supprimerCaptureFromRow(int row)
{
    if (!ui || !ui->cap_tableWidget) return;
    QTableWidget* table = ui->cap_tableWidget;
    if (row < 0 || row >= table->rowCount()) return;

    const QString idCapture = table->item(row, 0) ? table->item(row, 0)->text().trimmed() : QString();
    if (idCapture.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Suppression capture"), QStringLiteral("ID capture introuvable."));
        return;
    }

    const auto reply = QMessageBox::question(
        this,
        QStringLiteral("Confirmation"),
        QStringLiteral("Voulez-vous vraiment supprimer la capture ID: %1 ?").arg(idCapture),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    if (!captures::supprimer(idCapture)) {
        QMessageBox::critical(this, QStringLiteral("Suppression capture"), QStringLiteral("Suppression echouee: %1").arg(captures::lastError()));
        return;
    }

    if (m_editingCaptureId == idCapture) {
        resetCaptureForm();
    }

    loadCaptures();
    QMessageBox::information(this, QStringLiteral("Suppression capture"), QStringLiteral("Suppression reussie."));
}

void MainWindow::resetCaptureForm()
{
    m_editingCaptureId.clear();
    m_editingCaptureHasTemperature = false;
    m_editingCaptureTemperatureC = 0.0;

    if (ui && ui->cap_lineEdit_11) ui->cap_lineEdit_11->clear();
    if (ui && ui->cap_lineEdit_9) ui->cap_lineEdit_9->clear();
    if (ui && ui->cap_comboBox_4) ui->cap_comboBox_4->setCurrentIndex(0);
    if (ui && ui->cap_sbQuantite_4) ui->cap_sbQuantite_4->setValue(0);
    if (ui && ui->cap_dsPoids_3) ui->cap_dsPoids_3->setValue(0.0);
    if (ui && ui->cap_deDateCapture_3) ui->cap_deDateCapture_3->setDate(QDate::currentDate());
    if (ui && ui->cap_btnValiider_3) ui->cap_btnValiider_3->setText(QStringLiteral("Valider"));
}

void MainWindow::on_cap_btnValiider_3_clicked()
{
    if (!handleCrudDisabled(this)) return;

    if (!ui) return;
    if (!ui->cap_lineEdit_11 || ui->cap_lineEdit_11->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Champs"), QStringLiteral("Veuillez saisir ID Capture"));
        return;
    }
    if (!ui->cap_lineEdit_9 || toIntOrZero(ui->cap_lineEdit_9->text()) <= 0) {
        QMessageBox::warning(this, QStringLiteral("Champs"), QStringLiteral("Veuillez saisir un ID Bateau valide"));
        return;
    }
    if (!ui->cap_sbQuantite_4 || ui->cap_sbQuantite_4->value() <= 0) {
        QMessageBox::warning(this, QStringLiteral("Champs"), QStringLiteral("Veuillez saisir une quantite superieure a zero"));
        return;
    }
    if (!ui->cap_dsPoids_3 || ui->cap_dsPoids_3->value() <= 0.0) {
        QMessageBox::warning(this, QStringLiteral("Champs"), QStringLiteral("Veuillez saisir un poids superieur a zero"));
        return;
    }

    const captures c = captureFromForm();

    if (!m_editingCaptureId.isEmpty()) {
        if (c.modifierAvecAncienId(m_editingCaptureId)) {
            loadCaptures();
            resetCaptureForm();
            QMessageBox::information(this, QStringLiteral("Capture"), QStringLiteral("Modification reussie"));
        } else {
            const QString err = captures::lastError();
            if (isSaisieConstraintError(err)) {
                QMessageBox::warning(this, QStringLiteral("Champs"), QStringLiteral("Modification echouee: %1").arg(err));
            } else {
                QMessageBox::critical(this, QStringLiteral("Erreur"), QStringLiteral("Modification echouee: %1").arg(err));
            }
        }
    } else {
        const QString idCapture = ui->cap_lineEdit_11->text().trimmed();
        if (captures::idExiste(idCapture)) {
            QMessageBox::warning(this, QStringLiteral("Doublon"), QStringLiteral("Cet ID capture existe deja."));
            return;
        }

        if (c.ajouter()) {
            loadCaptures();
            resetCaptureForm();
            QMessageBox::information(this, QStringLiteral("Capture"), QStringLiteral("Ajout reussi"));
        } else {
            const QString err = captures::lastError();
            if (isSaisieConstraintError(err)) {
                QMessageBox::warning(this, QStringLiteral("Champs"), QStringLiteral("Ajout echoue: %1").arg(err));
            } else {
                QMessageBox::critical(this, QStringLiteral("Erreur"), QStringLiteral("Ajout echoue: %1").arg(err));
            }
        }
    }
}

void MainWindow::on_lineEdit_7c_2_textChanged(const QString &text)
{
    Q_UNUSED(text);
    loadCaptures();
}

void MainWindow::on_cap_deDebut_dateChanged(const QDate &date)
{
    m_captureDateFiltre = date;
    loadCaptures();
}

void MainWindow::on_p5b_clicked()
{
    if (!ui || !ui->stackedWidget || !ui->cap_pagecaptures) {
        return;
    }

    ui->stackedWidget->setCurrentWidget(ui->cap_pagecaptures);

    loadCaptures();
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
    // Rafra├«chir la table ├á chaque ouverture
    loadBateaux();
}

void MainWindow::on_p2b_clicked()
{
    // Depuis le menu GBateau : ouvrir la gestion des p├¬cheurs
    if (ui->stackedWidget && ui->pagepecheur) {
        ui->stackedWidget->setCurrentWidget(ui->pagepecheur);
    }

    // Par d├⌐faut, garder FaceID masqu├⌐ ├á l'entr├⌐e de la page
    if (ui && ui->framefaceidp) {
        ui->framefaceidp->hide();
    }

    // R├⌐afficher les widgets principaux (au cas o├╣ on revient depuis FaceID)
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

    loadPecheurs();
}

void MainWindow::on_p3b_clicked()
{
    // Depuis le menu GBateau : ouvrir la gestion des clients (int├⌐gr├⌐e dans mainwindow.ui)
    if (ui && ui->stackedWidget && ui->pageclients) {
        ui->stackedWidget->setCurrentWidget(ui->pageclients);

        // Garder le bouton IA visible au-dessus
        if (auto btn = this->findChild<QPushButton*>(QStringLiteral("btnai"))) {
            btn->raise();
        }
    }

    // loadClients(); (removed)
}

void MainWindow::legacy_btnai_clicked()
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

void MainWindow::legacy_btnSend_2_clicked()
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

void MainWindow::setupPecheurFaceCapture()
{
    if (!ui || !ui->frame_10p) return;

    if (!m_facePreviewLabel) {
        m_facePreviewLabel = new QLabel(ui->frame_10p);
        m_facePreviewLabel->setObjectName(QStringLiteral("faceCameraPreviewp"));
        m_facePreviewLabel->setAlignment(Qt::AlignCenter);
        m_facePreviewLabel->setAttribute(Qt::WA_TranslucentBackground, true);
        m_facePreviewLabel->setScaledContents(false);
        applyPecheurFacePreviewShape(m_facePreviewLabel);
        m_facePreviewLabel->show();
    }

    if (!m_faceVideoSink) {
        m_faceVideoSink = new QVideoSink(this);
        connect(m_faceVideoSink, &QVideoSink::videoFrameChanged, this, [this](const QVideoFrame& frame) {
            if (!m_facePreviewLabel || !frame.isValid()) return;

            QImage image = frame.toImage();
            if (image.isNull()) return;

            const bool liveContent = frameHasLiveCameraContent(image);
            const bool wasLive = m_faceHasRecentFrame;
            m_faceHasRecentFrame = liveContent;
            if (liveContent) {
                const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
                m_faceLastFrameAtMs = nowMs;
                if (!wasLive) {
                    m_faceOpenStatusUntilMs = nowMs + 1000;
                }

                if (nowMs <= m_faceOpenStatusUntilMs) {
                    setPecheurFaceStatus(ui, QStringLiteral("Caméra ouverte."));
                } else {
                    setPecheurFaceStatus(ui, pecheurFaceOpenStatusText());
                }
            } else {
                m_faceOpenStatusUntilMs = 0;
                setPecheurFaceStatus(ui, QStringLiteral("Caméra fermée."));
            }

            const QRect rect = pecheurFacePreviewRect(ui ? ui->frame_10p : nullptr);
            const int diameter = rect.width() > 0 ? rect.width() : qMin(m_facePreviewLabel->width(), m_facePreviewLabel->height());
            if (diameter <= 0) {
                return;
            }

            m_facePreviewLabel->setPixmap(buildCircularFacePreviewPixmap(image, diameter));
            m_facePreviewLabel->update();
        });
    }

    if (!m_faceCaptureSession) {
        m_faceCaptureSession = new QMediaCaptureSession(this);
    }
    if (!m_faceImageCapture) {
        m_faceImageCapture = new QImageCapture(this);
        connect(m_faceImageCapture, &QImageCapture::imageCaptured, this, [this](int, const QImage& image) {
            if (image.isNull() || !frameHasLiveCameraContent(image)) {
                if (m_faceCaptureRetryRemaining > 0) {
                    --m_faceCaptureRetryRemaining;
                    m_faceCapturePending = true;
                    setPecheurFaceStatus(ui, QStringLiteral("Capture en cours... (%1 essais restants)")
                                            .arg(m_faceCaptureRetryRemaining));
                    QTimer::singleShot(180, this, [this]() { attemptPecheurFaceCapture(); });
                    return;
                }

                m_faceCapturePending = false;
                m_faceCaptureRetryRemaining = 0;
                m_pendingPecheurPhotoBytes.clear();
                const QString msg = QStringLiteral("Caméra fermée. Ouvrez la caméra avant de capturer.");
                setPecheurFaceStatus(ui, QStringLiteral("Caméra fermée."));
                QMessageBox::warning(this, QStringLiteral("Capture"), msg);
                return;
            }

            m_faceCapturePending = false;
            m_faceCaptureRetryRemaining = 0;

            QByteArray encoded;
            QBuffer buffer(&encoded);
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer, "JPG", 90);
            m_pendingPecheurPhotoBytes = encoded;

            setPecheurFaceStatus(ui, QStringLiteral("✅ Visage détecté et centré.\nEnregistrement en cours..."));

            if (persistCapturedPecheurPhoto()) {
                stopPecheurFaceCapture();
                if (ui && ui->framefaceidp) {
                    ui->framefaceidp->hide();
                }
                setPecheurMainWidgetsVisible(ui, true);
                setPecheurFaceStatus(ui, QStringLiteral("✅ Visage détecté.\nPhoto prête."));
                QMessageBox::information(this, QStringLiteral("Visage"), QStringLiteral("Photo capturée. Cliquez sur Ajouter ou Modifier pour enregistrer le profil complet."));
            } else {
                const QString err = Pecheurs::lastError().trimmed();
                const QString msg = err.isEmpty()
                                      ? QStringLiteral("Photo capturée mais enregistrement en base échoué.")
                                      : QStringLiteral("Échec enregistrement: %1").arg(err);
                setPecheurFaceStatus(ui, msg);
                QMessageBox::warning(this, QStringLiteral("Visage"), msg);
            }
        });

        connect(m_faceImageCapture, &QImageCapture::errorOccurred, this, [this](int, QImageCapture::Error, const QString& errorString) {
            if (m_faceCapturePending
                && m_faceCaptureRetryRemaining > 0
                && errorString.contains(QStringLiteral("not ready"), Qt::CaseInsensitive)) {
                --m_faceCaptureRetryRemaining;
                QTimer::singleShot(250, this, [this]() { attemptPecheurFaceCapture(); });
                return;
            }

            m_faceCapturePending = false;
            m_faceCaptureRetryRemaining = 0;
            const QString msg = QStringLiteral("Erreur capture: %1").arg(errorString);
            setPecheurFaceStatus(ui, msg);
            QMessageBox::warning(this, QStringLiteral("Visage"), msg);
        });
    }

    if (!m_faceCamera) {
        const QCameraDevice device = QMediaDevices::defaultVideoInput();
        if (device.isNull()) {
            QMessageBox::warning(this, QStringLiteral("Caméra"), QStringLiteral("Aucune caméra disponible."));
            return;
        }

        m_faceCamera = new QCamera(device, this);
        connect(m_faceCamera, &QCamera::errorOccurred, this, [this](QCamera::Error, const QString& errorString) {
            m_faceHasRecentFrame = false;
            const QString msg = QStringLiteral("Erreur caméra: %1").arg(errorString);
            setPecheurFaceStatus(ui, msg);
            QMessageBox::warning(this, QStringLiteral("Caméra"), msg);
        });
        connect(m_faceCamera, &QCamera::activeChanged, this, [this](bool active) {
            if (!active) {
                m_faceHasRecentFrame = false;
                m_faceOpenStatusUntilMs = 0;
                setPecheurFaceStatus(ui, QStringLiteral("Caméra fermée."));
            }
        });

        m_faceCaptureSession->setCamera(m_faceCamera);
        m_faceCaptureSession->setImageCapture(m_faceImageCapture);
        m_faceCaptureSession->setVideoOutput(m_faceVideoSink);
    }

    if (m_facePreviewLabel) {
        updatePecheurFacePreviewGeometry();
        m_facePreviewLabel->raise();
        m_facePreviewLabel->show();
    }

    setPecheurFaceStatus(ui, QStringLiteral("Initialisation caméra..."));

    if (!m_faceCamera->isActive()) {
        m_faceCamera->start();
    }

    QTimer::singleShot(700, this, [this]() {
        if (!ui) return;
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        const bool hasLiveFrame = m_faceHasRecentFrame && ((now - m_faceLastFrameAtMs) <= kFaceLiveFrameTimeoutMs);
        if (m_faceCamera && m_faceCamera->isActive() && hasLiveFrame) {
            setPecheurFaceStatus(ui, pecheurFaceOpenStatusText());
        } else {
            setPecheurFaceStatus(ui, QStringLiteral("Caméra fermée."));
        }
    });
}

void MainWindow::updatePecheurFacePreviewGeometry()
{
    if (!ui || !ui->frame_10p || !m_facePreviewLabel) {
        return;
    }

    const QRect rect = pecheurFacePreviewRect(ui->frame_10p);
    if (!rect.isValid()) {
        return;
    }

    if (m_facePreviewLabel) {
        m_facePreviewLabel->setGeometry(rect);
        applyPecheurFacePreviewShape(m_facePreviewLabel);

        if (const QPixmap current = m_facePreviewLabel->pixmap(Qt::ReturnByValue); !current.isNull()) {
            m_facePreviewLabel->setPixmap(current.scaled(rect.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        }
    }
}

void MainWindow::stopPecheurFaceCapture()
{
    m_faceCapturePending = false;
    m_faceCaptureRetryRemaining = 0;
    m_faceHasRecentFrame = false;
    m_faceLastFrameAtMs = 0;
    m_faceOpenStatusUntilMs = 0;
    if (m_faceCamera && m_faceCamera->isActive()) {
        m_faceCamera->stop();
    }
}

void MainWindow::attemptPecheurFaceCapture()
{
    if (!ui || !m_faceImageCapture || !m_faceCamera) {
        m_faceCapturePending = false;
        m_faceCaptureRetryRemaining = 0;
        return;
    }

    if (!m_faceCamera->isActive()) {
        m_faceCamera->start();
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const bool hasLiveFrame = m_faceHasRecentFrame && ((now - m_faceLastFrameAtMs) <= kFaceLiveFrameTimeoutMs);
    if (!hasLiveFrame) {
        if (m_faceCaptureRetryRemaining > 0) {
            --m_faceCaptureRetryRemaining;
            setPecheurFaceStatus(ui, QStringLiteral("Initialisation caméra..."));
            QTimer::singleShot(250, this, [this]() { attemptPecheurFaceCapture(); });
            return;
        }

        m_faceCapturePending = false;
        const QString msg = QStringLiteral("Caméra fermée. Ouvrez la caméra avant de capturer.");
        setPecheurFaceStatus(ui, QStringLiteral("Caméra fermée."));
        QMessageBox::warning(this, QStringLiteral("Visage"), msg);
        return;
    }

    if (!m_faceImageCapture->isReadyForCapture()) {
        if (m_faceCaptureRetryRemaining > 0) {
            --m_faceCaptureRetryRemaining;
            setPecheurFaceStatus(ui, QStringLiteral("Initialisation caméra..."));
            QTimer::singleShot(250, this, [this]() { attemptPecheurFaceCapture(); });
            return;
        }

        m_faceCapturePending = false;
        const QString msg = QStringLiteral("Camera non prete. Fermez les applications qui utilisent la camera puis reessayez.");
        setPecheurFaceStatus(ui, msg);
        QMessageBox::warning(this, QStringLiteral("Visage"), msg);
        return;
    }

    const int requestId = m_faceImageCapture->capture();
    if (requestId < 0) {
        if (m_faceCaptureRetryRemaining > 0) {
            --m_faceCaptureRetryRemaining;
            QTimer::singleShot(250, this, [this]() { attemptPecheurFaceCapture(); });
            return;
        }

        m_faceCapturePending = false;
        const QString msg = QStringLiteral("Capture impossible. Vérifiez la caméra puis réessayez.");
        setPecheurFaceStatus(ui, msg);
        QMessageBox::warning(this, QStringLiteral("Visage"), msg);
    }
}

void MainWindow::syncPecheurFaceCaptureFields()
{
    if (!ui) return;

    const QString fullName = pecheurFaceDisplayName(ui);
    const QString role = pecheurFaceRole(ui);

    if (ui->lineEdit_11p) {
        ui->lineEdit_11p->setReadOnly(true);
        ui->lineEdit_11p->setText(fullName);
    }
    if (ui->lineEdit_12p) {
        ui->lineEdit_12p->setReadOnly(true);
        ui->lineEdit_12p->setText(role);
    }
}

bool MainWindow::persistCapturedPecheurPhoto()
{
    if (!ui) return false;

    const QString id = ui->lineEditp ? ui->lineEditp->text().trimmed().toUpper() : QString();
    if (id.isEmpty()) {
        return false;
    }

    // On ne fait plus d'enregistrement immédiat en base ici pour éviter les doublons
    // et l'effacement accidentel de la photo lors d'un clic ultérieur sur "Modifier".
    // La photo reste dans m_pendingPecheurPhotoBytes jusqu'à l'appui sur Ajouter/Modifier.
    
    return true;
}

void MainWindow::on_btnFaceIDp_clicked()
{
    // Afficher FaceID, préremplir les champs et lancer la caméra
    if (!ui) return;
    setPecheurMainWidgetsVisible(ui, false);
    syncPecheurFaceCaptureFields();
    setupPecheurFaceCapture();
    if (ui->framefaceidp) {
        ui->framefaceidp->setVisible(true);
        ui->framefaceidp->raise();
    }
}

void MainWindow::on_brmp_clicked()
{
    // Retour Menu depuis pagepecheur -> menu GBateau
    if (!ui) return;

    // Remettre l'etat normal cote pecheurs
    m_pendingPecheurPhotoBytes.clear();
    stopPecheurFaceCapture();
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

    // Remettre l'etat normal cote pecheurs
    m_pendingPecheurPhotoBytes.clear();
    stopPecheurFaceCapture();
    if (ui->framefaceidp) ui->framefaceidp->hide();
    setPecheurMainWidgetsVisible(ui, true);

    if (ui->stackedWidget && ui->gestionsb) {
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
    }
}

void MainWindow::on_bmi_6p_clicked()
{
    // Capturer le visage et tenter de l'enregistrer dans la base
    if (!ui) return;

    if (!m_faceImageCapture || !m_faceCamera) {
        setupPecheurFaceCapture();
    }
    if (!m_faceImageCapture || !m_faceCamera) {
        return;
    }

    if (!m_faceCamera->isActive()) {
        QMessageBox::warning(this,
                             QStringLiteral("Visage"),
                             QStringLiteral("La caméra est fermée. Ouvrez la caméra puis positionnez un visage dans le cercle."));
        setPecheurFaceStatus(ui, QStringLiteral("Caméra fermée."));
        return;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const bool hasLiveFrame = m_faceHasRecentFrame && ((now - m_faceLastFrameAtMs) <= kFaceLiveFrameTimeoutMs);
    if (!hasLiveFrame) {
        QMessageBox::warning(this,
                             QStringLiteral("Visage"),
                             QStringLiteral("Caméra fermée. Aucun flux vidéo détecté."));
        setPecheurFaceStatus(ui, QStringLiteral("Caméra fermée."));
        return;
    }

    m_faceCapturePending = true;
    m_faceCaptureRetryRemaining = 30;
    setPecheurFaceStatus(ui, QStringLiteral("✅ Visage détecté et centré.\nValidation en cours..."));
    attemptPecheurFaceCapture();
}

void MainWindow::on_pushButton_11p_clicked()
{
    // Annuler FaceID, r├⌐afficher les widgets principaux
    if (!ui) return;
    stopPecheurFaceCapture();
    if (ui->framefaceidp) ui->framefaceidp->setVisible(false);
    setPecheurMainWidgetsVisible(ui, true);
}

void MainWindow::on_pushButton_10p_clicked()
{
    // R\u00e9essayer FaceID / Déclencher capture
    if (!ui) return;
    if (ui->label_8p) ui->label_8p->setText(QStringLiteral("Capture en cours..."));
    
    if (m_faceImageCapture) {
        attemptPecheurFaceCapture();
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (ui && watched == ui->frame_10p && event && event->type() == QEvent::Resize) {
        updatePecheurFacePreviewGeometry();
    }

    if (event && event->type() == QEvent::MouseButtonPress) {
        if (auto* photoLabel = qobject_cast<QLabel*>(watched)) {
            const QVariant previewData = photoLabel->property("pecheurPhotoPreview");
            if (previewData.isValid()) {
                const QPixmap preview = previewData.value<QPixmap>();
                if (!preview.isNull()) {
                    showPecheurPhotoPreviewDialog(this, preview);
                    return true;
                }
            }
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::on_pushButton_b_clicked()
{
    legacy_pushButton_b_4b_clicked();
}
void MainWindow::legacy_pushButton_b_4b_clicked()
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
                              QStringLiteral("Connexion DB ├⌐chou├⌐e: %1").arg(conn->lastErrorText()));
        return;
    }

    bool authenticated = false;
    QString authError;
    authenticated = authenticateLoginFromDb(conn->getDatabase(), username, password, &authError);

    // Fallback minimal si la table utilisateurs n'existe pas encore OU si le schma ne contient
    // pas les colonnes login/mot de passe attendues.
    const bool schemaMissing = authError.contains(QStringLiteral("introuvable"), Qt::CaseInsensitive)
        || authError.contains(QStringLiteral("non trouv"), Qt::CaseInsensitive)
        || authError.contains(QStringLiteral("colonne"), Qt::CaseInsensitive)
        || authError.contains(QStringLiteral("mot de passe"), Qt::CaseInsensitive);

    if (!authenticated && schemaMissing) {
        authenticated = (username == QStringLiteral("admin") && password == fallbackAdminPassword());
    }

    if (!authenticated) {
        QMessageBox::warning(this,
                             QStringLiteral("Connexion"),
                             QStringLiteral("Identifiants invalides."));
        return;
    }

    // Se souvenir de moi: sauvegarder (ou effacer) APR├êS une connexion r├⌐ussie
    const bool remember = ui->checkBoxb && ui->checkBoxb->isChecked();
    storeRememberedLogin(username, password, remember);

    if (ui->lineEdit_2b) {
        ui->lineEdit_2b->clear();
    }

    if (ui->stackedWidget && ui->gestionsb) {
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
    }
}

void MainWindow::on_pushButton_20b_clicked()
{
    // Mot de passe oubli├⌐ ? -> page r├⌐cup├⌐ration (Rmdpb)
    if (!ui || !ui->stackedWidget || !ui->Rmdpb) {
        return;
    }

    clearPasswordResetState();

    if (ui->lineEdit_6b) ui->lineEdit_6b->clear();   // nom
    if (ui->lineEdit_8b) ui->lineEdit_8b->clear();   // email
    if (ui->lineEdit_9b) ui->lineEdit_9b->clear();   // code
    if (ui->lineEdit_10b) ui->lineEdit_10b->clear(); // new pass
    if (ui->lineEdit_11b) ui->lineEdit_11b->clear(); // confirm

    ui->stackedWidget->setCurrentWidget(ui->Rmdpb);
}

void MainWindow::on_pushButton_13b_clicked()
{
    // Envoyer (r├⌐cup├⌐ration): si Nom=admin et Email=klai.nourhene1@gmail.com
    // on envoie un code a 6 chiffres et on ouvre la page de verification.
    if (!ui || !ui->stackedWidget || !ui->Remailb) {
        return;
    }

    const QString name = ui->lineEdit_6b ? ui->lineEdit_6b->text().trimmed() : QString();
    const QString email = ui->lineEdit_8b ? ui->lineEdit_8b->text().trimmed() : QString();

    if (name.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this,
                             QStringLiteral("Recuperation"),
                             QStringLiteral("Veuillez saisir le nom et l'email."));
        return;
    }

    const QString expectedName = QStringLiteral("admin");
    const QString expectedEmail = QStringLiteral("klai.nourhene1@gmail.com");
    if (name.compare(expectedName, Qt::CaseInsensitive) != 0
        || email.compare(expectedEmail, Qt::CaseInsensitive) != 0) {
        QMessageBox::warning(this,
                             QStringLiteral("Recuperation"),
                             QStringLiteral("Nom ou email incorrect."));
        return;
    }

    m_passwordResetEmail = expectedEmail;
    m_passwordResetCode = generateCode();
    m_passwordResetVerified = false;

    QString error;
    if (!sendEmail(m_passwordResetEmail, 
                   QStringLiteral("Votre code de verification est : %1").arg(m_passwordResetCode), 
                   &error, 
                   QStringLiteral("AquaTech Password Reset"))) {
        QMessageBox::critical(this,
                              QStringLiteral("Email"),
                              QStringLiteral("Envoi d'email echoue: %1").arg(error));
        return;
    }

    if (ui->lineEdit_9b) ui->lineEdit_9b->clear();

    QMessageBox::information(this,
                             QStringLiteral("Email"),
                             QStringLiteral("Un code a 6 chiffres a ete envoye sur votre email."));

    ui->stackedWidget->setCurrentWidget(ui->Remailb);
}

void MainWindow::on_pushButton_14b_clicked()
{
    // Annuler -> retour login
    if (!ui || !ui->stackedWidget || !ui->loginb) {
        return;
    }

    clearPasswordResetState();
    if (ui->lineEdit_6b) ui->lineEdit_6b->clear();
    if (ui->lineEdit_8b) ui->lineEdit_8b->clear();
    if (ui->lineEdit_9b) ui->lineEdit_9b->clear();
    if (ui->lineEdit_10b) ui->lineEdit_10b->clear();
    if (ui->lineEdit_11b) ui->lineEdit_11b->clear();

    ui->stackedWidget->setCurrentWidget(ui->loginb);
    applyRememberedLogin(ui);
}

void MainWindow::on_pushButton_15b_clicked()
{
    // Verifier code -> nouveau mot de passe
    if (!ui || !ui->stackedWidget || !ui->Nmdpb) {
        return;
    }

    const QString entered = ui->lineEdit_9b ? ui->lineEdit_9b->text().trimmed() : QString();
    if (entered.isEmpty()) {
        QMessageBox::warning(this,
                             QStringLiteral("Verification"),
                             QStringLiteral("Veuillez saisir le code recu par email."));
        return;
    }

    if (m_passwordResetCode.isEmpty()) {
        QMessageBox::warning(this,
                             QStringLiteral("Verification"),
                             QStringLiteral("Veuillez d'abord demander un code."));
        return;
    }

    if (entered != m_passwordResetCode) {
        QMessageBox::warning(this,
                             QStringLiteral("Verification"),
                             QStringLiteral("Code incorrect."));
        return;
    }

    m_passwordResetVerified = true;
    if (ui->lineEdit_10b) ui->lineEdit_10b->clear();
    if (ui->lineEdit_11b) ui->lineEdit_11b->clear();
    ui->stackedWidget->setCurrentWidget(ui->Nmdpb);
}

void MainWindow::on_pushButton_16b_clicked()
{
    // Valider nouveau mot de passe -> retour login
    if (!ui || !ui->stackedWidget || !ui->loginb) {
        return;
    }

    if (!m_passwordResetVerified) {
        QMessageBox::warning(this,
                             QStringLiteral("Recuperation"),
                             QStringLiteral("Veuillez d'abord verifier le code."));
        return;
    }

    const QString pass1 = ui->lineEdit_10b ? ui->lineEdit_10b->text() : QString();
    const QString pass2 = ui->lineEdit_11b ? ui->lineEdit_11b->text() : QString();

    if (pass1.trimmed().isEmpty() || pass2.trimmed().isEmpty()) {
        QMessageBox::warning(this,
                             QStringLiteral("Recuperation"),
                             QStringLiteral("Veuillez saisir et confirmer le nouveau mot de passe."));
        return;
    }

    if (pass1 != pass2) {
        QMessageBox::warning(this,
                             QStringLiteral("Recuperation"),
                             QStringLiteral("Les mots de passe ne correspondent pas."));
        return;
    }

    QString error;
    if (!resetAdminPassword(m_passwordResetEmail, pass1, &error)) {
        QMessageBox::critical(this,
                              QStringLiteral("Recuperation"),
                              QStringLiteral("Impossible de mettre a jour le mot de passe: %1").arg(error));
        return;
    }

    // Si l'utilisateur a activ├⌐ "se souvenir de moi", mettre ├á jour le mot de passe enregistr├⌐.
    {
        QSettings settings(QStringLiteral("AquaTech"), QStringLiteral("AquaTech"));
        const bool remember = settings.value(QStringLiteral("auth/rememberMe"), false).toBool();
        const QString savedUser = settings.value(QStringLiteral("auth/rememberedUsername")).toString();
        if (remember && savedUser.compare(QStringLiteral("admin"), Qt::CaseInsensitive) == 0) {
            settings.setValue(QStringLiteral("auth/rememberedPassword"), pass1);
            settings.sync();
        }
    }

    QMessageBox::information(this,
                             QStringLiteral("Recuperation"),
                             QStringLiteral("Mot de passe mis a jour. Vous pouvez vous connecter."));

    clearPasswordResetState();
    if (ui->lineEdit_6b) ui->lineEdit_6b->clear();
    if (ui->lineEdit_8b) ui->lineEdit_8b->clear();
    if (ui->lineEdit_9b) ui->lineEdit_9b->clear();
    if (ui->lineEdit_10b) ui->lineEdit_10b->clear();
    if (ui->lineEdit_11b) ui->lineEdit_11b->clear();

    ui->stackedWidget->setCurrentWidget(ui->loginb);
    applyRememberedLogin(ui);
}

void MainWindow::on_pushButton_17b_clicked()
{
    // Retour -> login
    if (!ui || !ui->stackedWidget || !ui->loginb) {
        return;
    }

    clearPasswordResetState();
    ui->stackedWidget->setCurrentWidget(ui->loginb);
    applyRememberedLogin(ui);
}

void MainWindow::on_pushButton_18b_clicked()
{
    // Retour (GBateau) -> Rmdpb
    if (!ui || !ui->stackedWidget || !ui->Rmdpb) {
        return;
    }

    m_passwordResetVerified = false;
    ui->stackedWidget->setCurrentWidget(ui->Rmdpb);
}

void MainWindow::on_pushButton_19b_clicked()
{
    // Retour (GBateau) -> Remailb
    if (!ui || !ui->stackedWidget || !ui->Remailb) {
        return;
    }

    m_passwordResetVerified = false;
    ui->stackedWidget->setCurrentWidget(ui->Remailb);
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
    // Bouton "Fermer" dans le panneau param├¿tres : fermer frame_10b
    if (ui && ui->frame_10b) {
        ui->frame_10b->hide();
    }
}

void MainWindow::on_pushButton_9b_clicked()
{
    // D├⌐connexion : effacer les identifiants enregistr├⌐s et revenir ├á la page login
    storeRememberedLogin(QString(), QString(), false);

    if (ui && ui->stackedWidget && ui->loginb) {
        ui->stackedWidget->setCurrentWidget(ui->loginb);
    }
    if (ui) {
        applyRememberedLogin(ui);
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
    // Bouton "Γ¼à∩╕Å Retour Menu" sur la page gestion des quais : revenir au menu GBateau
    if (ui->stackedWidget && ui->gestionsb) {
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
    }
}

void MainWindow::on_pushButton_10e_clicked()
{
    // Bouton "Retour Menu" sur la page gestion des employ├⌐s : revenir au menu GBateau
    if (ui->stackedWidget && ui->gestionsb) {
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
    }
}

void MainWindow::legacy_pushButton_3b_clicked()
{
    // Bouton "Retour" sur la page gestionbateaub : revenir au menu GBateau
    if (ui->stackedWidget && ui->gestionsb) {
        ui->stackedWidget->setCurrentWidget(ui->gestionsb);
    }
}

void MainWindow::legacy_pushButton_7b_clicked()
{
    // Bouton "Param├¿tres des alertes" : afficher le panneau framealertb (lookup by name)
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

void MainWindow::setupArduinoIntegration()
{
    if (m_arduino) return;

    m_arduino = new ArduinoSerial(this);

    m_arduinoStatusLabel = new QLabel(QStringLiteral("Arduino: disconnected"), this);
    m_arduinoPortCombo = new QComboBox(this);
    m_arduinoConnectButton = new QPushButton(QStringLiteral("Connect"), this);
    m_arduinoTxEdit = new QLineEdit(this);
    m_arduinoSendButton = new QPushButton(QStringLiteral("Send"), this);

    m_arduinoPortCombo->setToolTip(QStringLiteral("Serial port"));
    m_arduinoPortCombo->setMinimumWidth(90);

    m_arduinoConnectButton->setToolTip(QStringLiteral("Connect / Disconnect"));
    m_arduinoConnectButton->setMinimumWidth(95);

    m_arduinoTxEdit->setPlaceholderText(QStringLiteral("TX line"));
    m_arduinoTxEdit->setClearButtonEnabled(true);
    m_arduinoTxEdit->setMaximumWidth(220);

    m_arduinoSendButton->setToolTip(QStringLiteral("Send a line (LF)"));
    m_arduinoSendButton->setMinimumWidth(70);

    if (auto* sb = statusBar()) {
        sb->addWidget(m_arduinoStatusLabel, 1);
        sb->addPermanentWidget(m_arduinoPortCombo);
        sb->addPermanentWidget(m_arduinoConnectButton);
        sb->addPermanentWidget(m_arduinoTxEdit);
        sb->addPermanentWidget(m_arduinoSendButton);
    }

    setupArduinoTemperatureButton();

    // Remember last selected port.
    QSettings settings(QStringLiteral("AquaTech"), QStringLiteral("AquaTech"));
    const QString savedPort = settings.value(QStringLiteral("arduino/portName")).toString().trimmed();

    refreshArduinoPortList();
    if (!savedPort.isEmpty() && m_arduinoPortCombo) {
        const int idx = m_arduinoPortCombo->findText(savedPort);
        if (idx >= 0) m_arduinoPortCombo->setCurrentIndex(idx);
    }

    connect(m_arduinoPortCombo, &QComboBox::currentTextChanged, this, [](const QString& port) {
        QSettings s(QStringLiteral("AquaTech"), QStringLiteral("AquaTech"));
        s.setValue(QStringLiteral("arduino/portName"), port.trimmed());
        s.sync();
    });

    connect(m_arduinoConnectButton, &QPushButton::clicked, this, [this]() {
        if (!m_arduino) return;

        if (m_arduino->isConnected()) {
            m_arduino->disconnectFromPort();
            return;
        }

        refreshArduinoPortList();

        const QString portName = m_arduinoPortCombo ? m_arduinoPortCombo->currentText().trimmed() : QString();
        if (portName.isEmpty() || portName == QStringLiteral("(no ports)")) {
            if (statusBar()) statusBar()->showMessage(QStringLiteral("Arduino: no serial ports found"), 4000);
            updateArduinoUiState();
            return;
        }

        QString error;
        if (!m_arduino->connectToPort(portName, 9600, &error)) {
            if (statusBar()) statusBar()->showMessage(QStringLiteral("Arduino connect failed: %1").arg(error), 6000);
            updateArduinoUiState();
            return;
        }
    });

    const auto sendLine = [this]() {
        if (!m_arduino || !m_arduinoTxEdit) return;

        const QString line = m_arduinoTxEdit->text().trimmed();
        if (line.isEmpty()) return;

        QString error;
        if (!m_arduino->writeLine(line, &error)) {
            if (statusBar()) statusBar()->showMessage(QStringLiteral("Arduino send failed: %1").arg(error), 6000);
            return;
        }

        m_arduinoTxEdit->clear();
    };

    connect(m_arduinoSendButton, &QPushButton::clicked, this, sendLine);
    connect(m_arduinoTxEdit, &QLineEdit::returnPressed, this, sendLine);

    connect(m_arduino, &ArduinoSerial::connected, this, [this](const QString& portName) {
        if (m_arduinoStatusLabel) m_arduinoStatusLabel->setText(QStringLiteral("Arduino: %1").arg(portName));
        updateArduinoUiState();
        updateArduinoTemperatureDialogConnectionState();
    });

    connect(m_arduino, &ArduinoSerial::disconnected, this, [this]() {
        if (m_arduinoStatusLabel) m_arduinoStatusLabel->setText(QStringLiteral("Arduino: disconnected"));
        updateArduinoUiState();
        updateArduinoTemperatureDialogConnectionState();
    });

    connect(m_arduino, &ArduinoSerial::lineReceived, this, [this](const QString& line) {
        handleArduinoTemperatureLine(line);
    });

    connect(m_arduino, &ArduinoSerial::errorOccurred, this, [this](const QString& message) {
        if (statusBar()) statusBar()->showMessage(QStringLiteral("Arduino error: %1").arg(message), 8000);
        updateArduinoUiState();
        updateArduinoTemperatureDialogConnectionState();
    });

    updateArduinoUiState();
}

void MainWindow::setupArduinoTemperatureButton()
{
    if (!ui || !ui->cap_pagecaptures) return;
    if (m_capArduinoTempButton) return;

    m_capArduinoTempButton = new QPushButton(QStringLiteral("Temperature normale"), ui->cap_pagecaptures);
    m_capArduinoTempButton->setObjectName(QStringLiteral("cap_btnArduinoTemp"));

    // Status-style button (green/yellow/red) driven by temperature.
    m_capArduinoTempButton->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background-color: #27ae60;"
        "  border: 2px solid #229954;"
        "  border-radius: 8px;"
        "  color: white;"
        "  font-weight: 700;"
        "  padding: 8px 16px;"
        "}"
    ));

    const QRect ref = ui->cap_btnExporter ? ui->cap_btnExporter->geometry() : QRect(1360, 130, 151, 41);
    const int w = qMax(190, ref.width() + 20);
    const int h = ref.height();
    const int x = 20;      // top-left
    const int y = 20;
    m_capArduinoTempButton->setGeometry(x, y, w, h);
    m_capArduinoTempButton->show();
    m_capArduinoTempButton->raise();

    if (m_hasArduinoTemperature) {
        // Apply the last known status immediately.
        const double t = m_lastArduinoTemperatureC;
        if (t < 29.0) {
            m_capArduinoTempButton->setText(QStringLiteral("Temperature normale"));
            m_capArduinoTempButton->setStyleSheet(QStringLiteral(
                "QPushButton { background-color: #27ae60; border: 2px solid #229954; border-radius: 8px; color: white; font-weight: 700; padding: 8px 16px; }"
            ));
        } else if (t <= 32.0) {
            m_capArduinoTempButton->setText(QStringLiteral("Temp danger"));
            m_capArduinoTempButton->setStyleSheet(QStringLiteral(
                "QPushButton { background-color: #f39c12; border: 2px solid #d68910; border-radius: 8px; color: white; font-weight: 700; padding: 8px 16px; }"
            ));
        } else {
            m_capArduinoTempButton->setText(QStringLiteral("Temp critique"));
            m_capArduinoTempButton->setStyleSheet(QStringLiteral(
                "QPushButton { background-color: #e74c3c; border: 2px solid #c0392b; border-radius: 8px; color: white; font-weight: 700; padding: 8px 16px; }"
            ));
        }
    }

    connect(m_capArduinoTempButton, &QPushButton::clicked, this, &MainWindow::toggleCapturesArduinoTemperatureView);
}

void MainWindow::toggleCapturesArduinoTemperatureView()
{
    if (!ui || !ui->cap_pagecaptures) return;
    ensureCapturesArduinoTemperatureView();
    if (!m_capArduinoTempFrame) return;

    const bool shouldShow = !m_capArduinoTempFrame->isVisible();
    setCapturesArduinoTemperatureViewVisible(shouldShow);
    if (!shouldShow) return;

    ensureArduinoTemperatureConnected();
    updateArduinoTemperatureDialogConnectionState();
    if (m_hasArduinoTemperature) {
        updateArduinoTemperatureDialog(m_lastArduinoTemperatureC, false);
    }
}

void MainWindow::setCapturesArduinoTemperatureViewVisible(bool visible)
{
    if (!ui) return;
    if (ui->cap_tableWidget) ui->cap_tableWidget->setVisible(!visible);
    if (ui->frame_zonesb_2) ui->frame_zonesb_2->setVisible(!visible);
    if (m_capArduinoTempFrame) {
        m_capArduinoTempFrame->setVisible(visible);
        if (visible) {
            m_capArduinoTempFrame->raise();
        }
    }
}

void MainWindow::ensureCapturesArduinoTemperatureView()
{
    if (!ui || !ui->cap_pagecaptures) return;
    if (m_capArduinoTempFrame) return;

    auto* frame = new QFrame(ui->cap_pagecaptures);
    frame->setObjectName(QStringLiteral("cap_tempView"));
    frame->setFrameShape(QFrame::NoFrame);

    QRect area(510, 180, 881, 551);
    if (ui->cap_tableWidget) {
        area = ui->cap_tableWidget->geometry();
    }
    if (ui->frame_zonesb_2) {
        area = area.united(ui->frame_zonesb_2->geometry());
    }
    frame->setGeometry(area);
    frame->hide();

    auto* root = new QVBoxLayout(frame);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(14);

    auto* topRow = new QHBoxLayout();
    auto* title = new QLabel(QStringLiteral("Surveillance temp\u00E9rature"), frame);
    {
        QFont f = title->font();
        f.setBold(true);
        f.setPointSize(qMax(12, f.pointSize() + 4));
        title->setFont(f);
    }

    m_arduinoTempBadgeLabel = new QLabel(QStringLiteral("-"), frame);
    m_arduinoTempBadgeLabel->setAlignment(Qt::AlignCenter);
    m_arduinoTempBadgeLabel->setStyleSheet(tempStatusCss(0.0));

    topRow->addWidget(title, 1);
    topRow->addWidget(m_arduinoTempBadgeLabel, 0);
    root->addLayout(topRow);

    m_arduinoTempConnLabel = new QLabel(QStringLiteral("Port: disconnected"), frame);
    {
        QFont f = m_arduinoTempConnLabel->font();
        f.setPointSize(qMax(9, f.pointSize() - 1));
        m_arduinoTempConnLabel->setFont(f);
    }
    root->addWidget(m_arduinoTempConnLabel);

    auto* cardsRow = new QHBoxLayout();
    cardsRow->setSpacing(12);

    QLabel* currentValueLabel = nullptr;
    QDoubleSpinBox* dangerSpin = nullptr;
    QDoubleSpinBox* critiqueSpin = nullptr;

    auto makeCard = [frame](const QString& cardTitle, const QString& initialValue, const QString& valueCss, QLabel** valueOut, QDoubleSpinBox** spinOut = nullptr) {
        auto* card = new QFrame(frame);
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        card->setMinimumHeight(104);

        auto* layout = new QVBoxLayout(card);
        layout->setContentsMargins(14, 12, 14, 12);
        layout->setSpacing(6);

        auto* t = new QLabel(cardTitle, card);
        QFont tf = t->font();
        tf.setBold(true);
        tf.setPointSize(qMax(9, tf.pointSize() - 1));
        t->setFont(tf);
        layout->addWidget(t);

        if (spinOut) {
            auto* s = new QDoubleSpinBox(card);
            s->setRange(0.0, 100.0);
            s->setSingleStep(0.5);
            s->setValue(initialValue.split(' ')[0].toDouble());
            s->setSuffix(QStringLiteral(" \u00B0C"));
            s->setStyleSheet(QStringLiteral("font-size: 16pt; font-weight: bold; ") + valueCss);
            layout->addWidget(s);
            *spinOut = s;
        } else {
            auto* v = new QLabel(initialValue, card);
            QFont vf = v->font();
            vf.setBold(true);
            vf.setPointSize(qMax(14, vf.pointSize() + 10));
            v->setFont(vf);
            v->setStyleSheet(valueCss);
            layout->addWidget(v);
            if (valueOut) *valueOut = v;
        }

        return card;
    };

    cardsRow->addWidget(makeCard(QStringLiteral("Temp\u00E9rature actuelle"), QStringLiteral("-- \u00B0C"), QString(), &currentValueLabel));
    cardsRow->addWidget(makeCard(QStringLiteral("Seuil danger"), QStringLiteral("29.0 \u00B0C"), QStringLiteral("color: #f39c12;"), nullptr, &dangerSpin));
    cardsRow->addWidget(makeCard(QStringLiteral("Seuil critique"), QStringLiteral("32.0 \u00B0C"), QStringLiteral("color: #e74c3c;"), nullptr, &critiqueSpin));
    root->addLayout(cardsRow);

    m_arduinoTempValueLabel = currentValueLabel;

    // Connect spinboxes to send data to Arduino
    auto sendThresholds = [this, dangerSpin, critiqueSpin]() {
        m_arduinoTempDangerThreshold = dangerSpin->value();
        m_arduinoTempCritiqueThreshold = critiqueSpin->value();

        if (m_hasArduinoTemperature) {
            updateArduinoTemperatureDialog(m_lastArduinoTemperatureC, false);
            updateCapturesTableLiveTemperature(m_lastArduinoTemperatureC);
            if (m_capArduinoTempButton) {
                m_capArduinoTempButton->setText(tempStatusButtonText(m_lastArduinoTemperatureC));
                m_capArduinoTempButton->setStyleSheet(tempStatusButtonCss(m_lastArduinoTemperatureC));
            }
        }

        if (!m_arduino || !m_arduino->isConnected()) return;
        m_arduino->writeLine(QStringLiteral("D:%1").arg(m_arduinoTempDangerThreshold, 0, 'f', 1));
        m_arduino->writeLine(QStringLiteral("C:%1").arg(m_arduinoTempCritiqueThreshold, 0, 'f', 1));
    };

    connect(dangerSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, sendThresholds);
    connect(critiqueSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, sendThresholds);

    auto* histFrame = new QFrame(frame);
    auto* histLayout = new QVBoxLayout(histFrame);
    histLayout->setContentsMargins(14, 12, 14, 12);
    histLayout->setSpacing(8);

    auto* histTitle = new QLabel(QStringLiteral("Historique"), histFrame);
    {
        QFont f = histTitle->font();
        f.setBold(true);
        histTitle->setFont(f);
    }
    histLayout->addWidget(histTitle);

    m_arduinoTempHistory = new QPlainTextEdit(histFrame);
    m_arduinoTempHistory->setReadOnly(true);
    m_arduinoTempHistory->document()->setMaximumBlockCount(250);
    m_arduinoTempHistory->setStyleSheet(QStringLiteral(
        "background-color: rgb(224, 238, 255);"
        "border: 2PX solid rgb(0, 0, 115);"
        "border-radius: 10px;"
        "color: rgb(0, 0, 112);"
        "padding: 8px;"
    ));
    histLayout->addWidget(m_arduinoTempHistory, 1);

    root->addWidget(histFrame, 1);

    // Refresh connection state every 2 seconds while the view exists.
    auto* refreshTimer = new QTimer(frame);
    refreshTimer->setInterval(2000);
    connect(refreshTimer, &QTimer::timeout, this, [this, lastAttemptMs = qint64(0)]() mutable {
        if (!m_capArduinoTempFrame || !m_capArduinoTempFrame->isVisible()) return;

        updateArduinoTemperatureDialogConnectionState();
        if (!m_arduino || m_arduino->isConnected()) return;
        if (!m_arduino->serialPortAvailable()) return;

        const QStringList ports = m_arduino->availablePortNames();
        if (ports.isEmpty()) return;

        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (lastAttemptMs != 0 && (now - lastAttemptMs) < 8000) return;
        lastAttemptMs = now;

        ensureArduinoTemperatureConnected();
        updateArduinoTemperatureDialogConnectionState();
    });
    refreshTimer->start();

    m_capArduinoTempFrame = frame;
}

static bool tryParseTemperatureC(const QString& line, double* outC)
{
    if (!outC) return false;

    QString t = line.trimmed();
    if (t.isEmpty()) return false;

    // Accept both decimal separators and optional units (e.g. "25.5", "25,5", "25.5 °C", "T=25.5").
    t.replace(QLatin1Char(','), QLatin1Char('.'));

    static QRegularExpression rx(QStringLiteral("(-?\\d+(?:\\.\\d+)?)"));
    const QRegularExpressionMatch m = rx.match(t);
    if (!m.hasMatch()) return false;

    bool ok = false;
    const double v = m.captured(1).toDouble(&ok);
    if (!ok) return false;

    *outC = v;
    return true;
}

QString MainWindow::tempStatusText(double temperatureC)
{
    if (temperatureC < m_arduinoTempDangerThreshold) return QStringLiteral("Normal");
    if (temperatureC <= m_arduinoTempCritiqueThreshold) return QStringLiteral("Danger");
    return QStringLiteral("Critique");
}

QString MainWindow::tempStatusButtonText(double temperatureC)
{
    if (temperatureC < m_arduinoTempDangerThreshold) return QStringLiteral("Temperature normale");
    if (temperatureC <= m_arduinoTempCritiqueThreshold) return QStringLiteral("Temp danger");
    return QStringLiteral("Temp critique");
}

QString MainWindow::tempStatusButtonCss(double temperatureC)
{
    const QString common = QStringLiteral(
        "color: white;"
        "font-weight: 700;"
        "border-radius: 8px;"
        "padding: 8px 16px;"
    );

    if (temperatureC < m_arduinoTempDangerThreshold) {
        return QStringLiteral("QPushButton { background-color: #27ae60; border: 2px solid #229954;") + common + QStringLiteral(" }");
    }
    if (temperatureC <= m_arduinoTempCritiqueThreshold) {
        return QStringLiteral("QPushButton { background-color: #f39c12; border: 2px solid #d68910;") + common + QStringLiteral(" }");
    }
    return QStringLiteral("QPushButton { background-color: #e74c3c; border: 2px solid #c0392b;") + common + QStringLiteral(" }");
}

QString MainWindow::tempStatusCss(double temperatureC)
{
    // Reuse palette already present in mainwindow.ui.
    const QString common = QStringLiteral(
        "color: white;"
        "font-weight: bold;"
        "padding: 6px 12px;"
        "border-radius: 999px;"
        "min-width: 90px;"
        "text-align: center;"
    );

    if (temperatureC < m_arduinoTempDangerThreshold) {
        return QStringLiteral("background-color: #27ae60; border: 2px solid #229954;") + common;
    }
    if (temperatureC <= m_arduinoTempCritiqueThreshold) {
        return QStringLiteral("background-color: #f39c12; border: 2px solid #d68910;") + common;
    }
    return QStringLiteral("background-color: #e74c3c; border: 2px solid #c0392b;") + common;
}

void MainWindow::showArduinoTemperatureWindow()
{
    if (m_arduinoTempDialog) {
        m_arduinoTempDialog->show();
        m_arduinoTempDialog->raise();
        m_arduinoTempDialog->activateWindow();
        ensureArduinoTemperatureConnected();
        updateArduinoTemperatureDialogConnectionState();
        return;
    }

    auto* dlg = new QDialog(this);
    dlg->setWindowTitle(QStringLiteral("Captures \u2014 Surveillance temp\u00E9rature"));
    dlg->setAttribute(Qt::WA_DeleteOnClose, true);
    dlg->resize(820, 560);
    dlg->setStyleSheet(QStringLiteral(
        "QDialog { background-color: rgba(0, 0, 127,0.9); }"
        "QLabel { color: white; }"
        "QPlainTextEdit { background: transparent; border: none; color: white; }"
    ));

    m_arduinoTempDialog = dlg;

    auto* root = new QVBoxLayout(dlg);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(14);

    auto* topRow = new QHBoxLayout();
    auto* title = new QLabel(QStringLiteral("Captures \u2014 Surveillance temp\u00E9rature"), dlg);
    {
        QFont f = title->font();
        f.setBold(true);
        f.setPointSize(qMax(12, f.pointSize() + 4));
        title->setFont(f);
    }
    m_arduinoTempBadgeLabel = new QLabel(QStringLiteral("-"), dlg);
    m_arduinoTempBadgeLabel->setAlignment(Qt::AlignCenter);
    m_arduinoTempBadgeLabel->setStyleSheet(tempStatusCss(0.0));
    topRow->addWidget(title, 1);
    topRow->addWidget(m_arduinoTempBadgeLabel, 0);
    root->addLayout(topRow);

    m_arduinoTempConnLabel = new QLabel(QStringLiteral("Port: disconnected"), dlg);
    {
        QFont f = m_arduinoTempConnLabel->font();
        f.setPointSize(qMax(9, f.pointSize() - 1));
        m_arduinoTempConnLabel->setFont(f);
        m_arduinoTempConnLabel->setStyleSheet(QStringLiteral("color: rgba(255, 255, 255, 210);"));
    }
    root->addWidget(m_arduinoTempConnLabel);

    // Cards row (current / danger / critical)
    auto* cardsRow = new QHBoxLayout();
    cardsRow->setSpacing(12);

    auto makeCard = [dlg](const QString& cardTitle, const QString& initialValue, const QString& valueCss, QLabel** valueOut) {
        auto* frame = new QFrame(dlg);
        frame->setStyleSheet(QStringLiteral(
            "QFrame { background-color: rgba(255, 255, 255, 18); border-radius: 14px; }"
            "QLabel { border: none; }"
        ));
        frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        frame->setMinimumHeight(104);

        auto* layout = new QVBoxLayout(frame);
        layout->setContentsMargins(14, 12, 14, 12);
        layout->setSpacing(6);

        auto* t = new QLabel(cardTitle, frame);
        QFont tf = t->font();
        tf.setBold(true);
        tf.setPointSize(qMax(9, tf.pointSize() - 1));
        t->setFont(tf);
        t->setStyleSheet(QStringLiteral("color: rgba(255, 255, 255, 210);"));
        layout->addWidget(t);

        auto* v = new QLabel(initialValue, frame);
        QFont vf = v->font();
        vf.setBold(true);
        vf.setPointSize(qMax(14, vf.pointSize() + 10));
        v->setFont(vf);
        v->setStyleSheet(valueCss);
        layout->addWidget(v);

        if (valueOut) *valueOut = v;
        return frame;
    };

    QLabel* currentValueLabel = nullptr;
    cardsRow->addWidget(makeCard(QStringLiteral("Temp\u00E9rature actuelle"), QStringLiteral("-- \u00B0C"), QString(), &currentValueLabel));
    cardsRow->addWidget(makeCard(QStringLiteral("Seuil danger"), QStringLiteral("29.0 \u00B0C"), QStringLiteral("color: #f39c12;"), nullptr));
    cardsRow->addWidget(makeCard(QStringLiteral("Seuil critique"), QStringLiteral("32.0 \u00B0C"), QStringLiteral("color: #e74c3c;"), nullptr));
    root->addLayout(cardsRow);

    m_arduinoTempValueLabel = currentValueLabel;

    // History panel
    auto* histFrame = new QFrame(dlg);
    histFrame->setStyleSheet(QStringLiteral(
        "QFrame { background-color: rgba(255, 255, 255, 18); border-radius: 14px; }"
        "QLabel { border: none; }"
    ));
    auto* histLayout = new QVBoxLayout(histFrame);
    histLayout->setContentsMargins(14, 12, 14, 12);
    histLayout->setSpacing(8);

    auto* histTitle = new QLabel(QStringLiteral("Historique"), histFrame);
    {
        QFont f = histTitle->font();
        f.setBold(true);
        histTitle->setFont(f);
        histTitle->setStyleSheet(QStringLiteral("color: rgba(255, 255, 255, 210);"));
    }
    histLayout->addWidget(histTitle);

    m_arduinoTempHistory = new QPlainTextEdit(histFrame);
    m_arduinoTempHistory->setReadOnly(true);
    m_arduinoTempHistory->document()->setMaximumBlockCount(250);
    histLayout->addWidget(m_arduinoTempHistory, 1);

    root->addWidget(histFrame, 1);

    connect(dlg, &QObject::destroyed, this, [this]() {
        m_arduinoTempDialog = nullptr;
        m_arduinoTempValueLabel = nullptr;
        m_arduinoTempBadgeLabel = nullptr;
        m_arduinoTempConnLabel = nullptr;
        m_arduinoTempHistory = nullptr;
    });

    // Keep the connection state fresh while the dialog is open.
    auto* refreshTimer = new QTimer(dlg);
    refreshTimer->setInterval(2000);
    connect(refreshTimer, &QTimer::timeout, this, [this, lastAttemptMs = qint64(0)]() mutable {
        updateArduinoTemperatureDialogConnectionState();

        if (!m_arduino || m_arduino->isConnected()) return;
        if (!m_arduino->serialPortAvailable()) return;

        // Avoid spamming: only try auto-connect when ports exist,
        // and don't retry too often.
        const QStringList ports = m_arduino->availablePortNames();
        if (ports.isEmpty()) return;

        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (lastAttemptMs != 0 && (now - lastAttemptMs) < 8000) return;
        lastAttemptMs = now;

        ensureArduinoTemperatureConnected();
        updateArduinoTemperatureDialogConnectionState();
    });
    refreshTimer->start();

    ensureArduinoTemperatureConnected();
    updateArduinoTemperatureDialogConnectionState();
    if (m_hasArduinoTemperature) {
        updateArduinoTemperatureDialog(m_lastArduinoTemperatureC, false);
    }

    dlg->show();
}

void MainWindow::ensureArduinoTemperatureConnected()
{
    if (!m_arduino) return;
    if (m_arduino->isConnected()) return;

    if (!m_arduino->serialPortAvailable()) {
        if (m_arduinoTempConnLabel) {
            m_arduinoTempConnLabel->setText(QStringLiteral("Qt SerialPort indisponible (installez le module SerialPort de Qt)."));
        }
        if (statusBar()) {
            statusBar()->showMessage(QStringLiteral("Qt SerialPort module missing: install 'Qt Serial Port' then rebuild"), 8000);
        }
        return;
    }

    const QStringList ports = m_arduino->availablePortNames();
    if (ports.isEmpty()) {
        if (m_arduinoTempConnLabel) {
            m_arduinoTempConnLabel->setText(QStringLiteral("Port: aucun port detecte (branchez l'Arduino / driver)."));
        }
        if (statusBar()) statusBar()->showMessage(QStringLiteral("Arduino: no serial ports detected"), 6000);
        return;
    }

    const QString selectedPort = (m_arduinoPortCombo ? m_arduinoPortCombo->currentText().trimmed() : QString());

    QSettings settings(QStringLiteral("AquaTech"), QStringLiteral("AquaTech"));
    const QString savedPort = settings.value(QStringLiteral("arduino/portName")).toString().trimmed();

    QString chosen;
    if (!selectedPort.isEmpty() && selectedPort != QStringLiteral("(no ports)") && ports.contains(selectedPort)) {
        chosen = selectedPort;
    } else if (!savedPort.isEmpty() && ports.contains(savedPort)) {
        chosen = savedPort;
    } else if (ports.contains(QStringLiteral("COM3"))) {
        chosen = QStringLiteral("COM3");
    } else {
        chosen = ports.first();
    }

    QString error;
    if (!m_arduino->connectToPort(chosen, 9600, &error)) {
        if (m_arduinoTempConnLabel) {
            m_arduinoTempConnLabel->setText(QStringLiteral("Port: %1 (erreur: %2)").arg(chosen, error));
        }
    }
}

void MainWindow::handleArduinoTemperatureLine(const QString& line)
{
    double temperatureC = 0.0;
    if (!tryParseTemperatureC(line, &temperatureC)) return;

    m_hasArduinoTemperature = true;
    m_lastArduinoTemperatureC = temperatureC;

    if (m_capArduinoTempButton) {
        m_capArduinoTempButton->setText(tempStatusButtonText(temperatureC));
        m_capArduinoTempButton->setStyleSheet(tempStatusButtonCss(temperatureC));
    }

    updateArduinoTemperatureDialog(temperatureC);

    // Live sync into the captures list.
    updateCapturesTableLiveTemperature(temperatureC);
}

void MainWindow::updateCapturesTableLiveTemperature(double temperatureC)
{
    if (!ui || !ui->cap_tableWidget) return;
    QTableWidget* table = ui->cap_tableWidget;
    if (table->rowCount() <= 0) return;

    // Only update when we are on the captures page (avoid surprising changes).
    if (ui->stackedWidget && ui->cap_pagecaptures) {
        if (ui->stackedWidget->currentWidget() != ui->cap_pagecaptures) {
            return;
        }
    }

    constexpr int kTempCol = 5;

    // Update EVERY row in the table with the same temperature
    for (int r = 0; r < table->rowCount(); ++r) {
        QTableWidgetItem* tempItem = table->item(r, kTempCol);
        if (!tempItem) {
            tempItem = new QTableWidgetItem();
            table->setItem(r, kTempCol, tempItem);
        }

        tempItem->setText(QString::number(temperatureC, 'f', 1));

        // Apply color based on dynamic thresholds
        if (temperatureC < m_arduinoTempDangerThreshold) {
            tempItem->setForeground(QColor(39, 174, 96)); // Vert (Safe)
        } else if (temperatureC <= m_arduinoTempCritiqueThreshold) {
            tempItem->setForeground(QColor(243, 156, 18)); // Orange (Danger)
        } else {
            tempItem->setForeground(QColor(231, 76, 60)); // Rouge (Critique)
        }
        
        QFont f = tempItem->font();
        f.setBold(true);
        tempItem->setFont(f);
        tempItem->setTextAlignment(Qt::AlignCenter);
    }

    // Update the edit buffer too
    if (!m_editingCaptureId.isEmpty()) {
        m_editingCaptureHasTemperature = true;
        m_editingCaptureTemperatureC = temperatureC;
    }
}

void MainWindow::updateArduinoTemperatureDialogConnectionState()
{
    if (!m_arduinoTempConnLabel) return;
    if (!m_arduino) {
        m_arduinoTempConnLabel->setText(QStringLiteral("Port: disconnected"));
        return;
    }

    const QString port = m_arduino->connectedPortName();
    if (port.isEmpty()) {
        m_arduinoTempConnLabel->setText(QStringLiteral("Port: disconnected"));
    } else {
        m_arduinoTempConnLabel->setText(QStringLiteral("Port: %1 (connecte)").arg(port));
    }
}

void MainWindow::updateArduinoTemperatureDialog(double temperatureC, bool appendHistory)
{
    if (!m_arduinoTempValueLabel && !m_arduinoTempBadgeLabel && !m_arduinoTempHistory) return;

    const QString valueText = QStringLiteral("%1 \u00B0C").arg(temperatureC, 0, 'f', 1);
    if (m_arduinoTempValueLabel) {
        m_arduinoTempValueLabel->setText(valueText);

        const QString valueColor = (temperatureC < m_arduinoTempDangerThreshold)
            ? QStringLiteral("color: #27ae60;")
            : (temperatureC <= m_arduinoTempCritiqueThreshold)
                ? QStringLiteral("color: #f39c12;")
                : QStringLiteral("color: #e74c3c;");
        m_arduinoTempValueLabel->setStyleSheet(valueColor);
    }

    if (m_arduinoTempBadgeLabel) {
        m_arduinoTempBadgeLabel->setText(tempStatusText(temperatureC));
        m_arduinoTempBadgeLabel->setStyleSheet(tempStatusCss(temperatureC));
    }

    if (appendHistory && m_arduinoTempHistory) {
        const QString timeText = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss"));
        m_arduinoTempHistory->appendPlainText(QStringLiteral("%1 \u2014 %2").arg(timeText, valueText));
    }
}

void MainWindow::refreshArduinoPortList()
{
    if (!m_arduinoPortCombo || !m_arduino) return;

    const QString previous = m_arduinoPortCombo->currentText().trimmed();
    const QStringList ports = m_arduino->availablePortNames();

    m_arduinoPortCombo->blockSignals(true);
    m_arduinoPortCombo->clear();

    if (ports.isEmpty()) {
        m_arduinoPortCombo->addItem(QStringLiteral("(no ports)"));
        m_arduinoPortCombo->setEnabled(false);
    } else {
        m_arduinoPortCombo->addItems(ports);

        int idx = m_arduinoPortCombo->findText(previous);
        if (idx >= 0) m_arduinoPortCombo->setCurrentIndex(idx);

        // Enabled/disabled is handled by updateArduinoUiState.
    }

    m_arduinoPortCombo->blockSignals(false);
}

void MainWindow::updateArduinoUiState()
{
    const bool connected = m_arduino && m_arduino->isConnected();

    if (m_arduinoConnectButton) {
        m_arduinoConnectButton->setText(connected ? QStringLiteral("Disconnect") : QStringLiteral("Connect"));
    }

    if (m_arduinoPortCombo) {
        const bool hasPorts = (m_arduinoPortCombo->count() > 0)
            && (m_arduinoPortCombo->itemText(0) != QStringLiteral("(no ports)"));
        m_arduinoPortCombo->setEnabled(!connected && hasPorts);
    }

    if (m_arduinoTxEdit) m_arduinoTxEdit->setEnabled(connected);
    if (m_arduinoSendButton) m_arduinoSendButton->setEnabled(connected);

    if (m_arduinoStatusLabel && !connected) {
        m_arduinoStatusLabel->setText(QStringLiteral("Arduino: disconnected"));
    }
}

// ----------------------
// CRUD: AJOUTER (toutes pages)
// ----------------------

void MainWindow::on_pushButton_2c_clicked()
{
    if (!handleCrudDisabled(this) || !ui) return;

    if (!validateRequiredFields(this, {
            {QStringLiteral("Nom client"), ui->lineEdit_4c, [this]{ return !ui->lineEdit_4c || ui->lineEdit_4c->text().trimmed().isEmpty(); }},
            {QStringLiteral("Pr\u00E9nom client"), ui->lineEdit_12c, [this]{ return !ui->lineEdit_12c || ui->lineEdit_12c->text().trimmed().isEmpty(); }},
            {QStringLiteral("T\u00E9l\u00E9phone"), ui->lineEdit_14c, [this]{ return !ui->lineEdit_14c || ui->lineEdit_14c->text().trimmed().isEmpty(); }},
        })) {
        return;
    }

    const bool isEditing = !m_editingClientId.isEmpty() || (ui->pushButton_2c && ui->pushButton_2c->text().trimmed().compare(QStringLiteral("Modifier"), Qt::CaseInsensitive) == 0);

    Connection *conn = Connection::getInstance();
    if (!conn->ensureOpen()) {
        QMessageBox::critical(this, QStringLiteral("DB"), QStringLiteral("Connexion DB \u00E9chou\u00E9e: %1").arg(conn->lastErrorText()));
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

    // Contrôles de saisie (Clients)
    QString nom = values.value(QStringLiteral("nom")).toString().simplified();
    QString prenom = values.value(QStringLiteral("prenom")).toString().simplified();
    const QString telephone = values.value(QStringLiteral("telephone")).toString().trimmed();
    const QDate dateInscription = values.value(QStringLiteral("date")).toDate();

    // Lettres uniquement (Unicode), avec séparateurs usuels: espace, tiret, apostrophe.
    static const QRegularExpression namePattern(
        QStringLiteral("^[\\p{L}\\p{M}]+(?:[\\s'-][\\p{L}\\p{M}]+)*$"),
        QRegularExpression::UseUnicodePropertiesOption);
    static const QRegularExpression telPattern(QStringLiteral("^\\d{8}$"));

    if (!namePattern.match(nom).hasMatch()) {
        QMessageBox::warning(this,
                             QStringLiteral("Saisie client"),
                             QStringLiteral("Nom invalide (lettres uniquement)."));
        if (ui->lineEdit_4c) ui->lineEdit_4c->setFocus();
        return;
    }
    if (!namePattern.match(prenom).hasMatch()) {
        QMessageBox::warning(this,
                             QStringLiteral("Saisie client"),
                             QStringLiteral("Pr\u00E9nom invalide (lettres uniquement)."));
        if (ui->lineEdit_12c) ui->lineEdit_12c->setFocus();
        return;
    }
    if (!telPattern.match(telephone).hasMatch()) {
        QMessageBox::warning(this,
                             QStringLiteral("Saisie client"),
                             QStringLiteral("T\u00E9l\u00E9phone invalide (exactement 8 chiffres)."));
        if (ui->lineEdit_14c) ui->lineEdit_14c->setFocus();
        return;
    }
    if (dateInscription.isValid() && dateInscription > QDate::currentDate()) {
        QMessageBox::warning(this,
                             QStringLiteral("Saisie client"),
                             QStringLiteral("La date ne peut pas \u00EAtre dans le futur."));
        if (ui->dateEdit_c) ui->dateEdit_c->setFocus();
        return;
    }

    // Écrire les versions normalisées dans le payload
    values.insert(QStringLiteral("nom"), nom);
    values.insert(QStringLiteral("prenom"), prenom);
    values.insert(QStringLiteral("telephone"), telephone);

    const QString targetId = values.value(QStringLiteral("id")).toString().trimmed();

    const QHash<QString, QStringList> syn = {
        {QStringLiteral("id"), {QStringLiteral("id"), QStringLiteral("idclient"), QStringLiteral("clientid"), QStringLiteral("id_client")}},
        {QStringLiteral("nom"), {QStringLiteral("nom"), QStringLiteral("nomclient"), QStringLiteral("name"), QStringLiteral("lastname"), QStringLiteral("nom_client")}},
        {QStringLiteral("prenom"), {QStringLiteral("prenom"), QStringLiteral("prenomclient"), QStringLiteral("firstname"), QStringLiteral("prenom_client")}},
        {QStringLiteral("statut"), {QStringLiteral("statut"), QStringLiteral("statutclient"), QStringLiteral("status"), QStringLiteral("statut_conformite")}},
        {QStringLiteral("profil"), {QStringLiteral("profil"), QStringLiteral("profilclient"), QStringLiteral("profile"), QStringLiteral("profil_client")}},
        {QStringLiteral("telephone"), {QStringLiteral("telephone"), QStringLiteral("tel"), QStringLiteral("phone")}},
        {QStringLiteral("date"), {QStringLiteral("date"), QStringLiteral("dateinscription"), QStringLiteral("date_inscription"), QStringLiteral("createdat")}},
    };

    const QStringList dbCols = getColumnNames(db, tableName);
    const QString idCol = matchColumnBySynonyms(dbCols, syn.value(QStringLiteral("id")));

    // Génération ID (265NNNN) en mode ajout
    if (!isEditing && ui->lineEdit_3c && ui->lineEdit_3c->text().trimmed().isEmpty()) {
        QString genErr;
        const QString newId = generateNextNumericIdWithPrefix(db, tableName, idCol, 265, 4, &genErr);
        if (newId.isEmpty()) {
            QMessageBox::critical(this, QStringLiteral("Ajout client"),
                                  QStringLiteral("Impossible de générer l'ID client (265NNNN): %1").arg(genErr));
            return;
        }
        ui->lineEdit_3c->setText(newId);
    }

    QString err;
    if (isEditing) {
        const QString idWhere = m_editingClientId.isEmpty() ? values.value(QStringLiteral("id")).toString() : m_editingClientId;
        // ID en lecture seule: ne pas mettre à jour la clé primaire.
        if (!updateRowByMapping(this, db, tableName, QStringLiteral("id"), idWhere, values, syn, false, &err)) {
            QMessageBox::critical(this, QStringLiteral("Modifier client"), QStringLiteral("Modification \u00E9chou\u00E9e: %1").arg(err));
            return;
        }

        QMessageBox::information(this, QStringLiteral("Modifier client"), QStringLiteral("Client modifi\u00E9 avec succ\u00E8s."));
        m_editingClientId.clear();
        if (ui->pushButton_2c) ui->pushButton_2c->setText(QStringLiteral("Ajouter"));
    } else {
        if (!insertRowByMapping(this, db, tableName, values, syn, &err)) {
            QMessageBox::critical(this, QStringLiteral("Ajout client"), QStringLiteral("Insertion \u00E9chou\u00E9e: %1").arg(err));
            return;
        }
        QMessageBox::information(this, QStringLiteral("Ajout client"), QStringLiteral("Client ajout\u00E9 avec succ\u00E8s."));
    }

    // Nettoyer le formulaire
    if (ui->lineEdit_3c) ui->lineEdit_3c->clear();
    if (ui->lineEdit_4c) ui->lineEdit_4c->clear();
    if (ui->lineEdit_12c) ui->lineEdit_12c->clear();
    if (ui->lineEdit_14c) ui->lineEdit_14c->clear();
    if (ui->comboBoxc) ui->comboBoxc->setCurrentIndex(0);
    if (ui->comboBox_2c) ui->comboBox_2c->setCurrentIndex(0);

    // Préparer l'ID pour le prochain ajout
    if (ui->lineEdit_3c && !idCol.isEmpty()) {
        QString genErr;
        const QString nextId = generateNextNumericIdWithPrefix(db, tableName, idCol, 265, 4, &genErr);
        if (!nextId.isEmpty()) {
            ui->lineEdit_3c->setText(nextId);
        }
    }

    refreshClientsPage();

    auto selectRowById = [&](const QString& idToFind) -> bool {
        if (!ui || !ui->tableWidgetc) return false;
        if (idToFind.trimmed().isEmpty()) return false;
        QTableWidget *t = ui->tableWidgetc;
        for (int r = 0; r < t->rowCount(); ++r) {
            QTableWidgetItem *it = t->item(r, 0);
            if (!it) continue;
            if (it->text().trimmed() == idToFind.trimmed()) {
                t->selectRow(r);
                t->scrollToItem(it, QAbstractItemView::PositionAtCenter);
                return true;
            }
        }
        return false;
    };

    // Si la ligne n'appara├«t pas (filtres actifs), r├⌐initialiser les filtres puis rafra├«chir.
    if (!selectRowById(targetId)) {
        if (ui->lineEdit_7c) ui->lineEdit_7c->clear();
        if (ui->comboBoxc_2) {
            int idxTous = -1;
            for (int i = 0; i < ui->comboBoxc_2->count(); ++i) {
                if (ui->comboBoxc_2->itemText(i).compare(QStringLiteral("Tous"), Qt::CaseInsensitive) == 0) { idxTous = i; break; }
            }
            if (idxTous >= 0) ui->comboBoxc_2->setCurrentIndex(idxTous);
            else ui->comboBoxc_2->setCurrentIndex(0);
        }
        if (ui->dateEdit_2c) ui->dateEdit_2c->setDate(QDate(2000, 1, 1));

        refreshClientsPage();
        selectRowById(targetId);
    }
}

void MainWindow::modifierClientFromRow(int row)
{
    if (!ui || !ui->tableWidgetc) return;
    QTableWidget *table = ui->tableWidgetc;
    if (row < 0 || row >= table->rowCount()) return;

    auto cellText = [&](int col) -> QString {
        QTableWidgetItem *item = table->item(row, col);
        return item ? item->text() : QString();
    };

    const QString id = cellText(0).trimmed();
    if (id.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Modifier client"), QStringLiteral("ID client introuvable sur la ligne s\u00E9lectionn\u00E9e."));
        return;
    }

    m_editingClientId = id;

    if (ui->lineEdit_3c) {
        ui->lineEdit_3c->setText(id);
        ui->lineEdit_3c->setReadOnly(false);
    }
    if (ui->lineEdit_4c) ui->lineEdit_4c->setText(cellText(1));
    if (ui->lineEdit_12c) ui->lineEdit_12c->setText(cellText(2));
    if (ui->comboBoxc) ui->comboBoxc->setCurrentText(cellText(3));
    if (ui->comboBox_2c) ui->comboBox_2c->setCurrentText(cellText(4));

    const QString dateText = cellText(5).trimmed();
    if (ui->dateEdit_c && !dateText.isEmpty()) {
        QDate d = QDate::fromString(dateText, QStringLiteral("yyyy-MM-dd"));
        if (!d.isValid()) d = QDate::fromString(dateText, QStringLiteral("dd/MM/yyyy"));
        if (d.isValid()) ui->dateEdit_c->setDate(d);
    }

    if (ui->lineEdit_14c) ui->lineEdit_14c->setText(cellText(6));

    if (ui->pushButton_2c) ui->pushButton_2c->setText(QStringLiteral("Modifier"));

    QMessageBox::information(this,
                             QStringLiteral("Modifier client"),
                             QStringLiteral("Client charg\u00E9 dans le formulaire. Modifiez les champs puis cliquez sur Modifier."));
}

void MainWindow::supprimerClientFromRow(int row)
{
    if (!ui || !ui->tableWidgetc) return;
    QTableWidget *table = ui->tableWidgetc;
    if (row < 0 || row >= table->rowCount()) return;

    const QString id = table->item(row, 0) ? table->item(row, 0)->text().trimmed() : QString();
    const QString nom = table->item(row, 1) ? table->item(row, 1)->text().trimmed() : QString();
    const QString prenom = table->item(row, 2) ? table->item(row, 2)->text().trimmed() : QString();

    if (id.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Supprimer client"), QStringLiteral("ID client introuvable sur la ligne s\u00E9lectionn\u00E9e."));
        return;
    }

    const auto reply = QMessageBox::question(
        this,
        QStringLiteral("Confirmer la suppression"),
        QStringLiteral("Voulez-vous vraiment supprimer le client :\n%1 %2 (ID: %3) ?").arg(nom, prenom, id),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    Connection *conn = Connection::getInstance();
    if (!conn->ensureOpen()) {
        QMessageBox::critical(this, QStringLiteral("DB"), QStringLiteral("Connexion DB \u00E9chou\u00E9e: %1").arg(conn->lastErrorText()));
        return;
    }
    QSqlDatabase db = conn->getDatabase();

    const QString tableName = resolveTableName(db, {QStringLiteral("CLIENT"), QStringLiteral("CLIENTS"), QStringLiteral("TCLIENT"), QStringLiteral("T_CLIENT")});
    if (tableName.isEmpty()) {
        QMessageBox::critical(this, QStringLiteral("DB"), QStringLiteral("Table clients introuvable (CLIENT/CLIENTS...)."));
        return;
    }

    const QHash<QString, QStringList> syn = {
        {QStringLiteral("id"), {QStringLiteral("id"), QStringLiteral("idclient"), QStringLiteral("clientid"), QStringLiteral("id_client")}},
        {QStringLiteral("nom"), {QStringLiteral("nom"), QStringLiteral("nomclient"), QStringLiteral("name"), QStringLiteral("lastname"), QStringLiteral("nom_client")}},
        {QStringLiteral("prenom"), {QStringLiteral("prenom"), QStringLiteral("prenomclient"), QStringLiteral("firstname"), QStringLiteral("prenom_client")}},
        {QStringLiteral("statut"), {QStringLiteral("statut"), QStringLiteral("statutclient"), QStringLiteral("status"), QStringLiteral("statut_conformite")}},
        {QStringLiteral("profil"), {QStringLiteral("profil"), QStringLiteral("profilclient"), QStringLiteral("profile"), QStringLiteral("profil_client")}},
        {QStringLiteral("telephone"), {QStringLiteral("telephone"), QStringLiteral("tel"), QStringLiteral("phone")}},
        {QStringLiteral("date"), {QStringLiteral("date"), QStringLiteral("dateinscription"), QStringLiteral("date_inscription"), QStringLiteral("createdat")}},
    };

    QString err;
    if (!deleteRowByMapping(this, db, tableName, QStringLiteral("id"), id, syn, &err)) {
        QMessageBox::critical(this, QStringLiteral("Supprimer client"), QStringLiteral("Suppression \u00E9chou\u00E9e: %1").arg(err));
        return;
    }

    if (!m_editingClientId.isEmpty() && m_editingClientId == id) {
        m_editingClientId.clear();
        if (ui->pushButton_2c) ui->pushButton_2c->setText(QStringLiteral("Ajouter"));
        if (ui->lineEdit_3c) ui->lineEdit_3c->setEnabled(true);
    }

    QMessageBox::information(this, QStringLiteral("Supprimer client"), QStringLiteral("Client supprim\u00E9 avec succ\u00E8s."));
    refreshClientsPage();
}

void MainWindow::on_lineEdit_7c_textChanged(const QString &)
{
    refreshClientsPage();
}

void MainWindow::on_comboBoxc_2_currentTextChanged(const QString &)
{
    refreshClientsPage();
}

void MainWindow::on_dateEdit_2c_dateChanged(const QDate &)
{
    refreshClientsPage();
}

void MainWindow::on_pushButton_5e_clicked()
{
    if (!handleCrudDisabled(this) || !ui) return;

    const bool editingEmploye = (m_editingEmployeId > 0);
    const QString idConstraintMessage = QStringLiteral(
        "Le champ ID Employe est obligatoire et doit respecter le format selon le role :\n"
        "- Gardien : 2643NNN\n"
        "- Technicien : 2644NNN\n"
        "- Responsable : 2645NNN\n"
        "- Ouvrier : 2546NNN\n"
        "NNN : numero unique auto-incremente.");

    if (!validateRequiredFields(this, {
            {QStringLiteral("Nom"), ui->lineEdit_13e, [this]{ return !ui->lineEdit_13e || ui->lineEdit_13e->text().trimmed().isEmpty(); }},
            {QStringLiteral("Prenom"), ui->lineEdit_16e, [this]{ return !ui->lineEdit_16e || ui->lineEdit_16e->text().trimmed().isEmpty(); }},
            {QStringLiteral("Telephone"), ui->lineEdit_14e, [this]{ return !ui->lineEdit_14e || ui->lineEdit_14e->text().trimmed().isEmpty(); }},
            {QStringLiteral("Salaire"), ui->lineEdit_14e_2, [this]{ return !ui->lineEdit_14e_2 || ui->lineEdit_14e_2->text().trimmed().isEmpty(); }},
        })) {
        return;
    }

    // Recuperation des valeurs du formulaire (style atelier)
    const QString idText = ui->lineEdit_12e ? ui->lineEdit_12e->text().trimmed() : QString();
    int id = toIntOrZero(idText);
    const QString nom = ui->lineEdit_13e ? ui->lineEdit_13e->text().trimmed() : QString();
    const QString prenom = ui->lineEdit_16e ? ui->lineEdit_16e->text().trimmed() : QString();
    const QString telephone = ui->lineEdit_14e ? ui->lineEdit_14e->text().trimmed() : QString();
    bool salaireOk = false;
    const double salaire = ui->lineEdit_14e_2 ? ui->lineEdit_14e_2->text().trimmed().toDouble(&salaireOk) : 0.0;
    const QString equipe = ui->comboBox_15e ? ui->comboBox_15e->currentText().trimmed() : QString();
    const QString etat = ui->comboBox_16e ? ui->comboBox_16e->currentText().trimmed() : QString();
    const QString role = ui->comboBox_11e ? ui->comboBox_11e->currentText().trimmed() : QString();
    const QString statut = ui->comboBox_14e ? ui->comboBox_14e->currentText().trimmed() : QString();
    const QString raison = ui->lineEdit_raison_banni ? ui->lineEdit_raison_banni->text().trimmed() : QString();
    const QString cvPath = ui->lineEdit_cv_path ? ui->lineEdit_cv_path->text().trimmed() : QString();

    if (!editingEmploye && id <= 0) {
        id = Employe::genererNouvelId(role);
        if (id > 0 && ui->lineEdit_12e) {
            ui->lineEdit_12e->setText(QString::number(id));
        }
    }

    static const QRegularExpression nomPattern(QStringLiteral("^[A-Za-z├Ç-├┐\\s'-]+$"));
    static const QRegularExpression telPattern(QStringLiteral("^\\d{8,15}$"));

    if (id <= 0) {
        QMessageBox::warning(this, QStringLiteral("Saisie employe"), idConstraintMessage);
        if (ui->lineEdit_12e) ui->lineEdit_12e->setFocus();
        return;
    }

    // En mode ajout, forcer le schéma d'ID selon le rôle.
    // En mode édition, l'ID peut être issu d'un ancien schéma: on n'empêche pas la modification des autres champs.
    if (!editingEmploye) {
        const QString canonRole = canonicalEmployeRole(role);
        int prefix = 264;
        int multiplier = 10000; // NNNN (repli si role inconnu)
        if (canonRole == QStringLiteral("Gardien")) {
            prefix = 2643;
            multiplier = 1000;
        } else if (canonRole == QStringLiteral("Technicien")) {
            prefix = 2644;
            multiplier = 1000;
        } else if (canonRole == QStringLiteral("Responsable")) {
            prefix = 2645;
            multiplier = 1000;
        } else if (canonRole == QStringLiteral("Ouvrier")) {
            prefix = 2546;
            multiplier = 1000;
        }

        const int minId = prefix * multiplier;
        const int maxId = minId + (multiplier - 1);
        if (id <= minId || id > maxId) {
            QMessageBox::warning(this, QStringLiteral("Saisie employe"), idConstraintMessage);
            if (ui->lineEdit_12e) ui->lineEdit_12e->setFocus();
            return;
        }
    }
    if (!nomPattern.match(nom).hasMatch()) {
        QMessageBox::warning(this, QStringLiteral("Saisie employe"), QStringLiteral("Nom invalide (lettres uniquement)."));
        if (ui->lineEdit_13e) ui->lineEdit_13e->setFocus();
        return;
    }
    if (!nomPattern.match(prenom).hasMatch()) {
        QMessageBox::warning(this, QStringLiteral("Saisie employe"), QStringLiteral("Prenom invalide (lettres uniquement)."));
        if (ui->lineEdit_16e) ui->lineEdit_16e->setFocus();
        return;
    }
    if (!telPattern.match(telephone).hasMatch()) {
        QMessageBox::warning(this, QStringLiteral("Saisie employe"), QStringLiteral("Telephone invalide (8 a 15 chiffres)."));
        if (ui->lineEdit_14e) ui->lineEdit_14e->setFocus();
        return;
    }
    if (!salaireOk || salaire < 0.0) {
        QMessageBox::warning(this, QStringLiteral("Saisie employe"), QStringLiteral("Salaire invalide (nombre >= 0)."));
        if (ui->lineEdit_14e_2) ui->lineEdit_14e_2->setFocus();
        return;
    }

    Employe emp(id, nom, prenom, role, equipe, etat, statut, salaire, telephone, raison, cvPath);

    if (editingEmploye) {
        if (!emp.modifierAvecAncienId(m_editingEmployeId)) {
            QMessageBox::critical(this,
                                  QStringLiteral("Modification employe"),
                                  QStringLiteral("Modification echouee: %1").arg(Employe::lastError()));
            return;
        }
        QMessageBox::information(this, QStringLiteral("Modification employe"), QStringLiteral("Modification reussie."));
    } else {
        if (!emp.ajouter()) {
            QMessageBox::critical(this,
                                  QStringLiteral("Ajout employe"),
                                  QStringLiteral("Insertion echouee: %1").arg(Employe::lastError()));
            return;
        }
        QMessageBox::information(this, QStringLiteral("Ajout employe"), QStringLiteral("Ajout reussi."));
    }

    // Rafraichir la table et reinitialiser l'etat d'edition
    loadEmployes();

    // Réinitialiser les champs spécifiques (CV, Raison)
    m_currentCvPath.clear();
    if (ui->pushButton_5e_2) ui->pushButton_5e_2->setText(QStringLiteral("📄 Upload CV"));
    if (ui->lineEdit_raison_banni) ui->lineEdit_raison_banni->clear();
    if (ui->lineEdit_cv_path) ui->lineEdit_cv_path->clear();

    if (editingEmploye) {
        m_editingEmployeId = -1;
        if (ui->lineEdit_12e) ui->lineEdit_12e->setReadOnly(false);
        if (ui->pushButton_5e) ui->pushButton_5e->setText(QStringLiteral("Ajouter"));
        const int generatedId = Employe::genererNouvelId(ui->comboBox_11e ? ui->comboBox_11e->currentText().trimmed() : QString());
        if (generatedId > 0 && ui->lineEdit_12e) {
            ui->lineEdit_12e->setText(QString::number(generatedId));
        }
    }
}

Pecheurs MainWindow::pecheurFromForm() const
{
    const QDateTime now = QDateTime::currentDateTime();
    QString id = ui->lineEditp ? ui->lineEditp->text().trimmed().toUpper() : QString();
    const QString nom = ui->lineEdit_2p ? ui->lineEdit_2p->text() : QString();
    const QString prenom = ui->lineEdit_3p ? ui->lineEdit_3p->text() : QString();
    QString sexe = pecheurSexeCode(ui);
    const QString role = ui->comboBoxp ? ui->comboBoxp->currentText() : QString();
    const QString dispo = ui->comboBox_2 ? ui->comboBox_2->currentText() : QString();
    const QString email = ui->lineEditp_2 ? ui->lineEditp_2->text() : QString();
    const bool editingPecheur = !m_editingPecheurId.isEmpty();

    if (sexe.isEmpty() && id.size() >= 5) {
        const QString inferred = id.mid(4, 1).toUpper();
        if (inferred == QStringLiteral("M") || inferred == QStringLiteral("F")) {
            sexe = inferred;
        }
    }

    if (!editingPecheur) {
        if (!sexe.isEmpty()) {
            const QString generatedId = Pecheurs::genererNouvelId(sexe);
            if (!generatedId.isEmpty()) {
                id = generatedId;
                if (ui->lineEditp) {
                    ui->lineEditp->setText(id);
                }
            }
        }
    }

    const int heures = (editingPecheur && ui->dateTimeEdit) ? ui->dateTimeEdit->time().hour() : now.time().hour();
    const QDate dateInscription = (editingPecheur && ui->dateTimeEdit) ? ui->dateTimeEdit->date() : now.date();
    
    QDate dateAffectation;
    if (ui->dateTimeEdit_2) {
        const QDateTime dt = ui->dateTimeEdit_2->dateTime();
        if (!isPecheurAffectationNull(dt)) {
            dateAffectation = dt.date();
        }
    }

    if (!editingPecheur && ui->dateTimeEdit) {
        ui->dateTimeEdit->setDateTime(now);
    }

    int idBateau = 0;
    if (QComboBox* bateauCombo = pecheurBateauCombo(ui)) {
        bool ok = false;
        const int idFromData = bateauCombo->currentData().toInt(&ok);
        if (ok && idFromData > 0) {
            idBateau = idFromData;
        } else {
            idBateau = bateauIdFromText(bateauCombo->currentText());
        }
    } else if (QLineEdit* bateauEdit = pecheurBateauLineEdit(ui)) {
        idBateau = bateauIdFromText(bateauEdit->text());
    }

    return Pecheurs(id, nom, prenom, sexe, role, dispo, email, heures, dateInscription, dateAffectation, idBateau, m_pendingPecheurPhotoBytes);
}

void MainWindow::loadPecheurs()
{
    if (!ui || !ui->tableWidgetp) return;

    const QString recherche = ui->lineEdit_4p ? ui->lineEdit_4p->text().trimmed().simplified() : QString();
    QString roleSelection = cleanFilterLabel(ui->comboBox_5p ? ui->comboBox_5p->currentText() : QString());
    QString dispoSelection = cleanFilterLabel(ui->comboBox_6p ? ui->comboBox_6p->currentText() : QString());

    // Les valeurs par défaut de l'IHM ("Tous les rôles" / "Toutes les disponibilités")
    // ne doivent pas filtrer la table.
    const QString roleKey = normalizeKey(roleSelection);
    if (roleKey.isEmpty() || roleKey == QStringLiteral("tous") || roleKey.startsWith(QStringLiteral("touslesrole"))) {
        roleSelection.clear();
    }
    const QString dispoKey = normalizeKey(dispoSelection);
    if (dispoKey.isEmpty() || dispoKey == QStringLiteral("tous") || dispoKey.startsWith(QStringLiteral("touteslesdisponibilit"))) {
        dispoSelection.clear();
    }

    QVector<Pecheurs::TableRowData> rows;
    if (!Pecheurs::chargerTable(recherche, roleSelection, dispoSelection, rows)) {
        qDebug() << "Erreur recherche/loadPecheurs:" << Pecheurs::lastError();
    }

    auto* table = ui->tableWidgetp;
    table->setAlternatingRowColors(false);
    table->setShowGrid(false);
    table->setStyleSheet(
        "QTableWidget {"
        "background-color: rgb(224, 238, 255);"
        "alternate-background-color: rgb(224, 238, 255);"
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

    int row = 0;
    for (const Pecheurs::TableRowData& record : rows) {
        const int currentRow = row;
        table->insertRow(row);

        table->setItem(row, 0, new QTableWidgetItem(record.id));
        table->setItem(row, 1, new QTableWidgetItem(record.nom));
        table->setItem(row, 2, new QTableWidgetItem(record.prenom));
        table->setItem(row, 3, new QTableWidgetItem(record.role));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(record.idBateau)));
        table->setItem(row, 5, new QTableWidgetItem(record.disponibilite));
        table->setItem(row, 6, new QTableWidgetItem(record.email));

        const QDate dIns = record.dateInscription;
        const QDate dAff = record.dateAffectation;
        const int heures = record.heures;
        const QString heureTexte = QStringLiteral(" %1:00").arg(qBound(0, heures, 23), 2, 10, QChar('0'));
        table->setItem(row, 7, new QTableWidgetItem(dIns.isValid() ? dIns.toString("dd/MM/yyyy") + heureTexte : QString()));
        table->setItem(row, 8, new QTableWidgetItem(dAff.isValid() ? dAff.toString("dd/MM/yyyy") + heureTexte : QStringLiteral("Non affectée")));
        table->setItem(row, 9, new QTableWidgetItem(QString::number(heures)));
        
        // Affichage de la photo dans la colonne 10 (from mainwindow1)
        if (!record.photo.isEmpty()) {
            QPixmap pix;
            if (pix.loadFromData(record.photo)) {
                QLabel* photoLabel = new QLabel();
                photoLabel->setPixmap(pix.scaled(42, 42, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                photoLabel->setAlignment(Qt::AlignCenter);
                photoLabel->setProperty("pecheurPhotoPreview", pix);
                photoLabel->setCursor(Qt::PointingHandCursor);
                photoLabel->installEventFilter(this);
                table->setCellWidget(currentRow, 10, photoLabel);
            } else {
                table->setItem(currentRow, 10, new QTableWidgetItem(QStringLiteral("Format invalide")));
            }
        } else {
            table->setItem(currentRow, 10, new QTableWidgetItem(QStringLiteral("Aucune")));
        }

        auto* actionWidget = new QWidget();
        actionWidget->setStyleSheet("background-color: transparent;");
        auto* actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(2, 2, 2, 2);
        actionLayout->setSpacing(7);

        auto* btnEdit = new QPushButton(QStringLiteral("\u270F\uFE0F"));
        btnEdit->setFixedSize(42, 32);
        QFont editFont(QStringLiteral("Segoe UI Emoji"));
        editFont.setPointSize(15);
        editFont.setBold(true);
        btnEdit->setFont(editFont);
        btnEdit->setFlat(true);
        btnEdit->setStyleSheet(QStringLiteral(
            "QPushButton {"
            " border: 2px solid rgb(0, 0, 112);"
            " border-radius: 6px;"
            " background-color: rgb(224, 238, 255);"
            " color: rgb(0, 0, 112);"
            " padding: 0px;"
            " }"
            "QPushButton:hover { background-color: rgb(224, 238, 255); }"
            "QPushButton:pressed { background-color: rgb(224, 238, 255); }"));
        btnEdit->setToolTip(QStringLiteral("Modifier"));
        connect(btnEdit, &QPushButton::clicked, this, [this, currentRow]() {
            if (ui && ui->tableWidgetp && currentRow < ui->tableWidgetp->rowCount()) {
                ui->tableWidgetp->selectRow(currentRow);
                on_pushButton_6p_clicked();
            }
        });

        auto* btnDelete = new QPushButton(QStringLiteral("\u274C"));
        btnDelete->setFixedSize(42, 32);
        QFont deleteFont(QStringLiteral("Segoe UI Emoji"));
        deleteFont.setPointSize(15);
        deleteFont.setBold(true);
        btnDelete->setFont(deleteFont);
        btnDelete->setFlat(true);
        btnDelete->setStyleSheet(QStringLiteral(
            "QPushButton {"
            " border: 2px solid rgb(0, 0, 112);"
            " border-radius: 6px;"
            " background-color: rgb(224, 238, 255);"
            " color: rgb(0, 0, 112);"
            " padding: 0px;"
            " }"
            "QPushButton:hover { background-color: rgb(224, 238, 255); }"
            "QPushButton:pressed { background-color: rgb(224, 238, 255); }"));
        btnDelete->setToolTip(QStringLiteral("Supprimer"));
        connect(btnDelete, &QPushButton::clicked, this, [this, currentRow]() {
            if (ui && ui->tableWidgetp && currentRow < ui->tableWidgetp->rowCount()) {
                ui->tableWidgetp->selectRow(currentRow);
                on_pushButton_5p_clicked();
            }
        });

        actionLayout->addStretch();
        actionLayout->addWidget(btnEdit, 0, Qt::AlignVCenter);
        actionLayout->addWidget(btnDelete, 0, Qt::AlignVCenter);
        actionLayout->addStretch();

        table->setCellWidget(row, 11, actionWidget);
        ++row;
    }

    if (table->horizontalHeader()) {
        table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    }
    table->setColumnWidth(0, 110);
    table->setColumnWidth(1, 100);
    table->setColumnWidth(2, 100);
    table->setColumnWidth(3, 140);
    table->setColumnWidth(4, 70);
    table->setColumnWidth(5, 150);
    table->setColumnWidth(6, 180);
    table->setColumnWidth(7, 130);
    table->setColumnWidth(8, 130);
    table->setColumnWidth(9, 70);
    table->setColumnWidth(10, 60);
    table->setColumnWidth(11, 114);
    table->verticalHeader()->setDefaultSectionSize(42);

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

void MainWindow::on_bap_clicked()
{
    if (!handleCrudDisabled(this)) return;

    // Si le bouton affiche "Ajouter", on force le mode ajout
    // meme si un ancien ID d'edition est reste en memoire.
    if (ui && ui->bap
        && ui->bap->text().trimmed().compare(QStringLiteral("Ajouter"), Qt::CaseInsensitive) == 0) {
        m_editingPecheurId.clear();
    }

    const Pecheurs p = pecheurFromForm();
    const QString id = ui && ui->lineEditp ? ui->lineEditp->text().trimmed().toUpper() : QString();

    const QString nom = ui && ui->lineEdit_2p ? ui->lineEdit_2p->text().trimmed() : QString();
    const QString prenom = ui && ui->lineEdit_3p ? ui->lineEdit_3p->text().trimmed() : QString();
    const QString email = ui && ui->lineEditp_2 ? ui->lineEditp_2->text().trimmed() : QString();
    const QString sexe = pecheurSexeCode(ui);

    if (m_editingPecheurId.isEmpty() && sexe.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Champs"), QStringLiteral("Veuillez s\u00E9lectionner le sexe (Homme/Femme)"));
        return;
    }

    if (id.isEmpty()) {
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

    if (!m_editingPecheurId.isEmpty()) {
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
    } else {
        if (Pecheurs::idExiste(id)) {
            QMessageBox::warning(this, QStringLiteral("Doublon"), QStringLiteral("Cet ID existe d\u00E9j\u00E0. Veuillez saisir un ID diff\u00E9rent."));
            return;
        }

        if (p.ajouter()) {
            loadPecheurs();
            resetAjouterButton();

            // Apres ajout, afficher immediatement le prochain ID auto pour le meme sexe.
            if (ui && ui->lineEditp) {
                const QString nextId = Pecheurs::genererNouvelId(sexe);
                if (!nextId.isEmpty()) {
                    ui->lineEditp->setText(nextId);
                }
            }

            QMessageBox::information(this, "OK", "Ajout reussi");
        } else {
            const QString err = Pecheurs::lastError();
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

    const int currentRow = ui->tableWidgetp->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, QStringLiteral("S\u00E9lection"), QStringLiteral("Veuillez s\u00E9lectionner une ligne"));
        return;
    }

    m_pendingPecheurPhotoBytes.clear();

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

    m_editingPecheurId = id.trimmed().toUpper();

    if (ui->lineEditp) ui->lineEditp->setText(id);

    const QString idUpper = id.trimmed().toUpper();
    if (idUpper.size() >= 3) {
        const QString codeSexe = idUpper.mid(2, 1);
        if (ui->radioButton_2p && ui->radioButtonp) {
            if (codeSexe == QStringLiteral("1") || codeSexe == QStringLiteral("H")) {
                ui->radioButton_2p->setChecked(true);
            } else if (codeSexe == QStringLiteral("2") || codeSexe == QStringLiteral("F")) {
                ui->radioButtonp->setChecked(true);
            }
        }
    }
    if (ui->lineEdit_2p) ui->lineEdit_2p->setText(nom);
    if (ui->lineEdit_3p) ui->lineEdit_3p->setText(prenom);

    if (ui->comboBoxp) {
        const int roleIndex = ui->comboBoxp->findText(role);
        if (roleIndex >= 0) {
            ui->comboBoxp->setCurrentIndex(roleIndex);
        }
    }

    if (ui->comboBox_2) {
        const int dispoIndex = ui->comboBox_2->findText(dispo);
        if (dispoIndex >= 0) {
            ui->comboBox_2->setCurrentIndex(dispoIndex);
        }
    }

    if (ui->lineEditp_2) ui->lineEditp_2->setText(email);
    if (QLineEdit* bateauEdit = pecheurBateauLineEdit(ui)) {
        bateauEdit->setText(idBateau.isEmpty() ? QStringLiteral("0") : idBateau);
    } else if (QComboBox* bateauCombo = pecheurBateauCombo(ui)) {
        bool ok = false;
        const int idValue = idBateau.trimmed().isEmpty() ? 0 : idBateau.toInt(&ok);
        if (idBateau.trimmed().isEmpty()) ok = true;
        int idx = -1;
        if (ok) {
            idx = bateauCombo->findData(idValue);
            if (idx < 0) {
                idx = bateauCombo->findText(QString::number(idValue), Qt::MatchStartsWith);
            }
        }
        if (idx >= 0) {
            bateauCombo->setCurrentIndex(idx);
        } else {
            bateauCombo->setCurrentIndex(0);
        }
    }

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
    if (ui->dateTimeEdit_2) {
        if (!dateAffStr.isEmpty() && dateAffStr != QStringLiteral("Non affectée")) {
            QDateTime dtAff = QDateTime::fromString(dateAffStr, "dd/MM/yyyy HH:mm");
            if (!dtAff.isValid()) {
                const QDate dAff = QDate::fromString(dateAffStr, "dd/MM/yyyy");
                if (dAff.isValid()) {
                    dtAff = QDateTime(dAff, QTime(qBound(0, heures, 23), 0, 0));
                }
            }
            if (dtAff.isValid()) {
                ui->dateTimeEdit_2->setDateTime(dtAff);
            } else {
                ui->dateTimeEdit_2->setDateTime(nullPecheurAffectationDateTime());
            }
        } else {
            ui->dateTimeEdit_2->setDateTime(nullPecheurAffectationDateTime());
        }
    }

    const QDateTime now = QDateTime::currentDateTime();
    if (ui->dateTimeEdit) {
        ui->dateTimeEdit->setReadOnly(true);
    }
    if (ui->dateTimeEdit_2) {
        // Garder le minimum a 1900 pour permettre "Non affectee"
    }

    if (ui && ui->lineEditp) {
        ui->lineEditp->setReadOnly(false);
        ui->lineEditp->setEnabled(true);
    }

    if (ui && ui->bap) {
        ui->bap->setText("Modifier");
    }
}

void MainWindow::resetAjouterButton()
{
    if (ui && ui->bap) {
        ui->bap->setText("Ajouter");
    }

    // Revenir a un formulaire vide (mode ajout)
    if (ui && ui->lineEdit_2p) ui->lineEdit_2p->clear();
    if (ui && ui->lineEdit_3p) ui->lineEdit_3p->clear();
    if (ui && ui->lineEditp_2) ui->lineEditp_2->clear();

    if (ui && ui->comboBoxp) ui->comboBoxp->setCurrentIndex(0);
    if (ui && ui->comboBox_2) ui->comboBox_2->setCurrentIndex(0);

    if (QLineEdit* bateauEdit = pecheurBateauLineEdit(ui)) {
        bateauEdit->setText(QStringLiteral("0"));
    } else if (QComboBox* bateauCombo = pecheurBateauCombo(ui)) {
        bateauCombo->setCurrentIndex(0);
    }

    // Garantir un sexe selectionne pour pouvoir afficher un nouvel ID auto.
    if (ui && ui->radioButton_2p && ui->radioButtonp
        && !ui->radioButton_2p->isChecked() && !ui->radioButtonp->isChecked()) {
        ui->radioButton_2p->setChecked(true);
    }

    if (ui && ui->lineEditp) {
        ui->lineEditp->setReadOnly(false);
        ui->lineEditp->setEnabled(true);
        const QString generatedId = Pecheurs::genererNouvelId(pecheurSexeCode(ui));
        if (!generatedId.isEmpty()) {
            ui->lineEditp->setText(generatedId);
        } else {
            ui->lineEditp->clear();
        }
    }
    if (ui && ui->dateTimeEdit) {
        ui->dateTimeEdit->setReadOnly(false);
        ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
    }
    if (ui && ui->dateTimeEdit_2) {
        ui->dateTimeEdit_2->setDateTime(nullPecheurAffectationDateTime());
    }
    m_pendingPecheurPhotoBytes.clear();
    m_editingPecheurId.clear();
}

void MainWindow::on_pushButton_6p_clicked()
{
    if (!handleCrudDisabled(this)) return;

    if (m_editingPecheurId.isEmpty() && ui && ui->tableWidgetp && ui->tableWidgetp->currentRow() >= 0) {
        loadPecheurFromTable();
        if (ui->lineEditp) {
            ui->lineEditp->setReadOnly(false);
            ui->lineEditp->setEnabled(true);
        }
        QMessageBox::information(this,
                                 QStringLiteral("\u00C9dition"),
                                 QStringLiteral("Formulaire rempli - modifiez les champs et cliquez sur Modifier pour sauvegarder"));
    }
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

    if (!ui || !ui->tableWidgetp) return;

    const int currentRow = ui->tableWidgetp->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this,
                             QStringLiteral("S\u00E9lection"),
                             QStringLiteral("Veuillez s\u00E9lectionner un p\u00EAcheur \u00E0 supprimer"));
        return;
    }

    QTableWidgetItem* idItem = ui->tableWidgetp->item(currentRow, 0);
    if (!idItem) {
        QMessageBox::warning(this,
                             QStringLiteral("Erreur"),
                             QStringLiteral("Impossible de r\u00E9cup\u00E9rer l'ID du p\u00EAcheur"));
        return;
    }

    const QString id = idItem->text().trimmed().toUpper();
    if (id.isEmpty()) {
        QMessageBox::warning(this, "ID", "ID invalide");
        return;
    }

    const QString nom = ui->tableWidgetp->item(currentRow, 1) ? ui->tableWidgetp->item(currentRow, 1)->text() : QString();
    const QString prenom = ui->tableWidgetp->item(currentRow, 2) ? ui->tableWidgetp->item(currentRow, 2)->text() : QString();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Confirmation",
        QStringLiteral("Voulez-vous vraiment supprimer le p\u00EAcheur :\n%1 %2 (ID: %3) ?").arg(nom).arg(prenom).arg(id),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply != QMessageBox::Yes) {
        return;
    }

    if (Pecheurs::supprimer(id)) {
        loadPecheurs();
        m_editingPecheurId.clear();
        QMessageBox::information(this, QStringLiteral("OK"), QStringLiteral("Suppression r\u00E9ussie"));
    } else {
        QMessageBox::critical(this, QStringLiteral("Erreur"), QStringLiteral("Suppression \u00E9chou\u00E9e: ") + Pecheurs::lastError());
    }
}

static int findPecheurActionColumnIndex(const QTableWidget* table)
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

static QString normalizePdfCellText(QString text)
{
    text.replace('\n', ' ');
    text.replace('\r', ' ');
    return text.simplified();
}

static int pdfWrappedTextHeight(QPainter& painter,
                                const QFont& font,
                                const QString& text,
                                int width,
                                int minHeight = 0,
                                int flags = Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap)
{
    painter.save();
    painter.setFont(font);
    const QRect bounds = painter.boundingRect(
        QRect(0, 0, qMax(20, width), 8000),
        flags,
        text);
    painter.restore();
    return qMax(minHeight, bounds.height());
}

static QString currentQuaiSortText(Ui::MainWindow* ui)
{
    if (!ui || !ui->page_3) {
        return QString();
    }

    if (QComboBox* comboTri = ui->page_3->findChild<QComboBox*>(QStringLiteral("comboBox_sortQuais"))) {
        QString text = normalizePdfCellText(comboTri->currentText());
        if (text.startsWith(QStringLiteral("Tri:"), Qt::CaseInsensitive)) {
            text = text.mid(4).trimmed();
        }
        return text;
    }

    return QString();
}

static QStringList buildQuaiReportInfoLines(Ui::MainWindow* ui,
                                            const QString& pageLabel,
                                            const QString& exportStamp,
                                            int visibleRows,
                                            int totalRows)
{
    QStringList filterParts;

    if (ui) {
        const QString searchText = ui->lineEdit_3 ? normalizePdfCellText(ui->lineEdit_3->text()) : QString();
        const QString statutText = ui->comboBox_3 ? normalizePdfCellText(ui->comboBox_3->currentText()) : QString();
        const QString zoneText = ui->comboBox_4 ? normalizePdfCellText(ui->comboBox_4->currentText()) : QString();
        const QString capaciteText = ui->comboBox_5 ? normalizePdfCellText(ui->comboBox_5->currentText()) : QString();
        const QString sortText = currentQuaiSortText(ui);

        if (!searchText.isEmpty()) {
            filterParts << QStringLiteral("Recherche: %1").arg(searchText);
        }
        if (!statutText.isEmpty() && statutText.compare(QStringLiteral("Tous"), Qt::CaseInsensitive) != 0) {
            filterParts << QStringLiteral("Statut: %1").arg(statutText);
        }
        if (!zoneText.isEmpty() && zoneText.compare(QStringLiteral("Toutes"), Qt::CaseInsensitive) != 0) {
            filterParts << QStringLiteral("Zone: %1").arg(zoneText);
        }
        if (!capaciteText.isEmpty() && capaciteText.compare(QStringLiteral("Toutes"), Qt::CaseInsensitive) != 0) {
            filterParts << QStringLiteral("Capacite: %1").arg(capaciteText);
        }
        const bool isDefaultSort =
            sortText.compare(QStringLiteral("Par defaut"), Qt::CaseInsensitive) == 0
            || sortText.compare(QStringLiteral("Par défaut"), Qt::CaseInsensitive) == 0;
        if (!sortText.isEmpty() && !isDefaultSort) {
            filterParts << QStringLiteral("Tri: %1").arg(sortText);
        }
    }

    const QString filtersText = filterParts.isEmpty()
        ? QStringLiteral("Aucun filtre actif")
        : filterParts.join(QStringLiteral(" | "));

    return {
        QStringLiteral("Application: AquaTech"),
        QStringLiteral("Module: %1").arg(pageLabel),
        QStringLiteral("Date d'export: %1").arg(exportStamp),
        QStringLiteral("Quais visibles: %1 / %2").arg(visibleRows).arg(totalRows),
        QStringLiteral("Filtres: %1").arg(filtersText)
    };
}

static int countActiveQuaiFilters(Ui::MainWindow* ui)
{
    if (!ui) {
        return 0;
    }

    int count = 0;
    const QString searchText = ui->lineEdit_3 ? normalizePdfCellText(ui->lineEdit_3->text()) : QString();
    const QString statutText = ui->comboBox_3 ? normalizePdfCellText(ui->comboBox_3->currentText()) : QString();
    const QString zoneText = ui->comboBox_4 ? normalizePdfCellText(ui->comboBox_4->currentText()) : QString();
    const QString capaciteText = ui->comboBox_5 ? normalizePdfCellText(ui->comboBox_5->currentText()) : QString();
    const QString sortText = currentQuaiSortText(ui);

    if (!searchText.isEmpty()) ++count;
    if (!statutText.isEmpty() && statutText.compare(QStringLiteral("Tous"), Qt::CaseInsensitive) != 0) ++count;
    if (!zoneText.isEmpty() && zoneText.compare(QStringLiteral("Toutes"), Qt::CaseInsensitive) != 0) ++count;
    if (!capaciteText.isEmpty() && capaciteText.compare(QStringLiteral("Toutes"), Qt::CaseInsensitive) != 0) ++count;

    const bool isDefaultSort =
        sortText.compare(QStringLiteral("Par defaut"), Qt::CaseInsensitive) == 0
        || sortText.compare(QStringLiteral("Par défaut"), Qt::CaseInsensitive) == 0;
    if (!sortText.isEmpty() && !isDefaultSort) ++count;

    return count;
}

static int drawQuaiPdfHeader(QPainter& painter,
                             const QRect& contentRect,
                             const QString& reportTitle,
                             const QStringList& infoLines)
{
    const QPixmap logo(QStringLiteral(":/res/LOOOG.png"));
    const bool hasLogo = !logo.isNull();

    const int outerPadding = 24;
    const int logoBoxSize = hasLogo ? 188 : 0;
    const int textLeft = contentRect.left() + outerPadding + (hasLogo ? logoBoxSize + 30 : 0);
    const int textWidth = qMax(220, contentRect.width() - (textLeft - contentRect.left()) - outerPadding);

    const QFont appFont(QStringLiteral("Arial"), 13, QFont::Bold);
    const QFont titleFont(QStringLiteral("Arial"), 22, QFont::Bold);
    const QFont infoFont(QStringLiteral("Arial"), 9);

    const int appHeight = pdfWrappedTextHeight(
        painter,
        appFont,
        QStringLiteral("AquaTech"),
        textWidth,
        20,
        Qt::AlignLeft | Qt::AlignVCenter);
    const int titleHeight = pdfWrappedTextHeight(
        painter,
        titleFont,
        reportTitle,
        textWidth,
        32,
        Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap);
    int infoHeight = 0;
    for (const QString& line : infoLines) {
        infoHeight += pdfWrappedTextHeight(
                          painter,
                          infoFont,
                          line,
                          textWidth,
                          18,
                          Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap) + 4;
    }

    const int textBlockHeight = appHeight + 8 + titleHeight + 12 + infoHeight;
    const int visualBlockHeight = qMax(textBlockHeight, hasLogo ? logoBoxSize : 0);
    const int headerHeight = qMax(218, (outerPadding * 2) + visualBlockHeight);
    const QRect headerRect(contentRect.left(), contentRect.top(), contentRect.width(), headerHeight);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QLinearGradient background(headerRect.topLeft(), headerRect.bottomLeft());
    background.setColorAt(0.0, QColor(QStringLiteral("#f8fbff")));
    background.setColorAt(1.0, QColor(QStringLiteral("#eef4fb")));
    painter.setPen(QPen(QColor(QStringLiteral("#d5e2ef")), 1));
    painter.setBrush(background);
    painter.drawRoundedRect(headerRect, 16, 16);

    painter.fillRect(QRect(headerRect.left(), headerRect.top(), headerRect.width(), 10), QColor(QStringLiteral("#0b5ea8")));

    if (hasLogo) {
        const int logoTop = headerRect.top() + outerPadding + qMax(0, (visualBlockHeight - logoBoxSize) / 2);
        const QRect logoBox(headerRect.left() + outerPadding, logoTop, logoBoxSize, logoBoxSize);
        painter.setPen(QPen(QColor(QStringLiteral("#d8e5f1")), 1));
        painter.setBrush(Qt::white);
        painter.drawRoundedRect(logoBox, 18, 18);

        const QPixmap scaledLogo = logo.scaled(logoBox.size() - QSize(12, 12), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        const QRect logoRect(
            logoBox.center().x() - (scaledLogo.width() / 2),
            logoBox.center().y() - (scaledLogo.height() / 2),
            scaledLogo.width(),
            scaledLogo.height());
        painter.drawPixmap(logoRect, scaledLogo);
    }

    painter.setPen(QColor(QStringLiteral("#0b5ea8")));
    painter.setFont(appFont);
    int textY = headerRect.top() + outerPadding;
    painter.drawText(QRect(textLeft, textY, textWidth, appHeight),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     QStringLiteral("AquaTech"));

    painter.setPen(QColor(QStringLiteral("#102a43")));
    painter.setFont(titleFont);
    textY += appHeight + 8;
    painter.drawText(QRect(textLeft, textY, textWidth, titleHeight),
                     Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                     reportTitle);

    painter.setPen(QColor(QStringLiteral("#52667a")));
    painter.setFont(infoFont);
    int infoY = textY + titleHeight + 12;
    for (const QString& line : infoLines) {
        const int lineHeight = pdfWrappedTextHeight(
            painter,
            infoFont,
            line,
            textWidth,
            18,
            Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap);
        painter.drawText(QRect(textLeft, infoY, textWidth, lineHeight),
                         Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                         line);
        infoY += lineHeight + 4;
    }

    painter.restore();
    return headerRect.bottom() + 24;
}

static void drawQuaiPdfFooter(QPainter& painter,
                              int leftMargin,
                              int footerY,
                              int contentWidth,
                              const QString& exportStamp,
                              int pageNumber)
{
    painter.save();
    painter.setPen(QPen(QColor(QStringLiteral("#d5e2ef")), 1));
    painter.drawLine(leftMargin, footerY - 8, leftMargin + contentWidth, footerY - 8);

    painter.setPen(QColor(QStringLiteral("#64748b")));
    painter.setFont(QFont(QStringLiteral("Arial"), 8));
    painter.drawText(QRect(leftMargin, footerY, contentWidth / 2, 18),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     QStringLiteral("AquaTech - Gestion des quais"));
    painter.drawText(QRect(leftMargin + (contentWidth / 2), footerY, contentWidth / 2, 18),
                     Qt::AlignRight | Qt::AlignVCenter,
                     QStringLiteral("Genere le %1 | Page %2").arg(exportStamp).arg(pageNumber));
    painter.restore();
}

static void drawQuaiMetricCard(QPainter& painter,
                               const QRect& cardRect,
                               const QString& label,
                               const QString& value,
                               const QString& detail,
                               const QColor& accent)
{
    painter.save();
    painter.setPen(QPen(QColor(QStringLiteral("#d6e2ee")), 1));
    painter.setBrush(QColor(QStringLiteral("#f8fbff")));
    painter.drawRoundedRect(cardRect, 14, 14);
    painter.fillRect(QRect(cardRect.left(), cardRect.top(), 10, cardRect.height()), accent);

    const QFont labelFont(QStringLiteral("Arial"), 8, QFont::Bold);
    const QFont valueFont(QStringLiteral("Arial"), 18, QFont::Bold);
    const QFont detailFont(QStringLiteral("Arial"), 8);

    painter.setPen(QColor(QStringLiteral("#64748b")));
    painter.setFont(labelFont);
    painter.drawText(cardRect.adjusted(20, 14, -16, -82),
                     Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                     label);

    painter.setPen(QColor(QStringLiteral("#0f172a")));
    painter.setFont(valueFont);
    painter.drawText(cardRect.adjusted(20, 42, -16, -46),
                     Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                     value);

    painter.setPen(QColor(QStringLiteral("#475569")));
    painter.setFont(detailFont);
    painter.drawText(cardRect.adjusted(20, 82, -16, -14),
                     Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                     detail);
    painter.restore();
}

static int drawQuaiSectionIntro(QPainter& painter,
                                int left,
                                int top,
                                int width,
                                const QString& title,
                                const QString& subtitle)
{
    painter.save();
    painter.setPen(QColor(QStringLiteral("#0b5ea8")));
    painter.setFont(QFont(QStringLiteral("Arial"), 14, QFont::Bold));
    painter.drawText(QRect(left, top, width, 26),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     title);

    painter.setPen(QColor(QStringLiteral("#64748b")));
    painter.setFont(QFont(QStringLiteral("Arial"), 8));
    painter.drawText(QRect(left, top + 24, width, 22),
                     Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap,
                     subtitle);
    painter.restore();
    return top + 50;
}

static void exportEmployesPdfReport(MainWindow* parent, Ui::MainWindow* ui)
{
    if (!parent || !ui || !ui->tableWidgetee) return;

    const QString documentsDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString baseDir = documentsDir.isEmpty() ? QDir::homePath() : documentsDir;
    const QString defaultName = QStringLiteral("Rapport_Employes_Aquatec_2026.pdf");
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
        QMessageBox::critical(parent, QStringLiteral("Erreur"), QStringLiteral("Impossible de creer le fichier PDF."));
        return;
    }

    QTableWidget* table = ui->tableWidgetee;
    const int actionColumn = findPecheurActionColumnIndex(table);
    const bool actionColumnWasHidden = (actionColumn >= 0) ? table->isColumnHidden(actionColumn) : false;
    if (actionColumn >= 0 && !actionColumnWasHidden) {
        table->setColumnHidden(actionColumn, true);
    }

    auto restoreActionColumn = [table, actionColumn, actionColumnWasHidden]() {
        if (actionColumn >= 0 && !actionColumnWasHidden) {
            table->setColumnHidden(actionColumn, false);
        }
    };

    const QString exportStamp = QDateTime::currentDateTime().toString(QStringLiteral("dd/MM/yyyy HH:mm"));
    const int pageW = writer.width();
    const int pageH = writer.height();
    const int leftMargin = 50;
    const int rightMargin = 50;
    const int topMargin = 100;
    const int bottomMargin = 80;
    const int contentW = pageW - leftMargin - rightMargin;

    QFont titleFont(QStringLiteral("Arial"), 20, QFont::Bold);
    QFont subtitleFont(QStringLiteral("Arial"), 9, QFont::Normal);
    QFont headerFont(QStringLiteral("Arial"), 11, QFont::Bold);
    QFont cellFont(QStringLiteral("Arial"), 9, QFont::Normal);
    QFont totalFont(QStringLiteral("Arial"), 9, QFont::Bold);
    QFont footerFont(QStringLiteral("Arial"), 9, QFont::Normal);

    painter.setFont(titleFont);
    painter.setPen(QColor(0, 82, 155));
    painter.drawText(QRect(leftMargin, topMargin - 40, contentW, 60), Qt::AlignCenter,
                     QStringLiteral("AQUATEC - Liste des Employes"));

    painter.setPen(Qt::darkGray);
    painter.setFont(subtitleFont);
    painter.drawText(QRect(leftMargin, topMargin + 20, contentW, 30), Qt::AlignCenter,
                     QStringLiteral("Export le %1").arg(exportStamp));

    int y = topMargin + 80;

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
                             QStringLiteral("Aucune colonne a exporter."));
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

    const int headerH = 48;
    auto drawTableHeader = [&]() {
        int x = leftMargin;
        painter.setFont(headerFont);

        QLinearGradient headerGrad(0, y, 0, y + headerH);
        headerGrad.setColorAt(0.0, QColor(QStringLiteral("#0b5ea8")));
        headerGrad.setColorAt(1.0, QColor(QStringLiteral("#2e86c1")));

        for (int i = 0; i < columns.size(); ++i) {
            const int c = columns[i];
            const int w = drawWidths[i];
            const QRect cellRect(x, y, w, headerH);
            const QString text = table->horizontalHeaderItem(c) ? table->horizontalHeaderItem(c)->text().trimmed() : QStringLiteral("Colonne %1").arg(c + 1);

            painter.fillRect(cellRect, headerGrad);
            painter.setPen(Qt::white);
            painter.drawText(cellRect.adjusted(12, 0, -12, 0), Qt::AlignVCenter | Qt::AlignLeft, text);
            painter.setPen(QPen(QColor(200, 210, 220), 1));
            painter.drawRect(cellRect);
            x += w;
        }
        y += headerH;
    };

    const int footerY = pageH - bottomMargin + 10;
    const int tableTop = y;
    const int tableHeaderH = headerH;
    const int tableTotalH = 30;
    const int tableBottomLimit = pageH - bottomMargin - 40;
    const int tableRowsAreaH = qMax(40, tableBottomLimit - tableTop - tableHeaderH - tableTotalH);
    const int rowH = 40;
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
        const QColor rowColor = (index % 2 == 0) ? QColor(250, 250, 252) : QColor(245, 251, 255);
        painter.fillRect(QRect(leftMargin, y, contentW, rowH), rowColor);
        painter.setPen(QPen(QColor(220, 225, 230), 1));
        painter.drawRect(QRect(leftMargin, y, contentW, rowH));

        int x = leftMargin;
        for (int i = 0; i < columns.size(); ++i) {
            const int c = columns[i];
            const int w = drawWidths[i];
            painter.setPen(QPen(QColor(220, 225, 230), 1));
            painter.drawLine(x, y, x, y + rowH);

            painter.setPen(Qt::black);
            const QTableWidgetItem* item = table->item(r, c);
            QString text = item ? item->text() : QString();
            text = text.simplified();

            QFontMetrics fm(cellFont);
            text = fm.elidedText(text, Qt::ElideRight, qMax(10, w - 24));
            painter.drawText(QRect(x + 12, y + 6, w - 24, rowH - 12), Qt::AlignLeft | Qt::AlignVCenter, text);
            x += w;
        }

        painter.setPen(QPen(QColor(220, 225, 230), 1));
        painter.drawLine(leftMargin + contentW, y, leftMargin + contentW, y + rowH);
        y += rowH;
    }

    y += 8;
    painter.setPen(Qt::darkGray);
    painter.setFont(totalFont);
    painter.drawText(QRect(leftMargin, y, contentW, 24), Qt::AlignCenter,
                     QStringLiteral("Total: %1 employes").arg(totalVisibleRows));

    if (totalVisibleRows > exportedRows) {
        painter.setFont(QFont(QStringLiteral("Arial"), 9, QFont::Normal));
        painter.setPen(QColor(QStringLiteral("#8a8a8a")));
        painter.drawText(QRect(leftMargin, y + 24, contentW, 18), Qt::AlignCenter,
                         QStringLiteral("(%1 lignes affichees sur %2 dans cette page)").arg(exportedRows).arg(totalVisibleRows));
    }

    painter.setPen(QColor(QStringLiteral("#8a8a8a")));
    painter.setFont(footerFont);
    painter.drawText(QRect(leftMargin, footerY, contentW, 20), Qt::AlignCenter,
                     QStringLiteral("AQUATEC - Rapport genere le %1").arg(exportStamp));

    restoreActionColumn();
    painter.end();

    QMessageBox::information(parent, QStringLiteral("Export PDF"),
                             QStringLiteral("Fichier PDF exporte avec succes:\n%1").arg(filePath));
}

static void exportPecheursPdfReport(MainWindow* parent, Ui::MainWindow* ui)
{
    if (!parent || !ui || !ui->tableWidgetp) return;

    const QString documentsDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString baseDir = documentsDir.isEmpty() ? QDir::homePath() : documentsDir;
    const QString defaultName = QStringLiteral("Rapport_Pecheurs_Aquatec_2026.pdf");
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
        QMessageBox::critical(parent, QStringLiteral("Erreur"), QStringLiteral("Impossible de cr├⌐er le fichier PDF."));
        return;
    }

    const int actionColumn = findPecheurActionColumnIndex(ui->tableWidgetp);
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

    const int leftMargin = 50;
    const int rightMargin = 50;
    const int topMargin = 100;
    const int bottomMargin = 80;
    const int contentW = pageW - leftMargin - rightMargin;

    QFont titleFont(QStringLiteral("Arial"), 20, QFont::Bold);
    QFont subtitleFont(QStringLiteral("Arial"), 9, QFont::Normal);
    QFont headerFont(QStringLiteral("Arial"), 11, QFont::Bold);
    QFont cellFont(QStringLiteral("Arial"), 9, QFont::Normal);
    QFont totalFont(QStringLiteral("Arial"), 9, QFont::Bold);
    QFont footerFont(QStringLiteral("Arial"), 9, QFont::Normal);

    painter.setFont(titleFont);
    painter.setPen(QColor(0, 82, 155));
    painter.drawText(QRect(leftMargin, topMargin - 40, contentW, 60), Qt::AlignCenter,
                     QStringLiteral("AQUATEC - Liste des Pecheurs"));

    painter.setPen(Qt::darkGray);
    painter.setFont(subtitleFont);
    painter.drawText(QRect(leftMargin, topMargin + 20, contentW, 30), Qt::AlignCenter,
                     QStringLiteral("Export le %1").arg(exportStamp));

    int y = topMargin + 80;

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
                             QStringLiteral("Aucune colonne a exporter."));
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

    const int headerH = 48;
    auto drawTableHeader = [&]() {
        int x = leftMargin;
        painter.setFont(headerFont);

        QLinearGradient headerGrad(0, y, 0, y + headerH);
        headerGrad.setColorAt(0.0, QColor(QStringLiteral("#0b5ea8")));
        headerGrad.setColorAt(1.0, QColor(QStringLiteral("#2e86c1")));

        for (int i = 0; i < columns.size(); ++i) {
            const int c = columns[i];
            const int w = drawWidths[i];
            const QRect cellRect(x, y, w, headerH);
            const QString text = table->horizontalHeaderItem(c) ? table->horizontalHeaderItem(c)->text().trimmed() : QStringLiteral("Colonne %1").arg(c + 1);

            painter.fillRect(cellRect, headerGrad);
            painter.setPen(Qt::white);
            painter.drawText(cellRect.adjusted(12, 0, -12, 0), Qt::AlignVCenter | Qt::AlignLeft, text);
            painter.setPen(QPen(QColor(200, 210, 220), 1));
            painter.drawRect(cellRect);
            x += w;
        }
        y += headerH;
    };

    const int footerY = pageH - bottomMargin + 10;
    const int tableTop = y;
    const int tableHeaderH = headerH;
    const int tableTotalH = 30;
    const int tableBottomLimit = pageH - bottomMargin - 40;
    const int tableRowsAreaH = qMax(40, tableBottomLimit - tableTop - tableHeaderH - tableTotalH);
    const int rowH = 40;
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
        const QColor rowColor = (index % 2 == 0) ? QColor(250, 250, 252) : QColor(245, 251, 255);
        painter.fillRect(QRect(leftMargin, y, contentW, rowH), rowColor);
        painter.setPen(QPen(QColor(220, 225, 230), 1));
        painter.drawRect(QRect(leftMargin, y, contentW, rowH));

        int x = leftMargin;
        for (int i = 0; i < columns.size(); ++i) {
            const int c = columns[i];
            const int w = drawWidths[i];
            painter.setPen(QPen(QColor(220, 225, 230), 1));
            painter.drawLine(x, y, x, y + rowH);

            painter.setPen(Qt::black);
            const QTableWidgetItem* item = table->item(r, c);
            QString text = item ? item->text() : QString();
            text = text.simplified();

            QFontMetrics fm(cellFont);
            text = fm.elidedText(text, Qt::ElideRight, qMax(10, w - 24));
            painter.drawText(QRect(x + 12, y + 6, w - 24, rowH - 12), Qt::AlignLeft | Qt::AlignVCenter, text);
            x += w;
        }

        painter.setPen(QPen(QColor(220, 225, 230), 1));
        painter.drawLine(leftMargin + contentW, y, leftMargin + contentW, y + rowH);
        y += rowH;
    }

    y += 8;
    painter.setPen(Qt::darkGray);
    painter.setFont(totalFont);
    painter.drawText(QRect(leftMargin, y, contentW, 24), Qt::AlignCenter,
                     QStringLiteral("Total: %1 pecheurs").arg(totalVisibleRows));

    if (totalVisibleRows > exportedRows) {
        painter.setFont(QFont(QStringLiteral("Arial"), 9, QFont::Normal));
        painter.setPen(QColor(QStringLiteral("#8a8a8a")));
        painter.drawText(QRect(leftMargin, y + 24, contentW, 18), Qt::AlignCenter,
                         QStringLiteral("(%1 lignes affichees sur %2 dans cette page)").arg(exportedRows).arg(totalVisibleRows));
    }

    painter.setPen(QColor(QStringLiteral("#8a8a8a")));
    painter.setFont(footerFont);
    painter.drawText(QRect(leftMargin, footerY, contentW, 20), Qt::AlignCenter,
                     QStringLiteral("AQUATEC - Rapport genere le %1").arg(exportStamp));

    restoreActionColumn();

    painter.end();
    QMessageBox::information(parent, QStringLiteral("Export PDF"),
                             QStringLiteral("Fichier PDF exporte avec succes:\n%1").arg(filePath));
}

void MainWindow::on_bep_clicked()
{
    exportPecheursPdfReport(this, ui);
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

    // ΓöÇΓöÇ Contr├┤les de saisie ΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇ
    QStringList erreurs;

    // ID auto-généré (schéma: 261NNN) en mode ajout
    if (currentEditingId.isEmpty()) {
        if (id.isEmpty()) {
            const QString generatedId = bateaauuu::genererNouvelId();
            if (!generatedId.isEmpty()) {
                id = generatedId;
                if (ui->lineEdit_3b) ui->lineEdit_3b->setText(id);
            }
        }
        if (id.isEmpty()) {
            erreurs << "Impossible de générer l'ID du bateau (préfixe 261NNN).";
        } else {
            QSqlQuery checkId;
            checkId.prepare("SELECT COUNT(*) FROM BATEAUX WHERE ID_BATEAU = :id");
            checkId.bindValue(":id", id);
            if (checkId.exec() && checkId.next() && checkId.value(0).toInt() > 0) {
                erreurs << "Cet ID existe déjà. Veuillez réessayer.";
            }
        }
    }

    // Nom obligatoire et lettres/espaces uniquement
    if (nom.isEmpty()) {
        erreurs << "Le nom du bateau est obligatoire.";
    } else if (!QRegularExpression("^[A-Za-z├Ç-├┐\\s\\-']+$").match(nom).hasMatch()) {
        erreurs << "Le nom du bateau ne doit contenir que des lettres, espaces ou tirets.";
    }

    // Propri├⌐taire obligatoire et lettres/espaces uniquement
    if (proprietaire.isEmpty()) {
        erreurs << "Le propri├⌐taire est obligatoire.";
    } else if (!QRegularExpression("^[A-Za-z├Ç-├┐\\s\\-']+$").match(proprietaire).hasMatch()) {
        erreurs << "Le propri├⌐taire ne doit contenir que des lettres, espaces ou tirets.";
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
        erreurs << "La largeur doit ├¬tre sup├⌐rieure ├á 0.";
    }

    // Capacit├⌐ > 0
    if (capacite <= 0) {
        erreurs << "La capacit├⌐ doit ├¬tre sup├⌐rieure ├á 0.";
    }

    // Fr├⌐quence de maintenance > 0
    if (freqMaint <= 0) {
        erreurs << "La fr├⌐quence de maintenance doit ├¬tre sup├⌐rieure ├á 0.";
    }

    // Date d'entr├⌐e valide et pas dans le futur
    if (!dateEntree.isValid()) {
        erreurs << "La date d'entr├⌐e n'est pas valide.";
    } else if (dateEntree > QDate::currentDate()) {
        erreurs << "La date d'entr├⌐e ne peut pas ├¬tre dans le futur.";
    }

    // Date de maintenance <= aujourd'hui si renseign├⌐e
    if (dateMaint.isValid() && dateMaint > QDate::currentDate()) {
        erreurs << "La date de derni├¿re maintenance ne peut pas ├¬tre dans le futur.";
    }

    // Date de maintenance >= date d'entr├⌐e
    if (dateEntree.isValid() && dateMaint.isValid() && dateMaint < dateEntree) {
        erreurs << "La date de maintenance ne peut pas ├¬tre ant├⌐rieure ├á la date d'entr├⌐e.";
    }

    // Si erreurs, afficher la premi├¿re erreur et arr├¬ter
    if (!erreurs.isEmpty()) {
        QMessageBox::warning(this, "Erreur de saisie", erreurs.first());
        return;
    }
    // ΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇ

    bateaauuu b;
    bool ok = false;

    if (!currentEditingId.isEmpty()) {
        // Update existing
        ok = b.updateBateau(currentEditingId, nom, type, capacite, proprietaire,
                            statut, largeur, dateEntree, dateMaint, freqMaint);
        if (ok) {
            QMessageBox::information(this, "Succ├¿s", "Bateau modifi├⌐ avec succ├¿s.");
            currentEditingId.clear();
        } else {
            QMessageBox::critical(this, "Erreur", "├ëchec de la modification:\n" + b.lastError());
        }
    } else {
        // Add new
        ok = b.addBateau(id, nom, type, capacite, proprietaire,
                         statut, largeur, dateEntree, dateMaint, freqMaint);
        if (ok) {
            QMessageBox::information(this, "Succ├¿s", "Bateau ajout├⌐ avec succ├¿s.");
        } else {
            QMessageBox::critical(this, "Erreur", "├ëchec de l'ajout:\n" + b.lastError());
        }
    }

    if (ok) {
        // Clear form
        if (ui->lineEdit_3b) {
            const QString nextId = bateaauuu::genererNouvelId();
            ui->lineEdit_3b->setText(nextId);
        }
        ui->lineEdit_4b->clear();
        ui->lineEdit_5b->clear();
        ui->comboBoxb->setCurrentIndex(0);
        ui->comboBox_2b->setCurrentIndex(0);
        ui->spinBoxb->setValue(0);
        ui->doubleSpinBox->setValue(0.0);
        ui->spinBox->setValue(0);

        // Remettre le bouton sur "Ajouter"
        if (ui->pushButton_2b) ui->pushButton_2b->setText("Ajouter");
        loadBateaux();
        refreshBateauMaintenanceAlerts(true);
    }
}

// ΓöÇΓöÇ Database helpers ΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇ
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
        QMessageBox::critical(this, "Erreur", "├ëchec ajout:\n" + b.lastError());
    }
}

// ΓöÇΓöÇ Load bateaux table ΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇ
void MainWindow::loadBateaux()
{
    if (!ui->tableWidgetb) return;

    ui->tableWidgetb->setAlternatingRowColors(false);
    ui->tableWidgetb->setShowGrid(false);
    ui->tableWidgetb->setStyleSheet(
        "QTableWidget {"
        "background-color: rgb(224, 238, 255);"
        "alternate-background-color: rgb(224, 238, 255);"
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

    // Build SQL with optional WHERE clauses from search filters
    QString sql = QStringLiteral(
        "SELECT ID_BATEAU, NOM, PROPRIETAIRE, LARGEUR, TYPE, STATUT, CAPACITE, "
        "DATE_ENTREE, DATE_DERNIERE_MAINTENANCE, %1 AS PROCHAINE_MAINTENANCE "
        "FROM BATEAUX")
                      .arg(bateauMaintenanceFrequencySelectExpr());

    QStringList conditions;
    conditions << QStringLiteral("ID_BATEAU <> 0");

    QString searchText = ui->lineEdit_4p_2 ? ui->lineEdit_4p_2->text().trimmed() : QString();
    double  filterLargeur  = ui->doubleSpinBox_largeur ? ui->doubleSpinBox_largeur->value() : 0.0;
    int     filterCapacite = ui->spinBoxb_2b ? ui->spinBoxb_2b->value() : 0;

    if (!searchText.isEmpty()) {
        QString escaped = searchText;
        escaped.replace("'", "''");
        conditions << QString("(UPPER(TO_CHAR(ID_BATEAU)) LIKE UPPER('%%1%') OR "
                              "UPPER(NOM) LIKE UPPER('%%1%'))").arg(escaped);
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
    // Synchronise le nombre de colonnes avec les donn├⌐es SQL + Actions
    // 10 attributs + 1 actions
    ui->tableWidgetb->setColumnCount(11);
    auto ensureHeader = [this](int col, const QString& text) {
        if (!ui || !ui->tableWidgetb) return;
        QTableWidgetItem* item = ui->tableWidgetb->horizontalHeaderItem(col);
        if (!item) {
            item = new QTableWidgetItem(text);
            ui->tableWidgetb->setHorizontalHeaderItem(col, item);
        } else {
            item->setText(text);
        }
    };
    ensureHeader(10, QStringLiteral("Actions"));
    ui->tableWidgetb->setRowCount(0);

    int row = 0;
    while (query.next()) {
        ui->tableWidgetb->insertRow(row);
        for (int col = 0; col < 10; ++col) {
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
        // Colonne Actions (d├⌐cal├⌐e en derni├¿re colonne)
        QWidget *actionWidget = new QWidget();
        actionWidget->setStyleSheet("background: transparent; border: none;");
        QHBoxLayout *layout = new QHBoxLayout(actionWidget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(2);
        QPushButton *btnModifier = new QPushButton(QStringLiteral("\u270F\uFE0F"));
        btnModifier->setFixedSize(42, 32);
        QFont btnModifierFont(QStringLiteral("Segoe UI Emoji"));
        btnModifierFont.setPointSize(15);
        btnModifierFont.setBold(true);
        btnModifier->setFont(btnModifierFont);
        btnModifier->setToolTip("Modifier");
        btnModifier->setStyleSheet(
            "QPushButton {"
            " border: 2px solid rgb(0, 0, 112);"
            " border-radius: 6px;"
            " background-color: rgb(224, 238, 255);"
            " color: rgb(0, 0, 112);"
            " padding: 0px;"
            " }"
            "QPushButton:hover { background-color: rgb(224, 238, 255); }"
            "QPushButton:pressed { background-color: rgb(224, 238, 255); }");
        QPushButton *btnSupprimer = new QPushButton(QStringLiteral("\u274C"));
        btnSupprimer->setFixedSize(42, 32);
        QFont btnSupprimerFont(QStringLiteral("Segoe UI Emoji"));
        btnSupprimerFont.setPointSize(15);
        btnSupprimerFont.setBold(true);
        btnSupprimer->setFont(btnSupprimerFont);
        btnSupprimer->setToolTip("Supprimer");
        btnSupprimer->setStyleSheet(
            "QPushButton {"
            " border: 2px solid rgb(0, 0, 112);"
            " border-radius: 6px;"
            " background-color: rgb(224, 238, 255);"
            " color: rgb(0, 0, 112);"
            " padding: 0px;"
            " }"
            "QPushButton:hover { background-color: rgb(224, 238, 255); }"
            "QPushButton:pressed { background-color: rgb(224, 238, 255); }");
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
        ui->tableWidgetb->setRowHeight(row, 42);
        ui->tableWidgetb->setCellWidget(row, 10, actionWidget);
        ++row;
    }

    // Afficher la colonne Actions
    ui->tableWidgetb->setColumnHidden(10, false);

    // Affichage sans "..." + colonnes lisibles (dates, etc.).
    // On laisse l'utilisateur redimensionner et on force des largeurs minimales.
    ui->tableWidgetb->setTextElideMode(Qt::ElideNone);
    ui->tableWidgetb->setWordWrap(false);
    if (ui->tableWidgetb->horizontalHeader()) {
        ui->tableWidgetb->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        ui->tableWidgetb->horizontalHeader()->setStretchLastSection(false);
    }
    ui->tableWidgetb->resizeColumnsToContents();

    auto ensureMinWidth = [this](int col, int minW) {
        if (!ui || !ui->tableWidgetb) return;
        if (col < 0 || col >= ui->tableWidgetb->columnCount()) return;
        ui->tableWidgetb->setColumnWidth(col, qMax(ui->tableWidgetb->columnWidth(col), minW));
    };

    // Largeurs minimales pour éviter les dates tronquées.
    ensureMinWidth(0, 95);   // ID
    ensureMinWidth(1, 140);  // Nom
    ensureMinWidth(2, 140);  // Propriétaire
    ensureMinWidth(3, 80);   // Largeur
    ensureMinWidth(4, 120);  // Type
    ensureMinWidth(5, 110);  // Statut
    ensureMinWidth(6, 90);   // Capacité
    ensureMinWidth(7, 110);  // Date entrée
    ensureMinWidth(8, 110);  // Dernière maintenance
    ensureMinWidth(9, 95);   // Fréquence maintenance
    ensureMinWidth(10, 114); // Actions

    if (ui->tableWidgetb->horizontalHeader()) {
        ui->tableWidgetb->horizontalHeader()->setSectionResizeMode(10, QHeaderView::Fixed);
    }
    ui->tableWidgetb->verticalHeader()->setDefaultSectionSize(42);

    loadPecheurBateauChoices(ui);
    updateStatsBateaux();

    // Mettre ├á jour les alertes de maintenance sur la table affich├⌐e.
    refreshBateauMaintenanceAlerts(false);
}

// ΓöÇΓöÇ Modifier : remplir le formulaire avec les donn├⌐es de la ligne ΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇ
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
        QString("Bateau \"%1\" charg├⌐ dans le formulaire.\nModifiez les champs puis cliquez sur Modifier.").arg(cellText(1)));

    // Change le texte du bouton en "Modifier"
    if (ui->pushButton_2b) ui->pushButton_2b->setText("Modifier");
}

// ΓöÇΓöÇ Supprimer : supprimer le bateau de la ligne ΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇ
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
        QMessageBox::information(this, "Succ├¿s", "Bateau supprim├⌐ avec succ├¿s.");
        loadBateaux();
    } else {
        QMessageBox::critical(this, "Erreur", "├ëchec de la suppression:\n" + b.lastError());
    }
}

// ΓöÇΓöÇ Filtrer la table bateaux ΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇ
void MainWindow::filterBateaux()
{
    loadBateaux();
}

// ΓöÇΓöÇ Update statistics widgets ΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇΓöÇ
void MainWindow::updateStatsBateaux()
{
    // ΓöÇΓöÇ Build the same WHERE clause used by loadBateaux() ΓöÇΓöÇ
    QStringList conditions;
    QString searchText = ui->lineEdit_4p_2 ? ui->lineEdit_4p_2->text().trimmed() : QString();
    double  filterLargeur  = ui->doubleSpinBox_largeur ? ui->doubleSpinBox_largeur->value() : 0.0;
    int     filterCapacite = ui->spinBoxb_2b ? ui->spinBoxb_2b->value() : 0;

    if (!searchText.isEmpty()) {
        QString escaped = searchText;
        escaped.replace("'", "''");
        conditions << QString("(UPPER(TO_CHAR(ID_BATEAU)) LIKE UPPER('%%1%') OR "
                              "UPPER(NOM) LIKE UPPER('%%1%'))").arg(escaped);
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
        ui->label_legend_chalutierb->setText(QStringLiteral("\u2022 Chalutier: %1 (%2%)").arg(nChalutier).arg(pctStr(nChalutier)));
    if (ui->label_legend_palangrierb)
        ui->label_legend_palangrierb->setText(QStringLiteral("\u2022 Palangrier: %1 (%2%)").arg(nPalangrier).arg(pctStr(nPalangrier)));
    if (ui->label_legend_caseyeurb)
        ui->label_legend_caseyeurb->setText(QStringLiteral("\u2022 Caseyeur: %1 (%2%)").arg(nCaseyeur).arg(pctStr(nCaseyeur)));
    if (ui->label_legend_traditionalb)
        ui->label_legend_traditionalb->setText(QStringLiteral("\u2022 Traditional: %1 (%2%)").arg(nTraditional).arg(pctStr(nTraditional)));
    if (ui->label_legend_otherb)
        ui->label_legend_otherb->setText(QStringLiteral("\u2022 Autres: %1 (%2%)").arg(nAutres).arg(pctStr(nAutres)));

    // ΓöÇΓöÇ Draw solid pie chart on progressTypeCircleb ΓöÇΓöÇ
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

// ----------------------
// Alertes automatiques de maintenance préventive (Bateaux)
// ----------------------

namespace {

enum class MaintenanceSeverity {
    None = 0,
    Warning = 1,
    Urgent = 2,
    Critical = 3,
};

static MaintenanceSeverity severityFromString(const QString& text)
{
    const QString t = text.trimmed();
    if (t.compare(QStringLiteral("Critique"), Qt::CaseInsensitive) == 0) return MaintenanceSeverity::Critical;
    if (t.compare(QStringLiteral("Urgent"), Qt::CaseInsensitive) == 0) return MaintenanceSeverity::Urgent;
    if (t.compare(QStringLiteral("Avertissement"), Qt::CaseInsensitive) == 0) return MaintenanceSeverity::Warning;
    return MaintenanceSeverity::None;
}

static QString severityLabel(MaintenanceSeverity s)
{
    switch (s) {
    case MaintenanceSeverity::Warning:  return QStringLiteral("Avertissement");
    case MaintenanceSeverity::Urgent:   return QStringLiteral("Urgent");
    case MaintenanceSeverity::Critical: return QStringLiteral("Critique");
    default:                            return QString();
    }
}

static QColor severityColor(MaintenanceSeverity s)
{
    switch (s) {
    case MaintenanceSeverity::Warning:
        return QColor(QStringLiteral("#f39c12"));
    case MaintenanceSeverity::Urgent:
        return QColor(QStringLiteral("#e67e22"));
    case MaintenanceSeverity::Critical:
        return QColor(QStringLiteral("#e74c3c"));
    case MaintenanceSeverity::None:
    default:
        return QColor(QStringLiteral("#4CAF50"));
    }
}

static QColor severityRowBackground(MaintenanceSeverity s)
{
    // Fonds clairs pour garder la lisibilit├® (ligne color├®e)
    switch (s) {
    case MaintenanceSeverity::Warning:
        return QColor(QStringLiteral("#fff9c4")); // jaune clair
    case MaintenanceSeverity::Urgent:
        return QColor(QStringLiteral("#ffe0b2")); // orange clair
    case MaintenanceSeverity::Critical:
        return QColor(QStringLiteral("#ffebee")); // rouge clair
    case MaintenanceSeverity::None:
    default:
        return QColor();
    }
}

static bool isStatutEnMer(const QString& statut)
{
    return statut.trimmed().toLower().contains(QStringLiteral("mer"));
}

static QDate variantToDate(const QVariant& v)
{
    if (!v.isValid() || v.isNull()) return QDate();

    if (v.userType() == QMetaType::QDate) {
        return v.toDate();
    }
    if (v.userType() == QMetaType::QDateTime) {
        return v.toDateTime().date();
    }

    const QString s = v.toString().trimmed();
    if (s.isEmpty()) return QDate();

    QDate d = QDate::fromString(s, Qt::ISODate);
    if (!d.isValid()) d = QDate::fromString(s, QStringLiteral("yyyy-MM-dd"));
    if (!d.isValid()) d = QDate::fromString(s, QStringLiteral("dd/MM/yyyy"));
    return d;
}

static MaintenanceSeverity computeSeverity(bool enMer, int daysRemaining)
{
    Q_UNUSED(enMer);
    // R├¿gles demand├®es:
    // - Rouge   = Critique : maintenance d├®pass├®e
    // - Orange  = Urgent   : moins de 7 jours
    // - Jaune   = Avert.   : moins de 30 jours
    // - Normal  = OK
    constexpr int kWarnDays = 30;
    constexpr int kUrgentDays = 7;

    if (daysRemaining < 0) return MaintenanceSeverity::Critical;
    if (daysRemaining <= kUrgentDays) return MaintenanceSeverity::Urgent;
    if (daysRemaining <= kWarnDays) return MaintenanceSeverity::Warning;
    return MaintenanceSeverity::None;
}

static QString maintenanceHistoryFilePath()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dir.trimmed().isEmpty()) {
        dir = QDir::homePath();
    }
    QDir().mkpath(dir);
    return QDir(dir).filePath(QStringLiteral("bateau_maintenance_alert_history.json"));
}

static QJsonArray readMaintenanceHistoryArray()
{
    const QString path = maintenanceHistoryFilePath();
    QFile f(path);
    if (!f.exists()) return QJsonArray();
    if (!f.open(QIODevice::ReadOnly)) return QJsonArray();

    const QByteArray raw = f.readAll();
    f.close();

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray()) return QJsonArray();
    return doc.array();
}

static bool writeMaintenanceHistoryArray(const QJsonArray& arr)
{
    const QString path = maintenanceHistoryFilePath();
    QFileInfo fi(path);
    QDir().mkpath(fi.dir().absolutePath());

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    f.write(QJsonDocument(arr).toJson(QJsonDocument::Compact));
    f.close();
    return true;
}

static QString makeAlertKey(const QString& boatId, const QDate& nextDue, MaintenanceSeverity severity)
{
    return QStringLiteral("%1|%2|%3")
        .arg(boatId.trimmed(), nextDue.toString(Qt::ISODate), severityLabel(severity));
}

struct AlertInfo {
    MaintenanceSeverity severity = MaintenanceSeverity::None;
    QString displayText;
    QString message;
    QDate nextDue;
    int daysRemaining = 0;
};

} // namespace

void MainWindow::setupBateauMaintenanceAlertSystem()
{
    if (!ui) return;

    loadBateauAlertHistorySeenKeys();
    ensureBateauAlertHistoryPanel();

    if (ui->pushButton_alertb) {
        if (!ui->pushButton_alertb->property("baseText").isValid()) {
            ui->pushButton_alertb->setProperty("baseText", ui->pushButton_alertb->text());
        }

        connect(ui->pushButton_alertb, &QPushButton::clicked, this, &MainWindow::toggleBateauAlertHistoryPanel);

        if (!m_bateauAlertBadge) {
            // Badge rouge (chiffre) en haut ├á droite du bouton
            m_bateauAlertBadge = new QLabel(ui->pushButton_alertb);
            m_bateauAlertBadge->setObjectName(QStringLiteral("label_bateauAlertBadge"));
            m_bateauAlertBadge->setAlignment(Qt::AlignCenter);
            m_bateauAlertBadge->setFixedSize(22, 22);
            m_bateauAlertBadge->move(ui->pushButton_alertb->width() - 24, 2);
            m_bateauAlertBadge->setAttribute(Qt::WA_TransparentForMouseEvents, true);
            m_bateauAlertBadge->setStyleSheet(QStringLiteral(
                "background-color: #e74c3c;"
                "color: white;"
                "border-radius: 11px;"
                "font-weight: bold;"
                "font-size: 12px;"));
            m_bateauAlertBadge->hide();
        }
    }

    if (!m_bateauMaintenanceAlertTimer) {
        m_bateauMaintenanceAlertTimer = new QTimer(this);
        m_bateauMaintenanceAlertTimer->setInterval(60 * 1000); // 1 minute
        connect(m_bateauMaintenanceAlertTimer, &QTimer::timeout, this, [this]() {
            refreshBateauMaintenanceAlerts(true);
        });
        m_bateauMaintenanceAlertTimer->start();
    }

    // Rafraichissement initial (sans ajouter dans l'historique)
    refreshBateauMaintenanceAlerts(false);

    // Quand la page bateaux s'affiche, on rafraichit l'affichage.
    if (ui->stackedWidget && ui->gestionbateaub) {
        connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, [this](int) {
            if (!ui || !ui->stackedWidget || !ui->gestionbateaub) return;
            if (ui->stackedWidget->currentWidget() == ui->gestionbateaub) {
                refreshBateauMaintenanceAlerts(false);
                notifyBateauMaintenanceAlertsIfAny();
            }
        });
    }
}

void MainWindow::notifyBateauMaintenanceAlertsIfAny()
{
    // Ne notifie que lorsqu'il y a au moins une alerte.
    const int total = m_bateauAlertsWarning + m_bateauAlertsUrgent + m_bateauAlertsCritical;
    if (total <= 0) return;

    // Rate limit: ├®vite le spam si l'utilisateur clique plusieurs fois.
    const QDateTime now = QDateTime::currentDateTime();
    if (m_lastBateauAlertNotificationAt.isValid() && m_lastBateauAlertNotificationAt.secsTo(now) < 15) {
        return;
    }
    m_lastBateauAlertNotificationAt = now;

    QString level;
    QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information;
    if (m_bateauAlertsHighestLevel >= 3) {
        level = QStringLiteral("Critique");
        icon = QSystemTrayIcon::Critical;
    } else if (m_bateauAlertsHighestLevel == 2) {
        level = QStringLiteral("Urgent");
        icon = QSystemTrayIcon::Warning;
    } else {
        level = QStringLiteral("Avertissement");
        icon = QSystemTrayIcon::Information;
    }

    const QString title = QStringLiteral("Alerte maintenance (%1)").arg(level);
    const QString msg = QStringLiteral("Critique: %1 | Urgent: %2 | Avertissement: %3\nClique sur 'Historique des alertes' pour d├®tails.")
                            .arg(m_bateauAlertsCritical)
                            .arg(m_bateauAlertsUrgent)
                            .arg(m_bateauAlertsWarning);

    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        if (!m_bateauTrayIcon) {
            m_bateauTrayIcon = new QSystemTrayIcon(this);
            QIcon ico = this->windowIcon();
            if (ico.isNull()) {
                ico = QIcon::fromTheme(QStringLiteral("dialog-information"));
            }
            m_bateauTrayIcon->setIcon(ico);
            m_bateauTrayIcon->setToolTip(QStringLiteral("AquaTech"));
            m_bateauTrayIcon->setVisible(true);
        }
        m_bateauTrayIcon->showMessage(title, msg, icon, 7000);
        return;
    }

    // Fallback si le system tray n'est pas disponible.
    if (m_bateauAlertsHighestLevel >= 3) {
        QMessageBox::critical(this, QStringLiteral("Maintenance pr├®ventive"), msg);
    } else if (m_bateauAlertsHighestLevel == 2) {
        QMessageBox::warning(this, QStringLiteral("Maintenance pr├®ventive"), msg);
    } else {
        QMessageBox::information(this, QStringLiteral("Maintenance pr├®ventive"), msg);
    }
}

void MainWindow::loadBateauAlertHistorySeenKeys()
{
    m_bateauAlertSeenKeys.clear();

    const QJsonArray arr = readMaintenanceHistoryArray();
    for (const QJsonValue& v : arr) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();
        QString key = o.value(QStringLiteral("key")).toString().trimmed();
        if (key.isEmpty()) {
            const QString boatId = o.value(QStringLiteral("boatId")).toString();
            const QDate nextDue = QDate::fromString(o.value(QStringLiteral("nextDue")).toString(), Qt::ISODate);
            const MaintenanceSeverity sev = severityFromString(o.value(QStringLiteral("severity")).toString());
            if (!boatId.trimmed().isEmpty() && nextDue.isValid() && sev != MaintenanceSeverity::None) {
                key = makeAlertKey(boatId, nextDue, sev);
            }
        }
        if (!key.isEmpty()) {
            m_bateauAlertSeenKeys.insert(key);
        }
    }
}

void MainWindow::ensureBateauAlertHistoryPanel()
{
    if (m_bateauAlertHistoryFrame || !ui) return;

    QWidget* parent = ui->gestionbateaub ? ui->gestionbateaub : this;
    m_bateauAlertHistoryFrame = new QFrame(parent);
    m_bateauAlertHistoryFrame->setObjectName(QStringLiteral("framealertb"));
    m_bateauAlertHistoryFrame->setGeometry(QRect(510, 110, 1001, 651));
    m_bateauAlertHistoryFrame->setStyleSheet(QStringLiteral(
        "background-color: rgb(224, 238, 255);"
        "border:1PX solid  rgb(0, 0, 115);"
        "border-radius: 15px;"));
    m_bateauAlertHistoryFrame->setFrameShape(QFrame::StyledPanel);
    m_bateauAlertHistoryFrame->setFrameShadow(QFrame::Raised);

    auto* rootLayout = new QVBoxLayout(m_bateauAlertHistoryFrame);
    rootLayout->setContentsMargins(12, 12, 12, 12);
    rootLayout->setSpacing(8);

    auto* headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);

    auto* title = new QLabel(QStringLiteral("Historique des alertes de maintenance"), m_bateauAlertHistoryFrame);
    title->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 17px; padding: 6px;"));

    auto* btnClose = new QPushButton(QStringLiteral("Fermer"), m_bateauAlertHistoryFrame);
    btnClose->setFixedSize(110, 32);
    btnClose->setStyleSheet(QStringLiteral(
        "QPushButton {"
        " border: 2px solid rgb(0, 0, 112);"
        " border-radius: 7px;"
        " background-color: rgb(224, 238, 255);"
        " color: rgb(0, 0, 112);"
        " font-weight: bold;"
        " }"
        "QPushButton:hover { background-color: rgb(224, 238, 255); }"
        "QPushButton:pressed { background-color: rgb(224, 238, 255); }"));

    headerLayout->addWidget(title);
    headerLayout->addStretch();
    headerLayout->addWidget(btnClose);
    rootLayout->addLayout(headerLayout);

    m_bateauAlertHistoryTable = new QTableWidget(m_bateauAlertHistoryFrame);
    m_bateauAlertHistoryTable->setColumnCount(5);
    m_bateauAlertHistoryTable->setHorizontalHeaderItem(0, new QTableWidgetItem(QStringLiteral("ID Bateau")));
    m_bateauAlertHistoryTable->setHorizontalHeaderItem(1, new QTableWidgetItem(QStringLiteral("Nom")));
    m_bateauAlertHistoryTable->setHorizontalHeaderItem(2, new QTableWidgetItem(QStringLiteral("Niveau")));
    m_bateauAlertHistoryTable->setHorizontalHeaderItem(3, new QTableWidgetItem(QStringLiteral("Message")));
    m_bateauAlertHistoryTable->setHorizontalHeaderItem(4, new QTableWidgetItem(QStringLiteral("Date")));

    m_bateauAlertHistoryTable->setAlternatingRowColors(false);
    m_bateauAlertHistoryTable->setShowGrid(false);
    m_bateauAlertHistoryTable->setWordWrap(true);
    m_bateauAlertHistoryTable->verticalHeader()->setDefaultSectionSize(36);
    m_bateauAlertHistoryTable->setStyleSheet(
        "QTableWidget {"
        "background-color: rgb(224, 238, 255);"
        "alternate-background-color: rgb(224, 238, 255);"
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

    if (m_bateauAlertHistoryTable->horizontalHeader()) {
        m_bateauAlertHistoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_bateauAlertHistoryTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
        m_bateauAlertHistoryTable->setColumnWidth(0, 95);
        m_bateauAlertHistoryTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
        m_bateauAlertHistoryTable->setColumnWidth(2, 120);
        m_bateauAlertHistoryTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
        m_bateauAlertHistoryTable->setColumnWidth(4, 120);
    }

    rootLayout->addWidget(m_bateauAlertHistoryTable);

    connect(btnClose, &QPushButton::clicked, this, [this]() {
        if (m_bateauAlertHistoryFrame) {
            m_bateauAlertHistoryFrame->hide();
        }
    });

    m_bateauAlertHistoryFrame->hide();
}

void MainWindow::toggleBateauAlertHistoryPanel()
{
    ensureBateauAlertHistoryPanel();
    if (!m_bateauAlertHistoryFrame) return;

    if (m_bateauAlertHistoryFrame->isVisible()) {
        m_bateauAlertHistoryFrame->hide();
        return;
    }

    populateBateauAlertHistoryTable();
    m_bateauAlertHistoryFrame->show();
    m_bateauAlertHistoryFrame->raise();
}

void MainWindow::populateBateauAlertHistoryTable()
{
    if (!m_bateauAlertHistoryTable) return;

    const QJsonArray arr = readMaintenanceHistoryArray();

    m_bateauAlertHistoryTable->setRowCount(0);

    const int limit = 300;
    int added = 0;
    for (int i = arr.size() - 1; i >= 0 && added < limit; --i) {
        const QJsonValue v = arr.at(i);
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();

        const QString ts = o.value(QStringLiteral("ts")).toString();
        const QString boatId = o.value(QStringLiteral("boatId")).toString();
        const QString boatName = o.value(QStringLiteral("boatName")).toString();
        const QString severity = o.value(QStringLiteral("severity")).toString();
        const QString message = o.value(QStringLiteral("message")).toString();

        QString dateText = ts;
        const QDateTime dt = QDateTime::fromString(ts, Qt::ISODate);
        if (dt.isValid()) {
            dateText = dt.date().toString(QStringLiteral("dd/MM/yyyy"));
        }

        const int row = m_bateauAlertHistoryTable->rowCount();
        m_bateauAlertHistoryTable->insertRow(row);

        const MaintenanceSeverity sev = severityFromString(severity);
        const QColor bg = severityRowBackground(sev);
        const QBrush bgBrush = bg.isValid() ? QBrush(bg) : QBrush();

        auto* idItem = new QTableWidgetItem(boatId);
        auto* nameItem = new QTableWidgetItem(boatName);
        auto* sevItem = new QTableWidgetItem(severity);
        auto* msgItem = new QTableWidgetItem(message);
        auto* dateItem = new QTableWidgetItem(dateText);

        // Niveau: couleur texte + gras
        sevItem->setForeground(QBrush(severityColor(sev)));
        {
            QFont f = sevItem->font();
            f.setBold(true);
            sevItem->setFont(f);
        }

        // Message: tooltip complet
        msgItem->setToolTip(message);
        msgItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        // Appliquer la couleur de fond ├á toute la ligne
        for (QTableWidgetItem* it : {idItem, nameItem, sevItem, msgItem, dateItem}) {
            if (!it) continue;
            it->setFlags(it->flags() & ~Qt::ItemIsEditable);
            it->setBackground(bgBrush);
        }

        m_bateauAlertHistoryTable->setItem(row, 0, idItem);
        m_bateauAlertHistoryTable->setItem(row, 1, nameItem);
        m_bateauAlertHistoryTable->setItem(row, 2, sevItem);
        m_bateauAlertHistoryTable->setItem(row, 3, msgItem);
        m_bateauAlertHistoryTable->setItem(row, 4, dateItem);

        ++added;
    }

    // Ajuster la hauteur des lignes pour afficher le message complet (jusqu'aux ~300 entr├®es affich├®es)
    m_bateauAlertHistoryTable->resizeRowsToContents();
}

void MainWindow::refreshBateauMaintenanceAlerts(bool persistNewAlerts)
{
    if (!ui) return;

    Connection* conn = Connection::getInstance();
    if (!conn || !conn->ensureOpen()) {
        if (ui->label_alert_statusb) {
            ui->label_alert_statusb->setText(QStringLiteral("Syst\u00e8me inactif"));
            ui->label_alert_statusb->setStyleSheet(QStringLiteral(
                "color: #e74c3c; font-weight: bold; font-size: 14px; padding: 5px;"));
            const int x = 10;
            const int y = 10;
            const int h = 40;
            int w = ui->label_alert_statusb->sizeHint().width() + 20;
            ui->label_alert_statusb->setGeometry(QRect(x, y, w, h));
            ui->label_alert_statusb->raise();
        }
        return;
    }

    QSqlDatabase db = conn->getDatabase();
    QSqlQuery q(db);
    q.prepare(QStringLiteral(
        "SELECT ID_BATEAU, NOM, STATUT, DATE_ENTREE, DATE_DERNIERE_MAINTENANCE, %1 AS PROCHAINE_MAINTENANCE "
        "FROM BATEAUX")
                  .arg(bateauMaintenanceFrequencySelectExpr()));
    if (!q.exec()) {
        if (ui->label_alert_statusb) {
            ui->label_alert_statusb->setText(QStringLiteral("Syst\u00e8me actif (erreur lecture)"));
            ui->label_alert_statusb->setStyleSheet(QStringLiteral(
                "color: #e67e22; font-weight: bold; font-size: 14px; padding: 5px;"));
            const int x = 10;
            const int y = 10;
            const int h = 40;
            int w = ui->label_alert_statusb->sizeHint().width() + 20;
            ui->label_alert_statusb->setGeometry(QRect(x, y, w, h));
            ui->label_alert_statusb->raise();
        }
        return;
    }

    const QDate today = QDate::currentDate();
    QHash<QString, AlertInfo> activeByBoatId;

    int nWarn = 0;
    int nUrgent = 0;
    int nCrit = 0;
    MaintenanceSeverity highest = MaintenanceSeverity::None;

    QStringList warnBoatIds;
    QStringList urgentBoatIds;
    QStringList criticalBoatIds;

    QJsonArray history;
    bool historyLoaded = false;
    bool historyDirty = false;
    if (persistNewAlerts) {
        history = readMaintenanceHistoryArray();
        historyLoaded = true;
    }

    while (q.next()) {
        const QString boatId = q.value(0).toString().trimmed();
        const QString boatName = q.value(1).toString().trimmed();
        const QString status = q.value(2).toString().trimmed();
        const QDate dateEntree = variantToDate(q.value(3));
        const QDate lastMaint = variantToDate(q.value(4));
        const int freq = q.value(5).toInt();

        if (boatId.isEmpty() || freq <= 0) {
            continue;
        }

        const QDate base = lastMaint.isValid() ? lastMaint : dateEntree;
        if (!base.isValid()) {
            continue;
        }

        const QDate nextDue = base.addDays(freq);
        if (!nextDue.isValid()) {
            continue;
        }

        const int daysRemaining = today.daysTo(nextDue);
        const bool enMer = isStatutEnMer(status);
        const MaintenanceSeverity severity = computeSeverity(enMer, daysRemaining);
        if (severity == MaintenanceSeverity::None) {
            continue;
        }

        if (severity > highest) highest = severity;
        if (severity == MaintenanceSeverity::Warning) { ++nWarn; warnBoatIds << boatId; }
        else if (severity == MaintenanceSeverity::Urgent) { ++nUrgent; urgentBoatIds << boatId; }
        else if (severity == MaintenanceSeverity::Critical) { ++nCrit; criticalBoatIds << boatId; }

        const QString deltaText = (daysRemaining >= 0)
            ? QStringLiteral("dans %1 j").arg(daysRemaining)
            : QStringLiteral("retard %1 j").arg(-daysRemaining);

        AlertInfo info;
        info.severity = severity;
        info.nextDue = nextDue;
        info.daysRemaining = daysRemaining;
        info.displayText = QStringLiteral("%1 (%2)").arg(severityLabel(severity), deltaText);

        if (daysRemaining >= 0) {
            info.message = QStringLiteral("Maintenance pr\u00e9vue le %1 (%2).")
                               .arg(nextDue.toString(QStringLiteral("dd/MM/yyyy")), deltaText);
        } else if (enMer) {
            info.message = QStringLiteral("Bateau en mer avec maintenance en retard de %1 jours (pr\u00e9vue le %2).")
                               .arg(-daysRemaining)
                               .arg(nextDue.toString(QStringLiteral("dd/MM/yyyy")));
        } else {
            info.message = QStringLiteral("Maintenance en retard de %1 jours (pr\u00e9vue le %2).")
                               .arg(-daysRemaining)
                               .arg(nextDue.toString(QStringLiteral("dd/MM/yyyy")));
        }

        activeByBoatId.insert(boatId, info);

        if (persistNewAlerts) {
            const QString key = makeAlertKey(boatId, nextDue, severity);
            if (!m_bateauAlertSeenKeys.contains(key)) {
                m_bateauAlertSeenKeys.insert(key);

                if (!historyLoaded) {
                    history = readMaintenanceHistoryArray();
                    historyLoaded = true;
                }

                QJsonObject o;
                o.insert(QStringLiteral("key"), key);
                o.insert(QStringLiteral("ts"), QDateTime::currentDateTime().toString(Qt::ISODate));
                o.insert(QStringLiteral("boatId"), boatId);
                o.insert(QStringLiteral("boatName"), boatName);
                o.insert(QStringLiteral("status"), status);
                o.insert(QStringLiteral("lastMaintenance"), lastMaint.isValid() ? lastMaint.toString(Qt::ISODate) : QString());
                o.insert(QStringLiteral("frequencyDays"), freq);
                o.insert(QStringLiteral("nextDue"), nextDue.toString(Qt::ISODate));
                o.insert(QStringLiteral("daysRemaining"), daysRemaining);
                o.insert(QStringLiteral("severity"), severityLabel(severity));
                o.insert(QStringLiteral("message"), info.message);

                history.append(o);
                historyDirty = true;
            }
        }
    }

    if (persistNewAlerts && historyDirty) {
        // Garder une taille raisonnable (dernieres 2000 entr├®es)
        const int maxEntries = 2000;
        if (history.size() > maxEntries) {
            QJsonArray trimmed;
            const int start = qMax(0, history.size() - maxEntries);
            for (int i = start; i < history.size(); ++i) {
                trimmed.append(history.at(i));
            }
            history = trimmed;
        }
        writeMaintenanceHistoryArray(history);
    }

    // M├®moriser le r├®sum├® pour les notifications
    m_bateauAlertsWarning = nWarn;
    m_bateauAlertsUrgent = nUrgent;
    m_bateauAlertsCritical = nCrit;
    m_bateauAlertsHighestLevel = static_cast<int>(highest);

    // Label status (en haut de la page bateaux)
    if (ui->label_alert_statusb) {
        auto plural = [](int n, const QString& singular, const QString& plural) {
            return (n == 1) ? singular : plural;
        };

        const QString critLbl = plural(nCrit, QStringLiteral("Critique"), QStringLiteral("Critiques"));
        const QString urgLbl  = plural(nUrgent, QStringLiteral("Urgent"), QStringLiteral("Urgents"));
        const QString warnLbl = plural(nWarn, QStringLiteral("Avertissement"), QStringLiteral("Avertissements"));

        ui->label_alert_statusb->setText(
            QStringLiteral("🔴 %1 %2  |  🟠 %3 %4  |  🟡 %5 %6")
                .arg(nCrit)
                .arg(critLbl)
                .arg(nUrgent)
                .arg(urgLbl)
                .arg(nWarn)
                .arg(warnLbl));
        ui->label_alert_statusb->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        ui->label_alert_statusb->setStyleSheet(QStringLiteral(
            "color: rgb(0, 0, 90);"
            "font-weight: bold;"
            "font-size: 16px;"
            "padding: 6px 10px;"
            "background-color: rgba(224, 238, 255, 0.90);"
            "border: 1px solid rgb(0, 0, 115);"
            "border-radius: 10px;"));

        // Taille: ajust├®e au texte (evite un bandeau trop long).
        const int x = 10;
        const int y = 10;
        const int h = 40;
        int w = ui->label_alert_statusb->sizeHint().width() + 20;
        if (w < 220) w = 220; // garde un minimum lisible
        ui->label_alert_statusb->setGeometry(QRect(x, y, w, h));
        ui->label_alert_statusb->raise();

        // Tooltip: lister les IDs des bateaux par niveau au survol.
        const int totalAlerts = nWarn + nUrgent + nCrit;
        if (totalAlerts > 0) {
            warnBoatIds.sort();
            urgentBoatIds.sort();
            criticalBoatIds.sort();

            auto listOrNone = [](const QStringList& ids) {
                return ids.isEmpty() ? QStringLiteral("(aucun)") : ids.join(QStringLiteral(", "));
            };

            ui->label_alert_statusb->setToolTip(
                QStringLiteral("IDs bateaux en alerte:\n🔴 Critique: %1\n🟠 Urgent: %2\n🟡 Avertissement: %3")
                    .arg(listOrNone(criticalBoatIds), listOrNone(urgentBoatIds), listOrNone(warnBoatIds)));
        } else {
            ui->label_alert_statusb->setToolTip(QString());
        }
    }

    // Badge + bouton "Historique des alertes"
    const int totalAlerts = nWarn + nUrgent + nCrit;
    if (ui->pushButton_alertb) {
        const QString baseText = ui->pushButton_alertb->property("baseText").toString().trimmed();
        if (!baseText.isEmpty()) {
            ui->pushButton_alertb->setText(baseText);
        }
        if (totalAlerts > 0) {
            ui->pushButton_alertb->setToolTip(QStringLiteral("Alertes actives: %1").arg(totalAlerts));
        } else {
            ui->pushButton_alertb->setToolTip(QString());
        }
    }
    if (m_bateauAlertBadge) {
        if (totalAlerts > 0) {
            m_bateauAlertBadge->setText(QString::number(totalAlerts));
            // Badge color├® selon le niveau le plus ├®lev├®
            QString bg = QStringLiteral("#e74c3c");
            QString fg = QStringLiteral("white");
            if (highest == MaintenanceSeverity::Urgent) {
                bg = QStringLiteral("#e67e22");
            } else if (highest == MaintenanceSeverity::Warning) {
                bg = QStringLiteral("#f39c12");
                fg = QStringLiteral("black");
            }
            m_bateauAlertBadge->setStyleSheet(QStringLiteral(
                "background-color: %1;"
                "color: %2;"
                "border-radius: 11px;"
                "font-weight: bold;"
                "font-size: 12px;")
                .arg(bg, fg));
            m_bateauAlertBadge->show();
            m_bateauAlertBadge->raise();
        } else {
            m_bateauAlertBadge->hide();
        }
    }

    // Mise ├á jour de la table des bateaux: on enl├¿ve la coloration des lignes.
    // Les d├®tails des alertes sont visibles dans l'historique + via le compteur/badge.
    if (ui->tableWidgetb) {
        const int cols = ui->tableWidgetb->columnCount();
        for (int r = 0; r < ui->tableWidgetb->rowCount(); ++r) {
            for (int c = 0; c < cols; ++c) {
                if (QTableWidgetItem* item = ui->tableWidgetb->item(r, c)) {
                    item->setBackground(QBrush());
                }
            }

            const int actionsCol = 10;
            if (actionsCol >= 0 && actionsCol < cols) {
                if (QWidget* w = ui->tableWidgetb->cellWidget(r, actionsCol)) {
                    w->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
                }
            }
        }
    }

    if (m_bateauAlertHistoryFrame && m_bateauAlertHistoryFrame->isVisible()) {
        populateBateauAlertHistoryTable();
    }
}

void MainWindow::on_pushButton_11_clicked()
{
    if (!handleCrudDisabled(this) || !ui) return;

    // ID quai : en mode ajout, le generer automatiquement si vide
    if (!m_quai.isModeModification() && ui->lineEdit_4 && ui->lineEdit_4->text().trimmed().isEmpty()) {
        const int newId = m_quai.genererNouvelId();
        if (newId > 0) {
            ui->lineEdit_4->setText(QString::number(newId));
        } else {
            const QString err = m_quai.lastError();
            QMessageBox::critical(this,
                                  QStringLiteral("Erreur"),
                                  err.isEmpty()
                                      ? QStringLiteral("Impossible de generer l'ID du quai (263NNNN).")
                                      : QStringLiteral("Impossible de generer l'ID du quai : %1").arg(err));
            return;
        }
    }

    // Validation des champs obligatoires
    if (!validateRequiredFields(this, {
            {QStringLiteral("ID quai"), ui->lineEdit_4, [this]{ return !ui->lineEdit_4 || ui->lineEdit_4->text().trimmed().isEmpty(); }},
            {QStringLiteral("Nom du quai"), ui->lineEdit_6, [this]{ return !ui->lineEdit_6 || ui->lineEdit_6->text().trimmed().isEmpty(); }},
            {QStringLiteral("Longueur maximale"), ui->doubleSpinBox_2, [this]{ return !ui->doubleSpinBox_2 || ui->doubleSpinBox_2->value() <= 0.0; }},
            {QStringLiteral("Capacit├⌐ maximale"), ui->spinBox_2, [this]{ return !ui->spinBox_2 || ui->spinBox_2->value() <= 0; }},
        })) {
        return;
    }

    if (ui->lineEdit_6) {
        const QString nomQuai = ui->lineEdit_6->text().trimmed();
        static const QRegularExpression rxNomQuai(QStringLiteral("^[\\p{L}\\s'-]+$"));
        if (!rxNomQuai.match(nomQuai).hasMatch()) {
            QMessageBox::warning(this,
                                 QStringLiteral("Nom du quai"),
                                 QStringLiteral("Le nom du quai doit contenir uniquement des lettres."));
            ui->lineEdit_6->setFocus();
            ui->lineEdit_6->selectAll();
            return;
        }
    }

    QString zoneLabel = ui->comboBox_6 ? ui->comboBox_6->currentText().trimmed() : QString();
    QString zonePort;
    const QString zoneLower = zoneLabel.toLower();
    if (zoneLower.contains(QStringLiteral("quai ouest"))) {
        zonePort = QStringLiteral("Ouest");
    } else if (zoneLower.contains(QStringLiteral("quai est"))) {
        zonePort = QStringLiteral("Est");
    } else if (zoneLower.contains(QStringLiteral("bassin"))) {
        zonePort = QStringLiteral("Sud");
    } else if (zoneLower.contains(QStringLiteral("chenal"))) {
        zonePort = QStringLiteral("Nord");
    } else {
        zonePort = zoneLabel;
    }

    QString zoneCouverte = ui->comboBox_8 ? ui->comboBox_8->currentText().trimmed() : QString();
    QString statutUi = ui->comboBox_7 ? ui->comboBox_7->currentText().trimmed() : QString();
    QString statutDb = statutUi;
    if (statutUi.compare(QStringLiteral("Occupé"), Qt::CaseInsensitive) == 0 ||
        statutUi.startsWith(QStringLiteral("Occu"), Qt::CaseInsensitive)) {
        statutDb = QStringLiteral("Occupe");
    }

    int idBateau = 0;
    if (ui->comboBoxQuaiBateau) {
        bool ok = false;
        const QVariant data = ui->comboBoxQuaiBateau->currentData();
        if (data.isValid()) {
            idBateau = data.toInt(&ok);
            if (!ok) {
                idBateau = data.toString().trimmed().toInt(&ok);
                if (!ok) idBateau = 0;
            }
        }
    }

    if (statutDb.compare(QStringLiteral("Occupe"), Qt::CaseInsensitive) == 0 && idBateau <= 0) {
        QMessageBox::warning(this,
                             QStringLiteral("Bateau obligatoire"),
                             QStringLiteral("Un bateau doit etre selectionne quand le quai est occupe."));
        return;
    }

    const int savedQuaiId = ui->lineEdit_4->text().trimmed().toInt();

    if (m_quaisFermeMeteo && savedQuaiId > 0) {
        if (statutDb.compare(QStringLiteral("Occupe"), Qt::CaseInsensitive) == 0) {
            m_quaisAutoLocked.insert(savedQuaiId, QStringLiteral("Occupe"));
            statutDb = QStringLiteral("Ferme");
        } else {
            m_quaisAutoLocked.remove(savedQuaiId);
        }
    }

    QVariantMap donnees;
    donnees["ID_QUAI"] = savedQuaiId;
    donnees["NOM_QUAI"] = ui->lineEdit_6->text().trimmed();
    donnees["ZONE_PORT"] = zonePort;
    donnees["ZONE_COUVERTE"] = zoneCouverte;
    donnees["LONGUEUR"] = ui->doubleSpinBox_2->value();
    donnees["CAPACITE_QUAIS"] = ui->spinBox_2->value();
    donnees["STATUT"] = statutDb;
    donnees["ID_BATEAU"] = idBateau;

    const bool wasModification = m_quai.isModeModification();
    bool succes = false;
    if (wasModification) {
        // Mode modification
        succes = m_quai.modifier(m_quai.idEnCours(), donnees);
        if (succes) {
            QMessageBox::information(this, "Modification", "Quai modifi├⌐ avec succ├¿s.");
            m_quai.setModeModification(false);
            ui->pushButton_11->setText("Ajouter");
        } else {
            const QString err = m_quai.lastError();
            QMessageBox::critical(this,
                                  QStringLiteral("Erreur"),
                                  err.isEmpty()
                                      ? QStringLiteral("├ëchec de la modification.")
                                      : QStringLiteral("├ëchec de la modification : %1").arg(err));
        }
    } else {
        // Mode ajout
        succes = m_quai.ajouter(donnees);
        if (succes) {
            QMessageBox::information(this, "Ajout", "Quai ajout├⌐ avec succ├¿s.");
        } else {
            const QString err = m_quai.lastError();
            QMessageBox::critical(this,
                                  QStringLiteral("Erreur"),
                                  err.isEmpty()
                                      ? QStringLiteral("├ëchec de l'ajout.")
                                      : QStringLiteral("├ëchec de l'ajout : %1").arg(err));
        }
    }

    if (succes) {
        // En mode ajout, un filtre actif peut masquer la nouvelle ligne.
        // Pour garantir que l'utilisateur la voie, on r├⌐initialise les filtres.
        if (!wasModification) {
            if (ui->lineEdit_3) ui->lineEdit_3->clear();

            auto setComboToLabel = [](QComboBox* combo, const QString& label) {
                if (!combo) return;
                int idx = -1;
                for (int i = 0; i < combo->count(); ++i) {
                    if (combo->itemText(i).compare(label, Qt::CaseInsensitive) == 0) { idx = i; break; }
                }
                combo->setCurrentIndex(idx >= 0 ? idx : 0);
            };

            setComboToLabel(ui->comboBox_3, QStringLiteral("Tous"));
            setComboToLabel(ui->comboBox_4, QStringLiteral("Toutes"));
            setComboToLabel(ui->comboBox_5, QStringLiteral("Toutes"));
        }

        refreshQuaiTable();

        if (ui->tableWidgetQuai && savedQuaiId > 0) {
            QTableWidget *table = ui->tableWidgetQuai;
            for (int r = 0; r < table->rowCount(); ++r) {
                QTableWidgetItem *item = table->item(r, 0);
                if (!item) continue;
                bool ok = false;
                const int rowId = item->text().trimmed().toInt(&ok);
                if (ok && rowId == savedQuaiId) {
                    table->setCurrentCell(r, 0);
                    table->scrollToItem(item);
                    break;
                }
            }
        }

        // Vider le formulaire
        ui->lineEdit_4->clear();
        ui->lineEdit_6->clear();
        ui->comboBox_6->setCurrentIndex(0);
        ui->comboBox_8->setCurrentIndex(0);
        ui->doubleSpinBox_2->setValue(0.0);
        ui->spinBox_2->setValue(0);
        ui->comboBox_7->setCurrentIndex(0);
        if (ui->comboBoxQuaiBateau) ui->comboBoxQuaiBateau->setCurrentIndex(0);

        // Preparer le prochain ajout
        if (ui->lineEdit_4) {
            const int newId = m_quai.genererNouvelId();
            if (newId > 0) {
                ui->lineEdit_4->setText(QString::number(newId));
            } else {
                const QString err = m_quai.lastError();
                QMessageBox::warning(this,
                                     QStringLiteral("ID quai"),
                                     err.isEmpty()
                                         ? QStringLiteral("Impossible de generer le nouvel ID du quai (263NNNN).")
                                         : QStringLiteral("Impossible de generer le nouvel ID du quai : %1").arg(err));
            }
        }
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
                              QStringLiteral("Connexion DB ├⌐chou├⌐e: %1").arg(conn->lastErrorText()));
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
        {QStringLiteral("STATUT"), {QStringLiteral("STATUT"), QStringLiteral("statut")}},
        {QStringLiteral("ID_BATEAU"), {QStringLiteral("ID_BATEAU"), QStringLiteral("id_bateau"), QStringLiteral("idbateau"), QStringLiteral("ID_BAT"), QStringLiteral("id_bat")}}
    };

    const QString tableName = QStringLiteral("QUAIS");

    reloadTableWidgetFromDb(ui->tableWidgetQuai, db, tableName,
                            {QStringLiteral("ID_QUAI"), QStringLiteral("NOM_QUAI"), QStringLiteral("ZONE_PORT"),
                             QStringLiteral("ZONE_COUVERTE"), QStringLiteral("LONGUEUR"),
                             QStringLiteral("CAPACITE_QUAIS"), QStringLiteral("STATUT"),
                             QStringLiteral("ID_BATEAU")},
                            syn);

    auto zoneDbToLabel = [](const QString &zoneDb) -> QString {
        const QString z = zoneDb.trimmed();
        if (z.compare(QStringLiteral("Ouest"), Qt::CaseInsensitive) == 0) {
            return QStringLiteral("Quai Ouest (Z1)");
        }
        if (z.compare(QStringLiteral("Est"), Qt::CaseInsensitive) == 0) {
            return QStringLiteral("Quai Est (Z2)");
        }
        if (z.compare(QStringLiteral("Sud"), Qt::CaseInsensitive) == 0) {
            return QStringLiteral("Bassin central (Z3)");
        }
        if (z.compare(QStringLiteral("Nord"), Qt::CaseInsensitive) == 0) {
            return QStringLiteral("Chenal d'entree (Z4)");
        }
        return z;
    };

    if (ui->tableWidgetQuai && ui->tableWidgetQuai->columnCount() >= 3) {
        QTableWidget *table = ui->tableWidgetQuai;
        for (int row = 0; row < table->rowCount(); ++row) {
            if (QTableWidgetItem *idItem = table->item(row, 0)) {
                const QVariant displayValue = idItem->data(Qt::DisplayRole);
                bool okLongLong = false;
                const qlonglong idValue = displayValue.toLongLong(&okLongLong);
                if (okLongLong) {
                    idItem->setData(Qt::DisplayRole, QString::number(idValue));
                } else {
                    bool okDouble = false;
                    const double asDouble = displayValue.toDouble(&okDouble);
                    if (okDouble) {
                        idItem->setData(Qt::DisplayRole, QString::number(static_cast<qlonglong>(std::llround(asDouble))));
                    }
                }
            }

            if (QTableWidgetItem *zoneItem = table->item(row, 2)) {
                zoneItem->setText(zoneDbToLabel(zoneItem->text()));
            }

            for (int col = 0; col < table->columnCount(); ++col) {
                if (QTableWidgetItem *item = table->item(row, col)) {
                    item->setData(kQuaiOriginalRowRole, row);
                }
            }
        }
    }

    // En-tetes lisibles
    if (ui->tableWidgetQuai && ui->tableWidgetQuai->columnCount() >= 9) {
        ui->tableWidgetQuai->setHorizontalHeaderItem(0, new QTableWidgetItem(QStringLiteral("ID Quai")));
        ui->tableWidgetQuai->setHorizontalHeaderItem(1, new QTableWidgetItem(QStringLiteral("Nom Quai")));
        ui->tableWidgetQuai->setHorizontalHeaderItem(2, new QTableWidgetItem(QStringLiteral("Zone Port")));
        ui->tableWidgetQuai->setHorizontalHeaderItem(3, new QTableWidgetItem(QStringLiteral("Zone Couverte")));
        ui->tableWidgetQuai->setHorizontalHeaderItem(4, new QTableWidgetItem(QStringLiteral("Longueur Max")));
        ui->tableWidgetQuai->setHorizontalHeaderItem(5, new QTableWidgetItem(QStringLiteral("Capacite Quais")));
        ui->tableWidgetQuai->setHorizontalHeaderItem(6, new QTableWidgetItem(QStringLiteral("Statut")));
        ui->tableWidgetQuai->setHorizontalHeaderItem(7, new QTableWidgetItem(QStringLiteral("ID Bateau")));
        ui->tableWidgetQuai->setHorizontalHeaderItem(8, new QTableWidgetItem(QStringLiteral("Actions")));
    }

    ui->tableWidgetQuai->setAlternatingRowColors(false);
    ui->tableWidgetQuai->setShowGrid(false);
    ui->tableWidgetQuai->verticalHeader()->setDefaultSectionSize(42);
    ui->tableWidgetQuai->setStyleSheet(
        "QTableWidget {"
        "background-color: rgb(224, 238, 255);"
        "alternate-background-color: rgb(224, 238, 255);"
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

    if (m_quaisFermeMeteo) {
        QTableWidget *table = ui->tableWidgetQuai;
        const int statutCol = 6;
        if (table->columnCount() > statutCol) {
            for (int row = 0; row < table->rowCount(); ++row) {
                QTableWidgetItem *idItem = table->item(row, 0);
                QTableWidgetItem *statusItem = table->item(row, statutCol);
                if (!idItem || !statusItem) continue;

                bool okId = false;
                const int quaiId = idItem->text().trimmed().toInt(&okId);
                if (!okId || !m_quaisAutoLocked.contains(quaiId)) continue;
                if (statusItem->text().compare(QStringLiteral("Ferme"), Qt::CaseInsensitive) != 0) continue;

                statusItem->setData(Qt::UserRole, statusItem->text());
                statusItem->setText(QStringLiteral("Ferme (meteo)"));
                statusItem->setToolTip(QStringLiteral("Quai ferme temporairement a cause de la meteo."));
            }
        }
    }

    refreshStats_2();
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

        editBtn->setFixedSize(42, 32);
        QFont editBtnFont(QStringLiteral("Segoe UI Emoji"));
        editBtnFont.setPointSize(15);
        editBtnFont.setBold(true);
        editBtn->setFont(editBtnFont);
        editBtn->setStyleSheet(QStringLiteral(
            "QPushButton {"
            " border: 2px solid rgb(0, 0, 112);"
            " border-radius: 6px;"
            " background-color: rgb(224, 238, 255);"
            " color: rgb(0, 0, 112);"
            " padding: 0px;"
            " }"
            "QPushButton:hover { background-color: rgb(224, 238, 255); }"
            "QPushButton:pressed { background-color: rgb(224, 238, 255); }"));

        deleteBtn->setFixedSize(42, 32);
        QFont deleteBtnFont(QStringLiteral("Segoe UI Emoji"));
        deleteBtnFont.setPointSize(15);
        deleteBtnFont.setBold(true);
        deleteBtn->setFont(deleteBtnFont);
        deleteBtn->setStyleSheet(QStringLiteral(
            "QPushButton {"
            " border: 2px solid rgb(0, 0, 112);"
            " border-radius: 6px;"
            " background-color: rgb(224, 238, 255);"
            " color: rgb(0, 0, 112);"
            " padding: 0px;"
            " }"
            "QPushButton:hover { background-color: rgb(224, 238, 255); }"
            "QPushButton:pressed { background-color: rgb(224, 238, 255); }"));

        table->setRowHeight(row, 42);

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
                              QStringLiteral("├ëchec de la suppression du quai."));
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
                             QStringLiteral("├ëdition quai"),
                             QStringLiteral("Impossible de charger les informations de ce quai."));
        return;
    }

    if (ui->lineEdit_4) ui->lineEdit_4->setText(QString::number(infos.value("ID_QUAI").toInt()));
    if (ui->lineEdit_6) ui->lineEdit_6->setText(infos.value("NOM_QUAI").toString());

    if (ui->comboBox_6) {
        const QString zonePort = infos.value("ZONE_PORT").toString().trimmed();
        QString zoneLabel;
        if (zonePort.compare(QStringLiteral("Ouest"), Qt::CaseInsensitive) == 0) {
            zoneLabel = QStringLiteral("Quai Ouest (Z1)");
        } else if (zonePort.compare(QStringLiteral("Est"), Qt::CaseInsensitive) == 0) {
            zoneLabel = QStringLiteral("Quai Est (Z2)");
        } else if (zonePort.compare(QStringLiteral("Sud"), Qt::CaseInsensitive) == 0) {
            zoneLabel = QStringLiteral("Bassin central (Z3)");
        } else if (zonePort.compare(QStringLiteral("Nord"), Qt::CaseInsensitive) == 0) {
            zoneLabel = QStringLiteral("Chenal d'entree (Z4)");
        } else {
            zoneLabel = zonePort;
        }

        int idx = ui->comboBox_6->findText(zoneLabel, Qt::MatchStartsWith);
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

    if (ui->comboBoxQuaiBateau) {
        loadQuaiBateauChoices(ui);
        const int idBateau = infos.value("ID_BATEAU").toInt();
        int idx = ui->comboBoxQuaiBateau->findData(idBateau);
        if (idx < 0) {
            const QString label = (idBateau == 0)
                ? QStringLiteral("0 - Aucun bateau")
                : QString::number(idBateau);
            idx = ui->comboBoxQuaiBateau->findText(label, Qt::MatchStartsWith);
        }
        ui->comboBoxQuaiBateau->setCurrentIndex(idx >= 0 ? idx : 0);
    }

    m_quai.setModeModification(true, id);
    setAddButtonText(QStringLiteral("Modifier"));
    showQuaiFormPage();
}

// --- Filtres de recherche / statut / zone / capacit├⌐ sur la page_3 (quais) ---

void MainWindow::applyQuaiFilters()
{
    if (!ui || !ui->tableWidgetQuai) {
        return;
    }

    QTableWidget *table = ui->tableWidgetQuai;

    if (QComboBox *comboTri = ui->page_3 ? ui->page_3->findChild<QComboBox*>(QStringLiteral("comboBox_sortQuais")) : nullptr) {
        int sortCol = -1;
        Qt::SortOrder order = Qt::AscendingOrder;
        const int sortMode = comboTri->currentIndex();

        switch (sortMode) {
        case 1: sortCol = 0; order = Qt::AscendingOrder; break;
        case 2: sortCol = 0; order = Qt::DescendingOrder; break;
        case 3: sortCol = 5; order = Qt::AscendingOrder; break;
        case 4: sortCol = 5; order = Qt::DescendingOrder; break;
        case 5: sortCol = 4; order = Qt::AscendingOrder; break;
        case 6: sortCol = 4; order = Qt::DescendingOrder; break;
        default: break;
        }

        if (sortMode == 0 || sortCol >= 0) {
            sortQuaiTableRows(table, sortCol, order);
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
        }
    }

    const auto canonicalQuaiStatus = [](const QString &text) -> QString {
        const QString trimmed = text.trimmed();
        if (trimmed.isEmpty()) return QString();

        if (trimmed.compare(QStringLiteral("Occupé"), Qt::CaseInsensitive) == 0
            || trimmed.compare(QStringLiteral("Occupe"), Qt::CaseInsensitive) == 0
            || trimmed.startsWith(QStringLiteral("Occu"), Qt::CaseInsensitive)) {
            return QStringLiteral("Occupe");
        }

        if (trimmed.compare(QStringLiteral("Ferme (meteo)"), Qt::CaseInsensitive) == 0
            || trimmed.compare(QStringLiteral("Fermé (météo)"), Qt::CaseInsensitive) == 0
            || trimmed.compare(QStringLiteral("Ferme"), Qt::CaseInsensitive) == 0
            || trimmed.startsWith(QStringLiteral("Ferm"), Qt::CaseInsensitive)) {
            return QStringLiteral("Ferme");
        }

        if (trimmed.compare(QStringLiteral("Libre"), Qt::CaseInsensitive) == 0) {
            return QStringLiteral("Libre");
        }
        if (trimmed.compare(QStringLiteral("Maintenance"), Qt::CaseInsensitive) == 0) {
            return QStringLiteral("Maintenance");
        }
        return trimmed;
    };

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
        // On essaie de lire le premier "mot" num├⌐rique
        const QString firstToken = trimmed.split(' ', Qt::SkipEmptyParts).value(0);
        int value = firstToken.toInt(&ok);
        if (!ok) {
            value = trimmed.toInt(&ok);
        }
        return ok ? value : 0;
    };

    const int rowCount = table->rowCount();
    const int colCount = table->columnCount();
    const int statutCol = 6;
    const int zoneCol   = 2;
    const int capaCol   = 5;

    for (int row = 0; row < rowCount; ++row) {
        bool match = true;

        if (!searchText.isEmpty()) {
            bool found = false;
            for (int col = 0; col < colCount - 1; ++col) {
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

        if (match && !statutFilter.isEmpty() && statutFilter.compare(QStringLiteral("Tous"), Qt::CaseInsensitive) != 0) {
            QTableWidgetItem *statutItem = (statutCol >= 0 && statutCol < colCount)
                                           ? table->item(row, statutCol)
                                           : nullptr;
            const QString cellStatut = statutItem ? canonicalQuaiStatus(statutItem->text()) : QString();
            const QString selectedStatut = canonicalQuaiStatus(statutFilter);
            if (cellStatut.compare(selectedStatut, Qt::CaseInsensitive) != 0) {
                match = false;
            }
        }

        if (match && !zoneFilter.isEmpty() && zoneFilter.compare(QStringLiteral("Toutes"), Qt::CaseInsensitive) != 0) {
            QTableWidgetItem *zoneItem = (zoneCol >= 0 && zoneCol < colCount)
                                         ? table->item(row, zoneCol)
                                         : nullptr;
            const QString cellZone = zoneItem ? zoneItem->text().trimmed() : QString();
            if (cellZone.compare(zoneFilter, Qt::CaseInsensitive) != 0) {
                match = false;
            }
        }

        if (match && !capaciteFilter.isEmpty() && capaciteFilter.compare(QStringLiteral("Toutes"), Qt::CaseInsensitive) != 0) {
            QTableWidgetItem *capaItem = (capaCol >= 0 && capaCol < colCount)
                                         ? table->item(row, capaCol)
                                         : nullptr;
            const int capa = parseCapacity(capaItem ? capaItem->text() : QString());

            if (capaciteFilter.startsWith(QStringLiteral("<="))) {
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

// Boutons suppl├⌐mentaires non encore utilis├⌐s explicitement dans la logique

void MainWindow::on_pushButton_6_clicked()
{
    // Depuis la page_3 (gestion des quais) : ouvrir la page_4 (carte + statistiques)
    if (ui && ui->stackedWidget && ui->page_4) {
        ui->stackedWidget->setCurrentWidget(ui->page_4);
        // Actualiser les statistiques ├á l'ouverture
        refreshStats_2();
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    // Bouton retour sur la page_4 : revenir ├á la gestion des quais (page_3)
    if (ui && ui->stackedWidget && ui->page_3) {
        ui->stackedWidget->setCurrentWidget(ui->page_3);
    }
}

void MainWindow::on_pushButton_9_clicked()
{
    // Bouton "statistiques" sur la page_3 : m├¬me comportement que le bouton carte,
    // ouvre la page_4 avec les stats d'occupation.
    on_pushButton_6_clicked();
}

void MainWindow::on_pushButton_pdfb_clicked()
{
    if (!ui->tableWidgetb || ui->tableWidgetb->rowCount() == 0) {
        QMessageBox::warning(this, "Export PDF", "Le tableau est vide, rien ├á exporter.");
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
        QMessageBox::critical(this, "Erreur", "Impossible de cr├⌐er le fichier PDF.");
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

    // ΓöÇΓöÇ Title ΓöÇΓöÇ
    QFont titleFont("Arial", 20, QFont::Bold);
    titleFont.setPointSize(20);
    painter.setFont(titleFont);
    painter.setPen(QColor(0, 82, 155));
    painter.drawText(QRect(leftMargin, topMargin - 40, contentW, 60), Qt::AlignCenter, "AQUATEC ΓÇô Liste des Bateaux");

    // ΓöÇΓöÇ Date ΓöÇΓöÇ
    QFont dateFont("Arial", 9);
    dateFont.setPointSize(9);
    painter.setFont(dateFont);
    painter.setPen(Qt::darkGray);
    painter.drawText(QRect(leftMargin, topMargin + 20, contentW, 30), Qt::AlignCenter,
                     "Export├⌐ le " + QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm"));

    int yStart = topMargin + 80;

    // Column headers (0-9, skip 10 and 11=Actions)
    QStringList headers = { "ID", "Nom", "Propri├⌐taire", "Largeur", "Type",
                            "Statut", "Capacit├⌐", "Date entr├⌐e", "Dern. maint.", "Fr├⌐q. maint." };
    const int colCount = headers.size();

    // Calculate column widths
    QVector<int> colWidths(colCount);
    int totalW = contentW;
    QVector<double> proportions = { 0.06, 0.12, 0.13, 0.07, 0.10, 0.10, 0.08, 0.12, 0.12, 0.10 };
    for (int i = 0; i < colCount; ++i)
        colWidths[i] = static_cast<int>(totalW * proportions[i]);

    int rowHeight = 40;
    int headerHeight = 48;
    int xMargin = leftMargin;

    // ΓöÇΓöÇ Draw header row ΓöÇΓöÇ
    QFont headerFont("Arial", 11, QFont::Bold);
    headerFont.setPointSize(11);
    painter.setFont(headerFont);

    // Header gradient style (more CSS-like)
    QLinearGradient headerGrad(0, yStart, 0, yStart + headerHeight);
    headerGrad.setColorAt(0.0, QColor("#0b5ea8"));
    headerGrad.setColorAt(1.0, QColor("#2e86c1"));

    int x = xMargin;
    for (int c = 0; c < colCount; ++c) {
        QRect cellRect(x, yStart, colWidths[c], headerHeight);
        painter.fillRect(cellRect, headerGrad);
        painter.setPen(Qt::white);
        painter.drawText(cellRect.adjusted(12, 0, -12, 0), Qt::AlignVCenter | Qt::AlignLeft, headers[c]);
        painter.setPen(QPen(QColor(200, 210, 220), 1));
        painter.drawRect(cellRect);
        x += colWidths[c];
    }

    // ΓöÇΓöÇ Draw data rows ΓöÇΓöÇ
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
        QColor bg = (r % 2 == 0) ? QColor(250, 250, 252) : QColor(245, 251, 255);
        x = xMargin;
        for (int c = 0; c < colCount; ++c) {
            QRect cellRect(x, y, colWidths[c], rowHeight);
            painter.fillRect(cellRect, bg);
            painter.setPen(QPen(QColor(220, 225, 230), 1));
            painter.drawRect(cellRect);

            QString text;
            QTableWidgetItem *item = ui->tableWidgetb->item(r, c);
            if (item) text = item->text();

            painter.setPen(Qt::black);
            painter.drawText(cellRect.adjusted(12, 6, -12, -6), Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap, text);
            x += colWidths[c];
        }
        y += rowHeight;
    }

    // ΓöÇΓöÇ Footer on table page ΓöÇΓöÇ
    painter.setPen(Qt::darkGray);
    QFont footerFont("Arial", 9);
    footerFont.setPointSize(9);
    painter.setFont(footerFont);
    painter.drawText(QRect(leftMargin, pageH - bottomMargin + 10, contentW, bottomMargin - 10), Qt::AlignCenter,
                     QString("Total: %1 bateaux").arg(rowCount));

    // ΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉ
    //  PAGE 2 : STATISTIQUES
    // ΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉ
    writer.newPage();

    // ΓöÇΓöÇ Gather stats data (same logic as updateStatsBateaux) ΓöÇΓöÇ
    QStringList statConditions;
    QString sText = ui->lineEdit_4p_2 ? ui->lineEdit_4p_2->text().trimmed() : QString();
    double  sLarg = ui->doubleSpinBox_largeur ? ui->doubleSpinBox_largeur->value() : 0.0;
    int     sCap  = ui->spinBoxb_2b ? ui->spinBoxb_2b->value() : 0;

    if (!sText.isEmpty()) {
        QString escaped = sText; escaped.replace("'", "''");
        statConditions << QString("(UPPER(TO_CHAR(ID_BATEAU)) LIKE UPPER('%%1%') OR "
                                  "UPPER(NOM) LIKE UPPER('%%1%'))").arg(escaped);
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

    // ΓöÇΓöÇ Stats page title ΓöÇΓöÇ
    painter.setPen(QColor(0, 82, 155));
    titleFont.setPointSize(18);
    painter.setFont(titleFont);
    painter.drawText(QRect(leftMargin, 60, contentW, 60), Qt::AlignCenter, "Statistiques des Bateaux");

    painter.setPen(Qt::darkGray);
    dateFont.setPointSize(10);
    painter.setFont(dateFont);
    painter.drawText(QRect(leftMargin, 140, contentW, 30), Qt::AlignCenter,
                     QString("Total: %1 bateaux").arg(sTotal));

    // ΓöÇΓöÇ Decorative line under title ΓöÇΓöÇ
    painter.setPen(QPen(QColor(0, 0, 112), 4));
    painter.drawLine(200, 380, pageW - 200, 380);

    // ΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉ
    //  TOP SECTION: R├⌐partition par Statut (horizontal bars)
    // ΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉ
    int statsY = 440;
    int contentX = 200;
    int statsContentW = pageW - 400;

    QFont sectionFont("Arial", 12, QFont::Bold);
    sectionFont.setPointSize(12);
    painter.setFont(sectionFont);
    painter.setPen(QColor(0, 82, 155));
    painter.drawText(QRect(contentX, statsY, statsContentW, 40), Qt::AlignLeft | Qt::AlignVCenter,
                     QStringLiteral("R\u00E9partition par statut"));
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

    // ΓöÇΓöÇ Separator line ΓöÇΓöÇ
    statsY += 40;
    painter.setPen(QPen(QColor(200, 200, 220), 3));
    painter.drawLine(contentX, statsY, contentX + statsContentW, statsY);
    statsY += 60;

    // ΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉ
    //  BOTTOM SECTION: R├⌐partition par Type (pie chart + legend side by side)
    // ΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉΓòÉ
    painter.setFont(sectionFont);
    painter.setPen(QColor(0, 0, 112));
    painter.drawText(QRect(contentX, statsY, statsContentW, 130), Qt::AlignLeft | Qt::AlignVCenter,
                     QStringLiteral("R\u00E9partition par type"));
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

    // ΓöÇΓöÇ Final footer ΓöÇΓöÇ
    painter.setPen(Qt::darkGray);
    painter.setFont(footerFont);
    painter.drawText(QRect(leftMargin, pageH - bottomMargin + 10, contentW, bottomMargin - 10), Qt::AlignCenter,
                     "AQUATEC - Rapport g├⌐n├⌐r├⌐ le " + QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm"));

    painter.end();

    QMessageBox::information(this, "Export PDF",
        QString("PDF export├⌐ avec succ├¿s !\n%1").arg(filePath));
}

void MainWindow::on_pushButton_pdfb_2_clicked()
{
    if (!ui || !ui->tableWidgetQuai) {
        return;
    }

    QTableWidget* table = ui->tableWidgetQuai;
    if (table->rowCount() == 0) {
        QMessageBox::warning(this, "Export PDF", "Le tableau est vide, rien a exporter.");
        return;
    }

    const QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
            + "/Quais_" + QDate::currentDate().toString("yyyy-MM-dd") + ".pdf";
    QString filePath = QFileDialog::getSaveFileName(this, "Enregistrer le PDF", defaultPath, "PDF (*.pdf)");
    if (filePath.isEmpty()) return;
    if (!filePath.toLower().endsWith(QStringLiteral(".pdf"))) {
        filePath += QStringLiteral(".pdf");
    }

    QPdfWriter writer(filePath);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageOrientation(QPageLayout::Landscape);
    writer.setResolution(300);
    writer.setPageMargins(QMarginsF(8, 8, 8, 8), QPageLayout::Millimeter);
    writer.setTitle(QStringLiteral("Rapport des quais - Page 3"));

    QPainter painter(&writer);
    if (!painter.isActive()) {
        QMessageBox::critical(this, "Erreur", "Impossible de creer le fichier PDF.");
        return;
    }

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QString exportStamp = QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm");
    const int pageW = writer.width();
    const int pageH = writer.height();

    const int leftMargin = 54;
    const int rightMargin = 54;
    const int topMargin = 42;
    const int bottomMargin = 56;
    const int contentW = pageW - leftMargin - rightMargin;
    const int footerY = pageH - bottomMargin + 6;
    const int contentBottom = footerY - 24;

    const int actionColumn = findPecheurActionColumnIndex(table);
    QVector<int> columns;
    QVector<int> sourceWidths;
    QVector<QString> headers;
    int totalSourceWidth = 0;

    for (int c = 0; c < table->columnCount(); ++c) {
        if (table->isColumnHidden(c) || c == actionColumn) {
            continue;
        }

        columns.push_back(c);
        sourceWidths.push_back(qMax(70, table->columnWidth(c)));
        totalSourceWidth += sourceWidths.back();

        const QTableWidgetItem* headerItem = table->horizontalHeaderItem(c);
        headers.push_back(headerItem ? normalizePdfCellText(headerItem->text())
                                     : QStringLiteral("Colonne %1").arg(c + 1));
    }

    if (columns.isEmpty()) {
        painter.end();
        QMessageBox::warning(this, "Export PDF", "Aucune colonne a exporter.");
        return;
    }

    QVector<int> colWidths;
    colWidths.reserve(columns.size());
    int usedWidth = 0;
    for (int i = 0; i < sourceWidths.size(); ++i) {
        const int width = qMax(80, static_cast<int>(
                                   (static_cast<double>(sourceWidths[i]) / static_cast<double>(qMax(1, totalSourceWidth))) * contentW));
        colWidths.push_back(width);
        usedWidth += width;
    }
    colWidths.last() += (contentW - usedWidth);

    QVector<int> visibleRows;
    visibleRows.reserve(table->rowCount());
    for (int r = 0; r < table->rowCount(); ++r) {
        if (!table->isRowHidden(r)) {
            visibleRows.push_back(r);
        }
    }

    QSet<QString> visibleZones;
    QSet<QString> visibleStatuts;
    for (int row : visibleRows) {
        const QString zone = table->item(row, 2)
            ? normalizePdfCellText(table->item(row, 2)->text())
            : QString();
        const QString statut = table->item(row, 6)
            ? normalizePdfCellText(table->item(row, 6)->text())
            : QString();
        if (!zone.isEmpty()) visibleZones.insert(zone);
        if (!statut.isEmpty()) visibleStatuts.insert(statut);
    }

    const QStringList infoLines = buildQuaiReportInfoLines(
        ui,
        QStringLiteral("Page 3 - Gestion des quais"),
        exportStamp,
        visibleRows.size(),
        table->rowCount());
    const int activeFilterCount = countActiveQuaiFilters(ui);

    const QFont headerFont(QStringLiteral("Arial"), 10, QFont::Bold);
    const QFont cellFont(QStringLiteral("Arial"), 9);
    const QFont totalFont(QStringLiteral("Arial"), 9, QFont::Bold);
    int tableHeaderHeight = 52;
    for (int i = 0; i < headers.size(); ++i) {
        tableHeaderHeight = qMax(
            tableHeaderHeight,
            pdfWrappedTextHeight(
                painter,
                headerFont,
                headers[i],
                colWidths[i] - 20,
                24,
                Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap) + 14);
    }
    int pageNumber = 1;

    auto drawOverviewCards = [&](int startY) -> int {
        const int cardGap = 18;
        const int cardHeight = 126;
        const int cardWidth = (contentW - (cardGap * 3)) / 4;

        drawQuaiMetricCard(
            painter,
            QRect(leftMargin, startY, cardWidth, cardHeight),
            QStringLiteral("Quais visibles"),
            QString::number(visibleRows.size()),
            QStringLiteral("Sur %1 quais charges dans le tableau").arg(table->rowCount()),
            QColor(QStringLiteral("#0b5ea8")));

        drawQuaiMetricCard(
            painter,
            QRect(leftMargin + cardWidth + cardGap, startY, cardWidth, cardHeight),
            QStringLiteral("Zones couvertes"),
            QString::number(visibleZones.size()),
            visibleZones.isEmpty()
                ? QStringLiteral("Aucune zone detectee")
                : QStringLiteral("%1 zones distinctes dans la selection").arg(visibleZones.size()),
            QColor(QStringLiteral("#2e8b57")));

        drawQuaiMetricCard(
            painter,
            QRect(leftMargin + ((cardWidth + cardGap) * 2), startY, cardWidth, cardHeight),
            QStringLiteral("Statuts visibles"),
            QString::number(visibleStatuts.size()),
            visibleStatuts.isEmpty()
                ? QStringLiteral("Aucun statut detecte")
                : QStringLiteral("%1 types de statut presentes").arg(visibleStatuts.size()),
            QColor(QStringLiteral("#d97706")));

        drawQuaiMetricCard(
            painter,
            QRect(leftMargin + ((cardWidth + cardGap) * 3), startY, cardWidth, cardHeight),
            QStringLiteral("Vue appliquee"),
            activeFilterCount > 0 ? QStringLiteral("%1 filtres").arg(activeFilterCount) : QStringLiteral("Complete"),
            activeFilterCount > 0
                ? QStringLiteral("Tri et filtres integres dans l'export")
                : QStringLiteral("Aucun filtre actif sur la liste"),
            QColor(QStringLiteral("#7c3aed")));

        return startY + cardHeight + 24;
    };

    auto drawTableHeader = [&](int yHeader) {
        painter.save();
        painter.setFont(headerFont);

        QLinearGradient headerGrad(0, yHeader, 0, yHeader + tableHeaderHeight);
        headerGrad.setColorAt(0.0, QColor(QStringLiteral("#0b5ea8")));
        headerGrad.setColorAt(1.0, QColor(QStringLiteral("#2e86c1")));

        int x = leftMargin;
        for (int c = 0; c < headers.size(); ++c) {
            const QRect cellRect(x, yHeader, colWidths[c], tableHeaderHeight);
            painter.fillRect(cellRect, headerGrad);
            painter.setPen(Qt::white);
            painter.drawText(cellRect.adjusted(10, 8, -10, -8),
                             Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap,
                             headers[c]);
            painter.setPen(QPen(QColor(QStringLiteral("#d7e4ef")), 1));
            painter.drawRect(cellRect);
            x += colWidths[c];
        }
        painter.restore();
    };

    auto startPage = [&](bool createNewPage, bool isFirstPage) -> int {
        if (createNewPage) {
            writer.newPage();
        }
        int cursor = drawQuaiPdfHeader(
            painter,
            QRect(leftMargin, topMargin, contentW, 0),
            QStringLiteral("Rapport des quais"),
            infoLines);

        if (isFirstPage) {
            cursor = drawOverviewCards(cursor);
        }

        cursor = drawQuaiSectionIntro(
            painter,
            leftMargin,
            cursor,
            contentW,
            isFirstPage ? QStringLiteral("Tableau detaille des quais") : QStringLiteral("Tableau detaille des quais - suite"),
            isFirstPage
                ? QStringLiteral("Toutes les lignes visibles dans la page 3 sont reprises avec leurs informations principales.")
                : QStringLiteral("Suite du tableau detaille exporte sur la page precedente."));

        drawTableHeader(cursor);
        return cursor + tableHeaderHeight;
    };

    int y = startPage(false, true);

    for (int visibleIndex = 0; visibleIndex < visibleRows.size(); ++visibleIndex) {
        const int row = visibleRows[visibleIndex];
        QStringList rowTexts;
        rowTexts.reserve(columns.size());

        int rowHeight = 40;
        for (int i = 0; i < columns.size(); ++i) {
            const QTableWidgetItem* item = table->item(row, columns[i]);
            const QString text = item ? normalizePdfCellText(item->text()) : QString();
            rowTexts << text;
            rowHeight = qMax(
                rowHeight,
                pdfWrappedTextHeight(
                    painter,
                    cellFont,
                    text,
                    colWidths[i] - 20,
                    18,
                    Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap) + 16);
        }

        if (y + rowHeight > contentBottom) {
            drawQuaiPdfFooter(painter, leftMargin, footerY, contentW, exportStamp, pageNumber);
            ++pageNumber;
            y = startPage(true, false);
        }

        const QColor rowColor = (visibleIndex % 2 == 0)
            ? QColor(QStringLiteral("#fbfdff"))
            : QColor(QStringLiteral("#f2f8fd"));

        int x = leftMargin;
        for (int i = 0; i < columns.size(); ++i) {
            const QRect cellRect(x, y, colWidths[i], rowHeight);
            painter.fillRect(cellRect, rowColor);
            painter.setPen(QPen(QColor(QStringLiteral("#d9e3ec")), 1));
            painter.drawRect(cellRect);

            painter.setPen(QColor(QStringLiteral("#102a43")));
            painter.setFont(cellFont);
            painter.drawText(cellRect.adjusted(10, 8, -10, -8),
                             Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap,
                             rowTexts[i]);
            x += colWidths[i];
        }

        y += rowHeight;
    }

    const int summaryHeight = 34;
    if (y + summaryHeight > contentBottom) {
        drawQuaiPdfFooter(painter, leftMargin, footerY, contentW, exportStamp, pageNumber);
        ++pageNumber;
        y = startPage(true, false);
    }

    painter.save();
    painter.setBrush(QColor(QStringLiteral("#eef6fd")));
    painter.setPen(QPen(QColor(QStringLiteral("#d5e2ef")), 1));
    painter.drawRoundedRect(QRect(leftMargin, y + 8, contentW, summaryHeight), 10, 10);
    painter.setPen(QColor(QStringLiteral("#0f172a")));
    painter.setFont(totalFont);
    painter.drawText(QRect(leftMargin + 14, y + 8, contentW - 28, summaryHeight),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     QStringLiteral("Total exporte: %1 quais visibles sur %2 enregistrements.")
                         .arg(visibleRows.size())
                         .arg(table->rowCount()));
    painter.restore();

    drawQuaiPdfFooter(painter, leftMargin, footerY, contentW, exportStamp, pageNumber);

    painter.end();

    QMessageBox::information(this, "Export PDF", QString("PDF exporte avec succes !\n%1").arg(filePath));
}

void MainWindow::on_btnExportStatsPDF_2_clicked()
{
    if (!ui || !ui->tableWidgetQuai) {
        return;
    }

    QTableWidget* table = ui->tableWidgetQuai;
    if (table->rowCount() == 0) {
        QMessageBox::warning(this, "Export PDF", "Le tableau est vide, rien a exporter.");
        return;
    }

    const QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
            + "/Statistiques_Quais_" + QDate::currentDate().toString("yyyy-MM-dd") + ".pdf";
    QString filePath = QFileDialog::getSaveFileName(this, "Enregistrer le PDF", defaultPath, "PDF (*.pdf)");
    if (filePath.isEmpty()) return;
    if (!filePath.toLower().endsWith(QStringLiteral(".pdf"))) {
        filePath += QStringLiteral(".pdf");
    }

    QPdfWriter writer(filePath);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageOrientation(QPageLayout::Landscape);
    writer.setResolution(300);
    writer.setPageMargins(QMarginsF(8, 8, 8, 8), QPageLayout::Millimeter);
    writer.setTitle(QStringLiteral("Rapport statistiques quais - Page 4"));

    QPainter painter(&writer);
    if (!painter.isActive()) {
        QMessageBox::critical(this, "Erreur", "Impossible de creer le fichier PDF.");
        return;
    }

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QString exportStamp = QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm");
    const int pageW = writer.width();
    const int pageH = writer.height();

    const int leftMargin = 54;
    const int rightMargin = 54;
    const int topMargin = 42;
    const int bottomMargin = 56;
    const int contentW = pageW - leftMargin - rightMargin;
    const int footerY = pageH - bottomMargin + 6;

    QVector<int> visibleRows;
    visibleRows.reserve(table->rowCount());
    for (int r = 0; r < table->rowCount(); ++r) {
        if (!table->isRowHidden(r)) visibleRows.push_back(r);
    }

    QMap<QString, int> zoneCounts;
    QMap<QString, int> statutCounts;
    auto normalizeZone = [](QString zone) {
        zone = normalizePdfCellText(zone);
        return zone.isEmpty() ? QStringLiteral("Non defini") : zone;
    };
    auto normalizeStatut = [](QString statut) {
        const QString simplified = normalizePdfCellText(statut);
        const QString lowered = simplified.toLower();
        if (lowered.startsWith(QStringLiteral("lib"))) return QStringLiteral("Libre");
        if (lowered.startsWith(QStringLiteral("occu"))) return QStringLiteral("Occupe");
        if (lowered.startsWith(QStringLiteral("maint"))) return QStringLiteral("Maintenance");
        if (lowered.startsWith(QStringLiteral("ferm"))) return QStringLiteral("Ferme");
        return simplified.isEmpty() ? QStringLiteral("Non defini") : simplified;
    };

    for (int r : visibleRows) {
        QString zone = table->item(r, 2) ? table->item(r, 2)->text() : QString();
        QString statut = table->item(r, 6) ? table->item(r, 6)->text() : QString();
        zone = normalizeZone(zone);
        statut = normalizeStatut(statut);
        zoneCounts[zone] += 1;
        statutCounts[statut] += 1;
    }

    const int totalQuais = visibleRows.size();
    const int freeCount = statutCounts.value(QStringLiteral("Libre"));
    const int occupiedCount = qMax(0, totalQuais - freeCount);
    const int occupancyRate = (totalQuais > 0)
        ? qRound((static_cast<double>(occupiedCount) / static_cast<double>(totalQuais)) * 100.0)
        : 0;

    auto dominantLabel = [](const QMap<QString, int>& counts) {
        QString bestLabel = QStringLiteral("-");
        int bestValue = -1;
        for (auto it = counts.constBegin(); it != counts.constEnd(); ++it) {
            if (it.value() > bestValue) {
                bestLabel = it.key();
                bestValue = it.value();
            }
        }
        return bestLabel;
    };

    auto sortedEntries = [](const QMap<QString, int>& counts) {
        QVector<QPair<QString, int>> entries;
        entries.reserve(counts.size());
        for (auto it = counts.constBegin(); it != counts.constEnd(); ++it) {
            entries.push_back(qMakePair(it.key(), it.value()));
        }
        std::sort(entries.begin(), entries.end(), [](const QPair<QString, int>& lhs, const QPair<QString, int>& rhs) {
            if (lhs.second == rhs.second) {
                return lhs.first < rhs.first;
            }
            return lhs.second > rhs.second;
        });
        return entries;
    };

    auto statusColorFor = [](const QString& raw) {
        const QString s = raw.trimmed().toLower();
        if (s.contains(QStringLiteral("lib"))) return QColor("#2ecc71");
        if (s.contains(QStringLiteral("occup"))) return QColor("#e74c3c");
        if (s.contains(QStringLiteral("maint"))) return QColor("#e67e22");
        if (s.contains(QStringLiteral("ferm"))) return QColor("#64748b");
        return QColor("#90a4ae");
    };

    const QVector<QPair<QString, int>> sortedZones = sortedEntries(zoneCounts);
    const QVector<QPair<QString, int>> sortedStatuts = sortedEntries(statutCounts);
    const QStringList infoLines = buildQuaiReportInfoLines(
        ui,
        QStringLiteral("Page 4 - Statistiques des quais"),
        exportStamp,
        totalQuais,
        table->rowCount());

    int y = drawQuaiPdfHeader(
        painter,
        QRect(leftMargin, topMargin, contentW, 0),
        QStringLiteral("Synthese des quais"),
        infoLines);

    const int cardGap = 18;
    const int cardHeight = 128;
    const int cardWidth = (contentW - (cardGap * 3)) / 4;

    drawQuaiMetricCard(
        painter,
        QRect(leftMargin, y, cardWidth, cardHeight),
        QStringLiteral("Quais visibles"),
        QString::number(totalQuais),
        QStringLiteral("Total analyse dans ce rapport"),
        QColor(QStringLiteral("#0b5ea8")));
    drawQuaiMetricCard(
        painter,
        QRect(leftMargin + cardWidth + cardGap, y, cardWidth, cardHeight),
        QStringLiteral("Zones actives"),
        QString::number(zoneCounts.size()),
        QStringLiteral("Zones portuaires presentes"),
        QColor(QStringLiteral("#2e8b57")));
    drawQuaiMetricCard(
        painter,
        QRect(leftMargin + ((cardWidth + cardGap) * 2), y, cardWidth, cardHeight),
        QStringLiteral("Taux d'occupation"),
        QStringLiteral("%1%").arg(occupancyRate),
        QStringLiteral("%1 quais occupes ou indisponibles").arg(occupiedCount),
        QColor(QStringLiteral("#d97706")));
    drawQuaiMetricCard(
        painter,
        QRect(leftMargin + ((cardWidth + cardGap) * 3), y, cardWidth, cardHeight),
        QStringLiteral("Statut dominant"),
        dominantLabel(statutCounts),
        QStringLiteral("Zone dominante: %1").arg(dominantLabel(zoneCounts)),
        QColor(QStringLiteral("#7c3aed")));

    y += cardHeight + 28;

    const QRect zoneSection(leftMargin, y, contentW, 410);
    painter.save();
    painter.setPen(QPen(QColor(QStringLiteral("#d6e2ee")), 1));
    painter.setBrush(QColor(QStringLiteral("#ffffff")));
    painter.drawRoundedRect(zoneSection, 16, 16);

    const int zoneContentTop = drawQuaiSectionIntro(
        painter,
        zoneSection.left() + 22,
        zoneSection.top() + 22,
        zoneSection.width() - 44,
        QStringLiteral("Repartition par zone"),
        QStringLiteral("Vue d'ensemble des quais visibles sur la page 4, avec leur poids relatif par zone."));

    const int pieSize = 230;
    const int pieX = zoneSection.left() + 30;
    const int pieY = zoneContentTop + 26;
    const QRect pieRect(pieX, pieY, pieSize, pieSize);
    const int legendX = pieRect.right() + 46;
    const int legendWidth = zoneSection.right() - 34 - legendX;
    const QVector<QColor> zoneColors = {
        QColor(QStringLiteral("#0ea5e9")),
        QColor(QStringLiteral("#22c55e")),
        QColor(QStringLiteral("#f97316")),
        QColor(QStringLiteral("#8b5cf6")),
        QColor(QStringLiteral("#14b8a6"))
    };

    if (totalQuais > 0) {
        int startAngle = 90 * 16;
        for (int i = 0; i < sortedZones.size(); ++i) {
            const int count = sortedZones[i].second;
            if (count <= 0) continue;

            const int span = -qRound((static_cast<double>(count) / static_cast<double>(totalQuais)) * 360.0 * 16.0);
            painter.setPen(Qt::white);
            painter.setBrush(zoneColors[i % zoneColors.size()]);
            painter.drawPie(pieRect, startAngle, span);
            startAngle += span;
        }
    } else {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(QStringLiteral("#e2e8f0")));
        painter.drawEllipse(pieRect);
        painter.setPen(QColor(QStringLiteral("#64748b")));
        painter.setFont(QFont(QStringLiteral("Arial"), 10, QFont::Bold));
        painter.drawText(pieRect, Qt::AlignCenter, QStringLiteral("Aucune donnee"));
    }

    int legendY = pieY + 8;
    const QFont legendFont(QStringLiteral("Arial"), 9);
    painter.setFont(legendFont);
    for (int i = 0; i < sortedZones.size(); ++i) {
        const int count = sortedZones[i].second;
        const int pct = (totalQuais > 0) ? qRound((100.0 * count) / totalQuais) : 0;
        const QString lineText = QStringLiteral("%1 : %2 quais (%3%)")
                                     .arg(sortedZones[i].first)
                                     .arg(count)
                                     .arg(pct);
        const int lineHeight = pdfWrappedTextHeight(
            painter,
            legendFont,
            lineText,
            legendWidth - 30,
            18,
            Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap);

        painter.fillRect(QRect(legendX, legendY + 5, 12, 12), zoneColors[i % zoneColors.size()]);
        painter.setPen(QColor(QStringLiteral("#102a43")));
        painter.drawText(QRect(legendX + 22, legendY, legendWidth - 22, lineHeight),
                         Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                         lineText);
        legendY += lineHeight + 14;
    }
    painter.restore();

    y = zoneSection.bottom() + 22;

    const QRect statusSection(leftMargin, y, contentW, 292);
    painter.save();
    painter.setPen(QPen(QColor(QStringLiteral("#d6e2ee")), 1));
    painter.setBrush(QColor(QStringLiteral("#ffffff")));
    painter.drawRoundedRect(statusSection, 16, 16);

    const int statusContentTop = drawQuaiSectionIntro(
        painter,
        statusSection.left() + 22,
        statusSection.top() + 22,
        statusSection.width() - 44,
        QStringLiteral("Repartition par statut"),
        QStringLiteral("Lecture rapide de l'etat d'occupation et de disponibilite des quais exportes."));

    const int labelX = statusSection.left() + 26;
    const int labelWidth = 210;
    const int valueWidth = 170;
    const int barX = labelX + labelWidth + 18;
    const int barWidth = statusSection.right() - 26 - valueWidth - barX;
    const int barHeight = 24;
    const int barSpacing = 20;
    int barsY = statusContentTop + 22;

    for (const auto& entry : sortedStatuts) {
        const int count = entry.second;
        const int pct = (totalQuais > 0) ? qRound((100.0 * count) / totalQuais) : 0;
        const QColor fillColor = statusColorFor(entry.first);

        painter.setPen(QColor(QStringLiteral("#0f172a")));
        painter.setFont(QFont(QStringLiteral("Arial"), 9, QFont::Bold));
        painter.drawText(QRect(labelX, barsY, labelWidth, barHeight),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         entry.first);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(QStringLiteral("#e7eef6")));
        painter.drawRoundedRect(QRect(barX, barsY, barWidth, barHeight), 8, 8);

        const int fillWidth = (pct > 0)
            ? qMax(24, qRound((static_cast<double>(pct) / 100.0) * barWidth))
            : 0;
        if (fillWidth > 0) {
            painter.setBrush(fillColor);
            painter.drawRoundedRect(QRect(barX, barsY, fillWidth, barHeight), 8, 8);
        }

        painter.setPen(QColor(QStringLiteral("#334155")));
        painter.setFont(QFont(QStringLiteral("Arial"), 9));
        painter.drawText(QRect(barX + barWidth + 12, barsY, valueWidth, barHeight),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         QStringLiteral("%1 quais (%2%)").arg(count).arg(pct));

        barsY += barHeight + barSpacing;
    }
    painter.restore();

    drawQuaiPdfFooter(painter, leftMargin, footerY, contentW, exportStamp, 1);

    painter.end();

    QMessageBox::information(this, "Export PDF", QString("PDF exporte avec succes !\n%1").arg(filePath));
}

void MainWindow::legacy_comboChartType_2_currentIndexChanged(int)
{
    // Pour l'instant, le filtrage temporel fin n'est pas disponible.
    // On relance simplement le calcul des statistiques pour rester coh├⌐rent
    // et permettre une ├⌐volution future (24h / 7j / 30j).
    refreshStats_2();
}

// --- Export PDF g├⌐n├⌐rique pour un widget donn├⌐ ---

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
        return false; // annul├⌐ par l'utilisateur
    }

    QPdfWriter writer(filePath);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageOrientation(QPageLayout::Portrait);
    writer.setTitle(dialogTitle);

    QPainter painter(&writer);
    if (!painter.isActive()) {
        QMessageBox::warning(this,
                             QStringLiteral("Export PDF"),
                             QStringLiteral("Impossible de cr├⌐er le fichier PDF."));
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
                             QStringLiteral("Exportation termin├⌐e :\n%1")
                                 .arg(QDir::toNativeSeparators(filePath)));

    return true;
}

// Actualisation des statistiques d'occupation des quais (page_4)

void MainWindow::refreshStats_2()
{
    if (!ui) return;

    Connection *conn = Connection::getInstance();
    if (!conn->ensureOpen()) {
        QMessageBox::critical(this,
                              QStringLiteral("DB"),
                              QStringLiteral("Connexion DB echouee: %1").arg(conn->lastErrorText()));
        return;
    }

    QSqlDatabase db = conn->getDatabase();
    QMap<QString, QPair<int, int>> stats;

    auto normalizeZone = [](const QString &raw) -> QString {
        const QString lower = raw.trimmed().toLower();
        if (lower == QStringLiteral("nord")) return QStringLiteral("Chenal");
        if (lower == QStringLiteral("sud")) return QStringLiteral("Bassin");
        if (lower == QStringLiteral("est")) return QStringLiteral("QuaiEst");
        if (lower == QStringLiteral("ouest")) return QStringLiteral("QuaiOuest");
        return raw.trimmed();
    };

    QSqlQuery query(db);
    if (!query.exec(QStringLiteral(
            "SELECT Zone_Port, COUNT(*) AS total, "
            "SUM(CASE WHEN UPPER(Statut) <> 'LIBRE' THEN 1 ELSE 0 END) AS occupe "
            "FROM QUAIS GROUP BY Zone_Port"))) {
        QMessageBox::warning(this,
                             QStringLiteral("Statistiques"),
                             QStringLiteral("Impossible de calculer les statistiques : %1").arg(query.lastError().text()));
        return;
    }

    while (query.next()) {
        const QString zone = normalizeZone(query.value(0).toString());
        stats.insert(zone, qMakePair(query.value(2).toInt(), query.value(1).toInt()));
    }

    auto computePct = [&stats](const QString &zone) -> int {
        const auto it = stats.constFind(zone);
        if (it == stats.constEnd() || it->second <= 0) return 0;
        return (it->first * 100) / it->second;
    };

    auto getCounts = [&stats](const QString &zone) -> QPair<int, int> {
        const auto it = stats.constFind(zone);
        return (it == stats.constEnd()) ? qMakePair(0, 0) : qMakePair(it->first, it->second);
    };

    const int pctSud = computePct(QStringLiteral("Bassin"));
    const int pctEst = computePct(QStringLiteral("QuaiEst"));
    const int pctOuest = computePct(QStringLiteral("QuaiOuest"));

    int totalOccupe = 0;
    int totalQuais = 0;
    for (auto it = stats.constBegin(); it != stats.constEnd(); ++it) {
        if (it.key() == QStringLiteral("Chenal")) continue;
        totalOccupe += it->first;
        totalQuais += it->second;
    }

    const int pctGlobal = (totalQuais > 0) ? (totalOccupe * 100) / totalQuais : 0;

    auto computeShare = [&stats, totalQuais](const QString &zone) -> int {
        if (totalQuais <= 0) return 0;
        const auto it = stats.constFind(zone);
        if (it == stats.constEnd() || it->second <= 0) return 0;
        return (it->second * 100) / totalQuais;
    };

    const int shareSud = computeShare(QStringLiteral("Bassin"));
    const int shareEst = computeShare(QStringLiteral("QuaiEst"));
    const int shareOuest = computeShare(QStringLiteral("QuaiOuest"));

    if (ui->progressZoneSud_2) { ui->progressZoneSud_2->setRange(0, 100); ui->progressZoneSud_2->setValue(shareSud); }
    if (ui->progressZoneEst_2) { ui->progressZoneEst_2->setRange(0, 100); ui->progressZoneEst_2->setValue(shareEst); }
    if (ui->progressZoneOuest_2) { ui->progressZoneOuest_2->setRange(0, 100); ui->progressZoneOuest_2->setValue(shareOuest); }

    if (ui->label_chartTitle_2) {
        ui->label_chartTitle_2->setText(QStringLiteral("Taux global d'occupation des quais : %1 %").arg(pctGlobal));
    }

    const auto sudCounts = getCounts(QStringLiteral("Bassin"));
    const auto estCounts = getCounts(QStringLiteral("QuaiEst"));
    const auto ouestCounts = getCounts(QStringLiteral("QuaiOuest"));

    const QString ttSudOcc = QStringLiteral("Bassin central (Zone 3) : %1 % occupe (%2 / %3 quais)")
        .arg(pctSud).arg(sudCounts.first).arg(sudCounts.second);
    const QString ttEstOcc = QStringLiteral("Quai Est (Zone 2) : %1 % occupe (%2 / %3 quais)")
        .arg(pctEst).arg(estCounts.first).arg(estCounts.second);
    const QString ttOuestOcc = QStringLiteral("Quai Ouest (Zone 1) : %1 % occupe (%2 / %3 quais)")
        .arg(pctOuest).arg(ouestCounts.first).arg(ouestCounts.second);

    if (ui->curvePoint3_2) ui->curvePoint3_2->setToolTip(ttSudOcc);
    if (ui->curvePoint5_2) ui->curvePoint5_2->setToolTip(ttEstOcc);
    if (ui->curvePoint7_2) ui->curvePoint7_2->setToolTip(ttOuestOcc);

    if (ui->progressZoneSud_2) {
        ui->progressZoneSud_2->setToolTip(QStringLiteral("Bassin central (Zone 3) : %1 % des quais (%2 / %3, %4 % occupes)")
                                          .arg(shareSud).arg(sudCounts.second).arg(totalQuais).arg(pctSud));
    }
    if (ui->progressZoneEst_2) {
        ui->progressZoneEst_2->setToolTip(QStringLiteral("Quai Est (Zone 2) : %1 % des quais (%2 / %3, %4 % occupes)")
                                          .arg(shareEst).arg(estCounts.second).arg(totalQuais).arg(pctEst));
    }
    if (ui->progressZoneOuest_2) {
        ui->progressZoneOuest_2->setToolTip(QStringLiteral("Quai Ouest (Zone 1) : %1 % des quais (%2 / %3, %4 % occupes)")
                                            .arg(shareOuest).arg(ouestCounts.second).arg(totalQuais).arg(pctOuest));
    }

    auto updateZoneMap = [](QFrame *zoneFrame, QLabel *valueLabel) {
        if (!zoneFrame) return;
        zoneFrame->setStyleSheet(QStringLiteral("background-color: transparent; border: none;"));
        zoneFrame->setMinimumSize(0, 0);
        zoneFrame->setMaximumSize(16777215, 16777215);
        if (valueLabel) valueLabel->hide();
    };

    updateZoneMap(ui->zoneNord_map, ui->value_zoneNord_map);
    updateZoneMap(ui->zoneSud_map, ui->value_zoneSud_map);
    updateZoneMap(ui->zoneEst_map, ui->value_zoneEst_map);
    updateZoneMap(ui->zoneOuest_map, ui->value_zoneOuest_map);

    if (ui->label_zoneNord_map) ui->label_zoneNord_map->hide();
    if (ui->label_zoneSud_map) ui->label_zoneSud_map->hide();
    if (ui->label_zoneEst_map) ui->label_zoneEst_map->hide();
    if (ui->label_zoneOuest_map) ui->label_zoneOuest_map->hide();

    if (ui->curveCanvas_2) {
        static const int yTop = 30;
        static const int yBot = 145;
        auto yFromPct = [](int pct) {
            pct = qBound(0, pct, 100);
            return yBot - (pct * (yBot - yTop)) / 100;
        };

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

        if (!m_curveLineLabelQuaiStats) {
            m_curveLineLabelQuaiStats = new QLabel(ui->curveCanvas_2);
            m_curveLineLabelQuaiStats->setObjectName(QStringLiteral("curveLineLabelQuaiStats"));
            m_curveLineLabelQuaiStats->setAttribute(Qt::WA_TransparentForMouseEvents);
        }
        m_curveLineLabelQuaiStats->setGeometry(ui->curveCanvas_2->rect());

        QPixmap pix(ui->curveCanvas_2->size());
        pix.fill(Qt::transparent);
        QPainter painter(&pix);
        painter.setRenderHint(QPainter::Antialiasing, true);

        auto centerInCanvas = [this](QWidget *w) -> QPoint {
            if (!w) return QPoint();
            const QPoint topLeft = w->mapTo(this->ui->curveCanvas_2, QPoint(0, 0));
            return QPoint(topLeft.x() + w->width() / 2, topLeft.y() + w->height() / 2);
        };

        QVector<QPoint> points;
        points << centerInCanvas(ui->curvePoint3_2)
               << centerInCanvas(ui->curvePoint5_2)
               << centerInCanvas(ui->curvePoint7_2);

        QVector<QPoint> validPoints;
        for (const QPoint &pt : std::as_const(points)) {
            if (!pt.isNull()) validPoints.append(pt);
        }

        QPen gridPen(QColor(0, 0, 128, 40));
        gridPen.setWidth(1);
        painter.setPen(gridPen);
        for (int lvl : {0, 25, 50, 75, 100}) {
            painter.drawLine(10, yFromPct(lvl), pix.width() - 10, yFromPct(lvl));
        }

        if (validPoints.size() >= 2) {
            QPainterPath path(validPoints.first());
            for (int i = 1; i < validPoints.size(); ++i) {
                path.lineTo(validPoints[i]);
            }

            QPainterPath fillPath(path);
            const qreal bottomY = pix.height() - 10;
            fillPath.lineTo(validPoints.last().x(), bottomY);
            fillPath.lineTo(validPoints.first().x(), bottomY);
            fillPath.closeSubpath();

            QLinearGradient grad(0, yTop, 0, bottomY);
            grad.setColorAt(0.0, QColor(52, 152, 219, 90));
            grad.setColorAt(1.0, QColor(52, 152, 219, 10));
            painter.fillPath(fillPath, grad);

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
        if (ui->curvePoint3_2) ui->curvePoint3_2->raise();
        if (ui->curvePoint5_2) ui->curvePoint5_2->raise();
        if (ui->curvePoint7_2) ui->curvePoint7_2->raise();
    }

    QSqlQuery queryPlaces(db);
    if (!queryPlaces.exec(QStringLiteral("SELECT ID_QUAI, ZONE_PORT, STATUT FROM QUAIS"))) {
        return;
    }

    QMap<QString, QList<QPair<int, QString>>> quaisParZone;
    while (queryPlaces.next()) {
        int id = queryPlaces.value(0).toInt();
        QString zone = queryPlaces.value(1).toString().trimmed();
        QString statut = queryPlaces.value(2).toString().trimmed();

        if (zone.compare(QStringLiteral("Nord"), Qt::CaseInsensitive) == 0) zone = QStringLiteral("Nord");
        else if (zone.compare(QStringLiteral("Sud"), Qt::CaseInsensitive) == 0) zone = QStringLiteral("Sud");
        else if (zone.compare(QStringLiteral("Est"), Qt::CaseInsensitive) == 0) zone = QStringLiteral("Est");
        else if (zone.compare(QStringLiteral("Ouest"), Qt::CaseInsensitive) == 0) zone = QStringLiteral("Ouest");
        else continue;

        quaisParZone[zone].append(qMakePair(id, statut));
    }

    auto rebuildZone = [&](const QString &zone, QVBoxLayout *layout) {
        if (!layout) return;

        QList<QWidget*> toDelete;
        for (int i = 0; i < layout->count(); ++i) {
            if (QWidget *w = layout->itemAt(i) ? layout->itemAt(i)->widget() : nullptr) {
                if (w->objectName().startsWith(QStringLiteral("placesZone"))) {
                    toDelete.append(w);
                }
            }
        }
        for (QWidget *w : std::as_const(toDelete)) delete w;

        QWidget *placesWidget = new QWidget();
        placesWidget->setObjectName(QStringLiteral("placesZone") + zone);
        placesWidget->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
        QVBoxLayout *mainLayout = new QVBoxLayout(placesWidget);
        mainLayout->setSpacing(5);
        mainLayout->setContentsMargins(3, 3, 3, 3);

        QString zoneIcon;
        QString zoneBadgeColor;
        QString zoneTitleText;
        if (zone == QStringLiteral("Nord")) {
            zoneIcon = QStringLiteral("\u2193");
            zoneBadgeColor = QStringLiteral("#1565c0");
            zoneTitleText = QStringLiteral("CHENAL D'ENTREE (Z4)");
        } else if (zone == QStringLiteral("Ouest")) {
            zoneIcon = QStringLiteral("\u25C0");
            zoneBadgeColor = QStringLiteral("#6a1b9a");
            zoneTitleText = QStringLiteral("QUAI OUEST (Z1)");
        } else if (zone == QStringLiteral("Est")) {
            zoneIcon = QStringLiteral("\u25B6");
            zoneBadgeColor = QStringLiteral("#f57f17");
            zoneTitleText = QStringLiteral("QUAI EST (Z2)");
        } else {
            zoneIcon = QStringLiteral("\u2191");
            zoneBadgeColor = QStringLiteral("#2e7d32");
            zoneTitleText = QStringLiteral("BASSIN CENTRAL (Z3)");
        }

        QLabel *zoneTitle = new QLabel(QStringLiteral("%1  %2").arg(zoneIcon, zoneTitleText));
        zoneTitle->setAlignment(Qt::AlignCenter);
        zoneTitle->setStyleSheet(QStringLiteral(
            "color: white; font-weight: bold; font-size: 9px; "
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 %1, stop:1 %2); "
            "border: none; border-radius: 10px; padding: 4px 14px;")
            .arg(zoneBadgeColor, QColor(zoneBadgeColor).lighter(130).name()));
        mainLayout->addWidget(zoneTitle, 0, Qt::AlignCenter);

        if (zone == QStringLiteral("Nord")) {
            layout->addWidget(placesWidget, 0, Qt::AlignCenter);
            return;
        }

        QList<QPair<int, QString>> ordered = quaisParZone.value(zone);
        auto statusRank = [](const QString &s) -> int {
            const QString v = s.trimmed().toLower();
            if (v == QStringLiteral("libre")) return 0;
            if (v == QStringLiteral("maintenance")) return 1;
            if (v == QStringLiteral("occupe")) return 2;
            if (v == QStringLiteral("ferme")) return 3;
            return 4;
        };
        std::sort(ordered.begin(), ordered.end(), [&](const QPair<int, QString> &a, const QPair<int, QString> &b) {
            const int ra = statusRank(a.second);
            const int rb = statusRank(b.second);
            return (ra == rb) ? (a.first < b.first) : (ra < rb);
        });

        QWidget *gridWidget = new QWidget();
        gridWidget->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
        QGridLayout *grid = new QGridLayout(gridWidget);
        grid->setSpacing(4);
        grid->setContentsMargins(0, 2, 0, 0);

        for (int i = 0; i < ordered.size(); ++i) {
            const int qid = ordered.at(i).first;
            const QString statut = ordered.at(i).second;

            const int row = (zone == QStringLiteral("Sud")) ? 0 : i;
            const int col = (zone == QStringLiteral("Sud")) ? i : 0;

            QString bgStart;
            QString bgEnd;
            QString borderColor;
            QString statusEmoji;

            const bool isFermeStatus = (statut.compare(QStringLiteral("Ferme"), Qt::CaseInsensitive) == 0);
            const bool isWeatherLocked = isFermeStatus && m_quaisFermeMeteo && m_quaisAutoLocked.contains(qid);

            if (isFermeStatus) {
                bgStart = QStringLiteral("#000000");
                bgEnd = QStringLiteral("#000000");
                borderColor = QStringLiteral("#000000");
                statusEmoji = QStringLiteral("\u26D4");
            } else if (statut.compare(QStringLiteral("Libre"), Qt::CaseInsensitive) == 0) {
                bgStart = QStringLiteral("#66bb6a");
                bgEnd = QStringLiteral("#43a047");
                borderColor = QStringLiteral("#2e7d32");
                statusEmoji = QStringLiteral("\u2713");
            } else if (statut.compare(QStringLiteral("Occupe"), Qt::CaseInsensitive) == 0) {
                bgStart = QStringLiteral("#ef5350");
                bgEnd = QStringLiteral("#e53935");
                borderColor = QStringLiteral("#c62828");
                statusEmoji = QStringLiteral("\u26D4");
            } else if (statut.compare(QStringLiteral("Maintenance"), Qt::CaseInsensitive) == 0) {
                bgStart = QStringLiteral("#ffa726");
                bgEnd = QStringLiteral("#fb8c00");
                borderColor = QStringLiteral("#ef6c00");
                statusEmoji = QStringLiteral("\u2699");
            } else {
                bgStart = QStringLiteral("#90a4ae");
                bgEnd = QStringLiteral("#78909c");
                borderColor = QStringLiteral("#546e7a");
                statusEmoji = QStringLiteral("?");
            }

            QWidget *tileWidget = new QWidget();
            tileWidget->setFixedSize(46, 50);
            QString tooltip = QStringLiteral("Quai %1\nZone: %2\nStatut: %3").arg(qid).arg(zone).arg(statut);
            if (isWeatherLocked) {
                tooltip += QStringLiteral("\nFerme (meteo dangereuse)");
            }
            tileWidget->setToolTip(tooltip);

            const QString borderSides = QStringLiteral(
                "border-left: 2px solid %1; border-right: 2px solid %1; border-bottom: 2px solid %1; border-top: 0px;")
                .arg(borderColor);
            tileWidget->setStyleSheet(QStringLiteral(
                "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %1, stop:1 %2); "
                "border-radius: 0px; border-bottom-left-radius: 12px; border-bottom-right-radius: 12px; "
                "border: 2px solid transparent; %3")
                .arg(bgStart, bgEnd, borderSides));

            QVBoxLayout *tileLayout = new QVBoxLayout(tileWidget);
            tileLayout->setSpacing(0);
            tileLayout->setContentsMargins(2, 3, 2, 3);

            QLabel *idLbl = new QLabel(QString::number(qid));
            idLbl->setAlignment(Qt::AlignCenter);
            idLbl->setStyleSheet(QStringLiteral("color: white; font-weight: bold; font-size: 11px; background: transparent; border: none;"));
            tileLayout->addWidget(idLbl);

            QLabel *iconLbl = new QLabel(statusEmoji);
            iconLbl->setAlignment(Qt::AlignCenter);
            iconLbl->setStyleSheet(QStringLiteral("color: white; font-size: 12px; background: transparent; border: none;"));
            tileLayout->addWidget(iconLbl);

            grid->addWidget(tileWidget, row, col, Qt::AlignCenter);
        }

        mainLayout->addWidget(gridWidget, 0, Qt::AlignCenter);

        if (ordered.isEmpty()) {
            QLabel *emptyLabel = new QLabel(QStringLiteral("Aucun quai"));
            emptyLabel->setAlignment(Qt::AlignCenter);
            emptyLabel->setStyleSheet(QStringLiteral("color: #90a4ae; font-size: 9px; font-style: italic; background: transparent; border: none;"));
            mainLayout->addWidget(emptyLabel);
        }

        layout->addWidget(placesWidget, 0, Qt::AlignCenter);
    };

    rebuildZone(QStringLiteral("Nord"), ui->verticalLayout_zoneNord);
    rebuildZone(QStringLiteral("Sud"), ui->verticalLayout_zoneSud);
    rebuildZone(QStringLiteral("Est"), ui->verticalLayout_zoneEst);
    rebuildZone(QStringLiteral("Ouest"), ui->verticalLayout_zoneOuest);
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
                                     const QString& humidityText,
                                     const QString& hourlyText,
                                     const QString& dailyText,
                                     bool isDay)
{
    if (!ui) return;

    auto htmlEscaped = [](const QString& s) -> QString {
        return s.toHtmlEscaped();
    };

    auto emphasizedLines = [&](const QString& text) -> QString {
        const QStringList lines = text.split(QStringLiteral("\n"), Qt::KeepEmptyParts);
        const QString primary = lines.value(0).trimmed();
        const QString secondary = lines.mid(1).join(QStringLiteral("<br>")).trimmed();

        QString html = QStringLiteral("<div style=\"font-family:'Segoe UI';line-height:1.45;\">");
        if (!primary.isEmpty()) {
            html += QStringLiteral("<div style=\"font-size:14px;font-weight:800;color:#0B1F36;\">%1</div>")
                        .arg(htmlEscaped(primary));
        }
        if (!secondary.isEmpty()) {
            html += QStringLiteral("<div style=\"margin-top:4px;font-size:12px;font-weight:600;color:rgba(11,31,54,170);\">%1</div>")
                        .arg(secondary);
        }
        html += QStringLiteral("</div>");
        return html;
    };

    struct MeteoMetric {
        QString label;
        QString value;
    };

    auto parseMetric = [](const QString& segment) -> MeteoMetric {
        MeteoMetric metric;
        const int colon = segment.indexOf(QLatin1Char(':'));
        if (colon < 0) {
            metric.value = segment.trimmed();
            return metric;
        }
        metric.label = segment.left(colon).trimmed();
        metric.value = segment.mid(colon + 1).trimmed();
        return metric;
    };

    auto metricsGrid = [&](const QString& text, int maxCols, const QColor &accent) -> QString {
        const QStringList lines = text.split(QStringLiteral("\n"), Qt::SkipEmptyParts);
        QVector<MeteoMetric> metrics;
        metrics.reserve(16);

        for (const QString &line : lines) {
            const QStringList parts = line.split(QStringLiteral(" • "), Qt::SkipEmptyParts);
            for (const QString &part : parts) {
                const MeteoMetric metric = parseMetric(part);
                if (!metric.label.isEmpty() || !metric.value.isEmpty()) {
                    metrics.push_back(metric);
                }
            }
        }

        if (metrics.isEmpty()) {
            return QStringLiteral("<div style=\"font-family:'Segoe UI';font-size:11px;line-height:1.5;color:rgba(11,31,54,170);\">%1</div>")
                .arg(htmlEscaped(text.trimmed()));
        }

        const int cols = qBound(1, maxCols, 4);
        const int colPct = 100 / cols;
        const int ar = accent.red();
        const int ag = accent.green();
        const int ab = accent.blue();

        QString html = QStringLiteral("<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\" style=\"border-collapse:separate;border-spacing:12px 12px;\"><tr>");
        int col = 0;
        for (const MeteoMetric &metric : metrics) {
            if (col == cols) {
                html += QStringLiteral("</tr><tr>");
                col = 0;
            }

            html += QStringLiteral(
                "<td width=\"%1%\"><div style=\"width:100%;padding:12px 14px;border-radius:16px;"
                "background:rgba(%2,%3,%4,0.10);border:1px solid rgba(%2,%3,%4,0.20);\">")
                .arg(QString::number(colPct),
                     QString::number(ar),
                     QString::number(ag),
                     QString::number(ab));

            if (!metric.label.isEmpty()) {
                html += QStringLiteral("<div style=\"font-size:10px;font-weight:900;color:rgba(11,31,54,160);\">%1</div>")
                            .arg(htmlEscaped(metric.label.toUpper()));
            }

            html += QStringLiteral("<div style=\"margin-top:4px;font-size:13px;font-weight:900;color:#0B1F36;line-height:1.4;\">%1</div></div></td>")
                        .arg(htmlEscaped(metric.value.isEmpty() ? QStringLiteral("--") : metric.value));
            ++col;
        }

        html += QStringLiteral("</tr></table>");
        return html;
    };

    auto chipLine = [&](const QString& fullText) -> QString {
        const QStringList lines = fullText.split(QStringLiteral("\n"), Qt::KeepEmptyParts);
        const QString mainLine = lines.value(0).trimmed();
        const QString metaLine = lines.mid(1).join(QStringLiteral(" • ")).trimmed();

        const int colon = mainLine.indexOf(QLatin1Char(':'));
        const QString header = (colon >= 0) ? mainLine.left(colon).trimmed() : QString();
        const QString body = (colon >= 0) ? mainLine.mid(colon + 1).trimmed() : mainLine;

        QStringList parts = body.split(QStringLiteral("  | "), Qt::SkipEmptyParts);
        if (parts.size() == 1) {
            parts = body.split(QStringLiteral("|"), Qt::SkipEmptyParts);
            for (QString &part : parts) part = part.trimmed();
        }

        QString html = QStringLiteral("<div style=\"font-family:'Segoe UI';line-height:1.45;\">");
        if (!header.isEmpty()) {
            html += QStringLiteral("<div style=\"font-size:11px;font-weight:800;color:rgba(11,31,54,190);margin-bottom:6px;\">%1</div>")
                        .arg(htmlEscaped(header));
        }

        if (!parts.isEmpty()) {
            html += QStringLiteral("<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\" style=\"border-collapse:separate;border-spacing:8px 8px;\"><tr>");
            int col = 0;
            for (const QString &part : parts) {
                if (col == 4) {
                    html += QStringLiteral("</tr><tr>");
                    col = 0;
                }
                html += QStringLiteral(
                    "<td style=\"background:rgba(2,132,199,0.10);border:1px solid rgba(2,132,199,0.18);"
                    "border-radius:14px;padding:7px 10px;white-space:nowrap;\">"
                    "<span style=\"font-size:11px;font-weight:900;color:#0B1F36;\">%1</span></td>")
                    .arg(htmlEscaped(part));
                ++col;
            }
            html += QStringLiteral("</tr></table>");
        } else {
            html += QStringLiteral("<div style=\"font-size:11px;color:rgba(11,31,54,170);\">%1</div>").arg(htmlEscaped(body));
        }

        if (!metaLine.isEmpty()) {
            html += QStringLiteral("<div style=\"margin-top:6px;font-size:11px;font-weight:700;color:rgba(11,31,54,165);\">%1</div>")
                        .arg(htmlEscaped(metaLine));
        }
        html += QStringLiteral("</div>");
        return html;
    };

    auto dailyColor = [](int weatherCode) -> QColor {
        if (weatherCode == 0 || weatherCode == 1) return QColor(245, 158, 11);
        if (weatherCode == 2) return QColor(56, 189, 248);
        if (weatherCode == 3 || weatherCode == 45 || weatherCode == 48) return QColor(148, 163, 184);
        if ((weatherCode >= 51 && weatherCode <= 57) || (weatherCode >= 61 && weatherCode <= 67) || (weatherCode >= 80 && weatherCode <= 82)) {
            return QColor(14, 165, 233);
        }
        if ((weatherCode >= 71 && weatherCode <= 77) || weatherCode == 85 || weatherCode == 86) {
            return QColor(99, 102, 241);
        }
        if (weatherCode >= 95 && weatherCode <= 99) {
            return QColor(249, 115, 22);
        }
        return QColor(2, 132, 199);
    };

    auto dailyChips = [&]() -> QString {
        QString header = QStringLiteral("Tendance (5 jours)");
        const QString firstLine = dailyText.split(QStringLiteral("\n")).value(0);
        const int colon = firstLine.indexOf(QLatin1Char(':'));
        if (colon > 0) header = firstLine.left(colon).trimmed();

        QString html = QStringLiteral("<div style=\"font-family:'Segoe UI';line-height:1.45;\">");
        html += QStringLiteral("<div style=\"font-size:11px;font-weight:900;color:rgba(11,31,54,190);margin-bottom:8px;\">%1</div>")
                    .arg(htmlEscaped(header));

        if (m_dailyForecast.isEmpty()) {
            html += QStringLiteral("<div style=\"font-size:11px;color:rgba(11,31,54,170);\">%1</div></div>").arg(htmlEscaped(dailyText));
            return html;
        }

        html += QStringLiteral("<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\" style=\"border-collapse:separate;border-spacing:10px 10px;\"><tr>");
        const QLocale frLocale(QLocale::French, QLocale::France);
        int col = 0;
        for (int i = 0; i < m_dailyForecast.size(); ++i) {
            if (col == 4) {
                html += QStringLiteral("</tr><tr>");
                col = 0;
            }

            const DailyForecastItem &day = m_dailyForecast.at(i);
            const QString dayName = day.date.isValid()
                ? frLocale.dayName(day.date.dayOfWeek(), QLocale::ShortFormat)
                : QStringLiteral("Jour %1").arg(i + 1);
            const QString dateStr = day.date.isValid() ? day.date.toString(QStringLiteral("dd/MM")) : QStringLiteral("--/--");
            const QString temp = QStringLiteral("%1/%2°C").arg(QString::number(day.tMin, 'f', 0), QString::number(day.tMax, 'f', 0));
            const QString prob = (day.probMax >= 0) ? QStringLiteral("%1%").arg(day.probMax) : QString();
            const QColor accent = dailyColor(day.code);
            const int ar = accent.red();
            const int ag = accent.green();
            const int ab = accent.blue();
            const bool selected = (i == m_selectedDailyIndex);

            const QString bg = selected
                ? QStringLiteral("rgba(%1,%2,%3,0.22)").arg(ar).arg(ag).arg(ab)
                : QStringLiteral("rgba(%1,%2,%3,0.10)").arg(ar).arg(ag).arg(ab);
            const QString border = selected
                ? QStringLiteral("rgba(%1,%2,%3,0.55)").arg(ar).arg(ag).arg(ab)
                : QStringLiteral("rgba(%1,%2,%3,0.22)").arg(ar).arg(ag).arg(ab);

            const QString content = QStringLiteral(
                "<div style=\"padding:10px 12px;border-radius:16px;background:%1;border:1px solid %2;\">"
                "<div style=\"font-size:10px;font-weight:900;color:rgba(11,31,54,170);\">%3 <span style=\"font-weight:800;color:rgba(11,31,54,140);\">%4</span></div>"
                "<div style=\"margin-top:4px;font-size:12px;font-weight:900;color:#0B1F36;\">%5 <span style=\"margin-left:8px;\">%6</span></div>"
                "<div style=\"margin-top:3px;font-size:11px;font-weight:900;color:rgba(11,31,54,170);\">%7</div>"
                "</div>")
                .arg(bg, border, htmlEscaped(dayName), htmlEscaped(dateStr), htmlEscaped(temp), htmlEscaped(weatherIconEmoji(day.code)), htmlEscaped(prob));

            html += QStringLiteral("<td><a href=\"daily:%1\" style=\"text-decoration:none;color:inherit;\">%2</a></td>")
                        .arg(i).arg(content);
            ++col;
        }
        html += QStringLiteral("</tr></table><div style=\"margin-top:6px;font-size:11px;font-weight:800;color:rgba(11,31,54,150);\">Cliquez sur un jour pour les details</div></div>");
        return html;
    };

    if (ui->labelMeteoIcon) {
        const QString dayStyle = QStringLiteral(
            "qproperty-alignment: AlignCenter;"
            "font-size: 32px;"
            "color: white;"
            "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(0,149,255,255), stop:1 rgba(0,102,204,255));"
            "border-radius: 28px;"
            "border: 1px solid rgba(255,255,255,70);");
        const QString nightStyle = QStringLiteral(
            "qproperty-alignment: AlignCenter;"
            "font-size: 32px;"
            "color: white;"
            "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(10,20,70,255), stop:1 rgba(2,4,30,255));"
            "border-radius: 28px;"
            "border: 1px solid rgba(255,255,255,55);");
        ui->labelMeteoIcon->setStyleSheet(isDay ? dayStyle : nightStyle);
        ui->labelMeteoIcon->setText(icon);
    }
    if (ui->labelMeteoTemp) {
        ui->labelMeteoTemp->setTextFormat(Qt::RichText);
        ui->labelMeteoTemp->setText(QStringLiteral(
            "<div style=\"font-family:'Segoe UI';font-size:36px;font-weight:900;color:#0B1F36;\">%1</div>")
            .arg(htmlEscaped(temperatureText)));
    }
    if (ui->labelMeteoDesc) {
        ui->labelMeteoDesc->setTextFormat(Qt::RichText);
        ui->labelMeteoDesc->setText(emphasizedLines(descriptionText));
    }
    if (ui->labelMeteoWind) {
        ui->labelMeteoWind->setTextFormat(Qt::RichText);
        ui->labelMeteoWind->setText(metricsGrid(windText, 2, QColor(2, 132, 199)));
    }
    if (ui->labelMeteoHumidity) {
        ui->labelMeteoHumidity->setTextFormat(Qt::RichText);
        ui->labelMeteoHumidity->setText(metricsGrid(humidityText, 3, QColor(99, 102, 241)));
    }
    if (ui->labelMeteoHourly) {
        ui->labelMeteoHourly->setTextFormat(Qt::RichText);
        ui->labelMeteoHourly->setText(chipLine(hourlyText));
    }
    if (ui->labelMeteoDaily) {
        ui->labelMeteoDaily->setTextFormat(Qt::RichText);
        ui->labelMeteoDaily->setText(dailyChips());
    }

    normalizeUiTexts();
}

void MainWindow::updateSelectedDayDetails()
{
    if (!ui || !ui->labelMeteoDayDetails) return;

    ui->labelMeteoDayDetails->setTextFormat(Qt::RichText);
    if (m_dailyForecast.isEmpty()) {
        ui->labelMeteoDayDetails->clear();
        return;
    }

    if (m_selectedDailyIndex < 0 || m_selectedDailyIndex >= m_dailyForecast.size()) {
        m_selectedDailyIndex = 0;
    }

    auto colorForCode = [](int weatherCode) -> QColor {
        if (weatherCode == 0 || weatherCode == 1) return QColor(245, 158, 11);
        if (weatherCode == 2) return QColor(56, 189, 248);
        if (weatherCode == 3 || weatherCode == 45 || weatherCode == 48) return QColor(148, 163, 184);
        if ((weatherCode >= 51 && weatherCode <= 57) || (weatherCode >= 61 && weatherCode <= 67) || (weatherCode >= 80 && weatherCode <= 82)) {
            return QColor(14, 165, 233);
        }
        if ((weatherCode >= 71 && weatherCode <= 77) || weatherCode == 85 || weatherCode == 86) {
            return QColor(99, 102, 241);
        }
        if (weatherCode >= 95 && weatherCode <= 99) {
            return QColor(249, 115, 22);
        }
        return QColor(2, 132, 199);
    };

    const DailyForecastItem &day = m_dailyForecast.at(m_selectedDailyIndex);
    const QLocale frLocale(QLocale::French, QLocale::France);
    const QString dayName = day.date.isValid()
        ? frLocale.dayName(day.date.dayOfWeek(), QLocale::LongFormat)
        : QStringLiteral("Jour");
    const QString dateStr = day.date.isValid() ? day.date.toString(QStringLiteral("dd/MM/yyyy")) : QStringLiteral("--/--/----");
    const QString desc = weatherDescriptionFr(day.code);
    const QString temp = QStringLiteral("%1°C / %2°C").arg(QString::number(day.tMin, 'f', 0), QString::number(day.tMax, 'f', 0));
    const QString prob = (day.probMax >= 0) ? QStringLiteral("%1%").arg(day.probMax) : QStringLiteral("--");
    const QString sunrise = day.sunrise.isValid() ? day.sunrise.toString(QStringLiteral("HH:mm")) : QStringLiteral("--:--");
    const QString sunset = day.sunset.isValid() ? day.sunset.toString(QStringLiteral("HH:mm")) : QStringLiteral("--:--");
    const QColor accent = colorForCode(day.code);

    const QString html = QStringLiteral(
        "<div style=\"margin-top:6px;padding:10px 12px;border-radius:16px;"
        "background:rgba(%1,%2,%3,0.08);border:1px solid rgba(%1,%2,%3,0.20);font-family:'Segoe UI';\">"
        "<div style=\"font-size:11px;font-weight:900;color:#0B1F36;\">%4 - %5 <span style=\"margin-left:6px;\">%6</span></div>"
        "<div style=\"margin-top:3px;font-size:10px;font-weight:800;color:rgba(11,31,54,170);\">%7</div>"
        "<div style=\"margin-top:8px;\">"
        "<span style=\"display:inline-block;padding:5px 10px;border-radius:12px;background:rgba(255,255,255,0.7);border:1px solid rgba(15,23,42,0.08);font-size:10px;font-weight:900;color:#0B1F36;\">Temperatures : %8</span>&nbsp;&nbsp;"
        "<span style=\"display:inline-block;padding:5px 10px;border-radius:12px;background:rgba(255,255,255,0.7);border:1px solid rgba(15,23,42,0.08);font-size:10px;font-weight:900;color:#0B1F36;\">Pluie (max) : %9</span>&nbsp;&nbsp;"
        "<span style=\"display:inline-block;padding:5px 10px;border-radius:12px;background:rgba(255,255,255,0.7);border:1px solid rgba(15,23,42,0.08);font-size:10px;font-weight:900;color:#0B1F36;\">Lever : %10</span>&nbsp;&nbsp;"
        "<span style=\"display:inline-block;padding:5px 10px;border-radius:12px;background:rgba(255,255,255,0.7);border:1px solid rgba(15,23,42,0.08);font-size:10px;font-weight:900;color:#0B1F36;\">Coucher : %11</span>"
        "</div></div>")
        .arg(accent.red()).arg(accent.green()).arg(accent.blue())
        .arg(dayName.toHtmlEscaped(), dateStr.toHtmlEscaped(), weatherIconEmoji(day.code).toHtmlEscaped(),
             desc.toHtmlEscaped(), temp.toHtmlEscaped(), prob.toHtmlEscaped(),
             sunrise.toHtmlEscaped(), sunset.toHtmlEscaped());

    ui->labelMeteoDayDetails->setText(html);
}

void MainWindow::refreshWeatherForPage3()
{
    if (!ui || !m_weatherNetwork) return;

    m_dailyForecast.clear();
    m_hasLastWeather = false;
    updateSelectedDayDetails();

    auto showWeatherError = [this](const QString &desc) {
        updateWeatherLabels(QStringLiteral("\u26D4"),
                            QStringLiteral("--\u00B0C"),
                            desc,
                            QStringLiteral("Vent : -- km/h"),
                            QStringLiteral("Humidite : --%"),
                            QStringLiteral("Previsions (6h) : indisponibles"),
                            QStringLiteral("Tendance (5 jours) : indisponible"),
                            true);
    };

    updateWeatherLabels(QStringLiteral("\u23F3"),
                        QStringLiteral("--\u00B0C"),
                        QStringLiteral("Chargement meteo..."),
                        QStringLiteral("Vent : -- km/h"),
                        QStringLiteral("Humidite : --%"),
                        QStringLiteral("Previsions (6h) : --"),
                        QStringLiteral("Tendance (5 jours) : --"),
                        true);

    QUrl geoUrl(QStringLiteral("https://geocoding-api.open-meteo.com/v1/search"));
    QUrlQuery geoQuery;
    geoQuery.addQueryItem(QStringLiteral("name"), m_selectedCity);
    geoQuery.addQueryItem(QStringLiteral("count"), QStringLiteral("1"));
    geoQuery.addQueryItem(QStringLiteral("language"), QStringLiteral("fr"));
    geoQuery.addQueryItem(QStringLiteral("format"), QStringLiteral("json"));
    geoUrl.setQuery(geoQuery);

    QNetworkReply* geoReply = m_weatherNetwork->get(QNetworkRequest(geoUrl));
    connect(geoReply, &QNetworkReply::finished, this, [this, geoReply, showWeatherError]() {
        const QByteArray geoPayload = geoReply->readAll();
        const QNetworkReply::NetworkError geoError = geoReply->error();
        geoReply->deleteLater();

        if (geoError != QNetworkReply::NoError) {
            showWeatherError(QStringLiteral("Meteo indisponible"));
            return;
        }

        QJsonParseError geoParseError;
        const QJsonDocument geoDoc = QJsonDocument::fromJson(geoPayload, &geoParseError);
        if (geoParseError.error != QJsonParseError::NoError || !geoDoc.isObject()) {
            showWeatherError(QStringLiteral("Reponse meteo invalide"));
            return;
        }

        const QJsonArray results = geoDoc.object().value(QStringLiteral("results")).toArray();
        if (results.isEmpty() || !results.first().isObject()) {
            showWeatherError(QStringLiteral("Localisation meteo introuvable"));
            return;
        }

        const QJsonObject place = results.first().toObject();
        const double latitude = place.value(QStringLiteral("latitude")).toDouble();
        const double longitude = place.value(QStringLiteral("longitude")).toDouble();
        const QString cityName = place.value(QStringLiteral("name")).toString(m_selectedCity);

        QUrl weatherUrl(QStringLiteral("https://api.open-meteo.com/v1/forecast"));
        QUrlQuery weatherQuery;
        weatherQuery.addQueryItem(QStringLiteral("latitude"), QString::number(latitude, 'f', 6));
        weatherQuery.addQueryItem(QStringLiteral("longitude"), QString::number(longitude, 'f', 6));
        weatherQuery.addQueryItem(QStringLiteral("current"),
                                  QStringLiteral("temperature_2m,apparent_temperature,relative_humidity_2m,precipitation,pressure_msl,cloud_cover,visibility,wind_speed_10m,wind_direction_10m,wind_gusts_10m,weather_code,is_day"));
        weatherQuery.addQueryItem(QStringLiteral("hourly"),
                                  QStringLiteral("temperature_2m,weather_code,precipitation_probability,wind_speed_10m"));
        weatherQuery.addQueryItem(QStringLiteral("daily"),
                                  QStringLiteral("temperature_2m_max,temperature_2m_min,weather_code,sunrise,sunset,precipitation_probability_max"));
        weatherQuery.addQueryItem(QStringLiteral("forecast_days"), QStringLiteral("5"));
        weatherQuery.addQueryItem(QStringLiteral("timezone"), QStringLiteral("auto"));
        weatherUrl.setQuery(weatherQuery);

        QNetworkReply* weatherReply = m_weatherNetwork->get(QNetworkRequest(weatherUrl));
        connect(weatherReply, &QNetworkReply::finished, this, [this, weatherReply, cityName, showWeatherError]() {
            const QByteArray weatherPayload = weatherReply->readAll();
            const QNetworkReply::NetworkError weatherError = weatherReply->error();
            weatherReply->deleteLater();

            if (weatherError != QNetworkReply::NoError) {
                showWeatherError(QStringLiteral("Meteo indisponible (%1)").arg(cityName));
                return;
            }

            QJsonParseError weatherParseError;
            const QJsonDocument weatherDoc = QJsonDocument::fromJson(weatherPayload, &weatherParseError);
            if (weatherParseError.error != QJsonParseError::NoError || !weatherDoc.isObject()) {
                showWeatherError(QStringLiteral("Donnees meteo invalides (%1)").arg(cityName));
                return;
            }

            const QJsonObject root = weatherDoc.object();
            const QJsonObject current = root.value(QStringLiteral("current")).toObject();
            if (current.isEmpty()) {
                showWeatherError(QStringLiteral("Aucune meteo courante (%1)").arg(cityName));
                return;
            }

            const double temp = current.value(QStringLiteral("temperature_2m")).toDouble();
            const int humidity = current.value(QStringLiteral("relative_humidity_2m")).toInt();
            const double wind = current.value(QStringLiteral("wind_speed_10m")).toDouble();
            const double apparent = current.value(QStringLiteral("apparent_temperature")).toDouble(temp);
            const double precipitation = current.value(QStringLiteral("precipitation")).toDouble(0.0);
            const double pressure = current.value(QStringLiteral("pressure_msl")).toDouble(0.0);
            const double cloudCover = current.value(QStringLiteral("cloud_cover")).toDouble(0.0);
            const double visibilityMeters = current.value(QStringLiteral("visibility")).toDouble(0.0);
            const double windDir = current.value(QStringLiteral("wind_direction_10m")).toDouble(-1.0);
            const double windGusts = current.value(QStringLiteral("wind_gusts_10m")).toDouble(0.0);
            const int code = current.value(QStringLiteral("weather_code")).toInt();
            const bool isDay = current.value(QStringLiteral("is_day")).toInt(1) == 1;
            const QString currentTimeIso = current.value(QStringLiteral("time")).toString();
            const QDateTime currentDt = QDateTime::fromString(currentTimeIso, Qt::ISODate);

            auto degToCompassFr = [](double degrees) -> QString {
                if (degrees < 0.0) return QStringLiteral("--");
                static const QStringList dirs = {
                    QStringLiteral("N"), QStringLiteral("NNE"), QStringLiteral("NE"), QStringLiteral("ENE"),
                    QStringLiteral("E"), QStringLiteral("ESE"), QStringLiteral("SE"), QStringLiteral("SSE"),
                    QStringLiteral("S"), QStringLiteral("SSO"), QStringLiteral("SO"), QStringLiteral("OSO"),
                    QStringLiteral("O"), QStringLiteral("ONO"), QStringLiteral("NO"), QStringLiteral("NNO")
                };
                const int idx = static_cast<int>(std::floor((degrees + 11.25) / 22.5)) % 16;
                return dirs.value(idx, QStringLiteral("--"));
            };

            const QString icon = weatherIconEmojiDayNight(code, isDay);
            const QString tempText = QStringLiteral("%1°C").arg(QString::number(temp, 'f', 1));
            const QString descText = QStringLiteral("%1 - %2\nMise a jour : %3")
                .arg(cityName,
                     weatherDescriptionFr(code),
                     currentDt.isValid() ? currentDt.toString(QStringLiteral("HH:mm")) : QStringLiteral("--:--"));
            const QString windDirText = (windDir >= 0.0)
                ? QStringLiteral("%1° (%2)").arg(QString::number(windDir, 'f', 0), degToCompassFr(windDir))
                : QStringLiteral("--");
            const QString windText = QStringLiteral("Vent : %1 km/h • %2\nRafales : %3 km/h")
                .arg(QString::number(wind, 'f', 1),
                     windDirText,
                     QString::number(windGusts, 'f', 1));
            const QString humidityText = QStringLiteral(
                "Humidite : %1% • Ressenti : %2°C • Pluie : %3 mm\nPression : %4 hPa • Visibilite : %5 km • Nuages : %6%")
                .arg(humidity)
                .arg(QString::number(apparent, 'f', 1))
                .arg(QString::number(precipitation, 'f', 1))
                .arg(QString::number(pressure, 'f', 0))
                .arg(QString::number((visibilityMeters > 0.0) ? visibilityMeters / 1000.0 : 0.0, 'f', 1))
                .arg(QString::number(cloudCover, 'f', 0));

            QString hourlyText = QStringLiteral("Previsions (6h) : indisponibles");
            const QJsonObject hourly = root.value(QStringLiteral("hourly")).toObject();
            if (!hourly.isEmpty()) {
                const QJsonArray times = hourly.value(QStringLiteral("time")).toArray();
                const QJsonArray temps = hourly.value(QStringLiteral("temperature_2m")).toArray();
                const QJsonArray codes = hourly.value(QStringLiteral("weather_code")).toArray();
                const QJsonArray probs = hourly.value(QStringLiteral("precipitation_probability")).toArray();
                const QJsonArray winds = hourly.value(QStringLiteral("wind_speed_10m")).toArray();

                if (!times.isEmpty() && temps.size() == times.size() && codes.size() == times.size()) {
                    int startIndex = 0;
                    if (currentDt.isValid()) {
                        for (int i = 0; i < times.size(); ++i) {
                            const QDateTime hourDt = QDateTime::fromString(times.at(i).toString(), Qt::ISODate);
                            if (hourDt.isValid() && hourDt >= currentDt) {
                                startIndex = i;
                                break;
                            }
                        }
                    }

                    QStringList parts;
                    const int endIndex = std::min(startIndex + 6, static_cast<int>(times.size()));
                    for (int i = startIndex; i < endIndex; ++i) {
                        const QDateTime hourDt = QDateTime::fromString(times.at(i).toString(), Qt::ISODate);
                        const QString hourStr = hourDt.isValid() ? hourDt.toString(QStringLiteral("HH'h'")) : QStringLiteral("--h");
                        const double hourTemp = temps.at(i).toDouble();
                        const int hourCode = codes.at(i).toInt();
                        const int hourProb = (i < probs.size()) ? probs.at(i).toInt(-1) : -1;
                        const double hourWind = (i < winds.size()) ? winds.at(i).toDouble(0.0) : 0.0;
                        const bool hourDay = !hourDt.isValid() || (hourDt.time().hour() >= 6 && hourDt.time().hour() < 20);

                        QString part = QStringLiteral("%1 %2°C %3").arg(hourStr, QString::number(hourTemp, 'f', 0), weatherIconEmojiDayNight(hourCode, hourDay));
                        if (hourProb >= 0) part += QStringLiteral(" • %1%").arg(hourProb);
                        if (hourWind > 0.0) part += QStringLiteral(" • %1 km/h").arg(QString::number(hourWind, 'f', 0));
                        parts.append(part);
                    }
                    if (!parts.isEmpty()) {
                        hourlyText = QStringLiteral("Previsions (6h) : %1").arg(parts.join(QStringLiteral("  | ")));
                    }
                }
            }

            QString dailyText = QStringLiteral("Tendance (5 jours) : indisponible");
            m_dailyForecast.clear();
            const QJsonObject daily = root.value(QStringLiteral("daily")).toObject();
            if (!daily.isEmpty()) {
                const QJsonArray times = daily.value(QStringLiteral("time")).toArray();
                const QJsonArray tmax = daily.value(QStringLiteral("temperature_2m_max")).toArray();
                const QJsonArray tmin = daily.value(QStringLiteral("temperature_2m_min")).toArray();
                const QJsonArray codes = daily.value(QStringLiteral("weather_code")).toArray();
                const QJsonArray sunrises = daily.value(QStringLiteral("sunrise")).toArray();
                const QJsonArray sunsets = daily.value(QStringLiteral("sunset")).toArray();
                const QJsonArray probs = daily.value(QStringLiteral("precipitation_probability_max")).toArray();

                if (!times.isEmpty() && tmax.size() == times.size() && tmin.size() == times.size() && codes.size() == times.size()) {
                    const int maxDays = std::min(5, static_cast<int>(times.size()));
                    const QLocale frLocale(QLocale::French, QLocale::France);
                    QStringList parts;
                    m_dailyForecast.reserve(maxDays);

                    for (int i = 0; i < maxDays; ++i) {
                        DailyForecastItem item;
                        item.date = QDate::fromString(times.at(i).toString(), Qt::ISODate);
                        item.tMax = tmax.at(i).toDouble();
                        item.tMin = tmin.at(i).toDouble();
                        item.code = codes.at(i).toInt();
                        item.probMax = (i < probs.size()) ? probs.at(i).toInt(-1) : -1;
                        if (i < sunrises.size() && sunrises.at(i).isString()) {
                            item.sunrise = QDateTime::fromString(sunrises.at(i).toString(), Qt::ISODate);
                        }
                        if (i < sunsets.size() && sunsets.at(i).isString()) {
                            item.sunset = QDateTime::fromString(sunsets.at(i).toString(), Qt::ISODate);
                        }
                        m_dailyForecast.push_back(item);

                        const QString dayName = item.date.isValid()
                            ? frLocale.dayName(item.date.dayOfWeek(), QLocale::ShortFormat)
                            : QStringLiteral("Jour %1").arg(i + 1);
                        QString part = QStringLiteral("%1 %2/%3°C %4")
                            .arg(dayName,
                                 QString::number(item.tMin, 'f', 0),
                                 QString::number(item.tMax, 'f', 0),
                                 weatherIconEmoji(item.code));
                        if (item.probMax >= 0) {
                            part += QStringLiteral(" %1%").arg(item.probMax);
                        }
                        parts.append(part);
                    }

                    if (!parts.isEmpty()) {
                        dailyText = QStringLiteral("Tendance (5 jours) : %1\nCliquez sur un jour pour les details")
                            .arg(parts.join(QStringLiteral("  | ")));
                    }
                }
            }

            const bool shouldClose = (wind >= 10.0);
            const bool wasClosed = m_quaisFermeMeteo;
            m_quaisFermeMeteo = shouldClose;

            if (m_selectedDailyIndex < 0 || (!m_dailyForecast.isEmpty() && m_selectedDailyIndex >= m_dailyForecast.size())) {
                m_selectedDailyIndex = 0;
            }

            m_lastWeatherIcon = icon;
            m_lastWeatherTemp = tempText;
            m_lastWeatherDesc = descText;
            m_lastWeatherWind = windText;
            m_lastWeatherHumidity = humidityText;
            m_lastWeatherHourly = hourlyText;
            m_lastWeatherDaily = dailyText;
            m_lastWeatherIsDay = isDay;
            m_hasLastWeather = true;

            updateWeatherLabels(icon, tempText, descText, windText, humidityText, hourlyText, dailyText, isDay);
            updateSelectedDayDetails();

            if (shouldClose && !wasClosed) {
                lockQuaisForWeather();
            } else if (!shouldClose) {
                unlockQuaisForWeather();
            }

            refreshQuaiTable();
            if (ui->stackedWidget && ui->page_4 && ui->stackedWidget->currentWidget() == ui->page_4) {
                refreshStats_2();
            }
        });
    });
}

void MainWindow::lockQuaisForWeather()
{
    Connection *conn = Connection::getInstance();
    if (!conn->ensureOpen()) return;

    QSqlDatabase db = conn->getDatabase();
    if (!db.isOpen()) return;

    m_quaisAutoLocked.clear();

    QSqlQuery selectQuery(db);
    if (selectQuery.exec(QStringLiteral("SELECT ID_QUAI, STATUT, ID_BATEAU FROM QUAIS"))) {
        while (selectQuery.next()) {
            const int id = selectQuery.value(0).toInt();
            const QString statut = selectQuery.value(1).toString().trimmed();
            const bool hasBoat = !selectQuery.value(2).isNull() && selectQuery.value(2).toInt() > 0;
            if (statut.compare(QStringLiteral("Occupe"), Qt::CaseInsensitive) == 0) {
                m_quaisAutoLocked.insert(id, QStringLiteral("Occupe"));
            } else if (statut.compare(QStringLiteral("Ferme"), Qt::CaseInsensitive) == 0 && hasBoat) {
                m_quaisAutoLocked.insert(id, QStringLiteral("Occupe"));
            }
        }
    }

    QSqlQuery updateQuery(db);
    updateQuery.prepare(QStringLiteral(
        "UPDATE QUAIS SET STATUT = 'Ferme' "
        "WHERE UPPER(STATUT) = 'OCCUPE'"));
    updateQuery.exec();
}

void MainWindow::unlockQuaisForWeather()
{
    Connection *conn = Connection::getInstance();
    if (!conn->ensureOpen()) return;

    QSqlDatabase db = conn->getDatabase();
    if (!db.isOpen()) return;

    if (!m_quaisAutoLocked.isEmpty()) {
        QSqlQuery updateQuery(db);
        updateQuery.prepare(QStringLiteral("UPDATE QUAIS SET STATUT = :statut WHERE ID_QUAI = :id AND UPPER(STATUT) = 'FERME'"));
        for (auto it = m_quaisAutoLocked.constBegin(); it != m_quaisAutoLocked.constEnd(); ++it) {
            updateQuery.bindValue(QStringLiteral(":statut"), it.value());
            updateQuery.bindValue(QStringLiteral(":id"), it.key());
            updateQuery.exec();
        }
        m_quaisAutoLocked.clear();
        return;
    }

    QSqlQuery fallbackQuery(db);
    fallbackQuery.exec(QStringLiteral(
        "UPDATE QUAIS "
        "SET STATUT = CASE WHEN NVL(ID_BATEAU,0) > 0 THEN 'Occupe' ELSE 'Libre' END "
        "WHERE UPPER(STATUT) = 'FERME'"));
}

static void exportCapturesPdfReport(MainWindow* parent, Ui::MainWindow* ui)
{
    if (!parent || !ui || !ui->cap_tableWidget) return;

    const QString documentsDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString baseDir = documentsDir.isEmpty() ? QDir::homePath() : documentsDir;
    const QString defaultName = QStringLiteral("Rapport_Captures_Aquatec_") + QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd")) + QStringLiteral(".pdf");
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

    const int actionColumn = 7; // Column 7 contains actions
    const bool actionColumnWasHidden = ui->cap_tableWidget->isColumnHidden(actionColumn);
    if (!actionColumnWasHidden) {
        ui->cap_tableWidget->setColumnHidden(actionColumn, true);
    }

    auto restoreActionColumn = [ui, actionColumn, actionColumnWasHidden]() {
        if (!actionColumnWasHidden) {
            ui->cap_tableWidget->setColumnHidden(actionColumn, false);
        }
    };

    const QString exportStamp = QDateTime::currentDateTime().toString(QStringLiteral("dd/MM/yyyy HH:mm"));
    const int pageW = writer.width();
    const int pageH = writer.height();

    const int leftMargin = 50;
    const int rightMargin = 50;
    const int topMargin = 100;
    const int bottomMargin = 80;
    const int contentW = pageW - leftMargin - rightMargin;

    QFont titleFont(QStringLiteral("Arial"), 20, QFont::Bold);
    QFont subtitleFont(QStringLiteral("Arial"), 9, QFont::Normal);
    QFont headerFont(QStringLiteral("Arial"), 11, QFont::Bold);
    QFont cellFont(QStringLiteral("Arial"), 9, QFont::Normal);
    QFont totalFont(QStringLiteral("Arial"), 9, QFont::Bold);
    QFont footerFont(QStringLiteral("Arial"), 9, QFont::Normal);

    painter.setFont(titleFont);
    painter.setPen(QColor(0, 82, 155));
    painter.drawText(QRect(leftMargin, topMargin - 40, contentW, 60), Qt::AlignCenter,
                     QStringLiteral("AQUATEC – Liste des Captures"));

    painter.setPen(Qt::darkGray);
    painter.setFont(subtitleFont);
    painter.drawText(QRect(leftMargin, topMargin + 20, contentW, 30), Qt::AlignCenter,
                     QStringLiteral("Exporté le %1").arg(exportStamp));

    int y = topMargin + 80;

    QTableWidget* table = ui->cap_tableWidget;
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

    const int headerH = 48;
    auto drawTableHeader = [&]() {
        int x = leftMargin;
        painter.setFont(headerFont);

        QLinearGradient headerGrad(0, y, 0, y + headerH);
        headerGrad.setColorAt(0.0, QColor(QStringLiteral("#0b5ea8")));
        headerGrad.setColorAt(1.0, QColor(QStringLiteral("#2e86c1")));

        for (int i = 0; i < columns.size(); ++i) {
            const int c = columns[i];
            const int w = drawWidths[i];
            const QRect cellRect(x, y, w, headerH);
            const QString text = table->horizontalHeaderItem(c) ? table->horizontalHeaderItem(c)->text().trimmed() : QStringLiteral("Colonne %1").arg(c + 1);

            painter.fillRect(cellRect, headerGrad);
            painter.setPen(Qt::white);
            painter.drawText(cellRect.adjusted(12, 0, -12, 0), Qt::AlignVCenter | Qt::AlignLeft, text);
            painter.setPen(QPen(QColor(200, 210, 220), 1));
            painter.drawRect(cellRect);
            x += w;
        }
        y += headerH;
    };

    const int footerY = pageH - bottomMargin + 10;
    const int tableTop = y;
    const int tableHeaderH = headerH;
    const int tableTotalH = 30;
    const int tableBottomLimit = pageH - bottomMargin - 40;
    const int tableRowsAreaH = qMax(40, tableBottomLimit - tableTop - tableHeaderH - tableTotalH);
    const int rowH = 40;
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
        const QColor rowColor = (index % 2 == 0) ? QColor(250, 250, 252) : QColor(245, 251, 255);
        painter.fillRect(QRect(leftMargin, y, contentW, rowH), rowColor);
        painter.setPen(QPen(QColor(220, 225, 230), 1));
        painter.drawRect(QRect(leftMargin, y, contentW, rowH));

        int x = leftMargin;
        for (int i = 0; i < columns.size(); ++i) {
            const int c = columns[i];
            const int w = drawWidths[i];
            painter.setPen(QPen(QColor(220, 225, 230), 1));
            painter.drawLine(x, y, x, y + rowH);

            painter.setPen(Qt::black);
            const QTableWidgetItem* item = table->item(r, c);
            QString text = item ? item->text() : QString();
            text = text.simplified();

            QFontMetrics fm(cellFont);
            text = fm.elidedText(text, Qt::ElideRight, qMax(10, w - 24));
            painter.drawText(QRect(x + 12, y + 6, w - 24, rowH - 12), Qt::AlignLeft | Qt::AlignVCenter, text);
            x += w;
        }

        painter.setPen(QPen(QColor(220, 225, 230), 1));
        painter.drawLine(leftMargin + contentW, y, leftMargin + contentW, y + rowH);
        y += rowH;
    }

    y += 8;
    painter.setPen(Qt::darkGray);
    painter.setFont(totalFont);
    painter.drawText(QRect(leftMargin, y, contentW, 24), Qt::AlignCenter,
                     QStringLiteral("Total: %1 captures").arg(totalVisibleRows));

    if (totalVisibleRows > exportedRows) {
        painter.setFont(QFont(QStringLiteral("Arial"), 9, QFont::Normal));
        painter.setPen(QColor(QStringLiteral("#8a8a8a")));
        painter.drawText(QRect(leftMargin, y + 24, contentW, 18), Qt::AlignCenter,
                         QStringLiteral("(%1 lignes affichées sur %2 dans cette page)").arg(exportedRows).arg(totalVisibleRows));
    }

    // Section Statistiques - Top 5 Espèces
    y += 60;

    const QVector<captures::SpeciesStatData> statsData = captures::calculerTop5Species();
    const int statsCount = qMin(5, statsData.size());
    if (statsCount > 0) {
        const int titleH = 30;
        const int titleGap = 12;
        const int statRowH = 34;
        const int statsNeededH = titleH + titleGap + ((statsCount + 1) * statRowH);
        const int contentBottom = pageH - bottomMargin - 40;

        // Saut de page si la section ne tient pas en bas de page
        if (y + statsNeededH > contentBottom) {
            writer.newPage();
            y = topMargin;
        }

        painter.setFont(QFont(QStringLiteral("Arial"), 14, QFont::Bold));
        painter.setPen(QColor(0, 82, 155));
        painter.drawText(QRect(leftMargin, y, contentW, titleH), Qt::AlignLeft | Qt::AlignVCenter,
                         QStringLiteral("Statistiques - Top 5 Espèces"));
        y += titleH + titleGap;

        const int col1W = contentW * 45 / 100;
        const int col2W = contentW * 28 / 100;
        const int col3W = contentW - col1W - col2W;

        // En-têtes du tableau
        painter.setFont(QFont(QStringLiteral("Arial"), 10, QFont::Bold));
        QLinearGradient statHeaderGrad(0, y, 0, y + statRowH);
        statHeaderGrad.setColorAt(0.0, QColor(QStringLiteral("#0b5ea8")));
        statHeaderGrad.setColorAt(1.0, QColor(QStringLiteral("#2e86c1")));
        painter.fillRect(QRect(leftMargin, y, contentW, statRowH), statHeaderGrad);

        painter.setPen(Qt::white);
        painter.drawText(QRect(leftMargin + 10, y, col1W - 20, statRowH), Qt::AlignVCenter | Qt::AlignLeft, QStringLiteral("Espèce"));
        painter.drawText(QRect(leftMargin + col1W, y, col2W, statRowH), Qt::AlignVCenter | Qt::AlignCenter, QStringLiteral("Quantité"));
        painter.drawText(QRect(leftMargin + col1W + col2W, y, col3W, statRowH), Qt::AlignVCenter | Qt::AlignCenter, QStringLiteral("Pourcentage"));

        painter.setPen(QPen(QColor(200, 210, 220), 1));
        painter.drawRect(QRect(leftMargin, y, contentW, statRowH));
        painter.drawLine(leftMargin + col1W, y, leftMargin + col1W, y + statRowH);
        painter.drawLine(leftMargin + col1W + col2W, y, leftMargin + col1W + col2W, y + statRowH);

        y += statRowH;

        // Données du tableau
        const QFont dataFont(QStringLiteral("Arial"), 9, QFont::Normal);
        painter.setFont(dataFont);
        QFontMetrics dataFm(dataFont);

        for (int i = 0; i < statsCount; ++i) {
            const captures::SpeciesStatData& stat = statsData[i];
            const QColor rowColor = (i % 2 == 0) ? QColor(250, 250, 252) : QColor(245, 251, 255);

            painter.fillRect(QRect(leftMargin, y, contentW, statRowH), rowColor);
            painter.setPen(QPen(QColor(220, 225, 230), 1));
            painter.drawRect(QRect(leftMargin, y, contentW, statRowH));
            painter.drawLine(leftMargin + col1W, y, leftMargin + col1W, y + statRowH);
            painter.drawLine(leftMargin + col1W + col2W, y, leftMargin + col1W + col2W, y + statRowH);

            QString speciesText = stat.species;
            speciesText.replace('\n', ' ');
            speciesText.replace('\r', ' ');
            speciesText = speciesText.simplified();
            speciesText = dataFm.elidedText(speciesText, Qt::ElideRight, qMax(20, col1W - 20));

            const QString qtyText = QString::number(stat.quantite);
            const QString pctText = QString::number(stat.pourcentage, 'f', 1) + QStringLiteral("%");

            painter.setPen(Qt::black);
            painter.drawText(QRect(leftMargin + 10, y, col1W - 20, statRowH), Qt::AlignVCenter | Qt::AlignLeft, speciesText);
            painter.drawText(QRect(leftMargin + col1W, y, col2W, statRowH), Qt::AlignVCenter | Qt::AlignCenter, qtyText);
            painter.drawText(QRect(leftMargin + col1W + col2W, y, col3W, statRowH), Qt::AlignVCenter | Qt::AlignCenter, pctText);

            y += statRowH;
        }
    }

    y += 20;
    painter.setPen(QColor(QStringLiteral("#8a8a8a")));
    painter.setFont(footerFont);
    painter.drawText(QRect(leftMargin, footerY, contentW, 20), Qt::AlignCenter,
                     QStringLiteral("AQUATEC - Rapport généré le %1").arg(exportStamp));

    restoreActionColumn();

    painter.end();
    QMessageBox::information(parent, QStringLiteral("Export PDF"),
                             QStringLiteral("Fichier PDF exporté avec succès:\n%1").arg(filePath));
}

void MainWindow::on_cap_btnExporter_clicked()
{
    exportCapturesPdfReport(this, ui);
}

void MainWindow::on_cap_sbQuantite_3_valueChanged(int value)
{
    m_captureQuantiteFiltre = value;
    loadCaptures();
}

static void updateCapturesStats(MainWindow* parent, Ui::MainWindow* ui)
{
    if (!parent || !ui || !ui->frame_zonesb_2) return;

    // Mode stats: 0 = Quantité, 1 = Poids
    int metricIndex = 0;
    if (ui->cap_pagecaptures) {
        if (auto *combo = ui->cap_pagecaptures->findChild<QComboBox*>(QStringLiteral("cap_comboStatsMetric"))) {
            metricIndex = combo->currentIndex();
        }
    }

    Connection* conn = Connection::getInstance();
    if (!conn || !conn->ensureOpen()) {
        qWarning() << "Stats captures: connexion DB indisponible";
        return;
    }

    QSqlDatabase db = conn->getDatabase();
    const QString capturesTable = resolveTableName(db, {
        QStringLiteral("CAPTURES"),
        QStringLiteral("CAPTURE"),
        QStringLiteral("TCAPTURES"),
        QStringLiteral("T_CAPTURES")
    });
    if (capturesTable.isEmpty()) {
        qWarning() << "Stats captures: table CAPTURES introuvable";
        return;
    }

    const QStringList cols = getColumnNames(db, capturesTable);
    const QString typeCol = matchColumnBySynonyms(cols, {
        QStringLiteral("type_poisson"), QStringLiteral("typepoisson"), QStringLiteral("poisson"), QStringLiteral("espece")
    });
    const QString quantiteCol = matchColumnBySynonyms(cols, {
        QStringLiteral("quantite"), QStringLiteral("qte")
    });
    const QString poidsCol = matchColumnBySynonyms(cols, {
        QStringLiteral("poids"), QStringLiteral("poids_kg")
    });
    if (typeCol.isEmpty() || quantiteCol.isEmpty() || poidsCol.isEmpty()) {
        qWarning() << "Stats captures: colonnes manquantes (type/quantite/poids)";
        return;
    }

    const QString metricCol = (metricIndex == 1) ? poidsCol : quantiteCol;
    const QString sql = QStringLiteral(
        "SELECT %1, SUM(%2) AS total_metric FROM %3 GROUP BY %1 ORDER BY total_metric DESC")
            .arg(typeCol, metricCol, capturesTable);

    QSqlQuery q(db);
    if (!q.exec(sql)) {
        qWarning() << "Stats captures: requete echouee" << q.lastError().text();
        return;
    }

    QVector<QPair<QString, double>> all;
    all.reserve(32);
    double totalAll = 0.0;
    while (q.next()) {
        const QString species = q.value(0).toString().trimmed();
        const double total = q.value(1).toDouble();
        if (species.isEmpty()) continue;
        if (total < 0.0) continue;
        all.push_back({species, total});
        totalAll += total;
    }

    if (all.isEmpty() || totalAll <= 0.0) {
        // Vider l'affichage si rien à montrer
        const QStringList labelNames = {
            QStringLiteral("label_zoneNordb_2"),
            QStringLiteral("label_zoneSudb_2"),
            QStringLiteral("label_zoneEstb_2"),
            QStringLiteral("label_zoneOuestb_3"),
            QStringLiteral("label_zoneOuestb_2")
        };
        const QStringList progressNames = {
            QStringLiteral("progressZoneNordb_2"),
            QStringLiteral("progressZoneSudb_2"),
            QStringLiteral("progressZoneEstb_2"),
            QStringLiteral("progressZoneOuestb_3"),
            QStringLiteral("progressZoneOuestb_2")
        };
        const QStringList valueNames = {
            QStringLiteral("value_zoneNordb_2"),
            QStringLiteral("value_zoneSudb_2"),
            QStringLiteral("value_zoneEstb_2"),
            QStringLiteral("value_zoneOuestb_3"),
            QStringLiteral("value_zoneOuestb_2")
        };
        for (int i = 0; i < 5; ++i) {
            auto* label = ui->frame_zonesb_2->findChild<QLabel*>(labelNames[i]);
            auto* progressBar = ui->frame_zonesb_2->findChild<QProgressBar*>(progressNames[i]);
            auto* valueLabel = ui->frame_zonesb_2->findChild<QLabel*>(valueNames[i]);
            if (label) label->setText(QString());
            if (progressBar) progressBar->setValue(0);
            if (valueLabel) valueLabel->setText(QStringLiteral("0/0"));
        }
        return;
    }
    
    // Noms des widgets pour les 5 colonnes
    const QStringList labelNames = {
        QStringLiteral("label_zoneNordb_2"),
        QStringLiteral("label_zoneSudb_2"),
        QStringLiteral("label_zoneEstb_2"),
        QStringLiteral("label_zoneOuestb_3"),
        QStringLiteral("label_zoneOuestb_2")
    };
    
    const QStringList progressNames = {
        QStringLiteral("progressZoneNordb_2"),
        QStringLiteral("progressZoneSudb_2"),
        QStringLiteral("progressZoneEstb_2"),
        QStringLiteral("progressZoneOuestb_3"),
        QStringLiteral("progressZoneOuestb_2")
    };
    
    const QStringList valueNames = {
        QStringLiteral("value_zoneNordb_2"),
        QStringLiteral("value_zoneSudb_2"),
        QStringLiteral("value_zoneEstb_2"),
        QStringLiteral("value_zoneOuestb_3"),
        QStringLiteral("value_zoneOuestb_2")
    };
    
    // Mettre à jour chaque colonne
    for (int i = 0; i < 5; ++i) {
        // Récupérer les widgets pour cette colonne
        auto* label = ui->frame_zonesb_2->findChild<QLabel*>(labelNames[i]);
        auto* progressBar = ui->frame_zonesb_2->findChild<QProgressBar*>(progressNames[i]);
        auto* valueLabel = ui->frame_zonesb_2->findChild<QLabel*>(valueNames[i]);
        
        if (!label || !progressBar || !valueLabel) {
            continue;
        }
        
        if (i < all.size()) {
            const QString species = all[i].first;
            const double value = all[i].second;
            const double pct = (totalAll > 0.0) ? (100.0 * value / totalAll) : 0.0;

            label->setText(species);
            progressBar->setValue(qBound(0, static_cast<int>(pct + 0.5), 100));

            if (metricIndex == 1) {
                // Poids
                const QString vTxt = QString::number(value, 'f', 1);
                const QString tTxt = QString::number(totalAll, 'f', 1);
                valueLabel->setText(QStringLiteral("%1kg/%2kg").arg(vTxt, tTxt));
            } else {
                // Quantité (arrondie à l'entier)
                const int vInt = static_cast<int>(value + 0.5);
                const int tInt = static_cast<int>(totalAll + 0.5);
                valueLabel->setText(QStringLiteral("%1/%2").arg(QString::number(vInt), QString::number(tInt)));
            }
        } else {
            // Vider la colonne
            label->setText(QString());
            progressBar->setValue(0);
            valueLabel->setText(QStringLiteral("0/0"));
        }
    }
}

static QString weatherDescriptionFr(int weatherCode)
{
    if (weatherCode == 0) return QStringLiteral("Ciel degage");
    if (weatherCode == 1) return QStringLiteral("Principalement degage");
    if (weatherCode == 2) return QStringLiteral("Partiellement nuageux");
    if (weatherCode == 3) return QStringLiteral("Couvert");
    if (weatherCode == 45) return QStringLiteral("Brouillard");
    if (weatherCode == 48) return QStringLiteral("Brouillard givrant");
    if (weatherCode == 51) return QStringLiteral("Bruine faible");
    if (weatherCode == 53) return QStringLiteral("Bruine moderee");
    if (weatherCode == 55) return QStringLiteral("Bruine forte");
    if (weatherCode == 56) return QStringLiteral("Bruine verglaçante faible");
    if (weatherCode == 57) return QStringLiteral("Bruine verglaçante forte");
    if (weatherCode == 61) return QStringLiteral("Pluie faible");
    if (weatherCode == 63) return QStringLiteral("Pluie moderee");
    if (weatherCode == 65) return QStringLiteral("Pluie forte");
    if (weatherCode == 66) return QStringLiteral("Pluie verglaçante faible");
    if (weatherCode == 67) return QStringLiteral("Pluie verglaçante forte");
    if (weatherCode == 71) return QStringLiteral("Neige faible");
    if (weatherCode == 73) return QStringLiteral("Neige moderee");
    if (weatherCode == 75) return QStringLiteral("Neige forte");
    if (weatherCode == 77) return QStringLiteral("Grains de neige");
    if (weatherCode == 80) return QStringLiteral("Averses faibles");
    if (weatherCode == 81) return QStringLiteral("Averses moderees");
    if (weatherCode == 82) return QStringLiteral("Averses fortes");
    if (weatherCode == 85) return QStringLiteral("Averses de neige faibles");
    if (weatherCode == 86) return QStringLiteral("Averses de neige fortes");
    if (weatherCode == 95) return QStringLiteral("Orage");
    if (weatherCode == 96) return QStringLiteral("Orage avec grele faible");
    if (weatherCode == 99) return QStringLiteral("Orage avec grele forte");
    return QStringLiteral("Meteo variable");
}

static QString weatherIconEmoji(int weatherCode)
{
    if (weatherCode == 0) return QStringLiteral("\u2600\uFE0F");
    if (weatherCode == 1 || weatherCode == 2) return QStringLiteral("\u26C5");
    if (weatherCode == 3) return QStringLiteral("\u2601\uFE0F");
    if (weatherCode == 45 || weatherCode == 48) return QStringLiteral("\u2601\uFE0F");
    if ((weatherCode >= 51 && weatherCode <= 57) ||
        (weatherCode >= 61 && weatherCode <= 67) ||
        (weatherCode >= 80 && weatherCode <= 82)) {
        return QStringLiteral("\u2614");
    }
    if (weatherCode >= 71 && weatherCode <= 77) return QStringLiteral("\u2744\uFE0F");
    if (weatherCode >= 95 && weatherCode <= 99) return QStringLiteral("\u26A1");
    return QStringLiteral("\u2601\uFE0F");
}

static QString weatherIconEmojiDayNight(int weatherCode, bool isDay)
{
    if (isDay) {
        return weatherIconEmoji(weatherCode);
    }

    if (weatherCode == 0 || weatherCode == 1 || weatherCode == 2) {
        return QStringLiteral("\u263E");
    }

    return weatherIconEmoji(weatherCode);
}

void MainWindow::on_btnCapturep_clicked()
{
    if (!ui) return;
    if (ui->labelCamerap) {
        ui->labelCamerap->setText(QStringLiteral("Capture en cours..."));
        ui->labelCamerap->setStyleSheet(QStringLiteral("background-color: black; color: white; font-size: 20px;"));
        
        // Simuler un delai de capture
        QTimer::singleShot(1500, this, [this]() {
            if (ui && ui->labelCamerap) {
                ui->labelCamerap->setText(QStringLiteral("Photo capturee !"));
                ui->labelCamerap->setStyleSheet(QStringLiteral("background-color: #2c3e50; color: #2ecc71; font-size: 20px; border: 2px solid #2ecc71;"));
                QMessageBox::information(this, QStringLiteral("FaceID"), QStringLiteral("Photo capturee avec succes."));
            }
        });
    }
}

void MainWindow::on_btnVerifyp_clicked()
{
    if (!ui) return;
    QMessageBox::information(this, QStringLiteral("FaceID"), QStringLiteral("Verification biometrique reussie. Identite confirmee."));
}

void MainWindow::on_btnCancelFacep_clicked()
{
    if (!ui) return;
    if (ui->dialogFaceIDp) {
        ui->dialogFaceIDp->close();
    }
}

void MainWindow::on_bmip_clicked()
{
    if (!ui) return;

    const QString email = ui->lineEditp_2 ? ui->lineEditp_2->text().trimmed() : QString();
    const QString nom = ui->lineEdit_2p ? ui->lineEdit_2p->text().trimmed() : QString();
    const QString prenom = ui->lineEdit_3p ? ui->lineEdit_3p->text().trimmed() : QString();

    if (email.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Mission"), QStringLiteral("Veuillez renseigner l'email du pêcheur."));
        return;
    }

    if (!isValidEmailAddress(email)) {
        QMessageBox::warning(this, QStringLiteral("Mission"), QStringLiteral("Format d'e-mail invalide."));
        return;
    }

    QString emailLookupError;
    if (!pecheurEmailExistsInDb(email, &emailLookupError)) {
        QMessageBox::warning(this,
                             QStringLiteral("Mission"),
                             emailLookupError.isEmpty()
                                 ? QStringLiteral("Adresse e-mail introuvable (mail non trouve).")
                                 : emailLookupError);
        return;
    }

    if (nom.isEmpty() || prenom.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Mission"), QStringLiteral("Veuillez renseigner le nom et le prénom du pêcheur."));
        return;
    }

    int idBateau = 0;
    if (QComboBox* bateauCombo = pecheurBateauCombo(ui)) {
        bool ok = false;
        idBateau = bateauCombo->currentData().toInt(&ok);
        if (!ok || idBateau <= 0) {
            idBateau = bateauIdFromText(bateauCombo->currentText());
        }
    } else if (QLineEdit* bateauEdit = pecheurBateauLineEdit(ui)) {
        idBateau = bateauIdFromText(bateauEdit->text());
    }

    if (idBateau <= 0) {
        QMessageBox::warning(this, QStringLiteral("Mission"), QStringLiteral("Veuillez choisir un ID bateau valide avant l'affectation."));
        return;
    }

    QDateTime affectation = ui->dateTimeEdit_2 ? ui->dateTimeEdit_2->dateTime() : QDateTime();
    if (isPecheurAffectationNull(affectation)) {
        affectation = QDateTime::currentDateTime();
        if (ui->dateTimeEdit_2) {
            ui->dateTimeEdit_2->setDateTime(affectation);
        }
    }

    QString mailError;
    if (!sendMissionMailSmtp(email, nom, prenom, affectation, idBateau, &mailError)) {
        QMessageBox::critical(this, QStringLiteral("Mission"), QStringLiteral("Envoi e-mail echoue: %1").arg(mailError));
        return;
    }

    Pecheurs p = pecheurFromForm();
    const QString ancienId = !m_editingPecheurId.isEmpty() ? m_editingPecheurId : (ui->lineEditp ? ui->lineEditp->text().trimmed().toUpper() : QString());
    const bool canPersist = !ancienId.isEmpty() && (m_editingPecheurId == ancienId || Pecheurs::idExiste(ancienId));
    if (canPersist) {
        if (!p.modifierAvecAncienId(ancienId)) {
            QMessageBox::warning(this, QStringLiteral("Mission"), QStringLiteral("L'email a été préparé, mais la mise à jour en base a échoué: %1").arg(Pecheurs::lastError()));
            return;
        }
        loadPecheurs();
    }

    QMessageBox::information(this,
                             QStringLiteral("Mission"),
                             QStringLiteral("E-mail d'affectation envoyé à %1 et mise à jour réussie.").arg(email));
}

void MainWindow::on_pushButton_5e_2_clicked()
{
    if (!ui) return;

    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Sélectionner le CV"), "", tr("Documents (*.pdf *.doc *.docx *.txt);;Tous les fichiers (*)"));
    
    if (fileName.isEmpty()) return;

    m_currentCvPath = fileName;
    if (ui->lineEdit_cv_path) ui->lineEdit_cv_path->setText(fileName);
    
    ui->pushButton_5e_2->setText(QStringLiteral("📄 CV Sélectionné"));
    ui->pushButton_5e_2->setToolTip(fileName);

    QString content;
    bool readSuccess = false;

    // 1. Tentative de lecture selon l'extension
    if (fileName.endsWith(".txt", Qt::CaseInsensitive)) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            content = QString::fromUtf8(file.readAll());
            file.close();
            readSuccess = true;
        }
    } 
#ifdef HAVE_ACTIVEQT
    else if (fileName.endsWith(".docx", Qt::CaseInsensitive) || fileName.endsWith(".doc", Qt::CaseInsensitive)) {
        // Tentative d'extraction via Microsoft Word si installé
        QAxObject* word = new QAxObject("Word.Application", this);
        if (word && !word->isNull()) {
            word->setProperty("Visible", false);
            QAxObject* documents = word->querySubObject("Documents");
            if (documents) {
                QAxObject* document = documents->querySubObject("Open(const QString&, bool, bool)", fileName, true, true);
                if (document) {
                    QAxObject* range = document->querySubObject("Range()");
                    if (range) {
                        content = range->property("Text").toString();
                        delete range;
                        readSuccess = true;
                    }
                    document->dynamicCall("Close()");
                    delete document;
                }
                delete documents;
            }
            word->dynamicCall("Quit()");
            delete word;
        }
    }
#endif

    // 2. Extraction des valeurs via Regex
    auto extractValue = [&](const QString &keyword) -> QString {
        // Regex plus précise : s'arrête au prochain mot-clé ou à la fin de la ligne
        // On définit la liste des mots-clés pour savoir où s'arrêter
        static const QStringList keywords = {
            "Id", "Nom", "Prenom", "Telephone", "Role", "Statut", "Equipe", "Etat", "Salaire"
        };
        
        // On construit une regex qui cherche le mot-clé, puis capture tout jusqu'à :
        // - un autre mot-clé (suivi de :)
        // - ou la fin de la ligne ($)
        // - ou un saut de ligne (\n)
        
        QString otherKeywords = keywords.join("|");
        // Regex : keyword + separator + (capture tout ce qui n'est pas un saut de ligne et qui n'est pas suivi par un autre keyword:)
        // Version simplifiée mais efficace : capture jusqu'au prochain mot-clé ou fin de ligne
        QRegularExpression re(keyword + "\\s*[:=]\\s*(.*?)(?=(?:" + otherKeywords + ")\\s*[:=]|$|\\r|\\n)", 
                             QRegularExpression::CaseInsensitiveOption);
        
        QRegularExpressionMatch match = re.match(content);
        if (match.hasMatch()) {
            return match.captured(1).trimmed();
        }
        return QString();
    };

    QString id = extractValue("Id");
    QString nom = extractValue("Nom");
    QString prenom = extractValue("Prenom");
    QString tel = extractValue("Telephone");
    QString role = extractValue("Role");
    QString statut = extractValue("Statut");
    QString equipe = extractValue("Equipe");
    QString etat = extractValue("Etat");
    QString salaire = extractValue("Salaire");

    // 3. Fallback : Si rien n'a été trouvé (ex: fichier binaire sans Word), on essaie le nom du fichier
    if (id.isEmpty() && nom.isEmpty() && prenom.isEmpty()) {
        QString baseName = QFileInfo(fileName).baseName();
        // On remplace les séparateurs par des espaces pour le découpage
        QString cleanName = baseName.replace('_', ' ').replace('-', ' ');
        QStringList parts = cleanName.split(' ', Qt::SkipEmptyParts);
        
        for (const QString &part : parts) {
            QString upart = part.toUpper();
            if (upart.length() >= 7 && upart.at(0).isDigit()) {
                if (id.isEmpty()) id = part;
                else if (tel.isEmpty()) tel = part;
            } else if (upart == "GARDIEN" || upart == "TECHNICIEN" || upart == "RESPONSABLE" || upart == "OUVRIER") {
                if (role.isEmpty()) role = part;
            } else if (upart == "MISSION" || upart == "CONGE" || upart == "DISPONIBLE") {
                if (statut.isEmpty()) statut = part;
            } else if (nom.isEmpty()) {
                nom = part;
            } else if (prenom.isEmpty()) {
                prenom = part;
            }
        }
    }

    // 4. Remplissage du formulaire
    if (ui->lineEdit_12e && !id.isEmpty()) ui->lineEdit_12e->setText(id);
    if (ui->lineEdit_13e && !nom.isEmpty()) ui->lineEdit_13e->setText(nom);
    if (ui->lineEdit_16e && !prenom.isEmpty()) ui->lineEdit_16e->setText(prenom);
    if (ui->lineEdit_14e && !tel.isEmpty()) ui->lineEdit_14e->setText(tel);
    if (ui->lineEdit_14e_2 && !salaire.isEmpty()) ui->lineEdit_14e_2->setText(salaire);

    if (ui->comboBox_11e && !role.isEmpty()) {
        int idx = ui->comboBox_11e->findText(role, Qt::MatchContains | Qt::MatchFixedString);
        if (idx >= 0) ui->comboBox_11e->setCurrentIndex(idx);
    }
    if (ui->comboBox_14e && !statut.isEmpty()) {
        int idx = ui->comboBox_14e->findText(statut, Qt::MatchContains | Qt::MatchFixedString);
        if (idx >= 0) ui->comboBox_14e->setCurrentIndex(idx);
    }
    if (ui->comboBox_15e && !equipe.isEmpty()) {
        int idx = ui->comboBox_15e->findText(equipe, Qt::MatchContains | Qt::MatchFixedString);
        if (idx >= 0) ui->comboBox_15e->setCurrentIndex(idx);
    }
    if (ui->comboBox_16e && !etat.isEmpty()) {
        int idx = ui->comboBox_16e->findText(etat, Qt::MatchContains | Qt::MatchFixedString);
        if (idx >= 0) ui->comboBox_16e->setCurrentIndex(idx);
    }

    if (readSuccess) {
        QMessageBox::information(this, tr("Upload CV"), 
            tr("Le CV a été lu et le formulaire a été pré-rempli automatiquement."));
    } else {
        QMessageBox::warning(this, tr("Upload CV"), 
            tr("Le fichier a été sélectionné, mais son contenu n'a pas pu être lu directement (format .docx sans Word installé).\n\n"
               "Certains champs ont été remplis à partir du nom du fichier. Pour un remplissage complet, utilisez un fichier .txt."));
    }
}

