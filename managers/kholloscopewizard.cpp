#include "kholloscopewizard.h"
#include "ui_kholloscopewizard.h"

KholloscopeWizard::KholloscopeWizard(QSqlDatabase *db, QWidget *parent) :
    QWizard(parent),
    ui(new Ui::KholloscopeWizard)
{
    ui->setupUi(this);

    m_db = db;

    m_students = new QList<Student*>();
    load_students();
    m_subjects = new QList<Subject*>();
    load_subjects();
    m_input = new QMap<int, QList<Student*> >;

    //Add pages
    addPage(new SubjectsPage(db));
    addPage(new UsersPage(db));
    addPage(new GeneratePage(db));
}

KholloscopeWizard::~KholloscopeWizard()
{
    delete ui;
    free_subjects();
    free_students();
    //free_assoc_subjects();
    //free_input();
    delete assoc_subjects;
    delete m_students;
    delete m_subjects;
    delete m_input;
}

QList<Subject*>* KholloscopeWizard::get_assoc_subjects() {
    return assoc_subjects;
}

void KholloscopeWizard::set_assoc_subjects(QList<Subject*> *list) {
    assoc_subjects = list;
}

void KholloscopeWizard::free_assoc_subjects() {
    while(!assoc_subjects->isEmpty()) {
        free(assoc_subjects->takeFirst());
    }
}

void KholloscopeWizard::load_students() {
    //Delete students
    free_students();

    //Prepare the query
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, first_name FROM tau_users ORDER BY name, first_name");

    //Treat the query
    while(query.next()) {
        Student *student = new Student();
        student->setId(query.value(0).toInt());
        student->setName(query.value(1).toString());
        student->setFirst_name(query.value(2).toString());
        m_students->append(student);
    }
}

void KholloscopeWizard::free_students() {
    while(!m_students->isEmpty()) {
        free(m_students->takeFirst());
    }
}

QList<Student*>* KholloscopeWizard::get_students() {
    return m_students;
}

void KholloscopeWizard::load_subjects() {
    //Delete the subjects
    free_subjects();

    //Prepare query
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, shortName, color FROM tau_subjects ORDER BY name");

    //Treat the query
    while(query.next()) {
        Subject* subject = new Subject();
        subject->setId(query.value(0).toInt());
        subject->setName(query.value(1).toString());
        subject->setShortName(query.value(2).toString());
        subject->setColor(query.value(3).toString());
        m_subjects->append(subject);
    }
}

void KholloscopeWizard::free_subjects() {
    while(!m_subjects->isEmpty()) {
        free(m_subjects->takeFirst());
    }
}

QList<Subject*>* KholloscopeWizard::get_subjects() {
    return m_subjects;
}

void KholloscopeWizard::set_input(QMap<int, QList<Student*> > *input) {
    m_input = input;
}

void KholloscopeWizard::free_input() {
    foreach(QList<Student*> l, *m_input) {
        while(!l.isEmpty()) {
            free(l.takeFirst());
        }
    }
}

QMap<int, QList<Student*> > *KholloscopeWizard::get_input() {
    return m_input;
}

/*int KholloscopeWizard::get_week() {
    return m_week;
}

void KholloscopeWizard::set_week(int week) {
    m_week = week;
}

QDate KholloscopeWizard::get_date() {
    return m_date;
}

void KholloscopeWizard::set_date(QDate date) {
    m_date = date;
}*/
