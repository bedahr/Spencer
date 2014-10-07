#ifndef DIALOGSTRATEGY_H
#define DIALOGSTRATEGY_H

#include <QFlags>

namespace DialogStrategy {
    enum DialogState {
        NullState = 0,
        InitState = 1,
        AskForUseCase = 2,
        AskForImportantAttribute = 4,
        AskForPerformanceImportant = 8,
        AskForPriceImportant = 16,
        AskForPortabilityImportant = 32,
        Recommendation = 64,
        MisunderstoodInput = 128,
        AskForHelp = 256,
        FinalState = 512
    };
    Q_DECLARE_FLAGS(DialogStates, DialogState)
    Q_DECLARE_OPERATORS_FOR_FLAGS(DialogStates)
}

#endif // DIALOGSTRATEGY_H
