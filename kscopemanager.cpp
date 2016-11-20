#include "kscopemanager.h"

KScopeManager::KScopeManager() {
    tables << "tau_users" << "tau_groups" << "tau_groups_users" << "tau_subjects";
    tables << "tau_teachers" << "tau_kholleurs" << "tau_courses" << "tau_record";
    tables << "tau_timeslots" << "tau_events" << "tau_events_groups" << "tau_kholles";
}

KScopeManager::~KScopeManager() {

}

bool KScopeManager::createFile(QString path) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    if (!db.open()) {
        QMessageBox::critical(NULL, "Echec", "Impossible d'ouvrir la base de données générée...");
        return false;
    }

    for(int i=0; i<tables.count(); i++)
        tablesStructures(&db, tables[i], Create);

    return true;
}

bool KScopeManager::openFile(QString path) {
    if(QFileInfo(path).fileName() == "localhost.kscope")
        return openLocalhostDB();

    // Open the QSQLite database
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    if (!db.open()) {
        QMessageBox::critical(NULL, "Echec", "Impossible d'ouvrir la base de données...");
        return false;
    }
    int nbRectifications = checkDBStructure(&db);
    if(nbRectifications < 0)
        QMessageBox::critical(NULL, "Echec", "Votre kholloscope est corrompu. :'( <br />Néanmoins, il a été chargé pour permettre d'identifier le problème.<br /> Par conséquence, il est possible que le logiciel ne supporte pas ce fichier.");
    else if(nbRectifications == 0)
        QMessageBox::information(NULL, "Succès", "Votre kholloscope a été chargé.<br />Vous pouvez l'utiliser. :p");
    else if(nbRectifications == 1)
        QMessageBox::information(NULL, "Succès", "Une anomalie a été détectée et corrigée dans votre fichier.<br />Vous pouvez, dès à présent, utiliser votre kholloscope.");
    else
        QMessageBox::information(NULL, "Succès", QString::number(nbRectifications) + " anomalies ont été détectées et corrigées dans votre fichier.<br />Vous pouvez, dès à présent, utiliser votre kholloscope.");

    return true;
}

bool KScopeManager::openLocalhostDB() {
    // Connection with the DB
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("lataupe");
    db.setUserName("root");
    db.setPassword("");
    if (!db.open()) {
        QMessageBox::critical(NULL, "Echec", "Impossible d'ouvrir la base de données (du serveur local)...");
        return false;
    }
    QMessageBox::information(NULL, "Succès", "La base de données qui est sur le serveur local a été chargée.<br />Vous pouvez l'utiliser. :p");
    return true;
}

int KScopeManager::checkDBStructure(QSqlDatabase* db) {
    int nbRectifications = 0;
    if(db->driverName() == "QSQLITE") {
        QStringList tab = db->tables();
        for(int i=0; i<tables.count(); i++) {
            if(tab.contains(tables[i])) {
                int res = tablesStructures(db, tables[i], Check);
                if(res >= 0)
                    nbRectifications += res;
                else return -1;
            }else {
                tablesStructures(db, tables[i], Create);
                nbRectifications++;
            }

        }
    }
    return nbRectifications;
}

