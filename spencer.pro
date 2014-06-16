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
        src/domainbase/numericalattribute.cpp \
        src/domainbase/stringattribute.cpp \
        src/domainbase/relationship.cpp \
        src/domainbase/offer.cpp \
        src/domainbase/valueattribute.cpp \
        src/domainbase/attributemodel.cpp \
        src/domainbase/attributefactory.cpp \
        src/domainbase/compoundattribute.cpp \
        src/domainbase/attribute.cpp \
        src/recommender/critiquerecommender.cpp \
        src/recommender/critique.cpp \
        src/dialogmanager/dialogmanager.cpp \
        src/ui/spencerview.cpp \
        src/ui/qmlspencerview.cpp \
        src/ui/avatar/avatar.cpp \
        src/ui/avatar/avatartask.cpp \
        src/ui/avatar/player.cpp \
        src/nlu/nlu.cpp \
        src/nlu/statement.cpp \
        src/nlu/commandstatement.cpp \
        src/nlu/constraintstatement.cpp

HEADERS += \
        src/spencer.h \
        src/spencersettings.h \
        src/domainbase/attribute.h \
        src/domainbase/numericalattribute.h \
        src/domainbase/stringattribute.h \
        src/domainbase/relationship.h \
        src/domainbase/offer.h \
        src/domainbase/valueattribute.h \
        src/domainbase/attributemodel.h \
        src/domainbase/attributefactory.h \
        src/domainbase/compoundattribute.h \
        src/domainbase/attributecreators.h \
        src/recommender/critique.h \
        src/recommender/critiquerecommender.h \
        src/dialogmanager/dialogmanager.h \
        src/ui/spencerview.h \
        src/ui/qmlspencerview.h \
        src/ui/avatar/avatar.h \
        src/ui/avatar/avatartask.h \
        src/ui/avatar/player.h \
        src/nlu/nlu.h \
        src/nlu/statement.h \
        src/nlu/commandstatement.h \
        src/nlu/constraintstatement.h


INCLUDEPATH += ../BBSimoneShared $${BBSIMONECOMMONIPATH} src/
LIBS += -L$${BBSIMONECOMMONPATH} -lSimoneShared
