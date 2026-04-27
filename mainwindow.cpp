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
#include <algorithm>
#include <functional>
#include <initializer_list>
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

static bool isLettersOnlyName(const QString& text)
{
    // Autorise les lettres (Unicode), les espaces et les traits d'union
    static const QRegularExpression rx(QStringLiteral("^[\\p{L}\\s-]+$"));
    return rx.match(text.trimmed()).hasMatch();
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

static int employeRoleCodeForIdUi(const QString& roleRaw)
{
    const QString role = canonicalEmployeRole(roleRaw);
    if (role == QStringLiteral("Gardien")) return 3;
    if (role == QStringLiteral("Technicien")) return 4;
    if (role == QStringLiteral("Responsable")) return 5;
    if (role == QStringLiteral("Ouvrier")) return 6;
    return 0;
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

    // IDs: champs modifiables selon demande utilisateur
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
    if (ui->frame_3b) ui->frame_3b->hide();        // Frame QR cach├⌐
    ui->frameb->show(); // Frame Connexion visible si tu veux
        
        
        
        
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

    int countGardien = 0;
    int countTechnicien = 0;
    int countResponsable = 0;
    int countOuvrier = 0;

    table->clearContents();
    table->setRowCount(0);
    table->setColumnCount(dataCols + 1);

    // En-tetes
    for (int c = 0; c < dataCols; ++c) {
        auto *item = new QTableWidgetItem(model->headerData(c, Qt::Horizontal).toString());
        table->setHorizontalHeaderItem(c, item);
    }
    table->setHorizontalHeaderItem(dataCols, new QTableWidgetItem(QStringLiteral("Actions")));

    // Donnees filtrees
    int visibleRow = 0;
    for (int r = 0; r < rowCount; ++r) {
        const QString nom = (colNom >= 0) ? model->data(model->index(r, colNom)).toString().trimmed() : QString();
        const QString prenom = (colPrenom >= 0) ? model->data(model->index(r, colPrenom)).toString().trimmed() : QString();
        const QString role = (colRole >= 0) ? model->data(model->index(r, colRole)).toString().trimmed() : QString();
        const QString etat = (colEtat >= 0) ? model->data(model->index(r, colEtat)).toString().trimmed() : QString();
        const QString statut = (colStatut >= 0) ? model->data(model->index(r, colStatut)).toString().trimmed() : QString();

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

    const QString idText = (colId >= 0 && table->item(row, colId)) ? table->item(row, colId)->text().trimmed() : QString();
    const QString nom = (colNom >= 0 && table->item(row, colNom)) ? table->item(row, colNom)->text().trimmed() : QString();
    const QString prenom = (colPrenom >= 0 && table->item(row, colPrenom)) ? table->item(row, colPrenom)->text().trimmed() : QString();
    const QString telephone = (colTelephone >= 0 && table->item(row, colTelephone)) ? table->item(row, colTelephone)->text().trimmed() : QString();
    const QString role = (colRole >= 0 && table->item(row, colRole)) ? table->item(row, colRole)->text().trimmed() : QString();
    const QString equipe = (colEquipe >= 0 && table->item(row, colEquipe)) ? table->item(row, colEquipe)->text().trimmed() : QString();
    const QString etat = (colEtat >= 0 && table->item(row, colEtat)) ? table->item(row, colEtat)->text().trimmed() : QString();
    const QString statut = (colStatut >= 0 && table->item(row, colStatut)) ? table->item(row, colStatut)->text().trimmed() : QString();
    const QString salaire = (colSalaire >= 0 && table->item(row, colSalaire)) ? table->item(row, colSalaire)->text().trimmed() : QString();

    if (ui->lineEdit_12e) ui->lineEdit_12e->setText(idText);
    if (ui->lineEdit_13e) ui->lineEdit_13e->setText(nom);
    if (ui->lineEdit_16e) ui->lineEdit_16e->setText(prenom);
    if (ui->lineEdit_14e) ui->lineEdit_14e->setText(telephone);
    if (ui->lineEdit_14e_2) ui->lineEdit_14e_2->setText(salaire);

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
        table->setItem(row, 5, new QTableWidgetItem(record.hasTemperature
                                 ? QString::number(record.temperatureC, 'f', 1)
                                 : QString()));
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

    const auto showPortBusyHint = [this](const QString& error) {
        if (!statusBar()) return;

        const QString e = error.toLower();
        const bool isBusy = e.contains(QStringLiteral("access"))
            || e.contains(QStringLiteral("busy"))
            || e.contains(QStringLiteral("denied"))
            || e.contains(QStringLiteral("in use"));

        if (isBusy) {
            statusBar()->showMessage(
                QStringLiteral("Port serie occupe. Fermez Arduino IDE/Serial Monitor, puis reconnectez."),
                9000);
        }
    };

    const auto tryConnectArduino = [this, savedPort, showPortBusyHint](const QString& preferredPort,
                                                                        bool userInitiated) -> bool {
        if (!m_arduino) return false;
        if (m_arduino->isConnected()) return true;
        if (!m_arduino->serialPortAvailable()) {
            if (statusBar() && userInitiated) {
                statusBar()->showMessage(QStringLiteral("Qt SerialPort indisponible. Installez le module Qt Serial Port."), 7000);
            }
            return false;
        }

        refreshArduinoPortList();
        const QStringList ports = m_arduino->availablePortNames();
        if (ports.isEmpty()) {
            if (statusBar() && userInitiated) {
                statusBar()->showMessage(QStringLiteral("Arduino: no serial ports found"), 5000);
            }
            updateArduinoUiState();
            return false;
        }

        QString chosen;
        const QString comboPort = m_arduinoPortCombo ? m_arduinoPortCombo->currentText().trimmed() : QString();
        if (!preferredPort.trimmed().isEmpty() && ports.contains(preferredPort.trimmed())) {
            chosen = preferredPort.trimmed();
        } else if (!comboPort.isEmpty() && comboPort != QStringLiteral("(no ports)") && ports.contains(comboPort)) {
            chosen = comboPort;
        } else if (!savedPort.isEmpty() && ports.contains(savedPort)) {
            chosen = savedPort;
        } else if (ports.contains(QStringLiteral("COM3"))) {
            chosen = QStringLiteral("COM3");
        } else {
            chosen = ports.first();
        }

        QString error;
        if (!m_arduino->connectToPort(chosen, 9600, &error)) {
            if (statusBar() && userInitiated) {
                statusBar()->showMessage(QStringLiteral("Arduino connect failed: %1").arg(error), 7000);
            }
            showPortBusyHint(error);
            updateArduinoUiState();
            return false;
        }

        if (statusBar()) {
            statusBar()->showMessage(QStringLiteral("Arduino connected on %1").arg(chosen), 3000);
        }
        return true;
    };

    connect(m_arduinoConnectButton, &QPushButton::clicked, this, [this, tryConnectArduino]() {
        if (!m_arduino) return;

        if (m_arduino->isConnected()) {
            m_arduino->disconnectFromPort();
            return;
        }

        const QString portName = m_arduinoPortCombo ? m_arduinoPortCombo->currentText().trimmed() : QString();
        tryConnectArduino(portName, true);
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

        // Ensure Arduino and Qt start from a clean protocol state.
        QString writeError;
        m_arduino->writeLine(QStringLiteral("RESET"), &writeError);
    });

    connect(m_arduino, &ArduinoSerial::disconnected, this, [this]() {
        if (m_arduinoStatusLabel) m_arduinoStatusLabel->setText(QStringLiteral("Arduino: disconnected"));
        updateArduinoUiState();
        updateArduinoTemperatureDialogConnectionState();
    });

    connect(m_arduino, &ArduinoSerial::lineReceived, this, [this](const QString& line) {
        if (statusBar()) {
            const QString compact = line.length() > 64 ? line.left(64) + QStringLiteral("...") : line;
            statusBar()->showMessage(QStringLiteral("Arduino RX: %1").arg(compact), 2000);
        }
        handleArduinoTemperatureLine(line);
        handleArduinoBoatEventLine(line);
    });

    connect(m_arduino, &ArduinoSerial::errorOccurred, this, [this](const QString& message) {
        if (statusBar()) statusBar()->showMessage(QStringLiteral("Arduino error: %1").arg(message), 8000);
        updateArduinoUiState();
        updateArduinoTemperatureDialogConnectionState();
    });

    updateArduinoUiState();

    // Auto-connect shortly after startup (user can still disconnect manually).
    QTimer::singleShot(900, this, [tryConnectArduino]() {
        tryConnectArduino(QString(), false);
    });

    // Keep trying in background if cable was re-plugged or port changed.
    auto* reconnectTimer = new QTimer(this);
    reconnectTimer->setInterval(2500);
    connect(reconnectTimer, &QTimer::timeout, this, [this, tryConnectArduino]() {
        if (!m_arduino || m_arduino->isConnected()) return;
        tryConnectArduino(QString(), false);
    });
    reconnectTimer->start();
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

// Forward declarations (used before definitions below)
static QString tempStatusCss(double temperatureC);

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

    auto makeCard = [frame](const QString& cardTitle, const QString& initialValue, const QString& valueCss, QLabel** valueOut) {
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

        auto* v = new QLabel(initialValue, card);
        QFont vf = v->font();
        vf.setBold(true);
        vf.setPointSize(qMax(14, vf.pointSize() + 10));
        v->setFont(vf);
        v->setStyleSheet(valueCss);
        layout->addWidget(v);

        if (valueOut) *valueOut = v;
        return card;
    };

    QLabel* currentValueLabel = nullptr;
    cardsRow->addWidget(makeCard(QStringLiteral("Temp\u00E9rature actuelle"), QStringLiteral("-- \u00B0C"), QString(), &currentValueLabel));
    cardsRow->addWidget(makeCard(QStringLiteral("Seuil danger"), QStringLiteral("29.0 \u00B0C"), QStringLiteral("color: #f39c12;"), nullptr));
    cardsRow->addWidget(makeCard(QStringLiteral("Seuil critique"), QStringLiteral("32.0 \u00B0C"), QStringLiteral("color: #e74c3c;"), nullptr));
    root->addLayout(cardsRow);

    m_arduinoTempValueLabel = currentValueLabel;

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

static bool isArduinoBoatDetectedMessage(const QString& line)
{
    QString normalized = line.trimmed().toLower();
    normalized.replace(QStringLiteral("é"), QStringLiteral("e"));
    normalized.replace(QStringLiteral("è"), QStringLiteral("e"));
    normalized.replace(QStringLiteral("ê"), QStringLiteral("e"));
    normalized.replace(QStringLiteral("à"), QStringLiteral("a"));
    normalized.replace(QStringLiteral("ù"), QStringLiteral("u"));
    normalized.replace(QStringLiteral("ï"), QStringLiteral("i"));
    normalized.replace(QStringLiteral("î"), QStringLiteral("i"));
    normalized.replace(QStringLiteral("_"), QStringLiteral(" "));
    normalized.replace(QStringLiteral("-"), QStringLiteral(" "));
    if (normalized.isEmpty()) return false;

    return normalized == QStringLiteral("arrivee")
        || normalized.contains(QStringLiteral("arrivee"))
        || normalized.contains(QStringLiteral("bateau detecte"))
        || normalized.contains(QStringLiteral("bateau gare"))
        || normalized.contains(QStringLiteral("bateau garre"))
        || normalized.contains(QStringLiteral("bateau amarre"))
        || normalized.contains(QStringLiteral("boat detected"))
        || normalized.contains(QStringLiteral("boat_detected"))
        || normalized.contains(QStringLiteral("presence=1"))
        || normalized.contains(QStringLiteral("ir:1"));
}

static bool isArduinoBoatDepartMessage(const QString& line)
{
    QString normalized = line.trimmed().toLower();
    normalized.replace(QStringLiteral("é"), QStringLiteral("e"));
    normalized.replace(QStringLiteral("è"), QStringLiteral("e"));
    normalized.replace(QStringLiteral("ê"), QStringLiteral("e"));
    normalized.replace(QStringLiteral("à"), QStringLiteral("a"));
    normalized.replace(QStringLiteral("ù"), QStringLiteral("u"));
    normalized.replace(QStringLiteral("ï"), QStringLiteral("i"));
    normalized.replace(QStringLiteral("î"), QStringLiteral("i"));
    normalized.replace(QStringLiteral("_"), QStringLiteral(" "));
    normalized.replace(QStringLiteral("-"), QStringLiteral(" "));
    if (normalized.isEmpty()) return false;

    return normalized == QStringLiteral("depart")
        || normalized.contains(QStringLiteral("depart"))
        || normalized.contains(QStringLiteral("bateau depart"))
        || normalized.contains(QStringLiteral("bateau parti"))
        || normalized.contains(QStringLiteral("boat left"))
        || normalized.contains(QStringLiteral("boat_departed"))
        || normalized.contains(QStringLiteral("presence=0"))
        || normalized.contains(QStringLiteral("ir:0"));
}

static QString tempStatusText(double temperatureC)
{
    if (temperatureC < 29.0) return QStringLiteral("Normal");
    if (temperatureC <= 32.0) return QStringLiteral("Danger");
    return QStringLiteral("Critique");
}

static QString tempStatusButtonText(double temperatureC)
{
    if (temperatureC < 29.0) return QStringLiteral("Temperature normale");
    if (temperatureC <= 32.0) return QStringLiteral("Temp danger");
    return QStringLiteral("Temp critique");
}

static QString tempStatusButtonCss(double temperatureC)
{
    const QString common = QStringLiteral(
        "color: white;"
        "font-weight: 700;"
        "border-radius: 8px;"
        "padding: 8px 16px;"
    );

    if (temperatureC < 29.0) {
        return QStringLiteral("QPushButton { background-color: #27ae60; border: 2px solid #229954;") + common + QStringLiteral(" }");
    }
    if (temperatureC <= 32.0) {
        return QStringLiteral("QPushButton { background-color: #f39c12; border: 2px solid #d68910;") + common + QStringLiteral(" }");
    }
    return QStringLiteral("QPushButton { background-color: #e74c3c; border: 2px solid #c0392b;") + common + QStringLiteral(" }");
}

static QString tempStatusCss(double temperatureC)
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

    if (temperatureC < 29.0) {
        return QStringLiteral("background-color: #27ae60; border: 2px solid #229954;") + common;
    }
    if (temperatureC <= 32.0) {
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

void MainWindow::handleArduinoBoatEventLine(const QString& line)
{
    if (!isArduinoBoatDetectedMessage(line) && !isArduinoBoatDepartMessage(line)) {
        return;
    }

    if (isArduinoBoatDetectedMessage(line)) {
        int quaiId = -1;
        QString error;
        if (!assignFreeQuaiToDetectedBoat(&quaiId, &error)) {
            if (statusBar()) {
                statusBar()->showMessage(
                    error.isEmpty()
                        ? QStringLiteral("Bateau detecte: aucun quai libre disponible.")
                        : QStringLiteral("Bateau detecte: %1").arg(error),
                    7000);
            }

            if (m_arduino && m_arduino->isConnected()) {
                QString writeError;
                m_arduino->writeLine(QStringLiteral("COMPLET"), &writeError);
            }
            return;
        }

        if (ui && ui->lineEdit_4) {
            ui->lineEdit_4->setText(QString::number(quaiId));
        }

        if (statusBar()) {
            statusBar()->showMessage(QStringLiteral("Bateau detecte -> idQuai:%1").arg(quaiId), 7000);
        }

        if (m_arduino && m_arduino->isConnected()) {
            QString writeError;
            if (!m_arduino->writeLine(QStringLiteral("QUAI:%1").arg(quaiId), &writeError) && statusBar()) {
                statusBar()->showMessage(QStringLiteral("Erreur envoi afficheur: %1").arg(writeError), 7000);
            }
        }

        refreshQuaiTable();
        return;
    }

    if (isArduinoBoatDepartMessage(line)) {
        QString error;
        if (!releaseAssignedQuai(&error)) {
            if (statusBar()) {
                statusBar()->showMessage(
                    error.isEmpty()
                        ? QStringLiteral("Depart bateau: impossible de liberer le quai.")
                        : QStringLiteral("Depart bateau: %1").arg(error),
                    7000);
            }
            return;
        }

        if (statusBar()) {
            statusBar()->showMessage(QStringLiteral("Bateau parti -> quai remis Libre."), 7000);
        }

        refreshQuaiTable();
    }
}

bool MainWindow::assignFreeQuaiToDetectedBoat(int* quaiIdOut, QString* errorOut)
{
    if (quaiIdOut) *quaiIdOut = -1;

    Connection* conn = Connection::getInstance();
    if (!conn || !conn->ensureOpen()) {
        if (errorOut) {
            *errorOut = conn
                ? QStringLiteral("Connexion DB echouee: %1").arg(conn->lastErrorText())
                : QStringLiteral("Instance connexion DB indisponible.");
        }
        return false;
    }

    QSqlDatabase db = conn->getDatabase();
    if (!db.isValid() || !db.isOpen()) {
        if (errorOut) *errorOut = QStringLiteral("Base de donnees non ouverte.");
        return false;
    }

    if (m_assignedArduinoQuaiId > 0) {
        QSqlQuery sync(db);
        sync.prepare(QStringLiteral("UPDATE QUAIS SET STATUT = :newStatut WHERE ID_QUAI = :id"));
        sync.bindValue(QStringLiteral(":newStatut"), QStringLiteral("Occupe"));
        sync.bindValue(QStringLiteral(":id"), m_assignedArduinoQuaiId);

        if (!sync.exec()) {
            if (errorOut) *errorOut = sync.lastError().text();
            return false;
        }

        if (sync.numRowsAffected() <= 0) {
            if (errorOut) *errorOut = QStringLiteral("Quai assigne introuvable en base.");
            return false;
        }

        if (quaiIdOut) *quaiIdOut = m_assignedArduinoQuaiId;
        return true;
    }

    if (!db.transaction()) {
        if (errorOut) *errorOut = QStringLiteral("Impossible de demarrer la transaction QUAIS.");
        return false;
    }

    QSqlQuery sel(db);
    if (!sel.exec(QStringLiteral(
            "SELECT ID_QUAI FROM ("
            " SELECT ID_QUAI FROM QUAIS"
            " WHERE UPPER(TRIM(STATUT)) = 'LIBRE'"
            " ORDER BY ID_QUAI"
            ") WHERE ROWNUM = 1"))) {
        db.rollback();
        if (errorOut) *errorOut = sel.lastError().text();
        return false;
    }

    if (!sel.next()) {
        db.rollback();
        if (errorOut) *errorOut = QStringLiteral("Aucun quai avec statut Libre.");
        return false;
    }

    const int quaiId = sel.value(0).toInt();
    if (quaiId <= 0) {
        db.rollback();
        if (errorOut) *errorOut = QStringLiteral("ID quai libre invalide.");
        return false;
    }

    QSqlQuery upd(db);
    upd.prepare(QStringLiteral(
        "UPDATE QUAIS SET STATUT = :newStatut "
        "WHERE ID_QUAI = :id AND UPPER(TRIM(STATUT)) = 'LIBRE'"));
    upd.bindValue(QStringLiteral(":newStatut"), QStringLiteral("Occupe"));
    upd.bindValue(QStringLiteral(":id"), quaiId);

    if (!upd.exec()) {
        db.rollback();
        if (errorOut) *errorOut = upd.lastError().text();
        return false;
    }

    if (upd.numRowsAffected() <= 0) {
        db.rollback();
        if (errorOut) *errorOut = QStringLiteral("Le quai libre choisi vient d'etre occupe ailleurs.");
        return false;
    }

    if (!db.commit()) {
        db.rollback();
        if (errorOut) *errorOut = QStringLiteral("Commit impossible lors de l'occupation du quai.");
        return false;
    }

    m_assignedArduinoQuaiId = quaiId;
    if (quaiIdOut) *quaiIdOut = quaiId;
    return true;
}

bool MainWindow::releaseAssignedQuai(QString* errorOut)
{
    if (m_assignedArduinoQuaiId <= 0) {
        return true;
    }

    Connection* conn = Connection::getInstance();
    if (!conn || !conn->ensureOpen()) {
        if (errorOut) {
            *errorOut = conn
                ? QStringLiteral("Connexion DB echouee: %1").arg(conn->lastErrorText())
                : QStringLiteral("Instance connexion DB indisponible.");
        }
        return false;
    }

    QSqlDatabase db = conn->getDatabase();
    if (!db.isValid() || !db.isOpen()) {
        if (errorOut) *errorOut = QStringLiteral("Base de donnees non ouverte.");
        return false;
    }

    QSqlQuery upd(db);
    upd.prepare(QStringLiteral("UPDATE QUAIS SET STATUT = :newStatut WHERE ID_QUAI = :id"));
    upd.bindValue(QStringLiteral(":newStatut"), QStringLiteral("Libre"));
    upd.bindValue(QStringLiteral(":id"), m_assignedArduinoQuaiId);

    if (!upd.exec()) {
        if (errorOut) *errorOut = upd.lastError().text();
        return false;
    }

    m_assignedArduinoQuaiId = -1;
    return true;
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

    constexpr int kIdCol = 0;
    constexpr int kTempCol = 5;

    int targetRow = -1;

    // If a capture is being edited, update that row specifically.
    if (!m_editingCaptureId.isEmpty()) {
        for (int r = 0; r < table->rowCount(); ++r) {
            const QTableWidgetItem* idItem = table->item(r, kIdCol);
            if (!idItem) continue;
            if (idItem->text().trimmed() == m_editingCaptureId) {
                targetRow = r;
                break;
            }
        }

        // Keep the edit buffer synced too.
        m_editingCaptureHasTemperature = true;
        m_editingCaptureTemperatureC = temperatureC;
    }

    // Otherwise, update the most recent row (row 0, sorted DESC by date).
    if (targetRow < 0) {
        const int current = table->currentRow();
        targetRow = (current >= 0) ? current : 0;
    }

    QTableWidgetItem* tempItem = table->item(targetRow, kTempCol);
    if (!tempItem) {
        tempItem = new QTableWidgetItem();
        table->setItem(targetRow, kTempCol, tempItem);
    }

    tempItem->setText(QString::number(temperatureC, 'f', 1));
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

        const QString valueColor = (temperatureC < 29.0)
            ? QStringLiteral("color: #27ae60;")
            : (temperatureC <= 32.0)
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

    Employe emp(id, nom, prenom, role, equipe, etat, statut, salaire, telephone);

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
    QString sql = "SELECT ID_BATEAU, NOM, PROPRIETAIRE, LARGEUR, TYPE, STATUT, CAPACITE, DATE_ENTREE, DATE_DERNIERE_MAINTENANCE, PROCHAINE_MAINTENANCE FROM BATEAUX";

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
        "SELECT ID_BATEAU, NOM, STATUT, DATE_ENTREE, DATE_DERNIERE_MAINTENANCE, PROCHAINE_MAINTENANCE FROM BATEAUX"));
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

    // ID quai est en lecture seule: en mode ajout, on le genere automatiquement si vide
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

    // R├⌐cup├⌐rer les donn├⌐es du formulaire
    QString zonePort = ui->comboBox_6->currentText().trimmed();
    QString zoneCouverte = ui->comboBox_8->currentText().trimmed();
    QString statutUi = ui->comboBox_7->currentText().trimmed();

    // Adapter les valeurs aux contraintes CHECK Oracle
    // Zone_Port: 'Nord','Sud','Est','Ouest' -> d├⌐j├á coh├⌐rent avec les valeurs de la combo
    // Zone_Couverte: 'Oui','Non' -> d├⌐j├á coh├⌐rent
    // Statut: 'Libre','Occupe','Maintenance' (sans accent sur Occupe)
    QString statutDb = statutUi;
    if (statutUi.compare(QStringLiteral("Occup├⌐"), Qt::CaseInsensitive) == 0 ||
        statutUi.startsWith(QStringLiteral("Occu"), Qt::CaseInsensitive)) {
        statutDb = QStringLiteral("Occupe");
    }

    const int savedQuaiId = ui->lineEdit_4->text().trimmed().toInt();

    QVariantMap donnees;
    donnees["ID_QUAI"] = savedQuaiId;
    donnees["NOM_QUAI"] = ui->lineEdit_6->text().trimmed();
    donnees["ZONE_PORT"] = zonePort;
    donnees["ZONE_COUVERTE"] = zoneCouverte;
    donnees["LONGUEUR"] = ui->doubleSpinBox_2->value();
    donnees["CAPACITE_QUAIS"] = ui->spinBox_2->value();
    donnees["STATUT"] = statutDb;

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

        // Preparer le prochain ajout: regenerer l'ID (toujours en lecture seule)
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
        {QStringLiteral("STATUT"), {QStringLiteral("STATUT"), QStringLiteral("statut")}}
    };

    const QString tableName = QStringLiteral("QUAIS");

    reloadTableWidgetFromDb(ui->tableWidgetQuai, db, tableName,
                            {QStringLiteral("ID_QUAI"), QStringLiteral("NOM_QUAI"), QStringLiteral("ZONE_PORT"),
                             QStringLiteral("ZONE_COUVERTE"), QStringLiteral("LONGUEUR"),
                             QStringLiteral("CAPACITE_QUAIS"), QStringLiteral("STATUT")},
                            syn);

    // En-têtes lisibles pour l'IHM
    if (ui->tableWidgetQuai && ui->tableWidgetQuai->columnCount() >= 8) {
        ui->tableWidgetQuai->setHorizontalHeaderItem(0, new QTableWidgetItem(QStringLiteral("ID Quai")));
        ui->tableWidgetQuai->setHorizontalHeaderItem(1, new QTableWidgetItem(QStringLiteral("Nom Quai")));
        ui->tableWidgetQuai->setHorizontalHeaderItem(2, new QTableWidgetItem(QStringLiteral("Zone Port")));
        ui->tableWidgetQuai->setHorizontalHeaderItem(3, new QTableWidgetItem(QStringLiteral("Zone Couverte")));
        ui->tableWidgetQuai->setHorizontalHeaderItem(4, new QTableWidgetItem(QStringLiteral("Longueur Max")));
        ui->tableWidgetQuai->setHorizontalHeaderItem(5, new QTableWidgetItem(QStringLiteral("Capacité Quais")));
        ui->tableWidgetQuai->setHorizontalHeaderItem(6, new QTableWidgetItem(QStringLiteral("Statut")));
        ui->tableWidgetQuai->setHorizontalHeaderItem(7, new QTableWidgetItem(QStringLiteral("Actions")));
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

    // Mettre ├á jour les statistiques d'occupation quand les quais changent
    refreshStats_2();

    // R├⌐appliquer les filtres (recherche, statut, zone, capacit├⌐)
    // apr├¿s rechargement de la table.
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

// --- Filtres de recherche / statut / zone / capacit├⌐ sur la page_3 (quais) ---

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
    const int statutCol = 6; // colonne "Statut" dans tableWidgetQuai
    const int zoneCol   = 2; // colonne "Z du port" / Zone_Port
    const int capaCol   = 5; // colonne "C maximale" / capacit├⌐ quais

    for (int row = 0; row < rowCount; ++row) {
        bool match = true;

        // 1) Filtre texte (recherche globale sur les colonnes de donn├⌐es)
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

        // 4) Filtre Capacit├⌐
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
        QMessageBox::warning(this, "Export PDF", "Le tableau est vide, rien ├á exporter.");
        return;
    }

    const QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
            + "/Quais_" + QDate::currentDate().toString("yyyy-MM-dd") + ".pdf";
    const QString filePath = QFileDialog::getSaveFileName(this, "Enregistrer le PDF", defaultPath, "PDF (*.pdf)");
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

    const QString exportStamp = QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm");
    const int pageW = writer.width();
    const int pageH = writer.height();

    const int leftMargin = 50;
    const int rightMargin = 50;
    const int topMargin = 100;
    const int bottomMargin = 80;
    const int contentW = pageW - leftMargin - rightMargin;

    QFont titleFont("Arial", 20, QFont::Bold);
    QFont dateFont("Arial", 9);
    QFont headerFont("Arial", 11, QFont::Bold);
    QFont cellFont("Arial", 9);
    QFont footerFont("Arial", 9);

    painter.setFont(titleFont);
    painter.setPen(QColor(0, 82, 155));
    painter.drawText(QRect(leftMargin, topMargin - 40, contentW, 60), Qt::AlignCenter, "AQUATEC ΓÇô Liste des Quais");

    painter.setFont(dateFont);
    painter.setPen(Qt::darkGray);
    painter.drawText(QRect(leftMargin, topMargin + 20, contentW, 30), Qt::AlignCenter,
                     QString("Export├⌐ le %1").arg(exportStamp));

    const int yStart = topMargin + 80;
    const QStringList headers = { "ID Quai", "Nom Quai", "Zone Port", "Zone Couverte", "Longueur Max", "Capacit├⌐", "Statut" };
    const QVector<double> proportions = { 0.08, 0.30, 0.12, 0.12, 0.12, 0.10, 0.16 };
    const int colCount = headers.size();

    QVector<int> colWidths(colCount, 0);
    int usedW = 0;
    for (int i = 0; i < colCount; ++i) {
        colWidths[i] = static_cast<int>(contentW * proportions[i]);
        usedW += colWidths[i];
    }
    if (colCount > 0) {
        colWidths[colCount - 1] += (contentW - usedW);
    }

    const int headerHeight = 48;
    const int rowHeight = 40;
    const int footerY = pageH - bottomMargin + 10;

    auto drawHeader = [&](int yHeader) {
        painter.setFont(headerFont);
        QLinearGradient headerGrad(0, yHeader, 0, yHeader + headerHeight);
        headerGrad.setColorAt(0.0, QColor("#0b5ea8"));
        headerGrad.setColorAt(1.0, QColor("#2e86c1"));

        int x = leftMargin;
        for (int c = 0; c < colCount; ++c) {
            QRect cellRect(x, yHeader, colWidths[c], headerHeight);
            painter.fillRect(cellRect, headerGrad);
            painter.setPen(Qt::white);
            painter.drawText(cellRect.adjusted(12, 0, -12, 0), Qt::AlignVCenter | Qt::AlignLeft, headers[c]);
            painter.setPen(QPen(QColor(200, 210, 220), 1));
            painter.drawRect(cellRect);
            x += colWidths[c];
        }
    };

    QVector<int> visibleRows;
    visibleRows.reserve(table->rowCount());
    for (int r = 0; r < table->rowCount(); ++r) {
        if (!table->isRowHidden(r)) {
            visibleRows.push_back(r);
        }
    }

    int y = yStart;
    drawHeader(y);
    y += headerHeight;

    painter.setFont(cellFont);
    for (int visibleIndex = 0; visibleIndex < visibleRows.size(); ++visibleIndex) {
        const int r = visibleRows[visibleIndex];

        if (y + rowHeight > pageH - bottomMargin - 30) {
            painter.setPen(QColor("#8a8a8a"));
            painter.setFont(footerFont);
            painter.drawText(QRect(leftMargin, footerY, contentW, 20), Qt::AlignCenter,
                             QString("AQUATEC - Rapport g├⌐n├⌐r├⌐ le %1").arg(exportStamp));

            writer.newPage();
            y = topMargin;
            drawHeader(y);
            y += headerHeight;
            painter.setFont(cellFont);
        }

        const QColor bg = (visibleIndex % 2 == 0) ? QColor(250, 250, 252) : QColor(245, 251, 255);
        int x = leftMargin;
        for (int c = 0; c < colCount; ++c) {
            QRect cellRect(x, y, colWidths[c], rowHeight);
            painter.fillRect(cellRect, bg);
            painter.setPen(QPen(QColor(220, 225, 230), 1));
            painter.drawRect(cellRect);

            const QTableWidgetItem* item = table->item(r, c);
            const QString text = item ? item->text().simplified() : QString();
            painter.setPen(Qt::black);
            painter.drawText(cellRect.adjusted(12, 6, -12, -6), Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap, text);
            x += colWidths[c];
        }
        y += rowHeight;
    }

    painter.setPen(Qt::darkGray);
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    painter.drawText(QRect(leftMargin, qMin(y + 8, footerY - 24), contentW, 22), Qt::AlignCenter,
                     QString("Total: %1 quais").arg(visibleRows.size()));

    painter.setPen(QColor("#8a8a8a"));
    painter.setFont(footerFont);
    painter.drawText(QRect(leftMargin, footerY, contentW, 20), Qt::AlignCenter,
                     QString("AQUATEC - Rapport g├⌐n├⌐r├⌐ le %1").arg(exportStamp));

    painter.end();

    QMessageBox::information(this, "Export PDF", QString("PDF export├⌐ avec succ├¿s !\n%1").arg(filePath));
}

void MainWindow::on_btnExportStatsPDF_2_clicked()
{
    if (!ui || !ui->tableWidgetQuai) {
        return;
    }

    QTableWidget* table = ui->tableWidgetQuai;
    if (table->rowCount() == 0) {
        QMessageBox::warning(this, "Export PDF", "Le tableau est vide, rien ├á exporter.");
        return;
    }

    const QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
            + "/Statistiques_Quais_" + QDate::currentDate().toString("yyyy-MM-dd") + ".pdf";
    const QString filePath = QFileDialog::getSaveFileName(this, "Enregistrer le PDF", defaultPath, "PDF (*.pdf)");
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

    const QString exportStamp = QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm");
    const int pageW = writer.width();
    const int pageH = writer.height();

    const int leftMargin = 50;
    const int rightMargin = 50;
    const int topMargin = 100;
    const int bottomMargin = 80;
    const int contentW = pageW - leftMargin - rightMargin;
    const int footerY = pageH - bottomMargin + 10;

    QVector<int> visibleRows;
    visibleRows.reserve(table->rowCount());
    for (int r = 0; r < table->rowCount(); ++r) {
        if (!table->isRowHidden(r)) visibleRows.push_back(r);
    }

    QMap<QString, int> zoneCounts;
    QMap<QString, int> statutCounts;
    for (int r : visibleRows) {
        QString zone = table->item(r, 2) ? table->item(r, 2)->text().trimmed() : QString();
        QString statut = table->item(r, 6) ? table->item(r, 6)->text().trimmed() : QString();
        if (zone.isEmpty()) zone = QStringLiteral("Non d├⌐fini");
        if (statut.isEmpty()) statut = QStringLiteral("Non d├⌐fini");
        zoneCounts[zone] += 1;
        statutCounts[statut] += 1;
    }

    const int totalQuais = visibleRows.size();

    int statsY = topMargin - 10;
    painter.setPen(QColor(0, 82, 155));
    painter.setFont(QFont("Arial", 18, QFont::Bold));
    painter.drawText(QRect(leftMargin, statsY, contentW, 44), Qt::AlignCenter,
                     QString::fromUtf8("Statistiques des Quais"));
    statsY += 50;

    painter.setPen(Qt::darkGray);
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(QRect(leftMargin, statsY, contentW, 22), Qt::AlignCenter,
                     QString("Total: %1 quais").arg(totalQuais));
    statsY += 34;

    painter.setPen(QColor(0, 82, 155));
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(QRect(leftMargin, statsY, contentW, 28), Qt::AlignCenter,
                     QString::fromUtf8("statistique par zone"));
    statsY += 32;

    const int pieSize = 180;
    const int legendW = 420;
    const int gap = 44;
    const int blockW = pieSize + gap + legendW;
    const int blockX = leftMargin + qMax(0, (contentW - blockW) / 2);
    const int pieX = blockX;
    const int pieY = statsY;
    const QRect pieRect(pieX, pieY, pieSize, pieSize);

    const QVector<QColor> zoneColors = {
        QColor("#3498db"), QColor("#2ecc71"), QColor("#e67e22"), QColor("#9b59b6"), QColor("#4A90D9")
    };

    painter.setRenderHint(QPainter::Antialiasing, true);
    if (totalQuais > 0) {
        int startAngle = 90 * 16;
        int colorIndex = 0;
        for (auto it = zoneCounts.constBegin(); it != zoneCounts.constEnd(); ++it) {
            const int cnt = it.value();
            if (cnt <= 0) continue;

            const int span = -qRound((static_cast<double>(cnt) / static_cast<double>(totalQuais)) * 360.0 * 16.0);
            painter.setBrush(zoneColors[colorIndex % zoneColors.size()]);
            painter.setPen(Qt::white);
            painter.drawPie(pieRect, startAngle, span);
            startAngle += span;
            ++colorIndex;
        }
    } else {
        painter.setBrush(QColor("#dadada"));
        painter.setPen(Qt::white);
        painter.drawEllipse(pieRect);
    }
    painter.setRenderHint(QPainter::Antialiasing, false);

    int legendY = pieY + 10;
    const int legendX = pieRect.right() + gap;
    painter.setFont(QFont("Arial", 10));
    int colorIndex = 0;
    for (auto it = zoneCounts.constBegin(); it != zoneCounts.constEnd(); ++it) {
        const int cnt = it.value();
        const int pct = (totalQuais > 0) ? qRound(100.0 * cnt / totalQuais) : 0;
        const QColor color = zoneColors[colorIndex % zoneColors.size()];

        painter.fillRect(QRect(legendX, legendY + 6, 10, 10), color);
        painter.setPen(Qt::black);
        painter.drawText(QRect(legendX + 18, legendY - 2, legendW - 24, 22),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         QString("%1: %2 (%3%)").arg(it.key()).arg(cnt).arg(pct));
        legendY += 24;
        ++colorIndex;
    }

    const int zoneBlockBottom = qMax(pieY + pieSize, legendY);
    const int minStatusStartY = static_cast<int>(pageH * 0.58);
    const int barsTitleY = qMax(zoneBlockBottom + 60, minStatusStartY);

    painter.setPen(QPen(QColor(200, 210, 220), 2));
    painter.drawLine(leftMargin + 60, barsTitleY - 12, leftMargin + contentW - 60, barsTitleY - 12);

    painter.setPen(QColor(0, 82, 155));
    painter.setFont(QFont("Arial", 13, QFont::Bold));
    painter.drawText(QRect(leftMargin + 24, barsTitleY, contentW - 48, 34), Qt::AlignLeft,
                     QString::fromUtf8("statistique par statut"));

    int barsY = barsTitleY + 44;

    const int labelX = leftMargin + 24;
    const int labelW = 130;
    const int barX = labelX + labelW + 14;
    const int valueW = 150;
    const int barMaxW = qMax(320, contentW - (barX - leftMargin) - valueW - 24);
    const int barH = 30;
    const int barSpacing = 16;

    auto statusColorFor = [](const QString& raw) {
        const QString s = raw.trimmed().toLower();
        if (s.contains(QStringLiteral("lib"))) return QColor("#2ecc71");
        if (s.contains(QStringLiteral("occup"))) return QColor("#e74c3c");
        if (s.contains(QStringLiteral("maint"))) return QColor("#e67e22");
        return QColor("#90a4ae");
    };

    for (auto it = statutCounts.constBegin(); it != statutCounts.constEnd(); ++it) {
        const int cnt = it.value();
        const int pct = (totalQuais > 0) ? qRound(100.0 * cnt / totalQuais) : 0;
        const QColor fillColor = statusColorFor(it.key());

        painter.setPen(Qt::black);
        painter.drawText(QRect(labelX, barsY, labelW, barH), Qt::AlignVCenter | Qt::AlignLeft, it.key());

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(230, 235, 245));
        painter.drawRoundedRect(QRect(barX, barsY, barMaxW, barH), 6, 6);

        const int fillW = (pct > 0) ? qMax(30, qRound((static_cast<double>(pct) / 100.0) * barMaxW)) : 0;
        if (fillW > 0) {
            painter.setBrush(fillColor);
            painter.drawRoundedRect(QRect(barX, barsY, fillW, barH), 6, 6);
        }

        painter.setPen(Qt::black);
        painter.drawText(QRect(barX + barMaxW + 12, barsY, valueW, barH), Qt::AlignVCenter | Qt::AlignLeft,
                         QString("%1 (%2%)").arg(cnt).arg(pct));

        barsY += barH + barSpacing;
    }

    painter.setPen(QColor("#8a8a8a"));
    painter.setFont(QFont("Arial", 9));
    painter.drawText(QRect(leftMargin, footerY, contentW, 20), Qt::AlignCenter,
                     QString("AQUATEC - Rapport g├⌐n├⌐r├⌐ le %1").arg(exportStamp));

    painter.end();

    QMessageBox::information(this, "Export PDF", QString("PDF export├⌐ avec succ├¿s !\n%1").arg(filePath));
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

    QMap<QString, QPair<int, int>> stats; // zone -> (occup├⌐s, total en base)

    // Normalise les libell├⌐s de zone pour correspondre exactement
    // aux cl├⌐s attendues ("Nord", "Sud", "Est", "Ouest"),
    // quel que soit le texte stock├⌐ en base.
    auto normalizeZone = [](const QString &raw) -> QString {
        const QString lower = raw.trimmed().toLower();
        if (lower == QStringLiteral("nord"))  return QStringLiteral("Nord");
        if (lower == QStringLiteral("sud"))   return QStringLiteral("Sud");
        if (lower == QStringLiteral("est"))   return QStringLiteral("Est");
        if (lower == QStringLiteral("ouest")) return QStringLiteral("Ouest");
        return raw.trimmed();
    };

    // Calcul des statistiques *uniquement* ├á partir de la base QUAIS
    // pour garantir que les valeurs affich├⌐es correspondent aux donn├⌐es r├⌐elles.
    Connection *conn = Connection::getInstance();
    if (!conn->ensureOpen()) {
        QMessageBox::critical(this,
                              QStringLiteral("DB"),
                              QStringLiteral("Connexion DB ├⌐chou├⌐e: %1").arg(conn->lastErrorText()));
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

    // Pourcentage d'occupation ├á l'int├⌐rieur d'une zone
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
    // bas├⌐e sur les m├¬mes donn├⌐es "stats" (table ou base).
    int totalOccupe = 0;
    int totalQuais  = 0;
    for (auto it = stats.constBegin(); it != stats.constEnd(); ++it) {
        totalOccupe += it->first;
        totalQuais  += it->second;
    }

    const int pctGlobal = (totalQuais > 0) ? (totalOccupe * 100) / totalQuais : 0;

    // R├⌐partition des quais par zone (pour le bloc "R├ëPARTITION PAR ZONE") :
    // pourcentage du nombre total de quais appartenant ├á chaque zone.
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

    // On utilise ces pourcentages de r├⌐partition pour les barres verticales
    // "R├ëPARTITION PAR ZONE".
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
            QStringLiteral("├ëvolution du taux d'occupation (%1 %)").arg(pctGlobal));
    }

    // Tooltips sur les points de la courbe pour donner les stats
    // d'occupation par zone directement depuis la base.
    const auto nordCounts  = getZoneCounts(QStringLiteral("Nord"));
    const auto sudCounts   = getZoneCounts(QStringLiteral("Sud"));
    const auto estCounts   = getZoneCounts(QStringLiteral("Est"));
    const auto ouestCounts = getZoneCounts(QStringLiteral("Ouest"));

    const QString ttNordOcc  = QStringLiteral("Zone Nord : %1 % occup├⌐ (%2 / %3 quais)")
                                   .arg(pctNord)
                                   .arg(nordCounts.first)
                                   .arg(nordCounts.second);
    const QString ttSudOcc   = QStringLiteral("Zone Sud : %1 % occup├⌐ (%2 / %3 quais)")
                                   .arg(pctSud)
                                   .arg(sudCounts.first)
                                   .arg(sudCounts.second);
    const QString ttEstOcc   = QStringLiteral("Zone Est : %1 % occup├⌐ (%2 / %3 quais)")
                                   .arg(pctEst)
                                   .arg(estCounts.first)
                                   .arg(estCounts.second);
    const QString ttOuestOcc = QStringLiteral("Zone Ouest : %1 % occup├⌐ (%2 / %3 quais)")
                                   .arg(pctOuest)
                                   .arg(ouestCounts.first)
                                   .arg(ouestCounts.second);

    // Occupation par zone pour la courbe
    if (ui->curvePoint1_2) ui->curvePoint1_2->setToolTip(ttNordOcc);
    if (ui->curvePoint3_2) ui->curvePoint3_2->setToolTip(ttSudOcc);
    if (ui->curvePoint5_2) ui->curvePoint5_2->setToolTip(ttEstOcc);
    if (ui->curvePoint7_2) ui->curvePoint7_2->setToolTip(ttOuestOcc);

    // R├⌐partition du nombre de quais pour les barres verticales
    const QString ttNordShare  = QStringLiteral("Zone Nord : %1 % des quais (%2 / %3, %4 % occup├⌐s)")
                                     .arg(shareNord)
                                     .arg(nordCounts.second)
                                     .arg(totalQuais)
                                     .arg(pctNord);
    const QString ttSudShare   = QStringLiteral("Zone Sud : %1 % des quais (%2 / %3, %4 % occup├⌐s)")
                                     .arg(shareSud)
                                     .arg(sudCounts.second)
                                     .arg(totalQuais)
                                     .arg(pctSud);
    const QString ttEstShare   = QStringLiteral("Zone Est : %1 % des quais (%2 / %3, %4 % occup├⌐s)")
                                     .arg(shareEst)
                                     .arg(estCounts.second)
                                     .arg(totalQuais)
                                     .arg(pctEst);
    const QString ttOuestShare = QStringLiteral("Zone Ouest : %1 % des quais (%2 / %3, %4 % occup├⌐s)")
                                     .arg(shareOuest)
                                     .arg(ouestCounts.second)
                                     .arg(totalQuais)
                                     .arg(pctOuest);

    if (ui->progressZoneNord_2)  ui->progressZoneNord_2->setToolTip(ttNordShare);
    if (ui->progressZoneSud_2)   ui->progressZoneSud_2->setToolTip(ttSudShare);
    if (ui->progressZoneEst_2)   ui->progressZoneEst_2->setToolTip(ttEstShare);
    if (ui->progressZoneOuest_2) ui->progressZoneOuest_2->setToolTip(ttOuestShare);

    // -------------------------------------------------------------
    // Mise ├á jour de la carte (zoneNord_map, zoneSud_map, etc.)
    // -------------------------------------------------------------
    auto updateZoneMap = [&](const QString &zoneName,
                             QFrame *zoneFrame,
                             QLabel *valueLabel,
                             int occupe, int total) {
        Q_UNUSED(zoneName);
        Q_UNUSED(occupe);
        Q_UNUSED(total);
        if (!zoneFrame) return;

        // Pas de grand carreau color├⌐ : fond transparent, pas de bordure
        zoneFrame->setStyleSheet(QStringLiteral("background-color: transparent; border: none;"));
        // Retirer les contraintes de taille fixe pour que le frame s'adapte
        zoneFrame->setMinimumSize(0, 0);
        zoneFrame->setMaximumSize(16777215, 16777215);

        // Cacher le label "X / Y occup├⌐s"
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
    // les points en fonction des pourcentages calcul├⌐s.
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

        // Points interm├⌐diaires pour lisser la courbe : moyenne entre zones voisines
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

        // Si tu ajoutes plus tard un point d├⌐di├⌐ au taux global
        // (par exemple curvePoint8_2 dans le .ui), tu pourras ici
        // le positionner en fonction de pctGlobal.

        // Dessine une ligne color├⌐e reliant tous les points
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

        // Supprime les points nuls ├⌐ventuels
        QVector<QPoint> validPoints;
        validPoints.reserve(curvePoints.size());
        for (const QPoint &pt : curvePoints) {
            if (!pt.isNull())
                validPoints.append(pt);
        }

        // Tracer des lignes de grille horizontales pour les niveaux cl├⌐s (0,25,50,75,100 %)
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
    // Affichage d├⌐taill├⌐ des places de quai par zone (carte)
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

            // Fonction pour mettre ├á jour l'affichage d'une zone donn├⌐e
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
                    zoneIcon = QStringLiteral("\u2191"); // ↑
                    zoneBadgeColor = QStringLiteral("#1565c0");
                } else if (zone == QStringLiteral("Sud")) {
                    zoneIcon = QStringLiteral("\u2193"); // ↓
                    zoneBadgeColor = QStringLiteral("#2e7d32");
                } else if (zone == QStringLiteral("Est")) {
                    zoneIcon = QStringLiteral("\u2192"); // →
                    zoneBadgeColor = QStringLiteral("#f57f17");
                } else if (zone == QStringLiteral("Ouest")) {
                    zoneIcon = QStringLiteral("\u2190"); // ←
                    zoneBadgeColor = QStringLiteral("#6a1b9a");
                }

                // Titre de la zone : badge color├⌐ avec ombre
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

                    // Couleurs selon le statut avec d├⌐grad├⌐
                    QString bgStart, bgEnd, borderColor, statusEmoji;
                    if (statut.compare(QStringLiteral("Libre"), Qt::CaseInsensitive) == 0) {
                        bgStart = QStringLiteral("#66bb6a");
                        bgEnd = QStringLiteral("#43a047");
                        borderColor = QStringLiteral("#2e7d32");
                        statusEmoji = QStringLiteral("\u2713"); // ✓
                    } else if (statut.compare(QStringLiteral("Occupe"), Qt::CaseInsensitive) == 0) {
                        bgStart = QStringLiteral("#ef5350");
                        bgEnd = QStringLiteral("#e53935");
                        borderColor = QStringLiteral("#c62828");
                        statusEmoji = QStringLiteral("\u26D4"); // ⛔
                    } else if (statut.compare(QStringLiteral("Maintenance"), Qt::CaseInsensitive) == 0) {
                        bgStart = QStringLiteral("#ffa726");
                        bgEnd = QStringLiteral("#fb8c00");
                        borderColor = QStringLiteral("#ef6c00");
                        statusEmoji = QStringLiteral("\u2699"); // ⚙
                    } else {
                        bgStart = QStringLiteral("#90a4ae");
                        bgEnd = QStringLiteral("#78909c");
                        borderColor = QStringLiteral("#546e7a");
                        statusEmoji = QStringLiteral("?");
                    }

                    // Carreau avec d├⌐grad├⌐, ID + emoji statut
                    QWidget *tileWidget = new QWidget();
                    tileWidget->setFixedSize(46, 50);
                    tileWidget->setToolTip(QStringLiteral(
                        "Quai %1\nZone: %2\nStatut: %3")
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
                        "background: transparent; border: none;"));
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
                    QLabel *emptyLabel = new QLabel(QStringLiteral("Aucun quai"));
                    emptyLabel->setAlignment(Qt::AlignCenter);
                    emptyLabel->setStyleSheet(QStringLiteral(
                        "color: #90a4ae; font-size: 9px; font-style: italic; "
                        "background: transparent; border: none;"));
                    mainVLayout->addWidget(emptyLabel);
                }

                layout->addWidget(placesWidget, 0, Qt::AlignCenter);
            };

            // Mettre ├á jour chaque zone
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

    // Some labels are updated asynchronously (network), so normalize after updates.
    normalizeUiTexts();
}

