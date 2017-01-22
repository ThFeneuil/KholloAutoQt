#include "kholloscopewizard.h"
#include "ui_kholloscopewizard.h"

KholloscopeWizard::KholloscopeWizard(QSqlDatabase *db, QWidget *parent) :
    QWizard(parent),
    ui(new Ui::KholloscopeWizard)
{
    ui->setupUi(this);

    //DB
    m_db = db;

    //Get the students and the subjects
    m_students = new QList<Student*>();
    load_students();
    m_subjects = new QList<Subject*>();
    load_subjects();
    m_input = new QMap<int, QList<Student*> >;
    assoc_subjects = new QList<Subject*>();

    //Add pages
    addPage(new SubjectsPage(db));
    addPage(new UsersPage(db));
    addPage(new GeneratePage(db, parent));

    m_shortcutNotepad = Notepad::shortcut();
    this->addAction(m_shortcutNotepad);
}

KholloscopeWizard::~KholloscopeWizard()
{
    delete ui;
    free_subjects();
    free_students();
    delete assoc_subjects;
    delete m_students;
    delete m_subjects;
    delete m_input;
    this->removeAction(m_shortcutNotepad);
    delete m_shortcutNotepad;
}

QList<Subject*>* KholloscopeWizard::get_assoc_subjects() {
    return assoc_subjects;
}

void KholloscopeWizard::load_students() {
    //Delete students
    free_students();

    //Prepare the query
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, first_name FROM tau_users ORDER BY UPPER(`name`), UPPER(`first_name`)");

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
        delete m_students->takeFirst();
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
    query.exec("SELECT id, name, shortName, color FROM tau_subjects ORDER BY UPPER(`name`)");

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
        delete m_subjects->takeFirst();
    }
}

QList<Subject*>* KholloscopeWizard::get_subjects() {
    return m_subjects;
}

QMap<int, QList<Student*> > *KholloscopeWizard::get_input() {
    return m_input;
}
