#include "printpdf.h"

PrintPDF::PrintPDF()
{

}

void PrintPDF::printKholles(QList<Student *> *students, QMap<int, Kholleur *> *kholleurs, QMap<int, Timeslot *> *timeslots, QDate monday_date, QMap<int, Kholle *> *kholloscope) {
    if(students->length() <= 0) {
        QMessageBox::critical(NULL, "Echec", "Aucun élève n'a été sélectionné.");
        return;
    }

    //Try to load directory preferences
    Preferences pref;
    QString pref_path = pref.dir();

    //Get file name
    QString filename = QFileDialog::getSaveFileName(NULL, "Enregistrer sous...",
                                                    pref_path + QDir::separator() + "Kholloscope_" + monday_date.toString("yyyyMMdd"),  "PDF (*.pdf)");

    if(filename == "")
        return;

    //Save directory in preferences
    QString dirpath = QFileInfo(filename).absoluteDir().absolutePath();
    pref.setDir(dirpath);

    switch(pref.formatPDF()) {
        case Preferences::StudentsDays:
            if(printKholles_StudentsDays(students, kholleurs, timeslots, monday_date, kholloscope, filename))
                QMessageBox::information(NULL, "Succès", "Fichier PDF généré.");
            break;
        case Preferences::StudentsSubjects:
            if(printKholles_StudentsSubjects(students, kholleurs, timeslots, monday_date, kholloscope, filename))
                QMessageBox::information(NULL, "Succès", "Fichier PDF généré.");
            break;
    }
}

