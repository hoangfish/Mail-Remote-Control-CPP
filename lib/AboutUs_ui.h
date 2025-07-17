#ifndef ABOUTUS_H
#define ABOUTUS_H

#include <QMainWindow>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QHBoxLayout>
#include <QVector>
#include <QString>
#include <QMap>
#include <QPixmap>
#include <QDesktopServices>
#include <QUrl>
#include <QScrollBar>
#include <QDebug>
struct TeamMember
{
    QString name;
    QString id;
    QString role;
    QString email;
    QString github;
    QString facebook;
    QString avatar;
};

class AboutUs : public QMainWindow
{
    Q_OBJECT

public:
    AboutUs(QWidget *parent = nullptr);
    ~AboutUs();

private slots:
    void handleBackButton();
    void scrollToCard(int index);

private:
    QWidget *scrollWidget;
    QScrollArea *scrollArea;
    QVBoxLayout *mainLayout;
    QVector<QFrame *> memberCards;
    QVector<TeamMember> teamMembers;

    void setupUI();
    void addSchoolLogo();
    void addAvatarsAndShortInfo();
    void addProjectDescription();
    void addFooter();
    QFrame *createMemberCard(const TeamMember &member);
    QString imagePath(const QString &filename);
};

#endif // ABOUTUS_H
