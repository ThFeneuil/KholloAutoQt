#include "reviewdialog.h"
#include "ui_reviewdialog.h"

ReviewDialog::ReviewDialog(QSqlDatabase *db, QWidget *parent, SelectionMethod method, Student *stdnt, Kholleur *khll) :
    QDialog(parent),
    ui(new Ui::ReviewDialog)
{
    // Initialisation
    ui->setupUi(this);
    m_db = db;
    m_shortcutNotepad = Notepad::shortcut();
    this->addAction(m_shortcutNotepad);

    m_students = new QList<Student*>();
    m_kholleurs = new QList<Kholleur*>();
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, first_name FROM tau_users ORDER BY UPPER(name), UPPER(first_name)");
    while (query.next()) {
        Student* stud = new Student();
        stud->setId(query.value(0).toInt());
        stud->setName(query.value(1).toString());
        stud->setFirst_name(query.value(2).toString());
        m_students->append(stud);
    }
    query.exec("SELECT id, name FROM tau_kholleurs ORDER BY UPPER(name)");
    while (query.next()) {
        Kholleur* khll = new Kholleur();
        khll->setId(query.value(0).toInt());
        khll->setName(query.value(1).toString());
        m_kholleurs->append(khll);
    }

    // Initialisation ComboBox
    ui->comboBox_select->addItem("Étudiant", FROMSTUDENTS);
    ui->comboBox_select->addItem("Kholleur", FROMKHOLLEURS);
    if(method == FROMSTUDENTS)
            ui->comboBox_select->setCurrentText("Étudiant");
    else    ui->comboBox_select->setCurrentText("Kholleur");
    connect(ui->comboBox_select, SIGNAL(currentIndexChanged(int)), this, SLOT(updateCol1()));

    // Initialisation Lists
    ui->listCol1->clear();
    ui->listCol2->clear();
    connect(ui->listCol1, SIGNAL(itemSelectionChanged()), this, SLOT(updateCol2()));
    connect(ui->listCol2, SIGNAL(itemSelectionChanged()), this, SLOT(updateInfoArea()));
    updateCol1();

    if(method == FROMSTUDENTS) {
        if(stdnt) {
            selectStudent(stdnt);
            updateCol2();
            if(khll) {
                selectKholleur(khll);
                updateInfoArea();
            }
        }
    } else {
        if(khll) {
            selectKholleur(khll);
            updateCol2();
            if(stdnt) {
                selectStudent(stdnt);
                updateInfoArea();
            }
        }
    }
}

ReviewDialog::~ReviewDialog() {
    delete ui;
    for(int i=0; i<m_students->length(); i++)
        delete m_students->at(i);
    for(int i=0; i<m_kholleurs->length(); i++)
        delete m_kholleurs->at(i);
    delete m_students;
    delete m_kholleurs;
    this->removeAction(m_shortcutNotepad);
    delete m_shortcutNotepad;
}

QListWidget* ReviewDialog::listStudents() {
     return ((ui->comboBox_select->currentData().toInt() == FROMSTUDENTS) ? ui->listCol1 : ui->listCol2);
}

QListWidget* ReviewDialog::listKholleurs() {
    return ((ui->comboBox_select->currentData().toInt() == FROMKHOLLEURS) ? ui->listCol1 : ui->listCol2);
}

void ReviewDialog::updateCol1() {
    // Clear the widgets
    disconnect(ui->listCol1, SIGNAL(itemSelectionChanged()), this, SLOT(updateCol2()));
    disconnect(ui->listCol2, SIGNAL(itemSelectionChanged()), this, SLOT(updateInfoArea()));
    ui->listCol1->clear();
    ui->listCol2->clear();
    connect(ui->listCol1, SIGNAL(itemSelectionChanged()), this, SLOT(updateCol2()));
    connect(ui->listCol2, SIGNAL(itemSelectionChanged()), this, SLOT(updateInfoArea()));
    ui->infoKholles->setText("");
    // Fill the first column
    if(ui->comboBox_select->currentData().toInt() == FROMKHOLLEURS)
        updateKholeurs(ui->listCol1);
    else
        updateStudents(ui->listCol1);
}

void ReviewDialog::updateCol2() {
    // Clear the widgets
    disconnect(ui->listCol2, SIGNAL(itemSelectionChanged()), this, SLOT(updateInfoArea()));
    ui->listCol2->clear();
    connect(ui->listCol2, SIGNAL(itemSelectionChanged()), this, SLOT(updateInfoArea()));
    ui->infoKholles->setText("");

    // Fill the second column
    if(ui->comboBox_select->currentData().toInt() == FROMKHOLLEURS)
        updateStudents(ui->listCol2);
    else
        updateKholeurs(ui->listCol2);
}

void ReviewDialog::updateStudents(QListWidget* list) {
    for(int i=0; i<m_students->length(); i++) {
        Student* stdnt = m_students->at(i);
        QString nameStudent = (ui->comboBox_select->currentData().toInt() == FROMKHOLLEURS) ?
                "["+QString::number(nbKholles(stdnt, (Kholleur*) ui->listCol1->currentItem()->data(Qt::UserRole).toULongLong()))+"] " +
                    stdnt->getName() + " " + stdnt->getFirst_name() :
                stdnt->getName() + " " + stdnt->getFirst_name();
        QListWidgetItem *item = new QListWidgetItem(nameStudent, list);
        item->setData(Qt::UserRole, (qulonglong) stdnt);
    }
}

