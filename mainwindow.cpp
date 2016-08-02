#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->action_File_Quit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->action_DB_Students, SIGNAL(triggered()), this, SLOT(openStudentsManager()));
    connect(ui->action_DB_Groups, SIGNAL(triggered()), this, SLOT(openGroupsManager()));
    connect(ui->action_DB_Subjects, SIGNAL(triggered()), this, SLOT(openSubjectsManager()));
    connect(ui->action_DB_Kholleurs, SIGNAL(triggered()), this, SLOT(openKholleursManager()));
    connect(ui->action_Schedule_Students_Groups, SIGNAL(triggered()), this, SLOT(openUsersGroupsManager()));
    connect(ui->action_Schedule_Timetable, SIGNAL(triggered()), this, SLOT(openCoursesManager()));
    connect(ui->action_Schedule_Kholles, SIGNAL(triggered()), this, SLOT(openTimeslotsManager()));
    connect(ui->action_Schedule_Events, SIGNAL(triggered()), this, SLOT(openEventsManager()));
    connect(ui->action_Kholles_Interface, SIGNAL(triggered()), this, SLOT(openInterface()));
    connect(ui->action_Kholles_Generate, SIGNAL(triggered()), this, SLOT(openKholloscope()));
    connect(ui->action_Kholles_Historic, SIGNAL(triggered()), this, SLOT(openReview()));
    connect(ui->action_Help, SIGNAL(triggered()), this, SLOT(openHelp()));
    connect(ui->action_AboutIt, SIGNAL(triggered()), this, SLOT(openAboutIt()));

    connect(ui->action_File_Create, SIGNAL(triggered()), this, SLOT(createKhollo()));
    connect(ui->action_File_Select, SIGNAL(triggered()), this, SLOT(loadDB()));

    // Connection with the DB
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("lataupe");
    db.setUserName("root");
    db.setPassword("");
    db.open();
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
        StudentsManager manager(&db);
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
        GroupsManager manager(&db);
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
        SubjectsManager manager(&db);
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
        KholleursManager manager(&db);
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
        usersgroupsmanager manager(&db);
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
        CoursesManager manager(&db);
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
        IntroTimeslots manager(&db);
        manager.exec();
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
        EventsManager manager(&db);
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

void MainWindow::openKholloscope() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        // Open the manager
        KholloscopeWizard manager(&db);
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
        ReviewDialog dialog(&db);
        dialog.exec();
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openAboutIt() {
    AboutItDialog dialog;
    dialog.exec();
}

void MainWindow::openHelp(){
    ContactDialog dialog;
    dialog.exec();
}



