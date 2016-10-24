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
}

GeneratePage::~GeneratePage() {
    m_abort = true;
    m_watcher.waitForFinished();

    delete ui;

    if(m_dbase != NULL)
        delete m_dbase;

    delete m_box;
    freeKholles();

    /*if(last_index != NULL)
        free(last_index);*/

    //QMessageBox::information(this, "OK", "destructor called...");
}

void GeneratePage::initializePage() {
    //Connect the finish button to save on finish
    connect(wizard()->button(QWizard::FinishButton), SIGNAL(clicked()), this, SLOT(saveKholles()));

    //Initialise random number generator
    qsrand((uint)QTime::currentTime().msec());

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
    //m_dbase->setConditionTimeslots("date>=\"" + m_date.toString("yyyy-MM-dd") + "\" AND date<=\"" + m_date.addDays(6).toString("yyyy-MM-dd") + "\"");
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

    m_box->hide();
    disconnect(m_box, SIGNAL(finished(int)), this, SLOT(abort()));
    disconnect(&m_watcher, SIGNAL(finished()), this, SLOT(finished()));

    //Clear list
    ui->tableWidget->clear();

    //delete m_dbase;
    //QMessageBox::information(this, "OK", "cleanupPage() called...");
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

float GeneratePage::proba(Student *user, Timeslot *timeslot) {
    /** Calculates a score for this (user, timeslot) couple
        Attention ! The Student and Timeslot need to have their "kholleur", "kholles", etc. properties set (in DataBase) **/
    int kholleur_total = 0;
    int kholles_total = 0;
    int this_kholleur = 0;

    float p = 0; //Insert formula here

    int i;
    foreach(Kholleur* k, *m_dbase->listKholleurs()) {
        if(k->getId_subjects() == timeslot->kholleur()->getId_subjects()) //Total number of kholleurs in this subject
            kholleur_total++;
    }

    for (i = 0; i < user->kholles()->length(); i++) {
        Timeslot* ts = user->kholles()->at(i)->timeslot();

        if(ts->kholleur()->getId_subjects() == timeslot->kholleur()->getId_subjects()) //Total number of kholles in this subject for this person
            kholles_total++;
        if(ts->getId_kholleurs() == timeslot->getId_kholleurs()) { //If it was the same kholleur
            this_kholleur++;

            if(ts->getDate() >= m_date.addDays(-21) && ts->getDate() <= m_date.addDays(28)) { //If it was within certain time limits
                p -= 40*pow(2, 28-abs(ts->getDate().daysTo(timeslot->getDate())));
            }
        }
    }

    p -= 20 * this_kholleur * kholleur_total;
    p += 20 * kholles_total;
    p += 1000;

    return p;
}

bool GeneratePage::compatible(int id_user, Timeslot *timeslot) {
    //Get all groups of user (the ids)
    QList<Group*> *groups = m_dbase->listStudents()->value(id_user)->groups();

    int i;
    if(groups->length() > 0) {
        QString request = "id_groups=" + QString::number(groups->at(0)->getId());
        for(i = 1; i < groups->length(); i++) {
            request = request + " OR id_groups=" + QString::number(groups->at(i)->getId());
        }

        //Get all courses that can interfere with this timeslot
        QSqlQuery courses_query(*m_db);
        courses_query.prepare("SELECT * FROM tau_courses WHERE (" + request + ") AND id_day=:id_day AND id_week=:id_week AND ("
                                                                              "(time_start <= :time_start AND time_end > :time_start) OR"
                                                                              "(time_start < :time_end AND time_end >= :time_end) OR"
                                                                              "(time_start >= :time_start AND time_end <= :time_end) )");
        courses_query.bindValue(":time_start", timeslot->getTime_start().toString("HH:mm:ss"));
        courses_query.bindValue(":time_end", timeslot->getTime_end().toString("HH:mm:ss"));
        courses_query.bindValue(":id_day", timeslot->getDate().dayOfWeek());
        courses_query.bindValue(":id_week", m_week);
        courses_query.exec();

        if(courses_query.next()) {
            return false;
        }

        //Get all events that can interfere with this timeslot
        QSqlQuery event_query(*m_db);
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

    //Get all kholles that can interfere with this timeslot
    QSqlQuery kholle_query(*m_db);
    kholle_query.prepare("SELECT * FROM tau_kholles AS K JOIN tau_timeslots AS T ON K.`id_timeslots` = T.`id` WHERE K.`id_users`=:id_users AND T.`date`=:date AND ("
                         "(T.`time_start` <= :time_start AND T.`time_end` > :time_start) OR "
                         "(T.`time_start` < :time_end AND T.`time_end` >= :time_end) OR "
                         "(T.`time_start` >= :time_start AND T.`time_end` <= :time_end))");
    kholle_query.bindValue(":id_users", id_user);
    kholle_query.bindValue(":date", timeslot->getDate().toString("yyyy-MM-dd"));
    kholle_query.bindValue(":time_start", timeslot->getTime_start().toString("HH:mm:ss"));
    kholle_query.bindValue(":time_end", timeslot->getTime_end().toString("HH:mm:ss"));
    kholle_query.exec();

    if(kholle_query.next()) {
        return false;
    }

    return true;
}

void GeneratePage::quickSort(QList<Timeslot *> *list, int i, int j, Student* user) {
    /** QuickSort of a QList<Timeslot*> based on the value of the probabilities
        Attention ! Student must have "kholles" and other properties set (from DataBase) **/
    if(i >= j)
        return;

    int pivot_index = i;
    int k;
    for(k = i+1; k <= j; k++) {
        if(proba(user, list->at(k)) > proba(user, list->at(pivot_index))) {
            Timeslot* pivot = list->at(pivot_index);
            list->replace(pivot_index, list->at(k));
            list->replace(k, list->at(pivot_index+1));
            list->replace(pivot_index+1, pivot);
            pivot_index++;
        }
    }

    quickSort(list, i, pivot_index-1, user);
    quickSort(list, pivot_index+1, j, user);
}

void GeneratePage::constructPoss() {
    //Create stream around log_file
    QTextStream out(log_file);

    //Get selected subjects
    QList<Subject*> *selected_subjects = ((KholloscopeWizard*) wizard())->get_assoc_subjects();

    //Get input
    QMap<int, QList<Student*> > *input = ((KholloscopeWizard*) wizard())->get_input();

    int i, j;
    for(i = 0; i < selected_subjects->length(); i++) { //For every selected subject
        //Create a new map
        QMap<int, QList<Timeslot*> > map;

        //The selected users for this subject
        QList<Student*> users = input->value(selected_subjects->at(i)->getId());

        for(j = 0; j < users.length(); j++) { //For every selected user
            //Create new list
            QList<Timeslot*> new_list;

            foreach(Timeslot* ts, *m_dbase->listTimeslots()) { //For every timeslot
                Subject* sub = ts->kholleur()->subject();

                if(sub->getId() == selected_subjects->at(i)->getId()
                        && ts->getDate() >= m_date
                        && ts->getDate() <= m_date.addDays(6)
                        && compatible(users[j]->getId(), ts)) { //Add the compatible timeslots
                    new_list.append(ts);

                    //Log the probability for this timeslot
                    if(log_file != NULL) {
                        out << users[j]->getName() << ", " << selected_subjects->at(i)->getShortName() << ", " << ts->kholleur()->getName() << ", " << ts->getId() << " : ";
                        out << proba(m_dbase->listStudents()->value(users[j]->getId()), ts) << "\n";
                    }
                }
            }

            // No sorting here !
            //quickSort(&new_list, 0, new_list.length() - 1, users[j]->getId()); //Sort the list based on probabilities

            /*QString string;
            int k;
            for(k = 0; k < new_list.length(); k++) {
                string += QString::number(new_list[k]->getId_kholleurs()) + "\n";
            }
            QMessageBox::information(this, QString::number(users[j]->getId()), string);*/

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

int GeneratePage::listMax(QList<Timeslot *> list, Student* user) {
    /** Returns the highest probability from the list or a very high score if the list is empty
        Attention ! Student must have "kholles" and other properties set (from DataBase) **/
    int i;
    bool is_empty = true;
    int max = 0;

    for(i = 0; i < list.length(); i++) {
        if(list[i]->getPupils() > 0) {
            float p = proba(user, list[i]);
            if(is_empty || p > max)
                max = p;
            is_empty = false;
        }
    }

    if(is_empty)
        return 1000000;
    else
        return max;
}

working_index *GeneratePage::findMax() {
    /** To find the place with least possibilities **/
    bool is_empty = true;
    int max = -1;
    QList<int> s_maxs;
    QList<int> u_maxs;

    QList<int> s_keys = poss.keys();
    int i, j;
    for(i = 0; i < s_keys.length(); i++) {
        QList<int> u_keys = poss.value(s_keys[i]).keys();

        for(j = 0; j < u_keys.length(); j++) {
            int score = listMax(poss.value(s_keys[i]).value(u_keys[j]), m_dbase->listStudents()->value(u_keys[j]));
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

    //QMessageBox::information(this, "OK", QString::number(profondeur));
    //qDebug() << QString::number(profondeur);

    //It is finished
    if(index == NULL) {
        return true;
    }

    //Copy current index to the global variable
    /*if(last_index != NULL)
        free(last_index);*/
    //last_index = (working_index*) malloc(sizeof(working_index));
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
    quickSort(&loop, 0, loop.length() - 1, m_dbase->listStudents()->value(index->current_student));

    //QMessageBox::information(this, "OK", QString::number(index->current_student));
    //QMessageBox::information(this, "OK", QString::number(loop.length()));
    //msg_display();

    int i;
    for(i = 0; i < loop.length(); i++) { //For every possibility
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
                delete old;
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
    display();
    m_box->hide();

    if(!m_watcher.future().result())
        displayBlocking();

    if(log_file != NULL) {
        delete log_file;
        log_file = NULL;
    }
}

void GeneratePage::abort() {
    m_abort = true;
    m_watcher.waitForFinished();
    //display();
    //m_box->hide();
}

int GeneratePage::nearestKholle(Student *user, Timeslot *timeslot) {
    /** Number of weeks to nearest Kholle with the same Kholleur. (-1) if no such Kholle
     *  Attention ! Student and Timeslot need to have all properties set (from DataBase) **/

    int id_kholleur = timeslot->getId_kholleurs();
    int min = -1;
    Timeslot* m = NULL;

    foreach(Kholle* k, *user->kholles()) { //Find the nearest one
        if(k->timeslot()->getId_kholleurs() == id_kholleur) {
            int daysTo = abs(k->timeslot()->getDate().daysTo(timeslot->getDate()));
            if(min == -1 || daysTo < min) {
                min = daysTo;
                m = k->timeslot();
            }
        }
    }

    if(min == -1)
        return (-1);
    else {
        //Get the corresponding mondays to have number of weeks between the two dates
        QDate d1 = m->getDate();
        while(d1.dayOfWeek() != 1)
            d1 = d1.addDays(-1);

        QDate d2 = timeslot->getDate();
        while(d2.dayOfWeek() != 1)
            d2 = d2.addDays(-1);

        return int(abs(d1.daysTo(d2)) / 7);
    }
}

void GeneratePage::display() {
    /** To display in the list **/
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(kholloscope.length());
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    int i;
    for(i = 0; i < kholloscope.length(); i++) {
        Student* student = m_dbase->listStudents()->value(kholloscope[i]->getId_students());
        Timeslot* timeslot = m_dbase->listTimeslots()->value(kholloscope[i]->getId_timeslots());
        Subject* subject = timeslot->kholleur()->subject();
        Kholleur* kholleur = timeslot->kholleur();

        int weeks = nearestKholle(student, timeslot);
        QTableWidgetItem *left = new QTableWidgetItem(student->getName() + ", " + subject->getShortName() + " :");

        if(weeks == -1) {
            QTableWidgetItem *right = new QTableWidgetItem(kholleur->getName() + ", jamais");
            ui->tableWidget->setItem(i, 1, right);
            left->setIcon(QIcon(QPixmap(":/images/ok.png")));
        }
        else if(weeks <= 1) {
            QTableWidgetItem *right = new QTableWidgetItem(kholleur->getName() + ", " + QString::number(weeks) + " semaine");
            ui->tableWidget->setItem(i, 1, right);
            left->setIcon(QIcon(QPixmap(":/images/error.png")));
        }
        else {
            QTableWidgetItem *right = new QTableWidgetItem(kholleur->getName() + ", " + QString::number(weeks) + " semaines");
            ui->tableWidget->setItem(i, 1, right);
            if(weeks == 2)
                left->setIcon(QIcon(QPixmap(":/images/warning.png")));
            else
                left->setIcon(QIcon(QPixmap(":/images/ok.png")));
        }
        ui->tableWidget->setItem(i, 0, left);
    }
}

void GeneratePage::msg_display() {
    /** For debugging purposes **/
    int i;
    QString msg;
    for(i = 0; i < kholloscope.length(); i++) {
        msg = msg + QString::number(kholloscope[i]->getId_students()) + ", " + QString::number(kholloscope[i]->getId_timeslots()) + "\n";
    }
    QMessageBox::information(this, "OK", msg);
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

    int i;

    //Only if checkbox selected...
    if(ui->checkBox->isChecked()) {
        QMessageBox box(QMessageBox::Information, "Sauvegarde en cours", "Veuillez patienter...");
        box.show();

        for(i = 0; i < kholloscope.length(); i++) {
            //Get kholle
            Kholle* k = kholloscope[i];

            //Prepare query for insertion
            QSqlQuery query(*m_db);
            query.prepare("INSERT INTO tau_kholles(id_users, id_timeslots) VALUES(:id_students, :id_timeslots)");
            query.bindValue(":id_students", k->getId_students());
            query.bindValue(":id_timeslots", k->getId_timeslots());
            query.exec();
        }
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

        //Add the generated kholles
        int i;
        for(i = 0; i < kholloscope.length(); i++) {
            khollo->insertMulti(0, kholloscope[i]);
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

