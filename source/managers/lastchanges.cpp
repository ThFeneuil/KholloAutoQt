#include "lastchanges.h"
#include "ui_lastchanges.h"

LastChanges::LastChanges(QSqlDatabase *db, int id_week, QDate *monday, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LastChanges)
{
    ui->setupUi(this);

    // Initialisation
    m_db = db;
    m_monday = monday;
    m_id_week = id_week;
    setWindowTitle("Outil de dépannage : semaine du " + m_monday->toString("dd/MM/yyyy") + " (" + (m_id_week == 1 ? "paire" : "impaire") + ")");

    days << "" << "Lundi" << "Mardi" << "Mercredi" << "Jeudi" << "Vendredi" << "Samedi" << "Dimanche";
    m_shortcutNotepad = Notepad::shortcut();
    this->addAction(m_shortcutNotepad);

    QSqlQuery query(*m_db);
    query.exec("SELECT K.id, K.name, K.id_subjects, K.duration, K.preparation, K.pupils, S.shortName "
               "FROM tau_kholleurs AS K "
               "LEFT OUTER JOIN tau_subjects AS S "
                  "ON K.id_subjects = S.id "
               "ORDER BY UPPER(K.name)");

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
    connect(ui->tableWidget_student, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(change_status_student(QTableWidgetItem*)));

    connect(ui->pushButton_valid, SIGNAL(clicked(bool)), this, SLOT(save_timeslotsChanges()));
    connect(ui->pushButton_interface, SIGNAL(clicked(bool)), this, SLOT(open_interface()));
    connect(ui->pushButton_reset, SIGNAL(clicked(bool)), this, SLOT(change_timeslotsList()));
    change_timeslotsList();

    connect(ui->radioButton_subject, SIGNAL(clicked(bool)), this, SLOT(update_khollesManager()));
    connect(ui->radioButton_all, SIGNAL(clicked(bool)), this, SLOT(update_khollesManager()));
    connect(ui->pushButton_delete, SIGNAL(clicked(bool)), this, SLOT(delete_kholles()));
    update_khollesManager();

    connect(ui->pushButton_resetMerging, SIGNAL(clicked(bool)), this, SLOT(reset_preferencesMerging()));

    QSqlQuery query_subj(*m_db);
    query_subj.exec("SELECT `id`, `name` FROM `tau_subjects` ORDER BY UPPER(`name`)");

    while (query_subj.next())
        ui->comboBox_subjects->addItem(query_subj.value(1).toString(), query_subj.value(0).toInt());
}

LastChanges::~LastChanges()
{
    delete ui;
    // To free memories with kholleurs
    while (!queue_displayedKholleurs.isEmpty())
        delete queue_displayedKholleurs.dequeue();
    free_timeslots();
    this->removeAction(m_shortcutNotepad);
    delete m_shortcutNotepad;
}

bool LastChanges::free_timeslots() {
    /** To free memories with timeslots **/
    while (!queue_displayedTimeslots.isEmpty()) {
        TimeslotChg* tsChg= queue_displayedTimeslots.dequeue();
        delete tsChg->start;
        delete tsChg->end;
        delete tsChg;
    }
    return true;
}

bool LastChanges::change_timeslots(QListWidgetItem *item) {
    TimeslotChg* tsChg = (TimeslotChg*) item->data(Qt::UserRole).toLongLong();
    UpdateTimeslotDialog dialog(tsChg->end, this);
    dialog.exec();

    update_timeslotsList(tsChg->start->getId());
    return true;
}

bool LastChanges::change_timeslotsList() {
    // Cleaning
    free_timeslots();
    ui->list_timeslots->clear();
    m_students.clear();

    // Get the kholleur
    Kholleur* khll = (Kholleur*) ui->comboBox_kholleurs->currentData().toLongLong();
    if(! khll)
        return false;
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

        QSqlQuery queryStudents(*m_db);
        queryStudents.prepare("SELECT S.`id`, S.`name`, S.`first_name`, S.`email` "
                      "FROM `tau_kholles` AS K "
                      "JOIN `tau_users` AS S "
                        "ON K.`id_users` = S.`id` "
                      "WHERE K.`id_timeslots` = :id_timeslots "
                      "ORDER BY UPPER(S.`name`), UPPER(S.`first_name`)");
        queryStudents.bindValue(":id_timeslots", tsChg->start->getId());
        queryStudents.exec();

        while(queryStudents.next()) {
            Student* stdnt = new Student();
            stdnt->setId(queryStudents.value(0).toInt());
            stdnt->setName(queryStudents.value(1).toString());
            stdnt->setFirst_name(queryStudents.value(2).toString());
            stdnt->setEmail(queryStudents.value(3).toString());

            StudentKholleChg* kholleChg = new StudentKholleChg();
            kholleChg->stdnt = stdnt;
            kholleChg->tsChg = tsChg;
            kholleChg->numTs = num;
            kholleChg->statusMessage = compatible(stdnt, tsChg->end);
            kholleChg->status = (kholleChg->statusMessage == "") ? OK : NotKeep;

            m_students.append(kholleChg);
        }
    }

    update_students();

    return true;
}

