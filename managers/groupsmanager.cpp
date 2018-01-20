#include "managers/groupsmanager.h"
#include "ui_groupsmanager.h"

GroupsManager::GroupsManager(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GroupsManager)
{
    // Apply the design of the dialog box
    ui->setupUi(this);
    connect(ui->pushButton_add, SIGNAL(clicked()), this, SLOT(add_group()));
    connect(ui->pushButton_delete, SIGNAL(clicked()), this, SLOT(delete_group()));

    // DB
    m_db = db;
    update_list();
}

GroupsManager::~GroupsManager() {
    delete ui;
    free_groups();
}

bool GroupsManager::free_groups() {
    while (!queue_displayedGroups.isEmpty())
        delete queue_displayedGroups.dequeue();
    return true;
}

bool GroupsManager::update_list() {
    // Clear the list
    ui->list_groups->clear();
    free_groups();

    // Make the request
    queue_displayedGroups = GroupsDBInterface(m_db).load("ORDER BY UPPER(name)");

    // Treat the request
    for (Group *grp : queue_displayedGroups) {
        QListWidgetItem *item = new QListWidgetItem(grp->getName(), ui->list_groups);
        item->setData(Qt::UserRole, (qulonglong) grp);
    }

    return true;
}

bool GroupsManager::add_group() {
    QString name = ui->lineEdit_name->text();

    if(name == "") {
        QMessageBox::critical(this, "Erreur", "Il faut renseigner le nom du groupe.");
        return false;
    } else {
        Group *g = new Group();
        g->setName(name);
        GroupsDBInterface(m_db).insert(g);
        delete g;

        ui->lineEdit_name->clear();

        update_list();
    }

    return true;
}

bool GroupsManager::delete_group() {
    QListWidgetItem *item = ui->list_groups->currentItem();

    if(item == NULL) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un groupe.");
        return false;
    } else {
        Group* grp = (Group*) item->data(Qt::UserRole).toULongLong();
        int res = QMessageBox::warning(this, "Suppression en cours",
                "Vous êtes sur le point de supprimer le groupe <strong>\"" + grp->getName() + "\"</strong> ainsi que les <strong>cours</strong> associés.<br /> Voulez-vous continuer ?",
                QMessageBox::Yes | QMessageBox::Cancel);
        if(res == QMessageBox::Yes) {
            m_db->transaction();
            GroupsDBInterface(m_db).remove(grp->getId());
            m_db->commit();

            update_list();
        }
    }

    return true;
}
