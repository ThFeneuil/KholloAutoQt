#include "selectgroupsdialog.h"
#include "ui_selectgroupsdialog.h"

SelectGroupsDialog::SelectGroupsDialog(QSqlDatabase *db, QList<Group*> *list, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectGroupsDialog)
{
    // Initialisation
    ui->setupUi(this);
    m_db = db;
    m_list = list;
    m_listAllGroups = new QList<Group*>();
    m_isSelected = new QMap<int,bool>;

    // Connect the buttons
    connect(ui->pushButton_add, SIGNAL(clicked(bool)), this, SLOT(add_group()));
    connect(ui->pushButton_remove, SIGNAL(clicked(bool)), this, SLOT(remove_group()));
    connect(ui->pushButton_valid, SIGNAL(clicked(bool)), this, SLOT(return_groups()));

    // Get the list of all the groups
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name FROM tau_groups ORDER BY name");
    while (query.next()) {
        Group* grp = new Group();
        grp->setId(query.value(0).toInt());
        grp->setName(query.value(1).toString());
        m_isSelected->insert(grp->getId(), false);
        m_listAllGroups->append(grp);
    }

    // Save the list of the groups selected
    for (int i = 0; i < m_list->size(); ++i) {
        int id = m_list->at(i)->getId();
        if( m_isSelected->contains(id) )
            (*m_isSelected)[id] = true;
    }

    // Update the widgets
    update_lists();
}

SelectGroupsDialog::~SelectGroupsDialog() {
    delete ui;
    // Free memories
    if(result() == QDialog::Rejected) {
        for (int i = 0; i < m_listAllGroups->size(); ++i)
            delete (*m_listAllGroups)[i];
    }
    free(m_listAllGroups);
    free(m_isSelected);
}

bool SelectGroupsDialog::update_lists() {
    /** Display the groups in the different lists **/
    // Clear the lists
    ui->list_yes->clear();
    ui->list_no->clear();

    for(int i = 0; i < m_listAllGroups->size(); ++i) {
        // Get the group
        Group* grp = m_listAllGroups->at(i);
        if((*m_isSelected)[grp->getId()]) {
            // If the group is selected, then insert it in the list YES
            QListWidgetItem *item = new QListWidgetItem(grp->getName(), ui->list_yes);
            item->setData(Qt::UserRole, (qulonglong) grp);
        } else {
            // Else, insert it in the list NO
            QListWidgetItem *item = new QListWidgetItem(grp->getName(), ui->list_no);
            item->setData(Qt::UserRole, (qulonglong) grp);
        }
    }

    return true;
}

bool SelectGroupsDialog::add_group() {
    /** Select a new group **/
    QListWidgetItem *item = ui->list_no->currentItem();

    if(item == NULL) { // If no groupe is selected
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un groupe.");
        return false;
    } else {
        // Update the list of the selected groups
        Group* grp = (Group*) item->data(Qt::UserRole).toULongLong();
        (*m_isSelected)[grp->getId()] = true;
        // Update the widgets
        update_lists();
    }

    return true;
}

bool SelectGroupsDialog::remove_group() {
    /** Remove a group from the list of selected groups **/
    QListWidgetItem *item = ui->list_yes->currentItem();

    if(item == NULL) { // If no groupe is selected
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un groupe.");
        return false;
    } else {
        // Update the list of the selected groups
        Group* grp = (Group*) item->data(Qt::UserRole).toULongLong();
        (*m_isSelected)[grp->getId()] = false;
        // Update the widgets
        update_lists();
    }

    return true;
}

bool SelectGroupsDialog::return_groups() {
    /** Save the selected groups in the list of selected groups **/
    // Remove the old groups
    for(int i=0; i<m_list->size(); ++i)
        delete (*m_list)[i];
    m_list->clear();

    // Add the new groups
    for(int i=0; i<m_listAllGroups->size(); ++i) {
        // Get the ID of the group
        int id = m_listAllGroups->at(i)->getId();
        // If selected, then insert in the list
        if( m_isSelected->contains(id) && (*m_isSelected)[id] == true)
                m_list->append(m_listAllGroups->at(i));
        else    delete (*m_listAllGroups)[i]; // Else free memory
    }

    // Close the window
    accept();

    return true;
}
