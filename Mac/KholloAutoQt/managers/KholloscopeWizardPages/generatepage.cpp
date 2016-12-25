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
                            QMessageBox::Cancel, this);

    //Set variables
    last_index.current_student = -1; //Set to impossible value
    log_file = NULL;
    m_dbase = NULL;

    //Pointer to MainWindow
    m_window = parent;

    //Connect buttons to slots
    connect(ui->notepad_khollo, SIGNAL(clicked()), this, SLOT(show_notepad_khollo()));
    connect(ui->notepad_collisions, SIGNAL(clicked()), this, SLOT(show_notepad_collisions()));
}

GeneratePage::~GeneratePage() {
    m_abort = true;
    m_watcher.waitForFinished();
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

    //Initialise random number generator
    qsrand((uint)QTime::currentTime().msecsSinceStartOfDay());

    m_week = field("current_week").toInt() + 1;
    m_date = field("monday_date").toDateTime().date();
    if(m_date.dayOfWeek() != 1) {
        while(m_date.dayOfWeek() != 1)
            m_date = m_date.addDays(-1);
        QMessageBox::information(this, "Information", "La date n'étant pas à un lundi, la date utilisée sera le lundi de la même semaine.");
    }

    profondeur = 0;
    m_abort = false;
    freeKholles();

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

    //Connect watcher to slot...
    connect(&m_watcher, SIGNAL(finished()), this, SLOT(finished()));

    //Open log file
    log_file = new QFile(QCoreApplication::applicationDirPath() + QDir::separator() + "gen_log.txt");
    if(!log_file->open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate)) {
        delete log_file;
        log_file = NULL;
    }

    //Prepare the table
    constructPoss();

    //Start work in separate thread
    QFuture<bool> future = QtConcurrent::run(this, &GeneratePage::generate);
    m_watcher.setFuture(future);

    //Display a message with an option to abort
    connect(m_box, SIGNAL(finished(int)), this, SLOT(abort()));
    m_box->show();
}

