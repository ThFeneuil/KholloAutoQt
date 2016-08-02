#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
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
#include "aboutitdialog.h"
#include "contactdialog.h"
#include "reviewdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QString addSlashes(QString str);

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
	void openKholloscope();
    void openReview();
    void openHelp();
    void openAboutIt();

    void createKhollo();

    void loadDB();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
