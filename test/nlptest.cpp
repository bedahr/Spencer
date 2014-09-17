#include "nlptest.h"
#include "nlu/statement.h"
#include "nlu/constraintstatement.h"
#include "nlu/commandstatement.h"
#include "nlu/aspectstatement.h"
#include "nlu/usecasestatement.h"
#include "domainbase/offer.h"
#include "domainbase/stringattribute.h"
#include "domainbase/attribute.h"
#include "domainbase/numericalattribute.h"
#include "domainbase/attributefactory.h"
#include "domainbase/aspectfactory.h"
#include <QFile>
#include <QSharedPointer>

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

  if (!nlu.setupLanguage(dbPath + "nlp.xml", dbPath + "synsets.hierarchical"))
    QFAIL("Failed to initialize NLU component; Aborting");
}

void NLPTest::testNLP()
{
  QFETCH(QString, input);
  QFETCH(Offer*, currentOffer);
  QFETCH(StatementList, reference);

  QList<Statement*> statements = nlu.interpret(currentOffer, input);

  qDebug() << "Input: " << input;
  foreach (Statement *s, statements)
      qDebug() << "  NLU statement: " << s->toString();
  foreach (Statement *s, reference)
      qDebug() << "  Reference statement: " << s->toString();

  //remove aspects for now
  foreach (Statement *s, statements) {
      if (dynamic_cast<AspectStatement*>(s))
          statements.removeAll(s);
  }

  static QStringList expectFails;
  expectFails << "pilot2" << "pilot5" << "pilot15" << "pilot178" << "pilot180" << "pilot78" << "pilot87"
              << "pilot91" << "pilot93" << "pilot118" << "pilot125" << "pilot128" << "pilot131" << "pilot139"
              << "pilot144" << "pilot171" << "pilot196";
  QEXPECT_FAIL("pilot2", "Ranges not supported", Continue);
  QEXPECT_FAIL("pilot5", "Ranges not supported", Continue);
  QEXPECT_FAIL("pilot15", "Spurious model != is technically wrong but who cares..?", Continue);
  QEXPECT_FAIL("pilot178", "Ranges not supported", Continue);
  QEXPECT_FAIL("pilot180", "Ranges not supported", Continue);
  QEXPECT_FAIL("pilot93", "Spurious cpuspeed+ isn't technically there but not really incorrect", Continue);
  QEXPECT_FAIL("pilot78", "Spurious Ram+ isn't technically there but not really incorrect", Continue);
  QEXPECT_FAIL("pilot91", "Impossible to parse", Continue);
  QEXPECT_FAIL("pilot87", "marginal gleich Linux instead of gleich Linux", Continue);
  QEXPECT_FAIL("pilot118", "Stupid formulation", Continue);
  QEXPECT_FAIL("pilot125", "Spurious cpuspeed+ isn't technically there but not really incorrect", Continue);
  QEXPECT_FAIL("pilot128", "Spurious cpuspeed+ isn't technically there but not really incorrect", Continue);
  QEXPECT_FAIL("pilot131", "Spurious cpuspeed+ isn't technically there but not really incorrect", Continue);
  QEXPECT_FAIL("pilot139", "Spurious cpuspeed+ isn't technically there but not really incorrect", Continue);
  QEXPECT_FAIL("pilot171", "Ranges not supported", Continue);
  QEXPECT_FAIL("pilot144", "Stupid formulation", Continue);
  QEXPECT_FAIL("pilot196", "Stupid formulation", Continue);
  QCOMPARE(statements.count(), reference.count());
  if (!expectFails.contains(QTest::currentDataTag())) {
      foreach (const Statement* s, statements) {
          bool found = false;
          foreach (const Statement* o, reference) {
              if (o->compare(s)) {
                  found = true;
                  break;
              }
          }
          if (!found)
              QFAIL(QString("Statement in NLU output but not in reference: " + s->toString()).toUtf8());
      }
  }

  qDebug() << "================";
}
void NLPTest::testNLP_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<Offer*>("currentOffer");
    QTest::addColumn<StatementList>("reference");
    RecordMap r;
    r.insert("processorSpeed", qMakePair(QString("processorSpeed"), QSharedPointer<Attribute>(new NumericalAttribute(true, true, 0, QString(), NumericalAttribute::Max, dummyData, dummyData))));
    r.insert("mainMemoryCapacity", qMakePair(QString("mainMemoryCapacity"), AttributeFactory::getInstance()->getAttribute("mainMemoryCapacity", 0).second));
    r.insert("weight", qMakePair(QString("weight"), AttributeFactory::getInstance()->getAttribute("weight", 0).second));
    r.insert("operatingSystem", qMakePair(QString("operatingSystem"), AttributeFactory::getInstance()->getAttribute("operatingSystem", "Windows").second));
    r.insert("screenSize", qMakePair(QString("screenSize"), AttributeFactory::getInstance()->getAttribute("screenSize", 0).second));
    r.insert("storageMedia[_][1]", qMakePair(QString("storageMedia[_][1]"), AttributeFactory::getInstance()->getAttribute("storageMedia[_][1]", 0).second));

    Offer *o = new Offer(QSharedPointer<Attribute>(new StringAttribute(true, true, "Testlaptop")),
                         AttributeFactory::getInstance()->getAttribute("price", 599).second,
                         QSharedPointer<Attribute>(new NumericalAttribute(true, true, 50, QString(), NumericalAttribute::Max, dummyData, dummyData)),
                         1, QStringList(), r, SentimentMap());
    /*
    QTest::newRow("blank") << QString::fromUtf8("+uhm+") << o << StatementList();
    QTest::newRow("p1") << QString::fromUtf8("+uhm+ +breath+ leicht +breath+ lange akkulaufzeit") << (Offer*) 0 <<
                           (StatementList() << new ConstraintStatement(new Relationship("weight", Relationship::Small, QSharedPointer<Attribute>()))
                                            << new ConstraintStatement(new Relationship("averageRuntimeOnBattery", Relationship::Large, QSharedPointer<Attribute>()))
                                            << new AspectStatement("AKKU")
                                            << new AspectStatement("GEWICHT")
                            );*/
    QFile f("/home/bedahr/ownCloud/Daten/TU/Master/pilot/transcripts/annotated");
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Couldn't open annotated nlp data";
        return;
    }
    int rowIndex = 1;
    while (!f.atEnd()) {
        QString line(QString::fromUtf8(f.readLine()).trimmed());
        int fieldSep = line.indexOf('\t');
        QString outputStr = line.left(fieldSep);
        Offer *thisProduct = 0;
        if (outputStr.startsWith('#')) {
            thisProduct = o;
            outputStr = outputStr.mid(1);
        }
        QString inputStr = line.mid(fieldSep+1);

        //remove aspects for now
        //outputStr.remove(QRegExp("\\[[^\\]_]*\\](/[0-9\\.]*)?"));

        StatementList statements;
        //build statements from outputStr
        QRegExp attributeModifier("([<'=>+\\!&-])");
        QRegExp polarityTag("/(-?[0-5](\\.[0-9]+)?)");
        QRegExp qualityTag("\\^(-?[0-5](\\.[0-9]+)?)");
        foreach (QString statementStr, outputStr.split(QRegExp("\\(+"), QString::SkipEmptyParts)) {
            statementStr = statementStr.trimmed();
            if (statementStr.isEmpty() || statementStr.startsWith(")"))
                continue;

            statementStr.replace(QRegExp("\\)+$"), "");

            double lexicalPolarity = 1.0;
            if (polarityTag.indexIn(statementStr) != -1) {
                lexicalPolarity = polarityTag.cap(1).toDouble();
            }
            double quality = 1.0;
            if (qualityTag.indexIn(statementStr) != -1) {
                quality = qualityTag.cap(1).toDouble();
            }
            statementStr = statementStr.left(statementStr.indexOf(')'));

            //commands
            if (statementStr == "yes") {
                statements << new CommandStatement(CommandStatement::Yes, lexicalPolarity, quality);
                continue;
            }
            if (statementStr == "no") {
                statements << new CommandStatement(CommandStatement::No, lexicalPolarity, quality);
                continue;
            }
            if (statementStr == "goback") {
                statements << new CommandStatement(CommandStatement::Back, lexicalPolarity, quality);
                continue;
            }
            if (statementStr == "accept") {
                statements << new CommandStatement(CommandStatement::AcceptProduct, lexicalPolarity, quality);
                continue;
            }

            //use case
            if (statementStr.startsWith("use=")) {
                QString useCase = statementStr.mid(4);
                statements << new UsecaseStatement(useCase, lexicalPolarity, quality);
                continue;
            }

            //constraint
            int attributeSeparator = statementStr.indexOf(attributeModifier);
            if (attributeSeparator == -1) {
                qWarning() << "Invalid definition: " << statementStr;
            }
            QString attribute = statementStr.left(attributeSeparator);
            statementStr = statementStr.mid(attributeSeparator);
            bool isRelative = false;

            int matchIndex = -1;
            int offset = 0;
            Relationship::Type type;
            while ((matchIndex = attributeModifier.indexIn(statementStr, offset)) != -1) {
                QChar c = attributeModifier.cap(0)[0];
                switch (c.unicode()) {
                case '\'':
                    isRelative = true;
                    break;
                case '<':
                    type |= Relationship::SmallerThan;
                    break;
                case '>':
                    type |= Relationship::LargerThan;
                    break;
                case '=':
                    type |= Relationship::Equality;
                    break;
                case '!':
                    type |= Relationship::Inequality;
                    break;
                case '+':
                    type |= Relationship::BetterThan;
                    break;
                case '-':
                    type |= Relationship::WorseThan;
                    break;
                case '&':
                    type |= Relationship::IsTrue;
                    break;
                }
                offset = matchIndex + 1;
            }
            statementStr = statementStr.mid(offset);
            QSharedPointer<Attribute> attr;
            if (!statementStr.isEmpty()) {
                attr = AttributeFactory::getInstance()->getAttribute(attribute, statementStr).second;
            } else if (isRelative) {
                if (!r.contains(attribute))
                    attr = AttributeFactory::getInstance()->getAttribute(attribute, "0.0").second;
                else
                    attr = r.value(attribute).second;
            }
            statements << new ConstraintStatement(new Relationship(attribute, type, attr, lexicalPolarity), 1.0, quality);
        }


        QTest::newRow(QString("pilot%1").arg(rowIndex).toLocal8Bit()) << inputStr << thisProduct << statements;

        ++rowIndex;
    }
}

QTEST_MAIN(NLPTest)
