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
    m_listStudents = new QList<Student*>();
    m_listGroups = new QList<Group*>();
    m_shortcutNotepad = Notepad::shortcut();
    this->addAction(m_shortcutNotepad);

    /// Get the list of the students
    QSqlQuery query(*m_db);
    query.exec("SELECT `id`, `name`, `first_name` FROM `tau_users` ORDER BY UPPER(`name`), UPPER(`first_name`)");
    while (query.next()) {
        Student* stdnt = new Student();
        stdnt->setId(query.value(0).toInt());
        stdnt->setName(query.value(1).toString());
        stdnt->setFirst_name(query.value(2).toString());
        // Add the student in a list
        m_listStudents->append(stdnt);
    }
    /// Get the list of the groups
    query.exec("SELECT `id`, `name` FROM `tau_groups` ORDER BY UPPER(`name`)");
    while (query.next()) {
        Group* grp = new Group();
        grp->setId(query.value(0).toInt());
        grp->setName(query.value(1).toString());
        // Add the group in a list
        m_listGroups->append(grp);
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
        delete m_listStudents->at(i);
    for(int i=m_listGroups->count()-1; i>=0; i--)
        delete m_listGroups->at(i);
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
        listToTreat = (QList<qulonglong>*) m_listStudents;
        ui->label_browse->setText("Elèves");
        ui->label_yes->setText("Groupes de l'élève");
        ui->label_no->setText("Autres groupes");
    }else {
        // If the browsing is with groups
        listToTreat = (QList<qulonglong>*) m_listGroups;
        ui->label_browse->setText("Groupes");
        ui->label_yes->setText("Elèves du groupe");
        ui->label_no->setText("Autres élèves");
    }

    /// Fill the browsing list
    for(int i=0; i<listToTreat->count(); i++) {
        // Get the text for the item
        QString text = "";
        if(browseStd)
                text = ((Student*) listToTreat->at(i))->getName() + " " + ((Student*) listToTreat->at(i))->getFirst_name();
        else    text = ((Group*) listToTreat->at(i))->getName() + " (" + QString::number(nbStudentsInGroup(((Group*) listToTreat->at(i))->getId()))+ ")";
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
        QSqlQuery student_gr(*m_db);
        student_gr.prepare("SELECT `id_groups` FROM `tau_groups_users` WHERE `id_users`=:id_users");
        student_gr.bindValue(":id_users", student->getId());
        student_gr.exec();

        // Create a map to identify quickly if the selected student is in a group
        QMap<int, bool> map;
        while(student_gr.next())
            map.insert(student_gr.value(0).toInt(), true);

        //Put every group into the right list
        for(int i=0; i<m_listGroups->count(); i++) {
            QListWidgetItem *item = new QListWidgetItem(m_listGroups->at(i)->getName() + " ("+ QString::number(nbStudentsInGroup(m_listGroups->at(i)->getId())) +")");
            item->setData(Qt::UserRole, (qulonglong) m_listGroups->at(i));

            // Use the map to identify the right list
            if(map.contains(m_listGroups->at(i)->getId()))
                    ui->list_yes->addItem(item);
            else    ui->list_no->addItem(item);
        }
    } else {
        // If the linking lists contain students
        Group *grp = (Group*) item->data(Qt::UserRole).toULongLong();

        //Get students of group
        QSqlQuery group_std(*m_db);
        group_std.prepare("SELECT id_users FROM tau_groups_users WHERE id_groups=:id_groups");
        group_std.bindValue(":id_groups", grp->getId());
        group_std.exec();

        // Create a map to identify quickly if a student is in the selected group
        QMap<int, bool> map;
        while(group_std.next())
            map.insert(group_std.value(0).toInt(), true);

        //Put every student into the right list
        for(int i=0; i<m_listStudents->count(); i++) {
            QListWidgetItem *item = new QListWidgetItem(m_listStudents->at(i)->getName() + " " + m_listStudents->at(i)->getFirst_name());
            item->setData(Qt::UserRole, (qulonglong) m_listStudents->at(i));

            // Use the map to identify the right list
            if(map.contains(m_listStudents->at(i)->getId()))
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

    // Prepare the query (to make only one global query)
    QString queryStr = "INSERT INTO tau_groups_users(id_groups, id_users) VALUES";
    QMap<QString, qulonglong> bindValuesMap;

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

        // Add the link in the variables preparing the query
        if(i) queryStr += ", ";
        QString varGrp = ":idGroup" + QString::number(i), varStd = ":idUser" + QString::number(i);
        queryStr += "("+varGrp+", "+varStd+")";
        bindValuesMap.insert(varGrp, (qulonglong) group->getId());
        bindValuesMap.insert(varStd, (qulonglong) student->getId());
    }

    /// Make the query
    QSqlQuery queryInsert(*m_db);
    queryInsert.prepare(queryStr);
    // Add the bind values
    QMapIterator<QString, qulonglong> i(bindValuesMap);
    while (i.hasNext()) {
        i.next();
        queryInsert.bindValue(i.key(), i.value());
    }
    queryInsert.exec();

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

    // Prepare the query (to make only one global query)
    QString queryStr = "DELETE FROM tau_groups_users WHERE ";
    QMap<QString, qulonglong> bindValuesMap;

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

        // Add the action to remove the link in the variables preparing the query
        if(i) queryStr += " OR ";
        QString varGrp = ":idGroup" + QString::number(i), varStd = ":idUser" + QString::number(i);
        queryStr += "(id_groups = "+varGrp+" AND id_users = "+varStd+")";
        bindValuesMap.insert(varGrp, (qulonglong) group->getId());
        bindValuesMap.insert(varStd, (qulonglong) student->getId());
    }

    /// Make the query
    QSqlQuery queryDelete(*m_db);
    queryDelete.prepare(queryStr);
    // Add the bind values
    QMapIterator<QString, qulonglong> i(bindValuesMap);
    while (i.hasNext()) {
        i.next();
        queryDelete.bindValue(i.key(), i.value());
    }
    queryDelete.exec();

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
