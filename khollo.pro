QT += widgets sql

FORMS += \
    mainwindow.ui \
    managers/studentsmanager.ui \
    managers/updatestudentdialog.ui \
    managers/groupsmanager.ui \
    managers/subjectsmanager.ui \
    managers/kholleursmanager.ui \
    managers/usersgroupsmanager.ui

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
    managers/usersgroupsmanager.h

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
    managers/usersgroupsmanager.cpp
