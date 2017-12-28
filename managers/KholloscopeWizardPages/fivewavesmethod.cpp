#include "fivewavesmethod.h"

FiveWavesMethod::FiveWavesMethod(QSqlDatabase *db, QDate date, int week) : GenerationMethod(db, date, week)
{

}

void FiveWavesMethod::start(QList<Subject*> *selected_subjects, QMap<int, QList<Student*> > *input) {
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

    emit generationEnd(0);
}

working_index *FiveWavesMethod::findMax() {
    /** To find the place with highest score **/
    bool is_empty = true;
    int max = -1;
    QList<int> s_maxs;
    QList<int> u_maxs;
/*
    QList<int> s_keys = m_poss.keys();
    int i, j;
    for(i = 0; i < s_keys.length(); i++) {
        QList<int> u_keys = m_poss.value(s_keys[i]).keys();

        for(j = 0; j < u_keys.length(); j++) {
            int score = Utilities::listMax(m_poss.value(s_keys[i]).value(u_keys[j]), probabilities.value(s_keys[i]).value(u_keys[j]));
            if(is_empty || score > max) {
                max = score;
                s_maxs.clear();
                u_maxs.clear();
                s_maxs.append(s_keys[i]);
                u_maxs.append(u_keys[j]);
            }
            if(score == max) {
                s_maxs.append(s_keys[i]);
                u_maxs.append(u_keys[j]);
            }
            is_empty = false;
        }
    }

    if(is_empty) {
        return NULL;
    }

    working_index *res = (working_index*) malloc(sizeof(working_index));
    res->max = max;

    //Get random number
    if(s_maxs.length() > 0) {
        int random_int = qrand() % s_maxs.length();
        res->current_student = u_maxs[random_int];
        res->current_subject = s_maxs[random_int];
    }
    else {
        res->current_student = 0;
        res->current_subject = 0;
    }
    return res;*/
    return NULL;
}

bool FiveWavesMethod::generate() {
    /** Generate the Kholloscope **//*
    profondeur++;
    working_index* index = findMax(); //Get the highest priority

    //It is finished
    if(index == NULL) {
        return true;
    }

    //Copy current index to the global variable
    last_index.current_student = index->current_student;
    last_index.current_subject = index->current_subject;
    last_index.max = index->max;

    //If abort needed, finish
    if(m_abort) {
        free(index);
        return false;
    }

    QMap<int, QList<Timeslot*> > map = m_poss.value(index->current_subject);
    QList<Timeslot*> loop = map.take(index->current_student); //Get possibilities and delete them from the possibilities map
    poss.insert(index->current_subject, map);

    //No need to sort again
    //QMap<int, float> *probas = Utilities::corrected_proba(m_dbase, m_dbase->listStudents()->value(index->current_student), loop, m_date); //Do we really need to recalculate ???!
    //Utilities::quickSort(&loop, 0, loop.length() - 1, probas);
    //delete probas;

    for(int i = 0; i < loop.length(); i++) { //For every possibility
        if(loop[i]->getPupils() > 0) { //If enough space
            createKholle(index->current_student, loop[i]);

            QMap<int, QList<Timeslot*> > *old = updatePoss(index->current_student, loop[i]); //Update the possibilities

            //Recursive call
            if(generate()) { //If true = we are finished
                resetPoss(index->current_student, old);
                delete old;
                map = poss.value(index->current_subject);
                map.insert(index->current_student, loop);
                poss.insert(index->current_subject, map);
                free(index);
                return true;
            }

            //If abort needed, return false
            if(m_abort) {
                delete old;
                free(index);
                return false;
            }

            //If false, reset everything
            resetPoss(index->current_student, old);
            delete old;
            delete kholloscope.takeLast();
            loop[i]->setPupils(loop[i]->getPupils() + 1);
        }
    }

    //Put everything back and return
    map = poss.value(index->current_subject);
    map.insert(index->current_student, loop);
    poss.insert(index->current_subject, map);
    profondeur--;
    free(index);*/
    return false;
}

bool FiveWavesMethod::exchange(int index, ExchangeType type, int score_limit) {
    /** Exchange timeslots between people in the kholloscope to improve score **/

    return true;
/*
    switch(type) {
        case Warnings:
            while(index < kholloscope()->length() && kholloscope()->at(index)->status() == Kholle::OK)
                index++;
            break;
        case Collisions:
            while(index < kholloscope()->length() &&
                  Utilities::sum_day(m_db, kholloscope()->at(index)->getId_students(), m_dbase->listTimeslots()->value(kholloscope()->at(index)->getId_timeslots())->getDate()) <= MaxWeightSubject)
                index++;
            break;
        default:
            break;
    }

    if(index >= kholloscope()->length())
        return true;

    Kholle* current = kholloscope()->at(index);
    Timeslot* t_current = m_dbase->listTimeslots()->value(current->getId_timeslots());
    Student* s_current = m_dbase->listStudents()->value(current->getId_students());

    float max_score = 0;
    int max_index = -1;
    for(int i = 0; i < kholloscope()->length(); i++) {
        if(i == index)
            continue;

        Kholle* k = kholloscope()->at(i);
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

                stat_info *info1 = Kholle::calculateStatus(m_db, m_dbase, s_current->getId(), t, m_week, *kholloscope(), current->getId());
                stat_info *info2 = Kholle::calculateStatus(m_db, m_dbase, s->getId(), t_current, m_week, *kholloscope(), k->getId());

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
        Kholle* k = kholloscope->at(max_index);
        Timeslot* t = m_dbase->listTimeslots()->value(k->getId_timeslots());
        Student* s = m_dbase->listStudents()->value(k->getId_students());

        m_downgraded.insert(s->getId(), true);
        m_downgraded.insert(s_current->getId(), false);

        Utilities::make_exchange(m_db, m_dbase, current, t_current, k, t, m_week, &kholloscope);
    }

    return exchange(index + 1, type, score_limit);*/
}
