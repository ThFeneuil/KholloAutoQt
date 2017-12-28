QT += widgets sql concurrent network

FORMS += \
    mainwindow.ui \
    managers/studentsmanager.ui \
    managers/updatestudentdialog.ui \
    managers/groupsmanager.ui \
    managers/subjectsmanager.ui \
    managers/kholleursmanager.ui \
    managers/updatekholleurdialog.ui \
    managers/usersgroupsmanager.ui \
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
    reviewdialog.ui \
    managers/lastchanges.ui \
    managers/updatetimeslotdialog.ui \
    managers/groupsswappingsmanager.ui \
    notepad.ui \
    settingsdialog.ui \
    managers/updatesubjectdialog.ui \
    interface/interfacedialog_mac.ui \
    interface/interfacetab_mac.ui \
    managers/timetablemanager.ui \
    managers/mergekholleursmanager.ui \
    managers/tribesmanager.ui \
    managers/KholloscopeWizardPages/generationwaitingdialog.ui

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
    managers/updatekholleurdialog.h \
    managers/usersgroupsmanager.h \
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
    reviewdialog.h \
    printpdf.h \
    managers/lastchanges.h \
    managers/updatetimeslotdialog.h \
    kscopemanager.h \
    managers/groupsswappingsmanager.h \
    notepad.h \
    settingsdialog.h \
    managers/KholloscopeWizardPages/utilities.h \
    interface/interfaceactionsrecord.h \
    managers/updatesubjectdialog.h \
    managers/timetablemanager.h \
    managers/timetable.h \
    onlinedatabase.h \
    managers/mergekholleursmanager.h \
    managers/tribesmanager.h \
    managers/studentsimportmanager.h \
    managers/KholloscopeWizardPages/generationmethod.h \
    managers/KholloscopeWizardPages/fivewavesmethod.h \
    managers/KholloscopeWizardPages/lpmethod.h \
    managers/KholloscopeWizardPages/generationwaitingdialog.h


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
    managers/updatekholleurdialog.cpp \
    managers/usersgroupsmanager.cpp \
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
    reviewdialog.cpp \
    printpdf.cpp \
    managers/lastchanges.cpp \
    managers/updatetimeslotdialog.cpp \
    kscopemanager.cpp \
    managers/groupsswappingsmanager.cpp \
    notepad.cpp \
    settingsdialog.cpp \
    managers/KholloscopeWizardPages/utilities.cpp \
    interface/interfaceactionsrecord.cpp \
    managers/updatesubjectdialog.cpp \
    managers/timetablemanager.cpp \
    managers/timetable.cpp \
    onlinedatabase.cpp \
    managers/mergekholleursmanager.cpp \
    managers/tribesmanager.cpp \
    managers/studentsimportmanager.cpp \
    managers/KholloscopeWizardPages/generationmethod.cpp \
    managers/KholloscopeWizardPages/fivewavesmethod.cpp \
    managers/KholloscopeWizardPages/lpmethod.cpp \
    managers/KholloscopeWizardPages/generationwaitingdialog.cpp
	
DISTFILES +=

RESOURCES += \
    khollo.qrc

VERSION = 1.2
DEFINES += "APP_VERSION='$$VERSION'"
QMAKE_TARGET_PRODUCT = "SPARK Kholloscope\0"

win32:RC_ICONS += "images/iconApp.ico"
macx {
    QMAKE_INFO_PLIST = Info.plist
    ICON = "images/iconApp.icns"
}

win32: LIBS += -L$$PWD/GLPK/ -lglpk_4_64

INCLUDEPATH += $$PWD/GLPK
DEPENDPATH += $$PWD/GLPK
