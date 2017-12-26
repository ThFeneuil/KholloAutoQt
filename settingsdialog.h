#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>
#include <QMessageBox>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

public slots:
    bool save();
    bool update();

private:
    Ui::SettingsDialog *ui;
};

class Preferences {
public:
    enum FormatPDF { StudentsDays, StudentsSubjects };

    Preferences();
    ~Preferences();

    bool initFile();
    bool read();
    bool write();

    // Setters
    bool setDir(QString dir);
    bool setFormatPDF(FormatPDF format);
    bool setServerDefault(bool isDefault);
    bool setServerScript(QString script);
    bool setServerPassword(QString password);

    // Getters
    QString dir();
    FormatPDF formatPDF();
    bool serverDefault();
    QString serverScript();
    QString serverPassword();

private:
    QString m_dir;
    FormatPDF m_formatPDF;
    bool m_serverDefault;
    QString m_serverScript;
    QString m_serverPassword;
};

#endif // SETTINGSDIALOG_H
