#include "lastchanges.h"
#include "ui_lastchanges.h"

LastChanges::LastChanges(QSqlDatabase *db, int id_week, QDate *monday, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LastChanges)
{
    ui->setupUi(this);

    //QMessageBox::information(NULL, "INFO", monday->toString("yyyy-MM-dd"));

    // Initialisation
    m_db = db;
    m_monday = monday;
    m_id_week = id_week;
    days << "" << "Lundi" << "Mardi" << "Mercredi" << "Jeudi" << "Vendredi" << "Samedi";

    QSqlQuery query(*m_db);
    query.exec("SELECT K.id, K.name, K.id_subjects, K.duration, K.preparation, K.pupils, S.shortName "
               "FROM tau_kholleurs AS K "
               "LEFT OUTER JOIN tau_subjects AS S "
                  "ON K.id_subjects = S.id "
               "ORDER BY K.name");

    // Treat the request & Display the kholleurs
    while (query.next()) {
        // Get the kholleur
        Kholleur* khll = new Kholleur();
        khll->setId(query.value(0).toInt());
        khll->setName(query.value(1).toString());
        khll->setId_subjects(query.value(2).toInt());
        khll->setDuration(query.value(3).toInt());
        khll->setPreparation(query.value(4).toInt());
        khll->setPupils(query.value(5).toInt());
        // Display the kholleur
        QString subject = query.value(6).toString();
        if(subject != "")
            subject = " (" + subject + ") ";
        ui->comboBox_kholleurs->addItem(khll->getName() + subject, (qulonglong) khll);
        queue_displayedKholleurs.enqueue(khll);
    }

    connect(ui->comboBox_kholleurs, SIGNAL(currentIndexChanged(int)), this, SLOT(change_timeslotsList()));
    connect(ui->list_timeslots, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(change_timeslots(QListWidgetItem*)));

    connect(ui->pushButton_reset, SIGNAL(clicked(bool)), this, SLOT(change_timeslotsList()));
    change_timeslotsList();

    connect(ui->radioButton_subject, SIGNAL(clicked(bool)), this, SLOT(update_khollesManager()));
    connect(ui->radioButton_all, SIGNAL(clicked(bool)), this, SLOT(update_khollesManager()));
    connect(ui->pushButton_delete, SIGNAL(clicked(bool)), this, SLOT(delete_kholles()));
    update_khollesManager();

    QSqlQuery query_subj(*m_db);
    query_subj.exec("SELECT `id`, `name` FROM `tau_subjects` ORDER BY `name`");

    while (query_subj.next())
        ui->comboBox_subjects->addItem(query_subj.value(1).toString(), query_subj.value(0).toInt());
}

LastChanges::~LastChanges()
{
    delete ui;
    free_kholleurs();
}

bool LastChanges::free_kholleurs() {
    /** To free memories with kholleurs **/
    while (!queue_displayedKholleurs.isEmpty())
        delete queue_displayedKholleurs.dequeue();
    return true;
}

bool LastChanges::free_timeslots() {
    /** To free memories with kholleurs **/
    while (!queue_displayedTimeslots.isEmpty()) {
        TimeslotChg* tsChg= queue_displayedTimeslots.dequeue();
        delete tsChg->start;
        delete tsChg->end;
        delete tsChg;
    }
    return true;
}

bool LastChanges::free_students() {
    /** To free memories with kholleurs **/
    while (!queue_displayedStudents.isEmpty())
        delete queue_displayedStudents.dequeue();
    return true;
}

bool LastChanges::change_timeslots(QListWidgetItem *item) {
    TimeslotChg* tsChg = (TimeslotChg*) item->data(Qt::UserRole).toLongLong();
    UpdateTimeslotDialog dialog(tsChg->end);
    dialog.exec();

    update_timeslotsList();
    return true;
}