void MainWindow::refreshWeatherForPage3()
{
    if (!ui || !m_weatherNetwork) return;

    updateWeatherLabels(QStringLiteral("\u23F3"),
                        QStringLiteral("--\u00B0C"),
                        QStringLiteral("Chargement m\u00E9t\u00E9o..."),
                        QStringLiteral("Vent: -- km/h"),
                        QStringLiteral("Humidit\u00E9: --%"));

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
            updateWeatherLabels(QStringLiteral("\u26D4"),
                                QStringLiteral("--\u00B0C"),
                                QStringLiteral("M\u00E9t\u00E9o indisponible"),
                                QStringLiteral("Vent: -- km/h"),
                                QStringLiteral("Humidit\u00E9: --%"));
            return;
        }

        QJsonParseError parseGeoError;
        const QJsonDocument geoDoc = QJsonDocument::fromJson(geoPayload, &parseGeoError);
        if (parseGeoError.error != QJsonParseError::NoError || !geoDoc.isObject()) {
            updateWeatherLabels(QStringLiteral("\u26D4"),
                                QStringLiteral("--\u00B0C"),
                                QStringLiteral("R\u00E9ponse m\u00E9t\u00E9o invalide"),
                                QStringLiteral("Vent: -- km/h"),
                                QStringLiteral("Humidit\u00E9: --%"));
            return;
        }

        const QJsonObject geoObj = geoDoc.object();
        const QJsonArray results = geoObj.value(QStringLiteral("results")).toArray();
        if (results.isEmpty() || !results.first().isObject()) {
            updateWeatherLabels(QStringLiteral("\u26D4"),
                                QStringLiteral("--\u00B0C"),
                                QStringLiteral("Localisation m\u00E9t\u00E9o introuvable"),
                                QStringLiteral("Vent: -- km/h"),
                                QStringLiteral("Humidit\u00E9: --%"));
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
                updateWeatherLabels(QStringLiteral("\u26D4"),
                                    QStringLiteral("--\u00B0C"),
                                    QStringLiteral("M\u00E9t\u00E9o indisponible (%1)").arg(cityName),
                                    QStringLiteral("Vent: -- km/h"),
                                    QStringLiteral("Humidit\u00E9: --%"));
                return;
            }

            QJsonParseError parseWeatherError;
            const QJsonDocument weatherDoc = QJsonDocument::fromJson(weatherPayload, &parseWeatherError);
            if (parseWeatherError.error != QJsonParseError::NoError || !weatherDoc.isObject()) {
                updateWeatherLabels(QStringLiteral("\u26D4"),
                                    QStringLiteral("--\u00B0C"),
                                    QStringLiteral("Donn\u00E9es m\u00E9t\u00E9o invalides (%1)").arg(cityName),
                                    QStringLiteral("Vent: -- km/h"),
                                    QStringLiteral("Humidit\u00E9: --%"));
                return;
            }

            const QJsonObject root = weatherDoc.object();
            const QJsonObject current = root.value(QStringLiteral("current")).toObject();
            if (current.isEmpty()) {
                updateWeatherLabels(QStringLiteral("\u26D4"),
                                    QStringLiteral("--\u00B0C"),
                                    QStringLiteral("Aucune m\u00E9t\u00E9o courante (%1)").arg(cityName),
                                    QStringLiteral("Vent: -- km/h"),
                                    QStringLiteral("Humidit\u00E9: --%"));
                return;
            }

            const double temp = current.value(QStringLiteral("temperature_2m")).toDouble();
            const int humidity = current.value(QStringLiteral("relative_humidity_2m")).toInt();
            const double wind = current.value(QStringLiteral("wind_speed_10m")).toDouble();
            const int code = current.value(QStringLiteral("weather_code")).toInt();

            const QString icon = weatherIconEmoji(code);
            const QString desc = weatherDescriptionFr(code);
            const QString tempText = QStringLiteral("%1\u00B0C").arg(QString::number(temp, 'f', 1));
            const QString descText = QStringLiteral("%1 \u2022 %2").arg(cityName, desc);
            const QString windText = QStringLiteral("Vent: %1 km/h").arg(QString::number(wind, 'f', 1));
            const QString humidityText = QStringLiteral("Humidit\u00E9: %1%").arg(QString::number(humidity));

            updateWeatherLabels(icon, tempText, descText, windText, humidityText);
        });
    });
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
    if (weatherCode == 0) return QStringLiteral("Ensoleill\u00E9");
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
    // Use BMP symbols to avoid encoding issues across toolchains.
    if (weatherCode == 0) return QStringLiteral("\u2600\uFE0F");        // ☀️
    if (weatherCode == 1 || weatherCode == 2) return QStringLiteral("\u26C5"); // ⛅
    if (weatherCode == 3) return QStringLiteral("\u2601\uFE0F");        // ☁️
    if (weatherCode == 45 || weatherCode == 48) return QStringLiteral("\u2601\uFE0F"); // ☁️ (fog fallback)
    if ((weatherCode >= 51 && weatherCode <= 57) ||
        (weatherCode >= 61 && weatherCode <= 67) ||
        (weatherCode >= 80 && weatherCode <= 82)) {
        return QStringLiteral("\u2614"); // ☔
    }
    if (weatherCode >= 71 && weatherCode <= 77) return QStringLiteral("\u2744\uFE0F"); // ❄️
    if (weatherCode >= 95 && weatherCode <= 99) return QStringLiteral("\u26A1");      // ⚡
    return QStringLiteral("\u2601\uFE0F");
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
