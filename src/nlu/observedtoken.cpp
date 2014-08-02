#include "observedtoken.h"
#include "token.h"

QString ObservedToken::toString() const
{
   return QString("%1: %2 (%3)").arg(m_position).arg(m_observation->toString()).arg(m_userInput);
}