bool LastChanges::change_timeslotsList() {
    free_timeslots();
    ui->list_timeslots->clear();
    Kholleur* khll = (Kholleur*) ui->comboBox_kholleurs->currentData().toLongLong();

    //QMessageBox::information(NULL, "INFO", m_monday->toString("yyyy-MM-dd"));

    // Get the list of timeslots
    QSqlQuery query(*m_db);
    query.prepare("SELECT id, time_start, time, time_end, id_kholleurs, date, pupils FROM tau_timeslots "
                  "WHERE id_kholleurs=:id_kholleurs AND date>=:monday_date AND date<=:sunday_date "
                  "ORDER BY date, time");
    query.bindValue(":id_kholleurs", khll->getId());
    query.bindValue(":monday_date", m_monday->toString("yyyy-MM-dd"));
    query.bindValue(":sunday_date", m_monday->addDays(6).toString("yyyy-MM-dd"));
    query.exec();

    //Treat
    int num = 0;
    while(query.next()) {
        num++;
        TimeslotChg* tsChg = new TimeslotChg();
        tsChg->start = new Timeslot();
        tsChg->start->setId(query.value(0).toInt());
        tsChg->start->setTime_start(query.value(1).toTime());
        tsChg->start->setTime(query.value(2).toTime());
        tsChg->start->setTime_end(query.value(3).toTime());
        tsChg->start->setId_kholleurs(query.value(4).toInt());
        tsChg->start->setDate(query.value(5).toDate());
        tsChg->start->setPupils(query.value(6).toInt());
        tsChg->start->setIsDeleted(false);
        tsChg->end = new Timeslot();
        tsChg->end->setId(query.value(0).toInt());
        tsChg->end->setTime_start(query.value(1).toTime());
        tsChg->end->setTime(query.value(2).toTime());
        tsChg->end->setTime_end(query.value(3).toTime());
        tsChg->end->setId_kholleurs(query.value(4).toInt());
        tsChg->end->setDate(query.value(5).toDate());
        tsChg->end->setPupils(query.value(6).toInt());
        tsChg->end->setIsDeleted(false);
        queue_displayedTimeslots.enqueue(tsChg);

        QListWidgetItem *item = new QListWidgetItem(QString::number(num) + "# " + days[tsChg->start->getDate().dayOfWeek()] + " : "
                + tsChg->start->getTime_start().toString("HH:mm") + " >> " + tsChg->start->getTime().toString("HH:mm") + " >> " + tsChg->start->getTime_end().toString("HH:mm")
                + ", " + QString::number(tsChg->start->getPupils()) + (tsChg->start->getPupils() <= 1 ? " élève" : " élèves"), ui->list_timeslots);
        item->setData(Qt::UserRole, (qulonglong) tsChg);
    }

    update_students();

    return true;
}

bool LastChanges::update_timeslotsList() {
    for(int i=0; i<ui->list_timeslots->count(); i++) {
        QListWidgetItem* item = ui->list_timeslots->item(i);
        TimeslotChg* tsChg = (TimeslotChg*) item->data(Qt::UserRole).toLongLong();
        if(! tsChg->end->isDeleted())
            item->setText(QString::number(i+1) + "# " + days[tsChg->end->getDate().dayOfWeek()] + " : "
                    + tsChg->end->getTime_start().toString("HH:mm") + " >> " + tsChg->end->getTime().toString("HH:mm") + " >> " + tsChg->end->getTime_end().toString("HH:mm")
                    + ", " + QString::number(tsChg->end->getPupils()) + (tsChg->end->getPupils() <= 1 ? " élève" : " élèves"));
        else
            item->setText(QString::number(i+1) + "# " + "Horaire de kholle supprimé...");
        if(! tsChg->end->isDeleted() && tsChg->start->getDate() == tsChg->end->getDate() && tsChg->start->getTime_start() == tsChg->end->getTime_start())
            item->setForeground(QBrush(QColor(0,0,0)));
        else {
            item->setForeground(QBrush(QColor(255,0,0)));
            item->setText(item->text() + " ("+days[tsChg->start->getDate().dayOfWeek()]+" "+tsChg->start->getTime_start().toString("HH:mm")+")");
        }
    }

    update_students();
    return true;
}

