#include "updatesubjectdialog.h"
#include "ui_updatesubjectdialog.h"

UpdateSubjectDialog::UpdateSubjectDialog(QSqlDatabase *db, Subject *subj, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateSubjectDialog)
{
    ui->setupUi(this);
    m_db = db;
    m_subject = subj;

    connect(ui->pushButton_update, SIGNAL(clicked()), this, SLOT(update_subject()));

    ui->lineEdit_long->setText(m_subject->getName());
    ui->lineEdit_short->setText(m_subject->getShortName());
    ui->spinBox_weight->setMinimum(0);
    ui->spinBox_weight->setMaximum(MaxWeightSubject);
    ui->spinBox_weight->setValue(m_subject->getWeight());
}

UpdateSubjectDialog::~UpdateSubjectDialog()
{
    delete ui;
}

bool UpdateSubjectDialog::update_subject() {
    QString name = ui->lineEdit_long->text();
    QString shortName = ui->lineEdit_short->text();
    int weight = ui->spinBox_weight->value();
    weight = (weight < 0) ? 0 : weight;
    weight = (weight > MaxWeightSubject) ? MaxWeightSubject : weight;

    if(name == "" || shortName == "") {
        QMessageBox::critical(this, "Erreur", "Il faut renseigner un nom long et un nom court pour la matière.");
        return false;
    } else {
        QSqlQuery query(*m_db);
        query.prepare("UPDATE tau_subjects SET name=:long, shortName=:short, weight=:weight WHERE id=:id");
        query.bindValue(":long", name);
        query.bindValue(":short", shortName);
        query.bindValue(":weight", weight);
        query.bindValue(":id", m_subject->getId());
        query.exec();

        accept();
    }

    return true;
}
