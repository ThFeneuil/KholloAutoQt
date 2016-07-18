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
    connect(ui->action_Schedule_Events, SIGNAL(triggered()), this, SLOT(openEventsManager()));

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
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échouée");
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
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échouée");
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
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échouée");
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
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échouée");
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
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échouée");
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
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échouée");
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
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échouée");
    }
}
