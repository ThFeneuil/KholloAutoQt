#include "userspage.h"
#include "ui_userspage.h"

UsersPage::UsersPage(QSqlDatabase *db, QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::UsersPage)
{
    ui->setupUi(this);
    ui->dateEdit->setDate(QDate::currentDate());

    //DB
    m_db = db;
}

UsersPage::~UsersPage()
{
    delete ui;
}

void UsersPage::initializePage() {
    //Clear the page
    ui->tabWidget->clear();

    list_selected_subjects = ((KholloscopeWizard*) wizard())->get_assoc_subjects();

    //Get the selected subjects
    get_selected_subjects();

    //Display the tabs and populate each tab
    int i;
    for(i = 0; i < list_selected_subjects->length(); i++) {
        QListWidget *list = new QListWidget();
        populate(list, list_selected_subjects->at(i)->getId());
        list->setSelectionMode(QAbstractItemView::MultiSelection);
        ui->tabWidget->addTab(list, list_selected_subjects->at(i)->getShortName());

        connect(list, SIGNAL(itemSelectionChanged()), this, SLOT(selection_changed()));
    }

    QDate nextMonday = QDate::currentDate();
    while(nextMonday.dayOfWeek() != 1)
        nextMonday = nextMonday.addDays(1);
    ui->dateEdit->setDate(nextMonday);

    registerField("current_week", ui->comboBox);
    registerField("monday_date", ui->dateEdit);
}

void UsersPage::cleanupPage() {
    ((KholloscopeWizard*) wizard())->get_input()->clear();
}

void UsersPage::get_selected_subjects() {
    /** Gets the selected subjects **/
    list_selected_subjects->clear();
    //Get all subjects
    QList<Subject*>* subjects = ((KholloscopeWizard*) wizard())->get_subjects();

    //Treat
    int i;
    for(i = 0; i < subjects->length(); i++) {
        int id = subjects->at(i)->getId();
        if(field("subject_"+QString::number(id)).toBool()) {
            list_selected_subjects->append(subjects->at(i));
        }
    }
}


void UsersPage::populate(QListWidget *list, int id_subject) {
    //Empty the list
    list->clear();

    //Make request in this subject
    QSqlQuery query(*m_db);
    query.prepare("SELECT K.`id_users`, COUNT(K.`id`) FROM tau_kholles AS K WHERE K.`id_timeslots` IN "
                    "(SELECT T.`id` FROM tau_timeslots AS T JOIN tau_kholleurs AS KR ON T.`id_kholleurs` = KR.`id` WHERE KR.`id_subjects` = :id_subject) "
                    "GROUP BY K.`id_users`;");
    query.bindValue(":id_subject", id_subject);
    query.exec();

    QMap<int, int> number_kholles;

    while(query.next()) {
        number_kholles.insert(query.value(0).toInt(), query.value(1).toInt());
    }

    //Populate list
    QList<Student*>* students = ((KholloscopeWizard*) wizard())->get_students();

    int i;
    for(i = 0; i < students->length(); i++) {
        int n = 0;
        if(number_kholles.contains(students->value(i)->getId()))
            n = number_kholles.value(students->value(i)->getId());

        QListWidgetItem *item = new QListWidgetItem("[" + QString::number(n) + "] " + students->at(i)->getName() + ", " + students->at(i)->getFirst_name(), list);
        item->setData(Qt::UserRole, (qulonglong) students->at(i));
    }
}

void UsersPage::selection_changed() {
    QMap<int, QList<Student*> > *input = ((KholloscopeWizard*) wizard())->get_input();
    int i, j;
    for(i = 0; i < ui->tabWidget->count(); i++) {
        QList<QListWidgetItem*> selection = ((QListWidget*)ui->tabWidget->widget(i))->selectedItems();
        QList<Student*> students;

        for(j = 0; j < selection.length(); j++) {
            students.append((Student*) selection[j]->data(Qt::UserRole).toULongLong());
        }
        input->insert(list_selected_subjects->at(i)->getId(), students);
    }
}
