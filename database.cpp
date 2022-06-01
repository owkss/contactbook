#include "database.h"

#include "contact.h"

#include <QDir>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QStandardPaths>

#include <iostream>

DataBase::DataBase(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<Contact>("Contact");
}

DataBase::~DataBase()
{

}

void DataBase::init()
{
    if (m_initialized)
        return;

    if (!create_db())
        return;

    QSqlQuery query(*m_db);
    query.setForwardOnly(true);

    if (!query.exec("PRAGMA foreign_keys = off"))
        std::cerr << "init(): failed to set foreign_keys" << std::endl;

    if (!query.exec("PRAGMA cache_size = 2048"))
        std::cerr << "init(): failed to set cache_size" << std::endl;

    if (!query.exec("PRAGMA synchronous = off"))
        std::cerr << "init(): failed to set synchronous" << std::endl;

    if (!query.exec("PRAGMA locking_mode = exclusive"))
        std::cerr << "init(): failed to set locking_mode" << std::endl;

    if (!query.exec("PRAGMA journal_mode = OFF"))
        std::cerr << "init(): failed to set journal_mode" << std::endl;

    if (!query.exec("PRAGMA page_size = 4096"))
        std::cerr << "init(): failed to set page_size" << std::endl;

    if (!query.exec("CREATE TABLE IF NOT EXISTS contacts("
                    "id INTEGER PRIMARY KEY ASC, "
                    "name TEXT NOT NULL, "
                    "private_number TEXT, "
                    "office_number TEXT, "
                    "address TEXT, "
                    "email TEXT, "
                    "comment TEXT, "
                    "icon BLOB"
                    ");"))
        std::cerr << "init(): failed to create table \"contacts\"" << std::endl;

    query.finish();
    m_initialized = true;
}

void DataBase::deinit()
{
    if (!m_initialized)
        return;

    m_db->close();
    delete_db();

    m_initialized = false;
}

void DataBase::request_data()
{
    if (!m_initialized)
    {
        emit error_occured(tr("Невозможно выполнить запрос: база данных не инициализирована"));
        return;
    }

    QSqlQuery query(*m_db);
    query.setForwardOnly(true);

    const bool r = query.exec("SELECT name, private_number, office_number, address, email, comment, icon FROM contacts");
    if (r)
    {
        QList<Contact> contacts;
        while (query.next())
        {
            Contact c;
            c.name = query.value(0).toString();
            c.private_number = query.value(1).toString();
            c.office_number = query.value(2).toString();
            c.address = query.value(3).toString();
            c.email = query.value(4).toString();
            c.comment = query.value(5).toString();
            c.icon = query.value(6).toByteArray();
            contacts.push_back(c);
        }

        emit request_data_reply(contacts);
    }
    else
    {
        emit error_occured(tr("Невозможно выполнить запрос: ошибка выполнения"));
    }
}

void DataBase::request_save_new_contact(const Contact &c)
{
    if (!m_initialized)
    {
        emit error_occured(tr("Невозможно выполнить запрос добавления: база данных не инициализирована"));
        return;
    }

    QSqlQuery query(*m_db);
    query.setForwardOnly(true);

    if (query.prepare("INSERT INTO contacts(name, private_number, office_number, address, email, comment, icon) VALUES(:name, :private_number, :office_number, :address, :email, :comment, :icon)"))
    {
        query.bindValue(":name", c.name);
        query.bindValue(":private_number", c.private_number);
        query.bindValue(":office_number", c.office_number);
        query.bindValue(":address", c.address);
        query.bindValue(":email", c.email);
        query.bindValue(":comment", c.comment);
        query.bindValue(":icon", c.icon);

        if (query.exec())
            emit request_save_new_contact_reply(c);
        else
            emit error_occured(tr("Невозможно добавить в БД контакт \"%1\"").arg(c.name));
    }
    else
    {
        emit error_occured(tr("Ошибка базы данных"));
    }
}

void DataBase::request_remove_contact(const Contact &c)
{
    if (!m_initialized)
    {
        emit error_occured(tr("Невозможно выполнить запрос удаления: база данных не инициализирована"));
        return;
    }

    QSqlQuery query(*m_db);
    query.setForwardOnly(true);

    if (!query.exec(QString("DELETE FROM contacts WHERE name = '%1'").arg(c.name)))
        emit error_occured(tr("Невозможно удалить из БД контакт \"%1\"").arg(c.name));
}

bool DataBase::create_db()
{
    const QString connection = "contactbook.db";

    m_db = new QSqlDatabase;
    if (!m_db)
    {
        std::cerr << "create_db(): memory allocation failed" << std::endl;
        return false;
    }

    *m_db = QSqlDatabase::addDatabase("QSQLITE", connection);
    if (!m_db->isValid())
    {
        delete_db();

        std::cerr << "create_db(): database is not valid" << std::endl;
        return false;
    }

    m_db->setDatabaseName(get_db_path(connection));
    m_db->setConnectOptions("QSQLITE_ENABLE_SHARED_CACHE");

    if (!m_db->open())
    {
        delete_db();

        std::cerr << "create_db(): failed to open database" << std::endl;
        return false;
    }

    return true;
}

void DataBase::delete_db()
{
    if (!m_db)
        return;

    delete m_db;
    m_db = nullptr;
}

QString DataBase::get_db_path(const QString &conn)
{
    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    if (locations.empty())
        return conn;

    QDir dir(locations.at(0));
    if (!dir.exists())
        dir.mkpath(dir.absolutePath());

    return QString("%1/%2").arg(dir.absolutePath()).arg(conn);
}