bool PrintPDF::printKholles_StudentsDays(QList<Student *> *students, QMap<int, Kholleur *> *kholleurs, QMap<int, Timeslot *> *timeslots, QDate monday_date, QMap<int, Kholle *> *kholloscope, QString filename) {
    //Create the PDF Writer
    QPdfWriter writer(filename);
    writer.setPageSize(QPdfWriter::A3);
    writer.setPageOrientation(QPageLayout::Landscape);
    writer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
    writer.setCreator("SPARK Kholloscope");

    QPainter painter;

    if(!painter.begin(&writer)) {
        QMessageBox::critical(NULL, "Erreur", "Erreur lors de l'écriture du fichier. Le fichier est peut-être ouvert ?");
        return false;
    }

    //Get size in default units
    int width = writer.width();
    int height = writer.height();

    //Paint here
    painter.setPen(QPen(QBrush(Qt::black), 5));

    //Calculate line height and cell width

    //Number of rows
    int num_rows = students->length() + 3 + 1;
    int row_height = height / num_rows;

    //Create two fonts -> one for the normal text, one for title
    QFont normal_font = painter.font();
    if(QFontMetrics(normal_font).lineSpacing() > row_height ||
            averageWidthStudents(QFontMetrics(normal_font), students) * 1.4 > width/7 ||
            QFontMetrics(normal_font).width("Mercredi") > width/7) {
        while(QFontMetrics(normal_font).lineSpacing() > row_height ||
              averageWidthStudents(QFontMetrics(normal_font), students) * 1.4 > width/7 ||
              QFontMetrics(normal_font).width("Mercredi") > width/7)
            normal_font.setPointSize(normal_font.pointSize() - 1);
    }
    if(QFontMetrics(normal_font).lineSpacing() < row_height &&
            averageWidthStudents(QFontMetrics(normal_font), students) * 1.4 < width/7 &&
            QFontMetrics(normal_font).width("Mercredi") < width/7) {
        while(QFontMetrics(normal_font).lineSpacing() <= row_height &&
              averageWidthStudents(QFontMetrics(normal_font), students) * 1.4 <= width/7 &&
              QFontMetrics(normal_font).width("Mercredi") <= width/7)
            normal_font.setPointSize(normal_font.pointSize() + 1);
        normal_font.setPointSize(normal_font.pointSize() - 1);
    }

    /// Write title
    writeTitle(&painter, width, 2*row_height, monday_date);

    //Get font metrics
    painter.setFont(normal_font);
    QFontMetrics font = painter.fontMetrics();

    //Name length (length of longest name) => also the width of 1st column
    int name_width = averageWidthStudents(QFontMetrics(normal_font), students) * 1.4;
    int cell_width = (width - name_width) / 6;

    //Draw the grid
    painter.drawLine(0, 2*row_height, 0, height-row_height);
    for(int i=0; i<=6; i++)
        painter.drawLine(name_width + i*cell_width, 2*row_height, name_width + i*cell_width, height-row_height);

    painter.drawLine(0, 2*row_height, width, 2*row_height);

    painter.setFont(normal_font);
    font = painter.fontMetrics();
    painter.drawText(name_width + cell_width/2 - font.width("Lundi")/2, 2*row_height + font.ascent() + font.leading()/2, "Lundi");
    painter.drawText(name_width + 3*cell_width/2 - font.width("Mardi")/2, 2*row_height + font.ascent() + font.leading()/2, "Mardi");
    painter.drawText(name_width + 5*cell_width/2 - font.width("Mercredi")/2, 2*row_height + font.ascent() + font.leading()/2, "Mercredi");
    painter.drawText(name_width + 7*cell_width/2 - font.width("Jeudi")/2, 2*row_height + font.ascent() + font.leading()/2, "Jeudi");
    painter.drawText(name_width + 9*cell_width/2 - font.width("Vendredi")/2, 2*row_height + font.ascent() + font.leading()/2, "Vendredi");
    painter.drawText(name_width + 11*cell_width/2 - font.width("Samedi")/2, 2*row_height + font.ascent() + font.leading()/2, "Samedi");

    painter.drawLine(0, 3*row_height, width, 3*row_height);

    QMap<int, int> assoc; //assoc table between student ids and their index in the alphabetical order list

    //Student names
    int i, j, k;
    for(i = 0; i < students->length(); i++) {
        painter.drawText(0, (3+i)*row_height + font.ascent() + font.leading()/2, displayStudent(students->at(i), name_width, normal_font));
        painter.drawLine(0, (4+i)*row_height, width, (4+i)*row_height);
        assoc.insert(students->at(i)->getId(), i);
    }

    //Reorganise the kholles
    QList<QList<QList<Kholle*>*>*> kholles;
    for(i = 0; i < students->length(); i++) {
        kholles.append(new QList<QList<Kholle*>*>);
        for(j = 0; j < 6; j++) {
            kholles[i]->append(new QList<Kholle*>);
        }
    }

    foreach(Kholle* k, *kholloscope) {
        kholles[assoc.value(k->getId_students())]->at(timeslots->value(k->getId_timeslots())->getDate().dayOfWeek() - 1)->append(k);
    }

    for(i = 0; i < students->length(); i++) {
        for(j = 0; j < 6; j++) {
            QList<Kholle*> *list = kholles[i]->at(j);

            QString text = "";
            for(k = 0; k < list->length(); k++) { //Paint
                text += (text != "") ? " | " : "";
                text += timeslots->value(list->at(k)->getId_timeslots())->getTime_start().toString("HH:mm") + " " +
                        kholleurs->value(timeslots->value(list->at(k)->getId_timeslots())->getId_kholleurs())->getName();
            }
            text = " " + text + " ";

            QFont kholle_font = QFont(normal_font);
            while(QFontMetrics(kholle_font).width(text) > cell_width)
                kholle_font.setPointSize(kholle_font.pointSize() - 1);
            painter.setFont(kholle_font);

            painter.drawText(name_width + j*cell_width,
                             (3+i)*row_height + font.ascent() + font.leading()/2,
                             text);

            painter.setFont(normal_font); //Put back normal font
        }
    }
    displaySPARK(&painter, width, height-row_height, row_height, normal_font);

    //Delete QLists
    while(!kholles.isEmpty()) {
        QList<QList<Kholle*>*> *list = kholles.takeFirst();
        while(!list->isEmpty()) {
            QList<Kholle*> *sub_list = list->takeFirst();
            sub_list->clear(); //Only clear, don't delete Kholles, they will be free'd in destructor
            delete sub_list;
        }
        delete list;
    }

    painter.end();
    return true;
/*
    //Create the PDF Writer
    QPdfWriter writer(filename);
    writer.setPageSize(QPdfWriter::A3);
    writer.setPageOrientation(QPageLayout::Landscape);
    writer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    QPainter painter;

    if(!painter.begin(&writer)) {
        QMessageBox::critical(NULL, "Erreur", "Erreur lors de l'écriture du fichier. Le fichier est peut-être ouvert ?");
        return false;
    }

    //Get size in default units
    int width = writer.width();
    int height = writer.height();

    //Paint here
    painter.setPen(QPen(QBrush(Qt::black), 5));

    //Reorganise the kholles
    QMap<int, int> assoc; //assoc table between student ids and their index in the alphabetical order list

    for(int i = 0; i < students->length(); i++)
        assoc.insert(students->at(i)->getId(), i);

    QList<QList<QList<Kholle*>*>*> kholles;
    for(int i = 0; i < students->length(); i++) {
        kholles.append(new QList<QList<Kholle*>*>);
        for(int j = 0; j < 6; j++)
            kholles[i]->append(new QList<Kholle*>);
    }

    foreach(Kholle* k, *kholloscope)
        kholles[assoc.value(k->getId_students())]->at(timeslots->value(k->getId_timeslots())->getDate().dayOfWeek() - 1)->append(k);

    //Calculate line height and cell width

    //Number of rows
    int num_rows = 3;
    for(int i = 0; i < students->length(); i++) {
        int height = heightStudent(i, &kholles);
        num_rows += (height >= 1) ? height : 1;
    }
    int row_height = height / num_rows;

    //Create two fonts -> one for the normal text, one for title
    QFont normal_font = painter.font();
    if(QFontMetrics(normal_font).lineSpacing() > row_height ||
            averageWidthStudents(QFontMetrics(normal_font), students) * 1.4 > width/7 ||
            QFontMetrics(normal_font).width("Mercredi") > width/7) {
        while(QFontMetrics(normal_font).lineSpacing() > row_height ||
              averageWidthStudents(QFontMetrics(normal_font), students) * 1.4 > width/7 ||
              QFontMetrics(normal_font).width("Mercredi") > width/7)
            normal_font.setPointSize(normal_font.pointSize() - 1);
    }
    if(QFontMetrics(normal_font).lineSpacing() < row_height &&
            averageWidthStudents(QFontMetrics(normal_font), students) * 1.4 < width/7 &&
            QFontMetrics(normal_font).width("Mercredi") < width/7) {
        while(QFontMetrics(normal_font).lineSpacing() <= row_height &&
              averageWidthStudents(QFontMetrics(normal_font), students) * 1.4 <= width/7 &&
              QFontMetrics(normal_font).width("Mercredi") <= width/7)
            normal_font.setPointSize(normal_font.pointSize() + 1);
        normal_font.setPointSize(normal_font.pointSize() - 1);
    }

    /// Write title
    writeTitle(&painter, width, 2*row_height, monday_date);

    //Get font metrics
    painter.setFont(normal_font);
    QFontMetrics font = painter.fontMetrics();

    //Name length (length of longest name) => also the width of 1st column
    int name_width = averageWidthStudents(QFontMetrics(normal_font), students) * 1.4;
    int cell_width = (width - name_width) / 6;

    //Draw the grid
    painter.drawLine(0, 2*row_height, 0, height);
    for(int i=0; i<=6; i++)
        painter.drawLine(name_width + i*cell_width, 2*row_height, name_width + i*cell_width, height);

    painter.drawLine(0, 2*row_height, width, 2*row_height);

    painter.setFont(normal_font);
    font = painter.fontMetrics();
    painter.drawText(name_width + cell_width/2 - font.width("Lundi")/2, 2*row_height + font.ascent() + font.leading()/2, "Lundi");
    painter.drawText(name_width + 3*cell_width/2 - font.width("Mardi")/2, 2*row_height + font.ascent() + font.leading()/2, "Mardi");
    painter.drawText(name_width + 5*cell_width/2 - font.width("Mercredi")/2, 2*row_height + font.ascent() + font.leading()/2, "Mercredi");
    painter.drawText(name_width + 7*cell_width/2 - font.width("Jeudi")/2, 2*row_height + font.ascent() + font.leading()/2, "Jeudi");
    painter.drawText(name_width + 9*cell_width/2 - font.width("Vendredi")/2, 2*row_height + font.ascent() + font.leading()/2, "Vendredi");
    painter.drawText(name_width + 11*cell_width/2 - font.width("Samedi")/2, 2*row_height + font.ascent() + font.leading()/2, "Samedi");

    painter.drawLine(0, 3*row_height, width, 3*row_height);

    int numRow = 0;
    int height_lastLine = 2*row_height;
    for(int i = 0; i < students->length(); i++) {
        // For each student
        int nbRowsStudent = 0;
        for(int j=0; j < kholles.at(i)->count(); j++) {
            int nbRowsDays = 0;
            QList<Kholle*> *list = kholles.at(i)->at(j);
            for(int k = 0; k < list->length(); k++) {
                // Generate the text of the kholle
                Timeslot* ts = timeslots->value(list->at(k)->getId_timeslots());
                QString text = " " + ts->getTime_start().toString("HH:mm") +
                                " " + kholleurs->value(timeslots->value(list->at(k)->getId_timeslots())->getId_kholleurs())->getName() + " ";

                // Choose a adapted font
                QFont kholle_font = QFont(normal_font);
                while(QFontMetrics(kholle_font).width(text) > cell_width)
                    kholle_font.setPointSize(kholle_font.pointSize() - 1);
                painter.setFont(kholle_font);

                // Display the kholle
                painter.drawText(name_width + j*cell_width,
                                 (3+numRow+k)*row_height + (row_height - QFontMetrics(kholle_font).height()) / 2 + QFontMetrics(kholle_font).ascent() + QFontMetrics(kholle_font).leading()/2,
                                 text);
                nbRowsDays++;

            }
            if(nbRowsDays > nbRowsStudent)
                nbRowsStudent = nbRowsDays;
        }
        // Write the student name and draw a horizontal line
        nbRowsStudent = (nbRowsStudent > 0) ? nbRowsStudent : 1;
        painter.setFont(normal_font);
        painter.drawText(0, (3+numRow)*row_height + (nbRowsStudent-1)*row_height/2 + (row_height - QFontMetrics(normal_font).height()) / 2 + nbRowsStudent+ font.ascent() + font.leading()/2,
                         displayStudent(students->at(i), name_width, normal_font));
        numRow += nbRowsStudent;
        painter.drawLine(0, (3+numRow)*row_height, name_width + 6*cell_width, (3+numRow)*row_height);
        height_lastLine = (3+numRow)*row_height;
    }

    //Delete QLists
    while(!kholles.isEmpty()) {
        QList<QList<Kholle*>*> *list = kholles.takeFirst();
        while(!list->isEmpty()) {
            QList<Kholle*> *sub_list = list->takeFirst();
            sub_list->clear(); //Only clear, don't delete Kholles, they will be free'd in destructor
            delete sub_list;
        }
        delete list;
    }

    painter.end();
    return true;
*/
}

