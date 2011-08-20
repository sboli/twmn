#ifndef MESSAGE_H
#define MESSAGE_H

#include <boost/optional.hpp>
#include <QString>
#include <QMap>
#include <QVariant>

struct Message
{
    QMap<QString, boost::optional<QVariant> > data;
};

#endif // MESSAGE_H
