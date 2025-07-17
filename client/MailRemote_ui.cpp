#include "MailRemote_ui.h"
MailRemoteUI::MailRemoteUI(QWidget *parent) : QWidget(parent)
{
    // Không gọi setupUI() ở đây, để MailRemote.cpp kiểm soát
}

void MailRemoteUI::setupUI()
{
    this->setAutoFillBackground(true); // Đảm bảo nền được vẽ
    this->setStyleSheet("background-color: #2c3e50;");

    // Nút Back
    btnBack = new QPushButton(QString::fromUtf8("⬅ Back"), this);
    btnBack->setFixedSize(100, 40);
    btnBack->setStyleSheet(
        "QPushButton { background-color: #e74c3c; color: white; font-weight: bold; border-radius: 8px; }"
        "QPushButton:hover { background-color: #c0392b; }"
    );

    // Nút Choose Mail Remote
    btnChooseMail = new QPushButton(QString::fromUtf8("📧 Choose Mail Remote"), this);
    btnChooseMail->setFixedSize(200, 40);
    btnChooseMail->setStyleSheet(
        "QPushButton { background-color: #27ae60; color: white; font-weight: bold; border-radius: 8px; }"
        "QPushButton:hover { background-color: #1e8449; }"
    );

    // Layout lưới chứa hai nút
    QGridLayout *topButtonLayout = new QGridLayout();
    topButtonLayout->addWidget(btnBack, 0, 0, Qt::AlignLeft);
    topButtonLayout->addWidget(btnChooseMail, 0, 1, Qt::AlignHCenter);
    topButtonLayout->setColumnStretch(0, 1);
    topButtonLayout->setColumnStretch(1, 1);
    topButtonLayout->setColumnStretch(2, 1);

    // Label Mail Requests
    labelRequests = new QLabel(QString::fromUtf8("📬 Mail Requests:"), this);
    labelRequests->setStyleSheet("color: white; font: bold 14px 'Segoe UI';");
    labelRequests->setAlignment(Qt::AlignLeft);

    // TextEdit Mail Requests
    textRequests = new QTextEdit(this);
    textRequests->setStyleSheet(
        "QTextEdit { background-color: #ecf0f1; border: 2px solid #bdc3c7;"
        "border-radius: 10px; padding: 10px; font: 13px 'Consolas'; color: black; }"
    );
    textRequests->setMinimumHeight(230);

    // Label Results
    labelResults = new QLabel(QString::fromUtf8("⚙️ Processing Results:"), this);
    labelResults->setStyleSheet("color: white; font: bold 14px 'Segoe UI';");
    labelResults->setAlignment(Qt::AlignLeft);

    // TextEdit Results
    textResults = new QTextEdit(this);
    textResults->setStyleSheet(
        "QTextEdit { background-color: #ecf0f1; border: 2px solid #bdc3c7;"
        "border-radius: 10px; padding: 10px; font: 13px 'Consolas'; color: black; }"
    );
    textResults->setMinimumHeight(230);

    // Layout chính
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    mainLayout->addLayout(topButtonLayout);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(labelRequests);
    mainLayout->addWidget(textRequests);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(labelResults);
    mainLayout->addWidget(textResults);
}