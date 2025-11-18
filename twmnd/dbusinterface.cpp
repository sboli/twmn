#include "dbusinterface.h"
#include <QDebug>
#include <QTimer>

DBusInterface::DBusInterface(QObject* parent) :
    QDBusAbstractAdaptor(parent),
    serviceName("org.freedesktop.Notifications"),
    dbus_conn(QDBusConnection::connectToBus(QDBusConnection::SessionBus, serviceName))
{
    Q_UNUSED(parent);

    if (!dbus_conn.isConnected())
        return;

    if (!dbus_conn.registerService(serviceName))
        return;

    dbus_conn.registerObject("/org/freedesktop/Notifications", this, QDBusConnection::ExportAllSlots);
}

void DBusInterface::GetCapabilities(QStringList& capabilities)
{
    capabilities << "body";
}

void DBusInterface::GetServerInformation(
    QString& name, QString& vendor, QString& version, QString& specVersion)
{
    name = "twmnd";
    vendor = "twmnd";
    version = "1.0";
    specVersion = "1.2";
}

void DBusInterface::CloseNotification(unsigned int id)
{
    Q_UNUSED(id);
}

void DBusInterface::Notify(
    const QString& appName,
    unsigned int id,
    const QString& icon,
    const QString& summary,
    const QString& body,
    const QStringList& actions,
    const QVariantMap& hints,
    int timeout,
    unsigned int& return_id)
{
    Q_UNUSED(appName);
    Q_UNUSED(actions);
    Q_UNUSED(hints);

    Message msg;
    if (!body.isEmpty())
        msg.data["content"] = Message::Data(body);
    if (!summary.isEmpty())
        msg.data[(msg.data["content"] ? "title" : "content")] = Message::Data(summary);
    if (!icon.isEmpty())
        msg.data["icon"] = Message::Data(icon);
    if (timeout != -1)
        msg.data["duration"] = Message::Data(timeout);
    msg.data["id"] = Message::Data(id ? id : ++lastNid);

    if (msg.data["content"])
    {
        emit messageReceived(msg);
    }

    // reply
    return_id = msg.data["id"]->toInt();
}

