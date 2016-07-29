#include "subjectspage.h"
#include "ui_subjectspage.h"

SubjectsPage::SubjectsPage(QSqlDatabase *db, QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::SubjectsPage)
{
    //UI
    ui->setupUi(this);

    //DB
    m_db = db;
}

SubjectsPage::~SubjectsPage()
{
    delete ui;
}

void SubjectsPage::initializePage() {
    //Finish setup
    QList<Subject*>* m_subjects = ((KholloscopeWizard*) wizard())->get_subjects();

    int i;
    for(i = 0; i < m_subjects->length(); i++) {
        QCheckBox *box = new QCheckBox(m_subjects->at(i)->getName());
        ui->subjects_layout->addWidget(box);
        registerField("subject_"+QString::number(m_subjects->at(i)->getId()), box);
    }
}
