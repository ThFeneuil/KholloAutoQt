#include "generatepage.h"
#include "ui_generatepage.h"

GeneratePage::GeneratePage(QSqlDatabase *db, QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::GeneratePage)
{
    ui->setupUi(this);

    //DB
    m_db = db;

    //Create message
    m_box = new QMessageBox(QMessageBox::Information, "Génération automatique", "Génération en cours. Veuillez patienter...",
                            QMessageBox::NoButton, this);

    //Set variables
    log_file = NULL;
    m_dbase = NULL;

    //Pointer to MainWindow
    m_window = parent;

    //Connect buttons to slots
    connect(ui->notepad_khollo, SIGNAL(clicked()), this, SLOT(show_notepad_khollo()));
    connect(ui->notepad_collisions, SIGNAL(clicked()), this, SLOT(show_notepad_collisions()));
}

GeneratePage::~GeneratePage() {
    m_db->rollback();

    delete ui;

    if(m_dbase != NULL)
        delete m_dbase;

    delete m_box;
    freeKholles();
}

void GeneratePage::initializePage() {
    //Connect the finish button to save on finish
    connect(wizard()->button(QWizard::FinishButton), SIGNAL(clicked()), this, SLOT(saveKholles()));

    m_week = field("current_week").toInt() + 1;
    m_date = field("monday_date").toDateTime().date();
    if(m_date.dayOfWeek() != 1) {
        while(m_date.dayOfWeek() != 1)
            m_date = m_date.addDays(-1);
        QMessageBox::information(this, "Information", "La date n'étant pas à un lundi, la date utilisée sera le lundi de la même semaine.");
    }

    freeKholles();

    //Open log file
    log_file = new QFile(QCoreApplication::applicationDirPath() + QDir::separator() + "gen_log.txt");
    if(!log_file->open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate)) {
        delete log_file;
        log_file = NULL;
    }

    QTextStream out(log_file);

    //Reinitialise DataBase object
    if(m_dbase != NULL)
        delete m_dbase;
    m_dbase = new DataBase(m_db);
    m_dbase->load();

    setPupilsOnTimeslots();

    //Warning if not enough free space for everyone...
    QList<Subject*> *problem_subjects = testAvailability();
    if(problem_subjects->length() > 0) {
        QString warning_text = "Il n'y a pas suffisamment d'horaires de kholles libres pour accommoder tous les élèves sélectionnés dans ";
        warning_text += (problem_subjects->length() >= 2 ? "les matières suivantes : <br />" : "la matière suivante : <br />");
        int i;
        for(i = 0; i < problem_subjects->length(); i++)
            warning_text += "<strong>" + problem_subjects->at(i)->getName() + "</strong> <br />";
        QMessageBox::warning(this, "Attention", warning_text);
    }
    delete problem_subjects;

    if(log_file != NULL)
        out << "\n\n Début de la génération...\n";

    m_box->show();
    //Start generating
    finished(generate());
}

void GeneratePage::cleanupPage() {
    disconnect(wizard()->button(QWizard::FinishButton), SIGNAL(clicked()), this, SLOT(saveKholles()));

    //Abort operation
    m_db->rollback();
    m_box->hide();

    //Clear list
    ui->tableKhollo->clear();
}

void GeneratePage::setPupilsOnTimeslots() {
    //Get timeslots
    QMap<int, Timeslot*> *map = m_dbase->listTimeslots();

    //Loop through all timeslots and update number of pupils
    foreach(Timeslot* ts, *map) {
        if(ts->getDate() >= m_date && ts->getDate() <= m_date.addDays(6))
            ts->setPupils(ts->getPupils() - ts->kholles()->length());
    }
}

