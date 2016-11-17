#include "coursesmanager.h"
#include "ui_coursesmanager.h"

CoursesManager::CoursesManager(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CoursesManager)
{
    //Create design
    ui->setupUi(this);
    connect(ui->list_groups, SIGNAL(itemSelectionChanged()), this, SLOT(onSelection_change()));
    connect(ui->pushButton_save, SIGNAL(clicked()), this, SLOT(save_changes()));
    //connect(ui->pushButton_close, SIGNAL(clicked()), this, SLOT(onClose_button()));
    connect(this, SIGNAL(accepted()), this, SLOT(onClose_button()));
    connect(this, SIGNAL(rejected()), this, SLOT(onClose_button()));
    connect(ui->copyToEven, SIGNAL(clicked()), this, SLOT(copyToEven()));
    connect(ui->copyToOdd, SIGNAL(clicked()), this, SLOT(copyToOdd()));
    m_shortcutNotepad = Notepad::shortcut();
    this->addAction(m_shortcutNotepad);

    //DB
    m_db = db;
    isChanged = false;
    current = NULL;
    begin_times << QTime::fromString("08:15:00", "h:mm:ss")
                << QTime::fromString("09:15:00", "h:mm:ss")
                << QTime::fromString("10:30:00", "h:mm:ss")
                << QTime::fromString("11:30:00", "h:mm:ss")
                << QTime::fromString("14:00:00", "h:mm:ss")
                << QTime::fromString("15:00:00", "h:mm:ss")
                << QTime::fromString("16:00:00", "h:mm:ss")
                << QTime::fromString("17:00:00", "h:mm:ss")
                << QTime::fromString("18:00:00", "h:mm:ss")
                << QTime::fromString("19:00:00", "h:mm:ss");

    end_times   << QTime::fromString("09:15:00", "h:mm:ss")
                << QTime::fromString("10:15:00", "h:mm:ss")
                << QTime::fromString("11:30:00", "h:mm:ss")
                << QTime::fromString("12:30:00", "h:mm:ss")
                << QTime::fromString("15:00:00", "h:mm:ss")
                << QTime::fromString("16:00:00", "h:mm:ss")
                << QTime::fromString("17:00:00", "h:mm:ss")
                << QTime::fromString("18:00:00", "h:mm:ss")
                << QTime::fromString("19:00:00", "h:mm:ss")
                << QTime::fromString("20:00:00", "h:mm:ss");

    //Get subjects and teachers
    get_subjects();
    get_teachers();
    //Set up design
    int day, i, j;
    for(day = 0; day < 6; day++) {
        for(i = 0; i < 10; i++) {
            //Semaine paire
            QVBoxLayout *layout = new QVBoxLayout();

            QComboBox *subjects = new QComboBox();
            subjects->addItem("");
            for(j = 0; j < list_subjects.length(); j++) {
                subjects->addItem(list_subjects[j]->getShortName(), list_subjects[j]->getId());
            }
            subjects->setEnabled(false);
            connect(subjects, SIGNAL(activated(int)), this, SLOT(courses_changed()));

            QComboBox *teachers = new QComboBox();
            teachers->addItem("");
            for(j = 0; j < list_teachers.length(); j++) {
                teachers->addItem(list_teachers[j]->getName(), list_teachers[j]->getId());
            }
            teachers->setEnabled(false);
            connect(teachers, SIGNAL(activated(int)), this, SLOT(courses_changed()));

            layout->setContentsMargins(5, 5, 5, 5);
            layout->setSpacing(2);
            layout->addWidget(subjects);
            layout->addWidget(teachers);

            ui->grid_even->addLayout(layout, i+1, day+1);

            //Semaine impaire
            layout = new QVBoxLayout();

            subjects = new QComboBox();
            subjects->addItem("");
            for(j = 0; j < list_subjects.length(); j++) {
                subjects->addItem(list_subjects[j]->getShortName(), list_subjects[j]->getId());
            }
            subjects->setEnabled(false);
            connect(subjects, SIGNAL(activated(int)), this, SLOT(courses_changed()));

            teachers = new QComboBox();
            teachers->addItem("");
            for(j = 0; j < list_teachers.length(); j++) {
                teachers->addItem(list_teachers[j]->getName(), list_teachers[j]->getId());
            }
            teachers->setEnabled(false);
            connect(teachers, SIGNAL(activated(int)), this, SLOT(courses_changed()));

            layout->setContentsMargins(5, 5, 5, 5);
            layout->setSpacing(2);
            layout->addWidget(subjects);
            layout->addWidget(teachers);

            ui->grid_odd->addLayout(layout, i+1, day+1);
        }
    }

    update_list_groups();

}

