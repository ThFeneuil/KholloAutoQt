#include "kholle.h"

Kholle::Kholle()
{
    m_id_students = 0;
    m_id_timeslots = 0;

    m_student = NULL;
    m_timeslot = NULL;
    m_status = OK;
    m_weeks = 0;
    m_past_id_timeslots.clear();
    m_id_pb_kholle = -1;
}

Kholle::~Kholle() {

}

//Getters
int Kholle::getId_students() const {
    return m_id_students;
}

int Kholle::getId_timeslots() const {
    return m_id_timeslots;
}
Student* Kholle::student() const {
    return m_student;
}
Timeslot* Kholle::timeslot() const {
    return m_timeslot;
}
int Kholle::status() const {
    return m_status;
}
int Kholle::weeks() const {
    return m_weeks;
}
QList<int>* Kholle::past_id_timeslots() {
    return &m_past_id_timeslots;
}
int Kholle::id_pb_kholle() const {
    return m_id_pb_kholle;
}

//Setters
void Kholle::setId_students(int id_students) {
    m_id_students = id_students;
}

void Kholle::setId_timeslots(int id_timeslots) {
    m_id_timeslots = id_timeslots;
}
void Kholle::setStudent(Student* stud) {
    m_student = stud;
}
void Kholle::setTimeslot(Timeslot* slot) {
    m_timeslot = slot;
}
void Kholle::setStatus(Status status) {
    m_status = status;
}
void Kholle::setWeeks(int weeks) {
    m_weeks = weeks;
}
void Kholle::setPast_id_timeslots(QList<int> l) {
    m_past_id_timeslots = l;
}
void Kholle::setId_pb_kholle(int id) {
    m_id_pb_kholle = id;
}


//Other functions
int Kholle::nearestKholle(QSqlDatabase *db, QMap<int, Timeslot*> *timeslots, int id_user, Timeslot *t, int id_kholle) {
    /** Number of weeks to nearest Kholle with the same Kholleur. (-1) if no such Kholle **/

    int id_kholleur = t->getId_kholleurs();

    QSqlQuery query(*db);
    query.prepare("SELECT K.`id_timeslots` FROM tau_kholles AS K JOIN tau_timeslots AS T ON K.`id_timeslots` = T.`id` "
                  "WHERE K.`id_users`=:id_user AND T.`id_kholleurs`=:id_kholleur AND K.`id`!=:id_kholle "
                  "ORDER BY ABS(strftime('%s', T.`date`) - strftime('%s', :date_current)) LIMIT 1;");
    query.bindValue(":id_user", id_user);
    query.bindValue(":id_kholleur", id_kholleur);
    query.bindValue(":date_current", t->getDate());
    query.bindValue(":id_kholle", id_kholle);
    query.exec();

    if(query.next()) {
        return t->weeksTo(timeslots->value(query.value(0).toInt()));
    }
    else {
        return (-1);
    }
}

int Kholle::nearest(QMap<int, Timeslot*> *timeslots, QSqlDatabase *db) {
    /** Number of weeks to nearest Kholle with the same Kholleur. (-1) if no such Kholle **/

    Timeslot* ts_current = timeslots->value(this->getId_timeslots());

    return Kholle::nearestKholle(db, timeslots, this->getId_students(), ts_current, this->getId());
}

int Kholle::correspondingStatus(int weeks) {
    if(weeks == -1 || weeks > 3)
        return (Kholle::OK);
    else if(weeks <= 1)
        return (Kholle::Error);
    else
        return (Kholle::Warning);
}

void Kholle::updateStatus(DataBase *dbase, QSqlDatabase *db, QList<Kholle*> kholloscope, int week) {
    /** Set the status of this kholle **/
    Timeslot* ts_current = dbase->listTimeslots()->value(this->getId_timeslots());

    stat_info *info = Kholle::calculateStatus(db, dbase, this->getId_students(), ts_current, week, kholloscope, this->getId());

    this->setStatus((Kholle::Status) info->status);
    this->setWeeks(info->weeks);
    this->setId_pb_kholle(info->id_pb_kholle);
    free(info);
}

stat_info* Kholle::calculateStatus(QSqlDatabase *db, DataBase *dbase, int id_user, Timeslot *t, int week, QList<Kholle *> kholloscope, int id_kholle) {
    /** Calculate status of this hypothetical kholle, including incompatible and impossible **/

    stat_info *info = (stat_info*) malloc(sizeof(stat_info));
    int id_pb_kholle = -1;
    bool compat = Utilities::compatible(db, dbase, id_user, t, week, id_kholle, &id_pb_kholle);

    if(compat) {
        int weeks = nearestKholle(db, dbase->listTimeslots(), id_user, t, id_kholle);
        info->weeks = weeks;
        info->status = correspondingStatus(weeks);
    }
    else {
        info->weeks = -1;
        if(kholloscope_contains(id_pb_kholle, kholloscope)) {
            info->status = Kholle::Incompatible;
        }
        else {
            info->status = Kholle::Impossible;
        }
    }
    info->id_pb_kholle = id_pb_kholle;

    return info;
}

bool Kholle::kholloscope_contains(int id, QList<Kholle *> kholloscope) {
    for(int i = 0; i < kholloscope.length(); i++) {
        if(id == kholloscope[i]->getId())
            return true;
    }
    return false;
}
