#ifndef NLPTEST_H
#define NLPTEST_H
#include <QtTest/QtTest>
#include "../src/spencer.h"

class NLPTest : public QObject
{
  Q_OBJECT
private slots:
  void initTestCase();
  void testNLP();
  void testNLP_data();
  void testRecognition();
  void testRecognition_data();
  
private:
  Spencer spencer;
};

#endif
