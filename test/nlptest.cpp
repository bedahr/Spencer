#include "nlptest.h"
#include <QFile>

const QString pathToProcessedRecognitionResults = "recognitionTestResults.txt";
 
void NLPTest::initTestCase()
{
  if (!spencer.init())
    QFAIL("Failed to initialize Spencer; Aborting");
}
void NLPTest::testNLP()
{
  QFETCH(QString, input);
  QFETCH(QString, output);

  spencer.reset();
  QStringList resultCritiques = spencer.critique(input).trimmed().split('\n', QString::SkipEmptyParts);
  resultCritiques.removeDuplicates();
  QString resultCritique = resultCritiques.join("\n");
  QEXPECT_FAIL("compound1", "Nested compound critiques are not supported", Continue);
  QCOMPARE(resultCritique, output);
}
void NLPTest::testNLP_data()
{
    QSKIP("Just Speech tests", SkipAll);
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");

    QTest::newRow("resolution1") << QString::fromUtf8("MEHR ALS 20 MEGAPIXEL") << QString::fromUtf8("Auflösung (Megapixel) mehr als 20");
    QTest::newRow("resolution2") << QString::fromUtf8("MEHR AUFLÖSUNG") << QString::fromUtf8("Auflösung (Megapixel) mehr als 16,1");
    QTest::newRow("resolution3") << QString::fromUtf8("GRÖSSERE AUFLÖSUNG") << QString::fromUtf8("Auflösung (Megapixel) mehr als 16,1");
    QTest::newRow("resolution4") << QString::fromUtf8("HÖHERE AUFLÖSUNG") << QString::fromUtf8("Auflösung (Megapixel) mehr als 16,1");
    QTest::newRow("resolution5") << QString::fromUtf8("MEHR MEGAPIXEL") << QString::fromUtf8("Auflösung (Megapixel) mehr als 16,1");

    QTest::newRow("zoom1") << QString::fromUtf8("DEFINITIV MEHR OPTISCHER ZOOM") << QString::fromUtf8("Optischer Zoom (X) mehr als 4");
    QTest::newRow("zoom2") << QString::fromUtf8("GIBT ES DIESES MODELL AUCH MIT EINEM HÖHEREN ZOOM") << QString::fromUtf8("Optischer Zoom (X) mehr als 4");
    QTest::newRow("zoom3") << QString::fromUtf8("GIBTS AN MIT MEHR ZOOM") << QString::fromUtf8("Optischer Zoom (X) mehr als 4");
    QTest::newRow("zoom4") << QString::fromUtf8("MEHR OPTISCHER ZOOM") << QString::fromUtf8("Optischer Zoom (X) mehr als 4");
    QTest::newRow("zoom5") << QString::fromUtf8("MEHR ZOOM") << QString::fromUtf8("Optischer Zoom (X) mehr als 4");
    QTest::newRow("zoom6") << QString::fromUtf8("MINDESTENS 10 FACHEN OPTISCHEN ZOOM") << QString::fromUtf8("Optischer Zoom (X) mehr als 10");
    QTest::newRow("zoom7") << QString::fromUtf8("OPTISCHER ZOOM HÖHER ALS 10") << QString::fromUtf8("Optischer Zoom (X) mehr als 10");
    QTest::newRow("zoom8") << QString::fromUtf8("OPTISCHER ZOOM 20") << QString::fromUtf8("Optischer Zoom (X) gleich 20");
    QTest::newRow("zoom9") << QString::fromUtf8("OPTISCHER ZOOM HÖHER") << QString::fromUtf8("Optischer Zoom (X) mehr als 4");
    QTest::newRow("zoom10") << QString::fromUtf8("WENIGER ALS 10 FACHEN OPTISCHEN ZOOM") << QString::fromUtf8("Optischer Zoom (X) weniger als 10");
    QTest::newRow("zoom11") << QString::fromUtf8("WENIGER OPTISCHER ZOOM") << QString::fromUtf8("Optischer Zoom (X) weniger als 4");
    QTest::newRow("zoom12") << QString::fromUtf8("WENIGER ZOOM") << QString::fromUtf8("Optischer Zoom (X) weniger als 4");


    QTest::newRow("commands1") << QString::fromUtf8("ALTERNATIVE") << QString::fromUtf8("Modell ungleich Digital Ixus 125 HS");
    QTest::newRow("commands2") << QString::fromUtf8("ANDERES MODELL") << QString::fromUtf8("Modell ungleich Digital Ixus 125 HS");
    QTest::newRow("commands3") << QString::fromUtf8("HABEN SIE NOCH 1 ANDERES MODELL ZUR AUSWAHL") << QString::fromUtf8("Modell ungleich Digital Ixus 125 HS");
    QTest::newRow("commands4") << QString::fromUtf8("GIBT ES DIESE KAMERA AUCH IN EINER ANDEREN AUSFÜHRUNG") << QString::fromUtf8("Modell ungleich Digital Ixus 125 HS");

    QTest::newRow("commands5") << QString::fromUtf8("DIE GEFÄLLT MIR LEIDER NICHT HABE ICH NOCH EINE ANDERE MÖGLICHKEIT")
                               << QString::fromUtf8("Modell ungleich Digital Ixus 125 HS");
    QTest::newRow("commands6") << QString::fromUtf8("WEITER")
                               << QString::fromUtf8("Modell ungleich Digital Ixus 125 HS");
    QTest::newRow("commands7") << QString::fromUtf8("DIESE KAMERA GEFÄLLT MIR OPTISCH NICHT GIBT ES DIE AUCH IN EINEM ANDEREN DESIGN ODER IN EINER ANDEREN BAUWEISE")
                               << QString::fromUtf8("Modell ungleich Digital Ixus 125 HS");
    QTest::newRow("commands8") << QString::fromUtf8("1 KOMPLETT ANDERES MODELL") << QString::fromUtf8("Modell signifikant ungleich Digital Ixus 125 HS");
    QTest::newRow("commands9") << QString::fromUtf8("GIBTS DA AUCH 1 ANDERES MODELL") << QString::fromUtf8("Modell ungleich Digital Ixus 125 HS");
    QTest::newRow("commands10") << QString::fromUtf8("GIBTS DIE VIELLEICHT AUCH IN EINEM ANDEREN DESIGN") << QString::fromUtf8("Modell ungleich Digital Ixus 125 HS");


    QTest::newRow("back1") << QString::fromUtf8("DIE LETZTE KAMERA HAT MIR BESSER GEFALLEN") << QString::fromUtf8("Zurück");
    QTest::newRow("back2") << QString::fromUtf8("ZURÜCK") << QString::fromUtf8("Zurück");
    QTest::newRow("back3") << QString::fromUtf8("VERGISS DAS") << QString::fromUtf8("Zurück");
    QTest::newRow("back4") << QString::fromUtf8("KÖNNEN WIR NOCH EINMAL ZURÜCK") << QString::fromUtf8("Zurück");
    QTest::newRow("back5") << QString::fromUtf8("LÖSCHEN") << QString::fromUtf8("Zurück");


    QTest::newRow("price1") << QString::fromUtf8("BILLIGER") << QString::fromUtf8("Preis (€) weniger als 136,1");
    QTest::newRow("price2") << QString::fromUtf8("NIEDRIGERER PREIS") << QString::fromUtf8("Preis (€) weniger als 136,1");

    QTest::newRow("price3") << QString::fromUtf8("DER PREIS IST MIR ZU HOCH") << QString::fromUtf8("Preis (€) weniger als 136,1");
    QTest::newRow("price4") << QString::fromUtf8("MAXIMAL 70 EURO") << QString::fromUtf8("Preis (€) weniger als 70");
    QTest::newRow("price5") << QString::fromUtf8("DER PREIS IST MIT 136 EURO ZU HOCH GIBT ES EINEN GÜNSTIGEREN PREIS")
                            << QString::fromUtf8("Preis (€) weniger als 136");
    QTest::newRow("price6") << QString::fromUtf8("NOCH BILLIGER") << QString::fromUtf8("Preis (€) weniger als 136,1");
    QTest::newRow("price7") << QString::fromUtf8("MEHR SOLLS KOSTEN") << QString::fromUtf8("Preis (€) mehr als 136,1");
    QTest::newRow("price8") << QString::fromUtf8("DER PREIS MIT 136 EURO ERSCHEINT MIR ZU HOCH") << QString::fromUtf8("Preis (€) weniger als 136");
    QTest::newRow("price9") << QString::fromUtf8("DER PREIS MIT 136 EURO IST MIR VIEL ZU HOCH") << QString::fromUtf8("Preis (€) signifikant weniger als 136");
    QTest::newRow("price10") << QString::fromUtf8("1 WENIG HOCHWERTIGER") << QString::fromUtf8("Preis (€) marginal mehr als 136,1");
    QTest::newRow("price11") << QString::fromUtf8("HABT IHR EINE ETWAS TEURERE") << QString::fromUtf8("Preis (€) marginal mehr als 136,1");
    QTest::newRow("price12") << QString::fromUtf8("HÖHERER PREIS") << QString::fromUtf8("Preis (€) mehr als 136,1");
    QTest::newRow("price13") << QString::fromUtf8("IHR KÖNNTS DEN PREIS ERHÖHEN") << QString::fromUtf8("Preis (€) mehr als 136,1");
    QTest::newRow("price14") << QString::fromUtf8("KANN RUHIG 1 BISSCHEN MEHR KOSTEN") << QString::fromUtf8("Preis (€) marginal mehr als 136,1");

    QTest::newRow("price15") << QString::fromUtf8("PREIS HÖHER") << QString::fromUtf8("Preis (€) mehr als 136,1");
    QTest::newRow("price16") << QString::fromUtf8("TEURER") << QString::fromUtf8("Preis (€) mehr als 136,1");
    QTest::newRow("price17") << QString::fromUtf8("VIEL TEURER") << QString::fromUtf8("Preis (€) signifikant mehr als 136,1");
    QTest::newRow("price18") << QString::fromUtf8("VIEL BILLIGER") << QString::fromUtf8("Preis (€) signifikant weniger als 136,1");
    QTest::newRow("price19") << QString::fromUtf8("VIEL HOCHWERTIGER") << QString::fromUtf8("Preis (€) signifikant mehr als 136,1");
    QTest::newRow("price20") << QString::fromUtf8("PREIS SIGNIFIKANT TEURER") << QString::fromUtf8("Preis (€) signifikant mehr als 136,1");
    QTest::newRow("price21") << QString::fromUtf8("PREIS HÖHER ALS 100 EURO") << QString::fromUtf8("Preis (€) mehr als 100");


    QTest::newRow("weight1") << QString::fromUtf8("GERINGERES GEWICHT") << QString::fromUtf8("Gewicht (Gramm) weniger als 135");
    QTest::newRow("weight2") << QString::fromUtf8("MEHR ALS 500 GRAMM WÄREN OK") << QString::fromUtf8("Gewicht (Gramm) mehr als 500");
    QTest::newRow("weight3") << QString::fromUtf8("GIBT ES EINE KAMERA MIT WENIGER GEWICHT") << QString::fromUtf8("Gewicht (Gramm) weniger als 135");
    QTest::newRow("weight4") << QString::fromUtf8("SCHWERER") << QString::fromUtf8("Gewicht (Gramm) mehr als 135");
    QTest::newRow("weight5") << QString::fromUtf8("VIEL LEICHTER") << QString::fromUtf8("Gewicht (Gramm) signifikant weniger als 135");



    QTest::newRow("size1") << QString::fromUtf8("GRÖSSER") << QString::fromUtf8("Größe (BxHxT) mehr als 93x57x20");
    QTest::newRow("size2") << QString::fromUtf8("VIEL GRÖSSER") << QString::fromUtf8("Größe (BxHxT) signifikant mehr als 93x57x20");
    QTest::newRow("size3") << QString::fromUtf8("SIE KANN RUHIG ETWAS DICKER SEIN") << QString::fromUtf8("Größe (BxHxT) marginal mehr als 93x57x20");
    QTest::newRow("size4") << QString::fromUtf8("ICH HÄTTE GERNE EINE KLEINERE KAMERA BITTE") << QString::fromUtf8("Größe (BxHxT) weniger als 93x57x20");
    QTest::newRow("size5") << QString::fromUtf8("KLEINER") << QString::fromUtf8("Größe (BxHxT) weniger als 93x57x20");
    QTest::newRow("size6") << QString::fromUtf8("ICH HÄTTE GERN EINE KLEINERE KAMERA") << QString::fromUtf8("Größe (BxHxT) weniger als 93x57x20");
    QTest::newRow("size7") << QString::fromUtf8("JA GUTEN TAG ICH SUCHE EINE KAMERA DIE KOMPAKT IST UND ICH AUCH ÜBERALL HIN MITNEHMEN KANN "
                                                "DIE ICH AUCH IN DEN IN DIE HOSENTASCHE BEKOMME ODER IN MEINE SAKKOTASCHE DAS ICH DIE JEDER "
                                                "ZEIT ZUR VERFÜGUNG HAB") << QString::fromUtf8("Größe (BxHxT) weniger als 93x57x20");
    QTest::newRow("size8") << QString::fromUtf8("GRÖSSERE") << QString::fromUtf8("Größe (BxHxT) mehr als 93x57x20");


    QTest::newRow("manufacturer1") << QString::fromUtf8("ABER TROTZDEM VON Canon") << QString::fromUtf8("Hersteller gleich Canon");
    QTest::newRow("manufacturer2") << QString::fromUtf8("HERSTELLER Canon") << QString::fromUtf8("Hersteller gleich Canon");
    QTest::newRow("manufacturer3") << QString::fromUtf8("HERSTELLER Nikon") << QString::fromUtf8("Hersteller gleich Nikon");
    QTest::newRow("manufacturer4") << QString::fromUtf8("VON Nikon") << QString::fromUtf8("Hersteller gleich Nikon");
    QTest::newRow("manufacturer5") << QString::fromUtf8("HERSTELLER Sony") << QString::fromUtf8("Hersteller gleich Sony");
    QTest::newRow("manufacturer6") << QString::fromUtf8("ICH HÄTTE GERN EINE Sony") << QString::fromUtf8("Hersteller gleich Sony");
    QTest::newRow("manufacturer7") << QString::fromUtf8("Canon") << QString::fromUtf8("Hersteller gleich Canon");
    QTest::newRow("manufacturer8") << QString::fromUtf8("VON Canon") << QString::fromUtf8("Hersteller gleich Canon");
    QTest::newRow("manufacturer9") << QString::fromUtf8("VON Canon BITTE") << QString::fromUtf8("Hersteller gleich Canon");
    QTest::newRow("manufacturer10") << QString::fromUtf8("ALSO ICH HAB A Canon") << QString::fromUtf8("Hersteller gleich Canon");
    QTest::newRow("manufacturer11") << QString::fromUtf8("NEIN KEINE BenQ BITTE") << QString::fromUtf8("Hersteller nicht gleich BenQ");

    QTest::newRow("manufacturer12") << QString::fromUtf8("NICHT Casio") << QString::fromUtf8("Hersteller nicht gleich Casio");
    QTest::newRow("manufacturer13") << QString::fromUtf8("NICHT Olympus") << QString::fromUtf8("Hersteller nicht gleich Olympus");
    QTest::newRow("manufacturer14") << QString::fromUtf8("NICHT VON Canon") << QString::fromUtf8("Hersteller nicht gleich Canon");
    QTest::newRow("manufacturer15") << QString::fromUtf8("Nikon") << QString::fromUtf8("Hersteller gleich Nikon");
    QTest::newRow("manufacturer16") << QString::fromUtf8("Rollei") << QString::fromUtf8("Hersteller gleich Rollei");

    QTest::newRow("manufacturer17") << QString::fromUtf8("ANDERER HERSTELLER") << QString::fromUtf8("Hersteller ungleich Canon");
    QTest::newRow("manufacturer18") << QString::fromUtf8("NEIN NICHT VON Canon") << QString::fromUtf8("Hersteller nicht gleich Canon");
    QTest::newRow("manufacturer19") << QString::fromUtf8("GIBT ES DIESE KAMERA AUCH VON EINEM ANDEREN HERSTELLER") << QString::fromUtf8("Hersteller ungleich Canon");

    QTest::newRow("compound1") << QString::fromUtf8("AHA FÜR MICH IST DIE AUFLÖSUNG DER VORGESCHLAGENEN KAMERA VON Canon ZU WENIG")
                                   << QString::fromUtf8("Hersteller gleich Canon\nAuflösung (Megapixel) mehr als 16,1");
    QTest::newRow("compound2") << QString::fromUtf8("DIE KAMERA SOLLTE ETWAS TEURER SEIN UND EINE HÖHERE AUFLÖSUNG BESITZEN")
                                   << QString::fromUtf8("Preis (€) marginal mehr als 136,1\nAuflösung (Megapixel) mehr als 16,1");
    QTest::newRow("compound3") << QString::fromUtf8("EINE Canon DIE 1 BISSCHEN KOMPAKTER IST")
                                   << QString::fromUtf8("Hersteller gleich Canon\nGröße (BxHxT) marginal weniger als 93x57x20");
    QTest::newRow("compound4") << QString::fromUtf8("JA Nikon IST GUT ABER EINE HÖHERE AUFLÖSUNG WÄRE GUT UND AH 1 BISSCHEN STABILER VIELLEICHT")
                                   << QString::fromUtf8("Hersteller gleich Nikon\nAuflösung (Megapixel) mehr als 16,1\nGröße (BxHxT) marginal mehr als 93x57x20");
    QTest::newRow("compound5") << QString::fromUtf8("MEHR OPTISCHER ZOOM UND LEICHTER")
                                   << QString::fromUtf8("Optischer Zoom (X) mehr als 4\nGewicht (Gramm) weniger als 135");

    QTest::newRow("garbage1") << QString::fromUtf8("DANKE") << QString();
    QTest::newRow("garbage2") << QString::fromUtf8("DIE IST SCHÖN") << QString();
    
    QTest::newRow("inch1") << QString::fromUtf8("MINDESTENS 1 , 2 ZOLL") << QString::fromUtf8("Sensor Größe (Zoll) mehr als 1,2");
    QTest::newRow("inch2") << QString::fromUtf8("MAXIMAL 0 . 8 ZOLL") << QString::fromUtf8("Sensor Größe (Zoll) weniger als 0,8");
    QTest::newRow("inch3") << QString::fromUtf8("1 ZOLL") << QString::fromUtf8("Sensor Größe (Zoll) gleich 1");
    
//     QTest::newRow("study2") << QString::fromUtf8("+BREADTH ÄH ICH HÄTTE GERN EINE KAMERA MIT 14 MEGAPIXEL AUFLÖSUNG +BREADTH "
// 						 "10 FACH OPTISCHEN ZOOM SIE SOLL NICHT MEHR WIEGEN ALS 200 GRAMM DIE GRÖSSE "
// 						 "IS MIR EGAL UND DER SENSOR SOLLTE 0 , 5 ZOLL GROSS SEIN +BREADTH AM LIEBSTEN "
// 						 "WÄR MIR BEI BEIM SENSOR TYP CCD") 
// 			    << QString::fromUtf8("Auflösung (Megapixel) gleich 14\nOptischer Zoom (X) gleich 10\nGewicht "
// 						 "(Gramm) nicht mehr als 200\nSensor Größe (Zoll) gleich 0,5\nSensor Typ gleich CCD");
    QTest::newRow("study5") << QString::fromUtf8("MIT 136 EURO ZU HOCH ") 
			    << QString::fromUtf8("Preis (€) weniger als 136");
    QTest::newRow("study1") << QString::fromUtf8("EINE KAMERA MIT 14 MEGAPIXEL AUFLÖSUNG KAMERA") 
			    << QString::fromUtf8("Auflösung (Megapixel) gleich 14");
    QTest::newRow("study3") << QString::fromUtf8("+BREADTH 14 MEGAPIXEL AUFLÖSUNG +BREADTH "
						 "10 FACH OPTISCHEN ZOOM") 
			    << QString::fromUtf8("Auflösung (Megapixel) gleich 14\nOptischer Zoom (X) gleich 10");
    QTest::newRow("study4") << QString::fromUtf8("10 FACH OPTISCHEN ZOOM") 
			    << QString::fromUtf8("Optischer Zoom (X) gleich 10");

    /*
     * Stuff that people said that goes beyond the scope:
     *
     * DAS MODELL POWERSHOT G15 IN SCHWARZ GIBT ES DAS MODELL AUCH IN DER FARBE SILBER
     * GIBT ES DAS MODELL AUCH IN SILBER
     * GIBT ES DIE KAMERA AUCH IN BLAU
     * GIBT ES DIE MODELLFARBE AUCH IN SILBER
     * IN BLAU
     * MODELL IXUS
     *
     * Stuff that is just icky:
     * ICH MÖCHTE BITTE NOCH EINMAL DAS LETZTE BILD SEHEN
     */
}


