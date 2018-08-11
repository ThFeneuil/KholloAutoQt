#include "generationmethod.h"

GenerationMethod::GenerationMethod(QSqlDatabase *db, QDate date, int week)
{
    m_db = db;
    m_db->transaction();

    m_date = date;
    m_week = week;
    m_abort = false;

    m_out_log = NULL;
    m_log_file = new QFile(QCoreApplication::applicationDirPath() + QDir::separator() + NAME_FILE_LOG);
    if(m_log_file->open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
        m_out_log = new QTextStream(m_log_file);

    m_dbase = new DataBase(m_db);
    m_dbase->load();
    setPupilsOnTimeslots();

    m_kholloscope = new QList<Kholle*>();
}

GenerationMethod::~GenerationMethod() {
    rollback();

    delete m_log_file;
    delete m_out_log;

    while(!m_kholloscope->isEmpty())
        delete m_kholloscope->takeFirst();
    delete m_kholloscope;

    delete m_dbase;
}

void GenerationMethod::launch(QList<Subject*> *selected_subjects, QMap<int, QList<Student*> > *input) {

    //Warning if not enough free space for everyone...
    QList<Subject*> *problem_subjects = testAvailability(selected_subjects, input);
    bool no_space = problem_subjects->length() > 0;
    if(no_space) {
        QString warning_text = "Il n'y a pas suffisamment d'horaires de kholles libres pour accommoder tous les élèves sélectionnés dans ";
        warning_text += (problem_subjects->length() >= 2 ? "les matières suivantes : <br />" : "la matière suivante : <br />");
        int i;
        for(i = 0; i < problem_subjects->length(); i++)
            warning_text += "<strong>" + problem_subjects->at(i)->getName() + "</strong> <br />";
        QMessageBox::warning(NULL, "Attention", warning_text);
    }
    delete problem_subjects;

    if(no_space) {
        emit generationEnd(GEN_FAIL);
        return;
    }

    m_future = QtConcurrent::run(this, &GenerationMethod::start, selected_subjects, input);
}

void GenerationMethod::abort() {
    log("Annulation de la génération...\n\n", true);
    m_abort = true;
    m_future.waitForFinished();
}

void GenerationMethod::commit() {
    m_future.waitForFinished();
    m_db->commit();
}

void GenerationMethod::rollback() {
    m_future.waitForFinished();
    m_db->rollback();
}

QList<Kholle*>* GenerationMethod::kholloscope() {
    return m_kholloscope;
}

void GenerationMethod::setPupilsOnTimeslots() {
    //Get timeslots
    QMap<int, Timeslot*> *map = m_dbase->listTimeslots();

    //Loop through all timeslots and update number of pupils
    foreach(Timeslot* ts, *map) {
        if(ts->getDate() >= m_date && ts->getDate() <= m_date.addDays(6))
            ts->setPupils(ts->getPupils() - ts->kholles()->length());
    }
}

float GenerationMethod::proba(Student *user, Timeslot *timeslot, QDate m_date) {
    return Utilities::proba(m_dbase, user, timeslot, m_date);
}

QMap<int, float> *GenerationMethod::corrected_proba(Student* user, QList<Timeslot*> timeslots, QDate m_date) {
    //User from DataBase !!!
    return Utilities::corrected_proba(m_dbase, user, timeslots, m_date);
}

QDate GenerationMethod::date() const {
    return m_date;
}

int GenerationMethod::week() const {
    return m_week;
}

QMap<int, Student*>* GenerationMethod::listStudents() const {
    return m_dbase->listStudents();
}

QMap<int, Timeslot*>* GenerationMethod::listTimeslots() const {
    return m_dbase->listTimeslots();
}

bool GenerationMethod::compatible(int id_user, Timeslot *timeslot, int week, int id_kholle_avoid, int *id_pb_kholle) {
    return Utilities::compatible(m_db, m_dbase, id_user, timeslot, week, id_kholle_avoid, id_pb_kholle);
}

void GenerationMethod::log(QString text, bool canBeDisplayed) {
    if(canBeDisplayed)
        emit newLogInfo(text);
    if(m_out_log != NULL)
        (*m_out_log) << text;
}

Kholle* GenerationMethod::createKholle(Student* stud, Timeslot *ts) {
    /** Creates a new Kholle with the parameters and returns the kholle **/
    //Create new kholle
    Kholle *k = new Kholle();
    k->setId_students(stud->getId());
    k->setId_timeslots(ts->getId());
    k->setStudent(stud);
    k->setTimeslot(ts);

    return k;
}

void GenerationMethod::setKhollesStatus() {
    /** Set the status of the generated kholles **/

    for(int i = 0; i < m_kholloscope->length(); i++) {
        if(m_kholloscope->at(i)->status() >= Kholle::Incompatible)
            m_kholloscope->at(i)->updateStatus(m_dbase, m_db, *m_kholloscope, m_week);
        else {
            int weeks = m_kholloscope->at(i)->nearest(m_dbase->listTimeslots(), m_db);
            m_kholloscope->at(i)->setStatus((Kholle::Status) Kholle::correspondingStatus(weeks));
            m_kholloscope->at(i)->setWeeks(weeks);
            m_kholloscope->at(i)->setId_pb_kholle(-1);
        }
    }
}

void GenerationMethod::saveInSql() {
    Utilities::saveInSql(m_db, m_kholloscope);
}

QList<Subject*>* GenerationMethod::testAvailability(QList<Subject*> *selected_subjects, QMap<int, QList<Student*> > *input) {
    /** Tests if there is enough free space for everyone selected **/

    int i;
    QList<Subject*> *result = new QList<Subject*>;
    QMap<int, Timeslot*> *map_ts = m_dbase->listTimeslots();

    //Test for every subject
    for(i = 0; i < selected_subjects->length(); i++) {
        QList<Student*> users = input->value(selected_subjects->at(i)->getId()); //Which users are selected

        //Sum up the free places during this week and in this subject
        int free_places = 0;
        foreach(Timeslot* ts, *map_ts) {
            if(ts->kholleur()->getId_subjects() == selected_subjects->at(i)->getId() && ts->getDate() >= m_date && ts->getDate() <= m_date.addDays(6))
                free_places += ts->getPupils();
        }

        //If not enough free place, then add it to the list
        if(users.length() > free_places)
            result->append(selected_subjects->at(i));
    }

    return result;
}
