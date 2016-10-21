#include "interfacetab.h"
#include "ui_interfacetab.h"

InterfaceTab::InterfaceTab(Subject* subj, int id_week, QDate monday, QSqlDatabase *db, DataBase *dbase, QWidget *parent, InterfaceDialog* interface) :
    QWidget(parent),
    ui(new Ui::InterfaceTab)
{
    ui->setupUi(this);
    m_subject = subj;
    m_db = db;
    m_id_week = id_week;
    m_monday = monday;
    m_dbase = dbase;
    m_interface = interface;

    KholloTable* scene = new KholloTable(m_db, id_week, m_monday, ui->areaKholles, m_dbase, m_interface, this);
    ui->viewTable->setScene(scene);
    //connect(ui->viewTable, SIGNAL())
    QSqlQuery query(*m_db);
    query.prepare("SELECT `id`, `name`, `id_subjects`, `duration`, `preparation`, `pupils` FROM `tau_kholleurs` WHERE `id_subjects`=:id_subjects ORDER BY `name`");
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
    connect(ui->list_kholleurs, SIGNAL(itemSelectionChanged()), this, SLOT(displayTeacher()));
}

InterfaceTab::~InterfaceTab()
{
    delete ui;
}

Subject* InterfaceTab::getSubject() const {
    return m_subject;
}

bool InterfaceTab::displayTeacher() {
    QListWidgetItem* item = ui->list_kholleurs->currentItem();

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
    for(int i=0; i<ui->list_kholleurs->count(); i++) {
        QListWidgetItem* item =  ui->list_kholleurs->item(i);
        Kholleur* kll = (Kholleur*) item->data(Qt::UserRole).toULongLong();
        if(stud) {
            QSqlQuery query(*m_db);
            /*query.prepare("SELECT COUNT(*) "
                          "FROM `tau_kholles` "
                          "WHERE `id_users` = :id_users AND `id_timeslots` IN "
                            "(SELECT `id` FROM `tau_timeslots` WHERE `id_kholleurs` = :id_kholleurs)");
            query.bindValue(":id_users", stud->getId());
            query.bindValue(":id_kholleurs", kll->getId());
            query.exec();
            if(query.next()) {
                QString nb = query.value(0).toString();
                item->setText(kll->getName() + " (" + nb+ ")");
            } else {
                QMessageBox::critical(NULL, "Erreur", "Erreur base de données. Code: 4253");
                exit(4253);
            }*/
            query.prepare("SELECT T.`date` "
                          "FROM `tau_kholles` AS K "
                          "JOIN `tau_timeslots` AS T "
                            "ON T.`id` = K.`id_timeslots` "
                          "WHERE `id_users` = :id_users AND T.`id_kholleurs` = :id_kholleurs");
            query.bindValue(":id_users", stud->getId());
            query.bindValue(":id_kholleurs", kll->getId());
            query.exec();

            int lastKholles = 0;
            int nbKholles = 0;
            QDate today = m_monday;
            while(query.next()) {
                int nbDays = query.value(0).toDate().daysTo(today);
                if(nbKholles == 0 || abs(nbDays) < abs(lastKholles))
                    lastKholles = nbDays;
                nbKholles++;
            }
            item->setText(kll->getName() + " (" + QString::number(nbKholles) + ")");
            if(nbKholles) {
                int week = (lastKholles > 0) ? (lastKholles-1) / 7 + 1 : lastKholles / 7;
                if(0 >= lastKholles && lastKholles > -7)
                    item->setToolTip("Cette semaine...");
                else if(abs(week) == 1)
                    item->setToolTip(QString::number(week) + " semaine (" + QString::number(lastKholles) + ")");
                else
                    item->setToolTip(QString::number(week) + " semaines (" + QString::number(lastKholles) + ")");
            } else
                item->setToolTip("");
        } else {
            item->setText(kll->getName());
            item->setToolTip("");
        }
    }
    ((KholloTable*) ui->viewTable->scene())->displayStudent(stud);
    return true;
}

bool InterfaceTab::selectKholleur(Kholleur* khll) {
    if(khll) {
        for(int i=0; i<ui->list_kholleurs->count(); i++) {
            QListWidgetItem* item =  ui->list_kholleurs->item(i);
            Kholleur* khll_item = (Kholleur*) item->data(Qt::UserRole).toULongLong();
            if(khll->getId() == khll_item->getId()) {
                ui->list_kholleurs->setCurrentItem(item);
                return true;
            }
        }
    }
    return false;
}
