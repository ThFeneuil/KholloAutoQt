#ifndef TIMESLOTSMANAGER_H
#define TIMESLOTSMANAGER_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include <QQueue>
#include <QListWidget>
#include <QListWidgetItem>
#include <QGridLayout>
#include <QStringList>
#include "storedData/kholleur.h"
#include "storedData/timeslot.h"
#include "managers/copytimeslots.h"
#include "managers/mergekholleursmanager.h"
#include "tools/notepad.h"
#include "tools/onlinedatabase.h"
#include "settingsdialog.h"

namespace Ui {
class TimeslotsManager;
}
/*
struct KholleurWithName {
    Kholleur* khll;
    QString name;
};*/

class TimeslotsManager : public QDialog
{
    Q_OBJECT

public:
    explicit TimeslotsManager(QSqlDatabase *db, QDate date, QWidget *parent = 0);
    ~TimeslotsManager();
    void getKholleurs();
    void freeKholleurs();
    void update_list_timeslots(int id_kholleur);
    void update_list_oftenTimeslots(int id_kholleur);
    void free_timeslots();
    void displayNameClass();

public slots:
    void onSelection_change();
    void deleteTimeslot(QListWidgetItem* item = NULL);
    void addTimeslot();
    void addOftenTimeslot(QListWidgetItem* item);
    void copyTimeslots();
    void copyAllTimeslots();
    void downloadTimeslots();
    void downloadedTimeslots(ODBRequest *req);
    void saveNameClass();

private:
    Ui::TimeslotsManager *ui;
    QSqlDatabase *m_db;
    QDate m_date;
    QQueue<Kholleur*> queue_displayedKholleurs;
    QQueue<Timeslot*> queue_displayedTimeslots;
    QStringList days;
    QAction* m_shortcutNotepad;
};

#endif // TIMESLOTSMANAGER_H
