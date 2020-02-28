#include "copytimeslots.h"
#include "ui_copytimeslots.h"

CopyTimeslots::CopyTimeslots(QSqlDatabase *db, QDate date, bool all, int id_kholleur, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CopyTimeslots)
{
    ui->setupUi(this);

    //DB and boolean and date
    m_db = db;
    m_all = all;
    m_date = date;
    m_id_kholleur = id_kholleur;

    //Set title depending on bool
    if(m_all)
        this->setWindowTitle("Copier pour tous les kholleurs...");
    else
        this->setWindowTitle("Copier pour ce kholleur...");

    //Initialise date
    ui->dateEdit->setDate(m_date);

    connect(ui->OKButton, SIGNAL(clicked()), this, SLOT(copy()));
}

CopyTimeslots::~CopyTimeslots()
{
    delete ui;
}

void CopyTimeslots::copy() {
    //Create a transaction to speed up process
    m_db->transaction();

    //Get selected date
    QDate old_date = ui->dateEdit->date();
    if(old_date.dayOfWeek() != 1) {
        while(old_date.dayOfWeek() != 1)
            old_date = old_date.addDays(-1);
        QMessageBox::information(this, "Information", "La date n'étant pas à un lundi, la date utilisée sera le lundi de la même semaine.");
    }

    bool changed = false;

    //2 possibilities depending on bool
    if(m_all) {
        //Display warning
        int res = QMessageBox::warning(this, "Copie en cours",
                                       "Vous êtes sur le point de copier les horaires de <strong>tous</strong> les kholleurs. "
                                       "Cela <strong>supprimera</strong> les horaires actuellement rentrés ainsi que les <strong>kholles</strong> associées. "
                                       "<br />Voulez-vous continuer ?",
                                       QMessageBox::Yes | QMessageBox::No);

        if(res == QMessageBox::Yes) {
            //Get all timeslots from the other week
            QSqlQuery get_query(*m_db);
            get_query.prepare("SELECT date, time_start, time, time_end, id_kholleurs, pupils FROM tau_timeslots WHERE date>=:monday_date AND date<=:sunday_date");
            get_query.bindValue(":monday_date", old_date.toString("yyyy-MM-dd"));
            get_query.bindValue(":sunday_date", old_date.addDays(6).toString("yyyy-MM-dd"));
            get_query.exec();

            //Delete all timeslots for this week
            QSqlQuery delete_query(*m_db);
            //Done automatically by SQLITE -->
            /*delete_query.prepare("DELETE FROM tau_kholles WHERE id_timeslots IN "
                                    "(SELECT id FROM tau_timeslots WHERE date>=:monday_date AND date<=:sunday_date)");
            delete_query.bindValue(":monday_date", m_date.toString("yyyy-MM-dd"));
            delete_query.bindValue(":sunday_date", m_date.addDays(6).toString("yyyy-MM-dd"));
            delete_query.exec();*/
            delete_query.prepare("DELETE FROM tau_timeslots WHERE date>=:monday_date AND date<=:sunday_date");
            delete_query.bindValue(":monday_date", m_date.toString("yyyy-MM-dd"));
            delete_query.bindValue(":sunday_date", m_date.addDays(6).toString("yyyy-MM-dd"));
            delete_query.exec();

            //And insert them for the new week
            while(get_query.next()) {
                //Check for a change
                QTime time_start = QTime::fromString(get_query.value(1).toString(), "h:mm:ss");
                QTime time = QTime::fromString(get_query.value(2).toString(), "h:mm:ss");
                QTime time_end = QTime::fromString(get_query.value(3).toString(), "h:mm:ss");

                QSqlQuery kholleur(*m_db);
                kholleur.prepare("SELECT preparation, duration, pupils FROM tau_kholleurs WHERE id=:id");
                kholleur.bindValue(":id", get_query.value(4).toInt());
                kholleur.exec();

                if(kholleur.next()) {
                    //QMessageBox::information(this, "OK", QString::number(time_start.secsTo(time)) + "\n" + QString::number(time.secsTo(time_end)) + "\n" + get_query.value(5).toString());
                    //QMessageBox::information(this, "OK", kholleur.value(0).toString() + "\n" + kholleur.value(1).toString() + "\n" + kholleur.value(2).toString());
                    if(time_start.secsTo(time) != kholleur.value(0).toInt()*60
                            || time.secsTo(time_end) != kholleur.value(1).toInt()*60
                            || get_query.value(5).toInt() != kholleur.value(2).toInt())
                        changed = true;
                }
                else {
                    QMessageBox::warning(this, "Attention", "Ce kholleur a déjà été supprimé... Comment est-ce possible ?");
                }


                QSqlQuery insert_query(*m_db);
                insert_query.prepare("INSERT INTO tau_timeslots(date, time_start, time, time_end, id_kholleurs, pupils) VALUES(:date, :time_start, :time, :time_end, :id_kholleurs, :pupils)");
                insert_query.bindValue(":date", m_date.addDays(QDate::fromString(get_query.value(0).toString(), "yyyy-M-d").dayOfWeek() - 1).toString("yyyy-MM-dd"));
                insert_query.bindValue(":time_start", get_query.value(1).toString());
                insert_query.bindValue(":time", get_query.value(2).toString());
                insert_query.bindValue(":time_end", get_query.value(3).toString());
                insert_query.bindValue(":id_kholleurs", get_query.value(4).toInt());
                insert_query.bindValue(":pupils", get_query.value(5).toInt());
                insert_query.exec();
            }
            accept();
        }
    }

    else {
        QSqlQuery warning_query(*m_db);
        warning_query.prepare("SELECT name FROM tau_kholleurs WHERE id=:id");
        warning_query.bindValue(":id", m_id_kholleur);
        warning_query.exec();
        if(warning_query.next()) {
            //Display warning
            int res = QMessageBox::warning(this, "Copie en cours",
                                           "Vous êtes sur le point de copier les horaires du kholleur <strong>" + warning_query.value(0).toString() + "</strong>. "
                                           "Cela <strong>supprimera</strong> les horaires actuellement rentrés ainsi que les <strong>kholles</strong> associées. "
                                           "<br />Voulez-vous continuer ?",
                                           QMessageBox::Yes | QMessageBox::No);

            if(res == QMessageBox::Yes) {
                //Get all timeslots from the other week for this kholleur
                QSqlQuery get_query(*m_db);
                get_query.prepare("SELECT date, time_start, time, time_end, id_kholleurs, pupils FROM tau_timeslots WHERE date>=:monday_date AND date<=:sunday_date AND id_kholleurs=:id_kholleurs");
                get_query.bindValue(":monday_date", old_date.toString("yyyy-MM-dd"));
                get_query.bindValue(":sunday_date", old_date.addDays(6).toString("yyyy-MM-dd"));
                get_query.bindValue(":id_kholleurs", m_id_kholleur);
                get_query.exec();

                //Delete all timeslots for this week for this kholleur
                QSqlQuery delete_query(*m_db);
                //Already done by SQLITE
                /*delete_query.prepare("DELETE FROM tau_kholles WHERE id_timeslots IN "
                                        "(SELECT id FROM tau_timeslots WHERE date>=:monday_date AND date<=:sunday_date AND id_kholleurs=:id_kholleurs)");
                delete_query.bindValue(":monday_date", m_date.toString("yyyy-MM-dd"));
                delete_query.bindValue(":sunday_date", m_date.addDays(6).toString("yyyy-MM-dd"));
                delete_query.bindValue(":id_kholleurs", m_id_kholleur);
                delete_query.exec();*/
                delete_query.prepare("DELETE FROM tau_timeslots WHERE date>=:monday_date AND date<=:sunday_date AND id_kholleurs=:id_kholleurs");
                delete_query.bindValue(":monday_date", m_date.toString("yyyy-MM-dd"));
                delete_query.bindValue(":sunday_date", m_date.addDays(6).toString("yyyy-MM-dd"));
                delete_query.bindValue(":id_kholleurs", m_id_kholleur);
                delete_query.exec();

                //And insert them for the new week
                while(get_query.next()) {
                    //Check for a change
                    QTime time_start = QTime::fromString(get_query.value(1).toString(), "h:mm:ss");
                    QTime time = QTime::fromString(get_query.value(2).toString(), "h:mm:ss");
                    QTime time_end = QTime::fromString(get_query.value(3).toString(), "h:mm:ss");

                    QSqlQuery kholleur(*m_db);
                    kholleur.prepare("SELECT preparation, duration, pupils FROM tau_kholleurs WHERE id=:id");
                    kholleur.bindValue(":id", get_query.value(4).toInt());
                    kholleur.exec();

                    if(kholleur.next()) {
                        if(time_start.secsTo(time) != kholleur.value(0).toInt()*60
                                || time.secsTo(time_end) != kholleur.value(1).toInt()*60
                                || get_query.value(5).toInt() != kholleur.value(2).toInt())
                            changed = true;
                    }
                    else {
                        QMessageBox::warning(this, "Attention", "Ce kholleur a déjà été supprimé... Comment est-ce possible ?");
                    }

                    QSqlQuery insert_query(*m_db);
                    insert_query.prepare("INSERT INTO tau_timeslots(date, time_start, time, time_end, id_kholleurs, pupils) VALUES(:date, :time_start, :time, :time_end, :id_kholleurs, :pupils)");
                    insert_query.bindValue(":date", m_date.addDays(QDate::fromString(get_query.value(0).toString(), "yyyy-M-d").dayOfWeek() - 1).toString("yyyy-MM-dd"));
                    insert_query.bindValue(":time_start", get_query.value(1).toString());
                    insert_query.bindValue(":time", get_query.value(2).toString());
                    insert_query.bindValue(":time_end", get_query.value(3).toString());
                    insert_query.bindValue(":id_kholleurs", get_query.value(4).toInt());
                    insert_query.bindValue(":pupils", get_query.value(5).toInt());
                    insert_query.exec();
                }
                accept();
            }
        }
        else {
            QMessageBox::critical(this, "Erreur", "Ce kholleur n'existe pas. Comment est-ce possible ?");
            m_db->commit();
            return;
        }
    }

    if(changed) {
        QMessageBox::warning(this, "Attention", "Les paramètres de certains kholleurs ont changé.\n"
                                                "Les horaires de kholles sont simplement copiés et ne sont pas automatiquement mis à jour.\n"
                                                "Pour que ces changements se répercutent sur les horaires, il faut les rentrer à nouveau.");
    }

    m_db->commit();
}
