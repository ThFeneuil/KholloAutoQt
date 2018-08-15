#include "lpmethod.h"

LPMethod::LPMethod(QSqlDatabase *db, QDate date, int week) : GenerationMethod(db, date, week)
{

}

void LPMethod::start(QList<Subject*> *selected_subjects, QMap<int, QList<Student*> > *input) {
    log("GENERATION DU " + date().toString("dd/MM/yyyy"), true);
    log("\n\nDébut de la génération...\n", true);
    bool success = generate(selected_subjects, input);
    saveInSql();
    setKhollesStatus();

    treatCollision(0, 50);

    if(m_abort) {
        emit generationEnd(GEN_CANCELLED);
        return;
    }

    emit generationEnd(success ? GEN_SUCCESS : GEN_FAIL);
}

bool LPMethod::generate(QList<Subject*> *selected_subjects, QMap<int, QList<Student*> > *input) {
    glp_term_hook(lpMethodsaveLog, this);

    //Transform input
    QMap<int, QVector<int> > map_students_subjects; //Subjects vector corresponding to each Student

    for(int i = 0; i < selected_subjects->length(); i++) {
        Subject *sub = selected_subjects->at(i);
        QList<Student*> selected_students = input->value(sub->getId());

        for(int j = 0; j < selected_students.length(); j++) {
            Student *student = selected_students.at(j);

            QVector<int> student_subs = map_students_subjects.value(student->getId());
            student_subs.append(sub->getId()); //Add this subject to this student
            map_students_subjects.insert(student->getId(), student_subs);
        }
    }

    //Tracking variables
    QMap<int, QMap<int, QVector<int> > > map_students_vars; //variable indexes corresponding to each (Student, Subject)
    QMap<int, QVector<int> > map_timeslots_vars; //variable indexes corresponding to each Timeslot
    QMap<int, Kholle*> map_vars_kholles; //kholle corresponding to each variable index
    int numrows = 0, numcols = 0;

    //Set up the problem
    glp_prob *P;
    P = glp_create_prob();
    glp_set_obj_dir(P, GLP_MAX); //we want to maximize the score

    //Go through every (student, subject) couple
    foreach(Student *s, *listStudents()) {
        QVector<int> selected_subjects = map_students_subjects.value(s->getId());
        QMap<int, QVector<int>> map;
        QMap<int, float> p_map;

        for(int j = 0; j < selected_subjects.length(); j++) {
            int id_sub = selected_subjects[j];
            QVector<int> vect;

            foreach(Timeslot* ts, *listTimeslots()) { //For every timeslot
                if(ts->kholleur()->getId_subjects() == id_sub
                        && ts->getDate() >= date()
                        && ts->getDate() <= date().addDays(6)
                        && compatible(s->getId(), ts, week())) { //Add the compatible timeslots

                    glp_add_cols(P, 1); //add a variable
                    numcols++;
                    glp_set_col_kind(P, numcols, GLP_BV); //the variable is binary (either timeslot is selected (1) or not (0))
                    p_map.insert(ts->getId(), proba(s, ts, date())); //add proba to (student, timeslot)
                    glp_set_obj_coef(P, numcols, p_map.value(ts->getId())); //the coef is the probability
                    qDebug() << p_map.value(ts->getId());

                    vect.append(numcols); //add the variable to (student, subject)

                    //add the variable to timeslot
                    QVector<int> list_ts_vars = map_timeslots_vars.value(ts->getId());
                    list_ts_vars.append(numcols);
                    map_timeslots_vars.insert(ts->getId(), list_ts_vars);

                    //create kholle and affect it to variable
                    map_vars_kholles.insert(numcols, createKholle(s, ts));

                    //Add compatibility constraint
                    for(int k = 0; k < j; k++) { //For all previous subjects
                        QVector<int> vars = map.value(selected_subjects[k]);
                        for(int l = 0; l < vars.length(); l++) {
                            Timeslot *ts2 = map_vars_kholles.value(vars[l])->timeslot();
                            if(!Utilities::compatible(ts, ts2)) {
                                //Add constraint
                                glp_add_rows(P, 1);
                                numrows++;
                                glp_set_row_bnds(P, numrows, GLP_UP, 0, 1);
                                QVector<int> affected;
                                affected << vars[l] << numcols;
                                set_constraint_row(P, numrows, affected);
                            }
                        }
                    }
                }
            }
            map.insert(id_sub, vect);

            //Add constraint => exactly one kholle for (student, subject) couple
            glp_add_rows(P, 1); //add a constraint
            numrows++;
            glp_set_row_bnds(P, numrows, GLP_FX, 1, 1); //fixed to 1
            set_constraint_row(P, numrows, vect);
        }
        map_students_vars.insert(s->getId(), map);
        m_probabilities.insert(s->getId(), p_map);
    }


    //Add constraint => at most ts->getPupils() pupils for ts
    foreach(Timeslot* ts, *listTimeslots()) {
        if(map_timeslots_vars.contains(ts->getId())) {
            glp_add_rows(P, 1);
            numrows++;
            glp_set_row_bnds(P, numrows, GLP_UP, 0, ts->getPupils());
            set_constraint_row(P, numrows, map_timeslots_vars.value(ts->getId()));
        }
    }

    //Run algorithm
    glp_simplex(P, NULL);
    glp_intopt(P, NULL);
    qDebug() << glp_get_obj_val(P);

    if(glp_mip_status(P) != GLP_OPT) {
        foreach(Kholle *k, map_vars_kholles)
            delete k;

        glp_delete_prob(P);
        return false;
    }

    for(int i = 1; i <= numcols; i++) {
        if(glp_mip_col_val(P, i)) {
            kholloscope()->append(map_vars_kholles.value(i));
        }
        else {
            delete map_vars_kholles.take(i);
        }
    }

    glp_delete_prob(P);
    return true;
}