QList<Subject*>* GeneratePage::testAvailability() {
    /** Tests if there is enough free space for everyone selected **/

    int i;
    QList<Subject*> *result = new QList<Subject*>;
    QList<Subject*> *selected_subjects = ((KholloscopeWizard*) wizard())->get_assoc_subjects();
    QMap<int, QList<Student*> > *input = ((KholloscopeWizard*) wizard())->get_input();
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

Kholle *GeneratePage::createKholle(int id_student, Timeslot *ts) {
    /** Creates a new Kholle with the parameters and returns the kholle **/
    //Create new kholle
    Kholle *k = new Kholle();
    k->setId_students(id_student);
    k->setId_timeslots(ts->getId());
    k->setTimeslot(ts);

    return k;
}

void GeneratePage::set_constraint_row(glp_prob *P, int i, QVector<int> vect) {
    QVector<int> ind(vect);
    ind.prepend(0); //arrays in GLPK start at 1

    QVector<double> vals(ind.length(), 1);

    glp_set_mat_row(P, i, vect.length(), ind.data(), vals.data());
}

bool GeneratePage::generate() {
    /** Generate the Kholloscope **/

    //Get selected subjects
    QList<Subject*> *selected_subjects = ((KholloscopeWizard*) wizard())->get_assoc_subjects();

    //Get input
    QMap<int, QList<Student*> > *input = ((KholloscopeWizard*) wizard())->get_input();

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
    foreach(Student *s, *m_dbase->listStudents()) {
        QVector<int> selected_subjects = map_students_subjects.value(s->getId());
        QMap<int, QVector<int>> map;

        for(int j = 0; j < selected_subjects.length(); j++) {
            int id_sub = selected_subjects[j];
            QVector<int> vect;

            foreach(Timeslot* ts, *m_dbase->listTimeslots()) { //For every timeslot
                if(ts->kholleur()->getId_subjects() == id_sub
                        && ts->getDate() >= m_date
                        && ts->getDate() <= m_date.addDays(6)
                        && Utilities::compatible(m_db, m_dbase, s->getId(), ts, m_week)) { //Add the compatible timeslots

                    glp_add_cols(P, 1); //add a variable
                    numcols++;
                    glp_set_col_kind(P, numcols, GLP_BV); //the variable is binary (either timeslot is selected (1) or not (0))
                    glp_set_obj_coef(P, numcols, Utilities::proba(m_dbase, s, ts, m_date)); //the coef is the probability

                    vect.append(numcols); //add the variable to (student, subject)

                    //add the variable to timeslot
                    QVector<int> list_ts_vars = map_timeslots_vars.value(ts->getId());
                    list_ts_vars.append(numcols);
                    map_timeslots_vars.insert(ts->getId(), list_ts_vars);

                    //create kholle and affect it to variable
                    map_vars_kholles.insert(numcols, createKholle(s->getId(), ts));

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
    foreach(Timeslot* ts, *m_dbase->listTimeslots()) {
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
            kholloscope.append(map_vars_kholles.value(i));
        }
        else {
            delete map_vars_kholles.take(i);
        }
    }

    return true;
}

void GeneratePage::finished(bool success) {
    /** Called when the background process finishes **/
    timestamp = QString::number(QDateTime::currentMSecsSinceEpoch());
    QTextStream out(log_file);

    if(!success) {
        if(log_file != NULL)
            out << "Erreur ! Il n'y a aucune solution...\n";
        m_box->hide();
        QMessageBox::critical(this, "Erreur", "Solution non trouvée... :'(");
        return;
    }

    if(log_file != NULL)
        out << "Fin du premier jet...\n" << "Timestamp : " << timestamp << "\n\n";

    m_db->transaction();
    Utilities::saveInSql(m_db, &kholloscope);

    setStatus();
    /*m_downgraded.clear();

    if(log_file != NULL)
        out << "Traitement des collisions...\n";
    exchange(0, Collisions, 0);

    if(log_file != NULL)
        out << "Traitement des furieux et déçus...\n";
    exchange(0, Warnings, 40);

    if(log_file != NULL)
        out << "Amélioration du score...\n\n";
    exchange(0, All, 15);*/


    if(log_file != NULL)
        out << "Affichage du kholloscope...\n";
    int errors = 0, warnings = 0, collisions = 0;
    display(&errors, &warnings);
    displayCollision(&collisions);
    m_box->hide();

    if(log_file != NULL)
        out << "Affichage de la conclusion...\n" << "Génération réussie !!\n\n";
    displayConclusion(errors, warnings, collisions);

    if(log_file != NULL) {
        delete log_file;
        log_file = NULL;
    }

}

void GeneratePage::setStatus() {
    /** Set the status of the generated kholles **/

    for(int i = 0; i < kholloscope.length(); i++) {
        if(kholloscope[i]->status() >= Kholle::Incompatible)
            kholloscope[i]->updateStatus(m_dbase, m_db, kholloscope, m_week);
        else {
            int weeks = kholloscope[i]->nearest(m_dbase->listTimeslots(), m_db);
            kholloscope[i]->setStatus((Kholle::Status) Kholle::correspondingStatus(weeks));
            kholloscope[i]->setWeeks(weeks);
            kholloscope[i]->setId_pb_kholle(-1);
        }
    }
}

bool GeneratePage::exchange(int index, ExchangeType type, int score_limit) {
    /** Exchange timeslots between people in the kholloscope to improve score **/

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
                }*/

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
                }*/

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

    return exchange(index + 1, type, score_limit);
}


void GeneratePage::display(int *errors, int *warnings) {
    /** To display in the list **/
    ui->tableKhollo->clear();
    ui->tableKhollo->setRowCount(kholloscope.length());
    ui->tableKhollo->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    khollo_message  = "========== Génération du " + m_date.toString("dd/MM/yyyy") + " ==========\n";
    khollo_message += "                                    Furieux et déçus\n\n";

    for(int i = 0; i < kholloscope.length(); i++) {
        Student* student = m_dbase->listStudents()->value(kholloscope[i]->getId_students());
        Timeslot* timeslot = m_dbase->listTimeslots()->value(kholloscope[i]->getId_timeslots());
        Subject* subject = timeslot->kholleur()->subject();
        Kholleur* kholleur = timeslot->kholleur();

        int weeks = kholloscope[i]->weeks();
        QTableWidgetItem *left = new QTableWidgetItem(student->getName() + ", " + subject->getShortName() + " :");

        if(kholloscope[i]->status() == Kholle::OK) {
            left->setIcon(QIcon(QPixmap(":/images/ok.png")));

            if(weeks == -1) {
                QTableWidgetItem *right = new QTableWidgetItem(kholleur->getName() + ", jamais");
                ui->tableKhollo->setItem(i, 1, right);
            }
            else {
                QTableWidgetItem *right = new QTableWidgetItem(kholleur->getName() + ", " + QString::number(weeks) + " semaines");
                ui->tableKhollo->setItem(i, 1, right);
            }

        }
        else if(kholloscope[i]->status() == Kholle::Error) {
            left->setIcon(QIcon(QPixmap(":/images/error.png")));

            QTableWidgetItem *right = new QTableWidgetItem(kholleur->getName() + ", " + QString::number(weeks) + " semaine");
            ui->tableKhollo->setItem(i, 1, right);

            khollo_message += student->getName() + " " + student->getFirst_name() + ", " + subject->getShortName() + " : Furieux\n";
            (*errors)++;
        }
        else {
            left->setIcon(QIcon(QPixmap(":/images/warning.png")));

            QTableWidgetItem *right = new QTableWidgetItem(kholleur->getName() + ", " + QString::number(weeks) + " semaines");
            ui->tableKhollo->setItem(i, 1, right);

            khollo_message += student->getName() + " " + student->getFirst_name() + ", " + subject->getShortName() + " : Déçu\n";
            (*warnings)++;
        }
        ui->tableKhollo->setItem(i, 0, left);
    }
}

void GeneratePage::displayCollision(int *collisions) {
    /** To display when kholles are on the same day **/
    delete m_dbase;
    m_dbase = new DataBase(m_db);
    m_dbase->setConditionTimeslots("`date` >= '"+ m_date.toString("yyyy-MM-dd") +"' AND `date` < '" + m_date.addDays(7).toString("yyyy-MM-dd") + "'");
    m_dbase->load();

    collisions_message  = "========== Génération du " + m_date.toString("dd/MM/yyyy") + " ==========\n";
    collisions_message += "                                         Collisions\n\n";

    ui->tableCollision->clear();
    ui->tableCollision->setRowCount(0);
    ui->tableCollision->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QStringList days;
    days << "" << "Lundi" << "Mardi" << "Mercredi" << "Jeudi" << "Vendredi" << "Samedi" << "Dimanche";

    QMap<int, QList< QList<Kholle*>* >* > map;

    foreach(Kholle* k, *m_dbase->listKholles()) {
        if(!map.contains(k->getId_students())) {
            QList<QList<Kholle*>* > *list = new QList<QList<Kholle*>* >();
            for(int i = 0; i <= 7; i++) {
                list->append(new QList<Kholle*>());
            }
            map.insert(k->getId_students(), list);
        }

        map.value(k->getId_students())->at(k->timeslot()->getDate().dayOfWeek())->append(k);
    }

    foreach(QList<QList<Kholle*>* > *list, map) {
        for(int i = 1; i <= 7; i++) {
            QList<Kholle*> *day = list->at(i);
            int total = 0;
            for(int j = 0; j < day->length(); j++) {
                total += day->at(j)->timeslot()->kholleur()->subject()->getWeight();
            }

            if(day->length() > 0 && total > MaxWeightSubject) {
                int r = ui->tableCollision->rowCount();
                ui->tableCollision->setRowCount(r + day->length());

                Student *s = day->at(0)->student();
                QTableWidgetItem *left = new QTableWidgetItem(s->getName() + ", " + days[i] + " (" + QString::number(total) + ")");
                ui->tableCollision->setItem(r, 0, left);
                collisions_message += s->getName() + " " + s->getFirst_name() + ", " + days[i] + " : ";

                for(int j = 0; j < day->length(); j++) {
                    QString sub_name = day->at(j)->timeslot()->kholleur()->subject()->getShortName();
                    QTableWidgetItem *right = new QTableWidgetItem(sub_name + " " + day->at(j)->timeslot()->getTime_start().toString());
                    ui->tableCollision->setItem(r + j, 1, right);
                    collisions_message += sub_name + " / ";
                }
                collisions_message.chop(3);
                collisions_message += "\n";
                (*collisions)++;
            }
        }
    }
}

void GeneratePage::displayConclusion(int errors, int warnings, int collisions) {
    QMessageBox::information(this, "Génération terminée", "La génération a abouti et a créé un kholloscope avec"
                                                          "<ul><li>" + QString::number(errors) + " furieux" +
                                                          "<li>" + QString::number(warnings) + ((warnings <= 1) ? " déçu" : " déçus") +
                                                          "<li>" + QString::number(collisions) + ((collisions <= 1) ? " collision" : " collisions") + "</ul>");
}

void GeneratePage::show_notepad_khollo() {
    if(timestamp != "" && khollo_message != "") {
        Notepad::add(timestamp, khollo_message);
    }
}

void GeneratePage::show_notepad_collisions() {
    if(timestamp != "" && collisions_message != "") {
        Notepad::add(timestamp, collisions_message);
    }
}

void GeneratePage::saveKholles() {
    /** To save the kholloscope that has been generated to the database and other things after finish **/

    //Only if checkbox selected...
    if(ui->checkBox->isChecked()) {
        QMessageBox box(QMessageBox::Information, "Sauvegarde en cours", "Veuillez patienter...");
        box.show();
        m_db->commit();

        box.hide();
    }

    if(ui->printCheckBox->isChecked()) {
        //Create QMap of Kholles
        QMap<int, Kholle*> *khollo = new QMap<int, Kholle*>;

        //Add the other kholles of the week for printing
        foreach(Kholle* k, *m_dbase->listKholles()) {
            if(k->timeslot()->getDate() >= m_date && k->timeslot()->getDate() <= m_date.addDays(6))
                khollo->insert(k->getId(), k);
        }



        //Print!
        //QMessageBox::information(this, "OK", QString::number(kholloscope.length()));
        PrintPDF::printKholles(((KholloscopeWizard*) wizard())->get_students(), m_dbase->listKholleurs(), m_dbase->listTimeslots(), m_date, khollo);
    }

    //Open interface if checkbox selected
    if(ui->interfaceCheckBox->isChecked()) {
        ((MainWindow*) m_window)->triggerInterface(m_date, m_week);
    }
}

void GeneratePage::freeKholles() {
    while(!kholloscope.isEmpty()) {
        delete kholloscope.takeFirst();
    }
}

