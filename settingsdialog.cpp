#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    ui->comboBox_formats->addItem("élèves-jours", Preferences::StudentsDays);
    ui->comboBox_formats->addItem("élèves-matières", Preferences::StudentsSubjects);

    Preferences pref;
    Preferences::FormatPDF format = pref.formatPDF();
    if(format == Preferences::StudentsDays)
        ui->comboBox_formats->setCurrentText("élèves-jours");
    else if(format == Preferences::StudentsSubjects)
        ui->comboBox_formats->setCurrentText("élèves-matières");

    connect(ui->pushButton_valid, SIGNAL(clicked(bool)), this, SLOT(save()));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

bool SettingsDialog::save() {
    Preferences pref;
    pref.setFormatPDF((Preferences::FormatPDF) ui->comboBox_formats->currentData().toInt());
    accept();
    return true;
}

Preferences::Preferences() {
    read();
}

Preferences::~Preferences() {

}

bool Preferences::read() {
    QFile read(QCoreApplication::applicationDirPath() + QDir::separator() + "preferences.pref");
    if(read.exists() && read.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&read);
        while(! in.atEnd()) {
            QString data = in.readLine();
            if(data == "Dir path:") {
                m_dir = in.readLine();
                if(m_dir == "" || !QDir(m_dir).exists())
                    m_dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
            } else if(data == "PDF Format:") {
                data = in.readLine();
                if(data == "StudentsDays")
                    m_formatPDF = StudentsDays;
                else if(data == "StudentsSubjects")
                    m_formatPDF = StudentsSubjects;
            }
        }
    }

    return true;
}

bool Preferences::write() {
    //QString dirpath = QFileInfo(filename).absoluteDir().absolutePath();
    QFile write(QCoreApplication::applicationDirPath() + QDir::separator() + "preferences.pref");
    if(write.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate)){
        QTextStream out(&write);
        out << "Dir path:\n" << m_dir << "\n\n";
        out << "PDF Format:\n";
        if(m_formatPDF == StudentsDays)
            out << "StudentsDays\n\n";
        else
            out << "StudentsSubjects\n\n";
        return true;
    }
    return false;
}

// Setters
bool Preferences::setDir(QString dir) {
    read();
    m_dir = dir;
    return write();
}

bool Preferences::setFormatPDF(FormatPDF format) {
    read();
    m_formatPDF = format;
    return write();
}

// Getters
QString Preferences::dir() {
    read();
    return m_dir;
}

Preferences::FormatPDF Preferences::formatPDF() {
    read();
    return m_formatPDF;
}