CoursesManager::~CoursesManager()
{
    delete ui;
    free_groups();
    free_subjects();
    free_teachers();
    this->removeAction(m_shortcutNotepad);
    delete m_shortcutNotepad;
}

bool CoursesManager::free_subjects() {
    while(!list_subjects.isEmpty()) {
        delete list_subjects.takeFirst();
    }
    return true;
}

bool CoursesManager::free_teachers() {
    while(!list_teachers.isEmpty()) {
        delete list_teachers.takeFirst();
    }
    return true;
}

bool CoursesManager::free_groups() {
    while(!queue_displayedGroups.isEmpty()) {
        delete queue_displayedGroups.dequeue();
    }
    return true;
}

bool CoursesManager::get_subjects() {
    //Clear the list
    free_subjects();

    //Prepare query
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, shortName, color FROM tau_subjects ORDER BY name");

    //Populate the list
    while(query.next()) {
        Subject* subj = new Subject();
        subj->setId(query.value(0).toInt());
        subj->setName(query.value(1).toString());
        subj->setShortName(query.value(2).toString());
        subj->setColor(query.value(3).toString());
        list_subjects.append(subj);
    }
    return true;
}

bool CoursesManager::get_teachers() {
    //Clear the list
    free_teachers();

    //Prepare query
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, id_subjects FROM tau_teachers ORDER BY name");

    //Populate the list
    while(query.next()) {
        Teacher* teacher = new Teacher();
        teacher->setId(query.value(0).toInt());
        teacher->setName(query.value(1).toString());
        teacher->setId_subjects(query.value(2).toInt());
        list_teachers.append(teacher);
    }
    return true;
}

bool CoursesManager::update_list_groups() {
    //Clear list
    free_groups();
    ui->list_groups->clear();

    //Prepare query
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name FROM tau_groups WHERE is_deleted = 0 ORDER BY name");

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

void CoursesManager::courses_changed() {
    isChanged = true;
}

bool CoursesManager::update_courses(QGridLayout *grid, int week) {
    //Prepare query
    QSqlQuery query(*m_db);
    query.prepare("SELECT id, id_subjects, id_teachers, id_day, time_start, time_end FROM tau_courses WHERE id_groups=:id_groups AND id_week=:id_week");
    query.bindValue(":id_groups", current->getId());
    query.bindValue(":id_week", week);
    query.exec();

    QList<QMap<QTime,Course*>*> list;
    int i, j;
    for(i = 0; i < 6; i++) {
        list.append(new QMap<QTime, Course*>);
    }

    while(query.next()) {
        Course* course = new Course();
        course->setId(query.value(0).toInt());
        course->setId_subjects(query.value(1).toInt());
        course->setId_teachers(query.value(2).toInt());
        course->setId_day(query.value(3).toInt());
        course->setTime_start(QTime::fromString(query.value(4).toString(), "h:mm:ss"));
        course->setTime_end(QTime::fromString(query.value(5).toString(), "h:mm:ss"));
        course->setId_groups(current->getId());
        course->setId_week(week);

        list[course->getId_day()-1]->insert(course->getTime_start(), course);
    }

    for(i = 0; i < 6; i++) {
        for(j = 0; j < 10; j++) {
            if(list[i]->contains(begin_times[j])) {
                Course* c = list[i]->value(begin_times[j]);
                QComboBox* subject = (QComboBox*) grid->itemAtPosition(j+1, i+1)->layout()->itemAt(0)->widget();
                QComboBox* teacher = (QComboBox*) grid->itemAtPosition(j+1, i+1)->layout()->itemAt(1)->widget();

                subject->setCurrentIndex(subject->findData(c->getId_subjects()));
                teacher->setCurrentIndex(teacher->findData(c->getId_teachers()));
            }
            else {
                QComboBox* subject = (QComboBox*) grid->itemAtPosition(j+1, i+1)->layout()->itemAt(0)->widget();
                QComboBox* teacher = (QComboBox*) grid->itemAtPosition(j+1, i+1)->layout()->itemAt(1)->widget();

                subject->setCurrentIndex(0);
                teacher->setCurrentIndex(0);
            }
        }
    }

    //Free everything
    while(!list.isEmpty()) {
        QMap<QTime, Course*> *map = list.takeFirst();
        foreach(Course* c, *map) {
            delete c;
        }
        delete map;
    }

    return true;
}

bool CoursesManager::save_changes() {
    if(current == NULL) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un groupe.");
        return false;
    }

    //Delete all courses of this group
    QSqlQuery query(*m_db);
    query.prepare("DELETE FROM tau_courses WHERE id_groups=:id_groups");
    query.bindValue(":id_groups", current->getId());
    query.exec();

    //Save both weeks and show feedback
    if(save(ui->grid_even, 1) && save(ui->grid_odd, 2)) {
        QMessageBox::information(this, "Sauvegarde réussie", "Les modifications ont bien été sauvegardées");
    }
    else {
        QMessageBox::warning(this, "Attention", "Certains cours ont été mal complétés et n'ont donc pu être enregistrés...");
    }
    isChanged = false;    

    //If this group is same as last group
    QList<QListWidgetItem*> selection = ui->list_groups->selectedItems();
    if(selection.length() > 0 && (Group*) selection[0]->data(Qt::UserRole).toULongLong() == current) {
        update_courses(ui->grid_even, 1);
        update_courses(ui->grid_odd, 2);
    }

    return true;
}