bool PrintPDF::printKholles_StudentsSubjects(QList<Student *> *students, QMap<int, Kholleur *> *kholleurs, QMap<int, Timeslot *> *timeslots, QDate monday_date, QMap<int, Kholle *> *kholloscope, QString filename) {
    /// Create the PDF Writer
    QPdfWriter writer(filename);
    writer.setPageSize(QPdfWriter::A3);
    writer.setPageOrientation(QPageLayout::Portrait);
    writer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
    writer.setCreator("SPARK Kholloscope");

    QPainter painter;

    if(!painter.begin(&writer)) {
        QMessageBox::critical(NULL, "Erreur", "Erreur lors de l'écriture du fichier. Le fichier est peut-être ouvert ?");
        return false;
    }

    //Get size in default units
    int width = writer.width();
    int height = writer.height();

    //Paint here
    painter.setPen(QPen(QBrush(Qt::black), 5));
    QFont normal_font = painter.font();

    /// Creation of the list of subjects
    QMap<int, Subject*> subjects_map;
    foreach(Kholle* k, *kholloscope) {
        Subject* s = timeslots->value(k->getId_timeslots())->kholleur()->subject();
        if(! subjects_map.contains(s->getId()))
            subjects_map.insert(s->getId(), s);
    }
    QList<int> subjects_list;
    QString maxSubjects = "";
    int maxWidthSubjects = 0;
    foreach(Subject* s, subjects_map) {
        subjects_list.append(s->getId());
        int width = QFontMetrics(normal_font).width(s->getName());
        if(width > maxWidthSubjects) {
            maxWidthSubjects = width;
            maxSubjects = s->getName();
        }
    }
    int nbColumns = subjects_list.length() + 1;

    /// Creation of the grid
    // Initialisation
    QList<QList<QList<Kholle*>*>*>* grid = new QList<QList<QList<Kholle*>*>*>();
    for(int i=0; i<students->length(); i++) {
        grid->append(new QList<QList<Kholle*>*>);
        for(int j=0; j<subjects_list.length(); j++)
            grid->at(i)->append(new QList<Kholle*>);
    }

    QMap<int, int> assoc_students; //assoc table between student ids and their index in the alphabetical order list
    QMap<int, int> assoc_subjects; //assoc table between subject ids and their index in the ordered list
    for(int i = 0; i < students->length(); i++)
        assoc_students.insert(students->at(i)->getId(), i);
    for(int i = 0; i < subjects_list.length(); i++)
        assoc_subjects.insert(subjects_list[i], i);

    // Filling of the grid
    foreach(Kholle* k, *kholloscope) {
        int id_stdnt = k->getId_students();
        int id_subject = timeslots->value(k->getId_timeslots())->kholleur()->getId_subjects();
        grid->at(assoc_students[id_stdnt])->at(assoc_subjects[id_subject])->append(k);
    }

    //Number of rows (one student can be have many rows)
    int num_rows = 3 + 1;
    for(int i = 0; i < students->length(); i++) {
        int height = heightStudent(i, grid);
        num_rows += (height >= 1) ? height : 1;
    }
    int row_height = height / num_rows;

    /// Create the stardard font
    normal_font.setPointSize(1);
    while(QFontMetrics(normal_font).lineSpacing() <= row_height &&
          averageWidthStudents(QFontMetrics(normal_font), students) * 1.4 <= width/nbColumns &&
          QFontMetrics(normal_font).width(" " + maxSubjects + " ") <= width/nbColumns)
        normal_font.setPointSize(normal_font.pointSize() + 1);
    normal_font.setPointSize(normal_font.pointSize() - 1);

    QFontMetrics font = painter.fontMetrics();

    /// Write title
    writeTitle(&painter, width, 2*row_height, monday_date);
    if(subjects_list.length() > 0) {
        //Get font metrics
        painter.setFont(normal_font);
        font = painter.fontMetrics();

        // Width of the columns (Name and others)
        int name_width = averageWidthStudents(QFontMetrics(normal_font), students) * 1.4;
        int cell_width = (width - name_width) / (nbColumns - 1);

        /// Create First row of the kholloscope (with subjects)
        // Horizontal line
        painter.drawLine(0, 2*row_height, name_width + (nbColumns-1)*cell_width, 2*row_height);
        painter.drawLine(0, 3*row_height, name_width + (nbColumns-1)*cell_width, 3*row_height);

        // Write subjects
        painter.setFont(normal_font);
        font = painter.fontMetrics();
        for(int i=0; i<subjects_list.count(); i++) {
            Subject* s = subjects_map[subjects_list[i]];
            painter.drawText(name_width + (2*i+1)*cell_width/2 - font.width(" " + s->getName() + " ")/2,
                             2*row_height + (row_height - QFontMetrics(normal_font).height()) / 2 + font.ascent() + font.leading()/2,
                             " " + s->getName() + " ");
        }


        /// Write the kholles
        int numRow = 0;
        QStringList initDays;
        initDays << "" << "Lu" << "Ma" << "Me" << "Je" << "Ve" << "Sa" << "Di";
        int height_lastLine = 2*row_height;
        for(int i = 0; i < students->length(); i++) {
            // For each student
            int nbRowsStudent = 0;
            for(int j=0; j < grid->at(i)->count(); j++) {
                int nbRowsSubject = 0;
                QList<Kholle*> *list = grid->at(i)->at(j);
                for(int k = 0; k < list->length(); k++) {
                    // Generate the text of the kholle
                    Timeslot* ts = timeslots->value(list->at(k)->getId_timeslots());
                    QString text = " " + initDays[timeslots->value(list->at(k)->getId_timeslots())->getDate().dayOfWeek()] + " " + ts->getTime_start().toString("HH:mm") +
                                    " - " + kholleurs->value(timeslots->value(list->at(k)->getId_timeslots())->getId_kholleurs())->getName() + " ";

                    // Choose a adapted font
                    QFont kholle_font = QFont(normal_font);
                    while(QFontMetrics(kholle_font).width(text) > cell_width)
                        kholle_font.setPointSize(kholle_font.pointSize() - 1);
                    painter.setFont(kholle_font);

                    // Display the kholle
                    painter.drawText(name_width + j*cell_width,
                                     (3+numRow+k)*row_height + (row_height - QFontMetrics(kholle_font).height()) / 2 + QFontMetrics(kholle_font).ascent() + QFontMetrics(kholle_font).leading()/2,
                                     text);
                    nbRowsSubject++;

                }
                if(nbRowsSubject > nbRowsStudent)
                    nbRowsStudent = nbRowsSubject;
            }
            // Write the student name and draw a horizontal line
            nbRowsStudent = (nbRowsStudent > 0) ? nbRowsStudent : 1;
            painter.setFont(normal_font);
            painter.drawText(0, (3+numRow)*row_height + (nbRowsStudent-1)*row_height/2 + (row_height - QFontMetrics(normal_font).height()) / 2 + nbRowsStudent+ font.ascent() + font.leading()/2,
                             displayStudent(students->at(i), name_width, normal_font));
            numRow += nbRowsStudent;
            painter.drawLine(0, (3+numRow)*row_height, name_width + (nbColumns-1)*cell_width, (3+numRow)*row_height);
            height_lastLine = (3+numRow)*row_height;
        }

        /// Draw the vertical lines
        painter.drawLine(0, 2*row_height, 0, height_lastLine);
        for(int i=0; i<=subjects_list.count(); i++)
            painter.drawLine(name_width + i*cell_width, 2*row_height, name_width + i*cell_width, height_lastLine);

        displaySPARK(&painter, width, height_lastLine, row_height, normal_font);
    } else {
        QFont text_font = QFont(normal_font);
        text_font.setPointSize(40);
        painter.setFont(text_font);
        QString textInfo = "Pas de kholles cette semaine...";
        painter.drawText((width-QFontMetrics(text_font).width(textInfo))/2, (height-QFontMetrics(text_font).height())/2, textInfo);

        displaySPARK(&painter, width, height - row_height, row_height, text_font);
    }


    /// Delete the GRID
    while(!grid->isEmpty()) {
        QList<QList<Kholle*>*> *column = grid->takeFirst();
        while(!column->isEmpty()) {
            QList<Kholle*> *kholles = column->takeFirst();
            kholles->clear(); //Only clear, don't delete Kholles, they will be free'd in destructor
            delete kholles;
        }
        delete column;
    }

    painter.end();
    return true;
}

