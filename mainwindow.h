#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui { class MainWindow; }
class QSplitter;
struct Contact;
class QListWidgetItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:

public slots:
    void error_occured(const QString &msg); // Отображение ошибок в графическом виде
    void request_data_reply(const QList<Contact> &contacts); // Получение всех данных из БД
    void request_save_new_contact_reply(const Contact &c); // Признак успешного добавления в БД
    void request_refresh_contact_reply(const Contact &c, const int row); // Признак успешного редактирования записи

private slots:
    void save_state();
    void load_state();

    void new_contact_action_triggered();
    void remove_button_clicked();
    void ready_button_clicked();
    void refresh_button_clicked();
    void quit_action_triggered();

    void image_btn_clicked();
    void item_selection_changed();
    void filter_text_changed(const QString &text);
    void contact_list_right_mouse_button_clicked(const QPoint &pos);
    void field_changed(const QString &);

private:
    void create_ui();
    Contact get_fields() const;
    void set_fields(const Contact &c);
    void clear_fields();

    void add_contact_to_list(const Contact &c);

    Ui::MainWindow *ui;
    QSplitter *m_splitter = nullptr;

    bool m_contact_editting = false; // Признак редактирование контакта

signals:
    void request_data(); // Запрос на выдачу всех данные из БД
    void request_save_new_contact(const Contact &c); // Запрос на сохранение в БД новой записи
    void request_refresh_contact(const Contact &c, const int row); // Запрос за редактирование данных
    void request_remove_contact(const Contact &c);
};
#endif // MAINWINDOW_H
