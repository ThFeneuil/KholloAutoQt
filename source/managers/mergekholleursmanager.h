#ifndef MERGEKHOLLEURSMANAGER_H
#define MERGEKHOLLEURSMANAGER_H

#include <QDialog>
#include <QtSql>
#include <QLabel>
#include <QStandardItem>
#include "managers/updatekholleurdialog.h"
#include "storedData/kholleur.h"

namespace Ui {
class MergeKholleursManager;
}

class MergeKholleursManager : public QDialog
{
    Q_OBJECT

public:
    explicit MergeKholleursManager(QSqlDatabase* db, QList<Kholleur*>* anonymousKholleurs, QMap<QString, int>* idKholleurs, QWidget *parent = 0);
    ~MergeKholleursManager();

public slots:
    void save();

private:
    Ui::MergeKholleursManager *ui;
    QSqlDatabase* m_db;
    QList<Kholleur*>* m_anonymousKholleurs;
    QMap<QString, int>* m_idKholleurs;
    QList<Kholleur*>* m_listKholleurs;
    QList<QComboBox*>* m_listComboBox;
};

#endif // MERGEKHOLLEURSMANAGER_H
