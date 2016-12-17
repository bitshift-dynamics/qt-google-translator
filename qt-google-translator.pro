TARGET = qt-google-translator
TEMPLATE = app

QT += widgets network

SOURCES +=\
    gtranslator.cc \
    main.cc \
    mainwindow.cc

HEADERS  += mainwindow.h \
    apitoken.h \
    gtranslator.h

FORMS    += mainwindow.ui
