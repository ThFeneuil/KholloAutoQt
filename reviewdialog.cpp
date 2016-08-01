#include "reviewdialog.h"
#include "ui_reviewdialog.h"

ReviewDialog::ReviewDialog(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReviewDialog)
{
    ui->setupUi(this);
    m_db = db;





    // Clear the list
    ui->listCol1->clear();
    free_students();


    // Make the request
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, first_name, email FROM tau_users ORDER BY name, first_name");

    // Treat the request
    while (query.next()) {
        Student* stdnt = new Student();
        stdnt->setId(query.value(0).toInt());
        stdnt->setName(query.value(1).toString());
        stdnt->setFirst_name(query.value(2).toString());
        stdnt->setEmail(query.value(3).toString());
        QListWidgetItem *item = new QListWidgetItem(stdnt->getName() + " " + stdnt->getFirst_name(), ui->listCol1);
        item->setData(Qt::UserRole, (qulonglong) stdnt);
        queue_displayedStudents.enqueue(stdnt);
    }
}

ReviewDialog::~ReviewDialog() {
    delete ui;
    free_students();
}

bool ReviewDialog::free_students() {
    while (!queue_displayedStudents.isEmpty())
        delete queue_displayedStudents.dequeue();
    return true;
}
