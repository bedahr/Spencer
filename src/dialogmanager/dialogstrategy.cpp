#include "dialogstrategy.h"

QString DialogStrategy::dialogStrategyString(DialogStrategy::DialogState s)
{
    switch (s) {
    case NullState:
        return "NullState";
    case InitState:
        return "InitState";
    case AskForUseCase:
        return "AskForUseCase";
    case AskForImportantAttribute:
        return "AskForImportantAttribute";
    case AskForPerformanceImportant:
        return "AskForPerformanceImportant";
    case AskForPriceImportant:
        return "AskForPriceImportant";
    case AskForPortabilityImportant:
        return "AskForPortabilityImportant";
    case Recommendation:
        return "Recommendation";
    case MisunderstoodInput:
        return "MisunderstoodInput";
    case AskForHelp:
        return "AskForHelp";
    case FinalState:
        return "FinalState";
    }
    return "Unknown";
}
