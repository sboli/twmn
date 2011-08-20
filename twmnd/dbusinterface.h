#ifndef DBUSINTERFACE_H
#define DBUSINTERFACE_H

#include <QObject>
#include <dbus/dbus.h>
#include "message.h"

class DBusInterface : public QObject
{
    Q_OBJECT
public:
    DBusInterface(QObject* parent = 0);


private slots:
    void            check();

private:
    void            GetServerInformation(DBusMessage* msg);
    void            GetCapabilities(DBusMessage* msg);
    void            CloseNotification(DBusMessage* msg);
    void            Notify(DBusMessage* msg);

signals:
    void            messageReceived(const Message& msg);

private:
    DBusConnection* dbus_conn;
    dbus_uint32_t   serial;
};

#endif // DBUSINTERFACE_H
