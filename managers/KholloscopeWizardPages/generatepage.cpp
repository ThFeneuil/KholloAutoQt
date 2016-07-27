#include "generatepage.h"
#include "ui_generatepage.h"

GeneratePage::GeneratePage(QSqlDatabase *db, QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::GeneratePage)
{
    ui->setupUi(this);

    //DB
    m_db = db;
    profondeur = 0;
}

GeneratePage::~GeneratePage()
{
    delete ui;
    freeTimeslots();
    freeKholleurs();
    freeKholles();
}

void GeneratePage::initializePage() {
    connect(wizard()->button(QWizard::FinishButton), SIGNAL(clicked()), this, SLOT(saveKholles()));

    getTimeslots();
    loadSubjects();
    getKholleurs();

    m_week = field("current_week").toInt() + 1;
    m_date = field("monday_date").toDateTime().date();

    calculateProba();
    constructPoss();
    generate();
    display();
}

void GeneratePage::getTimeslots() {
    freeTimeslots();

    //Prepare query
    QSqlQuery query(*m_db);
    query.exec("SELECT id, time, time_start, time_end, id_kholleurs, id_day, pupils FROM tau_timeslots");

    //Treat
    while(query.next()) {
        Timeslot *ts = new Timeslot();
        ts->setId(query.value(0).toInt());
        ts->setTime(QTime::fromString(query.value(1).toString(), "h:mm:ss"));
        ts->setTime_start(QTime::fromString(query.value(2).toString(), "h:mm:ss"));
        ts->setTime_end(QTime::fromString(query.value(3).toString(), "h:mm:ss"));
        ts->setId_kholleurs(query.value(4).toInt());
        ts->setId_day(query.value(5).toInt());
        ts->setPupils(query.value(6).toInt());
        timeslots.append(ts);
    }
}

void GeneratePage::freeTimeslots() {
    while(!timeslots.isEmpty()) {
        free(timeslots.takeFirst());
    }
}

void GeneratePage::loadSubjects() {
    QList<Subject*> *list = ((KholloscopeWizard*) wizard())->get_subjects();

    int i;
    for(i = 0; i < list->length(); i++) {
        subjects.insert(list->at(i)->getId(), list->at(i));
    }
}

void GeneratePage::getKholleurs() {
    freeKholleurs();

    //Prepare query
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, id_subjects, duration, preparation, pupils FROM tau_kholleurs");

    //Treat query
    while(query.next()) {
        Kholleur *k = new Kholleur();
        k->setId(query.value(0).toInt());
        k->setName(query.value(1).toString());
        k->setId_subjects(query.value(2).toInt());
        k->setDuration(query.value(3).toInt());
        k->setPreparation(query.value(4).toInt());
        k->setPupils(query.value(5).toInt());
        kholleurs.insert(k->getId(), k);
    }
}

void GeneratePage::freeKholleurs() {
    QList<int> keys = kholleurs.keys();
    int i;
    for(i = 0; i < keys.length(); i++) {
        free(kholleurs.take(keys[i]));
    }
}

