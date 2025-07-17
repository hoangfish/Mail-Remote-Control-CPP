#include "../lib/CustomMode_ui.h"
CustomModeUI::CustomModeUI(QWidget *parent)
    : QWidget(parent)
{
    this->setWindowTitle("Custom Mode - Control Center");
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Taskbar
    QWidget *taskbarWidget = new QWidget;
    taskbarWidget->setStyleSheet("background-color: #2c3e50; padding: 12px; border-radius: 8px;");
    QHBoxLayout *taskbarLayout = new QHBoxLayout(taskbarWidget);
    QLabel *labelTitle = new QLabel("🔧 Custom Control Center");
    labelTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: white;");
    taskbarLayout->addWidget(labelTitle);
    taskbarLayout->addStretch();
    mainLayout->addWidget(taskbarWidget);

    // IP input
    QHBoxLayout *ipLayout = new QHBoxLayout;
    ipLayout->setSpacing(10);
    ipLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *labelIP = new QLabel("Server IP:");
    lineEditServerIP = new QLineEdit;
    lineEditServerIP->setPlaceholderText("Enter server IP...");
    ipLayout->addWidget(labelIP);
    ipLayout->addWidget(lineEditServerIP);
    mainLayout->addLayout(ipLayout);

    // Content layout
    QHBoxLayout *contentLayout = new QHBoxLayout;

    // Sidebar
    QWidget *sidebar = new QWidget;
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setSpacing(8);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);

    // Sidebar buttons
    btnSystem = new QPushButton("🖥️ System");
    btnProcesses = new QPushButton("Processes");
    btnApps = new QPushButton("Apps");
    btnFiles = new QPushButton("📁 Files");
    btnWebcam = new QPushButton("📷 Webcam");
    btnKeylogger = new QPushButton("Keylogger");

    // Icons
    btnProcesses->setIcon(QIcon("images/process.png"));
    btnApps->setIcon(QIcon("images/apps.png"));
    btnKeylogger->setIcon(QIcon("images/keylogger.png"));
    btnProcesses->setIconSize(QSize(20, 20));
    btnApps->setIconSize(QSize(20, 20));
    btnKeylogger->setIconSize(QSize(20, 20));

    QList<QPushButton*> buttons = { btnSystem, btnProcesses, btnApps, btnFiles, btnWebcam, btnKeylogger };
    for (auto btn : buttons) {
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        btn->setMinimumHeight(40);
        setButtonStyle(btn);
        sidebarLayout->addWidget(btn);
    }

    sidebarLayout->addStretch();

    QLabel *schoolLogo = new QLabel;
    QPixmap pix("images/school_logo.png");
    schoolLogo->setFixedSize(150, 150);
    schoolLogo->setPixmap(pix.scaled(schoolLogo->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    schoolLogo->setAlignment(Qt::AlignCenter);
    schoolLogo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    sidebarLayout->addSpacing(5);
    sidebarLayout->addWidget(schoolLogo, 0, Qt::AlignTop | Qt::AlignHCenter);
    sidebarLayout->addSpacing(5);

    sidebarLayout->addStretch();  // Sẽ đẩy nút back xuống cuối cùng

    QPushButton *backButton = new QPushButton("⬅ Back");
    backButton->setFixedSize(120, 36);
    backButton->setCursor(Qt::PointingHandCursor);
    backButton->setStyleSheet(
        "QPushButton { background-color: #ff3b30; color: white; font-weight: bold; font-size: 15px;"
        " border: none; border-radius: 10px; padding: 6px 12px; text-align: center; }"
        "QPushButton:hover { background-color: #e53e3e; }"
    );
    sidebarLayout->addWidget(backButton, 0, Qt::AlignCenter);
    sidebarLayout->addSpacing(10);
    sidebar->setFixedWidth(160);
    contentLayout->addWidget(sidebar);

    // Stacked widget
    stackedWidget = new QStackedWidget;
    stackedWidget->addWidget(createSystemPage());
    stackedWidget->addWidget(createProcessesPage());
    stackedWidget->addWidget(createAppsPage());
    stackedWidget->addWidget(createFilesPage());
    stackedWidget->addWidget(createWebcamPage());
    stackedWidget->addWidget(createKeyloggerPage());

    contentLayout->addWidget(stackedWidget);
    mainLayout->addLayout(contentLayout);
    this->setLayout(mainLayout);

    // Back button signal
    connect(backButton, &QPushButton::clicked, this, &CustomModeUI::handleBackButton);
}

CustomModeUI::~CustomModeUI() {}

void CustomModeUI::setButtonStyle(QPushButton *button) {
    button->setStyleSheet(
        "QPushButton { background-color: #2c3e50; color: white; font-size: 14px; padding: 10px; text-align: left; border: none; }"
        "QPushButton:hover { background-color: #34495e; }"
    );
}

void CustomModeUI::makeButtonsFixed(const QList<QPushButton *> &buttons, int width, int height) {
    for (auto btn : buttons)
        btn->setFixedSize(width, height);
}

QTextEdit* CustomModeUI::createOutputLog(QWidget *parent) {
    QTextEdit *log = new QTextEdit(parent);
    log->setReadOnly(true);
    log->setMinimumHeight(80);
    log->setStyleSheet("background-color: #f4f4f4; font-family: Consolas;color:black");
    return log;
}

QWidget* CustomModeUI::createSystemPage() {
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);

    layout->setSpacing(10);
    QLabel *label = new QLabel("❗ Use these actions with caution. They will affect the entire system.");
    label->setStyleSheet("color: gray; font-size: 12px;");
    layout->addWidget(label);

    btnRestart = new QPushButton("🔄 Restart");
    btnShutdown = new QPushButton("📴 Shutdown");
    makeButtonsFixed({ btnRestart, btnShutdown });

    layout->addWidget(btnRestart);
    layout->addWidget(btnShutdown);
    layout->addSpacing(15);

    textOutputSystem = createOutputLog();
    layout->addWidget(new QLabel("📤 Output Log:"));
    layout->addWidget(textOutputSystem);

    return page;
}