void MainWindow::createKhollo() {
    //Try to load directory preferences
    QString pref_path;
    QFile read(QDir::currentPath() + QDir::separator() + "dir_preferences.pref");
    if(read.exists() && read.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&read);
        pref_path = in.readLine();
    }

    if(pref_path == "" || !QDir(pref_path).exists())
        pref_path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    //Get file name
    QString filename = QFileDialog::getSaveFileName(this, "Enregistrer sous...",
                                                    pref_path + QDir::separator() + "kholloscope",  "KSCOPE (*.kscope)");

    if(filename == "")
        return;

    //Save directory in preferences
    QString dirpath = QFileInfo(filename).absoluteDir().absolutePath();
    QFile pref_file(QDir::currentPath() + QDir::separator() + "dir_preferences.pref");
    if(pref_file.open(QIODevice::ReadWrite | QIODevice::Text)){
        QTextStream out(&pref_file);
        out << dirpath;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(filename);
    if (!db.open()) {
        QMessageBox::critical(NULL, "Echec", "Impossible d'ouvrir la base de données générée...");
        return;
    }

    QSqlQuery qCreate(db);
    // TABLE USERS
    qCreate.exec("CREATE TABLE `tau_users` ( "
                    "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                    "`name`	TEXT NOT NULL, "
                    "`first_name`	TEXT NOT NULL, "
                    "`email`	TEXT NOT NULL "
                ");");
    // TABLE GROUPS
    qCreate.exec("CREATE TABLE `tau_groups` ( "
                    "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                    "`name`	TEXT NOT NULL, "
                    "`is_deleted`	INTEGER NOT NULL DEFAULT 0 "
                ");");
    // TABLE GROUPS-USERS
    qCreate.exec("CREATE TABLE `tau_groups_users` ( "
                    "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                    "`id_groups`	INTEGER NOT NULL, "
                    "`id_users`	INTEGER NOT NULL "
                ");");
    // TABLE SUBJECTS
    qCreate.exec("CREATE TABLE `tau_subjects` ( "
                    "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                    "`name`	TEXT NOT NULL, "
                    "`shortName`	TEXT NOT NULL, "
                    "`color`	TEXT NOT NULL "
                ");");
    // TABLE TEACHERS
    qCreate.exec("CREATE TABLE `tau_teachers` ( "
                    "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                    "`name`	TEXT NOT NULL, "
                     "`id_subjects`	INTEGER NOT NULL "
                ");");
    // TABLE KHOLLEURS
    qCreate.exec("CREATE TABLE `tau_kholleurs` ( "
                    "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                    "`name`	TEXT NOT NULL, "
                    " `id_subjects`	INTEGER NOT NULL, "
                    "`duration`	INTEGER NOT NULL DEFAULT 0, "
                    "`preparation`	INTEGER NOT NULL DEFAULT 0, "
                    "`pupils`	INTEGER NOT NULL DEFAULT 0 "
                ");");
    // TABLE COURSES
    qCreate.exec("CREATE TABLE `tau_courses` ( "
                    "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                    "`id_subjects`	INTEGER NOT NULL, "
                    "`time_start`	BLOB NOT NULL, "
                    "`time_end`	TEXT NOT NULL, "
                    "`id_groups`	INTEGER NOT NULL, "
                    "`id_teachers`	INTEGER NOT NULL, "
                    "`id_day`	INTEGER NOT NULL, "
                    "`id_week`	INTEGER NOT NULL "
                ");");
    // TABLE TIMESLOTS
    qCreate.exec("CREATE TABLE `tau_timeslots` ( "
                    "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                    "`time`	TEXT NOT NULL, "
                    "`time_end`	TEXT NOT NULL, "
                    "`id_kholleurs`	INTEGER NOT NULL, "
                    "`date`	TEXT NOT NULL, "
                    "`time_start`	TEXT NOT NULL, "
                    "`pupils`	INTEGER NOT NULL "
                ");");
    // TABLE EVENTS
    qCreate.exec("CREATE TABLE `tau_events` ( "
                    "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                    "`name`	TEXT NOT NULL, "
                    "`comment`	TEXT NOT NULL, "
                    "`start`	TEXT NOT NULL, "
                    "`end`	TEXT NOT NULL "
                ");");
    // TABLE EVENTS-GROUPS
    qCreate.exec("CREATE TABLE `tau_events_groups` ( "
                    "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                    "`id_events`	INTEGER NOT NULL, "
                    "`id_groups`	INTEGER NOT NULL "
                ");");
    // TABLE KHOLLES
    qCreate.exec("CREATE TABLE `tau_kholles` ( "
                    "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                    "`id_users`	INTEGER NOT NULL, "
                    "`id_timeslots`	INTEGER NOT NULL "
                ");");
    QMessageBox::information(NULL, "Succès", "Votre kholloscope a été créé.<br />Vous pouvons dès maintenant l'utiliser. :p");
}

void MainWindow::loadDB() {
    //Try to load directory preferences
    QString pref_path;
    QFile read(QDir::currentPath() + QDir::separator() + "dir_preferences.pref");
    if(read.exists() && read.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&read);
        pref_path = in.readLine();
    }

    if(pref_path == "" || !QDir(pref_path).exists())
        pref_path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    //Get file name
    QString fileBackup = QFileDialog::getOpenFileName(this, "Ouvrir un fichier", pref_path + QDir::separator(), "KSCOPE (*.kscope)");

    if(fileBackup == "")
        return;

    //Save directory in preferences
    QString dirpath = QFileInfo(fileBackup).absoluteDir().absolutePath();
    QFile pref_file(QDir::currentPath() + QDir::separator() + "dir_preferences.pref");
    if(pref_file.open(QIODevice::ReadWrite | QIODevice::Text)){
        QTextStream out(&pref_file);
        out << dirpath;
    }

    QFile file(fileBackup);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    if (!db.open())
        return;

    QSqlQuery query(db);

    QTextStream in(&fileBackup);
    while (!in.atEnd()) {
        QString line = in.readLine();
        query.exec(line);
    }

    return;
}

QString MainWindow::addSlashes(QString str)  {
    QString newStr = "";

    for(int i=0;i<str.length();i++) {
        if(str[i] == '\0') {
            newStr += "\\\0";

        } else if(str[i] == '\'')
            newStr += "\\'";

        else if(str[i] == '\"')
            newStr += "\\\"";

        else if(str[i] == '\\')
            newStr += "\\\\";

        else
            newStr.append(str[i]);
     }
    return newStr;
}
