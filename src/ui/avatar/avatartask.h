#ifndef AVATARTASK_H
#define AVATARTASK_H

#include <QObject>
#include <QString>

class AvatarTask {
public:
    enum Expression {
        Idle=0,
        Intro=1,
        AskUseCase=2,
        AskMostImportantAttribute=3,
        AskPerformanceImportant=4,
        AskEasyTransportImportant=5,
        AskPriceImporant=6,
        PresentItem=7,
        Misunderstood=8,
        Custom=9
    };


    AvatarTask(Expression e) : m_e(e) {}
    AvatarTask(Expression e, const QString& text, const QString& description) :
        m_e(e), m_text(text), m_description(description) {}

    QString text() const { return m_text; }
    Expression expression() const { return m_e; }
    QString description() const {
        switch (m_e) {
        case Idle:
            return QString();
        case Intro:
            return QString();
        case AskUseCase:
            return QObject::tr("Wie wollen Sie den Laptop nutzen?");
        case AskMostImportantAttribute:
            return QObject::tr("Gibt es etwas, was Ihnen besonders wichtig ist?");
        case AskPerformanceImportant:
            return QObject::tr("Muss der Laptop schnell sein?");
        case AskPriceImporant:
            return QObject::tr("Muss der Laptop günstig sein?");
        case AskEasyTransportImportant:
            return QObject::tr("Muss der Laptop transportabel sein?");
        case PresentItem:
            return QObject::tr("Vorschlag:");
        case Misunderstood:
            return QObject::tr("Wie bitte?");
        default:
            return m_description;
        }
    }

private:
    Expression m_e;
    QString m_text;
    QString m_description;
};

#endif // AVATARTASK_H
