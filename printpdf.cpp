#include "printpdf.h"

PrintPDF::PrintPDF()
{

}

int PrintPDF::longestUser(QFontMetrics font, QList<Student *> *students) {
    int max = 0;
    int i;
    for(i = 0; i < students->length(); i++) {
        int width = font.width(students->at(i)->getName() + "  " + students->at(i)->getFirst_name());
        if(width > max) {
            max = width;
        }
    }
    return max;
}

int PrintPDF::longestKholleur(QFontMetrics font, QMap<int, Kholleur*> *kholleurs) {
    int max = 0;
    foreach(Kholleur* k, *kholleurs) {
        int width = font.width("20:00  " + k->getName());
        if(width > max) {
            max = width;
        }
    }
    return max;
}

void PrintPDF::printKholles(QList<Student *> *students, QMap<int, Kholleur *> *kholleurs, QMap<int, Timeslot *> *timeslots, QDate monday_date, QMap<int, Kholle *> *kholloscope) {
    //Try to load directory preferences
    QString pref_path;
    QFile read(QDir::currentPath() + QDir::separator() + "dir_preferences.pref");
    if(read.exists() && read.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&read);
        pref_path = in.readLine();
    }

    if(pref_path == "" || !QDir(pref_path).exists())
        pref_path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    //Get file name
    QString filename = QFileDialog::getSaveFileName(NULL, "Enregistrer sous...",
                                                    pref_path + QDir::separator() + "Kholloscope_" + monday_date.toString("yyyyMMdd"),  "PDF (*.pdf)");
    //QMessageBox::information(this, "OK", filename);

    if(filename == "")
        return;

    //Save directory in preferences
    QString dirpath = QFileInfo(filename).absoluteDir().absolutePath();
    QFile pref_file(QDir::currentPath() + QDir::separator() + "dir_preferences.pref");
    if(pref_file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate)){
        QTextStream out(&pref_file);
        out << dirpath;
    }

    //Create the PDF Writer
    QPdfWriter writer(filename);
    writer.setPageSize(QPdfWriter::A3);
    writer.setPageOrientation(QPageLayout::Landscape);
    writer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    QPainter painter;

    if(!painter.begin(&writer)) {
        QMessageBox::critical(NULL, "Erreur", "Erreur lors de l'écriture du fichier. Le fichier est peut-être ouvert ?");
        return;
    }

    //Get size in default units
    int width = writer.width();
    int height = writer.height();

    //Paint here
    painter.setPen(QPen(QBrush(Qt::black), 5));

    //Calculate line height and cell width

    //Number of rows
    int num_rows = students->length() + 3;
    int row_height = height / num_rows;

    //Create two fonts -> one for the normal text, one for title
    QFont normal_font = painter.font();
    if(QFontMetrics(normal_font).lineSpacing() > row_height) {
        while(QFontMetrics(normal_font).lineSpacing() > row_height)
            normal_font.setPointSize(normal_font.pointSize() - 1);
    }
    if(QFontMetrics(normal_font).lineSpacing() < row_height) {
        while(QFontMetrics(normal_font).lineSpacing() <= row_height)
            normal_font.setPointSize(normal_font.pointSize() + 1);
        normal_font.setPointSize(normal_font.pointSize() - 1);
    }

    QFont title_font = painter.font();
    if(QFontMetrics(title_font).lineSpacing() > 2*row_height) {
        while(QFontMetrics(title_font).lineSpacing() > 2*row_height)
            title_font.setPointSize(title_font.pointSize() - 1);
    }
    if(QFontMetrics(title_font).lineSpacing() < 2*row_height) {
        while(QFontMetrics(title_font).lineSpacing() <= 2*row_height)
            title_font.setPointSize(title_font.pointSize() + 1);
        title_font.setPointSize(title_font.pointSize() - 1);
    }

    //Draw the title
    QString title = "Semaine du lundi " + monday_date.toString("dd/MM/yyyy") + " au samedi " + monday_date.addDays(5).toString("dd/MM/yyyy");
    painter.setFont(title_font);
    QFontMetrics font = painter.fontMetrics();
    painter.drawText((width - font.width(title)) / 2, font.ascent() + font.leading()/2, title);

    //Get font metrics
    painter.setFont(normal_font);
    font = painter.fontMetrics();

    //Name length (length of longest name) => also the width of 1st column
    int name_width = longestUser(font, students);
    int cell_width = (width - name_width) / 6;
    int kholleur_width = longestKholleur(font, kholleurs);

    //Draw the grid
    painter.drawLine(0, 2*row_height, 0, height);
    painter.drawLine(name_width, 2*row_height, name_width, height);
    painter.drawLine(name_width + cell_width, 2*row_height, name_width + cell_width, height);
    painter.drawLine(name_width + 2*cell_width, 2*row_height, name_width + 2*cell_width, height);
    painter.drawLine(name_width + 3*cell_width, 2*row_height, name_width + 3*cell_width, height);
    painter.drawLine(name_width + 4*cell_width, 2*row_height, name_width + 4*cell_width, height);
    painter.drawLine(name_width + 5*cell_width, 2*row_height, name_width + 5*cell_width, height);
    painter.drawLine(name_width + 6*cell_width, 2*row_height, name_width + 6*cell_width, height);

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
        painter.drawText(0, (3+i)*row_height + font.ascent() + font.leading()/2, students->at(i)->getName() + " " + students->at(i)->getFirst_name());
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
            int k_width = kholleur_width; //Copy kholleur_width

            if(list->length() * kholleur_width > cell_width) {
                QFont small_font(normal_font);
                while(longestKholleur(QFontMetrics(small_font), kholleurs) * list->length() > cell_width)
                    small_font.setPointSize(small_font.pointSize() - 1);

                painter.setFont(small_font);
                k_width = longestKholleur(QFontMetrics(small_font), kholleurs); //Update if necessary
            }

            for(k = 0; k < list->length(); k++) { //Paint
                painter.drawText(name_width + j*cell_width + k*k_width,
                                 (3+i)*row_height + font.ascent() + font.leading()/2,
                                 timeslots->value(list->at(k)->getId_timeslots())->getTime_start().toString("HH:mm") + " "
                                 + kholleurs->value(timeslots->value(list->at(k)->getId_timeslots())->getId_kholleurs())->getName());
            }

            painter.setFont(normal_font); //Put back normal font
        }
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
}
