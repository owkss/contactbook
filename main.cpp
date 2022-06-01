#include "mainwindow.h"
#include "database.h"

#include <QThread>
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Fusion"));

    DataBase db;
    QThread database_thread;

    QObject::connect(&database_thread, &QThread::started, &db, &DataBase::init);
    QObject::connect(&database_thread, &QThread::finished, &db, &DataBase::deinit);

    MainWindow w;
    QObject::connect(&db, &DataBase::error_occured, &w, &MainWindow::error_occured);
    QObject::connect(&db, &DataBase::request_data_reply, &w, &MainWindow::request_data_reply);
    QObject::connect(&db, &DataBase::request_save_new_contact_reply, &w, &MainWindow::request_save_new_contact_reply);
    QObject::connect(&w, &MainWindow::request_data, &db, &DataBase::request_data);
    QObject::connect(&w, &MainWindow::request_save_new_contact, &db, &DataBase::request_save_new_contact);
    QObject::connect(&w, &MainWindow::request_remove_contact, &db, &DataBase::request_remove_contact);

    database_thread.start();
    w.show();

    const int r = a.exec();

    database_thread.quit();
    database_thread.wait(1000);

    return r;
}
