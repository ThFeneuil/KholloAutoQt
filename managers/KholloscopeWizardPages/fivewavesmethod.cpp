#include "fivewavesmethod.h"

FiveWavesMethod::FiveWavesMethod(QSqlDatabase *db, QDate date, int week) : GenerationMethod(db, date, week)
{

}

void FiveWavesMethod::launch(QList<Subject *> *selected_subjects, QMap<int, QList<Student *> > *input) {
    //Start a timeout timer
    m_timeout = false;
    QTimer::singleShot(TIMEOUT_INT, this, SLOT(timeout()));

    GenerationMethod::launch(selected_subjects, input);
}

void FiveWavesMethod::start(QList<Subject*> *selected_subjects, QMap<int, QList<Student*> > *input) {
    last_index.current_student = -1; //Set to impossible value
    last_index.current_subject = -1;
    last_index.max = -1;

    //Initialise random number generator
    qsrand((uint)QTime::currentTime().msecsSinceStartOfDay());

    log("GENERATION DU " + date().toString("dd/MM/yyyy"), true);
    log("\n\nDébut de la génération...\n", true);

    //Prepare the table
    constructPoss(selected_subjects, input);

    /// Wave 1
    bool res = wave_1();

    //Cancelled generation
    if(m_abort) {
        //displayBlocking();
        clearPoss();
        emit generationEnd(GEN_CANCELLED);
        return;
    }

    //Failed wave 1 => timeout or no solution
    if(!res) {
        if(!m_timeout) {
            //displayBlocking();
            clearPoss();
            emit generationEnd(GEN_FAIL);
            return;
        }
        else {
            log("Timeout ! (" + QString::number(TIMEOUT_INT / 1000) + " s)\n", true);
            force();
        }
    }

    log("Fin du premier jet...\n", true);

    saveInSql();

    setKhollesStatus();

    ///Case failed wave 1 => treat impossible
    if(!res && m_timeout) {
        log("Traitement des kholles impossibles et incompatibles...\n\n", true);

        for(int i = 0; i < MAX_ITERATION && (remainImpossible(Kholle::Impossible) != -1 || remainImpossible(Kholle::Incompatible) != -1); i++) {
            ///First phase : treat impossible
            for(int i = 0; i < MAX_ITERATION && remainImpossible(Kholle::Impossible) != -1; i++)
                treatImpossible(0, Kholle::Impossible);

            ///Second phase : treat incompatible
            for(int i = 0; i < MAX_ITERATION && remainImpossible(Kholle::Incompatible) != -1; i++)
                treatImpossible(0, Kholle::Incompatible);
        }

        int index = remainImpossible(Kholle::Impossible);
        if(index != -1) {
            last_index.current_student = kholloscope()->at(index)->getId_students();
            last_index.current_subject = kholloscope()->at(index)->timeslot()->kholleur()->getId_subjects();
        }
        else {
            index = remainImpossible(Kholle::Incompatible);
            if(index != -1) {
                last_index.current_student = kholloscope()->at(index)->getId_students();
                last_index.current_subject = kholloscope()->at(index)->timeslot()->kholleur()->getId_subjects();
            }
        }

        if(index != -1) {
            //displayBlocking();
            clearPoss();
            emit generationEnd(m_abort ? GEN_CANCELLED : GEN_FAIL);
            return;
        }
    }

    log("Traitement des collisions...\n", true);
    exchange(0, Collisions, 0);

    log("Traitement des furieux et déçus...\n", true);
    exchange(0, Warnings, 40);

    log("Amélioration du score...\n\n", true);
    exchange(0, All, 15);

    clearPoss();
    emit generationEnd(m_abort ? GEN_CANCELLED : GEN_SUCCESS);
}

