/*
 *	File:			(Source) usersgroupsmanager.cpp
 *  Comment:        MANAGERS FILE
 *	Description:    Class of the dialog where user links the students and the groups
 *
 */

#include "UsersGroupsManager.h"
#include "ui_usersgroupsmanager.h"

UsersGroupsManager::UsersGroupsManager(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UsersGroupsManager)
{   /** CONSTRUCTOR **/

    /// Initialisation of the class properties
    ui->setupUi(this); // GUI
    m_db = db;
    m_listStudents = new QList<qulonglong>();
    m_listGroups = new QList<qulonglong>();
    m_shortcutNotepad = Notepad::shortcut();
    this->addAction(m_shortcutNotepad);

    /// Get the list of the students
    QList<Student *> sdts = StudentsDBInterface(m_db).load("ORDER BY UPPER(`name`), UPPER(`first_name`)");
    for (Student *stdnt : sdts) {
        // Add the student in a list
        m_listStudents->append((qulonglong) stdnt);
    }
    /// Get the list of the groups
    QList<Group *> grps = GroupsDBInterface(m_db).load("ORDER BY UPPER(`name`)");
    for (Group *grp : grps) {
        // Add the group in a list
        m_listGroups->append((qulonglong) grp);
    }

    /// Connect to detect when user click on an item or on a button
    connect(ui->comboBox_browse, SIGNAL(currentIndexChanged(int)), this, SLOT(update_list_browse()));
    connect(ui->list_browse, SIGNAL(itemSelectionChanged()), this, SLOT(update_listsYesNo()));
    connect(ui->pushButton_add, SIGNAL(clicked()), this, SLOT(add_elements()));
    connect(ui->pushButton_remove, SIGNAL(clicked()), this, SLOT(remove_elements()));

    /// Fill the combobox which selects the method to link
    ui->comboBox_browse->addItem("A partir des élèves", BrowseStudents);
    ui->comboBox_browse->addItem("A partir des groupes", BrowseGroups);

    /// Update the lists
    update_list_browse();
}

UsersGroupsManager::~UsersGroupsManager() {
    /** DESTRUCTOR **/
    delete ui; // GUI
    // Clear the lists (students and groups)
    for(int i=m_listStudents->count()-1; i>=0; i--)
        delete ((Student*) m_listStudents->at(i));
    for(int i=m_listGroups->count()-1; i>=0; i--)
        delete ((Group*) m_listGroups->at(i));
    // Delete the list (students and groups)
    delete m_listStudents;
    delete m_listGroups;
    this->removeAction(m_shortcutNotepad);
    delete m_shortcutNotepad;
}

bool UsersGroupsManager::update_list_browse() {
    /** METHOD TO UPDATE THE LISTS (BROWSING AND LINKING) **/
    // Clear the list
    ui->list_browse->clear();

    /// Adapt the variables and labels according the chosen method to link students and groups
    bool browseStd = (ui->comboBox_browse->currentData().toInt() == BrowseStudents);
    QList<qulonglong>* listToTreat = NULL;
    if(browseStd) {
        // If the browsing is with students
        listToTreat = m_listStudents;
        ui->label_browse->setText("Elèves");
        ui->label_yes->setText("Groupes de l'élève");
        ui->label_no->setText("Autres groupes");
    }else {
        // If the browsing is with groups
        listToTreat = m_listGroups;
        ui->label_browse->setText("Groupes");
        ui->label_yes->setText("Elèves du groupe");
        ui->label_no->setText("Autres élèves");
    }

    /// Fill the browsing list
    for(int i=0; i<listToTreat->count(); i++) {
        // Get the text for the item
        QString text = "";
        if(browseStd) {
                Student* std = (Student*) listToTreat->at(i);
                text = std->getName() + " " + std->getFirst_name();
        } else    text = ((Group*) listToTreat->at(i))->getName() + " (" + QString::number(nbStudentsInGroup(((Group*) listToTreat->at(i))->getId()))+ ")";
        // Create the item with the element
        QListWidgetItem *item = new QListWidgetItem(text, ui->list_browse);
        item->setData(Qt::UserRole, (qulonglong) listToTreat->at(i));
    }

    /// Update the linking lists (lists YES and NO)
    update_listsYesNo();

    return true;
}

