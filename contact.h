#ifndef CONTACT_H
#define CONTACT_H

#include <QString>
#include <QMetaType>
#include <QByteArray>

struct Contact
{
    Contact() {}
    Contact(const Contact &other) { *this = other; }
    Contact(Contact &&other)      { *this = std::move(other); }

    Contact &operator=(const Contact &other) noexcept
    {
        if (this != &other)
        {
            name = other.name;
            private_number = other.private_number;
            office_number = other.office_number;
            address = other.address;
            email = other.email;
            comment = other.comment;
            icon = other.icon;
            id = other.id;
        }

        return *this;
    }

    Contact &operator=(Contact &&other) noexcept
    {
        if (this != &other)
        {
            name = std::move(other.name);
            private_number = std::move(other.private_number);
            office_number = std::move(other.office_number);
            address = std::move(other.address);
            email = std::move(other.email);
            comment = std::move(other.comment);
            icon = std::move(other.icon);
            id = other.id;
            other.id = -1;
        }

        return *this;
    }

    bool valid() const noexcept { return !name.isEmpty() || id == -1; }
    operator bool() const noexcept { return valid(); }

    QString name;
    QString private_number;
    QString office_number;
    QString address;
    QString email;
    QString comment;
    QByteArray icon;
    int id = -1; // Служебное поле
};
Q_DECLARE_METATYPE(Contact);

#endif // CONTACT_H
