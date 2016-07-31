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
    //Connect the finish button to save on finish
    connect(wizard()->button(QWizard::FinishButton), SIGNAL(clicked()), this, SLOT(saveKholles()));

    //Initialise random number
    qsrand((uint)QTime::currentTime().msec());

    m_week = field("current_week").toInt() + 1;
    m_date = field("monday_date").toDateTime().date();
    if(m_date.dayOfWeek() != 1) {
        while(m_date.dayOfWeek() != 1)
            m_date = m_date.addDays(-1);
        QMessageBox::information(this, "Information", "La date n'étant pas à un lundi, la date utilisée sera le lundi de la même semaine.");
    }

    freeKholles();
    getKholleurs();
    getTimeslots();
    loadSubjects();

    calculateProba();
    constructPoss();
    generate();
    display();
}

void GeneratePage::getTimeslots() {
    freeTimeslots();

    //Prepare query
    QSqlQuery query(*m_db);
    query.prepare("SELECT id, time, time_start, time_end, id_kholleurs, date, pupils FROM tau_timeslots WHERE date>=:monday_date AND date<=:sunday_date");
    query.bindValue(":monday_date", m_date.toString("yyyy-MM-dd"));
    query.bindValue(":sunday_date", m_date.addDays(6).toString("yyyy-MM-dd"));
    query.exec();
    //QMessageBox::information(this, "OK", query.executedQuery());

    //Treat
    while(query.next()) {
        Timeslot *ts = new Timeslot();
        ts->setId(query.value(0).toInt());
        ts->setTime(QTime::fromString(query.value(1).toString(), "h:mm:ss"));
        ts->setTime_start(QTime::fromString(query.value(2).toString(), "h:mm:ss"));
        ts->setTime_end(QTime::fromString(query.value(3).toString(), "h:mm:ss"));
        ts->setId_kholleurs(query.value(4).toInt());
        ts->setDate(QDate::fromString(query.value(5).toString(), "yyyy-M-d"));
        ts->setPupils(query.value(6).toInt());
        timeslots.insert(ts->getId(), ts);
    }
}

void GeneratePage::freeTimeslots() {
    QList<int> keys = timeslots.keys();
    int i;
    for(i = 0; i < keys.length(); i++) {
        delete timeslots.take(keys[i]);
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
        delete kholleurs.take(keys[i]);
    }
}

