#include "timetablemanager.h"
#include "ui_timetablemanager.h"

TimetableManager::TimetableManager(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TimetableManager)
{
    ui->setupUi(this);
    installEventFilter(this);

    m_db = db;

    connect(ui->list_groups, SIGNAL(currentRowChanged(int)), this, SLOT(select_group()));

    update_list_groups();
    QMap<int, Subject*> subjects = update_list_subjects();

    currentScene = new TimeTable(m_db, NULL, subjects);
    currentScene->setInputs(ui->timeEdit_hour, ui->timeEdit_duration, ui->comboBox_subjects);
    ui->viewTable->setScene(currentScene);
}

TimetableManager::~TimetableManager()
{
    delete ui;
    free_groups();
    free_subjects();
}

bool TimetableManager::free_groups() {
    while(!queue_displayedGroups.isEmpty()) {
        delete queue_displayedGroups.dequeue();
    }
    return true;
}

bool TimetableManager::update_list_groups() {
    //Clear list
    free_groups();
    ui->list_groups->clear();

    //Prepare query
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name FROM tau_groups ORDER BY UPPER(name)");

    //Treat result
    while(query.next()) {
        Group* group = new Group();
        group->setId(query.value(0).toInt());
        group->setName(query.value(1).toString());
        QListWidgetItem *item = new QListWidgetItem(group->getName(), ui->list_groups);
        item->setData(Qt::UserRole, (qulonglong) group);
        queue_displayedGroups.enqueue(group);
    }
    return true;
}

bool TimetableManager::free_subjects() {
    while(!queue_displayedSubjects.isEmpty()) {
        delete queue_displayedSubjects.dequeue();
    }
    return true;
}

QMap<int, Subject*> TimetableManager::update_list_subjects() {
    //Clear list
    free_subjects();
    ui->comboBox_subjects->clear();
    QMap<int, Subject*> subjects;

    //Prepare query
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, shortName FROM tau_subjects ORDER BY UPPER(name)");

    //Treat result
    while(query.next()) {
        Subject* subj = new Subject();
        subj->setId(query.value(0).toInt());
        subj->setName(query.value(1).toString());
        subj->setShortName(query.value(2).toString());
        ui->comboBox_subjects->addItem(subj->getName(), (qulonglong) subj);
        queue_displayedSubjects.enqueue(subj);
        subjects[subj->getId()] = subj;
    }
    return subjects;
}

bool TimetableManager::select_group() {
    QListWidgetItem* item = ui->list_groups->currentItem();
    if(item)
        currentScene->setCurrent_group((Group*) item->data(Qt::UserRole).toULongLong());
    return true;
}

bool TimetableManager::eventFilter(QObject* obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Delete) {
                currentScene->deleteSelectedCourse();
                return true;
            }
        }
        return QDialog::eventFilter(obj, event);
}
