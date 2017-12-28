#include "generatepage.h"
#include "ui_generatepage.h"

GeneratePage::GeneratePage(QSqlDatabase *db, QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::GeneratePage)
{
    ui->setupUi(this);

    //DB
    m_db = db;
    m_dbase = NULL;

    m_genMethod = NULL;

    //Create message
    m_box = new QMessageBox(QMessageBox::Information, "Génération automatique", "Génération en cours. Veuillez patienter...",
                            QMessageBox::NoButton, this);

    //Pointer to MainWindow
    m_window = parent;

    //Connect buttons to slots
    connect(ui->notepad_khollo, SIGNAL(clicked()), this, SLOT(show_notepad_khollo()));
    connect(ui->notepad_collisions, SIGNAL(clicked()), this, SLOT(show_notepad_collisions()));
}

GeneratePage::~GeneratePage() {
    if(m_genMethod != NULL)
        m_genMethod->rollback();

    delete ui;

    delete m_box;
    delete m_genMethod;
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

    delete m_genMethod; m_genMethod = NULL;
    m_genMethod = new LPMethod(m_db, m_date, m_week);

    //Warning if not enough free space for everyone...
    QList<Subject*> *problem_subjects = m_genMethod->testAvailability(((KholloscopeWizard*) wizard())->get_assoc_subjects(), ((KholloscopeWizard*) wizard())->get_input());
    if(problem_subjects->length() > 0) {
        QString warning_text = "Il n'y a pas suffisamment d'horaires de kholles libres pour accommoder tous les élèves sélectionnés dans ";
        warning_text += (problem_subjects->length() >= 2 ? "les matières suivantes : <br />" : "la matière suivante : <br />");
        int i;
        for(i = 0; i < problem_subjects->length(); i++)
            warning_text += "<strong>" + problem_subjects->at(i)->getName() + "</strong> <br />";
        QMessageBox::warning(this, "Attention", warning_text);
    }
    delete problem_subjects;

    m_box->show();
    //Start generating
    finished(m_genMethod->start(((KholloscopeWizard*) wizard())->get_assoc_subjects(), ((KholloscopeWizard*) wizard())->get_input()));
}

void GeneratePage::cleanupPage() {
    disconnect(wizard()->button(QWizard::FinishButton), SIGNAL(clicked()), this, SLOT(saveKholles()));

    //Abort operation
    m_genMethod->rollback();
    m_box->hide();

    //Clear list
    ui->tableKhollo->clear();
}

void GeneratePage::finished(bool success) {
    /** Called when the background process finishes **/
    timestamp = QString::number(QDateTime::currentMSecsSinceEpoch());

    if(!success) {
        m_genMethod->log("Erreur ! Il n'y a aucune solution...\n", true);
        m_box->hide();
        QMessageBox::critical(this, "Erreur", "Solution non trouvée... :'(");
        return;
    }

    m_genMethod->log("Affichage du kholloscope...\n", true);

    delete m_dbase;
    m_dbase = new DataBase(m_db);
    m_dbase->setConditionTimeslots("`date` >= '"+ m_date.toString("yyyy-MM-dd") +"' AND `date` < '" + m_date.addDays(7).toString("yyyy-MM-dd") + "'");
    m_dbase->load();

    int errors = 0, warnings = 0, collisions = 0;
    display(&errors, &warnings);
    displayCollision(&collisions);
    m_box->hide();

    m_genMethod->log("Affichage de la conclusion...\nGénération réussie !!\n\n", true);
    displayConclusion(errors, warnings, collisions);
}

void GeneratePage::display(int *errors, int *warnings) {
    QList<Kholle*> kholloscope = *(m_genMethod->kholloscope());

    /** To display in the list **/
    ui->tableKhollo->clear();
    ui->tableKhollo->setRowCount(kholloscope.length());
    ui->tableKhollo->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    khollo_message  = "========== Génération du " + m_date.toString("dd/MM/yyyy") + " ==========\n";
    khollo_message += "                                    Furieux et déçus\n\n";

    for(int i = 0; i < kholloscope.length(); i++) {
        Student* student = m_genMethod->listStudents()->value(kholloscope[i]->getId_students());
        Timeslot* timeslot = m_genMethod->listTimeslots()->value(kholloscope[i]->getId_timeslots());
        Subject* subject = timeslot->kholleur()->subject();
        Kholleur* kholleur = timeslot->kholleur();

        int weeks = kholloscope[i]->weeks();
        QTableWidgetItem *left = new QTableWidgetItem(student->getName() + ", " + subject->getShortName() + " :");

        if(kholloscope[i]->status() == Kholle::OK) {
            left->setIcon(QIcon(QPixmap(":/images/ok.png")));


            QTableWidgetItem *right = NULL;

            if(weeks == -1)
                    right = new QTableWidgetItem(kholleur->getName() + ", jamais");
            else    right = new QTableWidgetItem(kholleur->getName() + ", " + QString::number(weeks) + " semaines");
            ui->tableKhollo->setItem(i, 1, right);

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
        m_genMethod->commit();

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

