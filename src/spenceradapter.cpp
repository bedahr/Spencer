#include "spenceradapter.h"
#include "spencer.h"

SpencerAdapter::SpencerAdapter(Spencer *spencer) :
    QDBusAbstractAdaptor(spencer), m_spencer(spencer)
{}

void SpencerAdapter::simulateInput(const QString& input)
{
    m_spencer->userInput(input);
}
