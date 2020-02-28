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

    bool isDefault = pref.serverDefault();

    ui->radioButton_default->setChecked(isDefault);
    ui->radioButton_other->setChecked(! isDefault);

    ui->lineEdit_script->setEnabled(! isDefault);
    ui->lineEdit_password->setEnabled(! isDefault);
    ui->lineEdit_script->setText(pref.serverScript());
    ui->lineEdit_password->setText(pref.serverPassword());

    connect(ui->pushButton_valid, SIGNAL(clicked(bool)), this, SLOT(save()));
    connect(ui->radioButton_other, SIGNAL(toggled(bool)), this, SLOT(update()));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

bool SettingsDialog::update() {
    ui->lineEdit_script->setEnabled(ui->radioButton_other->isChecked());
    ui->lineEdit_password->setEnabled(ui->radioButton_other->isChecked());
    return true;
}

bool SettingsDialog::save() {
    Preferences pref;
    pref.setFormatPDF((Preferences::FormatPDF) ui->comboBox_formats->currentData().toInt());
    pref.setServerDefault(ui->radioButton_default->isChecked());
    pref.setServerScript(ui->lineEdit_script->text());
    pref.setServerPassword(ui->lineEdit_password->text());
    accept();
    return true;
}

Preferences::Preferences() {
    m_dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    m_serverDefault = true;
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
            } else if(data == "Default server:") {
                data = in.readLine();
                m_serverDefault = (data == "YES");
            } else if(data == "Server script:") {
                m_serverScript = in.readLine();
            } else if(data == "Server password:") {
                m_serverPassword = in.readLine();
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
        out << "Default server:\n" << (m_serverDefault ? "YES" : "NO") << "\n\n";
        out << "Server script:\n" << m_serverScript << "\n\n";
        out << "Server password:\n" << m_serverPassword << "\n\n";

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

bool Preferences::setServerDefault(bool isDefault) {
    read();
    m_serverDefault = isDefault;
    return write();
}

bool Preferences::setServerScript(QString script) {
    read();
    m_serverScript = script;
    return write();
}

bool Preferences::setServerPassword(QString password) {
    read();
    m_serverPassword = password;
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

bool Preferences::serverDefault() {
    read();
    return m_serverDefault;
}

QString Preferences::serverScript() {
    read();
    return m_serverScript;
}

QString Preferences::serverPassword() {
    read();
    return m_serverPassword;
}
