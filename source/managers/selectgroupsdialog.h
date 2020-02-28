#ifndef SELECTGROUPSDIALOG_H
#define SELECTGROUPSDIALOG_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QQueue>
#include <QMap>
#include "storedData/group.h"

namespace Ui {
class SelectGroupsDialog;
}

class SelectGroupsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectGroupsDialog(QSqlDatabase *db, QList<Group*> *list, QWidget *parent = 0);
    ~SelectGroupsDialog();

public slots:
    bool update_lists();
    bool add_group();
    bool remove_group();
    bool return_groups();

private:
    Ui::SelectGroupsDialog *ui;
    QSqlDatabase *m_db;
    QList<Group*> *m_list;
    QList<Group*> *m_listAllGroups;
    QMap<int, bool> *m_isSelected;
};

#endif // SELECTGROUPSDIALOG_H
