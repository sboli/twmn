#include <QtGui/QApplication>
#include <QTextCodec>
#include <QLocale>
#include <QLibraryInfo>
#include <QTranslator>
#include "widget.h"

void logOutput(QtMsgType type, const char *msg)
{
#ifdef QT_NO_DEBUG_OUTPUT
    Q_UNUSED(type);
    Q_UNUSED(msg);
    return;
#else
    std::cout << "[" << QTime::currentTime().toString("hh:mm:ss").toStdString() << "]";
    switch (type){
    case QtDebugMsg:
        std::cout << " " << msg;
        break;
    case QtWarningMsg:
        std::cout << "[warning] " << msg;
        break;
    case QtCriticalMsg:
        std::cout << "[critical] " << msg;
        break;
    case QtFatalMsg:
        std::cout << "[fatal] " << msg;
        break;
    }
    std::cout << std::endl;
#endif
}

int main(int argc, char *argv[])
{
    qInstallMsgHandler(logOutput);
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
