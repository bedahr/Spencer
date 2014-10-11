#include "spenceradapter.h"
#include "spencer.h"
#include <recognitionresult.h>
#include <QList>

SpencerAdapter::SpencerAdapter(Spencer *spencer) :
    QDBusAbstractAdaptor(spencer), m_spencer(spencer)
{}

void SpencerAdapter::simulateInput(const QString& input)
{
    QList<float> confidence;
    foreach (const QString& r, input.split(' '))
        confidence << 1;

    m_spencer->userInput(RecognitionResultList() << RecognitionResult(input, QString(), QString(), 0, confidence));
}
