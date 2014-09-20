#ifndef MESSAGE_H
#define MESSAGE_H

#include <boost/optional.hpp>
#include <QString>
#include <QMap>
#include <QVariant>

struct Message
{
    typedef boost::optional<QVariant> Data;
    QMap<QString, Data> data;
};

#endif // MESSAGE_H
