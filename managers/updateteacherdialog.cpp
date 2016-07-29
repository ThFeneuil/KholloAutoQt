#include "managers/updateteacherdialog.h"
#include "ui_updateteacherdialog.h"

UpdateTeacherDialog::UpdateTeacherDialog(QSqlDatabase *db, Teacher *tcher, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateTeacherDialog)
{
    // Initialisation
    ui->setupUi(this);
    m_db = db;
    m_teacher = tcher;

    // Set the input field with the data of the kholleur
    ui->lineEdit_name->setText(m_teacher->getName());

    // Connect the button to update
    connect(ui->pushButton_update, SIGNAL(clicked()), this, SLOT(update_teacher()));

    // Make the request for the subjects
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, shortName, color FROM tau_subjects ORDER BY shortName");

    // Treat the request & Display the subjects in the comboBox
    ui->comboBox_subjects->addItem("", (qulonglong) 0);
    while (query.next()) {
        Subject* subj = new Subject();
        subj->setId(query.value(0).toInt());
        subj->setName(query.value(1).toString());
        subj->setShortName(query.value(2).toString());
        subj->setColor(query.value(3).toString());

        ui->comboBox_subjects->addItem(subj->getShortName(), (qulonglong) subj);
        queue_subjects.enqueue(subj);

        // To select the correct subject
        if(m_teacher->getId_subjects() == subj->getId()) {
            int index = ui->comboBox_subjects->findData((qulonglong) subj);
            if(index != -1)
               ui->comboBox_subjects->setCurrentIndex(index);
        }

    }
}

UpdateTeacherDialog::~UpdateTeacherDialog() {
    delete ui;
    // Free the subjects of the comboBox
    while (!queue_subjects.isEmpty())
        delete queue_subjects.dequeue();
}

bool UpdateTeacherDialog::update_teacher() {
    // Get the parameters given par the user
    QString name = ui->lineEdit_name->text();
    Subject* subj = (Subject*) ui->comboBox_subjects->currentData().toLongLong();

    if(name == "") { //If there is no name...
        QMessageBox::critical(this, "Erreur", "Il faut renseigner le nom de l'enseignant.");
        return false;
    } else { // Else update the kholleur
        QSqlQuery query(*m_db);
        query.prepare("UPDATE tau_teachers SET name=:name, id_subjects=:id_subjects WHERE id=:id");
        query.bindValue(":name", name);
        query.bindValue(":id_subjects", subj ? subj->getId() : 0);
        query.bindValue(":id", m_teacher->getId());
        query.exec();

        accept();
    }

    return true;
}
