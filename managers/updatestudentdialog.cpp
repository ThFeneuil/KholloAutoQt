#include "managers/updatestudentdialog.h"
#include "ui_updatestudentdialog.h"

UpdateStudentDialog::UpdateStudentDialog(QSqlDatabase *db, Student *stud, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateStudentDialog)
{
    ui->setupUi(this);
    m_db = db;
    m_student = stud;

    connect(ui->pushButton_update, SIGNAL(clicked()), this, SLOT(update_student()));

    ui->lineEdit_name->setText(m_student->getName());
    ui->lineEdit_firstName->setText(m_student->getFirst_name());
    ui->lineEdit_email->setText(m_student->getEmail());
}

UpdateStudentDialog::~UpdateStudentDialog()
{
    delete ui;
}

bool UpdateStudentDialog::update_student() {
    QString name = ui->lineEdit_name->text();
    QString firstName = ui->lineEdit_firstName->text();
    QString email = ui->lineEdit_email->text();

    if(name == "" || firstName == "") {
        QMessageBox::critical(this, "Erreur", "Il faut renseigner le nom et le prénom.");
        return false;
    } else {
        QSqlQuery query(*m_db);
        query.prepare("UPDATE tau_users SET name=:name, first_name=:fname, email=:email WHERE id=:id");
        query.bindValue(":name", name);
        query.bindValue(":fname", firstName);
        query.bindValue(":email", email);
        query.bindValue(":id", m_student->getId());
        query.exec();

        accept();
    }

    return true;
}
