#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QByteArray>
#include <QTimer>
#include "managers/studentsmanager.h"
#include "managers/groupsmanager.h"
#include "managers/subjectsmanager.h"
#include "managers/kholleursmanager.h"
#include "managers/usersgroupsmanager.h"
#include "managers/coursesmanager.h"
#include "managers/introtimeslots.h"
#include "managers/eventsmanager.h"
#include "managers/kholloscopewizard.h"
#include "interface/interfacedialog.h"
#include "interface/introinterface.h"
#include "managers/lastchanges.h"
#include "aboutitdialog.h"
#include "contactdialog.h"
#include "reviewdialog.h"
#include "kscopemanager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void openStudentsManager();
    void openGroupsManager();
    void openSubjectsManager();
    void openKholleursManager();
    void openUsersGroupsManager();
    void openCoursesManager();
    void openTimeslotsManager();
    void openEventsManager();
    void openInterface();
    void openInterfaceWithDate(QDate date, int id_week);
	void openKholloscope();
    void openReview();
    void openLastChanges();
    void openHelp();
    void openAboutIt();

    void createKhollo();
    void openKhollo();
    void updateWindow();
    void record(bool start = true);
    void updateRecord();

signals:
    void triggerInterface(QDate date, int id_week);

private:
    Ui::MainWindow *ui;
    KScopeManager kscopemanager;
    QTimer* m_timer;
    int m_idRecord;
};

#endif // MAINWINDOW_H
