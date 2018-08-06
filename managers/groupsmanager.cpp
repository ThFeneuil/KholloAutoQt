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
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name FROM tau_groups ORDER BY UPPER(name)");

    // Treat the request
    while (query.next()) {
        Group* grp = new Group();
        grp->setId(query.value(0).toInt());
        grp->setName(query.value(1).toString());
        QListWidgetItem *item = new QListWidgetItem(grp->getName(), ui->list_groups);
        item->setData(Qt::UserRole, (qulonglong) grp);
        queue_displayedGroups.enqueue(grp);
    }

    return true;
}

bool GroupsManager::add_group() {
    QString name = ui->lineEdit_name->text();

    if(name == "") {
        QMessageBox::critical(this, "Erreur", "Il faut renseigner le nom du groupe.");
        return false;
    } else {
        QSqlQuery query(*m_db);
        query.prepare("INSERT INTO tau_groups(name) VALUES(:name)");
        query.bindValue(":name", name);
        query.exec();

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
            QSqlQuery query(*m_db);
            /*query.prepare("DELETE FROM tau_courses WHERE id_groups=:id_groups");
            query.bindValue(":id_groups", grp->getId());
            query.exec();
            query.prepare("DELETE FROM tau_events_groups WHERE id_groups=:id_groups");
            query.bindValue(":id_groups", grp->getId());
            query.exec();
            query.prepare("DELETE FROM tau_groups_users WHERE id_groups=:id_groups");
            query.bindValue(":id_groups", grp->getId());
            query.exec();*/
            query.prepare("DELETE FROM tau_groups WHERE id=:id");
            query.bindValue(":id", grp->getId());
            query.exec();

            update_list();;
        }
    }

    return true;
}
