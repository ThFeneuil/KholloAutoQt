#include "kscopemanager.h"

KScopeManager::KScopeManager() {
    ///In order of dependencies !! A table can only be dependent on tables BEFORE it in the list !!
    tables << "tau_users" << "tau_groups" << "tau_groups_users" << "tau_subjects";
    tables << "tau_kholleurs" << "tau_courses" << "tau_record" << "tau_timeslots";
    tables << "tau_events" << "tau_events_groups" << "tau_kholles" << "tau_merge_kholleurs";
    tables << "tau_general" << "tau_tribes";
}

KScopeManager::~KScopeManager() {

}

bool KScopeManager::createFile(QString path) {
    //Try deleting the file if it exists
    QSqlDatabase::database().close();
    QFile f(path); f.remove();

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    if (!db.open()) {
        QMessageBox::critical(NULL, "Echec", "Impossible d'ouvrir la base de données générée...");
        return false;
    }

    //Enable foreign keys
    db.exec("PRAGMA foreign_keys = 1;");

    db.transaction();
    for(int i=0; i<tables.count(); i++)
        tablesStructures(&db, tables[i], Create, NULL);
    db.commit();

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

    //Enable foreign keys
    db.exec("PRAGMA foreign_keys = 1;");

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
        int alertIrreversibleMsg = QMessageBox::Retry;
        for(int i=0; i<tables.count(); i++) {
            if(tab.contains(tables[i])) {
                int res = tablesStructures(db, tables[i], Check, &alertIrreversibleMsg);
                if(res >= 0)
                    nbRectifications += res;
                else return -1;
            }else {
                tablesStructures(db, tables[i], Create, NULL);
                nbRectifications++;
            }

        }
    }

    //Test integrity of database
    if(db->exec("PRAGMA foreign_key_check;").next()) {
        QMessageBox::critical(NULL, "Erreur", "La base de données est corrompue <br/> Echec de \'foreign_key_check\'");
        return -1;
    }

    QSqlQuery q = db->exec("PRAGMA integrity_check;");
    if(q.next() && q.value(0).toString() != "ok") {
        QMessageBox::critical(NULL, "Erreur", "La base de données est corrompue <br/> Echec de \'integrity_check\'");
        return -1;
    }

    return nbRectifications;
}

