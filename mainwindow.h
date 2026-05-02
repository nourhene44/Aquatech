
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QLabel>
#include <QDateTime>
#include <QTimer>
#include <QHash>
#include <QSet>
#include <QVector>
#include "connection.h"
#include "quai.h"
#include "pecheurs.h"
#include "employe.h"
#include "captures.h"

class QFrame;
class QTableWidget;
class QSystemTrayIcon;
class QComboBox;
class QDialog;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;

class ArduinoSerial;

class QNetworkAccessManager;

#include <QMediaCaptureSession>
#include <QCamera>
#include <QImageCapture>
#include <QVideoSink>
#include <QVideoFrame>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Connection* getOracleConnection();
    void closeOracleConnection(Connection* conn);
    void ajouterBateau(const QString& nom, const QString& type, int capacite,
                       const QString& proprietaire, const QString& statut,
                       double largeur);
    void ajouterBateauFromForm();

public slots:
    void editQuaiFromTable(int row);

private slots:
    // Tous vos slots de navigation existants (conserv├⌐s)
    void on_pushButton_3b_17_clicked();
    void on_pushButton_3b_54_clicked();
    void on_pushButton_3b_48_clicked();
    void on_pushButton_3b_5_clicked();
    void on_pushButton_3b_46_clicked();
    void on_pushButton_3b_11_clicked();
    void on_pushButton_3b_42_clicked();
    void on_pushButton_3b_4_clicked();
    void on_pushButton_3b_10_clicked();
    void on_pushButton_3b_16_clicked();
    void on_pushButton_3b_53_clicked();
    void on_pushButton_3b_41_clicked();
    void on_pushButton_3b_40_clicked();
    void on_pushButton_3b_47_clicked();
    void on_pushButton_3b_52_clicked();
    void on_pushButton_3b_55_clicked();
    void on_pushButton_3b_15_clicked();
    void on_pushButton_3b_43_clicked();
    void on_pushButton_3b_49_clicked();
    void on_pushButton_3b_6_clicked();
    void on_pushButton_3b_3_clicked();
    void on_pushButton_3b_18_clicked();
    void on_pushButton_3b_12_clicked();
    void on_pushButton_3b_9_clicked();
    void on_pushButton_3b_19_clicked();
    void on_pushButton_3b_56_clicked();
    void on_pushButton_3b_44_clicked();
    void on_pushButton_3b_45_clicked();
    void on_pushButton_3b_50_clicked();
    void on_pushButton_3b_7_clicked();
    void on_pushButton_3b_13_clicked();
    void on_pushButton_3b_39_clicked();
    void on_pushButton_3b_51_clicked();
    void on_pushButton_3b_8_clicked();
    void on_pushButton_3b_2_clicked();
    void on_pushButton_3b_14_clicked();
    void legacy_pushButton_b_2b_clicked();
    void legacy_pushButton_b_3b_clicked();
    void on_pushButton_b_clicked();
    void legacy_pushButton_b_4b_clicked();
    void on_pushButton_20b_clicked();
    void on_pushButton_13b_clicked();
    void on_pushButton_14b_clicked();
    void on_pushButton_15b_clicked();
    void on_pushButton_16b_clicked();
    void on_pushButton_17b_clicked();
    void on_pushButton_18b_clicked();
    void on_pushButton_19b_clicked();
    void on_p2b_clicked();
    void on_p1b_clicked();
    void on_p3b_clicked();
    void legacy_pushButton_3b_clicked();
    void on_p1_2b_clicked();
    void on_pushButton_12b_clicked();
    void on_pushButton_9b_clicked();
    void on_btnFaceIDp_clicked();
    void on_btnCapturep_clicked();
    void on_btnVerifyp_clicked();
    void on_btnCancelFacep_clicked();
    void on_bmip_clicked();
    void on_bmi_6p_clicked();
    void on_pushButton_11p_clicked();
    void on_pushButton_10p_clicked();
    void on_brmp_clicked();
    void on_p4b_clicked();
    void on_pushButton_clicked();
    void on_p6b_clicked();
    void on_pushButton_10e_clicked();
    void on_pushButton_pdfb_5_clicked();
    void on_pushButton_7c_5_clicked();
    void on_pushButton_8c_4_clicked();
    void on_btnRefresh_4_clicked();
    void on_btnTelegramRemise_clicked();
    void on_btnSend_4_clicked();
    void on_pushButton_8c_3_clicked();
    void on_p5b_clicked();
    void on_cap_btnBackMenu_clicked();
    void on_cap_btnShowSurpeche_clicked();
    void on_cap_btnShowTendance_clicked();
    void on_cap_btnBackToCapturesMainSurpeche_clicked();
    void on_cap_btnBackToCapturesMainTendance_clicked();
    void legacy_pushButton_7b_clicked();
    void legacy_btnai_clicked();
    void legacy_btnSend_2_clicked();
    void on_pushButton_7c_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_9_clicked();
    void refreshStats_2();
    void on_lineEdit_3_textChanged(const QString &text);
    void on_comboBox_3_currentIndexChanged(int index);
    void on_comboBox_4_currentIndexChanged(int index);
    void on_comboBox_5_currentIndexChanged(int index);
    void on_comboBoxCity_currentIndexChanged(int index);
    void on_pushButton_pdfb_clicked();
    void on_pushButton_pdfb_2_clicked();
    void on_btnExportStatsPDF_2_clicked();
    void legacy_comboChartType_2_currentIndexChanged(int index);
    void on_pushButton_2c_clicked();
    void on_lineEdit_7c_textChanged(const QString &text);
    void on_comboBoxc_2_currentTextChanged(const QString &text);
    void on_dateEdit_2c_dateChanged(const QDate &date);
    void on_pushButton_5e_clicked();
    void on_pushButton_5e_2_clicked();
    void on_bap_clicked();
    void on_bep_clicked();
    void on_pushButton_5p_clicked();
    void on_pushButton_6p_clicked();
    void on_lineEdit_4p_textChanged(const QString &text);
    void on_comboBox_5p_currentTextChanged(const QString &text);
    void on_comboBox_6p_currentTextChanged(const QString &text);
    void on_pushButton_2b_clicked();
    void on_pushButton_11_clicked();
    void on_btnExportMapPdf_clicked();
    void on_cap_btnValiider_3_clicked();
    void on_lineEdit_7c_2_textChanged(const QString &text);
    void on_cap_sbQuantite_3_valueChanged(int value);
    void on_cap_deDebut_dateChanged(const QDate &date);
    void on_cap_btnExporter_clicked();
    void on_cap_btnValiider_4_clicked();
    void handleArduinoUid(const QString& uid);
    void handleArduinoDoorOpened(const QString& uid);
    void on_btnAnalyzeRfidQt_clicked();
    void retryArduinoPortDetection();
    void retryArduinoTemperaturePortDetection();
    void retryArduinoBoatPortDetection();

