#include "interfacedialog.h"
#include "ui_interfacedialog.h"

InterfaceDialog::InterfaceDialog(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InterfaceDialog)
{
    ui->setupUi(this);
    m_db = db;
    m_students = new QList<Student*>();

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

    query.exec("SELECT `id`, `name`, `shortName`, `color` FROM `tau_subjects` WHERE `id` IN (SELECT DISTINCT `id_subjects` FROM `tau_kholleurs`)");
    while (query.next()) {
        Subject* subj = new Subject();
        subj->setId(query.value(0).toInt());
        subj->setName(query.value(1).toString());
        subj->setShortName(query.value(2).toString());
        subj->setColor(query.value(3).toString());

        QTableWidget* table = new QTableWidget(12, 6, this);
        QStringList days;
        days << "Lundi" << "Mardi" << "Mercredi" << "Jeudi" << "Vendredi" << "Samedi";
        table->setHorizontalHeaderLabels(days);
        QStringList hours;
        hours << "8:00" << "9:00" << "10:00" << "11:00" << "12:00" << "13:00" << "14:00" << "15:00" << "16:00" << "17:00" << "18:00" << "19:00";
        table->setVerticalHeaderLabels(hours);
        QListWidget* list_kholleurs = new QListWidget();
        list_kholleurs->setMaximumWidth(100);
        QSqlQuery queryKholleurs(*m_db);
        queryKholleurs.prepare("SELECT `id`, `name`, `id_subjects`, `duration`, `preparation`, `pupils` FROM `tau_kholleurs` WHERE `id_subjects`=:id_subjects");
        queryKholleurs.bindValue(":id_subjects", subj->getId());
        queryKholleurs.exec();
        while (queryKholleurs.next()) {
            Kholleur* khll = new Kholleur();
            khll->setId(queryKholleurs.value(0).toInt());
            khll->setName(queryKholleurs.value(1).toString());
            khll->setId_subjects(queryKholleurs.value(2).toInt());
            khll->setDuration(queryKholleurs.value(3).toInt());
            khll->setPreparation(queryKholleurs.value(4).toInt());
            khll->setPupils(queryKholleurs.value(5).toInt());

            QListWidgetItem *item = new QListWidgetItem(khll->getName(), list_kholleurs);
            item->setData(Qt::UserRole, (qulonglong) khll);
        }
        QHBoxLayout* layout = new QHBoxLayout();
        layout->addWidget(table);
        layout->addWidget(list_kholleurs);
        QWidget* tab = new QWidget();
        tab->setLayout(layout);

        ui->tabWidget->addTab(tab, subj->getShortName());
    }
}

InterfaceDialog::~InterfaceDialog()
{
    delete ui;
}
