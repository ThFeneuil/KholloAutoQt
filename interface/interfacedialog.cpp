#include "interfacedialog.h"
#include "ui_interfacedialog.h"

InterfaceDialog::InterfaceDialog(QSqlDatabase *db, int id_week, QDate monday, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InterfaceDialog)
{
    ui->setupUi(this);
    m_db = db;
    m_students = new QList<Student*>();
    m_id_week = id_week;
    m_monday = monday;

    QMessageBox::information(this, "DataBase", "START !");
    DataBase* database = new DataBase(m_db);
    database->load();
    QMessageBox::information(this, "DataBase", "STOP !");
    delete database;

    // Get the list of all the students
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, first_name, email FROM tau_users ORDER BY name, first_name");
    while (query.next()) {
        Student* stdnt = new Student();
        stdnt->setId(query.value(0).toInt());
        stdnt->setName(query.value(1).toString());
        stdnt->setFirst_name(query.value(2).toString());
        stdnt->setEmail(query.value(3).toString());
        m_students->append(stdnt);

        // Display the student
        QListWidgetItem *item = new QListWidgetItem(stdnt->getName() + " " + stdnt->getFirst_name(), ui->list_students);
        item->setData(Qt::UserRole, (qulonglong) stdnt);
    }
    connect(ui->list_students, SIGNAL(itemSelectionChanged()), this, SLOT(selectStudent()));

    query.exec("SELECT `id`, `name`, `shortName`, `color` FROM `tau_subjects` WHERE `id` IN (SELECT DISTINCT `id_subjects` FROM `tau_kholleurs`)");
    while (query.next()) {
        Subject* subj = new Subject();
        subj->setId(query.value(0).toInt());
        subj->setName(query.value(1).toString());
        subj->setShortName(query.value(2).toString());
        subj->setColor(query.value(3).toString());

        InterfaceTab* tab = new InterfaceTab(subj, m_id_week, m_monday, m_db);
        ui->tabWidget->addTab(tab, subj->getShortName());
    }
}

InterfaceDialog::~InterfaceDialog()
{
    delete ui;
}

bool InterfaceDialog::selectStudent() {
    QListWidgetItem *item = ui->list_students->currentItem();

    if(item == NULL) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un étudiant.");
        return false;
    } else {
        Student* stud = (Student*) item->data(Qt::UserRole).toULongLong();
        for(int i=0; i<ui->tabWidget->count(); i++)
            ((InterfaceTab*) ui->tabWidget->widget(i))->selectStudent(stud);
    }

    return true;
}
