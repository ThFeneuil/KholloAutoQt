#include "usersgroupsmanager.h"
#include "ui_usersgroupsmanager.h"

usersgroupsmanager::usersgroupsmanager(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::usersgroupsmanager)
{
    ui->setupUi(this);
    connect(ui->list_students, SIGNAL(itemSelectionChanged()), this, SLOT(update_list_groups()));
    connect(ui->pushButton_add, SIGNAL(clicked()), this, SLOT(add_group()));
    connect(ui->pushButton_remove, SIGNAL(clicked()), this, SLOT(remove_group()));

    m_db = db;
    update_list_students();
}

usersgroupsmanager::~usersgroupsmanager()
{
    delete ui;
    free_students();
    free_groups();
}

bool usersgroupsmanager::update_list_students() {
    //Clear the list
    ui->list_students->clear();
    free_students();

    //Send request
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, first_name FROM tau_users ORDER BY name, first_name");

    // Treat the request
    while (query.next()) {
        Student* stdnt = new Student();
        stdnt->setId(query.value(0).toInt());
        stdnt->setName(query.value(1).toString());
        stdnt->setFirst_name(query.value(2).toString());
        QListWidgetItem *item = new QListWidgetItem(stdnt->getName() + ", " + stdnt->getFirst_name(), ui->list_students);
        item->setData(Qt::UserRole, (qulonglong) stdnt);
        queue_displayedStudents.enqueue(stdnt);
    }

    return true;
}

bool usersgroupsmanager::free_students() {
    while(!queue_displayedStudents.isEmpty()) {
        delete queue_displayedStudents.dequeue();
    }
    return true;
}

bool usersgroupsmanager::free_groups() {
    while(!queue_displayedGroups.isEmpty()) {
        delete queue_displayedGroups.dequeue();
    }
    return true;
}

bool usersgroupsmanager::update_list_groups() {
    //Empty the lists
    ui->list_groups_no->clear();
    ui->list_groups_yes->clear();
    free_groups();

    //Get current selection
    QList<QListWidgetItem*> selection = ui->list_students->selectedItems();

    //Disable the lists if no user selected
    if(selection.length() <= 0) {
        ui->list_groups_no->setEnabled(false);
        ui->list_groups_yes->setEnabled(false);
        return false;
    }

    //Enable the lists
    ui->list_groups_no->setEnabled(true);
    ui->list_groups_yes->setEnabled(true);

    QListWidgetItem *item = selection[0];
    Student *student = (Student*) item->data(Qt::UserRole).toULongLong();

    //Get groups of student
    QSqlQuery student_gr(*m_db);
    student_gr.prepare("SELECT id, id_groups FROM tau_groups_users WHERE id_users=:id_users");
    student_gr.bindValue(":id_users", student->getId());
    student_gr.exec();

    QMap<int, bool> map;

    while(student_gr.next()) {
        map.insert(student_gr.value(1).toInt(), true);
    }

    //Get all groups
    QSqlQuery groups(*m_db);
    groups.exec("SELECT id, name FROM tau_groups WHERE is_deleted = 0 ORDER BY name");

    //Put every group into the right place
    while(groups.next()) {
        Group* group = new Group();
        group->setId(groups.value(0).toInt());
        group->setName(groups.value(1).toString());
        QListWidgetItem *item = new QListWidgetItem(group->getName());
        item->setData(Qt::UserRole, (qulonglong) group);
        queue_displayedGroups.enqueue(group);

        if(map.contains(group->getId()))
            ui->list_groups_yes->addItem(item);
        else
            ui->list_groups_no->addItem(item);
    }

    return true;
}

bool usersgroupsmanager::add_group() {
    //Get current selection
    QList<QListWidgetItem*> selection = ui->list_groups_no->selectedItems();

    //Error if no group selected
    if(selection.length() <= 0) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un groupe.");
        return false;
    }

    //Get selected group
    QListWidgetItem* item = selection[0];
    Group* group = (Group*) item->data(Qt::UserRole).toULongLong();

    //Get selected user
    selection = ui->list_students->selectedItems();
    item = selection[0];
    Student *student = (Student*) item->data(Qt::UserRole).toULongLong();

    //Execute query
    QSqlQuery query(*m_db);
    query.prepare("INSERT INTO tau_groups_users(id_groups, id_users) VALUES(:id_groups, :id_users)");
    query.bindValue(":id_groups", group->getId());
    query.bindValue(":id_users", student->getId());
    query.exec();

    update_list_groups();

    return true;
}

bool usersgroupsmanager::remove_group() {
    //Get current selection
    QList<QListWidgetItem*> selection = ui->list_groups_yes->selectedItems();

    //Error if no group selected
    if(selection.length() <= 0) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un groupe.");
        return false;
    }

    //Get selected group
    QListWidgetItem* item = selection[0];
    Group* group = (Group*) item->data(Qt::UserRole).toULongLong();

    //Get selected user
    selection = ui->list_students->selectedItems();
    item = selection[0];
    Student *student = (Student*) item->data(Qt::UserRole).toULongLong();

    //Execute query
    QSqlQuery query(*m_db);
    query.prepare("DELETE FROM tau_groups_users WHERE id_users=:id_users AND id_groups=:id_groups");
    query.bindValue(":id_groups", group->getId());
    query.bindValue(":id_users", student->getId());
    query.exec();

    update_list_groups();

    return true;
}