void NLPTest::testRecognition()
{
//   QSKIP("Just NLP tests for now", SkipAll);
  QFETCH(QString, prompt);
  QFETCH(QString, result);

  spencer.reset();
  QStringList promptCritiques = spencer.critique(prompt).trimmed().split('\n', QString::SkipEmptyParts);
  spencer.reset();
  QStringList resultCritiques = spencer.critique(result).trimmed().split('\n', QString::SkipEmptyParts);
  promptCritiques.removeDuplicates();
  resultCritiques.removeDuplicates();
  QString promptCritique = promptCritiques.join("\n");
  QString resultCritique = resultCritiques.join("\n");
  QCOMPARE(resultCritique, promptCritique);
}
void NLPTest::testRecognition_data()
{
    QTest::addColumn<QString>("prompt");
    QTest::addColumn<QString>("result");

    QFile f(pathToProcessedRecognitionResults);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open path to processed recognition results: " << pathToProcessedRecognitionResults;
        return;
    }
    int i = 0;
    while (!f.atEnd()) {
	++i;
        QByteArray fileName = f.readLine().trimmed();
        QString prompt = QString::fromUtf8(f.readLine().trimmed());
        QString result = QString::fromUtf8(f.readLine().trimmed());
        QTest::newRow(fileName.constData()) << prompt << result;
    }
    qDebug() << "Testing " << i << " samples";
}

QTEST_MAIN(NLPTest)