bool CoursesManager::save(QGridLayout *grid, int week) {
    bool res = true;

    //Cycle through all QComboboxes
    int i, j;
    for(i = 0; i < 6; i++) {
        for(j = 0; j < 10; j++) {
            QComboBox* subject = (QComboBox*) grid->itemAtPosition(j+1, i+1)->layout()->itemAt(0)->widget();
            QComboBox* teacher = (QComboBox*) grid->itemAtPosition(j+1, i+1)->layout()->itemAt(1)->widget();

            if(subject->currentText() == "" || teacher->currentText() == "") {
                if(subject->currentText() != "" || teacher->currentText() != "")
                    res = false;

                continue;
            }

            QSqlQuery query(*m_db);
            query.prepare("INSERT INTO tau_courses(id_subjects, time_start, time_end, id_groups, id_teachers, id_day, id_week) VALUES(:id_subjects, :time_start, :time_end, :id_groups, :id_teachers, :id_day, :id_week)");
            query.bindValue(":id_subjects", subject->currentData().toInt());
            query.bindValue(":time_start", begin_times[j].toString("hh:mm:ss"));
            query.bindValue(":time_end", end_times[j].toString("hh:mm:ss"));
            query.bindValue(":id_groups", current->getId());
            query.bindValue(":id_teachers", teacher->currentData().toInt());
            query.bindValue(":id_day", i+1);
            query.bindValue(":id_week", week);
            query.exec();
        }
    }
    return res;
}

