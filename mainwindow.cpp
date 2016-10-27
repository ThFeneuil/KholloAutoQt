#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_idRecord = -1;

    connect(ui->action_File_Quit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->action_DB_Students, SIGNAL(triggered()), this, SLOT(openStudentsManager()));
    connect(ui->action_DB_Groups, SIGNAL(triggered()), this, SLOT(openGroupsManager()));
    connect(ui->action_DB_Subjects, SIGNAL(triggered()), this, SLOT(openSubjectsManager()));
    connect(ui->action_DB_Kholleurs, SIGNAL(triggered()), this, SLOT(openKholleursManager()));
    connect(ui->action_Schedule_Students_Groups, SIGNAL(triggered()), this, SLOT(openUsersGroupsManager()));
    connect(ui->action_Schedule_Timetable, SIGNAL(triggered()), this, SLOT(openCoursesManager()));
    connect(ui->action_Schedule_Swapping_Groups, SIGNAL(triggered()), this, SLOT(openGroupsSwappingsManager()));
    connect(ui->action_Schedule_Kholles, SIGNAL(triggered()), this, SLOT(openTimeslotsManager()));
    connect(ui->action_Schedule_Events, SIGNAL(triggered()), this, SLOT(openEventsManager()));
    connect(ui->action_Kholles_Interface, SIGNAL(triggered()), this, SLOT(openInterface()));
    connect(ui->action_Kholles_Generate, SIGNAL(triggered()), this, SLOT(openKholloscope()));
    connect(ui->action_Kholles_Historic, SIGNAL(triggered()), this, SLOT(openReview()));
    connect(ui->action_Kholles_LastChanges, SIGNAL(triggered()), this, SLOT(openLastChanges()));
    connect(ui->action_Help, SIGNAL(triggered()), this, SLOT(openHelp()));
    connect(ui->action_AboutIt, SIGNAL(triggered()), this, SLOT(openAboutIt()));


    connect(ui->action_File_Create, SIGNAL(triggered()), this, SLOT(createKhollo()));
    connect(ui->action_File_Open, SIGNAL(triggered()), this, SLOT(openKhollo()));

    connect(this, SIGNAL(triggerInterface(QDate,int)), this, SLOT(openInterfaceWithDate(QDate,int)));

    updateWindow();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openStudentsManager() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        // Open the manager
        StudentsManager manager(&db, this);
        manager.exec();
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openGroupsManager() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        // Open the manager
        GroupsManager manager(&db, this);
        manager.exec();
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openSubjectsManager() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        // Open the manager
        SubjectsManager manager(&db, this);
        manager.exec();
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openKholleursManager() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        // Open the manager
        KholleursManager manager(&db, this);
        manager.exec();
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openUsersGroupsManager() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        // Open the manager
        UsersGroupsManager manager(&db, this);
        manager.exec();
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openCoursesManager() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        // Open the manager
        CoursesManager manager(&db, this);
        manager.exec();
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openGroupsSwappingsManager() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        // Open the manager
        GroupsSwappingsManager manager(&db, this);
        manager.exec();
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openTimeslotsManager() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        // Open the manager
        QDate monday;
        IntroTimeslots intro(&monday, this);
        if(intro.exec() == IntroTimeslots::Accepted) {
            //Open TimeslotsManager with this date and DB connection
            TimeslotsManager manager(&db, monday, this);
            manager.exec();
        }
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openEventsManager() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        // Open the manager
        EventsManager manager(&db, this);
        manager.exec();
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openInterface() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        // Open the intro of the interface
        int id_week = -1;
        QDate monday;
        IntroInterface intro(&id_week, &monday, this);
        if(intro.exec() == IntroInterface::Accepted) {
            // If intro valided, open the interface
            InterfaceDialog manager(&db, id_week, monday, this);
            manager.exec();
        }
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openInterfaceWithDate(QDate date, int id_week) {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        InterfaceDialog manager(&db, id_week, date, this);
        manager.exec();
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openKholloscope() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        // Open the manager
        KholloscopeWizard manager(&db, this);
        manager.exec();
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openReview() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        // Open the manager
        ReviewDialog dialog(&db, this);
        dialog.exec();
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openLastChanges() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        int id_week = -1;
        QDate monday;
        IntroInterface intro(&id_week, &monday, this);
        if(intro.exec() == IntroInterface::Accepted) {
            LastChanges manager(&db, id_week, &monday, this);
            manager.exec();
        }
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openAboutIt() {
    AboutItDialog dialog(this);
    dialog.exec();
}

void MainWindow::openHelp(){
    ContactDialog dialog(this);
    dialog.exec();
}


void MainWindow::createKhollo() {
    record(false);
    //Try to load directory preferences
    QString pref_path;
    QFile read(QCoreApplication::applicationDirPath() + QDir::separator() + "dir_preferences.pref");
    if(read.exists() && read.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&read);
        pref_path = in.readLine();
    }

    if(pref_path == "" || !QDir(pref_path).exists())
        pref_path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    //Get file name
    QString filename = QFileDialog::getSaveFileName(this, "Enregistrer sous...",
                                                    pref_path + QDir::separator() + "kholloscope",  "KSCOPE (*.kscope)");

    if(filename == "") {
        updateWindow();
        return;
    }

    //Save directory in preferences
    QString dirpath = QFileInfo(filename).absoluteDir().absolutePath();
    QFile pref_file(QCoreApplication::applicationDirPath() + QDir::separator() + "dir_preferences.pref");
    if(pref_file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate)){
        QTextStream out(&pref_file);
        out << dirpath;
    }

    if(kscopemanager.createFile(filename))
        QMessageBox::information(NULL, "Succès", "Votre kholloscope a été créé.<br />Vous pouvez dès maintenant l'utiliser. :p");

    updateWindow();
    record(QSqlDatabase::database().isOpen());
    return;
}