QWidget* CustomModeUI::createProcessesPage() {
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(10);

    QLabel *label = new QLabel("📋 Manage system processes. View, start, or stop processes.");
    label->setStyleSheet("color: gray; font-size: 12px;");
    layout->addWidget(label);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(10);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnListProcesses = new QPushButton("📄 List");
    btnStartProcess  = new QPushButton("🟢 Start");
    btnStopProcess   = new QPushButton("🔴 Stop");
    makeButtonsFixed({ btnListProcesses, btnStartProcess, btnStopProcess });

    btnLayout->addWidget(btnListProcesses);
    btnLayout->addWidget(btnStartProcess);
    btnLayout->addWidget(btnStopProcess);
    layout->addLayout(btnLayout);

    tableProcesses = new QTableWidget;
    tableProcesses->setColumnCount(3);
    tableProcesses->setHorizontalHeaderLabels({ "PID", "Name", "User" });
    tableProcesses->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableProcesses->setSelectionMode(QAbstractItemView::SingleSelection);
    tableProcesses->verticalHeader()->setVisible(false);
    tableProcesses->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableProcesses->setSortingEnabled(true);
    tableProcesses->setMinimumHeight(180);
    tableProcesses->setMaximumHeight(200);
    tableProcesses->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(tableProcesses);

    layout->addWidget(new QLabel("📤 Output Log:"));
    textOutputProcess = createOutputLog();
    layout->addWidget(textOutputProcess);

    return page;
}