bool LastChanges::update_students() {
    free_students();
    ui->tableWidget_student->clear();
    int nbRows = 0;
    // Je sais c'est horrible...
    for(int i=0; i<ui->list_timeslots->count(); i++) {
        QListWidgetItem* item = ui->list_timeslots->item(i);
        TimeslotChg* tsChg = (TimeslotChg*) item->data(Qt::UserRole).toLongLong();
        QSqlQuery query(*m_db);
        query.prepare("SELECT COUNT(*) "
                      "FROM `tau_kholles` AS K "
                      "JOIN `tau_users` AS S "
                        "ON K.`id_users` = S.`id` "
                      "WHERE K.`id_timeslots` = :id_timeslots "
                      "ORDER BY S.`name`, S.`first_name`");
        query.bindValue(":id_timeslots", tsChg->start->getId());
        query.exec();
        if(query.next())
            nbRows += query.value(0).toInt();
    }
    ui->tableWidget_student->setRowCount(nbRows);
    ui->tableWidget_student->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    int numStudent = 0;
    for(int i=0; i<ui->list_timeslots->count(); i++) {
        QListWidgetItem* item = ui->list_timeslots->item(i);
        TimeslotChg* tsChg = (TimeslotChg*) item->data(Qt::UserRole).toLongLong();
        Timeslot* initial = tsChg->start;
        Timeslot* final = tsChg->end;
        QSqlQuery query(*m_db);
        query.prepare("SELECT S.`id`, S.`name`, S.`first_name`, S.`email` "
                      "FROM `tau_kholles` AS K "
                      "JOIN `tau_users` AS S "
                        "ON K.`id_users` = S.`id` "
                      "WHERE K.`id_timeslots` = :id_timeslots "
                      "ORDER BY S.`name`, S.`first_name`");
        query.bindValue(":id_timeslots", initial->getId());
        query.exec();
        //QMessageBox::information(NULL, QString::number(tsChg->start->getId()), query.lastError().text());

        while(query.next()) {
            //QMessageBox::information(NULL, QString::number(tsChg->start->getId()), "OK");
            Student* stdnt = new Student();
            stdnt->setId(query.value(0).toInt());
            stdnt->setName(query.value(1).toString());
            stdnt->setFirst_name(query.value(2).toString());
            stdnt->setEmail(query.value(3).toString());

            QTableWidgetItem* left = new QTableWidgetItem(stdnt->getName() + " " + stdnt->getFirst_name() + " #"+QString::number(i));
            QTableWidgetItem* right = new QTableWidgetItem();
            if(final->isDeleted())
                left->setIcon(QIcon(QPixmap(":/images/warning.png")));
            else{
                QString res = compatible(stdnt, final);
                if(res == "")
                    left->setIcon(QIcon(QPixmap(":/images/ok.png")));
                else {
                    left->setIcon(QIcon(QPixmap(":/images/error.png")));
                    right->setText(res);
                }
            }
            delete stdnt;
            ui->tableWidget_student->setItem(numStudent, 0, left);
            ui->tableWidget_student->setItem(numStudent, 1, right);
            numStudent++;
        }
    }

    return true;
}