void LPMethod::set_constraint_row(glp_prob *P, int i, QVector<int> vect) {
    QVector<int> ind(vect);
    ind.prepend(0); //arrays in GLPK start at 1

    QVector<double> vals(ind.length(), 1);

    glp_set_mat_row(P, i, vect.length(), ind.data(), vals.data());
}

int lpMethodsaveLog(void *info, const char *s) {
    LPMethod* m = (LPMethod*) info;
    m->log(s, true);
    return 1;
}

void LPMethod::treatCollision(int index, int score_limit) {
    /** Exchange timeslots between people in the kholloscope solve collisions **/

    while(index < kholloscope()->length() &&
          Utilities::sum_day(m_db, kholloscope()->at(index)->getId_students(), kholloscope()->at(index)->timeslot()->getDate()) <= MaxWeightSubject)
        index++;

    if(index >= kholloscope()->length())
        return;

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

        if(t_current->getDate() == t->getDate())
            continue;

        if(t_current->kholleur()->getId_subjects() == t->kholleur()->getId_subjects()) {
            if(compatible(s_current->getId(), t, week(), current->getId())
                    && compatible(s->getId(), t_current, week(), k->getId())) {
                int sub_weight = t_current->kholleur()->subject()->getWeight();
                int w_current_old = Utilities::sum_day(m_db, s_current->getId(), t_current->getDate());
                int w_current_new = Utilities::sum_day(m_db, s_current->getId(), t->getDate()) + (t_current->getDate() != t->getDate() ? sub_weight : 0);
                int w_old = Utilities::sum_day(m_db, s->getId(), t->getDate());
                int w_new = Utilities::sum_day(m_db, s->getId(), t_current->getDate()) + (t_current->getDate() != t->getDate() ? sub_weight : 0);

                QMap<int, float> p_current = m_probabilities.value(s_current->getId());
                QMap<int, float> p = m_probabilities.value(s->getId());

                stat_info *info1 = Kholle::calculateStatus(m_db, m_dbase, s_current->getId(), t, week(), *kholloscope(), current->getId());
                stat_info *info2 = Kholle::calculateStatus(m_db, m_dbase, s->getId(), t_current, week(), *kholloscope(), k->getId());

                bool weight_ok = (w_current_new <= MaxWeightSubject || w_current_new < w_current_old)
                                && (w_new <= MaxWeightSubject || w_new < w_old);
                bool status_ok = (info1->status <= current->status()
                                && info2->status <= k->status());
                bool probas_ok = (p_current.value(t->getId()) + p.value(t_current->getId())
                                - p_current.value(t_current->getId()) - p.value(t->getId()))
                                    >= -score_limit;

                if(weight_ok && status_ok && probas_ok) {
                    float delta_proba = p_current.value(t->getId()) + p.value(t_current->getId())
                                            - p_current.value(t_current->getId()) - p.value(t->getId());
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

    return treatCollision(index + 1, score_limit);
}