void FiveWavesMethod::constructPoss(QList<Subject*> *selected_subjects, QMap<int, QList<Student*> > *input) {
    /** To construct the 3D table of possibilities - a list for every (Student, Subject) couple **/

    log("\n\nConstruction de la table des possibilités, avec les scores suivants : \n\n", false);

    int i, j, k;
    for(i = 0; i < selected_subjects->length(); i++) { //For every selected subject
        //Create a new map
        QMap<int, QList<Timeslot*> > map;
        QMap<int, QMap<int, float>* > p_map;

        //The selected users for this subject
        QList<Student*> users = input->value(selected_subjects->at(i)->getId());

        for(j = 0; j < users.length(); j++) { //For every selected user
            //Create new list
            QList<Timeslot*> new_list;

            foreach(Timeslot* ts, *listTimeslots()) { //For every timeslot
                if(ts->kholleur()->getId_subjects() == selected_subjects->at(i)->getId()
                        && ts->getDate() >= date()
                        && ts->getDate() <= date().addDays(6)
                        && compatible(users[j]->getId(), ts, week())) { //Add the compatible timeslots
                    new_list.append(ts);
                }
            }

            QMap<int, float> *probas = corrected_proba(listStudents()->value(users[j]->getId()), new_list, date()); //User from DataBase !

            //Log the probability for these timeslots
            for(k = 0; k < new_list.length(); k++) {
                log(users[j]->getName() + ", " + selected_subjects->at(i)->getShortName() + ", " + new_list[k]->kholleur()->getName() + ", " + QString::number(new_list[k]->getId()) + " : ", false);
                log(QString::number(probas->value(new_list[k]->getId())) + "\n", false);
            }

            Utilities::quickSort(&new_list, 0, new_list.length() - 1, probas); //Sort the list based on probabilities

            map.insert(users[j]->getId(), new_list); //Insert the list
            p_map.insert(users[j]->getId(), probas);
        }

        m_poss.insert(selected_subjects->at(i)->getId(), map); //Insert the map
        m_probabilities.insert(selected_subjects->at(i)->getId(), p_map);
    }
}

void FiveWavesMethod::clearPoss() {
    m_poss.clear();

    QList<int> keys_1 = m_probabilities.keys();
    for(int i = 0; i < keys_1.length(); i++) {
        QMap<int, QMap<int, float>* > map = m_probabilities.value(keys_1[i]);
        QList<int> keys_2 = map.keys();
        for(int j = 0; j < keys_2.length(); j++) {
            delete map.take(keys_2[j]);
        }
    }
    m_probabilities.clear();
}

QMap<int, QList<Timeslot *> > *FiveWavesMethod::updatePoss(int id_user, Timeslot* current) {
    /** To update the possibilities of this user based on the kholle that has been fixed **/
    QMap<int, QList<Timeslot*> > *res = new QMap<int, QList<Timeslot*> >; //The old possibilities, sorted by subject

    int i, j, k;
    QList<int> keys_s = m_poss.keys();
    for(i = 0; i < keys_s.length(); i++) { //For every subject
        QMap<int, QList<Timeslot*> > map = m_poss.value(keys_s[i]);
        QList<int> keys_u = map.keys();
        for(j = 0; j < keys_u.length(); j++) { //For every student
            if(keys_u[j] == id_user) { //If it's this student
                QList<Timeslot*> ts = map.value(keys_u[j]);
                QList<Timeslot*> new_ts;

                //Save the old possibilities
                res->insert(keys_s[i], ts);

                //Change the possibilities
                for(k = 0; k < ts.length(); k++) {
                    if(Utilities::compatible(ts[k], current))
                        new_ts.append(ts[k]);
                }
                map.insert(keys_u[j], new_ts);
            }
        }
        m_poss.insert(keys_s[i], map);
    }

    return res;
}

