#include "mergekholleursmanager.h"
#include "ui_mergekholleursmanager.h"

MergeKholleursManager::MergeKholleursManager(QSqlDatabase* db, QList<Kholleur*>* anonymousKholleurs, QMap<QString, int>* idKholleurs, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MergeKholleursManager)
{
    ui->setupUi(this);
    m_db = db;
    m_anonymousKholleurs = anonymousKholleurs;
    m_idKholleurs = idKholleurs;
    m_listKholleurs = new QList<Kholleur*>;

    // Get the kholleur
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, id_subjects, duration, preparation, pupils FROM tau_kholleurs ORDER BY UPPER(name)");
    while(query.next()) {
        Kholleur* k = new Kholleur();
        k->setId(query.value(0).toInt());
        k->setName(query.value(1).toString());
        k->setId_subjects(query.value(2).toInt());
        k->setDuration(query.value(3).toInt());
        k->setPreparation(query.value(4).toInt());
        k->setPupils(query.value(5).toInt());
        m_listKholleurs->append(k);
    }

    m_listComboBox = new QList<QComboBox*>;
    for(int i=0; i<m_anonymousKholleurs->length(); i++) {
        QHBoxLayout* row = new QHBoxLayout;
        row->addWidget(new QLabel("<font style=\"color:red;\">"+m_anonymousKholleurs->at(i)->getName()+"</font>"));
        QComboBox* box = new QComboBox;
        box->addItem("Ajouter ce nouveau kholleur", (qulonglong) 0);
        for(int j=0; j<m_listKholleurs->length(); j++) {
            box->addItem(m_listKholleurs->at(j)->getName(), (qulonglong) m_listKholleurs->at(j));
            box->setItemData(j+1, QColor(0,128,0), Qt::TextColorRole); // Green
        }
        m_listComboBox->append(box);
        row->addWidget(m_listComboBox->at(i));
        ui->centralLayout->insertLayout(i+2, row); // +2 : +1 for the spacer, +1 for the label
    }

    connect(ui->button_valid, SIGNAL(clicked(bool)), this, SLOT(save()));
}

MergeKholleursManager::~MergeKholleursManager()
{
    delete ui;
    for(int i=0; i<m_listKholleurs->length(); i++)
        delete m_listKholleurs->at(i);
    delete m_listKholleurs;
    delete m_listComboBox;
}

void MergeKholleursManager::save() {
    QSqlQuery query(*m_db);
    for(int i=0; i<m_anonymousKholleurs->length(); i++) {
        QString name = m_anonymousKholleurs->at(i)->getName();
        if(m_listComboBox->at(i)->currentData().toULongLong()) {
            int id = ((Kholleur*) m_listComboBox->at(i)->currentData().toULongLong())->getId();
            query.prepare("INSERT INTO tau_merge_kholleurs(name, id_kholleurs) VALUES(:name, :id_kholleurs)");
            query.bindValue(":name", name);
            query.bindValue(":id_kholleurs", id);
            query.exec();
            m_idKholleurs->insert(name, id);
        } else {
            Kholleur* khll = m_anonymousKholleurs->at(i);
            query.prepare("INSERT INTO tau_kholleurs(name, id_subjects, duration, preparation, pupils) VALUES(:name, 0, 0, 0, 0)");
            query.bindValue(":name", name);
            query.exec();
            khll->setId(query.lastInsertId().toInt());
            UpdateKholleurDialog dialog(m_db, khll, false, this);
            if(dialog.exec() == QDialog::Rejected) {
                query.prepare("DELETE FROM tau_kholleurs WHERE id=:id");
                query.bindValue(":id", khll->getId());
                query.exec();
                done(QDialog::Rejected);
            }
            query.prepare("INSERT INTO tau_merge_kholleurs(name, id_kholleurs) VALUES(:name, :id_kholleurs)");
            query.bindValue(":name", name);
            query.bindValue(":id_kholleurs", khll->getId());
            query.exec();
            m_idKholleurs->insert(name, khll->getId());
        }
    }
    done(QDialog::Accepted);
}
