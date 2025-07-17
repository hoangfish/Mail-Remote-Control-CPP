#include "MainWindow_ui.h"
#include "MailRemote_ui.h"
#include "CustomMode.h"
#include "AboutUs_ui.h"
#include "MailRemote.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      gmailWindow(nullptr),
      customWindow(nullptr),
      aboutWindow(nullptr)
{
    this->resize(800, 600);
    this->setWindowTitle("Remote Control - Dashboard");
    this->setStyleSheet("background-color: white;");

    centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    // Title
    titleLabel = new QLabel("REMOTE CONTROL SYSTEM", centralWidget);
    titleLabel->setGeometry(0, 50, 800, 50);
    QFont font;
    font.setPointSize(20);
    font.setBold(true);
    titleLabel->setFont(font);
    titleLabel->setAlignment(Qt::AlignCenter);

    // Gmail Button
    btn_gmail = new QPushButton("📧 Gmail Remote", centralWidget);
    btn_gmail->setGeometry(300, 150, 200, 50);
    btn_gmail->setStyleSheet("background-color: #26c6da; color: white; font-size: 16px; border-radius: 10px;");
    connect(btn_gmail, &QPushButton::clicked, this, &MainWindow::openGmailRemote);

    // Custom Button
    btn_custom = new QPushButton("🛠️ Custom Mode", centralWidget);
    btn_custom->setGeometry(300, 230, 200, 50);
    btn_custom->setStyleSheet("background-color: #26a69a; color: white; font-size: 16px; border-radius: 10px;");
    connect(btn_custom, &QPushButton::clicked, this, &MainWindow::openCustomMode);

    // About Button
    btn_about = new QPushButton("👨‍💻 About Us", centralWidget);
    btn_about->setGeometry(300, 310, 200, 50);
    btn_about->setStyleSheet("background-color: #7e57c2; color: white; font-size: 16px; border-radius: 10px;");
    connect(btn_about, &QPushButton::clicked, this, &MainWindow::openAboutUs);

    // Menu + Status
    this->setMenuBar(new QMenuBar(this));
    this->setStatusBar(new QStatusBar(this));
}

MainWindow::~MainWindow() {}

void MainWindow::openGmailRemote()
{
    if (!gmailWindow)
        gmailWindow = new MailRemote();
    gmailWindow->show();
}

void MainWindow::openCustomMode()
{
    if (!customWindow)
        customWindow = new CustomMode();
    customWindow->show();
}

void MainWindow::openAboutUs()
{
    if (!aboutWindow)
        aboutWindow = new AboutUs();
    aboutWindow->show();
}
