#ifndef UPDATEEVENTDIALOG_H
#define UPDATEEVENTDIALOG_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include <QQueue>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include "managers/selectgroupsdialog.h"
#include "storedData/event.h"
#include "storedData/group.h"

namespace Ui {
class UpdateEventDialog;
}

class UpdateEventDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateEventDialog(QSqlDatabase *db, Event *event, QWidget *parent = 0);
    ~UpdateEventDialog();

public slots:
    bool update_event();
    bool select_groups();
    bool display_groups();

private:
    Ui::UpdateEventDialog *ui;
    QSqlDatabase *m_db;
    Event *m_event;
    QList<Group*> *m_list_groups;
};

#endif // UPDATEEVENTDIALOG_H
