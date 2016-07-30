#ifndef KHOLLOSCOPEWIZARD_H
#define KHOLLOSCOPEWIZARD_H

#include <QWizard>
#include <QtSql>
#include <QMessageBox>
#include <QQueue>
#include <QMap>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDate>
#include "managers/KholloscopeWizardPages/subjectspage.h"
#include "managers/KholloscopeWizardPages/userspage.h"
#include "managers/KholloscopeWizardPages/generatepage.h"
#include "storedData/subject.h"
#include "storedData/student.h"

namespace Ui {
class KholloscopeWizard;
}

class KholloscopeWizard : public QWizard
{
    Q_OBJECT

public:
    explicit KholloscopeWizard(QSqlDatabase *db, QWidget *parent = 0);
    ~KholloscopeWizard();

    QList<Subject *> *get_assoc_subjects();
    void set_assoc_subjects(QList<Subject*> *list);

    void load_students();
    void free_students();
    QList<Student*> *get_students();

    void load_subjects();
    void free_subjects();
    QList<Subject*> *get_subjects();

    void set_input(QMap<int, QList<Student*> > *input);
    QMap<int, QList<Student*> > *get_input();

private:
    Ui::KholloscopeWizard *ui;
    QSqlDatabase *m_db;
    QList<Subject*> *assoc_subjects;
    QList<Student*> *m_students;
    QList<Subject*> *m_subjects;
    QMap<int, QList<Student*> > *m_input;
};

#endif // KHOLLOSCOPEWIZARD_H