void GeneratePage::calculateProba() {
    //Get the input (which users have been chosen on previous page)
    QMap<int, QList<Student*> > *input = ((KholloscopeWizard*) wizard())->get_input();

    //For every kholleur
    foreach(Kholleur* k, kholleurs) {
        QMap<int, float> map; //Create a new map of floats

        QList<Student*> users = input->value(k->getId_subjects()); //Get the chosen users for this subject

        int i;
        for(i = 0; i < users.length(); i++) { //For each student
            float p = 100; //Calculate probability here

            //Get all past (and future) kholles for the same student with same kholleur
            QSqlQuery kholle_query(*m_db);
            kholle_query.prepare("SELECT K.`id`, K.`id_users`, K.`id_timeslots`, T.`date` "
                                 "FROM tau_kholles AS K JOIN tau_timeslots AS T ON K.`id_timeslots` = T.`id`"
                                 "WHERE K.`id_users` = :id_student AND T.`id_kholleurs` = :id_kholleurs ORDER BY T.`date` DESC");
            kholle_query.bindValue(":id_student", users[i]->getId());
            kholle_query.bindValue(":id_kholleurs", k->getId());
            kholle_query.exec();

            while(kholle_query.next()) {
                //-10 for the every kholle with the same kholleur
                p -= 10;

                QDate kholle_date = QDate::fromString(kholle_query.value(3).toString(), "yyyy-M-d");

                if(kholle_date >= m_date.addDays(-21) && kholle_date <= m_date.addDays(27)) //-30 if within 3 weeks
                    p -= 30;

                if(kholle_date >= m_date.addDays(-14) && kholle_date <= m_date.addDays(20)) //-40 if within 2 weeks
                    p -= 10;

                if(kholle_date >= m_date.addDays(-7) && kholle_date <= m_date.addDays(13)) //-50 if within a week
                    p -= 10;
            }

            map.insert(users[i]->getId(), p); //Insert the new probability
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

    if(query.next()) {
        QString request = "id_groups=" + QString::number(query.value(0).toInt());
        while(query.next()) {
            request = request + " OR id_groups=" + QString::number(query.value(0).toInt());
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
        QDate new_date = m_date.addDays(timeslot->getDate().dayOfWeek() - 1);

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
    /** QuickSort of a QList<Timeslot*> based on the value of the probabilities **/
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

    int i, j;
    for(i = 0; i < selected_subjects->length(); i++) { //For every selected subject
        //Create a new map
        QMap<int, QList<Timeslot*> > map;

        //The selected users for this subject
        QList<Student*> users = input->value(selected_subjects->at(i)->getId());

        for(j = 0; j < users.length(); j++) { //For every selected user
            //Create new list
            QList<Timeslot*> new_list;

            foreach(Timeslot* ts, timeslots) { //For every timeslot
                Subject* sub = subjects.value(kholleurs.value(ts->getId_kholleurs())->getId_subjects());

                if(sub == selected_subjects->at(i) && compatible(users[j]->getId(), ts)) { //Add the compatible timeslots
                    new_list.append(ts);
                }
            }
            quickSort(&new_list, 0, new_list.length() - 1, users[j]->getId()); //Sort the list based on probabilities

            /*QString string;
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

int GeneratePage::my_count(QList<Timeslot *> list) {
    /** Own count function, takes into account the timeslots with 0 free pupils left **/
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
    /** To find the place with least possibilities **/
    int min = -1;
    QList<int> s_mins;
    QList<int> u_mins;

    QList<int> s_keys = poss.keys();
    int i, j;
    for(i = 0; i < s_keys.length(); i++) {
        QList<int> u_keys = poss.value(s_keys[i]).keys();

        for(j = 0; j < u_keys.length(); j++) {
            int count = my_count(poss.value(s_keys[i]).value(u_keys[j]));
            if(min == -1 || count < min) {
                min = count;
                s_mins.clear();
                u_mins.clear();
                s_mins.append(s_keys[i]);
                u_mins.append(u_keys[j]);
            }
            if(count == min) {
                s_mins.append(s_keys[i]);
                u_mins.append(u_keys[j]);
            }
        }
    }

    working_index *res = (working_index*) malloc(sizeof(working_index));
    res->min = min;

    //Get random number
    if(s_mins.length() > 0) {
        int random_int = qrand() % s_mins.length();
        res->current_student = u_mins[random_int];
        res->current_subject = s_mins[random_int];
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
    working_index* index = findMin(); //Get the min
    //QMessageBox::information(this, "OK", QString::number(profondeur));

    //It is finished
    if(index->min == -1) {
        free(index);
        return true;
    }

    QMap<int, QList<Timeslot*> > map = poss.value(index->current_subject);
    QList<Timeslot*> loop = map.take(index->current_student); //Get possibilities
    poss.insert(index->current_subject, map);

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
                free(index);
                return true;
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

void GeneratePage::display() {
    /** To display in the list **/
    ui->listWidget->clear();
    int i;
    for(i = 0; i < kholloscope.length(); i++) {
        ui->listWidget->addItem(QString::number(kholloscope[i]->getId_students()) + ", " + QString::number(kholloscope[i]->getId_timeslots()));
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

void GeneratePage::saveKholles() {
    /** To save the kholloscope that has been generated to the database **/
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

    if(ui->printCheckBox->isChecked())
        printKholles();
}

void GeneratePage::freeKholles() {
    while(!kholloscope.isEmpty()) {
        delete kholloscope.takeFirst();
    }
}

int GeneratePage::longestUser(QFontMetrics font) {
    int max = 0;
    QList<Student*> *students = ((KholloscopeWizard*) wizard())->get_students();

    int i;
    for(i = 0; i < students->length(); i++) {
        int width = font.width(students->at(i)->getName() + " " + students->at(i)->getFirst_name());
        if(width > max) {
            max = width;
        }
    }
    return max;
}

int GeneratePage::longestKholleur(QFontMetrics font) {
    int max = 0;

    foreach(Kholleur* k, kholleurs) {
        int width = font.width("20:00 " + k->getName());
        if(width > max) {
            max = width;
        }
    }
    return max;
}

void GeneratePage::printKholles() {
    //Get file name
    QString filename = QFileDialog::getSaveFileName(this, "Enregistrer sous...", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),  "PDF (*.pdf)");
    QMessageBox::information(this, "OK", filename);

    //Create the PDF Writer
    QPdfWriter writer(filename);
    writer.setPageSize(QPdfWriter::A3);
    writer.setPageOrientation(QPageLayout::Landscape);
    writer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    QPainter painter;

    if(!painter.begin(&writer)) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de l'écriture du fichier. Le fichier est peut-être ouvert ?");
        return;
    }

    //Get size in default units
    int width = writer.width();
    int height = writer.height();

    //Paint here
    painter.setPen(QPen(QBrush(Qt::black), 5));

    //Calculate line height and cell width

    //Number of rows
    QSqlQuery numquery(*m_db);
    numquery.exec("SELECT COUNT(id) FROM tau_users WHERE is_deleted = 0");
    int num_rows = 0;
    if(numquery.next())
        num_rows = numquery.value(0).toInt() + 3;
    else {
        QMessageBox::critical(this, "Erreur", "Erreur avec la base de données");
        return;
    }

    int row_height = height / num_rows;

    //Create two fonts -> one for the normal text, one for title
    QFont normal_font = painter.font();
    if(QFontMetrics(normal_font).lineSpacing() > row_height) {
        while(QFontMetrics(normal_font).lineSpacing() > row_height)
            normal_font.setPointSize(normal_font.pointSize() - 1);
    }
    if(QFontMetrics(normal_font).lineSpacing() < row_height) {
        while(QFontMetrics(normal_font).lineSpacing() <= row_height)
            normal_font.setPointSize(normal_font.pointSize() + 1);
        normal_font.setPointSize(normal_font.pointSize() - 1);
    }

    QFont title_font = painter.font();
    if(QFontMetrics(title_font).lineSpacing() > 2*row_height) {
        while(QFontMetrics(title_font).lineSpacing() > 2*row_height)
            title_font.setPointSize(title_font.pointSize() - 1);
    }
    if(QFontMetrics(title_font).lineSpacing() < 2*row_height) {
        while(QFontMetrics(title_font).lineSpacing() <= 2*row_height)
            title_font.setPointSize(title_font.pointSize() + 1);
        title_font.setPointSize(title_font.pointSize() - 1);
    }

    //Get font metrics
    painter.setFont(normal_font);
    QFontMetrics font = painter.fontMetrics();

    //Name length (length of longest name) => also the width of 1st column
    int name_width = longestUser(font);
    int cell_width = (width - name_width) / 6;
    int kholleur_width = longestKholleur(font);

    //Draw the title
    QString title = "Semaine du lundi " + m_date.toString("dd/MM/yyyy") + " au samedi " + m_date.addDays(5).toString("dd/MM/yyyy");
    painter.setFont(title_font);
    font = painter.fontMetrics();
    painter.drawText((width - font.width(title)) / 2, font.ascent() + font.leading()/2, title);

    //Draw the grid
    painter.drawLine(0, 2*row_height, 0, height);
    painter.drawLine(name_width, 2*row_height, name_width, height);
    painter.drawLine(name_width + cell_width, 2*row_height, name_width + cell_width, height);
    painter.drawLine(name_width + 2*cell_width, 2*row_height, name_width + 2*cell_width, height);
    painter.drawLine(name_width + 3*cell_width, 2*row_height, name_width + 3*cell_width, height);
    painter.drawLine(name_width + 4*cell_width, 2*row_height, name_width + 4*cell_width, height);
    painter.drawLine(name_width + 5*cell_width, 2*row_height, name_width + 5*cell_width, height);
    painter.drawLine(name_width + 6*cell_width, 2*row_height, name_width + 6*cell_width, height);

    painter.drawLine(0, 2*row_height, width, 2*row_height);

    painter.setFont(normal_font);
    font = painter.fontMetrics();
    painter.drawText(name_width + cell_width/2 - font.width("Lundi")/2, 2*row_height + font.ascent() + font.leading()/2, "Lundi");
    painter.drawText(name_width + 3*cell_width/2 - font.width("Mardi")/2, 2*row_height + font.ascent() + font.leading()/2, "Mardi");
    painter.drawText(name_width + 5*cell_width/2 - font.width("Mercredi")/2, 2*row_height + font.ascent() + font.leading()/2, "Mercredi");
    painter.drawText(name_width + 7*cell_width/2 - font.width("Jeudi")/2, 2*row_height + font.ascent() + font.leading()/2, "Jeudi");
    painter.drawText(name_width + 9*cell_width/2 - font.width("Vendredi")/2, 2*row_height + font.ascent() + font.leading()/2, "Vendredi");
    painter.drawText(name_width + 11*cell_width/2 - font.width("Samedi")/2, 2*row_height + font.ascent() + font.leading()/2, "Samedi");

    painter.drawLine(0, 3*row_height, width, 3*row_height);

    QList<Student*> *students = ((KholloscopeWizard*) wizard())->get_students();
    QMap<int, int> assoc;

    //Student names
    int i, j, k;
    for(i = 0; i < students->length(); i++) {
        painter.drawText(0, (3+i)*row_height + font.ascent() + font.leading()/2, students->at(i)->getName() + " " + students->at(i)->getFirst_name());
        painter.drawLine(0, (4+i)*row_height, width, (4+i)*row_height);
        assoc.insert(students->at(i)->getId(), i);
    }

    //Reorganise the kholles
    QList<QList<QList<Kholle*>*>*> kholles;
    for(i = 0; i < students->length(); i++) {
        kholles.append(new QList<QList<Kholle*>*>);
        for(j = 0; j < 6; j++) {
            kholles[i]->append(new QList<Kholle*>);
        }
    }

    for(i = 0; i < kholloscope.length(); i++) {
        kholles[assoc.value(kholloscope[i]->getId_students())]->at(timeslots.value(kholloscope[i]->getId_timeslots())->getDate().dayOfWeek() - 1)->append(kholloscope[i]);
    }

    for(i = 0; i < students->length(); i++) {
        for(j = 0; j < 6; j++) {
            QList<Kholle*> *list = kholles[i]->at(j);
            int k_width = kholleur_width; //Copy kholleur_width

            if(list->length() * kholleur_width > cell_width) {
                QFont small_font(normal_font);
                while(longestKholleur(QFontMetrics(small_font)) * list->length() > cell_width)
                    small_font.setPointSize(small_font.pointSize() - 1);

                painter.setFont(small_font);
                k_width = longestKholleur(QFontMetrics(small_font)); //Update if necessary
            }

            for(k = 0; k < list->length(); k++) { //Paint
                painter.drawText(name_width + j*cell_width + k*k_width,
                                 (3+i)*row_height + font.ascent() + font.leading()/2,
                                 timeslots.value(list->at(k)->getId_timeslots())->getTime_start().toString("HH:mm") + " "
                                 + kholleurs.value(timeslots.value(list->at(k)->getId_timeslots())->getId_kholleurs())->getName());
            }
        }
    }

    painter.end();
}
