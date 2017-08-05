#include "timetablemanager.h"
#include "ui_timetablemanager.h"

TimetableManager::TimetableManager(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TimetableManager)
{   /** CONSTRUCTOR **/

    ui->setupUi(this);
    installEventFilter(this);

    /// Initalisation of the class properties
    // Database (SQL and local)
    m_db = db;

    /// Detection when the user change the selected group
    connect(ui->list_groups, SIGNAL(currentRowChanged(int)), this, SLOT(select_group()));

    /// Display the groups and the subjects
    update_list_groups();
    QMap<int, Subject*> subjects = update_list_subjects();

    /// Create the timetable to manage the courses
    currentScene = new TimeTable(m_db, NULL, subjects);
    currentScene->setInputs(ui->timeEdit_hour, ui->timeEdit_duration, ui->comboBox_subjects);
    ui->viewTable->setScene(currentScene);
}

TimetableManager::~TimetableManager()
{    /** DESTRUCTOR **/

    delete ui;
    free_groups();
    free_subjects();
}

bool TimetableManager::free_groups() {
    /** METHOD TO FREE THE GROUPS OF THE MEMORY **/

    while(!queue_displayedGroups.isEmpty())
        delete queue_displayedGroups.dequeue();
    return true;
}

bool TimetableManager::update_list_groups() {
    /** METHOD TO GET AND DISPLAY THE GROUPS **/

    //Clear list of groups
    free_groups();
    ui->list_groups->clear();

    //Prepare query to get the groups
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name FROM tau_groups ORDER BY UPPER(name)");

    //Treat result
    while(query.next()) {
        // Build the object Group
        Group* group = new Group();
        group->setId(query.value(0).toInt());
        group->setName(query.value(1).toString());
        // Display the group
        QListWidgetItem *item = new QListWidgetItem(group->getName(), ui->list_groups);
        item->setData(Qt::UserRole, (qulonglong) group);
        queue_displayedGroups.enqueue(group);
    }
    return true;
}

bool TimetableManager::free_subjects() {
    /** METHOD TO FREE THE SUBJECTS OF THE MEMORY **/

    while(!queue_displayedSubjects.isEmpty())
        delete queue_displayedSubjects.dequeue();
    return true;
}

QMap<int, Subject*> TimetableManager::update_list_subjects() {
    /** METHOD TO GET AND DISPLAY THE SUBJECTS **/
    /**     RETURN A MAP ASSOCIATING THE ID OF SUBJECTS WITH THE OBJECT **/

    //Clear list of subjects
    free_subjects();
    ui->comboBox_subjects->clear();

    // Map associating the id of subjects with the object
    QMap<int, Subject*> subjects;

    //Prepare query to get the subjects
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, shortName FROM tau_subjects ORDER BY UPPER(name)");

    //Treat result
    while(query.next()) {
        // To build a object "Subject"
        Subject* subj = new Subject();
        subj->setId(query.value(0).toInt());
        subj->setName(query.value(1).toString());
        subj->setShortName(query.value(2).toString());
        // To display the subject
        ui->comboBox_subjects->addItem(subj->getName(), (qulonglong) subj);
        queue_displayedSubjects.enqueue(subj);
        subjects[subj->getId()] = subj;
    }
    return subjects;
}

bool TimetableManager::select_group() {
    /** SLOT TO SELECT THE GROUP CHOOSE BY THE USER **/

    QListWidgetItem* item = ui->list_groups->currentItem();
    if(item)
        currentScene->setCurrent_group((Group*) item->data(Qt::UserRole).toULongLong());
    return true;
}

bool TimetableManager::eventFilter(QObject* obj, QEvent *event) {
    /** METHOD TO CATCH EVENTS **/

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Delete) {
            /// EVENT = PRESS DELETE KEY ==> DELETE THE SELECTED COURSE
            currentScene->deleteSelectedCourse();
            return true;
        }
    }
    return QDialog::eventFilter(obj, event);
}