bool LastChanges::update_timeslotsList(int idTs) {
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

    update_students(idTs);
    return true;
}

bool LastChanges::update_students(int idTs) {
    ui->tableWidget_student->clear();
    ui->tableWidget_student->setRowCount(m_students.count());
    ui->tableWidget_student->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for(int numRow=0; numRow<m_students.count(); numRow++) {
        Timeslot* final = m_students[numRow]->tsChg->end;

        QTableWidgetItem* left = new QTableWidgetItem(m_students[numRow]->stdnt->getName() + " " + m_students[numRow]->stdnt->getFirst_name() + " #"+QString::number(m_students[numRow]->numTs));
        QTableWidgetItem* right = new QTableWidgetItem();

        if(idTs < 0 || idTs == m_students[numRow]->tsChg->start->getId()) {
            if(final->isDeleted()) {
                left->setIcon(QIcon(QPixmap(":/images/warning.png")));
                m_students[numRow]->status = ImpossibleToKeep;
            } else {
                m_students[numRow]->statusMessage = compatible(m_students[numRow]->stdnt, final);
                if(m_students[numRow]->statusMessage == "") {
                    left->setIcon(QIcon(QPixmap(":/images/ok.png")));
                    m_students[numRow]->status = OK;
                } else {
                    left->setIcon(QIcon(QPixmap(":/images/error.png")));
                    right->setText(m_students[numRow]->statusMessage);
                    m_students[numRow]->status = NotKeep;
                }
            }
        } else {
            switch(m_students[numRow]->status) {
            case OK:
            case Keep:
                left->setIcon(QIcon(QPixmap(":/images/ok.png")));
                break;
            case Remove:
                left->setIcon(QIcon(QPixmap(":/images/delete.png")));
                break;
            case NotKeep:
                left->setIcon(QIcon(QPixmap(":/images/error.png")));
                break;
            case ImpossibleToKeep:
                left->setIcon(QIcon(QPixmap(":/images/warning.png")));
                break;
            }
            right->setText(m_students[numRow]->statusMessage);
        }

        ui->tableWidget_student->setItem(numRow, 0, left);
        ui->tableWidget_student->setItem(numRow, 1, right);
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
        courses_query.prepare("SELECT `id_subjects` FROM `tau_courses` WHERE (" + request_groups + ") AND `id_day`=:id_day AND `id_week`=:id_week AND ("
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
            QSqlQuery info(*m_db);
            info.prepare("SELECT `name` FROM `tau_subjects` WHERE `id` = :id");
            info.bindValue(":id", idSubject);
            info.exec();
            res += info.next() ? info.value(0).toString() : "???";
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

bool LastChanges::save_timeslotsChanges() {
    int res = QMessageBox::warning(this, "Avertissement", "Vous êtes sur le point d'enregistrer toutes les modifications. <br />"
                                                          "Toutes les kholles sans le status OK seront supprimées. <br />"
                                                          "Voulez-vous continuez ?", QMessageBox::Yes | QMessageBox::Cancel);
    if(res != QMessageBox::Yes)
        return false;

    QSqlQuery query(*m_db);
    int numStudents = 0;
    QString textNotepad = "";
    for(int i=0; i<ui->list_timeslots->count(); i++) {
        QListWidgetItem* item = ui->list_timeslots->item(i);
        TimeslotChg* tsChg = (TimeslotChg*) item->data(Qt::UserRole).toLongLong();
        Timeslot* initial = tsChg->start;
        Timeslot* final = tsChg->end;

        query.prepare("SELECT S.`id`, S.`name`, S.`first_name` "
                      "FROM `tau_kholles` AS K "
                      "JOIN `tau_users` AS S "
                        "ON K.`id_users` = S.`id` "
                      "WHERE K.`id_timeslots` = :id_timeslots "
                      "ORDER BY UPPER(S.`name`), UPPER(S.`first_name`)");
        query.bindValue(":id_timeslots", initial->getId());
        query.exec();

        while(query.next()) {
            Student stdnt;
            stdnt.setId(query.value(0).toInt());
            stdnt.setName(query.value(1).toString());
            stdnt.setFirst_name(query.value(2).toString());

            if(final->isDeleted() || (m_students[numStudents]->status != Keep && m_students[numStudents]->status != OK)) {
                QSqlQuery queryRemove(*m_db);
                queryRemove.prepare("DELETE FROM `tau_kholles` WHERE `id_timeslots` = :id_timeslots AND `id_users` = :id_users");
                queryRemove.bindValue(":id_timeslots", final->getId());
                queryRemove.bindValue(":id_users", stdnt.getId());
                queryRemove.exec();

                textNotepad += stdnt.getName() + " " + stdnt.getFirst_name() + ": kholle avec " + ui->comboBox_kholleurs->currentText() + "\n";
            }

            numStudents++;
        }

        if(final->isDeleted()) {
            /*query.prepare("DELETE FROM `tau_kholles` WHERE `id_timeslots` = :id_timeslots");
            query.bindValue(":id", final->getId());
            query.exec();*/
            query.prepare("DELETE FROM `tau_timeslots` WHERE `id` = :id");
            query.bindValue(":id", final->getId());
            query.exec();
        } else {
            query.prepare("UPDATE `tau_timeslots` SET `date`=:date, `time_start`=:time_start, `time`=:time, `time_end`=:time_end WHERE `id`=:id");
            query.bindValue(":date", final->getDate().toString("yyyy-MM-dd"));
            query.bindValue(":time_start", final->getTime_start().toString("hh:mm:ss"));
            query.bindValue(":time", final->getTime().toString("hh:mm:ss"));
            query.bindValue(":time_end", final->getTime_end().toString("hh:mm:ss"));
            query.bindValue(":id", final->getId());
            query.exec();
        }
    }

    change_timeslotsList();
    res = QMessageBox::information(this, "Succès", "Toutes les changements d'horaires de kholles ont été sauvegardés.<br />"
                                "Voulez-vous garder les kholles supprimées dans un bloc-note ?", QMessageBox::Yes | QMessageBox::No);
    if(res == QMessageBox::Yes) {
        if(textNotepad == "")
            textNotepad = "Aucune kholle n'a été supprimée...";
        Notepad::add("Outils de dépannage", "====== Outis de dépannage (" + QTime::currentTime().toString("hh:mm:ss") + ") ======\n" + textNotepad);
    }

    return true;
}

bool LastChanges::change_status_student(QTableWidgetItem* item) {
    int row = item->row();
    switch(m_students[row]->status) {
    case OK:
        m_students[row]->status = Remove;
        ui->tableWidget_student->item(row, 0)->setIcon(QIcon(QPixmap(":/images/delete.png")));
        break;
    case Remove:
        m_students[row]->status = OK;
        ui->tableWidget_student->item(row, 0)->setIcon(QIcon(QPixmap(":/images/ok.png")));
        break;
    case Keep:
        m_students[row]->status = NotKeep;
        ui->tableWidget_student->item(row, 0)->setIcon(QIcon(QPixmap(":/images/error.png")));
        break;
    case NotKeep:
        m_students[row]->status = Keep;
        ui->tableWidget_student->item(row, 0)->setIcon(QIcon(QPixmap(":/images/ok.png")));
        break;
    case ImpossibleToKeep:
        QMessageBox::critical(this, "Echec", "Impossible de garder cette kholle. L'horaire de kholle a été supprimé...");
        break;
    default:
        break;
    }
    return true;
}

bool LastChanges::open_interface() {
    InterfaceDialog dialog(m_db, m_id_week, *m_monday, this);
    dialog.exec();
    return change_timeslotsList();
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

    int res = QMessageBox::warning(this, "Suppression de kholles", confirmation1, QMessageBox::Yes | QMessageBox::Cancel);
    if(res == QMessageBox::Yes) {
        res = QMessageBox::warning(this, "Suppression de kholles", confirmation2, QMessageBox::Yes | QMessageBox::Cancel);
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
                QMessageBox::critical(this, "Echec de la suppression", "La suppression a échoué : " + QString::number(nbDeletedKholle) + " kholles ont été supprimé");
            else if(nbDeletedKholle == 0 || nbDeletedKholle == 1)
                QMessageBox::information(this, "Suppression effectuée", "La suppression des kholles a été effectuée : " + QString::number(nbDeletedKholle) + " kholle a été supprimée");
            else
                QMessageBox::information(this, "Suppression effectuée", "La suppression des kholles a été effectuée : " + QString::number(nbDeletedKholle) + " kholles ont été supprimées");
        }
    }

    update_timeslotsList();

    return true;
}

bool LastChanges::reset_preferencesMerging() {
    int res = QMessageBox::warning(this, "Réinitialisation", "Vous être sur le point de réinitialiser toutes les préférences pour les associations des kholleurs téléchargés avec les horaires de kholles. Voulez-vous continuer ?", QMessageBox::Yes | QMessageBox::Cancel);
    if(res == QMessageBox::Yes) {
        QSqlQuery query(*m_db);
        query.exec("DELETE FROM tau_merge_kholleurs");
        QMessageBox::information(this, "Réinitialisation", "Réinitialisation réussite !");
    }
    return true;
}
