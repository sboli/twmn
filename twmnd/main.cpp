#include <QtGui/QApplication>
#include <QTextCodec>
#include <QLocale>
#include <QLibraryInfo>
#include <QTranslator>
#include "widget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setQuitOnLastWindowClosed(true);
    QApplication::setApplicationName("twmn");
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    const QString locale = QLocale::system().name().section('_', 0, 0);
    QTranslator translator;
    translator.load("qt_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&translator);
    Widget w;
    return a.exec();
}
