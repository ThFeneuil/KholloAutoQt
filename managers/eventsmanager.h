#ifndef EVENTSMANAGER_H
#define EVENTSMANAGER_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include <QQueue>
#include <QListWidget>
#include "managers/updateeventdialog.h"
#include "storedData/event.h"
#include "notepad.h"

namespace Ui {
class EventsManager;
}

class EventsManager : public QDialog
{
    Q_OBJECT

public:
    explicit EventsManager(QSqlDatabase *db, QWidget *parent = 0);
    ~EventsManager();
    bool free_events(); // Free memories with the events

public slots:
    bool update_list();
    bool display_event(QListWidgetItem* item);
    bool add_event();
    bool update_event();
    bool delete_event();

private:
    Ui::EventsManager *ui;
    QSqlDatabase *m_db;
    QQueue<Event*> queue_displayedEvents;  // Displayed events in the list "Events"
    QAction* m_shortcutNotepad;
};

#endif // EVENTSMANAGER_H
