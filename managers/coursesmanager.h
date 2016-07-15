#ifndef COURSESMANAGER_H
#define COURSESMANAGER_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include <QQueue>
#include <QListWidget>
#include <QListWidgetItem>
#include <QGridLayout>
#include <QStringList>
#include "storedData/group.h"
#include "storedData/subject.h"
#include "storedData/teacher.h"
#include "storedData/course.h"

namespace Ui {
class CoursesManager;
}

class CoursesManager : public QDialog
{
    Q_OBJECT

public:
    explicit CoursesManager(QSqlDatabase *db, QWidget *parent = 0);
    ~CoursesManager();
    bool get_subjects();
    bool get_teachers();
    bool update_list_groups();
    bool free_groups();
    bool free_subjects();
    bool free_teachers();
    void save(QGridLayout *grid, int week);


public slots:
    bool update_courses(QGridLayout *grid, int week);
    bool save_changes();
    void onSelection_change();
    void courses_changed(int i);

private:
    Ui::CoursesManager *ui;
    QSqlDatabase *m_db;
    QQueue<Group*> queue_displayedGroups;
    QList<Subject*> list_subjects;
    QList<Teacher*> list_teachers;
    bool isChanged;
    Group *current;
    QStringList begin_times;
    QStringList end_times;
};

#endif // COURSESMANAGER_H