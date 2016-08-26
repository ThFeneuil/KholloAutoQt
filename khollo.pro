QT += widgets sql concurrent

FORMS += \
    mainwindow.ui \
    managers/studentsmanager.ui \
    managers/updatestudentdialog.ui \
    managers/groupsmanager.ui \
    managers/subjectsmanager.ui \
    managers/kholleursmanager.ui \
    managers/updateteacherdialog.ui \
    managers/updatekholleurdialog.ui \
    managers/usersgroupsmanager.ui \
	managers/coursesmanager.ui \
    managers/eventsmanager.ui \
    managers/updateeventdialog.ui \
    managers/selectgroupsdialog.ui \
	managers/kholloscopewizard.ui \
    managers/KholloscopeWizardPages/subjectspage.ui \
    managers/KholloscopeWizardPages/userspage.ui \
    managers/KholloscopeWizardPages/generatepage.ui \
    interface/interfacedialog.ui \
    interface/interfacetab.ui \
    interface/introinterface.ui \
    managers/timeslotsmanager.ui \
    managers/introtimeslots.ui \
    managers/copytimeslots.ui \
    aboutitdialog.ui \
    contactdialog.ui \
    reviewdialog.ui

HEADERS += \
    mainwindow.h \
    storedData/student.h \
    storedData/group.h \
    managers/studentsmanager.h \
    managers/updatestudentdialog.h \
    managers/groupsmanager.h \
    managers/subjectsmanager.h \
    storedData/subject.h \
    managers/kholleursmanager.h \
    storedData/kholleur.h \
    storedData/teacher.h \
    managers/updateteacherdialog.h \
    managers/updatekholleurdialog.h \
    managers/usersgroupsmanager.h \
	managers/coursesmanager.h \
    storedData/course.h \
    managers/eventsmanager.h \
    storedData/event.h \
    managers/updateeventdialog.h \
    managers/selectgroupsdialog.h \
	managers/kholloscopewizard.h \
    managers/KholloscopeWizardPages/subjectspage.h \
    managers/KholloscopeWizardPages/userspage.h \
    managers/KholloscopeWizardPages/generatepage.h \
    storedData/timeslot.h \
    storedData/kholle.h \
    interface/interfacedialog.h \
    interface/khollotable.h \
    interface/interfacetab.h \
    interface/introinterface.h \
    managers/timeslotsmanager.h \
    managers/introtimeslots.h \
    managers/copytimeslots.h \
    database.h \
    aboutitdialog.h \
    contactdialog.h \
    reviewdialog.h \
    printpdf.h

SOURCES += \
    mainwindow.cpp \
    main.cpp \
    storedData/student.cpp \
    storedData/group.cpp \
    managers/studentsmanager.cpp \
    managers/updatestudentdialog.cpp \
    managers/groupsmanager.cpp \
    managers/subjectsmanager.cpp \
    storedData/subject.cpp \
    managers/kholleursmanager.cpp \
    storedData/kholleur.cpp \
    storedData/teacher.cpp \
    managers/updateteacherdialog.cpp \
    managers/updatekholleurdialog.cpp \
    managers/usersgroupsmanager.cpp \
	managers/coursesmanager.cpp \
    storedData/course.cpp \
    managers/eventsmanager.cpp \
    storedData/event.cpp \
    managers/updateeventdialog.cpp \
    managers/selectgroupsdialog.cpp \
	managers/kholloscopewizard.cpp \
    managers/KholloscopeWizardPages/subjectspage.cpp \
    managers/KholloscopeWizardPages/userspage.cpp \
    managers/KholloscopeWizardPages/generatepage.cpp \
    storedData/timeslot.cpp \
    storedData/kholle.cpp \
    interface/interfacedialog.cpp \
    interface/khollotable.cpp \
    interface/interfacetab.cpp \
    interface/introinterface.cpp \
	managers/timeslotsmanager.cpp \
    managers/introtimeslots.cpp \
    managers/copytimeslots.cpp \
    database.cpp \
    aboutitdialog.cpp \
    contactdialog.cpp \
    reviewdialog.cpp \
    printpdf.cpp
DISTFILES +=

RESOURCES += \
    khollo.qrc

VERSION = 1.0
QMAKE_TARGET_PRODUCT = "KholloAuto\0"

win32:RC_ICONS += "images/iconApp.ico"
macx {
    QMAKE_INFO_PLIST = Info.plist
    ICON = "images/iconApp.icns"
}
