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
}

void GeneratePage::initializePage() {
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
            float p = 1; //Calculate probability here
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
        courses_query.prepare("SELECT * FROM tau_courses WHERE (" + request + ") AND time_start <= :time AND time_end > :time AND id_day=:id_day AND id_week=:id_week");
        courses_query.bindValue(":time", timeslot->getTime_start());
        courses_query.bindValue(":id_day", timeslot->getId_day());
        courses_query.bindValue(":id_week", m_week);
        courses_query.exec();

        if(courses_query.next()) {
            return false;
        }

        QSqlQuery courses_query2(*m_db);
        courses_query2.prepare("SELECT * FROM tau_courses WHERE (" + request + ") AND time_start < :time AND time_end >= :time AND id_day=:id_day AND id_week=:id_week");
        courses_query2.bindValue(":time", timeslot->getTime_end());
        courses_query2.bindValue(":id_day", timeslot->getId_day());
        courses_query2.bindValue(":id_week", m_week);
        courses_query2.exec();

        if(courses_query2.next()) {
            return false;
        }
    }

    return true;
}

void GeneratePage::quickSort(QList<Timeslot *> list, int i, int j, int id_user) {
    if(i >= j)
        return;

    int pivot_index = i;
    int k;
    for(k = i+1; k <= j; k++) {
        if((proba.value(list[k]->getId_kholleurs())).value(id_user) > (proba.value(list[pivot_index]->getId_kholleurs())).value(id_user)) {
            Timeslot* pivot = list[pivot_index];
            list[pivot_index] = list[k];
            list[k] = list[pivot_index+1];
            list[pivot_index+1] = pivot;
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
            quickSort(new_list, 0, new_list.length() - 1, users[j]->getId());
            map.insert(users[j]->getId(), new_list);
        }

        poss.insert(selected_subjects->at(i)->getId(), map);
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
    if(index->min == -1) {
        free(index);
        return true;
    }

    QList<Timeslot*> loop = poss.value(index->current_subject).value(index->current_student);
    QMap<int, QList<Timeslot*> > map = poss.value(index->current_subject);
    map.remove(index->current_student);
    poss.insert(index->current_subject, map);

    //QMessageBox::information(this, "OK", QString::number(profondeur));
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

            if(generate()) {
                free(index);
                return true;
            }

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