int KScopeManager::tablesStructures(QSqlDatabase* db, QString nameTable, ActionType action, int *alertIrreversibleMsg) {
    QSqlQuery qCreate(*db);
    QMap<QString, DataType> columns;
    int nbRectifications = 0;
    if(nameTable == "tau_users") {
        switch(action) {
            case Create:
                qCreate.exec("CREATE TABLE `tau_users` ( "
                                "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                                "`name`	TEXT NOT NULL DEFAULT '', "
                                "`first_name`	TEXT NOT NULL DEFAULT '', "
                                "`email`	TEXT NOT NULL DEFAULT '' "
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
                                "`name`	TEXT NOT NULL DEFAULT '' "
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
                            "`id_groups`	INTEGER NOT NULL DEFAULT 0, "
                            "`id_users`	INTEGER NOT NULL DEFAULT 0, "
                            "FOREIGN KEY(`id_users`) REFERENCES `tau_users`(`id`) ON DELETE CASCADE, "
                            "FOREIGN KEY(`id_groups`) REFERENCES `tau_groups`(`id`) ON DELETE CASCADE "
                        ");");
            break;
            case Check:
                columns.insert("id", Id);
                columns.insert("id_groups", ForeignKey);
                columns.insert("id_users", ForeignKey);
            break;
        }
    } else if(nameTable == "tau_subjects") {
        switch(action) {
            case Create:
            qCreate.exec("CREATE TABLE `tau_subjects` ( "
                            "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                            "`name`	TEXT NOT NULL DEFAULT '', "
                            "`shortName`	TEXT NOT NULL DEFAULT '', "
                            "`color`	TEXT NOT NULL DEFAULT '', "
                            "`weight`	INTEGER NOT NULL DEFAULT 0 "
                        ");");
            break;
            case Check:
                columns.insert("id", Id);
                columns.insert("name", Text);
                columns.insert("shortName", Text);
                columns.insert("weight", Int);
            break;
        }
    } else if(nameTable == "tau_kholleurs") {
        switch(action) {
            case Create:
            qCreate.exec("CREATE TABLE `tau_kholleurs` ( "
                            "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                            "`name`	TEXT NOT NULL DEFAULT '', "
                            "`id_subjects`	INTEGER NOT NULL DEFAULT 0, "
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
                            "`id_subjects`	INTEGER NOT NULL DEFAULT 0, "
                            "`time_start`	TEXT NOT NULL DEFAULT '', "
                            "`time_end`	TEXT NOT NULL DEFAULT '', "
                            "`id_groups`	INTEGER NOT NULL DEFAULT 0, "
                            "`id_day`	INTEGER NOT NULL DEFAULT 0, "
                            "`id_week`	INTEGER NOT NULL DEFAULT 0, "
                            "FOREIGN KEY(`id_groups`) REFERENCES `tau_groups`(`id`) ON DELETE CASCADE, "
                            "FOREIGN KEY(`id_subjects`) REFERENCES `tau_subjects`(`id`) ON DELETE CASCADE "
                        ");");
            break;
            case Check:
                columns.insert("id", Id);
                columns.insert("id_subjects", ForeignKey);
                columns.insert("time_start", Text);
                columns.insert("time_end", Text);
                columns.insert("id_groups", ForeignKey);
                columns.insert("id_day", Int);
                columns.insert("id_week", Int);
            break;
        }
    } else if(nameTable == "tau_timeslots") {
        switch(action) {
            case Create:
            qCreate.exec("CREATE TABLE `tau_timeslots` ( "
                            "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                            "`time`	TEXT NOT NULL DEFAULT '', "
                            "`time_end`	TEXT NOT NULL DEFAULT '', "
                            "`id_kholleurs`	INTEGER NOT NULL DEFAULT 0, "
                            "`date`	TEXT NOT NULL DEFAULT '', "
                            "`time_start`	TEXT NOT NULL DEFAULT '', "
                            "`pupils`	INTEGER NOT NULL DEFAULT 0, "
                            "FOREIGN KEY(`id_kholleurs`) REFERENCES `tau_kholleurs`(`id`) ON DELETE CASCADE "
                        ");");
            break;
            case Check:
                columns.insert("id", Id);
                columns.insert("time", Text);
                columns.insert("time_end", Text);
                columns.insert("id_kholleurs", ForeignKey);
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
                            "`name`	TEXT NOT NULL DEFAULT '', "
                            "`comment`	TEXT NOT NULL DEFAULT '', "
                            "`start`	TEXT NOT NULL DEFAULT '', "
                            "`end`	TEXT NOT NULL DEFAULT '' "
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
                            "`id_events`	INTEGER NOT NULL DEFAULT 0, "
                            "`id_groups`	INTEGER NOT NULL DEFAULT 0, "
                            "FOREIGN KEY(`id_events`) REFERENCES `tau_events`(`id`) ON DELETE CASCADE, "
                            "FOREIGN KEY(`id_groups`) REFERENCES `tau_groups`(`id`) ON DELETE CASCADE "
                        ");");
            break;
            case Check:
                columns.insert("id", Id);
                columns.insert("id_events", ForeignKey);
                columns.insert("id_groups", ForeignKey);
            break;
        }
    } else if(nameTable == "tau_kholles") {
        switch(action) {
            case Create:
            qCreate.exec("CREATE TABLE `tau_kholles` ( "
                          "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                          "`id_users`	INTEGER NOT NULL DEFAULT 0, "
                          "`id_timeslots`	INTEGER NOT NULL DEFAULT 0, "
                          "FOREIGN KEY(`id_users`) REFERENCES `tau_users`(`id`) ON DELETE CASCADE, "
                          "FOREIGN KEY(`id_timeslots`) REFERENCES `tau_timeslots`(`id`) ON DELETE CASCADE "
                      ");");
            break;
            case Check:
                columns.insert("id", Id);
                columns.insert("id_users", ForeignKey);
                columns.insert("id_timeslots", ForeignKey);
            break;
        }
    } else if(nameTable == "tau_record") {
        switch(action) {
            case Create:
            qCreate.exec("CREATE TABLE `tau_record` ( "
                          "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                          "`date`	TEXT NOT NULL DEFAULT '', "
                          "`minutes`	INTEGER NOT NULL DEFAULT 0, "
                          "`version` TEXT NOT NULL DEFAULT '' "
                      ");");
            break;
            case Check:
                columns.insert("id", Id);
                columns.insert("date", Text);
                columns.insert("minutes", Int);
                columns.insert("version", Text);
            break;
        }
    } else if(nameTable == "tau_merge_kholleurs") {
        switch(action) {
            case Create:
            qCreate.exec("CREATE TABLE `tau_merge_kholleurs` ( "
                          "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                          "`name`	TEXT NOT NULL DEFAULT '', "
                          "`id_kholleurs`	INTEGER NOT NULL DEFAULT 0, "
                          "FOREIGN KEY(`id_kholleurs`) REFERENCES `tau_kholleurs`(`id`) ON DELETE CASCADE "
                      ");");
            break;
            case Check:
                columns.insert("id", Id);
                columns.insert("name", Text);
                columns.insert("id_kholleurs", ForeignKey);
            break;
        }
    } else if(nameTable == "tau_general") {
        switch(action) {
            case Create:
            qCreate.exec("CREATE TABLE `tau_general` ( "
                          "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                          "`key`	TEXT NOT NULL DEFAULT '', "
                          "`value`	TEXT NOT NULL DEFAULT '' "
                      ");");
            break;
            case Check:
                columns.insert("id", Id);
                columns.insert("key", Text);
                columns.insert("value", Text);
            break;
        }
    } else if(nameTable == "tau_tribes") {
        switch(action) {
            case Create:
            qCreate.exec("CREATE TABLE `tau_tribes` ( "
                          "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                          "`id_users`	INTEGER NOT NULL DEFAULT 0, "
                          "`id_subjects`	INTEGER NOT NULL DEFAULT 0, "
                          "`name_tribe`    TEXT NOT NULL DEFAULT '', "
                          "FOREIGN KEY(`id_users`) REFERENCES `tau_users`(`id`) ON DELETE CASCADE, "
                          "FOREIGN KEY(`id_subjects`) REFERENCES `tau_subjects`(`id`) ON DELETE CASCADE "
                      ");");
            break;
            case Check:
                columns.insert("id", Id);
                columns.insert("id_users", ForeignKey);
                columns.insert("id_subjects", ForeignKey);
                columns.insert("name_tribe", Text);
            break;
        }
    } else {
        QMessageBox::critical(NULL, "Erreur", "Table SQL inexistante : " + nameTable);
        return -1;
    }

    if(action == Check && columns.count()>0) {
        QSqlRecord rec = db->record(nameTable);
        QSqlQuery qCheck(*db);
        QMap<QString, DataType> missing_cols(columns);
        for(int i=0; i<rec.count(); i++)
            missing_cols.remove(rec.fieldName(i));
        QMapIterator<QString, DataType> i(missing_cols);
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
                case ForeignKey:
                    QMessageBox::critical(NULL, "Fichier corrompu", "Clé étrangère " + i.key() + " de la table " + nameTable + " est manquante <br /> Le problème ne peut être résolu.");
                    return -1;
                    break;
            }
        }

        /// TEST IF FOREIGN KEYS CORRECTLY REGISTERED !!
        bool copy = false;
        QMap<QString, DataType> not_fk(columns);

        qCheck.exec("PRAGMA foreign_key_list(`" + nameTable + "`);");
        while(qCheck.next())
            not_fk.remove(qCheck.value("from").toString());

        foreach(DataType type, not_fk)
            if(type == ForeignKey) {
                copy = true;
                break;
            }

        //If foreign keys not correct => need to copy the table
        if(copy) {
            int res = alertIrreversibleMsg ? (*alertIrreversibleMsg) : QMessageBox::Yes;
            if(res == QMessageBox::Retry) {
                res = QMessageBox::warning(NULL, "Vieux fichier", "Le fichier que vous tentez d'ouvrir a été généré par une ancienne version du logiciel, et doit donc subir une opération irréversible pour fonctionner correctement sur cette version. "
                                 "Cette opération vous empêchera de retravailler avec ce fichier sur une version plus ancienne. Voulez-vous effectuer cette opération ?", QMessageBox::Yes | QMessageBox::No);
                if(alertIrreversibleMsg)
                    *alertIrreversibleMsg = res;

                if(res != QMessageBox::Yes)
                    QMessageBox::critical(NULL, "Vieux fichier", "Vous avez choisi d'ignorer cette opération. Le logiciel pourra rencontrer des erreurs lors de son exécution avec ce fichier...");
            }

            if(res == QMessageBox::Yes) {
                QSqlQuery qComp(*db);
                qComp.exec("ALTER TABLE `" + nameTable + "` RENAME TO `" + nameTable + "_compatibility`;");
                tablesStructures(db, nameTable, Create, NULL);

                QString structure = "";
                QList<QString> col_names = columns.keys();
                for(int i = 0; i < col_names.length(); i++) {
                    if(structure != "")
                        structure += ", ";
                    structure += col_names[i];
                }

                qComp.exec("INSERT INTO `" + nameTable + "`(" + structure + ") "
                                                         "SELECT " + structure + " FROM `" + nameTable + "_compatibility`");
                qComp.exec("DROP TABLE `" + nameTable + "_compatibility`");
                nbRectifications++;
            }
        }

        /// COMPATIBILITY WITH OLD KSCOPE FILE (version <= v1.1 ) : DATA TEACHER !!
        /*if(nameTable == "tau_courses") {
            QSqlQuery qCompatibility(*db);
            /// Test if the software can add a course
            qCompatibility.exec("INSERT INTO tau_courses(id_subjects, time_start, time_end, id_groups, id_day, id_week) VALUES(0, 'TEST_COMPATIBILITY', 'TEST_COMPATIBILITY', 0, 0, 0)");

            if(qCompatibility.lastError().isValid()) {
                /// If no, remove the column teacher
                int res = QMessageBox::warning(NULL, "Vieux fichier", "Le fichier que vous tentez d'ouvrir a été généré par une ancienne version du logiciel, et doit donc subir une opération irréversible pour fonctionner correctement sur cette version. "
                                     "Cette opération vous empêchera de retravailler avec ce fichier sur une version plus ancienne. Voulez-vous effectuer cette opération ?", QMessageBox::Yes | QMessageBox::No);
                if(res == QMessageBox::Yes) {
                    qCompatibility.exec("ALTER TABLE tau_courses RENAME TO tau_courses_compatibility;");
                    qCompatibility.exec("CREATE TABLE `tau_courses` ( "
                                        "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                                        "`id_subjects`	INTEGER NOT NULL DEFAULT 0, "
                                        "`time_start`	TEXT NOT NULL DEFAULT '', "
                                        "`time_end`	TEXT NOT NULL DEFAULT '', "
                                        "`id_groups`	INTEGER NOT NULL DEFAULT 0, "
                                        "`id_day`	INTEGER NOT NULL DEFAULT 0, "
                                        "`id_week`	INTEGER NOT NULL DEFAULT 0 "
                                    ");");
                    qCompatibility.exec("SELECT id, id_subjects, time_start, time_end, id_groups, id_day, id_week FROM tau_courses_compatibility");
                    QString qInsert_string = "";
                    while(qCompatibility.next()) {
                        // Build the objec "Course"
                        int course_id = qCompatibility.value(0).toInt();
                        int course_id_subjects = qCompatibility.value(1).toInt();
                        QString course_time_start = qCompatibility.value(2).toString();
                        QString course_time_end = qCompatibility.value(3).toString();
                        int course_id_groups = qCompatibility.value(4).toInt();
                        int course_id_day = qCompatibility.value(5).toInt();
                        int course_id_week = qCompatibility.value(6).toInt();

                        qInsert_string += (qInsert_string != "") ? ", " : "";
                        qInsert_string += "("+QString::number(course_id)+", "+QString::number(course_id_subjects)+", '"+course_time_start+"', '"+course_time_end+"', "+QString::number(course_id_groups)+", "+QString::number(course_id_day)+", "+QString::number(course_id_week)+")";
                    }
                    qInsert_string = "INSERT INTO tau_courses(id, id_subjects, time_start, time_end, id_groups, id_day, id_week) VALUES " + qInsert_string + ";";
                    qCompatibility.exec(qInsert_string);
                    qCompatibility.exec("DROP TABLE tau_courses_compatibility");
                    nbRectifications++;
                } else
                    QMessageBox::critical(NULL, "Vieux fichier", "Vous avez choisi d'ignorer cette opération. Le logiciel pourra rencontrer des erreurs lors de son exécution avec ce fichier...");
            } else
                qCompatibility.exec("DELETE FROM tau_courses WHERE id_subjects=0 AND time_start='TEST_COMPATIBILITY' AND time_end='TEST_COMPATIBILITY' AND id_groups=0 AND id_day=0 AND id_week=0");
        }*/
    }

    return nbRectifications;
}