void GeneratePage::calculateProba() {
    foreach(Kholleur* k, kholleurs) {
        QMap<int, float> map;

        QMap<int, QList<Student*> > *input = ((KholloscopeWizard*) wizard())->get_input();
        QList<Student*> users = input->value(k->getId_subjects());

        int i;
        for(i = 0; i < users.length(); i++) {
            float p = 100; //Calculate probability here

            QSqlQuery kholle_query(*m_db);
            kholle_query.prepare("SELECT id, time_start, time, time_end, id_subjects, id_users, id_teachers FROM tau_kholles WHERE id_users = :id_student AND id_teachers = :id_teacher ORDER BY time DESC");
            kholle_query.bindValue(":id_student", users[i]->getId());
            kholle_query.bindValue(":id_teacher", k->getId());
            kholle_query.exec();

            if(kholle_query.size() != -1) {
                p -= kholle_query.size() * 10;
            }
            else {
                QSqlQuery count_query(*m_db);
                count_query.prepare("SELECT COUNT(id) FROM tau_kholles WHERE id_users = :id_student AND id_teachers = :id_teacher");
                count_query.bindValue(":id_student", users[i]->getId());
                count_query.bindValue(":id_teacher", k->getId());
                count_query.exec();

                if(count_query.next()) {
                    p -= count_query.value(0).toInt() * 10;
                }
                else {
                    QMessageBox::critical(this, "Erreur", "Erreur avec la base de donnée");
                }
            }

            if(kholle_query.next()) {
                QDateTime last_time = QDateTime::fromString(kholle_query.value(2).toString(), "yyyy-MM-dd HH:mm:ss");
                QDateTime monday_date = QDateTime(m_date);

                if(last_time >= monday_date.addDays(-21))
                    p -= 30;

                if(last_time >= monday_date.addDays(-14))
                    p -= 10;

                if(last_time >= monday_date.addDays(-7))
                    p -= 10;
            }


            map.insert(users[i]->getId(), p);
        }

        proba.insert(k->getId(), map);
    }
}

bool GeneratePage::compatible(int id_user, Timeslot *timeslot) {
    //Get all groups of user (the ids)
    QSqlQuery query(*m_db);
    query.prepare("SELECT id_groups FROM tau_groups_users WHERE id_users=:id_users");
    query.bindValue(":id_users", id_user);
    query.exec();

    //Get all courses that can interfere with this timeslot
    if(query.next()) {
        QString request = "id_groups=" + QString::number(query.value(0).toInt());
        while(query.next()) {
            request = request + " OR id_groups=" + QString::number(query.value(0).toInt());
        }

        QSqlQuery courses_query(*m_db);
        courses_query.prepare("SELECT * FROM tau_courses WHERE (" + request + ") AND id_day=:id_day AND id_week=:id_week AND ("
                                                                              "(time_start <= :time_start AND time_end > :time_start) OR"
                                                                              "(time_start < :time_end AND time_end >= :time_end) OR"
                                                                              "(time_start >= :time_start AND time_end <= :time_end) )");
        courses_query.bindValue(":time_start", timeslot->getTime_start());
        courses_query.bindValue(":time_end", timeslot->getTime_end());
        courses_query.bindValue(":id_day", timeslot->getId_day());
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
        QDate new_date = m_date.addDays(timeslot->getId_day() - 1);

        event_query.bindValue(":start_time", QDateTime(new_date, timeslot->getTime_start()).toString("yyyy-MM-dd HH:mm:ss"));
        event_query.bindValue(":end_time", QDateTime(new_date, timeslot->getTime_end()).toString("yyyy-MM-dd HH:mm:ss"));
        event_query.exec();

        if(event_query.next()) {
            return false;
        }
    }

    return true;
}

void GeneratePage::quickSort(QList<Timeslot *> *list, int i, int j, int id_user) {
    if(i >= j)
        return;

    int pivot_index = i;
    int k;
    for(k = i+1; k <= j; k++) {
        if((proba.value(list->at(k)->getId_kholleurs())).value(id_user) > (proba.value(list->at(pivot_index)->getId_kholleurs())).value(id_user)) {
            Timeslot* pivot = list->at(pivot_index);
            list->replace(pivot_index, list->at(k));
            list->replace(k, list->at(pivot_index+1));
            list->replace(pivot_index+1, pivot);
            pivot_index++;
        }
    }

    quickSort(list, i, pivot_index-1, id_user);
    quickSort(list, pivot_index+1, j, id_user);
}

