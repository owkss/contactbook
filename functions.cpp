#include "functions.h"

#include <QIcon>
#include <QBuffer>
#include <QByteArray>
#include <QDataStream>

QIcon contactbook::load_from_data(const QByteArray &data)
{
    QIcon icon;

    QDataStream out(data);
    out >> icon;

    if (icon.isNull())
        return QIcon(":/images/dummy.png");
    else
        return icon;
}

QByteArray contactbook::save_to_data(const QIcon &icon)
{
    QByteArray data;

    QDataStream in(&data, QIODevice::WriteOnly);
    in << icon;
    return data;
}