void ReviewDialog::updateKholeurs(QListWidget* list) {
    for(int i=0; i<m_kholleurs->length(); i++) {
        Kholleur* khll = m_kholleurs->at(i);
        QString nameKholleur = (ui->comboBox_select->currentData().toInt() == FROMSTUDENTS) ?
                "["+QString::number(nbKholles((Student*) ui->listCol1->currentItem()->data(Qt::UserRole).toULongLong(), khll))+"] " +
                    khll->getName() :
                khll->getName();
        QListWidgetItem *item = new QListWidgetItem(nameKholleur, list);
        item->setData(Qt::UserRole, (qulonglong) khll);
    }
}

void ReviewDialog::updateInfoArea() {
    // Get the student and the kholleur
    Kholleur* khll = NULL;
    Student* stud = NULL;
    stud = (Student*) listStudents()->currentItem()->data(Qt::UserRole).toULongLong();
    khll = (Kholleur*) listKholleurs()->currentItem()->data(Qt::UserRole).toULongLong();

    if(!khll || !stud) {
        ui->infoKholles->setText("");
        return;
    }
    QString text = "";
    text += "<strong>Étudiant :</strong> " + stud->getName() + " " + stud->getFirst_name() + "<br />";
    text += "<strong>Kholleur :</strong> " + khll->getName() + "<br />";

    // Get the list of kholles
    QSqlQuery qKholles(*m_db);
    QSqlQuery qTimeslots(*m_db);
    qKholles.prepare("SELECT K.id, K.id_users, K.id_timeslots FROM tau_kholles AS K "
                     "JOIN tau_timeslots AS T "
                        "ON K.id_timeslots = T.id "
                     "WHERE K.id_users = :id_students AND T.id_kholleurs = :id_kholleurs "
                     "ORDER BY T.date, T.time");
    qKholles.bindValue(":id_students", stud->getId());
    qKholles.bindValue(":id_kholleurs", khll->getId());
    qKholles.exec();

    QString textKholles = "";
    int numKholle = 0;
    while(qKholles.next()) {
        numKholle++;
        // Get the kholle
        Kholle* klle = new Kholle();
        klle->setId(qKholles.value(0).toInt());
        klle->setId_students(qKholles.value(1).toInt());
        klle->setId_timeslots(qKholles.value(2).toInt());

        // Get the timeslot of this kholle
        qTimeslots.prepare("SELECT id, time_start, time, time_end, id_kholleurs, date, pupils FROM tau_timeslots WHERE id=:id_timeslots");
        qTimeslots.bindValue(":id_timeslots", klle->getId_timeslots());
        qTimeslots.exec();
        if(qTimeslots.next()) {
            Timeslot* ts = new Timeslot();
            ts->setId(qTimeslots.value(0).toInt());
            ts->setTime_start(qTimeslots.value(1).toTime());
            ts->setTime(qTimeslots.value(2).toTime());
            ts->setTime_end(qTimeslots.value(3).toTime());
            ts->setId_kholleurs(qTimeslots.value(4).toInt());
            ts->setDate(qTimeslots.value(5).toDate());
            ts->setPupils(qTimeslots.value(6).toInt());
            klle->setTimeslot(ts);
        } else
            continue;

        // Display the information
        textKholles += "<br />";
        textKholles += "<strong>Kholle " + QString::number(numKholle) + " :</strong><br />";
        textKholles += "Le " + klle->timeslot()->getDate().toString("dd/MM/yyyy") + " à " + klle->timeslot()->getTime().toString("hh:mm") + "<br />";
    }

    text += "<br />Nombre de kholles : " + QString::number(numKholle) + "<br />" + textKholles;

    // Update the widgets
    ui->infoKholles->setText(text);
}

bool ReviewDialog::selectStudent(Student* stdnt) {
    for(int i=0; i<listStudents()->count() && stdnt; i++) {
        QListWidgetItem* item = listStudents()->item(i);
        if(((Student*) item->data(Qt::UserRole).toULongLong())->getId() == stdnt->getId()) {
            listStudents()->setCurrentItem(item);
            return true;
        }
    }
    return false;
}

bool ReviewDialog::selectKholleur(Kholleur* khll) {
    for(int i=0; i<listKholleurs()->count() && khll; i++) {
        QListWidgetItem* item = listKholleurs()->item(i);
        if(((Kholleur*) item->data(Qt::UserRole).toULongLong())->getId() == khll->getId()) {
            listKholleurs()->setCurrentItem(item);
            return true;
        }
    }
    return false;
}

int ReviewDialog::nbKholles(Student* stdnt, Kholleur* khll) {
    if(stdnt && khll) {
        QSqlQuery qKholles(*m_db);
        qKholles.prepare("SELECT COUNT(*) FROM tau_kholles "
                            "WHERE id_users = :id_students AND id_timeslots IN "
                            "(SELECT id FROM tau_timeslots WHERE id_kholleurs = :id_kholleurs)");
        qKholles.bindValue(":id_students", stdnt->getId());
        qKholles.bindValue(":id_kholleurs", khll->getId());
        qKholles.exec();
        if(qKholles.next())
            return qKholles.value(0).toInt();
    }
    return -1;
}
