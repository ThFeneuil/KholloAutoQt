#ifndef KHOLLEURSMANAGER_H
#define KHOLLEURSMANAGER_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include <QQueue>
#include "managers/updateteacherdialog.h"
#include "managers/updatekholleurdialog.h"
#include "storedData/teacher.h"
#include "storedData/kholleur.h"

namespace Ui {
class KholleursManager;
}

class KholleursManager : public QDialog
{
    Q_OBJECT

public:
    explicit KholleursManager(QSqlDatabase *db, QWidget *parent = 0);
    ~KholleursManager();
    bool free_teachers(); // Free memories with the teachers
    bool free_kholleurs(); // Free memories with the kholleurs

public slots:
    // Teachers
    bool update_listTeachers();
    bool add_teacher();
    bool update_teacher();
    bool delete_teacher();
    // Kholleurs
    bool update_listKholleurs(int idSelected = 0);
    bool add_kholleur();
    bool update_kholleur();
    bool delete_kholleur();

private:
    Ui::KholleursManager *ui;
    QSqlDatabase *m_db;
    QQueue<Teacher*> queue_displayedTeachers;  // Displayed teachers in the list "Teachers"
    QQueue<Kholleur*> queue_displayedKholleurs; // Displayed teachers in the list "Kholleurs"
};

#endif // KHOLLEURSMANAGER_H