bool UsersGroupsManager::update_listsYesNo() {
    /** METHOD TO UPDATE THE LINKING LISTS (LISTS YES AND NO) **/
    // Clear the lists
    ui->list_no->clear();
    ui->list_yes->clear();

    //Get current selection in browsing list
    QListWidgetItem *item = ui->list_browse->currentItem();
    bool browseStd = (ui->comboBox_browse->currentData().toInt() == BrowseStudents);

    //Disable the lists if no browing element is selected
    if(! item) {
        ui->list_no->setEnabled(false);
        ui->list_yes->setEnabled(false);
        return false;
    }

    //Enable the linking lists
    ui->list_no->setEnabled(true);
    ui->list_yes->setEnabled(true);

    /// Fill the linking lists
    if(browseStd) {
        // If the linking lists contain groups
        Student *student = (Student*) item->data(Qt::UserRole).toULongLong();

        // Get groups of student
        QList<StudentGroupLink*> student_gr = StudentGroupLinksDBInterface(m_db).load("WHERE `id_users`=" + QString::number(student->getId()));

        // Create a map to identify quickly if the selected student is in a group
        QMap<int, bool> map;
        for(StudentGroupLink *link : student_gr)
            map.insert(link->getId_groups(), true);

        //Put every group into the right list
        for(int i=0; i<m_listGroups->count(); i++) {
            QListWidgetItem *item = new QListWidgetItem(((Group*) m_listGroups->at(i))->getName() + " ("+ QString::number(nbStudentsInGroup(((Group*) m_listGroups->at(i))->getId())) +")");
            item->setData(Qt::UserRole, m_listGroups->at(i));

            // Use the map to identify the right list
            if(map.contains(((Group*) m_listGroups->at(i))->getId()))
                    ui->list_yes->addItem(item);
            else    ui->list_no->addItem(item);
        }
    } else {
        // If the linking lists contain students
        Group *grp = (Group*) item->data(Qt::UserRole).toULongLong();

        //Get students of group
        QList<StudentGroupLink*> group_std = StudentGroupLinksDBInterface(m_db).load("WHERE id_groups=" + QString::number(grp->getId()));

        // Create a map to identify quickly if a student is in the selected group
        QMap<int, bool> map;
        for(StudentGroupLink *link : group_std)
            map.insert(link->getId_students(), true);

        //Put every student into the right list
        for(int i=0; i<m_listStudents->count(); i++) {
            QListWidgetItem *item = new QListWidgetItem(((Student*) m_listStudents->at(i))->getName() + " " + ((Student*) m_listStudents->at(i))->getFirst_name());
            item->setData(Qt::UserRole, m_listStudents->at(i));

            // Use the map to identify the right list
            if(map.contains(((Student*) m_listStudents->at(i))->getId()))
                    ui->list_yes->addItem(item);
            else    ui->list_no->addItem(item);
        }
    }

    return true;
}