int PrintPDF::heightStudent(int numStudent, QList<QList<QList<Kholle*>*>*>* grid) {
    QList<QList<Kholle*>*>* subjects = grid->at(numStudent);
    int height = 0;
    for(int i=0; i<subjects->length(); i++) {
        if(height < subjects->at(i)->length())
            height = subjects->at(i)->length();
    }
    return height;
}
/*
int PrintPDF::heightStudent1(int numStudent, QList<QList<QList<Kholle*>*>*>* kholles) {
    QList<QList<Kholle*>*>* days = kholles->at(numStudent);
    int height = 0;
    for(int i=0; i<days->length(); i++) {
        if(height < days->at(i)->length())
            height = days->at(i)->length();
    }
    return height;
}*/

double PrintPDF::averageWidthStudents(QFontMetrics font, QList<Student *> *students) {
    int sum = 0;
    for(int i = 0; i < students->length(); i++) {
        QString text = " " + students->at(i)->getName() + "  " + students->at(i)->getFirst_name() + " ";
        sum += font.width(text);
    }
    return (sum/students->length());
}

bool PrintPDF::writeTitle(QPainter* painter, int width, int maxHeight, QDate monday, double ratio) {
    QString title = "Semaine du lundi " + monday.toString("dd/MM/yyyy") + " au samedi " + monday.addDays(5).toString("dd/MM/yyyy");

    QFont title_font = painter->font();
    title_font.setBold(true);
    if(QFontMetrics(title_font).lineSpacing() > maxHeight || QFontMetrics(title_font).width(title) > width*ratio) {
        while(QFontMetrics(title_font).lineSpacing() > maxHeight || QFontMetrics(title_font).width(title) > width*ratio)
            title_font.setPointSize(title_font.pointSize() - 1);
    }

    if(QFontMetrics(title_font).lineSpacing() < maxHeight && QFontMetrics(title_font).width(title) < width*ratio) {
        while(QFontMetrics(title_font).lineSpacing() <= maxHeight && QFontMetrics(title_font).width(title) <= width*ratio)
            title_font.setPointSize(title_font.pointSize() + 1);
        title_font.setPointSize(title_font.pointSize() - 1);
    }

    //Draw the title
    painter->setFont(title_font);
    QFontMetrics font = painter->fontMetrics();
    painter->drawText((width - font.width(title)) / 2, (maxHeight-font.height())/2 + font.ascent() + font.leading()/2, title);
    return true;
}

QString PrintPDF::displayStudent(Student* s, int maxWidth, QFont font) {
    QString standardText = " " + s->getName() + " " + s->getFirst_name() + " ";
    QString text = standardText;
    while(QFontMetrics(font).width(text) > maxWidth && standardText != "") {
        standardText.resize(standardText.length()-1);
        text = standardText + "... ";
    }
    return text;
}

void PrintPDF::displaySPARK(QPainter* painter, int width, int height, int maxHeight, QFont normal) {
    QString text = "Généré par S.P.A.R.K.";

    // Choose the font
    QFont signature_font = QFont(normal);
    while(QFontMetrics(signature_font).lineSpacing() > maxHeight ||
          QFontMetrics(signature_font).width(text) > width ||
          signature_font.pointSize() > 13)
        signature_font.setPointSize(signature_font.pointSize() - 1);

    painter->setFont(signature_font);
    QFontMetrics font = painter->fontMetrics();
    painter->drawText(width - font.width(text), height + (maxHeight-font.height())/2 + font.ascent() + font.leading()/2, text);
}
