#include "interfacetab.h"
#include "ui_interfacetab.h"

InterfaceTab::InterfaceTab(Subject* subj, int id_week, QDate monday, QSqlDatabase *db, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InterfaceTab)
{
    ui->setupUi(this);
    m_subject = subj;
    m_db = db;
    m_id_week = id_week;
    m_monday = monday;

    KholloTable* scene = new KholloTable(m_db, id_week, m_monday);
    ui->viewTable->setScene(scene);

    QSqlQuery query(*m_db);
    query.prepare("SELECT `id`, `name`, `id_subjects`, `duration`, `preparation`, `pupils` FROM `tau_kholleurs` WHERE `id_subjects`=:id_subjects");
    query.bindValue(":id_subjects", m_subject->getId());
    query.exec();
    while (query.next()) {
        Kholleur* khll = new Kholleur();
        khll->setId(query.value(0).toInt());
        khll->setName(query.value(1).toString());
        khll->setId_subjects(query.value(2).toInt());
        khll->setDuration(query.value(3).toInt());
        khll->setPreparation(query.value(4).toInt());
        khll->setPupils(query.value(5).toInt());

        QListWidgetItem *item = new QListWidgetItem(khll->getName(), ui->list_kholleurs);
        item->setData(Qt::UserRole, (qulonglong) khll);
    }
    connect(ui->list_kholleurs, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(displayTeacher(QListWidgetItem*)));
}

InterfaceTab::~InterfaceTab()
{
    delete ui;
}

bool InterfaceTab::displayTeacher(QListWidgetItem *item) {
    if(item == NULL) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un kholleur.");
        return false;
    } else {
        Kholleur* kll = (Kholleur*) item->data(Qt::UserRole).toULongLong();
        ((KholloTable*) ui->viewTable->scene())->displayKholleur(kll);

    }

    return true;
}

bool InterfaceTab::selectStudent(Student* stud) {
    ((KholloTable*) ui->viewTable->scene())->displayStudent(stud);
    return true;
}
