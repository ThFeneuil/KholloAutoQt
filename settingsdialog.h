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

    // Getters
    QString dir();
    FormatPDF formatPDF();

private:
    QString m_dir;
    FormatPDF m_formatPDF;
};

#endif // SETTINGSDIALOG_H
