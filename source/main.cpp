#include <QApplication>
#include <QPushButton>
#include <QHBoxLayout>
#include "mainwindow.h"
#include <QMessageBox>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QString locale = QLocale::system().name().section('_', 0, 0);
    QTranslator translator;
    translator.load(QString("qt_") + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&translator);

    MainWindow fenetre;
    // Pour détecter si on a ouvert directement un fichier sur Mac
    QApplication::instance()->installEventFilter(&fenetre);
    fenetre.show();

    return app.exec();
}
