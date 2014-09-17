#ifndef DIALOGMANAGER_H
#define DIALOGMANAGER_H

class Critique;
class Aspect;
class MentionedAspect;

class DialogManager
{
public:
    virtual bool undo() = 0;
    virtual bool constrain(Critique* c) = 0;
    virtual bool applyAspect(MentionedAspect* c) = 0;
    virtual bool accept(double strength) = 0;
    virtual bool requestForHelp(double strength) = 0;
};

#endif // DIALOGMANAGER_H
