#include "utilities.h"

Utilities::Utilities()
{

}

float Utilities::proba(DataBase* dbase, Student *user, Timeslot *timeslot, QDate m_date) {
    /** Calculates a score for this (user, timeslot) couple
        Attention ! The Student and Timeslot need to have their "kholleur", "kholles", etc. properties set (in DataBase) **/
    int kholleur_total = 0;
    int kholles_total = 0;
    int this_kholleur = 0;

    float p = 0; //Insert formula here

    int i;
    foreach(Kholleur* k, *dbase->listKholleurs()) {
        if(k->getId_subjects() == timeslot->kholleur()->getId_subjects()) //Total number of kholleurs in this subject
            kholleur_total++;
    }

    for (i = 0; i < user->kholles()->length(); i++) {
        Timeslot* ts = user->kholles()->at(i)->timeslot();

        if(ts->kholleur()->getId_subjects() == timeslot->kholleur()->getId_subjects()) //Total number of kholles in this subject for this person
            kholles_total++;
        if(ts->getId_kholleurs() == timeslot->getId_kholleurs()) { //If it was the same kholleur
            this_kholleur++;

            if(ts->getDate() >= m_date.addDays(-21) && ts->getDate() <= m_date.addDays(27)) { //If it was within certain time limits
                p -= 40*pow(2, 3-timeslot->weeksTo(ts));
//                p -= 40*pow(2, 28-abs(ts->getDate().daysTo(timeslot->getDate())));
            }
        }
    }

    p -= 20 * this_kholleur * kholleur_total;
    p += 20 * kholles_total;

    return p;
}


QMap<int, float>* Utilities::corrected_proba(DataBase* dbase, Student *user, QList<Timeslot*> list, QDate m_date) {
    /** Calculates the probability for this user and list of timeslots, taking into account the average probability for this user
        Attention, user and timeslots from DataBase !**/

    QMap<int, float> *probas = new QMap<int, float>(); //Result
    int i;
    float sum = 0;
    for(i = 0; i < list.length(); i++) {
        float p = proba(dbase, user, list[i], m_date);
        probas->insert(list[i]->getId(), p);
        sum += p;
    }

    float corr = 1000 - (sum / list.length()); //The correction to be applied...

    for(i = 0; i < list.length(); i++) {
        int id = list[i]->getId();
        probas->insert(id, probas->value(id) + corr);
    }

    return probas;
}


bool Utilities::compatible(QSqlDatabase *db, DataBase *dbase, int id_user, Timeslot *timeslot, int week, int id_kholle_avoid, int *id_pb_kholle) {
    //Get all groups of user (the ids)
    QList<Group*> *groups = dbase->listStudents()->value(id_user)->groups();

    int i;
    if(groups->length() > 0) {
        QString request = "id_groups=" + QString::number(groups->at(0)->getId());
        for(i = 1; i < groups->length(); i++) {
            request = request + " OR id_groups=" + QString::number(groups->at(i)->getId());
        }

        //Get all courses that can interfere with this timeslot
        QSqlQuery courses_query(*db);
        courses_query.prepare("SELECT * FROM tau_courses WHERE (" + request + ") AND id_day=:id_day AND id_week=:id_week AND ("
                                                                              "(time_start <= :time_start AND time_end > :time_start) OR"
                                                                              "(time_start < :time_end AND time_end >= :time_end) OR"
                                                                              "(time_start >= :time_start AND time_end <= :time_end) )");
        courses_query.bindValue(":time_start", timeslot->getTime_start().toString("HH:mm:ss"));
        courses_query.bindValue(":time_end", timeslot->getTime_end().toString("HH:mm:ss"));
        courses_query.bindValue(":id_day", timeslot->getDate().dayOfWeek());
        courses_query.bindValue(":id_week", week);
        courses_query.exec();

        if(courses_query.next()) {
            return false;
        }

        //Get all events that can interfere with this timeslot
        QSqlQuery event_query(*db);
        event_query.prepare("SELECT * FROM tau_events AS E JOIN tau_events_groups AS G ON E.`id` = G.`id_events` WHERE (" + request + ") AND ("
                                      "(E.`start` <= :start_time AND E.`end` > :start_time) OR"
                                      "(E.`start` < :end_time AND E.`end` >= :end_time) OR"
                                      "(E.`start` >= :start_time AND E.`end` <= :end_time))");
        event_query.bindValue(":start_time", QDateTime(timeslot->getDate(), timeslot->getTime_start()).toString("yyyy-MM-dd HH:mm:ss"));
        event_query.bindValue(":end_time", QDateTime(timeslot->getDate(), timeslot->getTime_end()).toString("yyyy-MM-dd HH:mm:ss"));
        event_query.exec();

        if(event_query.next()) {
            return false;
        }

    }

    //Get all kholles that can interfere with this timeslot (except the kholle to avoid)
    QSqlQuery kholle_query(*db);
    kholle_query.prepare("SELECT K.`id` FROM tau_kholles AS K JOIN tau_timeslots AS T ON K.`id_timeslots` = T.`id` WHERE K.`id_users`=:id_users AND T.`date`=:date AND K.`id`!=:id_avoid AND ("
                         "(T.`time_start` <= :time_start AND T.`time_end` > :time_start) OR "
                         "(T.`time_start` < :time_end AND T.`time_end` >= :time_end) OR "
                         "(T.`time_start` >= :time_start AND T.`time_end` <= :time_end))");
    kholle_query.bindValue(":id_users", id_user);
    kholle_query.bindValue(":date", timeslot->getDate().toString("yyyy-MM-dd"));
    kholle_query.bindValue(":time_start", timeslot->getTime_start().toString("HH:mm:ss"));
    kholle_query.bindValue(":time_end", timeslot->getTime_end().toString("HH:mm:ss"));
    kholle_query.bindValue(":id_avoid", id_kholle_avoid);
    kholle_query.exec();

    if(kholle_query.next()) {
        if(id_pb_kholle != NULL) {
            int id = kholle_query.value(0).toInt();
            if(!kholle_query.next())
                *id_pb_kholle = id;
        }
        return false;
    }

    return true;
}

