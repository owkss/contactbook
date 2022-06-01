#ifndef FUNCTIONS_H
#define FUNCTIONS_H

class QIcon;
class QByteArray;

namespace contactbook
{
    QIcon load_from_data(const QByteArray &data);
    QByteArray save_to_data(const QIcon &icon);
}

#endif // FUNCTIONS_H