int KScopeManager::tablesStructures(QSqlDatabase* db, QString nameTable, ActionType action) {
    QSqlQuery qCreate(*db);
    QMap<QString, DataType> columns;
    int nbRectifications = 0;
    if(nameTable == "tau_users") {
        switch(action) {
            case Create:
                qCreate.exec("CREATE TABLE `tau_users` ( "
                                "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                                "`name`	TEXT NOT NULL, "
                                "`first_name`	TEXT NOT NULL, "
                                "`email`	TEXT NOT NULL "
                            ");");
                break;
            case Check:
                columns.insert("id", Id);
                columns.insert("name", Text);
                columns.insert("first_name", Text);
                columns.insert("email", Text);
                break;
        }
    } else if(nameTable == "tau_groups") {
        switch(action) {
            case Create:
                qCreate.exec("CREATE TABLE `tau_groups` ( "
                                "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                                "`name`	TEXT NOT NULL "
                            ");");
                break;
            case Check:
                columns.insert("id", Id);
                columns.insert("name", Text);
            break;
        }
    } else if(nameTable == "tau_groups_users") {
        switch(action) {
            case Create:
            qCreate.exec("CREATE TABLE `tau_groups_users` ( "
                            "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                            "`id_groups`	INTEGER NOT NULL, "
                            "`id_users`	INTEGER NOT NULL "
                        ");");
            break;
            case Check:
                columns.insert("id", Id);
                columns.insert("id_groups", Int);
                columns.insert("id_users", Int);
            break;
        }
    } else if(nameTable == "tau_subjects") {
        switch(action) {
            case Create:
            qCreate.exec("CREATE TABLE `tau_subjects` ( "
                            "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                            "`name`	TEXT NOT NULL, "
                            "`shortName`	TEXT NOT NULL, "
                            "`color`	TEXT NOT NULL "
                            "`weight`	INTEGER NOT NULL "
                        ");");
            break;
            case Check:
                columns.insert("id", Id);
                columns.insert("name", Text);
                columns.insert("shortName", Text);
                columns.insert("weight", Int);
            break;
        }
    } else if(nameTable == "tau_teachers") {
        switch(action) {
            case Create:
            qCreate.exec("CREATE TABLE `tau_teachers` ( "
                            "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                            "`name`	TEXT NOT NULL, "
                             "`id_subjects`	INTEGER NOT NULL "
                        ");");
            break;
            case Check:
                columns.insert("id", Id);
                columns.insert("name", Text);
                columns.insert("id_subjects", Int);
            break;
        }
    } else if(nameTable == "tau_kholleurs") {
        switch(action) {
            case Create:
            qCreate.exec("CREATE TABLE `tau_kholleurs` ( "
                            "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                            "`name`	TEXT NOT NULL, "
                            "`id_subjects`	INTEGER NOT NULL, "
                            "`duration`	INTEGER NOT NULL DEFAULT 0, "
                            "`preparation`	INTEGER NOT NULL DEFAULT 0, "
                            "`pupils`	INTEGER NOT NULL DEFAULT 0 "
                        ");");
            break;
            case Check:
                columns.insert("id", Id);
                columns.insert("name", Text);
                columns.insert("id_subjects", Int);
                columns.insert("duration", Int);
                columns.insert("preparation", Int);
                columns.insert("pupils", Int);
            break;
        }
    } else if(nameTable == "tau_courses") {
        switch(action) {
            case Create:
            qCreate.exec("CREATE TABLE `tau_courses` ( "
                            "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                            "`id_subjects`	INTEGER NOT NULL, "
                            "`time_start`	TEXT NOT NULL, "
                            "`time_end`	TEXT NOT NULL, "
                            "`id_groups`	INTEGER NOT NULL, "
                            "`id_teachers`	INTEGER NOT NULL, "
                            "`id_day`	INTEGER NOT NULL, "
                            "`id_week`	INTEGER NOT NULL "
                        ");");
            break;
            case Check:
                columns.insert("id", Id);
                columns.insert("id_subjects", Int);
                columns.insert("time_start", Text);
                columns.insert("time_end", Text);
                columns.insert("id_groups", Int);
                columns.insert("id_teachers", Int);
                columns.insert("id_day", Int);
                columns.insert("id_week", Int);
            break;
        }
    } else if(nameTable == "tau_timeslots") {
        switch(action) {
            case Create:
            qCreate.exec("CREATE TABLE `tau_timeslots` ( "
                            "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                            "`time`	TEXT NOT NULL, "
                            "`time_end`	TEXT NOT NULL, "
                            "`id_kholleurs`	INTEGER NOT NULL, "
                            "`date`	TEXT NOT NULL, "
                            "`time_start`	TEXT NOT NULL, "
                            "`pupils`	INTEGER NOT NULL "
                        ");");
            break;
            case Check:
                columns.insert("id", Id);
                columns.insert("time", Text);
                columns.insert("time_end", Text);
                columns.insert("id_kholleurs", Int);
                columns.insert("date", Text);
                columns.insert("time_start", Text);
                columns.insert("pupils", Int);
            break;
        }
    } else if(nameTable == "tau_events") {
        switch(action) {
            case Create:
            qCreate.exec("CREATE TABLE `tau_events` ( "
                            "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                            "`name`	TEXT NOT NULL, "
                            "`comment`	TEXT NOT NULL, "
                            "`start`	TEXT NOT NULL, "
                            "`end`	TEXT NOT NULL "
                        ");");
            break;
            case Check:
                columns.insert("id", Id);
                columns.insert("name", Text);
                columns.insert("comment", Text);
                columns.insert("start", Text);
                columns.insert("end", Text);
            break;
        }
    } else if(nameTable == "tau_events_groups") {
        switch(action) {
            case Create:
            qCreate.exec("CREATE TABLE `tau_events_groups` ( "
                            "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                            "`id_events`	INTEGER NOT NULL, "
                            "`id_groups`	INTEGER NOT NULL "
                        ");");
            break;
            case Check:
                columns.insert("id", Id);
                columns.insert("id_events", Int);
                columns.insert("id_groups", Int);
            break;
        }
    } else if(nameTable == "tau_kholles") {
        switch(action) {
            case Create:
            qCreate.exec("CREATE TABLE `tau_kholles` ( "
                          "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                          "`id_users`	INTEGER NOT NULL, "
                          "`id_timeslots`	INTEGER NOT NULL "
                      ");");
            break;
            case Check:
                columns.insert("id", Id);
                columns.insert("id_users", Int);
                columns.insert("id_timeslots", Int);
            break;
        }
    } else if(nameTable == "tau_record") {
        switch(action) {
            case Create:
            qCreate.exec("CREATE TABLE `tau_record` ( "
                          "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                          "`date`	TEXT NOT NULL, "
                          "`minutes`	INTEGER NOT NULL "
                      ");");
            break;
            case Check:
                columns.insert("id", Id);
                columns.insert("date", Text);
                columns.insert("minutes", Int);
            break;
        }
    } else {
        QMessageBox::critical(NULL, "Erreur", "Table SQL inexistante : " + nameTable);
        return -1;
    }

    if(action == Check && columns.count()>0) {
        QSqlRecord rec = db->record(nameTable);
        QSqlQuery qCheck(*db);
        for(int i=0; i<rec.count(); i++)
            columns.remove(rec.fieldName(i));
        QMapIterator<QString, DataType> i(columns);
        while(i.hasNext()) {
            i.next();
            switch(i.value()) {
                case Id:
                    QMessageBox::critical(NULL, "Fichier corrompu", "Identifiant de la table " + nameTable + " est manquante <br /> Le problème ne peut être résolu.");
                    return -1;
                    break;
                case Int:
                    qCheck.exec("ALTER TABLE `"+nameTable+"` ADD `"+i.key()+"` INTEGER NOT NULL DEFAULT 0;");
                    nbRectifications++;
                    break;
                case Text:
                    qCheck.exec("ALTER TABLE `"+nameTable+"` ADD `"+i.key()+"` TEXT NOT NULL DEFAULT '';");
                    nbRectifications++;
                    break;
            }
        }
    }

    return nbRectifications;
}

