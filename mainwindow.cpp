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
    connect(ui->action_Help, SIGNAL(triggered()), this, SLOT(openHelp()));
    connect(ui->action_AboutIt, SIGNAL(triggered()), this, SLOT(openAboutIt()));

    connect(ui->action_File_Create, SIGNAL(triggered()), this, SLOT(saveDB()));
    connect(ui->action_File_Select, SIGNAL(triggered()), this, SLOT(loadDB()));

    // Connection with the DB
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("mysql:host=tfeneuilunadmin.mysql.db");
    db.setDatabaseName("tfeneuilunadmin");
    db.setUserName("tfeneuilunadmin");
    db.setPassword("G1ERi5Ps");
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

void MainWindow::openAboutIt() {
    AboutItDialog dialog;
    dialog.exec();
}

void MainWindow::openHelp(){
    ContactDialog dialog;
    dialog.exec();
}



void MainWindow::saveDB() {
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
                                                    pref_path + QDir::separator() + "DBsave",  "KSCOPE (*.kscope)");

    if(filename == "")
        return;

    //Save directory in preferences
    QString dirpath = QFileInfo(filename).absoluteDir().absolutePath();
    QFile pref_file(QDir::currentPath() + QDir::separator() + "dir_preferences.pref");
    if(pref_file.open(QIODevice::ReadWrite | QIODevice::Text)){
        QTextStream out(&pref_file);
        out << dirpath;
    }

    QFile fileBackup(filename);
    if (!fileBackup.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&fileBackup);
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);

    // Get the list of tables
    query.exec("SHOW TABLES");
    QStringList tables;
    while(query.next())
        tables << query.value(0).toString();

    for(int i=0; i<tables.count(); i++) { // For each table
        // Get the number of field
        int nbFields = -1;
        QSqlQuery result(db);
        result.exec("SELECT * FROM `" + tables.at(i) + "`");
        QSqlRecord record = result.record();
        if(result.next())
            nbFields = record.count();

        // Request to create the table
        query.exec("SHOW CREATE TABLE `" + tables.at(i) + "`");
        if(query.next())
            out << "\n\n" + query.value(1).toString() + ";\n\n";
        else
            return;


        // Request to insert the data
        int num = 0;
        while(result.next()) {
            if(num) {
                out << ", ";
            } else {
                out << "INSERT INTO `" + tables.at(i) + "` VALUES";
            }
            out << "(";
            for(int j=0; j<nbFields; j++) {
                if(j)   out << ", ";
                out << "'" + addSlashes(result.value(j).toString()) + "'";
            }
            out << ")";
            num++;
        }
        if(num)
            out << ";";

    }
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
