#include "tribesmanager.h"
#include "ui_tribesmanager.h"

TribesManager::TribesManager(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TribesManager)
{
    ui->setupUi(this);
    m_db = db;

    getSubjects();
    getStudents();

    connect(ui->list_subjects, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
    connect(ui->btn_associate, SIGNAL(clicked()), this, SLOT(associate()));
    connect(ui->list_students, SIGNAL(itemSelectionChanged()), this, SLOT(studentsChanged()));

    m_shortcutNotepad = Notepad::shortcut();
    this->addAction(m_shortcutNotepad);
}

TribesManager::~TribesManager()
{
    delete ui;
    freeSubjects();
    freeStudents();
}

void TribesManager::getSubjects() {
    //Delete what was before
    freeSubjects();
    ui->list_subjects->clear();

    //Load the list of subjects and display it (alphabetically)
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, shortName, weight FROM tau_subjects ORDER BY UPPER(name)");

    while(query.next()) {
        Subject *s = new Subject();
        s->setId(query.value(0).toInt());
        s->setName(query.value(1).toString());
        s->setShortName(query.value(2).toString());
        s->setWeight(query.value(3).toInt());
        queue_displayedSubjects.enqueue(s);

        QListWidgetItem *item = new QListWidgetItem(s->getName(), ui->list_subjects);
        item->setData(Qt::UserRole, (qulonglong) s);
    }
}

void TribesManager::freeSubjects() {
    while(!queue_displayedSubjects.isEmpty()) {
        delete queue_displayedSubjects.dequeue();
    }
}

void TribesManager::getStudents() {
    //Free students
    freeStudents();

    //Load the list of all students alphabetically (no displaying)
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, first_name, email FROM tau_users ORDER BY UPPER(name)");

    while(query.next()) {
        Student *s = new Student();
        s->setId(query.value(0).toInt());
        s->setName(query.value(1).toString());
        s->setFirst_name(query.value(2).toString());
        s->setEmail(query.value(3).toString());
        list_displayedStudents.append(s);
    }
}

void TribesManager::freeStudents() {
    while(!list_displayedStudents.isEmpty()) {
        delete list_displayedStudents.takeFirst();
    }
}

void TribesManager::updateStudentList(int id_subject) {
    //Clear list
    ui->list_students->clear();

    //Load tribes names in map
    map_students_tribes.clear();

    QSqlQuery query(*m_db);
    query.prepare("SELECT id_users, name_tribe FROM tau_tribes WHERE id_subjects=:id_subjects");
    query.bindValue(":id_subjects", id_subject);
    query.exec();

    while(query.next()) {
        int id_student = query.value(0).toInt();
        QString name_tribe = query.value(1).toString();
        map_students_tribes.insert(id_student, name_tribe);
    }

    //Populate list
    for(int i = 0; i < list_displayedStudents.length(); i++) {
        Student *s = list_displayedStudents[i];
        QString text = s->getName() + " " + s->getFirst_name() + " ";

        if(text.length() > 50) {
            text.truncate(46);
            text.append("... ");
        }
        text = text.leftJustified(50, ' ');

        if(map_students_tribes.contains(s->getId()))
            text += "\t" + map_students_tribes.value(s->getId());
        else
            text += "\tAucune tribu";

        QListWidgetItem *item = new QListWidgetItem(text, ui->list_students);
        item->setData(Qt::UserRole, (qulonglong) s);
    }

    //Populate ComboBox
    ui->tribe_name->clear();
    ui->tribe_name->addItem("Aucune tribu");

    QSqlQuery names_query(*m_db);
    names_query.prepare("SELECT DISTINCT name_tribe FROM tau_tribes WHERE id_subjects=:id_subjects ORDER BY UPPER(name_tribe)");
    names_query.bindValue(":id_subjects", id_subject);
    names_query.exec();

    while(names_query.next()) {
        ui->tribe_name->addItem(names_query.value(0).toString());
    }

    ui->tribe_name->setCurrentText("");
    QLineEdit *edit = ui->tribe_name->lineEdit();
    if(edit != NULL)
        edit->setPlaceholderText("Tapez ici le nom d'une tribu...");
}

void TribesManager::selectionChanged() {
    ui->list_students->clear();
    QList<QListWidgetItem*> selection = ui->list_subjects->selectedItems();

    if(selection.length() <= 0) {
        //Disable the right pane
        ui->list_students->setEnabled(false);
        ui->tribe_name->setEnabled(false);
        ui->btn_associate->setEnabled(false);
        return;
    }

    //Enable the right pane
    ui->list_students->setEnabled(true);
    ui->tribe_name->setEnabled(true);
    ui->btn_associate->setEnabled(true);

    updateStudentList(((Subject*) selection[0]->data(Qt::UserRole).toULongLong())->getId());
}

void TribesManager::associate() {
    //Get tribe name
    QString tribe_name = ui->tribe_name->currentText();

    //Trim and avoid empty string
    tribe_name = tribe_name.trimmed();
    if(tribe_name.isEmpty()) {
        QMessageBox::critical(this, "Erreur", "Il faut renseigner le nom de la tribu.");
        return;
    }

    //Get subject id
    QList<QListWidgetItem*> subjects = ui->list_subjects->selectedItems();
    if(subjects.length() <= 0) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner une matière.");
        return;
    }
    int id_subject = ((Subject*) subjects[0]->data(Qt::UserRole).toULongLong())->getId();

    //Get students list
    QList<QListWidgetItem*> students = ui->list_students->selectedItems();

    //Treat students
    m_db->transaction();
    for(int i = 0; i < students.length(); i++) {
        Student *s = (Student*) students[i]->data(Qt::UserRole).toULongLong();

        if(map_students_tribes.contains(s->getId())) {
            //Only update
            if(tribe_name.toUpper() == "AUCUNE TRIBU") {
                //Delete
                QSqlQuery delete_query(*m_db);
                delete_query.prepare("DELETE FROM tau_tribes WHERE id_subjects=:id_subjects AND id_users=:id_students");
                delete_query.bindValue(":id_subjects", id_subject);
                delete_query.bindValue(":id_students", s->getId());
                delete_query.exec();
            }
            else {
                //Update
                QSqlQuery update_query(*m_db);
                update_query.prepare("UPDATE tau_tribes SET name_tribe=:tribe_name WHERE id_subjects=:id_subjects AND id_users=:id_students");
                update_query.bindValue(":tribe_name", tribe_name);
                update_query.bindValue(":id_subjects", id_subject);
                update_query.bindValue(":id_students", s->getId());
                update_query.exec();
            }
        }
        else {
            if(tribe_name.toUpper() != "AUCUNE TRIBU") {
                //Insert
                QSqlQuery insert_query(*m_db);
                insert_query.prepare("INSERT INTO tau_tribes(id_subjects, id_users, name_tribe) VALUES(:id_subjects, :id_students, :tribe_name)");
                insert_query.bindValue(":id_subjects", id_subject);
                insert_query.bindValue(":id_students", s->getId());
                insert_query.bindValue(":tribe_name", tribe_name);
                insert_query.exec();
            }
        }
    }
    m_db->commit();

    selectionChanged();
}

void TribesManager::studentsChanged() {
    QList<QListWidgetItem*> selection = ui->list_students->selectedItems();

    QString text = QString::number(selection.length()) + (selection.length() <= 1 ? " élève sélectionné" : " élèves sélectionnés");
    ui->student_no->setText(text);
}
