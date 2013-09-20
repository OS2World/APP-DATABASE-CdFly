FORMS += findwidget.ui \
         settingwidget.ui \
         newdiskwidget.ui \
         progresswidget.ui \
         aboutwidget.ui
HEADERS += cdfly.h \
           find.h \
           mimeicon.h \
           settings.h \
           cdsql.h \
           qsqlitems.h \
           newdisk.h \
           progress.h
SOURCES += cdfly.cpp \
           find.cpp \
           main.cpp \
           mimeicon.cpp \
           settings.cpp \
           cdsql.cpp \
           newdisk.cpp \
           progress.cpp
TRANSLATIONS += cdfly_it_IT.ts \
                cdfly_fr_FR.ts \
                cdfly_pl_PL.ts \
                cdfly_he_IL.ts \
                cdfly_ru_RU.ts \
                cdfly_tr_TR.ts
RESOURCES = mainres.qrc
QT += sql
RC_FILE = app.rc
TEMPLATE = app
