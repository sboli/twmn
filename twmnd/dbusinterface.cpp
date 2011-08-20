#include "dbusinterface.h"
#include <QDebug>
#include <QTimer>
#include <stdexcept>

DBusInterface::DBusInterface(QObject *parent)
{
    DBusError dbus_err;

    dbus_error_init(&dbus_err);

    dbus_conn = dbus_bus_get(DBUS_BUS_SESSION, &dbus_err);
    if (!dbus_conn)
        throw std::runtime_error(dbus_err.message);

    int ret = dbus_bus_request_name(dbus_conn, "org.freedesktop.Notifications", DBUS_NAME_FLAG_REPLACE_EXISTING , &dbus_err);
    if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret)
        throw std::runtime_error(dbus_err.message);

    dbus_error_free(&dbus_err);
    QTimer* timer = new QTimer(this);
    timer->setSingleShot(false);
    timer->setInterval(200);
    connect(timer, SIGNAL(timeout()), this, SLOT(check()));
    timer->start();
}

void DBusInterface::check()
{
    DBusMessage* msg;
    dbus_connection_read_write(dbus_conn, 0);
    msg = dbus_connection_pop_message(dbus_conn);
    if (!msg)
        return;

   if (dbus_message_is_method_call(msg, "org.freedesktop.Notifications", "Notify"))
      Notify(msg);
   if (dbus_message_is_method_call(msg, "org.freedesktop.Notifications", "GetCapabilities"))
      GetCapabilities(msg);
   if (dbus_message_is_method_call(msg, "org.freedesktop.Notifications", "GetServerInformation"))
      GetServerInformation(msg);
   if (dbus_message_is_method_call(msg, "org.freedesktop.Notifications", "CloseNotification"))
      CloseNotification(msg);

   dbus_message_unref(msg);
   dbus_connection_flush(dbus_conn);
}

void DBusInterface::GetCapabilities(DBusMessage *msg)
{
    DBusMessage* reply;
    DBusMessageIter args;
    DBusMessageIter subargs;
    int ncaps = 1;

    char *caps[1] = {"body"}, **ptr = caps;  // workaround (see specs)
    serial++;

    //printf("GetCapabilities called!\n");

    reply = dbus_message_new_method_return(msg);
    if(!reply)
       return;

    dbus_message_iter_init_append(reply, &args);
    if (!dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, NULL, &subargs ) ||
           !dbus_message_iter_append_fixed_array(&subargs, DBUS_TYPE_STRING, &ptr, ncaps) ||
           !dbus_message_iter_close_container(&args, &subargs) ||
    !dbus_connection_send(dbus_conn, reply, &serial))
        return;

    dbus_message_unref(reply);
}

void DBusInterface::GetServerInformation(DBusMessage *msg)
{
    DBusMessage* reply;
    DBusMessageIter args;

    char* info[4] = {"twmnd", "twmnd", "2011", "2011"};
    serial++;

    //printf("GetServerInfo called!\n");

    reply = dbus_message_new_method_return(msg);

    dbus_message_iter_init_append(reply, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &info[0]) ||
    !dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &info[1]) ||
    !dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &info[2]) ||
    !dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &info[3]) ||
    !dbus_connection_send(dbus_conn, reply, &serial))
       return;

    dbus_message_unref(reply);
}

void DBusInterface::CloseNotification(DBusMessage *msg)
{
    /*
    DBusMessage* reply;
    DBusMessageIter args;
    dbus_uint32_t nid=0;
    notification *ptr = messages;

    dbus_message_iter_init(msg, &args);
    dbus_message_iter_get_basic(&args, &nid);
    if( ptr!=NULL && ptr->nid==nid ) {
       ptr->expires_after=(time(NULL) - ptr->started_at)*EXPIRE_MULT;
       ptr->closed=1;
    } else if( ptr!=NULL ) {
       while( ptr->next != NULL && ptr->next->nid != nid ) {
          ptr=ptr->next;
       }

       if( ptr->next != NULL && ptr->next->nid==nid ) {
          ptr = ptr->next;
          ptr->expires_after=(time(NULL) - ptr->started_at)*EXPIRE_MULT;
          ptr->closed=1;
       }
    }

    reply = dbus_message_new_method_return(msg);
    if( !dbus_connection_send(dbus_conn, reply, &serial)) return 1;
        dbus_message_unref(reply);
        */
}

void DBusInterface::Notify(DBusMessage *msg)
{
    DBusMessage* reply;
    DBusMessageIter args;
    const char *appname;
    const char *summary;
    const char *body;
    const char* icon;
    dbus_uint32_t nid=0;
    dbus_int32_t expires=-1;
    /*
    notification *ptr = messages;
    notification *note = NULL;
    */

    serial++;


    dbus_message_iter_init(msg, &args);
    dbus_message_iter_get_basic(&args, &appname);
    dbus_message_iter_next( &args );
    dbus_message_iter_get_basic(&args, &nid);
    dbus_message_iter_next( &args );
    dbus_message_iter_get_basic(&args, &icon);
    dbus_message_iter_next( &args );
    dbus_message_iter_get_basic(&args, &summary);
    dbus_message_iter_next( &args );
    dbus_message_iter_get_basic(&args, &body);
    dbus_message_iter_next( &args );
    dbus_message_iter_next( &args );  // skip actions
    dbus_message_iter_next( &args );  // skip hints
    dbus_message_iter_get_basic(&args, &expires);

    Message m;
    if (strlen(body))
        m.data["content"] = boost::optional<QVariant>(QString::fromAscii(body));
    if (strlen(summary))
        m.data[m.data["content"] ? "title" : "content"] = boost::optional<QVariant>(QString::fromAscii(summary));
    if (strlen(icon))
        m.data["icon"] = boost::optional<QVariant>(QString::fromAscii(icon));
    if (expires != -1) {
        m.data["duration"] = boost::optional<QVariant>(int(expires));
    }
    emit messageReceived(m);
    /*
    if( nid!=0 ) { // update existing message
     note = messages;
     if( note!=NULL )
        while( note->nid != nid && note->next!=NULL ) note=note->next;

     if( note==NULL || note->nid!=nid ) { // not found, re-create
        note = calloc(sizeof(notification), 1);
        note->nid=nid;
        nid=0;
      }
    } else {
      note = calloc(sizeof(notification), 1);
      note->nid=curNid++;
      note->started_at = time(NULL);
    }
*/

    //note->expires_after = (time_t)(expires<0?EXPIRE_DEFAULT:expires*EXPIRE_MULT);
    //note->closed=0;
    //strncpy( note->appname, appname, 20);
    //strncpy( note->summary, summary, 64);
   // strncpy( note->body,    body, 256);

    reply = dbus_message_new_method_return(msg);

    dbus_message_iter_init_append(reply, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT32, &nid) ||
    !dbus_connection_send(dbus_conn, reply, &serial))
     return;
    dbus_message_unref(reply);
}
