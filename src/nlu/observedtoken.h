#ifndef OBSERVEDTOKEN_H
#define OBSERVEDTOKEN_H

class Token;
#include <QString>
#include <QDebug>

class ObservedToken
{
public:
    ObservedToken(const Token* observation, const QString& userInput,
                  float polarity, const QString& capturedValue,
                  int position, int startWordIndex) :
        m_observation(observation), m_userInput(userInput),
        m_polarity(polarity), m_capturedValue(capturedValue),
        m_position(position), m_startWordIndex(startWordIndex)
    {
    }
    int matchLength() const {
        return m_userInput.length();
    }
    int position() const {
        return m_position;
    }
    int startWordIndex() const {
        return m_startWordIndex;
    }
    int endWordIndex() const {
        return m_startWordIndex + m_userInput.count(" ") + 1;
    }

    const Token* token() const {
        return m_observation;
    }
    QString capturedValue() const {
        return m_capturedValue;
    }
    float polarity() const {
        return m_polarity;
    }

    /// Returns number of intermediate words between two observed tokens
    int distance(const ObservedToken* other) const {
        if (overlaps(other))
            return 0;
        if (other->startWordIndex() < m_startWordIndex) // other is to the left
            return startWordIndex() - other->endWordIndex();
        if (other->startWordIndex() > m_startWordIndex) // other is to the right
            return other->startWordIndex() - endWordIndex();

        Q_ASSERT_X(false, "ObservedToken::distance", "Overlap is obviously broken");
        return -1;
    }

    bool overlaps(const ObservedToken* other) const {
        int positionDiff = other->position() - m_position;
        // case 1:
        // this:  |------------|
        // Other:       |------------|
        // positionDiff = 6
        // Case 2:
        // this:        |------------|
        // other: |------|
        // positionDiff = -6
        bool out = ((positionDiff >= 0) && (positionDiff < matchLength())) ||
                ((positionDiff < 0) && ((-positionDiff) < other->matchLength()));
        //qDebug() << toString() << " overlaps " << other->toString() << "?: " << out;
        return out;
    }

    QString toString() const;

private:
    const Token* m_observation;
    QString m_userInput;
    float m_polarity;
    QString m_capturedValue;
    int m_position;
    int m_startWordIndex;
};
#endif // OBSERVEDTOKEN_H
