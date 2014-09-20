#ifndef DBUSINTERFACE_H
#define DBUSINTERFACE_H

#include <QObject>
#include <QtDBus/QtDBus>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusServiceWatcher>
#include "message.h"

class DBusInterface : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Notifications")
public:
    DBusInterface(QObject* parent = 0);

public slots:
    void GetCapabilities(QStringList& capabilities);
    void CloseNotification(unsigned int id);
    void GetServerInformation(
        QString& name,
        QString& vendor,
        QString& version,
        QString& specVersion
    );
    void Notify(
        const QString& appName,
        unsigned int id,
        const QString& icon,
        const QString& summary,
        const QString& body,
        const QStringList& actions,
        const QVariantMap& hints,
        int timeout,
        unsigned int& return_id
    );

signals:
    void messageReceived(const Message& msg);

private:
    QString serviceName;
    QDBusConnection dbus_conn;
    QDBusServiceWatcher serviceWatcher;
    unsigned int lastNid;
};

#endif // DBUSINTERFACE_H