bool Utilities::compatible(Timeslot *t1, Timeslot *t2) {
    return (t1->getDate() != t2->getDate() || t1->getTime_end() <= t2->getTime_start() || t1->getTime_start() >= t2->getTime_end());
}

void Utilities::make_exchange(QSqlDatabase *db, DataBase *dbase, Kholle* current, Timeslot* t_current, Kholle* k, Timeslot* t, int week, QList<Kholle*> kholloscope) {
    /** Realises the exchange locally and on DB **/

    QSqlQuery query(*db);
    query.prepare("UPDATE tau_kholles SET id_timeslots=:id_ts WHERE id=:id");
    query.bindValue(":id", current->getId());
    query.bindValue(":id_ts", t->getId());
    query.exec();

    query.prepare("UPDATE tau_kholles SET id_timeslots=:id_ts WHERE id=:id");
    query.bindValue(":id", k->getId());
    query.bindValue(":id_ts", t_current->getId());
    query.exec();

    current->setId_timeslots(t->getId());
    current->updateStatus(dbase, db, kholloscope, week);
    k->setId_timeslots(t_current->getId());
    k->updateStatus(dbase, db, kholloscope, week);
}

void Utilities::quickSort(QList<Timeslot *> *list, int i, int j, QMap<int, float> *probas) {
    /** QuickSort of a QList<Timeslot*> based on the value of the probabilities **/

    if(i >= j)
        return;

    int pivot_index = i;
    int k;
    for(k = i+1; k <= j; k++) {
        if(probas->value(list->at(k)->getId()) > probas->value(list->at(pivot_index)->getId())) {
            Timeslot* pivot = list->at(pivot_index);
            list->replace(pivot_index, list->at(k));
            list->replace(k, list->at(pivot_index+1));
            list->replace(pivot_index+1, pivot);
            pivot_index++;
        }
    }

    quickSort(list, i, pivot_index-1, probas);
    quickSort(list, pivot_index+1, j, probas);
}

int Utilities::listMax(QList<Timeslot *> list, QMap<int, float> *probas) {
    /** Returns the highest probability from the list or a very high score if the list is empty
        Attention ! Student must have "kholles" and other properties set (from DataBase) **/
    int i;
    bool is_empty = true;
    int max = 0;

    //QMap<int, float> *probas = corrected_proba(dbase, user, list, m_date);

    for(i = 0; i < list.length(); i++) {
        float p = probas->value(list[i]->getId());
        if(p > max || i == 0)
            max = p;
        if(list[i]->getPupils() > 0) {
            /*float p = probas->value(list[i]->getId());
            if(is_empty || p > max)
                max = p;*/
            is_empty = false;
        }
    }

    //delete probas;

    if(is_empty)
        return 1000000;
    else
        return max;
}

void Utilities::saveInSql(QSqlDatabase *db, QList<Kholle*>* kholloscope) {
    for(int i = 0; i < kholloscope->length(); i++) {
        //Get kholle
        Kholle* k = kholloscope->at(i);

        //Prepare query for insertion
        QSqlQuery query(*db);
        query.prepare("INSERT INTO tau_kholles(id_users, id_timeslots) VALUES(:id_students, :id_timeslots)");
        query.bindValue(":id_students", k->getId_students());
        query.bindValue(":id_timeslots", k->getId_timeslots());
        query.exec();
        kholloscope->at(i)->setId(query.lastInsertId().toInt());
    }
}

int Utilities::sum_day(QSqlDatabase *db, int id_user, QDate date) {
    /** Determines if this kholle is on a day with collision; (-1) if error **/

    QSqlQuery query(*db);
    query.prepare("SELECT SUM(S.`weight`) FROM "
                  "(SELECT * FROM tau_kholles WHERE `id_users` = :id_user) as K "
                  "JOIN (SELECT * FROM tau_timeslots WHERE `date`=:date) as T ON K.`id_timeslots` = T.`id` "
                  "JOIN tau_kholleurs AS P ON T.`id_kholleurs` = P.`id` "
                  "JOIN tau_subjects AS S ON P.`id_subjects` = S.`id`;");
    query.bindValue(":id_user", id_user);
    query.bindValue(":date", date);
    query.exec();

    if(query.next())
        return query.value(0).toInt();
    else
        return (-1);
}