QString LastChanges::compatible(Student* stdnt, Timeslot *timeslot) {
    /** METHOD TO DETERMINE IF A STUDENT AND A TIMESLOT IS COMPATIBLE **/
    //Get all groups of student (the ids)
    QSqlQuery groups_query(*m_db);
    groups_query.prepare("SELECT G.`id` "
                         "FROM `tau_groups_users` AS L "
                         "JOIN `tau_groups` AS G "
                            "ON L.`id_groups` = G.`id` "
                         "WHERE L.`id_users` = :id_users");
    groups_query.bindValue(":id_users", stdnt->getId());
    groups_query.exec();
    int nbGroups = 0;
    QString request_groups = "";
    while(groups_query.next()) {
        int idGroup = groups_query.value(0).toInt();
        if(nbGroups)
            request_groups += + " OR `id_groups`=" + QString::number(idGroup);
        else
            request_groups = "`id_groups`=" + QString::number(idGroup);
        nbGroups++;
    }
//    m_dbase->listStudents()->value(stdnt->getId())->groups();

    if(nbGroups > 0) {
        /// Check if a course interfere with this timeslot
        //Get all courses that can interfere with this timeslot
        QSqlQuery courses_query(*m_db);
        courses_query.prepare("SELECT `id_subjects`, `id_teachers` FROM `tau_courses` WHERE (" + request_groups + ") AND `id_day`=:id_day AND `id_week`=:id_week AND ("
                                                                              "(`time_start` <= :time_start AND `time_end` > :time_start) OR"
                                                                              "(`time_start` < :time_end AND `time_end` >= :time_end) OR"
                                                                              "(`time_start` >= :time_start AND `time_end` <= :time_end) )");
        courses_query.bindValue(":time_start", timeslot->getTime_start().toString("HH:mm:ss"));
        courses_query.bindValue(":time_end", timeslot->getTime_end().toString("HH:mm:ss"));
        courses_query.bindValue(":id_day", timeslot->getDate().dayOfWeek());
        courses_query.bindValue(":id_week", m_id_week);
        courses_query.exec();

        // If there is a course or more --> Not compatible
        if(courses_query.next()) {
            QString res = "Cours : ";
            int idSubject = courses_query.value(0).toInt();
            int idTeacher = courses_query.value(1).toInt();
            QSqlQuery info(*m_db);
            info.prepare("SELECT `name` FROM `tau_subjects` WHERE `id` = :id");
            info.bindValue(":id", idSubject);
            info.exec();
            if(info.next())
                res += info.value(0).toString();
            info.prepare("SELECT `name` FROM `tau_teachers` WHERE `id` = :id");
            info.bindValue(":id", idTeacher);
            info.exec();
            if(info.next())
                res += " (" + info.value(0).toString() + ")";
            return res;
        }

        /// Check if a event interfere with this timeslot
        //Get all events that can interfere with this timeslot
        QSqlQuery event_query(*m_db);
        event_query.prepare("SELECT `name` FROM `tau_events` AS E JOIN `tau_events_groups` AS G ON E.`id` = G.`id_events` WHERE (" + request_groups + ") AND ("
                                      "(E.`start` <= :start_time AND E.`end` > :start_time) OR"
                                      "(E.`start` < :end_time AND E.`end` >= :end_time) OR"
                                      "(E.`start` >= :start_time AND E.`end` <= :end_time))");
        event_query.bindValue(":start_time", QDateTime(timeslot->getDate(), timeslot->getTime_start()).toString("yyyy-MM-dd HH:mm:ss"));
        event_query.bindValue(":end_time", QDateTime(timeslot->getDate(), timeslot->getTime_end()).toString("yyyy-MM-dd HH:mm:ss"));
        event_query.exec();

        // If there is a event or more --> Not compatible
        if(event_query.next())
            return "Evènement : " + event_query.value(0).toString();
    }

    /// Check if a kholle interfere with this timeslot
    //Get all kholles that can interfere with this timeslot
    QSqlQuery kholle_query(*m_db);
    kholle_query.prepare("SELECT T.`id_kholleurs` FROM `tau_kholles` AS K JOIN `tau_timeslots` AS T ON K.`id_timeslots` = T.`id` "
                         "WHERE K.`id_users`=:id_users AND T.`date`=:date AND T.`id` != :id_timeslots AND ("
                         "(T.`time_start` <= :time_start AND T.`time_end` > :time_start) OR "
                         "(T.`time_start` < :time_end AND T.`time_end` >= :time_end) OR "
                         "(T.`time_start` >= :time_start AND T.`time_end` <= :time_end))");
    kholle_query.bindValue(":id_users", stdnt->getId());
    kholle_query.bindValue(":id_timeslots", timeslot->getId());
    kholle_query.bindValue(":date", timeslot->getDate().toString("yyyy-MM-dd"));
    kholle_query.bindValue(":time_start", timeslot->getTime_start().toString("HH:mm:ss"));
    kholle_query.bindValue(":time_end", timeslot->getTime_end().toString("HH:mm:ss"));
    kholle_query.exec();

    // If there is a kholle or more --> Not compatible
    if(kholle_query.next()) {
        QString res = "Kholle : ";
        QString nameKh = "??";
        int idKholleur = kholle_query.value(0).toInt();
        int idSubject = 0;
        QSqlQuery info(*m_db);
        info.prepare("SELECT `name`, `id_subjects` FROM `tau_kholleurs` WHERE `id` = :id");
        info.bindValue(":id", idKholleur);
        info.exec();
        if(info.next()) {
            nameKh = info.value(0).toString();
            idSubject = info.value(1).toInt();
        }
        info.prepare("SELECT `name` FROM `tau_subjects` WHERE `id` = :id");
        info.bindValue(":id", idSubject);
        info.exec();
        if(info.next())
            res += info.value(0).toString() + " (" + nameKh + ")";
        return res;
    }

    /// Else, compatible
    return "";
}

