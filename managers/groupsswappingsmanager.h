#ifndef GROUPSSWAPPINGSMANAGER_H
#define GROUPSSWAPPINGSMANAGER_H

#include <QDialog>
#include <QSqlQuery>
#include <QMessageBox>
#include "storedData/group.h"
#include "tools/notepad.h"

namespace Ui {
class GroupsSwappingsManager;
}

class GroupsSwappingsManager : public QDialog
{
    Q_OBJECT

public:
    explicit GroupsSwappingsManager(QSqlDatabase* db, QWidget *parent = 0);
    ~GroupsSwappingsManager();

public slots:
    bool swapGroups();

private:
    Ui::GroupsSwappingsManager *ui;
    QSqlDatabase* m_db;
    QList<Group*>* m_listGroups;
    QAction* m_shortcutNotepad;
};

#endif // GROUPSSWAPPINGSMANAGER_H