QWidget* CustomModeUI::createAppsPage() {
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(10);

    QLabel *label = new QLabel("🧩 Manage running applications.");
    label->setStyleSheet("color: gray; font-size: 12px;");
    layout->addWidget(label);

    QHBoxLayout *listLayout = new QHBoxLayout;
    listLayout->setSpacing(10);
    listLayout->setContentsMargins(0, 0, 0, 0);
    btnListApps = new QPushButton("📄 List Apps");
    makeButtonsFixed({ btnListApps });
    listLayout->addWidget(btnListApps);
    listLayout->addStretch();
    layout->addLayout(listLayout);

    QHBoxLayout *startLayout = new QHBoxLayout;
    startLayout->setSpacing(10);
    startLayout->setContentsMargins(0, 0, 0, 0);
    btnStartAppOnly = new QPushButton("🟢 Start App");
    lineEditStartAppName = new QLineEdit;
    lineEditStartAppName->setPlaceholderText("Enter application name...");
    makeButtonsFixed({ btnStartAppOnly });
    startLayout->addWidget(btnStartAppOnly);
    startLayout->addWidget(lineEditStartAppName);
    layout->addLayout(startLayout);

    QHBoxLayout *stopLayout = new QHBoxLayout;
    stopLayout->setSpacing(10);
    stopLayout->setContentsMargins(0, 0, 0, 0);
    btnStopAppOnly = new QPushButton("🔴 Stop App");
    lineEditStopAppName = new QLineEdit;
    lineEditStopAppName->setPlaceholderText("Enter application name...");
    makeButtonsFixed({ btnStopAppOnly });
    stopLayout->addWidget(btnStopAppOnly);
    stopLayout->addWidget(lineEditStopAppName);
    layout->addLayout(stopLayout);

    layout->addWidget(new QLabel("📤 Output Log:"));
    textEditAppLog = createOutputLog();
    layout->addWidget(textEditAppLog);

    return page;
}

QWidget* CustomModeUI::createFilesPage() {
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(10);

    QLabel *label = new QLabel("📂 Enter full path to retrieve a file.");
    label->setStyleSheet("color: gray; font-size: 12px;");
    layout->addWidget(label);

    QHBoxLayout *fileLayout = new QHBoxLayout;
    fileLayout->setSpacing(10);
    fileLayout->setContentsMargins(0, 0, 0, 0);
    lineEditFilePath = new QLineEdit;
    lineEditFilePath->setPlaceholderText("C:/path/to/file.txt");
    btnGetFile = new QPushButton("📥 Get File");
    makeButtonsFixed({ btnGetFile });
    fileLayout->addWidget(lineEditFilePath);
    fileLayout->addWidget(btnGetFile);
    layout->addLayout(fileLayout);

    layout->addWidget(new QLabel("📤 Output Log:"));
    textOutputFiles = createOutputLog();
    layout->addWidget(textOutputFiles);

    return page;
}

QWidget* CustomModeUI::createWebcamPage() {
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(10);

    QLabel *label = new QLabel("📸 Capture webcam image or video.");
    label->setStyleSheet("color: gray; font-size: 12px;");
    layout->addWidget(label);

    QHBoxLayout *camLayout = new QHBoxLayout;
    camLayout->setSpacing(10);
    camLayout->setContentsMargins(0, 0, 0, 0);
    btnCapturePhoto = new QPushButton("📷 Capture Photo");
    btnRecordVideo  = new QPushButton("🎥 Record Video");
    makeButtonsFixed({ btnCapturePhoto, btnRecordVideo });
    spinBoxRecordSec = new QSpinBox;
    spinBoxRecordSec->setRange(1, 120);
    spinBoxRecordSec->setSuffix(" sec");
    camLayout->addWidget(btnCapturePhoto);
    camLayout->addWidget(btnRecordVideo);
    camLayout->addWidget(spinBoxRecordSec);
    layout->addLayout(camLayout);

    layout->addWidget(new QLabel("📤 Output Log:"));
    textOutputWebcam = createOutputLog();
    layout->addWidget(textOutputWebcam);

    return page;
}

QWidget* CustomModeUI::createKeyloggerPage() {
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);

    QLabel *label = new QLabel("📝 View recorded keystrokes.");
    label->setStyleSheet("color: gray; font-size: 12px;");
    layout->addWidget(label);

    btnKeyLogger = new QPushButton("🧠 Start Keylogger");
    makeButtonsFixed({ btnKeyLogger });
    layout->addWidget(btnKeyLogger);

    layout->addWidget(new QLabel("📤 Output Log:"));
    textOutputKeylogger = createOutputLog();
    layout->addWidget(textOutputKeylogger);

    return page;
}

void CustomModeUI::handleBackButton() {
    if (this->parentWidget())
        this->parentWidget()->close();
}