void FiveWavesMethod::resetPoss(int id_user, QMap<int, QList<Timeslot *> > *old) {
    /** To put back the old possibilities **/
    int i;
    QList<int> keys_s = m_poss.keys();

    //Put back the old
    for(i = 0; i < keys_s.length(); i++) { //For every subject
        if(old->contains(keys_s[i])) {
            QMap<int, QList<Timeslot*> > map = m_poss.value(keys_s[i]);

            map.insert(id_user, old->value(keys_s[i]));
            m_poss.insert(keys_s[i], map);
        }
    }
}


working_index *FiveWavesMethod::findMax() {
    /** To find the place with highest score **/
    bool is_empty = true;
    int max = -1;
    QList<int> s_maxs;
    QList<int> u_maxs;

    QList<int> s_keys = m_poss.keys();
    int i, j;
    for(i = 0; i < s_keys.length(); i++) {
        QList<int> u_keys = m_poss.value(s_keys[i]).keys();

        for(j = 0; j < u_keys.length(); j++) {
            int score = Utilities::listMax(m_poss.value(s_keys[i]).value(u_keys[j]), m_probabilities.value(s_keys[i]).value(u_keys[j]));
            if(is_empty || score > max) {
                max = score;
                s_maxs.clear();
                u_maxs.clear();
                s_maxs.append(s_keys[i]);
                u_maxs.append(u_keys[j]);
            }
            else if(score == max) {
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
    return res;
}

Kholle *FiveWavesMethod::create_add_kholle(Student *s, Timeslot *ts) {
    /** Creates a new Kholle with the parameters, adds it to kholloscope, and returns the kholle **/
    //Create new kholle
    Kholle *k = createKholle(s, ts);

    //Add it
    kholloscope()->append(k);
    ts->setPupils(ts->getPupils() - 1); //Substract one person

    return k;
}

bool FiveWavesMethod::wave_1() {
    /** Generate the Kholloscope **/
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
    if(m_abort || m_timeout) {
        free(index);
        return false;
    }

    QMap<int, QList<Timeslot*> > map = m_poss.value(index->current_subject);
    QList<Timeslot*> loop = map.take(index->current_student); //Get possibilities and delete them from the possibilities map
    m_poss.insert(index->current_subject, map);

    //No need to sort again
    //QMap<int, float> *probas = Utilities::corrected_proba(m_dbase, m_dbase->listStudents()->value(index->current_student), loop, m_date); //Do we really need to recalculate ???!
    //Utilities::quickSort(&loop, 0, loop.length() - 1, probas);
    //delete probas;

    for(int i = 0; i < loop.length(); i++) { //For every possibility
        if(loop[i]->getPupils() > 0) { //If enough space
            create_add_kholle(listStudents()->value(index->current_student), loop[i]);

            QMap<int, QList<Timeslot*> > *old = updatePoss(index->current_student, loop[i]); //Update the possibilities

            //Recursive call
            if(wave_1()) { //If true = we are finished
                resetPoss(index->current_student, old);
                delete old;
                map = m_poss.value(index->current_subject);
                map.insert(index->current_student, loop);
                m_poss.insert(index->current_subject, map);
                free(index);
                return true;
            }

            //If abort needed, return false
            if(m_abort || m_timeout) {
                delete old;
                free(index);
                return false;
            }

            //If false, reset everything
            resetPoss(index->current_student, old);
            delete old;
            delete kholloscope()->takeLast();
            loop[i]->setPupils(loop[i]->getPupils() + 1);
        }
    }

    //Put everything back and return
    map = m_poss.value(index->current_subject);
    map.insert(index->current_student, loop);
    m_poss.insert(index->current_subject, map);
    free(index);
    return false;
}


void FiveWavesMethod::force() {
    /** Attributes Kholles randomly **/

    QList<int> s_keys = m_poss.keys();
    for(int i = 0; i < s_keys.length(); i++) {
        QList<int> u_keys = m_poss.value(s_keys[i]).keys();
        for(int j = 0; j < u_keys.length(); j++) {
            bool success = false;

            QList<Timeslot*> loop = m_poss.value(s_keys[i]).value(u_keys[j]);

            for(int k = 0; k < loop.length() && !success; k++) {
                if(loop[k]->getPupils() > 0) {
                    success = true;
                    create_add_kholle(listStudents()->value(u_keys[j]), loop[k]);
                    delete updatePoss(u_keys[j], loop[k]);
                }
            }

            if(!success) {
                foreach(Timeslot* ts, *listTimeslots()) {
                    if(ts->getPupils() > 0) {
                        if(ts->kholleur()->getId_subjects() == s_keys[i]) {
                            if(ts->getDate() >= date() && ts->getDate() <= date().addDays(6)) {
                                //All conditions respected (not compatibility) => insert new Kholle
                                Kholle *k = create_add_kholle(listStudents()->value(u_keys[j]), ts);
                                delete updatePoss(u_keys[j], ts);

                                k->setStatus(Kholle::Impossible);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

bool FiveWavesMethod::treatImpossible(int index, Kholle::Status stat_correct) {
    /** Exchange timeslots between pupils in order to resolve impossible situation **/
    QList<Kholle*> kholloscope = *(this->kholloscope());

    while(index < kholloscope.length() && kholloscope[index]->status() != stat_correct)
        index++;
    if(index >= kholloscope.length())
        return true;

    Kholle* current = kholloscope[index];
    Timeslot* t_current = current->timeslot();
    Student* s_current = current->student();

    int max_index = -1;
    float max_score = 0;

    ///First pass : only exchange if both are compatible
    for(int i = 0; i < kholloscope.length(); i++) {
        Kholle* k = kholloscope[i];
        Timeslot* t = k->timeslot();
        Student* s = k->student();

        if(t_current->kholleur()->getId_subjects() == t->kholleur()->getId_subjects()) {
            if(compatible(s_current->getId(), t, week(), current->getId())
                    && compatible(s->getId(), t_current, week(), k->getId())) {
                float p1 = m_probabilities.value(t_current->kholleur()->getId_subjects()).value(s_current->getId())->value(t->getId());
                float p2 = m_probabilities.value(t->kholleur()->getId_subjects()).value(s->getId())->value(t_current->getId());

                if(p1+p2 > max_score || max_index == -1) {
                    max_score = p1+p2;
                    max_index = i;
                }
            }
        }
    }

    if(max_index != -1) {
        Kholle* k = kholloscope[max_index];
        Timeslot* t = k->timeslot();

        Utilities::make_exchange(m_db, m_dbase, current, t_current, k, t, week(), kholloscope);
        return treatImpossible(index + 1, stat_correct);
    }

    ///Second pass : exchange even if it is still incompatible
    for(int i = 0; i < kholloscope.length(); i++) {
        if(i == index)
            continue;

        Kholle* k = kholloscope[i];
        Timeslot* t = k->timeslot();
        Student* s = k->student();

        if(t->getId() == t_current->getId())
            continue;
        if(current->past_id_timeslots()->contains(t->getId()))
            continue;

        if(t_current->kholleur()->getId_subjects() == t->kholleur()->getId_subjects()) {
            stat_info *info = Kholle::calculateStatus(m_db, m_dbase, s_current->getId(), t, week(), kholloscope, current->getId());
            if(compatible(s->getId(), t_current, week(), k->getId())
                    && info->status <= Kholle::Incompatible) {
                float p = m_probabilities.value(t->kholleur()->getId_subjects()).value(s->getId())->value(t_current->getId());

                if(p > max_score || max_index == -1) {
                    max_score = p;
                    max_index = i;
                }
            }
            free(info);
        }
    }

    if(max_index != -1) {
        Kholle* k = kholloscope[max_index];
        Timeslot* t = k->timeslot();

        Utilities::make_exchange(m_db, m_dbase, current, t_current, k, t, week(), kholloscope);
        current->past_id_timeslots()->append(t_current->getId());
        return treatImpossible(index + 1, stat_correct);
    }

    ///Distinction based on treated case

    if(stat_correct == Kholle::Impossible) {
        ///Third pass : exchange even if both are incompatible

        for(int i = 0; i < kholloscope.length(); i++) {
            Kholle* k = kholloscope[i];
            Timeslot* t = k->timeslot();
            Student* s = k->student();

            if(t_current->kholleur()->getId_subjects() == t->kholleur()->getId_subjects()) {
                stat_info *info1 = Kholle::calculateStatus(m_db, m_dbase, s_current->getId(), t, week(), kholloscope, current->getId());
                stat_info *info2 = Kholle::calculateStatus(m_db, m_dbase, s->getId(), t_current, week(), kholloscope, k->getId());
                if(info1->status <= Kholle::Incompatible
                        && info2->status <= Kholle::Incompatible) {

                    Utilities::make_exchange(m_db, m_dbase, current, t_current, k, t, week(), kholloscope);
                    free(info1);
                    free(info2);
                    return treatImpossible(index + 1, stat_correct);
                }
                free(info1);
                free(info2);
            }
        }

        ///Fourth pass : exchange if current still impossible

        for(int i = 0; i < kholloscope.length(); i++) {
            if(i == index)
                continue;

            Kholle* k = kholloscope[i];
            Timeslot* t = k->timeslot();
            Student* s = k->student();

            if(t->getId() == t_current->getId())
                continue;
            if(current->past_id_timeslots()->contains(t->getId()))
                continue;

            if(t_current->kholleur()->getId_subjects() == t->kholleur()->getId_subjects()) {
                stat_info *info = Kholle::calculateStatus(m_db, m_dbase, s->getId(), t_current, week(), kholloscope, k->getId());
                if(info->status <= Kholle::Incompatible) {

                    Utilities::make_exchange(m_db, m_dbase, current, t_current, k, t, week(), kholloscope);
                    current->past_id_timeslots()->append(t_current->getId());
                    free(info);
                    return treatImpossible(index + 1, stat_correct);
                }
                free(info);
            }
        }
    }

    if(stat_correct == Kholle::Incompatible) {
        ///Third pass : transpose incompatibility

        current->updateStatus(m_dbase, m_db, kholloscope, week());

        if(current->status() == Kholle::Incompatible && current->id_pb_kholle() != -1) {
            for(int i = 0; i < kholloscope.length(); i++) {
                if(kholloscope[i]->getId() == current->id_pb_kholle()) {
                    kholloscope[i]->updateStatus(m_dbase, m_db, kholloscope, week());

                    int weeks = current->nearest(m_dbase->listTimeslots(), m_db);
                    current->setStatus((Kholle::Status) Kholle::correspondingStatus(weeks));
                    current->setWeeks(weeks);
                    current->setId_pb_kholle(-1);

                    break;
                }
            }
        }
    }

    return treatImpossible(index + 1, stat_correct);
}

int FiveWavesMethod::remainImpossible(Kholle::Status stat) {
    /** Return (-1) if nothing remains, else return the index of problem **/
    for(int i = 0; i < kholloscope()->length(); i++) {
        if(kholloscope()->at(i)->status() == stat) {
            return i;
        }
    }
    return -1;
}


bool FiveWavesMethod::exchange(int index, ExchangeType type, int score_limit) {
    /** Exchange timeslots between people in the kholloscope to improve score **/

    switch(type) {
        case Warnings:
            while(index < kholloscope()->length() && kholloscope()->at(index)->status() == Kholle::OK)
                index++;
            break;
        case Collisions:
            while(index < kholloscope()->length() &&
                  Utilities::sum_day(m_db, kholloscope()->at(index)->getId_students(), kholloscope()->at(index)->timeslot()->getDate()) <= MaxWeightSubject)
                index++;
            break;
        default:
            break;
    }

    if(index >= kholloscope()->length())
        return true;

    Kholle* current = kholloscope()->at(index);
    Timeslot* t_current = current->timeslot();
    Student* s_current = current->student();

    float max_score = 0;
    int max_index = -1;
    for(int i = 0; i < kholloscope()->length(); i++) {
        if(i == index)
            continue;

        Kholle* k = kholloscope()->at(i);
        Timeslot* t = k->timeslot();
        Student* s = k->student();
        if(m_downgraded.contains(s->getId()) && m_downgraded.value(s->getId()))
            continue;

        if(type == Collisions && t_current->getDate() == t->getDate())
            continue;

        if(t_current->kholleur()->getId_subjects() == t->kholleur()->getId_subjects()) {
            if(compatible(s_current->getId(), t, week(), current->getId())
                    && compatible(s->getId(), t_current, week(), k->getId())) {
                int sub_weight = t_current->kholleur()->subject()->getWeight();
                int w_current_old = Utilities::sum_day(m_db, s_current->getId(), t_current->getDate());
                int w_current_new = Utilities::sum_day(m_db, s_current->getId(), t->getDate()) + (t_current->getDate() != t->getDate() ? sub_weight : 0);
                int w_old = Utilities::sum_day(m_db, s->getId(), t->getDate());
                int w_new = Utilities::sum_day(m_db, s->getId(), t_current->getDate()) + (t_current->getDate() != t->getDate() ? sub_weight : 0);

                QMap<int, float> *p_current = m_probabilities.value(t_current->kholleur()->getId_subjects()).value(s_current->getId());
                QMap<int, float> *p = m_probabilities.value(t->kholleur()->getId_subjects()).value(s->getId());

                stat_info *info1 = Kholle::calculateStatus(m_db, m_dbase, s_current->getId(), t, week(), *kholloscope(), current->getId());
                stat_info *info2 = Kholle::calculateStatus(m_db, m_dbase, s->getId(), t_current, week(), *kholloscope(), k->getId());

                bool weight_ok = false, status_ok = false, probas_ok = false;
                if(type == Collisions) {
                    weight_ok = (w_current_new <= MaxWeightSubject || w_current_new < w_current_old)
                                    && (w_new <= MaxWeightSubject || w_new < w_old);
                    status_ok = (info1->status <= current->status()
                                    && info2->status <= k->status());
                    probas_ok = (p_current->value(t->getId()) + p->value(t_current->getId())
                                    >= p_current->value(t_current->getId()) + p->value(t->getId()));
                }

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
        Kholle* k = kholloscope()->at(max_index);
        Timeslot* t = k->timeslot();
        Student* s = k->student();

        m_downgraded.insert(s->getId(), true);
        m_downgraded.insert(s_current->getId(), false);

        Utilities::make_exchange(m_db, m_dbase, current, t_current, k, t, week(), *kholloscope());
    }

    return exchange(index + 1, type, score_limit);
}

void FiveWavesMethod::timeout() {
    m_timeout = true;
}

void FiveWavesMethod::displayBlocking() {
    if(last_index.current_student == -1)
        return;

    QString message = "La génération a été interrompue ou n’a pas pu être finie. Le dernier élément sur lequel a travaillé le logiciel est le couple suivant : <br />";
    message += "Matière : <strong>" + m_dbase->listSubjects()->value(last_index.current_subject)->getName() + "</strong> <br />";
    message += "Élève : <strong>" + m_dbase->listStudents()->value(last_index.current_student)->getName() + ", "
            + m_dbase->listStudents()->value(last_index.current_student)->getFirst_name() + "</strong> <br />";
    message += "Il est donc probable qu’il y ait une incohérence au niveau de cette matière ou de cet/cette élève. "
               "Veuillez vérifier les horaires de kholles, groupes, emplois du temps correspondants.";

    QMessageBox::warning(NULL, "La génération n'a pas abouti...", message);
}
