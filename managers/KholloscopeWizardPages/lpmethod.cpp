#include "lpmethod.h"

LPMethod::LPMethod(QSqlDatabase *db, QDate date, int week) : GenerationMethod(db, date, week)
{

}

void LPMethod::start(QList<Subject*> *selected_subjects, QMap<int, QList<Student*> > *input) {

    testAvailability(selected_subjects, input);

    log("\n\nDébut de la génération...\n", true);
    bool success = generate(selected_subjects, input);
    saveInSql();
    setKhollesStatus();

    emit generationEnd(success ? 0 : 1);
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
                    glp_set_obj_coef(P, numcols, proba(s, ts, date())); //the coef is the probability

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

    if(glp_mip_status(P) != GLP_OPT) {
        foreach(Kholle *k, map_vars_kholles)
            delete k;
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
