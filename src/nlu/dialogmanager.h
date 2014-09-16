#ifndef DIALOGMANAGER_H
#define DIALOGMANAGER_H

class Critique;
class Aspect;

class DialogManager
{
public:
    virtual void yes() = 0;
    virtual void no() = 0;
    virtual void constrain(Critique* c) = 0;
    virtual void applyAspect(Aspect* c) = 0;

};

#endif // DIALOGMANAGER_H
