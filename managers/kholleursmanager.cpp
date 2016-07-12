#include "managers/kholleursmanager.h"
#include "ui_kholleursmanager.h"

KholleursManager::KholleursManager(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KholleursManager)
{
    ui->setupUi(this);
    connect(ui->pushButton_addTeacher, SIGNAL(clicked()), this, SLOT(add_teacher()));
    connect(ui->pushButton_updateTeacher, SIGNAL(clicked()), this, SLOT(update_teacher()));
    connect(ui->pushButton_deleteTeacher, SIGNAL(clicked()), this, SLOT(delete_teacher()));
    connect(ui->list_teachers, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(update_teacher()));
    connect(ui->pushButton_addKholleur, SIGNAL(clicked()), this, SLOT(add_kholleur()));
    connect(ui->pushButton_updateKholleur, SIGNAL(clicked()), this, SLOT(update_kholleur()));
    connect(ui->pushButton_deleteKholleur, SIGNAL(clicked()), this, SLOT(delete_kholleur()));
    connect(ui->list_kholleurs, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(update_kholleur()));

    // DB
    m_db = db;
    update_listTeachers();
    update_listKholleurs();
}

KholleursManager::~KholleursManager()
{
    delete ui;
    free_kholleurs();
    free_teachers();
}

bool KholleursManager::free_teachers() {
    while (!queue_displayedTeachers.isEmpty())
        free(queue_displayedTeachers.dequeue());
    return true;
}

bool KholleursManager::free_kholleurs() {
    while (!queue_displayedKholleurs.isEmpty())
        free(queue_displayedKholleurs.dequeue());
    return true;
}

bool KholleursManager::update_listTeachers() {
    // Clear the list
    ui->list_teachers->clear();
    free_teachers();

    // Make the request
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, id_subjects FROM tau_teachers ORDER BY name");

    // Treat the request
    while (query.next()) {
        Teacher* tcher = new Teacher();
        tcher->setId(query.value(0).toInt());
        tcher->setName(query.value(1).toString());
        tcher->setId_subjects(query.value(2).toInt());
        QListWidgetItem *item = new QListWidgetItem(tcher->getName() + " (" + QString::number(tcher->getId_subjects()) + ")", ui->list_teachers);
        item->setData(Qt::UserRole, (qulonglong) tcher);
        queue_displayedTeachers.enqueue(tcher);
    }

    return true;
}

bool KholleursManager::add_teacher() {
    return true;
}

bool KholleursManager::update_teacher() {
    return true;
}

bool KholleursManager::delete_teacher() {
    return true;
}

bool KholleursManager::update_listKholleurs() {
    // Clear the list
    ui->list_kholleurs->clear();
    free_kholleurs();

    // Make the request
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, id_subjects, duration, preparation, pupils FROM tau_kholleurs ORDER BY name");

    // Treat the request
    while (query.next()) {
        Kholleur* khll = new Kholleur();
        khll->setId(query.value(0).toInt());
        khll->setName(query.value(1).toString());
        khll->setId_subjects(query.value(2).toInt());
        khll->setDuration(query.value(3).toInt());
        khll->setPreparation(query.value(4).toInt());
        khll->setPupils(query.value(5).toInt());
        QListWidgetItem *item = new QListWidgetItem(khll->getName() + " (" + QString::number(khll->getId_subjects()) + ") : " + QString::number(khll->getDuration()) + ", " + QString::number(khll->getPreparation()) + ", " + QString::number(khll->getPupils()), ui->list_kholleurs);
        item->setData(Qt::UserRole, (qulonglong) khll);
        queue_displayedKholleurs.enqueue(khll);
    }

    return true;
}

bool KholleursManager::add_kholleur() {
    return true;
}

bool KholleursManager::update_kholleur() {
    return true;
}

bool KholleursManager::delete_kholleur() {
    return true;
}
