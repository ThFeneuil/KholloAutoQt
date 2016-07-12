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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openStudentsManager() {
    // Connection with the DB
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("lataupe");
    db.setUserName("root");
    db.setPassword("");
    bool ok = db.open();

    if(ok) {
        // Open the manager
        StudentsManager manager(&db);
        manager.exec();
    }
}

void MainWindow::openGroupsManager() {
    // Connection with the DB
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("lataupe");
    db.setUserName("root");
    db.setPassword("");
    bool ok = db.open();

    if(ok) {
        // Open the manager
        GroupsManager manager(&db);
        manager.exec();
    }
}

void MainWindow::openSubjectsManager() {
    // Connection with the DB
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("lataupe");
    db.setUserName("root");
    db.setPassword("");
    bool ok = db.open();

    if(ok) {
        // Open the manager
        SubjectsManager manager(&db);
        manager.exec();
    }
}

void MainWindow::openKholleursManager() {
    // Connection with the DB
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("lataupe");
    db.setUserName("root");
    db.setPassword("");
    bool ok = db.open();

    if(ok) {
        // Open the manager
        KholleursManager manager(&db);
        manager.exec();
    }
}