void MainWindow::openKhollo() {
    record(false);
    //Try to load directory preferences
    QString pref_path;
    QFile read(QCoreApplication::applicationDirPath() + QDir::separator() + "dir_preferences.pref");
    if(read.exists() && read.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&read);
        pref_path = in.readLine();
    }

    if(pref_path == "" || !QDir(pref_path).exists())
        pref_path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    //Get file name
    QString fileDB = QFileDialog::getOpenFileName(this, "Ouvrir un fichier", pref_path + QDir::separator(), "KSCOPE (*.kscope)");

    if(fileDB == "") {
        updateWindow();
        return;
    }

    //Save directory in preferences
    QString dirpath = QFileInfo(fileDB).absoluteDir().absolutePath();
    QFile pref_file(QCoreApplication::applicationDirPath() + QDir::separator() + "dir_preferences.pref");
    if(pref_file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate)){
        QTextStream out(&pref_file);
        out << dirpath;
    }

    kscopemanager.openFile(fileDB);
    updateWindow();
    record(QSqlDatabase::database().isOpen());
    return;
}

void MainWindow::updateWindow() {
    QSqlDatabase db = QSqlDatabase::database();

    QString info = "";
    if(db.databaseName() != "") {
        info += "<strong> Kholloscope :</strong> " + db.databaseName() + "<br />";
        info += "<strong> Driver :</strong> " + db.driverName() + "<br />";
        if(db.isOpen())
                info += "<strong> Chargé :</strong> VRAI :p<br />";
        else    info += "<strong> Chargé :</strong> FAUX :'(<br />";
    } else {
        info += "<strong> Aucun kholloscope n'a été chargé.</strong><br />Veuillez en créer ou en ouvrir un via le menu \"Fichier\"...";
    }

    ui->info->setText(info);

    // Update the menu
    ui->action_DB_Students->setEnabled(db.isOpen());
    ui->action_DB_Groups->setEnabled(db.isOpen());
    ui->action_DB_Subjects->setEnabled(db.isOpen());
    ui->action_DB_Kholleurs->setEnabled(db.isOpen());
    ui->action_Schedule_Timetable->setEnabled(db.isOpen());
    ui->action_Schedule_Students_Groups->setEnabled(db.isOpen());
    ui->action_Schedule_Swapping_Groups->setEnabled(db.isOpen());
    ui->action_Schedule_Kholles->setEnabled(db.isOpen());
    ui->action_Schedule_Events->setEnabled(db.isOpen());
    ui->action_Kholles_Interface->setEnabled(db.isOpen());
    ui->action_Kholles_Generate->setEnabled(db.isOpen());
    ui->action_Kholles_Historic->setEnabled(db.isOpen());
    ui->action_Kholles_LastChanges->setEnabled(db.isOpen());
}

void MainWindow::record(bool start) {
    if(start) {
        QSqlDatabase db = QSqlDatabase::database();
        QSqlQuery qInilisation(db);
        qInilisation.prepare("INSERT INTO `tau_record`(`date`, `minutes`) VALUES(:date, 0)");
        qInilisation.bindValue(":date", QDate::currentDate().toString("yyyy-MM-dd"));
        qInilisation.exec();
        m_idRecord = qInilisation.lastInsertId().toInt();
        if(m_idRecord >= 0) {
            m_timer = new QTimer(this);
            connect(m_timer, SIGNAL(timeout()), this, SLOT(updateRecord()));
            m_timer->start(60*1000);
        }
    } else if(!start && m_idRecord>=0) {
        m_idRecord = -1;
        disconnect(m_timer, SIGNAL(timeout()), this, SLOT(updateRecord()));
        m_timer->stop();
    }
}

void MainWindow::updateRecord() {
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.prepare("UPDATE tau_record SET minutes = minutes + 1 WHERE id = :id_record");
    query.bindValue(":id_record", m_idRecord);
    query.exec();
}
