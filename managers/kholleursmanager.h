#ifndef KHOLLEURSMANAGER_H
#define KHOLLEURSMANAGER_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include <QQueue>
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
    bool free_teachers();
    bool free_kholleurs();

public slots:
    // Teachers
    bool update_listTeachers();
    bool add_teacher();
    bool update_teacher();
    bool delete_teacher();
    // Kholleurs
    bool update_listKholleurs();
    bool add_kholleur();
    bool update_kholleur();
    bool delete_kholleur();

private:
    Ui::KholleursManager *ui;
    QSqlDatabase *m_db;
    QQueue<Teacher*> queue_displayedTeachers;
    QQueue<Kholleur*> queue_displayedKholleurs;
};

#endif // KHOLLEURSMANAGER_H
