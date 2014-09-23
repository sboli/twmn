#include <QApplication>
#include <QTextCodec>
#include <QtGlobal>
#include <QLocale>
#include <QLibraryInfo>
#include <QTranslator>
#include "widget.h"
#include "xcb/xcb.h"
#include "dbusinterface.h"

void logOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
#ifdef QT_NO_DEBUG_OUTPUT
    Q_UNUSED(type);
    Q_UNUSED(msg);
    Q_UNUSED(context);
    return;
#else
    std::cout << "[" << QTime::currentTime().toString("hh:mm:ss").toStdString() << "]";
    switch (type){
    case QtDebugMsg:
        std::cout << " " << msg;
        break;
    case QtWarningMsg:
        std::cout << "[warning][" << context.file << '-' << context.line << "] " << msg.toStdString();
        break;
    case QtCriticalMsg:
        std::cout << "[critical][" << context.file << '-' << context.line << "] " << msg.toStdString();
        break;
    case QtFatalMsg:
        std::cout << "[fatal][" << context.file << '-' << context.line << "] " <<  msg.toStdString();
        break;
    }
    std::cout << std::endl;
#endif
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(logOutput);
    QApplication a(argc, argv);
    QApplication::setQuitOnLastWindowClosed(true);
    QApplication::setApplicationName("twmn");
    QPalette p = a.palette();
    p.setBrush(QPalette::Link, QBrush(QColor("black")));
    a.setPalette(p);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    const char* wname = "twmn";
    if (argc > 1) {
      wname = argv[1];
    }
    DBusInterface dbus(&a);
    Widget w(wname);
    w.connectToDBus(dbus);
    return a.exec();
}