void GeneratePage::constructPoss() {
    //Get selected subjects
    QList<Subject*> *selected_subjects = ((KholloscopeWizard*) wizard())->get_assoc_subjects();

    //Get input
    QMap<int, QList<Student*> > *input = ((KholloscopeWizard*) wizard())->get_input();

    int i, j, k;
    for(i = 0; i < selected_subjects->length(); i++) {
        QMap<int, QList<Timeslot*> > map;
        QList<Student*> users = input->value(selected_subjects->at(i)->getId());

        for(j = 0; j < users.length(); j++) {
            QList<Timeslot*> new_list;

            for(k = 0; k < timeslots.length(); k++) {
                Subject* sub = subjects.value(kholleurs.value(timeslots[k]->getId_kholleurs())->getId_subjects());

                if(sub == selected_subjects->at(i) && compatible(users[j]->getId(), timeslots[k])) {
                    new_list.append(timeslots[k]);
                }
            }
            quickSort(&new_list, 0, new_list.length() - 1, users[j]->getId());

            /*QString string;
            for(k = 0; k < new_list.length(); k++) {
                string += QString::number(new_list[k]->getId_kholleurs()) + "\n";
            }
            QMessageBox::information(this, QString::number(users[j]->getId()), string);*/
            map.insert(users[j]->getId(), new_list);
        }

        poss.insert(selected_subjects->at(i)->getId(), map);
    }
}

QMap<int, QList<Timeslot *> > *GeneratePage::updatePoss(int id_user, Timeslot* current) {
    QMap<int, QList<Timeslot*> > *res = new QMap<int, QList<Timeslot*> >;

    int i, j, k;
    QList<int> keys_s = poss.keys();
    for(i = 0; i < keys_s.length(); i++) {
        QMap<int, QList<Timeslot*> > map = poss.value(keys_s[i]);
        QList<int> keys_u = map.keys();
        for(j = 0; j < keys_u.length(); j++) {
            if(keys_u[j] == id_user) {
                QList<Timeslot*> ts = map.value(keys_u[j]);

                //Save the old possibilities
                res->insert(keys_s[i], ts);

                //Change the possibilities
                for(k = 0; k < ts.length(); k++) {
                    if(ts[i]->getId_day() == current->getId_day()) {
                        if((ts[i]->getTime_start() <= current->getTime_start() && ts[i]->getTime_end() > current->getTime_start())
                                || (ts[i]->getTime_start() < current->getTime_end() && ts[i]->getTime_end() >= current->getTime_end())
                                || (ts[i]->getTime_start() >= current->getTime_start() && ts[i]->getTime_end() <= current->getTime_end())) {
                                ts.removeAt(k);
                        }
                    }
                }
                map.insert(keys_u[j], ts);
            }
        }
        poss.insert(keys_s[i], map);
    }

    return res;
}

void GeneratePage::resetPoss(int id_user, QMap<int, QList<Timeslot *> > *old) {
    int i;
    QList<int> keys_s = poss.keys();

    //Put back the old
    for(i = 0; i < keys_s.length(); i++) {
        if(old->contains(keys_s[i])) {
            QMap<int, QList<Timeslot*> > map = poss.value(keys_s[i]);

            map.insert(id_user, old->value(keys_s[i]));
            poss.insert(keys_s[i], map);
        }
    }
}

int GeneratePage::my_count(QList<Timeslot *> list) {
    int i;
    int counter = 0;

    for(i = 0; i < list.length(); i++) {
        if(list[i]->getPupils() > 0) {
            counter++;
        }
    }
    return counter;
}

working_index *GeneratePage::findMin() {
    int min = -1;
    int s_min = 0, u_min = 0;

    QList<int> s_keys = poss.keys();
    int i, j;
    for(i = 0; i < s_keys.length(); i++) {
        QList<int> u_keys = poss.value(s_keys[i]).keys();

        for(j = 0; j < u_keys.length(); j++) {
            int count = my_count(poss.value(s_keys[i]).value(u_keys[j]));
            if(min == -1 || count < min) {
                min = count;
                s_min = s_keys[i];
                u_min = u_keys[j];
            }
        }
    }

    working_index *res = (working_index*) malloc(sizeof(working_index));
    res->min = min;
    res->current_student = u_min;
    res->current_subject = s_min;
    return res;
}

