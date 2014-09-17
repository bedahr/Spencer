#ifndef DIALOGSTRATEGY_H
#define DIALOGSTRATEGY_H

namespace DialogStrategy {
    enum DialogState {
        InitState,
        AskForUseCase,
        AskForImportantAttribute,
        AskForPerformanceImportant,
        AskForPriceImportant,
        AskForPortabilityImportant,
        Recommendation,
        MisunderstoodInput,
        FinalState
    };
}

#endif // DIALOGSTRATEGY_H
