#include "fivewavesmethod.h"

FiveWavesMethod::FiveWavesMethod(QSqlDatabase *db, QDate date, int week) : GenerationMethod(db, date, week)
{

}

bool FiveWavesMethod::start(QList<Subject*> *selected_subjects, QMap<int, QList<Student*> > *input) {
    // Wave 1
    saveInSql();

    setKhollesStatus();
    //m_downgraded.clear();

    log("Traitement des collisions...\n", true);
    exchange(0, Collisions, 0);

    log("Traitement des furieux et déçus...\n", true);
    exchange(0, Warnings, 40);

    log("Amélioration du score...\n\n", true);
    exchange(0, All, 15);

    return true;
}

bool FiveWavesMethod::exchange(int index, ExchangeType type, int score_limit) {
    /** Exchange timeslots between people in the kholloscope to improve score **/

    return true;
/*
    switch(type) {
        case Warnings:
            while(index < kholloscope.length() && kholloscope[index]->status() == Kholle::OK)
                index++;
            break;
        case Collisions:
            while(index < kholloscope.length() &&
                  Utilities::sum_day(m_db, kholloscope[index]->getId_students(), m_dbase->listTimeslots()->value(kholloscope[index]->getId_timeslots())->getDate()) <= MaxWeightSubject)
                index++;
            break;
        default:
            break;
    }

    if(index >= kholloscope.length())
        return true;

    Kholle* current = kholloscope[index];
    Timeslot* t_current = m_dbase->listTimeslots()->value(current->getId_timeslots());
    Student* s_current = m_dbase->listStudents()->value(current->getId_students());

    float max_score = 0;
    int max_index = -1;
    for(int i = 0; i < kholloscope.length(); i++) {
        if(i == index)
            continue;

        Kholle* k = kholloscope[i];
        Timeslot* t = m_dbase->listTimeslots()->value(k->getId_timeslots());
        Student* s = m_dbase->listStudents()->value(k->getId_students());
        if(m_downgraded.contains(s->getId()) && m_downgraded.value(s->getId()))
            continue;

        if(type == Collisions && t_current->getDate() == t->getDate())
            continue;

        if(t_current->kholleur()->getId_subjects() == t->kholleur()->getId_subjects()) {
            if(Utilities::compatible(m_db, m_dbase, s_current->getId(), t, m_week, current->getId())
                    && Utilities::compatible(m_db, m_dbase, s->getId(), t_current, m_week, k->getId())) {
                int sub_weight = t_current->kholleur()->subject()->getWeight();
                int w_current_old = Utilities::sum_day(m_db, s_current->getId(), t_current->getDate());
                int w_current_new = Utilities::sum_day(m_db, s_current->getId(), t->getDate()) + (t_current->getDate() != t->getDate() ? sub_weight : 0);
                int w_old = Utilities::sum_day(m_db, s->getId(), t->getDate());
                int w_new = Utilities::sum_day(m_db, s->getId(), t_current->getDate()) + (t_current->getDate() != t->getDate() ? sub_weight : 0);

                //QMap<int, float> *p_current = Utilities::corrected_proba(m_dbase, s_current, poss.value(t_current->kholleur()->getId_subjects()).value(s_current->getId()), m_date);
                //QMap<int, float> *p = Utilities::corrected_proba(m_dbase, s, poss.value(t->kholleur()->getId_subjects()).value(s->getId()), m_date);
                QMap<int, float> *p_current = probabilities.value(t_current->kholleur()->getId_subjects()).value(s_current->getId());
                QMap<int, float> *p = probabilities.value(t->kholleur()->getId_subjects()).value(s->getId());

                stat_info *info1 = Kholle::calculateStatus(m_db, m_dbase, s_current->getId(), t, m_week, kholloscope, current->getId());
                stat_info *info2 = Kholle::calculateStatus(m_db, m_dbase, s->getId(), t_current, m_week, kholloscope, k->getId());

                //bool do_exchange = false;

                bool weight_ok = false, status_ok = false, probas_ok = false;
                if(type == Collisions) {
                    weight_ok = (w_current_new <= MaxWeightSubject || w_current_new < w_current_old)
                                    && (w_new <= MaxWeightSubject || w_new < w_old);
                    status_ok = (info1->status <= current->status()
                                    && info2->status <= k->status());
                    probas_ok = (p_current->value(t->getId()) + p->value(t_current->getId())
                                    >= p_current->value(t_current->getId()) + p->value(t->getId()));
                }

                /*if(type == Collisions) {
                    if((w_current_new <= MaxWeightSubject || w_current_new < w_current_old)
                            && (w_new <= MaxWeightSubject || w_new < w_old)) {
                        if(Kholle::correspondingStatus(n1) <= current->status()
                                && Kholle::correspondingStatus(n2) <= k->status()) {
                            if(p_current->value(t->getId()) + p->value(t_current->getId()) >= p_current->value(t_current->getId()) + p->value(t->getId())) {
                                do_exchange = true;
                            }
                        }
                    }
                }*-/

                else{
                    weight_ok = (w_current_new <= MaxWeightSubject || w_current_new <= w_current_old)
                            && (w_new <= MaxWeightSubject || w_new <= w_old);
                    probas_ok = (p_current->value(t->getId()) - p_current->value(t_current->getId()) >= score_limit
                            && p->value(t_current->getId()) - p->value(t->getId()) >= -score_limit);

                    if(type == Warnings)
                        status_ok = (info1->status < current->status()
                                     && info2->status < k->status());
                    else
                        status_ok = (info1->status <= current->status()
                                     && info2->status <= k->status());
                }

                /*else {
                    if((w_current_new <= MaxWeightSubject || w_current_new <= w_current_old)
                            && (w_new <= MaxWeightSubject || w_new <= w_old)) {
                        if(Kholle::correspondingStatus(n1) == Kholle::OK
                                && Kholle::correspondingStatus(n2) == Kholle::OK) {
                            if(p_current->value(t->getId()) - p_current->value(t_current->getId()) >= score_limit
                                    && p->value(t_current->getId()) - p->value(t->getId()) >= -score_limit) {
                                do_exchange = true;
                            }
                        }
                    }
                }*-/

                if(weight_ok && status_ok && probas_ok) {
                    float delta_proba = p_current->value(t->getId()) + p->value(t_current->getId())
                                            - p_current->value(t_current->getId()) - p->value(t->getId());
                    if(max_index == -1 || delta_proba > max_score) {
                        max_index = i;
                        max_score = delta_proba;
                    }
                }

                free(info1);
                free(info2);
            }
        }
    }

    if(max_index != -1) {
        Kholle* k = kholloscope[max_index];
        Timeslot* t = m_dbase->listTimeslots()->value(k->getId_timeslots());
        Student* s = m_dbase->listStudents()->value(k->getId_students());

        m_downgraded.insert(s->getId(), true);
        m_downgraded.insert(s_current->getId(), false);

        Utilities::make_exchange(m_db, m_dbase, current, t_current, k, t, m_week, kholloscope);
    }

    return exchange(index + 1, type, score_limit);*/
}
