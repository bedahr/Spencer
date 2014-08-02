#include "nlptest.h"
#include "nlu/statement.h"
#include "domainbase/offer.h"
#include "domainbase/stringattribute.h"
#include "domainbase/numericalattribute.h"
#include "domainbase/attributefactory.h"
#include "domainbase/aspectfactory.h"
#include <QFile>

typedef QList<Statement*> StatementList;
Q_DECLARE_METATYPE(StatementList)

const QString dbPath = "/home/bedahr/ownCloud/Daten/TU/Master/Spencer/db/";
 
void NLPTest::initTestCase()
{
  qRegisterMetaType<StatementList>("StatementList");

  if (!AttributeFactory::getInstance()->parseStructure(dbPath + "structure.xml"))
      QFAIL("Failed to initialize attribute factory component; Aborting");
  if (!AspectFactory::getInstance()->parseStructure(dbPath + "sentiment.xml"))
      QFAIL("Failed to initialize aspect factory component; Aborting");

  if (!nlu.setupLanguage(dbPath + "nlp.xml"))
    QFAIL("Failed to initialize NLU component; Aborting");
}
void NLPTest::testNLP()
{
  QFETCH(QString, input);
  QFETCH(Offer*, currentOffer);
  QFETCH(StatementList, output);

  QList<Statement*> statements = nlu.interpret(currentOffer, input);

  qDebug() << "Input: " << input;
  foreach (Statement *s, statements)
      qDebug() << "  Statement: " << s->toString();

  //QEXPECT_FAIL("compound1", "Nested compound critiques are not supported", Continue);
  QCOMPARE(statements, output);

  qDebug() << "================";
}
void NLPTest::testNLP_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<Offer*>("currentOffer");
    QTest::addColumn<StatementList>("output");

    Offer *o = new Offer(QSharedPointer<Attribute>(new StringAttribute(true, true, "Testlaptop")),
                         QSharedPointer<Attribute>(new NumericalAttribute(true, true, 599, QString(), NumericalAttribute::Min)),
                         QSharedPointer<Attribute>(new NumericalAttribute(true, true, 50, QString(), NumericalAttribute::Max)),
                         1, QStringList(), RecordMap(), SentimentMap());
    QTest::newRow("blank") << QString::fromUtf8("+uhm+") << o << StatementList();
}

QTEST_MAIN(NLPTest)
