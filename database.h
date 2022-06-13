#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>

class QSqlDatabase;
struct Contact;

class DataBase : public QObject
{
    Q_OBJECT
public:
    DataBase(QObject *parent = nullptr);
    ~DataBase();

protected:

public slots:
    void init();
    void deinit();

    void request_data(); // Запрос на выдачу всех данные из БД
    void request_save_new_contact(const Contact &c); // Запрос на сохранение в БД новой записи
    void request_refresh_contact(const Contact &c, const int row); // Запрос на редактирование записи
    void request_remove_contact(const Contact &c);

private slots:

private:
    bool create_db();
    void delete_db();

    QString get_db_path(const QString &conn);

    QSqlDatabase *m_db = nullptr;
    bool m_initialized = false;

signals:
    void error_occured(const QString &msg); // Отображение в графическом виде каких-либо ошибок
    void request_data_reply(const QList<Contact> &contacts); // Выдача всех данных из БД
    void request_save_new_contact_reply(const Contact &c); // Признак успешного добавления в БД
    void request_refresh_contact_reply(const Contact &c, const int row); // Признак успешного редактирования записи
};

#endif // DATABASE_H