bool GeneratePage::generate() {
    profondeur++;
    working_index* index = findMin();
    //QMessageBox::information(this, "OK", QString::number(profondeur));
    if(index->min == -1) {
        free(index);
        return true;
    }

    QList<Timeslot*> loop = poss.value(index->current_subject).value(index->current_student);
    QMap<int, QList<Timeslot*> > map = poss.value(index->current_subject);
    map.remove(index->current_student);
    poss.insert(index->current_subject, map);

    //QMessageBox::information(this, "OK", QString::number(index->current_student));
    //QMessageBox::information(this, "OK", QString::number(loop.length()));
    //msg_display();

    int i;
    for(i = 0; i < loop.length(); i++) {
        if(loop[i]->getPupils() > 0) {
            Kholle *k = new Kholle();
            k->setId_students(index->current_student);
            k->setId_subjects(index->current_subject);
            k->setId_teachers(loop[i]->getId_kholleurs());

            QDate new_date = m_date.addDays(loop[i]->getId_day() - 1);

            k->setTime(QDateTime(new_date, loop[i]->getTime()));
            k->setTime_start(QDateTime(new_date, loop[i]->getTime_start()));
            k->setTime_end(QDateTime(new_date, loop[i]->getTime_end()));

            kholloscope.append(k);
            loop[i]->setPupils(loop[i]->getPupils() - 1);
            QMap<int, QList<Timeslot*> > *old = updatePoss(index->current_student, loop[i]);

            if(generate()) {
                free(index);
                return true;
            }

            resetPoss(index->current_student, old);
            delete old;
            delete kholloscope.takeLast();
            loop[i]->setPupils(loop[i]->getPupils() + 1);
        }
    }
    map = poss.value(index->current_subject);
    map.insert(index->current_student, loop);
    poss.insert(index->current_subject, map);
    profondeur--;
    free(index);
    return false;
}

void GeneratePage::display() {
    int i;
    for(i = 0; i < kholloscope.length(); i++) {
        ui->listWidget->addItem(QString::number(kholloscope[i]->getId_students()) + ", " + QString::number(kholloscope[i]->getId_teachers()) + ", " + kholloscope[i]->getTime_start().toString("dd/MM/yyyy hh:mm:ss"));
    }
}

void GeneratePage::msg_display() {
    int i;
    QString msg;
    for(i = 0; i < kholloscope.length(); i++) {
        msg = msg + QString::number(kholloscope[i]->getId_students()) + ", " + QString::number(kholloscope[i]->getId_teachers()) + ", " + kholloscope[i]->getTime_start().toString("dd/MM/yyyy hh:mm:ss") + "\n";
    }
    QMessageBox::information(this, "OK", msg);
}

void GeneratePage::saveKholles() {
    /** To save the kholloscope that has been generated to the database **/
    int i;

    //Only if checkbox selected...
    if(ui->checkBox->isChecked()) {
        for(i = 0; i < kholloscope.length(); i++) {
            //Get kholle
            Kholle* k = kholloscope[i];

            //Prepare query for insertion
            QSqlQuery query(*m_db);
            query.prepare("INSERT INTO tau_kholles(time_start, time, time_end, id_subjects, id_users, id_teachers) VALUES(:time_start, :time, :time_end, :id_subjects, :id_students, :id_teachers)");
            query.bindValue(":time_start", k->getTime_start().toString("yyyy-MM-dd HH:mm:ss"));
            query.bindValue(":time", k->getTime().toString("yyyy-MM-dd HH:mm:ss"));
            query.bindValue(":time_end", k->getTime_end().toString("yyyy-MM-dd HH:mm:ss"));
            query.bindValue(":id_subjects", k->getId_subjects());
            query.bindValue(":id_students", k->getId_students());
            query.bindValue(":id_teachers", k->getId_teachers());
            query.exec();
        }
    }
}

void GeneratePage::freeKholles() {
    while(!kholloscope.isEmpty()) {
        delete kholloscope.takeFirst();
    }
}