void GeneratePage::cleanupPage() {
    disconnect(wizard()->button(QWizard::FinishButton), SIGNAL(clicked()), this, SLOT(saveKholles()));

    //Abort operation
    m_abort = true;
    m_watcher.waitForFinished();
    m_db->rollback();

    m_box->hide();
    disconnect(m_box, SIGNAL(finished(int)), this, SLOT(abort()));
    disconnect(&m_watcher, SIGNAL(finished()), this, SLOT(finished()));

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

void GeneratePage::constructPoss() {
    /** To construct the 3D table of possibilities - a list for every (Student, Subject) couple **/

    //Create stream around log_file
    QTextStream out(log_file);

    //Get selected subjects
    QList<Subject*> *selected_subjects = ((KholloscopeWizard*) wizard())->get_assoc_subjects();

    //Get input
    QMap<int, QList<Student*> > *input = ((KholloscopeWizard*) wizard())->get_input();

    int i, j, k;
    for(i = 0; i < selected_subjects->length(); i++) { //For every selected subject
        //Create a new map
        QMap<int, QList<Timeslot*> > map;

        //The selected users for this subject
        QList<Student*> users = input->value(selected_subjects->at(i)->getId());

        for(j = 0; j < users.length(); j++) { //For every selected user
            //Create new list
            QList<Timeslot*> new_list;

            foreach(Timeslot* ts, *m_dbase->listTimeslots()) { //For every timeslot
                if(ts->kholleur()->getId_subjects() == selected_subjects->at(i)->getId()
                        && ts->getDate() >= m_date
                        && ts->getDate() <= m_date.addDays(6)
                        && Utilities::compatible(m_db, m_dbase, users[j]->getId(), ts, m_week)) { //Add the compatible timeslots
                    new_list.append(ts);
                }
            }

            //Log the probability for these timeslots
            if(log_file != NULL) {
                QMap<int, float> *probas = Utilities::corrected_proba(m_dbase, m_dbase->listStudents()->value(users[j]->getId()), new_list, m_date);
                for(k = 0; k < new_list.length(); k++) {
                    out << users[j]->getName() << ", " << selected_subjects->at(i)->getShortName() << ", " << new_list[k]->kholleur()->getName() << ", " << new_list[k]->getId() << " : ";
                    out << probas->value(new_list[k]->getId()) << "\n";
                }
                delete probas;
            }

            // No sorting here !
            //quickSort(&new_list, 0, new_list.length() - 1, users[j]->getId()); //Sort the list based on probabilities

            map.insert(users[j]->getId(), new_list); //Insert the list
        }

        poss.insert(selected_subjects->at(i)->getId(), map); //Insert the map
    }
}

QMap<int, QList<Timeslot *> > *GeneratePage::updatePoss(int id_user, Timeslot* current) {
    /** To update the possibilities of this user based on the kholle that has been fixed **/
    QMap<int, QList<Timeslot*> > *res = new QMap<int, QList<Timeslot*> >; //The old possibilities, sorted by subject

    int i, j, k;
    QList<int> keys_s = poss.keys();
    for(i = 0; i < keys_s.length(); i++) { //For every subject
        QMap<int, QList<Timeslot*> > map = poss.value(keys_s[i]);
        QList<int> keys_u = map.keys();
        for(j = 0; j < keys_u.length(); j++) { //For every student
            if(keys_u[j] == id_user) { //If it's this student
                QList<Timeslot*> ts = map.value(keys_u[j]);
                QList<Timeslot*> new_ts;

                //Save the old possibilities
                res->insert(keys_s[i], ts);

                //Change the possibilities
                for(k = 0; k < ts.length(); k++) {
                    if(ts[k]->getDate() == current->getDate()) {
                        if((ts[k]->getTime_start() <= current->getTime_start() && ts[k]->getTime_end() > current->getTime_start())
                                || (ts[k]->getTime_start() < current->getTime_end() && ts[k]->getTime_end() >= current->getTime_end())
                                || (ts[k]->getTime_start() >= current->getTime_start() && ts[k]->getTime_end() <= current->getTime_end())) {
                                //Do nothing if incompatible
                        }
                        else
                            new_ts.append(ts[k]);
                    }
                    else
                        new_ts.append(ts[k]);
                }
                map.insert(keys_u[j], new_ts);
            }
        }
        poss.insert(keys_s[i], map);
    }

    return res;
}

void GeneratePage::resetPoss(int id_user, QMap<int, QList<Timeslot *> > *old) {
    /** To put back the old possibilities **/
    int i;
    QList<int> keys_s = poss.keys();

    //Put back the old
    for(i = 0; i < keys_s.length(); i++) { //For every subject
        if(old->contains(keys_s[i])) {
            QMap<int, QList<Timeslot*> > map = poss.value(keys_s[i]);

            map.insert(id_user, old->value(keys_s[i]));
            poss.insert(keys_s[i], map);
        }
    }
}

working_index *GeneratePage::findMax() {
    /** To find the place with highest score **/
    bool is_empty = true;
    int max = -1;
    QList<int> s_maxs;
    QList<int> u_maxs;

    QList<int> s_keys = poss.keys();
    int i, j;
    for(i = 0; i < s_keys.length(); i++) {
        QList<int> u_keys = poss.value(s_keys[i]).keys();

        for(j = 0; j < u_keys.length(); j++) {
            int score = Utilities::listMax(m_dbase, poss.value(s_keys[i]).value(u_keys[j]), m_dbase->listStudents()->value(u_keys[j]), m_date);
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
    return res;
}

bool GeneratePage::generate() {
    /** Generate the Kholloscope **/
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

    QMap<int, QList<Timeslot*> > map = poss.value(index->current_subject);
    QList<Timeslot*> loop = map.take(index->current_student); //Get possibilities and delete them from the possibilities map
    poss.insert(index->current_subject, map);

    //Sort the possibilities
    QMap<int, float> *probas = Utilities::corrected_proba(m_dbase, m_dbase->listStudents()->value(index->current_student), loop, m_date);
    Utilities::quickSort(&loop, 0, loop.length() - 1, probas);
    delete probas;

    for(int i = 0; i < loop.length(); i++) { //For every possibility
        if(loop[i]->getPupils() > 0) { //If enough space
            //Create new kholle
            Kholle *k = new Kholle();
            k->setId_students(index->current_student);
            k->setId_timeslots(loop[i]->getId());

            //Add it
            kholloscope.append(k);
            loop[i]->setPupils(loop[i]->getPupils() - 1); //Substract one person
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
    free(index);
    return false;
}

void GeneratePage::finished() {
    /** Called when the background process finishes **/
    timestamp = QString::number(QDateTime::currentMSecsSinceEpoch());

    if(log_file != NULL) {
        delete log_file;
        log_file = NULL;
    }
    m_db->transaction();
    Utilities::saveInSql(m_db, &kholloscope);

    setStatus();

    if(!m_watcher.future().result())
        displayBlocking();
    else {
        exchange(0, true, 40);
        exchange(0, false, 15);
    }
    /*m_db->rollback();
    m_db->transaction();
    saveInSql();*/

    int errors = 0, warnings = 0, collisions = 0;
    display(&errors, &warnings);
    displayCollision(&collisions);
    m_box->hide();

    if(m_watcher.future().result())
        displayConclusion(errors, warnings, collisions);
}

void GeneratePage::abort() {
    m_abort = true;
    m_watcher.waitForFinished();
}

void GeneratePage::setStatus() {
    /** Set the status of the generated kholles **/

    for(int i = 0; i < kholloscope.length(); i++) {
        kholloscope[i]->updateStatus(m_dbase->listTimeslots(), m_db);
    }
}

bool GeneratePage::exchange(int index, bool only_warnings, int score_limit) {
    /** Exchange timeslots between people in the kholloscope to improve score **/

    if(index >= kholloscope.length())
        return true;

    if(only_warnings) {
        while(index < kholloscope.length() && kholloscope[index]->status() == Kholle::OK)
            index++;
        if(index == kholloscope.length())
            return true;
    }

    Kholle* current = kholloscope[index];
    Timeslot* t_current = m_dbase->listTimeslots()->value(current->getId_timeslots());
    Student* s_current = m_dbase->listStudents()->value(current->getId_students());

    int i;
    for(i = 0; i < kholloscope.length(); i++) {
        if(i == index)
            continue;

        Kholle* k = kholloscope[i];
        Timeslot* t = m_dbase->listTimeslots()->value(k->getId_timeslots());
        Student* s = m_dbase->listStudents()->value(k->getId_students());
        if(m_downgraded.contains(s->getId()) && m_downgraded.value(s->getId()))
            continue;

        if(t_current->kholleur()->getId_subjects() == t->kholleur()->getId_subjects()) {
            if(Utilities::compatible(m_db, m_dbase, s_current->getId(), t, m_week)
                    && Utilities::compatible(m_db, m_dbase, s->getId(), t_current, m_week)) {
                QMap<int, float> *p_current = Utilities::corrected_proba(m_dbase, s_current, poss.value(t_current->kholleur()->getId_subjects()).value(s_current->getId()), m_date);
                QMap<int, float> *p = Utilities::corrected_proba(m_dbase, s, poss.value(t->kholleur()->getId_subjects()).value(s->getId()), m_date);

                int n1 = Kholle::nearestKholle(m_db, m_dbase->listTimeslots(), s_current->getId(), t, 0);
                int n2 = Kholle::nearestKholle(m_db, m_dbase->listTimeslots(), s->getId(), t_current, 0);

                if(p_current->value(t->getId()) - p_current->value(t_current->getId()) >= score_limit
                        && p->value(t_current->getId()) - p->value(t->getId()) >= -score_limit
                        && (n1 == -1 || n1 > 3)
                        && (n2 == -1 || n2 > 3)) {
                    current->setId_timeslots(t->getId());
                    current->setWeeks(n1);
                    current->setStatus(Kholle::OK);
                    k->setId_timeslots(t_current->getId());
                    k->setWeeks(n2);
                    k->setStatus(Kholle::OK);
                    m_downgraded.insert(s->getId(), true);
                    m_downgraded.insert(s_current->getId(), false);

                    QSqlQuery query(*m_db);
                    query.prepare("UPDATE tau_kholles SET id_timeslots=:id_ts WHERE id=:id");
                    query.bindValue(":id", current->getId());
                    query.bindValue(":id_ts", t->getId());
                    query.exec();

                    query.prepare("UPDATE tau_kholles SET id_timeslots=:id_ts WHERE id=:id");
                    query.bindValue(":id", k->getId());
                    query.bindValue(":id_ts", t_current->getId());
                    query.exec();

                    break;
                }
            }
        }
    }

    return exchange(index + 1, only_warnings, score_limit);
}

void GeneratePage::display(int *errors, int *warnings) {
    /** To display in the list **/
    ui->tableKhollo->clear();
    ui->tableKhollo->setRowCount(kholloscope.length());
    ui->tableKhollo->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    khollo_message  = "========== Génération du " + m_date.toString("dd/MM/yyyy") + " ==========\n";
    khollo_message += "                            Erreurs et avertissements\n";

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

            khollo_message += student->getName() + " " + student->getFirst_name() + " : Erreur\n";
            (*errors)++;
        }
        else {
            left->setIcon(QIcon(QPixmap(":/images/warning.png")));

            QTableWidgetItem *right = new QTableWidgetItem(kholleur->getName() + ", " + QString::number(weeks) + " semaines");
            ui->tableKhollo->setItem(i, 1, right);

            khollo_message += student->getName() + " " + student->getFirst_name() + " : Avertissement\n";
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
    collisions_message += "                                         Collisions\n";

    ui->tableCollision->clear();
    ui->tableCollision->setRowCount(0);
    ui->tableCollision->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QStringList days;
    days << "" << "Lundi" << "Mardi" << "Mercredi" << "Jeudi" << "Vendredi" << "Samedi" << "Dimanche";

    foreach(Kholle* ki, *m_dbase->listKholles()) {
        foreach(Kholle* kj, *m_dbase->listKholles()) {
            if(ki->getId() <= kj->getId())
                continue;
            Timeslot* tsi = ki->timeslot();
            Student* sti = ki->student();
            Timeslot* tsj = kj->timeslot();
            Student* stj = kj->student();
            if(sti->getId() == stj->getId()
                    && tsi->getDate() == tsj->getDate()) {
                int r = ui->tableCollision->rowCount();
                ui->tableCollision->setRowCount(r + 1);

                QTableWidgetItem *left = new QTableWidgetItem(sti->getName() + ", " + tsi->kholleur()->subject()->getShortName() + " / " + tsj->kholleur()->subject()->getShortName() + " :");
                ui->tableCollision->setItem(r, 0, left);

                QTableWidgetItem *right = new QTableWidgetItem(days[tsi->getDate().dayOfWeek()] + " " + tsi->getTime_start().toString() + " / " + tsj->getTime_start().toString());
                ui->tableCollision->setItem(r, 1, right);

                collisions_message += sti->getName() + " " + sti->getFirst_name() + " : " + tsi->kholleur()->subject()->getShortName() + " / " + tsj->kholleur()->subject()->getShortName() + "\n";
                (*collisions)++;
            }
        }
    }
}

void GeneratePage::displayConclusion(int errors, int warnings, int collisions) {
    QMessageBox::information(this, "Génération terminée", "La génération a abouti et a créé un kholloscope avec"
                                                          "<ul><li>" + QString::number(errors) + ((errors <= 1) ? " erreur" : " erreurs") +
                                                          "<li>" + QString::number(warnings) + ((warnings <= 1) ? " avertissement" : " avertissements") +
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

void GeneratePage::displayBlocking() {
    if(last_index.current_student == -1)
        return;

    QString message = "La génération a été interrompue ou n’a pas pu être finie. Le dernier élément sur lequel a travaillé le logiciel est le couple suivant : <br />";
    message += "Matière : <strong>" + m_dbase->listSubjects()->value(last_index.current_subject)->getName() + "</strong> <br />";
    message += "Élève : <strong>" + m_dbase->listStudents()->value(last_index.current_student)->getName() + ", "
            + m_dbase->listStudents()->value(last_index.current_student)->getFirst_name() + "</strong> <br />";
    message += "Il est donc probable qu’il y ait une incohérence au niveau de cette matière ou de cet/cette élève. "
               "Veuillez vérifier les horaires de kholles, groupes, emplois du temps correspondants.";

    QMessageBox::information(this, "La génération n'a pas abouti...", message);
}

void GeneratePage::saveKholles() {
    /** To save the kholloscope that has been generated to the database and other things after finish **/
    //Wait for finish if not finished
    m_watcher.waitForFinished();

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

