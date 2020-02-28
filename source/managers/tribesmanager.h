#ifndef TRIBESMANAGER_H
#define TRIBESMANAGER_H

#include <QDialog>
#include <QtSql>
#include <QQueue>
#include <QLineEdit>
#include "storedData/subject.h"
#include "storedData/student.h"
#include "tools/notepad.h"

namespace Ui {
class TribesManager;
}

class TribesManager : public QDialog
{
    Q_OBJECT

public:
    explicit TribesManager(QSqlDatabase *db, QWidget *parent = 0);
    ~TribesManager();
    void getSubjects();
    void freeSubjects();
    void getStudents();
    void freeStudents();
    void updateStudentList(int id_subject);


public slots:
    void selectionChanged();
    void associate();
    void studentsChanged();


private:
    Ui::TribesManager *ui;
    QSqlDatabase *m_db;
    QQueue<Subject*> queue_displayedSubjects;
    QList<Student*> list_displayedStudents;
    QMap<int, QString> map_students_tribes;
    QMap<QString, QColor> map_colors;
    QList<QColor> list_colors;
    QAction* m_shortcutNotepad;
};

#endif // TRIBESMANAGER_H
