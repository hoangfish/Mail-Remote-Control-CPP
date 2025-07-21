#include "AboutUs_ui.h"
AboutUs::AboutUs(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

AboutUs::~AboutUs() {}

QString AboutUs::imagePath(const QString &filename)
{
    return QString("images/%1").arg(filename); // Assuming working dir is at /client
}

void AboutUs::setupUI()
{
    setWindowTitle("About Us");
    resize(800, 900);
    setStyleSheet("background-color: #f0f2f5;");

    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);

    scrollWidget = new QWidget();
    scrollArea->setWidget(scrollWidget);

    mainLayout = new QVBoxLayout(scrollWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(25);

    QPushButton *backButton = new QPushButton("⬅ Back");
    backButton->setFixedSize(100, 40);
    backButton->setCursor(Qt::PointingHandCursor);
    backButton->setStyleSheet(
        "QPushButton { background-color: #ff3b30; color: white; font-weight: bold;"
        "border: none; border-radius: 12px; padding-left: 10px; }"
        "QPushButton:hover { background-color: #e53e3e; }");
    connect(backButton, &QPushButton::clicked, this, &AboutUs::handleBackButton);

    QFrame *backFrame = new QFrame();
    QHBoxLayout *backLayout = new QHBoxLayout(backFrame);
    backLayout->setContentsMargins(20, 0, 0, 0);
    backLayout->setAlignment(Qt::AlignLeft);
    backLayout->addWidget(backButton);

    mainLayout->addSpacing(20);
    mainLayout->addWidget(backFrame);

    addSchoolLogo();

    QLabel *titleLabel = new QLabel("👨‍💻 Meet the Team");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 26px; font-weight: bold; color: #222;");
    mainLayout->addWidget(titleLabel);

    addProjectDescription();
    addAvatarsAndShortInfo();

    for (const auto &member : teamMembers)
    {
        QFrame *card = createMemberCard(member);
        mainLayout->addWidget(card);
        memberCards.append(card);
    }

    addFooter();

    setCentralWidget(scrollArea);
}

void AboutUs::handleBackButton()
{
    close();
}

void AboutUs::addSchoolLogo()
{
    QHBoxLayout *logoRow = new QHBoxLayout();
    logoRow->setAlignment(Qt::AlignCenter);
    logoRow->setSpacing(40); // tăng spacing giữa logo và text

    QLabel *logo = new QLabel();
    QPixmap pixmap(imagePath("school_logo.png"));
    logo->setPixmap(pixmap.scaled(140, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logo->setAlignment(Qt::AlignCenter);
    logoRow->addWidget(logo);

    QVBoxLayout *textBox = new QVBoxLayout();
    textBox->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    textBox->setSpacing(6);

    QLabel *fitLabel = new QLabel("fit@hcmus");
    fitLabel->setStyleSheet("font-size: 34px; font-weight: bold; color: #003399;");

    QLabel *vnuLabel = new QLabel("VNUHCM - UNIVERSITY OF SCIENCE");
    vnuLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #222;");

    QLabel *facultyLabel = new QLabel("FACULTY OF INFORMATION TECHNOLOGY");
    facultyLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #222;");

    textBox->addWidget(fitLabel);
    textBox->addWidget(vnuLabel);
    textBox->addWidget(facultyLabel);
    logoRow->addLayout(textBox);

    mainLayout->addLayout(logoRow);

    QLabel *classLabel = new QLabel("Class: Graduation topic on computer network");
    classLabel->setAlignment(Qt::AlignCenter);
    classLabel->setStyleSheet("font-size: 16px; color: #222; font-weight: 500; margin-top: 10px;");
    mainLayout->addWidget(classLabel);
}

void AboutUs::addProjectDescription()
{
    QLabel *projectLabel = new QLabel();
    projectLabel->setWordWrap(true);
    projectLabel->setText(
        "<b>🛠 Project Description:</b><br>"
        "A simple remote-control tool using Gmail for command exchange and system interaction.");
    projectLabel->setStyleSheet(
        "font-size: 14px; color: #333; line-height: 1.6; background-color: #ffffff;"
        "border-radius: 10px; padding: 15px; border: 1px solid #ddd;");
    mainLayout->addWidget(projectLabel);
}

void AboutUs::addAvatarsAndShortInfo()
{
    teamMembers = {
        {"Nguyễn Khang Hy", "24127280", "UI Designer &", "khanghy0604@gmail.com",
         "https://github.com/nguyenkhanghy", "https://facebook.com/khanghy204", "hy.png"},
        {"Lê Đinh Nguyên Phong", "24127215", "System Developer & Socket Communication", "phong.opg@gmail.com",
         "https://github.com/46thanft", "https://www.facebook.com/phong.opg.2025", "phong.png"},
        {"Phạm Hoàng Phúc", "24127502", "Process Control & Gmail Integration Feature", "phphuc2417@clc.fitus.edu.vn",
         "https://github.com/hoangfish", "https://www.facebook.com/fish.hoang.2025", "phuc.png"}};

    QHBoxLayout *rowLayout = new QHBoxLayout();
    rowLayout->setSpacing(40);
    rowLayout->setAlignment(Qt::AlignCenter);

    for (int i = 0; i < teamMembers.size(); ++i)
    {
        const auto &member = teamMembers[i];

        QVBoxLayout *infoLayout = new QVBoxLayout();
        infoLayout->setAlignment(Qt::AlignCenter);

        QPushButton *avatarBtn = new QPushButton();
        QPixmap pixmap(imagePath(member.avatar));
        avatarBtn->setIcon(QIcon(pixmap));
        avatarBtn->setIconSize(QSize(100, 100));
        avatarBtn->setFixedSize(110, 110);
        avatarBtn->setStyleSheet("border: none;");
        connect(avatarBtn, &QPushButton::clicked, this, [this, i]()
                { scrollToCard(i); });

        QLabel *idLabel = new QLabel(member.id);
        idLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #222;");
        idLabel->setAlignment(Qt::AlignCenter);

        QLabel *nameLabel = new QLabel(member.name);
        nameLabel->setStyleSheet("font-size: 13px; color: #333;");
        nameLabel->setAlignment(Qt::AlignCenter);

        QString githubUser = member.github.section('/', -1);
        QLabel *githubLabel = new QLabel(QString("GitHub: <a href='%1'>%2</a>").arg(member.github, githubUser));
        githubLabel->setTextFormat(Qt::RichText);
        githubLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        githubLabel->setOpenExternalLinks(true);
        githubLabel->setStyleSheet("font-size: 12px; color: #1a73e8;");
        githubLabel->setAlignment(Qt::AlignCenter);

        infoLayout->addWidget(avatarBtn);
        infoLayout->addSpacing(5);
        infoLayout->addWidget(idLabel);
        infoLayout->addWidget(nameLabel);
        infoLayout->addWidget(githubLabel);

        QWidget *infoWidget = new QWidget();
        infoWidget->setLayout(infoLayout);
        rowLayout->addWidget(infoWidget);
    }

    mainLayout->addLayout(rowLayout);
}

void AboutUs::scrollToCard(int index)
{
    if (index < memberCards.size())
    {
        QWidget *card = memberCards[index];
        int y = card->y();
        scrollArea->verticalScrollBar()->setValue(y);
    }
}

QFrame *AboutUs::createMemberCard(const TeamMember &member)
{
    QFrame *card = new QFrame();
    card->setStyleSheet("background-color: white; border-radius: 12px; padding: 16px; border: 1px solid #e0e0e0;");
    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setSpacing(8);

    // Label helper
    auto addLabel = [&](const QString &text)
    {
        QLabel *label = new QLabel(text);
        label->setStyleSheet("font-size: 13px; color: #444;");
        return label;
    };

    // Name
    QLabel *nameLabel = new QLabel("👤 Name: " + member.name);
    nameLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #222;");
    layout->addWidget(nameLabel);

    // Student ID & Role
    layout->addWidget(addLabel("🎓 Student ID: " + member.id));
    layout->addWidget(addLabel("🧩 Role: " + member.role));

    // Email (icon + label + clickable link)
    QLabel *emailLabel = new QLabel();
    emailLabel->setTextFormat(Qt::RichText);
    emailLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    emailLabel->setOpenExternalLinks(true);
    emailLabel->setStyleSheet("font-size: 13px; color: #444;");
    emailLabel->setText(QString(
                            "<img src='%1' width='16' height='16' style='vertical-align:middle; margin-right:4px;'> "
                            "<span style='font-weight:normal;'>Email:</span> "
                            "<a href='mailto:%2'>%2</a>")
                            .arg(imagePath("gmail.png").replace("\\", "/"), member.email));
    layout->addWidget(emailLabel);

    // GitHub row with icon + label + link
    QLabel *githubLabel = new QLabel();
    githubLabel->setTextFormat(Qt::RichText);
    githubLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    githubLabel->setOpenExternalLinks(true);
    githubLabel->setStyleSheet("font-size: 13px; color: #444;");
    githubLabel->setText(QString(
                             "<img src='%1' width='16' height='16' style='vertical-align:middle; margin-right:4px;'> "
                             "<span style='font-weight:normal;'>GitHub:</span> "
                             "<a href='%2'>%2</a>")
                             .arg(imagePath("github.png").replace("\\", "/"), member.github));
    layout->addWidget(githubLabel);

    // Facebook row with icon + label + link
    QLabel *facebookLabel = new QLabel();
    facebookLabel->setTextFormat(Qt::RichText);
    facebookLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    facebookLabel->setOpenExternalLinks(true);
    facebookLabel->setStyleSheet("font-size: 13px; color: #444;");
    facebookLabel->setText(QString(
                               "<img src='%1' width='16' height='16' style='vertical-align:middle; margin-right:4px;'> "
                               "<span style='font-weight:normal;'>Facebook:</span> "
                               "<a href='%2'>%2</a>")
                               .arg(imagePath("facebook.png").replace("\\", "/"), member.facebook));
    layout->addWidget(facebookLabel);

    return card;
}

void AboutUs::addFooter()
{
    QLabel *footer = new QLabel();
    footer->setAlignment(Qt::AlignCenter);
    footer->setText("<b>🎓 Course:</b> Computer Networks – HCMUS<br>"
                    "<b>📅 Semester:</b> 2025 - Term 3<br>"
                    "<b>👨‍🏫 Class:</b> 24C06");
    footer->setStyleSheet("font-size: 13px; color: #444; padding: 10px;");
    mainLayout->addWidget(footer);
}
