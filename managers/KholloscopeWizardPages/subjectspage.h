#ifndef SUBJECTSPAGE_H
#define SUBJECTSPAGE_H

#include <QWizardPage>
#include <QtSql>
#include <QMessageBox>
#include <QQueue>
#include <QMap>
#include <QListWidget>
#include <QListWidgetItem>
#include <QCheckBox>
#include "storedData/subject.h"
#include "managers/kholloscopewizard.h"

namespace Ui {
class SubjectsPage;
}

class SubjectsPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit SubjectsPage(QSqlDatabase *db, QWidget *parent = 0);
    ~SubjectsPage();
    void initializePage();

private:
    Ui::SubjectsPage *ui;
    QSqlDatabase *m_db;
};

#endif // SUBJECTSPAGE_H
