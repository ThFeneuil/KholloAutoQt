#ifndef UPDATEEVENTDIALOG_H
#define UPDATEEVENTDIALOG_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include <QQueue>
#include <QListWidget>
#include <QListWidgetItem>
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

private:
    Ui::UpdateEventDialog *ui;
    QSqlDatabase *m_db;
    Event *m_event;
    QQueue<Group*> queue_groups; // Displayed group in the comboBox
};

#endif // UPDATEEVENTDIALOG_H
