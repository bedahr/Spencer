#ifndef SPENCERADAPTER_H
#define SPENCERADAPTER_H

#include <QtDBus/QDBusAbstractAdaptor>
class Spencer;

class SpencerAdapter : public QDBusAbstractAdaptor
{
    Q_OBJECT

public:
    SpencerAdapter(Spencer* spencer);

public slots:
    void simulateInput(const QString& input);

private:
    Spencer* m_spencer;
};

#endif // SPENCERADAPTER_H
