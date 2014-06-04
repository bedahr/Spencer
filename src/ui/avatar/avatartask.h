#ifndef AVATARTASK_H
#define AVATARTASK_H

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
    Expression e;
    QString text;
    AvatarTask(Expression e) : e(e) {}
    AvatarTask(Expression e, const QString& text) : e(e), text(text) {}
};

#endif // AVATARTASK_H
