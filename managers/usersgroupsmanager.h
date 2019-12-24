/*
 *	File:			(Header) usersgroupsmanager.h
 *  Comment:        MANAGERS FILE
 *	Description:    Class of the dialog where user links the students and the groups
 *
 */

#ifndef USERSGROUPSMANAGER_H
#define USERSGROUPSMANAGER_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include <QQueue>
#include <QListWidget>
#include <QListWidgetItem>
#include "storedData/student.h"
#include "storedData/group.h"
#include "tools/notepad.h"

namespace Ui {
class UsersGroupsManager;
}

class UsersGroupsManager : public QDialog
{
    Q_OBJECT

public:
    explicit UsersGroupsManager(QSqlDatabase *db, QWidget *parent = 0);
    ~UsersGroupsManager();
    int nbStudentsInGroup(int idGroup); // To get the number of students in a group
    enum { // Enumeration of methods to link users and groups
        BrowseStudents, BrowseGroups
    };

public slots:
    bool update_list_browse(); // To update the lists (browsing and linking)
    bool update_listsYesNo(); // To update the linking lists (lists YES and NO)
    bool add_elements(); // To link groups with groups with a student or students with a group
    bool remove_elements(); // To remove a link between groups and students

private:
    Ui::UsersGroupsManager *ui; // GUI
    QSqlDatabase *m_db; // SQL database
    QList<qulonglong> *m_listStudents; // Students list
    QList<qulonglong> *m_listGroups; // Group list
    QAction* m_shortcutNotepad;
};

#endif // USERSGROUPSMANAGER_H