void CoursesManager::onSelection_change() {
    if(isChanged) {
        int res = QMessageBox::question(this, "Modifications non sauvegardées", "Voulez-vous sauvegarder les modifications faites à l'emploi du temps de ce groupe?", QMessageBox::Yes | QMessageBox::No);
        if(res == QMessageBox::Yes) {
            save_changes();
        }
    }
    isChanged = false;

    QList<QListWidgetItem*> selection = ui->list_groups->selectedItems();
    if(selection.length() <= 0) {
        //Disable all
        ui->copyToEven->setEnabled(false);
        ui->copyToOdd->setEnabled(false);

        int i, j;
        for(i = 0; i < 6; i++) {
            for(j = 0; j < 10; j++) {
                ui->grid_even->itemAtPosition(j+1, i+1)->layout()->itemAt(0)->widget()->setEnabled(false);
                ui->grid_even->itemAtPosition(j+1, i+1)->layout()->itemAt(1)->widget()->setEnabled(false);
                ui->grid_odd->itemAtPosition(j+1, i+1)->layout()->itemAt(0)->widget()->setEnabled(false);
                ui->grid_odd->itemAtPosition(j+1, i+1)->layout()->itemAt(1)->widget()->setEnabled(false);
            }
        }

        //Set current
        current = NULL;
        return;
    }

    //Enable all
    ui->copyToEven->setEnabled(true);
    ui->copyToOdd->setEnabled(true);

    int i, j;
    for(i = 0; i < 6; i++) {
        for(j = 0; j < 10; j++) {
            ui->grid_even->itemAtPosition(j+1, i+1)->layout()->itemAt(0)->widget()->setEnabled(true);
            ui->grid_even->itemAtPosition(j+1, i+1)->layout()->itemAt(1)->widget()->setEnabled(true);
            ui->grid_odd->itemAtPosition(j+1, i+1)->layout()->itemAt(0)->widget()->setEnabled(true);
            ui->grid_odd->itemAtPosition(j+1, i+1)->layout()->itemAt(1)->widget()->setEnabled(true);
        }
    }

    //Set current
    current = (Group*) selection[0]->data(Qt::UserRole).toULongLong();

    update_courses(ui->grid_even, 1);
    update_courses(ui->grid_odd, 2);
}

void CoursesManager::copyToEven() {
    //Display warning
    int res = QMessageBox::warning(this, "Copie en cours", "Vous êtes sur le point de copier les cours de la semaine impaire vers la semaine paire. <br />Voulez-vous continuer ?", QMessageBox::Yes | QMessageBox::No);

    //Copy
    if(res == QMessageBox::Yes) {
        int i, j;
        for(i = 0; i < 6; i++) {
            for(j = 0; j < 10; j++) {
                ((QComboBox*)ui->grid_even->itemAtPosition(j+1, i+1)->layout()->itemAt(0)->widget())->setCurrentIndex(
                            ((QComboBox*)ui->grid_odd->itemAtPosition(j+1, i+1)->layout()->itemAt(0)->widget())->currentIndex());
                ((QComboBox*)ui->grid_even->itemAtPosition(j+1, i+1)->layout()->itemAt(1)->widget())->setCurrentIndex(
                            ((QComboBox*)ui->grid_odd->itemAtPosition(j+1, i+1)->layout()->itemAt(1)->widget())->currentIndex());
            }
        }
        isChanged = true;
    }
}

void CoursesManager::copyToOdd() {
    //Display warning
    int res = QMessageBox::warning(this, "Copie en cours", "Vous êtes sur le point de copier les cours de la semaine paire vers la semaine impaire. <br />Voulez-vous continuer ?", QMessageBox::Yes | QMessageBox::No);

    //Copy
    if(res == QMessageBox::Yes) {
        int i, j;
        for(i = 0; i < 6; i++) {
            for(j = 0; j < 10; j++) {
                ((QComboBox*)ui->grid_odd->itemAtPosition(j+1, i+1)->layout()->itemAt(0)->widget())->setCurrentIndex(
                            ((QComboBox*)ui->grid_even->itemAtPosition(j+1, i+1)->layout()->itemAt(0)->widget())->currentIndex());
                ((QComboBox*)ui->grid_odd->itemAtPosition(j+1, i+1)->layout()->itemAt(1)->widget())->setCurrentIndex(
                            ((QComboBox*)ui->grid_even->itemAtPosition(j+1, i+1)->layout()->itemAt(1)->widget())->currentIndex());
            }
        }
        isChanged = true;
    }
}

void CoursesManager::onClose_button() {
    //Ask for confirmation
    if(isChanged) {
        int res = QMessageBox::question(this, "Modifications non sauvegardées", "Voulez-vous sauvegarder les modifications faites à l'emploi du temps de ce groupe?", QMessageBox::Yes | QMessageBox::No);
        if(res == QMessageBox::Yes) {
            save_changes();
        }
    }
}