bool LastChanges::update_khollesManager() {
    ui->comboBox_subjects->setEnabled(ui->radioButton_subject->isChecked());
    return true;
}

bool LastChanges::delete_kholles() {
    bool onlySubject = ui->radioButton_subject->isChecked();
    int idSubject = ui->comboBox_subjects->currentData().toInt();
    QString nameSubject = ui->comboBox_subjects->currentText();

    QString confirmation1 = "", confirmation2 = "";
    if(onlySubject) {
        confirmation1 = "Vous êtes sur le point de supprimer <strong>TOUTES</strong> les kholles de <strong>"+nameSubject+"</strong> de la <strong>semaine du " + m_monday->toString("dd/MM/yyyy") + "</strong> !!<br />"
                        "Voulez-vous vraiment continuez ?";
        confirmation2 = "Seconde confirmation : Voulez-vous vraiment supprimer <strong>TOUTES</strong> les kholles de <strong>"+nameSubject+"</strong> de la <strong>semaine du " + m_monday->toString("dd/MM/yyyy") + "</strong> ?<br />"
                        "Vous ne pourrez pas revenir en arrière !!";
    } else {
        confirmation1 = "Vous êtes sur le point de supprimer <strong>TOUTES</strong> les kholles de la <strong>semaine du " + m_monday->toString("dd/MM/yyyy") + "</strong> !!<br />"
                        "Voulez-vous vraiment continuez ?";
        confirmation2 = "Seconde confirmation : Voulez-vous vraiment supprimer <strong>TOUTES</strong> les kholles de la <strong>semaine du " + m_monday->toString("dd/MM/yyyy") + "</strong> ?<br />"
                        "Vous ne pourrez pas revenir en arrière !!";
    }

    int res = QMessageBox::warning(NULL, "Suppression de kholles", confirmation1, QMessageBox::Yes | QMessageBox::Cancel);
    if(res == QMessageBox::Yes) {
        res = QMessageBox::warning(NULL, "Suppression de kholles", confirmation2, QMessageBox::Yes | QMessageBox::Cancel);
        if(res == QMessageBox::Yes) {
            QSqlQuery qDelete(*m_db);
            if(onlySubject) {
                qDelete.prepare("DELETE FROM `tau_kholles` WHERE `id_timeslots` IN "
                                    "(SELECT `id` FROM `tau_timeslots` WHERE `date` >=:start AND `date` < :end  AND `id_kholleurs` IN "
                                        "(SELECT `id` FROM `tau_kholleurs` WHERE `id_subjects` = :id_subjects) )");
                qDelete.bindValue(":start", *m_monday);
                qDelete.bindValue(":end", m_monday->addDays(7));
                qDelete.bindValue(":id_subjects", idSubject);
            } else {
                qDelete.prepare("DELETE FROM `tau_kholles` WHERE `id_timeslots` IN "
                                    "(SELECT `id` FROM `tau_timeslots` WHERE `date` >=:start AND `date` < :end)");
                qDelete.bindValue(":start", *m_monday);
                qDelete.bindValue(":end",  m_monday->addDays(7));
            }
            qDelete.exec();
            int nbDeletedKholle = qDelete.numRowsAffected();
            if(nbDeletedKholle < 0)
                QMessageBox::critical(NULL, "Echec de la suppression", "La suppression a échoué : " + QString::number(nbDeletedKholle) + " kholles ont été supprimé");
            else if(nbDeletedKholle == 0 || nbDeletedKholle == 1)
                QMessageBox::information(NULL, "Suppression effectuée", "La suppression des kholles a été effectuée : " + QString::number(nbDeletedKholle) + " kholle a été supprimée");
            else
                QMessageBox::information(NULL, "Suppression effectuée", "La suppression des kholles a été effectuée : " + QString::number(nbDeletedKholle) + " kholles ont été supprimées");
        }
    }

    update_timeslotsList();

    return true;
}
