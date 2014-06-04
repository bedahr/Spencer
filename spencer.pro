TEMPLATE = app
TARGET = Spencer
DEPENDPATH += .

BBSIMONECOMMONIPATH = /home/bedahr/ownCloud/simon/src/tools/Simone/BlackBerry/BBSimoneShared
BBSIMONECOMMONPATH  = /home/bedahr/ownCloud/simon/src/tools/Simone/BlackBerry/BBSimoneShared
QT += quick xml multimedia

OTHER_FILES += \
        src/qml/spencer/main.qml \
        src/qml/spencer/EditableComboBox.qml \
        src/qml/spencer/ComboBox.qml \
        src/qml/spencer/Button.qml \
        src/qml/spencer/LineEdit.qml \
        src/qml/spencer/CheckBox.qml \
        src/qml/spencer/AnimatedItem.qml \
        src/qml/spencer/ProgressBar.qml \
        src/qml/spencer/Dialog.qml \
        src/qml/spencer/ConnectionSettings.qml \
        src/qml/spencer/ActiveConsole.qml \
        src/qml/spencer/Offer.qml \
        src/qml/spencer/AttributeDelegate.qml \
        src/qml/spencer/AnimatedText.qml \
        src/qml/spencer/Avatar.qml

SOURCES += src/main.cpp \
        src/spencer.cpp \
        src/spencersettings.cpp \
        src/recommender/numericalattribute.cpp \
        src/recommender/stringattribute.cpp \
        src/recommender/relationship.cpp \
        src/recommender/critique.cpp \
        src/recommender/offer.cpp \
        src/recommender/valueattribute.cpp \
        src/recommender/critiquerecommender.cpp \
        src/recommender/attributemodel.cpp \
        src/recommender/attributefactory.cpp \
        src/recommender/compoundattribute.cpp \
        src/recommender/attribute.cpp \
        src/dialogmanager/dialogmanager.cpp \
        src/ui/spencerview.cpp \
        src/ui/qmlspencerview.cpp \
        src/ui/avatar/avatar.cpp \
        src/ui/avatar/avatartask.cpp \
        src/ui/avatar/player.cpp

HEADERS += \
        src/spencer.h \
        src/spencersettings.h \
        src/recommender/attribute.h \
        src/recommender/numericalattribute.h \
        src/recommender/stringattribute.h \
        src/recommender/relationship.h \
        src/recommender/critique.h \
        src/recommender/offer.h \
        src/recommender/valueattribute.h \
        src/recommender/critiquerecommender.h \
        src/recommender/attributemodel.h \
        src/recommender/attributefactory.h \
        src/recommender/compoundattribute.h \
        src/recommender/attributecreators.h \
        src/dialogmanager/dialogmanager.h \
        src/ui/spencerview.h \
        src/ui/qmlspencerview.h \ 
        src/ui/avatar/avatar.h \
        src/ui/avatar/avatartask.h \
        src/ui/avatar/player.h


INCLUDEPATH += ../BBSimoneShared $${BBSIMONECOMMONIPATH} src/
LIBS += -L$${BBSIMONECOMMONPATH} -lSimoneShared
