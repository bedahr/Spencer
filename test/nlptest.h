#ifndef NLPTEST_H
#define NLPTEST_H
#include <QtTest/QtTest>
#include "../src/nlu/nlu.h"

class NLPTest : public QObject
{
  Q_OBJECT
private slots:
  void initTestCase();
  void testNLP();
  void testNLP_data();
  
private:
  NLU nlu;
};

#endif
