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
	void openKholloscope();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
