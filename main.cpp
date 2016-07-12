#include <QApplication>
#include <QPushButton>
#include <QHBoxLayout>
#include "mainwindow.h"
#include <QMessageBox>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include "storedData/student.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QString locale = QLocale::system().name().section('_', 0, 0);
    QTranslator translator;
    translator.load(QString("qt_") + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&translator);

    MainWindow fenetre;
    fenetre.show();

    return app.exec();
}