bool UsersGroupsManager::add_elements() {
    /** METHOD TO LINK GROUPS WITH A STUDENT OR STUDENTS WITH A GROUP **/
    //Get current selection
    QList<QListWidgetItem*> selection = ui->list_no->selectedItems();
    bool browseStd = (ui->comboBox_browse->currentData().toInt() == BrowseStudents);

    //Error if no group selected
    if(selection.length() <= 0) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un item.");
        return false;
    }

    //Use interface to insert items
    m_db->transaction();
    StudentGroupLinksDBInterface inter(m_db);

    /// Get selected elements in linking lists
    for(int i=0; i<selection.count(); i++) {
        QListWidgetItem* itemFromNo = selection.at(i);
        QListWidgetItem* itemFromBrowse = ui->list_browse->currentItem();

        // Identify the items according the selected method to link
        Group* group = NULL;
        Student* student = NULL;
        if(browseStd) {
            group = (Group*) itemFromNo->data(Qt::UserRole).toULongLong();
            student = (Student*) itemFromBrowse->data(Qt::UserRole).toULongLong();
        } else {
            group = (Group*) itemFromBrowse->data(Qt::UserRole).toULongLong();
            student = (Student*) itemFromNo->data(Qt::UserRole).toULongLong();
        }

        // Add the link
        StudentGroupLink *link = new StudentGroupLink();
        link->setId_groups(group->getId());
        link->setId_students(student->getId());
        inter.insert(link);
        delete link;
    }

    m_db->commit();

    /// Update the linking lists
    update_listsYesNo();
    // Update the selected item in the browsing list
    QListWidgetItem* item = ui->list_browse->currentItem();
    if(item && !browseStd){
        // If the selected item is a group --> change the number of students
        Group* grp = (Group*) item->data(Qt::UserRole).toLongLong();
        item->setText(grp->getName() + " ("+QString::number(nbStudentsInGroup(grp->getId()))+")");
    }

    return true;
}

bool UsersGroupsManager::remove_elements() {
    /** METHOD TO REMOVE A LINK BETWEEN GROUPS AND STUDENTS **/
    //Get current selection
    QList<QListWidgetItem*> selection = ui->list_yes->selectedItems();
    bool browseStd = (ui->comboBox_browse->currentData().toInt() == BrowseStudents);

    //Error if no group selected
    if(selection.length() <= 0) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un item.");
        return false;
    }

    //Use interface to delete links
    m_db->transaction();
    StudentGroupLinksDBInterface inter(m_db);

    /// Get selected elements in linking lists
    for(int i=0; i<selection.count(); i++) {
        QListWidgetItem* itemFromYes = selection.at(i);
        QListWidgetItem* itemFromBrowse = ui->list_browse->currentItem();

        // Identify the items according the selected method to link
        Group* group = NULL;
        Student* student = NULL;
        if(browseStd) {
            group = (Group*) itemFromYes->data(Qt::UserRole).toULongLong();
            student = (Student*) itemFromBrowse->data(Qt::UserRole).toULongLong();
        } else {
            group = (Group*) itemFromBrowse->data(Qt::UserRole).toULongLong();
            student = (Student*) itemFromYes->data(Qt::UserRole).toULongLong();
        }

        // Get the link for this couple
        QList<StudentGroupLink *> to_delete = inter.load("WHERE id_groups = " + QString::number(group->getId())
                                                         + " AND id_users = " + QString::number(student->getId()));
        // Delete the link
        for(StudentGroupLink *link : to_delete)
            inter.remove(link->getId());
    }
    m_db->commit();

    /// Update the linking lists
    update_listsYesNo();
    // Update the selected item in the browsing list
    QListWidgetItem* item = ui->list_browse->currentItem();
    if(item && !browseStd){
        // If the selected item is a group --> change the number of students
        Group* grp = (Group*) item->data(Qt::UserRole).toLongLong();
        item->setText(grp->getName() + " ("+QString::number(nbStudentsInGroup(grp->getId()))+")");
    }

    return true;
}

int UsersGroupsManager::nbStudentsInGroup(int idGroup) {
    /** METHOD TO GET THE NUMBER OF STUDENTS IN A GROUP **/
    // Make the query to get the number of students in the group
    QSqlQuery query(*m_db);
    query.prepare("SELECT COUNT(*) FROM tau_groups_users WHERE id_groups = :id_groups");
    query.bindValue(":id_groups", idGroup);
    query.exec();

    // Return the number (-1: error)
    if(query.next())
        return query.value(0).toInt();
    else
        return -1;
}