public:
    // Ajout des m├⌐thodes manquantes pour la gestion des quais
    void refreshQuaiTable();
    void ensureQuaiActionsColumn(const QString& css);
    void showQuaiFormPage();
    void setAddButtonText(const QString& text);
    void deleteQuaiById(int id, const QString& css);

    // Temperature status helpers
    QString tempStatusText(double temperatureC);
    QString tempStatusButtonText(double temperatureC);
    QString tempStatusButtonCss(double temperatureC);
    QString tempStatusCss(double temperatureC);

protected:
    Quai m_quai;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::MainWindow *ui;
    QPlainTextEdit* m_portConsole = nullptr;
    int m_lastKnownDistanceCm = -1;
    int m_lastRawPulseMicros = -1;
    int m_currentAssignedQuaiId = -1;
    bool m_boatPresent = false;
    bool m_boatDocked = false;
    qint64 m_arrivalDetectedAtMs = 0;
    int m_baselineDistanceCm = -1;
    qint64 m_baselineSum = 0;
    int m_baselineSamples = 0;
    int m_consecutiveBelow = 0;
    int m_consecutiveAbove = 0;
    QString m_editingPecheurId;
    int m_editingEmployeId = -1;
    QString m_currentCvPath;
    QString m_editingClientId;
    QString m_editingCaptureId;
    int m_captureQuantiteFiltre = 0;
    QDate m_captureDateFiltre;
        bool m_editingCaptureHasTemperature = false;
        double m_editingCaptureTemperatureC = 0.0;
    QLabel* m_curveLineLabelQuaiStats = nullptr;
    QTimer* m_statsTimer = nullptr;
    QTimer* m_weatherTimer = nullptr;
    QNetworkAccessManager* m_weatherNetwork = nullptr;
    QString m_selectedCity = QStringLiteral("Bizerte");
    bool m_quaisFermeMeteo = false;
    QHash<int, QString> m_quaisAutoLocked;

    struct DailyForecastItem {
        QDate date;
        double tMin = 0.0;
        double tMax = 0.0;
        int code = 0;
        int probMax = -1;
        QDateTime sunrise;
        QDateTime sunset;
    };
    QVector<DailyForecastItem> m_dailyForecast;
    int m_selectedDailyIndex = 0;

    QString m_lastWeatherIcon;
    QString m_lastWeatherTemp;
    QString m_lastWeatherDesc;
    QString m_lastWeatherWind;
    QString m_lastWeatherHumidity;
    QString m_lastWeatherHourly;
    QString m_lastWeatherDaily;
    bool m_lastWeatherIsDay = true;
    bool m_hasLastWeather = false;

    // --- Arduino / Serial integration ---
    ArduinoSerial* m_arduino = nullptr;
    ArduinoSerial* m_arduinoTemp = nullptr;
    ArduinoSerial* m_arduinoBoat = nullptr;
    QComboBox* m_arduinoPortCombo = nullptr;
    QPushButton* m_arduinoConnectButton = nullptr;
    QLineEdit* m_arduinoTxEdit = nullptr;
    QPushButton* m_arduinoSendButton = nullptr;
    QLabel* m_arduinoStatusLabel = nullptr;
    QTimer* m_arduinoPortDetectionTimer = nullptr;
    QTimer* m_arduinoTempPortDetectionTimer = nullptr;
    QTimer* m_arduinoBoatPortDetectionTimer = nullptr;
    QPushButton* m_capArduinoTempButton = nullptr;
    QFrame* m_capArduinoTempFrame = nullptr;
     QDialog* m_arduinoTempDialog = nullptr;
     QLabel* m_arduinoTempValueLabel = nullptr;
     QLabel* m_arduinoTempBadgeLabel = nullptr;
     QLabel* m_arduinoTempConnLabel = nullptr;
     QPlainTextEdit* m_arduinoTempHistory = nullptr;
     bool m_hasArduinoTemperature = false;
     double m_lastArduinoTemperatureC = 0.0;
     double m_arduinoTempDangerThreshold = 29.0;
     double m_arduinoTempCritiqueThreshold = 32.0;
     QString m_lastAppliedUid;
     qint64 m_lastAppliedMs = 0;

    // --- Mot de passe oublie (OTP email) ---
    QString m_passwordResetCode;
    QString m_passwordResetEmail;
    bool m_passwordResetVerified = false;

    QString generateCode();
    QString fallbackAdminPassword() const;
    void setFallbackAdminPassword(const QString& newPassword);
    void clearPasswordResetState();

    bool sendEmail(const QString& to, const QString& content, QString* errorOut = nullptr, const QString& subject = "AquaTech Notification");
    bool resetAdminPassword(const QString& adminEmail, const QString& newPassword, QString* errorOut = nullptr);

    // --- Bateau CRUD ---
    QString currentEditingId;
    void loadBateaux();
    void modifierBateauFromRow(int row);
    void supprimerBateauFromRow(int row);

    // --- Client CRUD (Actions column) ---
    void modifierClientFromRow(int row);
    void supprimerClientFromRow(int row);
    void filterBateaux();
    void updateStatsBateaux();
    Pecheurs pecheurFromForm() const;
    void loadPecheurs();
    void loadPecheurFromTable();
    void resetAjouterButton();

    // --- Captures CRUD ---
    captures captureFromForm() const;
    void loadCaptures();
    void loadCaptureFromTable(int row);
    void supprimerCaptureFromRow(int row);
    void resetCaptureForm();

    // --- Quotas (page Captures) ---
    void loadQuotas(bool showErrors = false);
    bool saveQuotas(QString* errorOut = nullptr);
    void setQuotaTableEditable(bool editable);
    bool m_quotaEditMode = false;

    // --- Employe CRUD ---
    void loadEmployes();
    void editEmployeFromTable(int row);
    void ensureEmployeActionsColumn(const QString& css);
    void ensureEmployeRfidUi();

    void showFrame(QWidget* frameToShow);
    void setupFrames();
    void normalizeUiTexts();
    void setupArduinoIntegration();
    QString preferredRfidPortName(const QStringList& ports, bool honorCurrentSelection = true) const;
    QString preferredTemperaturePortName(const QStringList& ports) const;
    QString preferredBoatPortName(const QStringList& ports) const;
    void refreshArduinoPortList();
    void updateArduinoUiState();
    void updateEmployeePageRfidStatus();
    void setupArduinoTemperatureButton();
    void toggleCapturesArduinoTemperatureView();
    void ensureCapturesArduinoTemperatureView();
    void setCapturesArduinoTemperatureViewVisible(bool visible);
    void showArduinoTemperatureWindow();
    void ensureArduinoTemperatureConnected();
    void sendArduinoTemperatureSetup();
    void handleArduinoTemperatureLine(const QString& line);
    void handleArduinoBoatLine(const QString& line);
    void updateArduinoTemperatureDialogConnectionState();
     void updateArduinoTemperatureDialog(double temperatureC, bool appendHistory = true);
    void updateCapturesTableLiveTemperature(double temperatureC);
    void debugAfficherTousLesQuais();
    int findAndReserveFreeQuai();
    void markQuaiFree(int idQuai);
    void applyQuaiFilters();
    void refreshWeatherForPage3();
    void lockQuaisForWeather();
    void unlockQuaisForWeather();
    void updateWeatherLabels(const QString& icon,
                             const QString& temperatureText,
                             const QString& descriptionText,
                             const QString& windText,
                             const QString& humidityText,
                             const QString& hourlyText = QString(),
                             const QString& dailyText = QString(),
                             bool isDay = true);
    void updateSelectedDayDetails();
    bool exportWidgetToPdf(QWidget *widget,
                           const QString &defaultFileName,
                           const QString &dialogTitle);

    // Mise en forme tableaux
    void adjustClientTableColumns();
    void adjustTopClientsStatsColumns();

    // Gestion des clients (recherche + statistiques)
    void refreshClientsPage();
    void refreshTopClientsStats();

    // --- Alertes automatiques de maintenance préventive (Bateaux) ---
    void setupBateauMaintenanceAlertSystem();
    void refreshBateauMaintenanceAlerts(bool persistNewAlerts);
    void notifyBateauMaintenanceAlertsIfAny();
    void loadBateauAlertHistorySeenKeys();
    void ensureBateauAlertHistoryPanel();
    void toggleBateauAlertHistoryPanel();
    void populateBateauAlertHistoryTable();

    QTimer* m_bateauMaintenanceAlertTimer = nullptr;
    QSystemTrayIcon* m_bateauTrayIcon = nullptr;
    QDateTime m_lastBateauAlertNotificationAt;

    QLabel* m_bateauAlertBadge = nullptr;

    QFrame* m_bateauAlertHistoryFrame = nullptr;
    QTableWidget* m_bateauAlertHistoryTable = nullptr;
    QSet<QString> m_bateauAlertSeenKeys;

    int m_bateauAlertsWarning = 0;
    int m_bateauAlertsUrgent = 0;
    int m_bateauAlertsCritical = 0;
    int m_bateauAlertsHighestLevel = 0; // 0=None,1=Warn,2=Urgent,3=Critical

    // --- FaceID / Camera (from mainwindow1) ---
    QByteArray m_pendingPecheurPhotoBytes;
    QString m_faceCaptureTempFile;
    QCamera* m_faceCamera = nullptr;
    QMediaCaptureSession* m_faceCaptureSession = nullptr;
    QImageCapture* m_faceImageCapture = nullptr;
    QLabel* m_facePreviewLabel = nullptr;
    QVideoSink* m_faceVideoSink = nullptr;
    bool m_faceHasRecentFrame = false;
    qint64 m_faceLastFrameAtMs = 0;
    qint64 m_faceOpenStatusUntilMs = 0;
    bool m_faceCapturePending = false;
    int m_faceCapturePendingWaitMs = 0;
    int m_faceCaptureRetryRemaining = 0;

    // --- Face capture pêcheur methods ---
    void setupPecheurFaceCapture();
    void stopPecheurFaceCapture();
    void attemptPecheurFaceCapture();
    void syncPecheurFaceCaptureFields();
    bool persistCapturedPecheurPhoto();
    void updatePecheurFacePreviewGeometry();
    void applyRfidEmployeeToggle(const QString& uid);
};
#endif // MAINWINDOW_H
