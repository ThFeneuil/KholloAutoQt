#include "studentsimportmanager.h"

StudentsImportManager::StudentsImportManager(QSqlDatabase *db, QWidget *p)
{
    m_db = db;
    parent = p;
}

bool StudentsImportManager::load(QString filename) {
    m_table.clear();
    m_number_distinct.clear();

    if(!readFile(filename)) {
        QMessageBox::critical(parent, "Erreur", "Erreur lors de la lecture du fichier.");
        return false;
    }

    if(!testTable()) {
        QMessageBox::critical(parent, "Erreur", "Fichier corrompu ou non exploitable.");
        return false;
    }

    calculateDistinct();

    int m1 = -1;
    int m2 = -1;

    findMaxes(&m1, &m2);
    qDebug() << m1 << " " << m2;

    if(m1 == -1 || m2 == -1) {
        QMessageBox::critical(parent, "Erreur", "Fichier corrompu ou non exploitable.");
        return false;
    }

    if(m1 > m2)
        insertStudents(m2, m1);
    else
        insertStudents(m1, m2);

    return true;
}

bool StudentsImportManager::readFile(QString filename) {
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&file);
    while(!in.atEnd()) {
        QString line = in.readLine();
        treatLine(line);
    }
    return true;
}

void StudentsImportManager::treatLine(QString line) {
    QStringList list = line.split(';');

    //Ignore if less than 2 fields not empty
    int count = 0;
    for(int i = 0; i < list.length(); i++) {
        if(!list[i].isEmpty())
            count++;
    }

    if(count <= 2)
        return;

    m_table.append(list);
}

bool StudentsImportManager::testTable() {
    if(m_table.length() == 0)
        return false;

    int num_cols = m_table[0].count();
    for(int i = 0; i < m_table.length(); i++) {
        if(m_table[i].count() != num_cols)
            return false;
    }

    return true;
}

void StudentsImportManager::calculateDistinct() {
    /** ATTENTION : assumes file is in order and m_table not empty **/
    m_number_distinct.clear();

    QStringList one = m_table[0];
    for(int i = 0; i < one.length(); i++) {
        int distinct = treatColumn(i);
        m_number_distinct.append(distinct);
    }
}

int StudentsImportManager::treatColumn(int index) {
    QMap<QString, int> map;

    for(int i = 0; i < m_table.length(); i++) {
        QString s = m_table[i].at(index);
        if(!map.contains(s))
            map[s] = 0;
        map[s] += 1;
    }

    return map.count();
}

void StudentsImportManager::findMaxes(int *max1, int *max2) {
    *max1 = -1;
    *max2 = -1;

    int v1 = -1;
    int v2 = -1;

    for(int i = 0; i < m_number_distinct.length(); i++) {
        int distinct = m_number_distinct[i];

        if(distinct > v1) {
            v2 = v1;
            *max2 = *max1;
            v1 = distinct;
            *max1 = i;
        }
        else {
            if(distinct > v2) {
                v2 = distinct;
                *max2 = i;
            }
        }
    }
}

void StudentsImportManager::insertStudents(int indexName, int indexFirstName) {
    for(int i = 0; i < m_table.length(); i++) {
        QString name = m_table[i].at(indexName);
        QString first_name = m_table[i].at(indexFirstName);

        QSqlQuery query(*m_db);
        query.prepare("INSERT INTO tau_users(name, first_name, email) VALUES(:name, :first_name, :email)");
        query.bindValue(":name", name);
        query.bindValue(":first_name", first_name);
        query.bindValue(":email", "");
        query.exec();
    }
}
