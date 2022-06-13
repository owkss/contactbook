#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "contact.h"
#include "functions.h"

#include <QDebug>
#include <QTimer>
#include <QSplitter>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    create_ui();

    QObject::connect(ui->image_btn, &QToolButton::clicked, this, &MainWindow::image_btn_clicked);
    QObject::connect(ui->add_contact_action, &QAction::triggered, this, &MainWindow::new_contact_action_triggered);
    QObject::connect(ui->add_contact_btn, &QPushButton::clicked, this, &MainWindow::new_contact_action_triggered);
    QObject::connect(ui->remove_contact_btn, &QPushButton::clicked, this, &MainWindow::remove_button_clicked);
    QObject::connect(ui->ready_btn, &QPushButton::clicked, this, &MainWindow::ready_button_clicked);
    QObject::connect(ui->refresh_btn, &QPushButton::clicked, this, &MainWindow::refresh_button_clicked);
    QObject::connect(ui->quit_action, &QAction::triggered, this, &MainWindow::quit_action_triggered);
    QObject::connect(ui->filter_edit, &QLineEdit::textChanged, this, &MainWindow::filter_text_changed);
    QObject::connect(ui->contact_list, &QListWidget::itemSelectionChanged, this, &MainWindow::item_selection_changed);
    QObject::connect(ui->contact_list, &QListWidget::customContextMenuRequested, this, &MainWindow::contact_list_right_mouse_button_clicked);

    QObject::connect(ui->name_edit, &QLineEdit::textChanged, this, &MainWindow::field_changed);
    QObject::connect(ui->private_number_edit, &QLineEdit::textChanged, this, &MainWindow::field_changed);
    QObject::connect(ui->office_number_edit, &QLineEdit::textChanged, this, &MainWindow::field_changed);
    QObject::connect(ui->address_edit, &QLineEdit::textChanged, this, &MainWindow::field_changed);
    QObject::connect(ui->email_edit, &QLineEdit::textChanged, this, &MainWindow::field_changed);
    QObject::connect(ui->comment_edit, &QLineEdit::textChanged, this, &MainWindow::field_changed);

    QTimer::singleShot(0, this, &MainWindow::load_state);
    QTimer::singleShot(0, this, &MainWindow::request_data);
}

MainWindow::~MainWindow()
{
    save_state();
    delete ui;
}

void MainWindow::error_occured(const QString &msg)
{
    QMessageBox::critical(this, tr("Ошибка"), msg);
}

void MainWindow::request_data_reply(const QList<Contact> &contacts)
{
    ui->contact_list->clear();
    if (contacts.empty())
        return;

    for (int i = 0; i < contacts.size(); ++i)
    {
        const Contact &c = contacts.at(i);
        add_contact_to_list(c);
    }
}

void MainWindow::request_save_new_contact_reply(const Contact &c)
{
    add_contact_to_list(c);
}

void MainWindow::request_refresh_contact_reply(const Contact &c, const int row)
{
    QListWidgetItem *item = ui->contact_list->item(row);
    if (!item)
    {
        error_occured(tr("Внутрення ошибка (название \"%1\", строка \"%2\")").arg(c.name).arg(row));
        return;
    }

    QVariant var;
    var.setValue(c);

    item->setText(c.name);
    item->setData(Qt::UserRole, var);
}

void MainWindow::save_state()
{
    QSettings sett("true", "contactbook");
    sett.beginGroup("mainwindow");
    sett.setValue("geometry", saveGeometry());
    sett.setValue("splitter", m_splitter->saveState());
    sett.endGroup();
}

void MainWindow::load_state()
{
    QSettings sett("true", "contactbook");
    sett.beginGroup("mainwindow");
    restoreGeometry(sett.value("geometry").toByteArray());
    m_splitter->restoreState(sett.value("splitter").toByteArray());
    sett.endGroup();
}

void MainWindow::new_contact_action_triggered()
{
    m_contact_editting = false;
    clear_fields();

    ui->contact_list->setEnabled(false);
    ui->add_contact_btn->setEnabled(false);
    ui->add_contact_action->setEnabled(false);

    ui->contact_info_widget->setEnabled(true);
    ui->ready_btn->setVisible(true);
}

void MainWindow::remove_button_clicked()
{
    QList<QListWidgetItem*> selected = ui->contact_list->selectedItems();
    if (selected.empty())
    {
        ui->remove_contact_btn->setVisible(false);
        return;
    }

    QListWidgetItem *item = selected.at(0);
    Contact c = item->data(Qt::UserRole).value<Contact>();

    QMessageBox::StandardButton btn = QMessageBox::question(this, tr("Удаление контакта \"%1\"").arg(c.name), tr("Вы уверены?"));
    if (btn != QMessageBox::Yes)
        return;

    ui->contact_list->removeItemWidget(item);
    delete item;

    if (ui->name_edit->text() == c.name)
        clear_fields();
    if (ui->contact_list->count() < 1)
        ui->contact_info_widget->setEnabled(false);

    emit request_remove_contact(c);
}

void MainWindow::ready_button_clicked()
{
    Contact c = get_fields();
    clear_fields();

    ui->contact_list->setEnabled(true);
    ui->add_contact_btn->setEnabled(true);
    ui->add_contact_action->setEnabled(true);

    ui->contact_info_widget->setEnabled(false);
    ui->ready_btn->setVisible(false);

    emit request_save_new_contact(c);
}

void MainWindow::refresh_button_clicked()
{
    QList<QListWidgetItem*> selected = ui->contact_list->selectedItems();
    if (selected.empty())
        return;

    const int row = ui->contact_list->row(selected.at(0));

    m_contact_editting = false;
    ui->refresh_btn->setVisible(false);

    Contact c = get_fields();
    emit request_refresh_contact(c, row);
}

void MainWindow::quit_action_triggered()
{
    close();
}

void MainWindow::image_btn_clicked()
{
    QString image_path = QFileDialog::getOpenFileName(this, tr("Выбор изображения"), QApplication::applicationDirPath(), tr("Файлы изображений(*.bmp *.gif *.jpg *.jpeg *.png)"));
    if (image_path.isEmpty())
        return;

    QImage img(image_path);
    ui->image_btn->setIcon(QPixmap::fromImage(std::move(img)));
}

void MainWindow::item_selection_changed()
{
    m_contact_editting = false;
    ui->refresh_btn->setVisible(false);

    QList<QListWidgetItem*> selected = ui->contact_list->selectedItems();
    if (selected.empty())
        return;

    ui->contact_info_widget->setEnabled(true);

    QListWidgetItem *item = selected.at(0);
    Contact c = item->data(Qt::UserRole).value<Contact>();
    set_fields(c);

    m_contact_editting = true;
}

void MainWindow::filter_text_changed(const QString &text)
{
    QListWidget *lw = ui->contact_list;
    lw->setUpdatesEnabled(false);

    for (int i = 0; i < lw->count(); ++i)
    {
        lw->setRowHidden(i, false);
    }

    for (int i = 0; i < lw->count(); ++i)
    {
        QListWidgetItem *item = lw->item(i);
        QString name = item->text();

        if (!name.contains(text, Qt::CaseInsensitive))
        {
            lw->setRowHidden(i, true);
        }
    }

    lw->setUpdatesEnabled(true);
}

void MainWindow::contact_list_right_mouse_button_clicked(const QPoint &pos)
{
    ui->remove_contact_btn->setVisible(bool(ui->contact_list->itemAt(pos)));
}

void MainWindow::field_changed(const QString &)
{
    if (!m_contact_editting)
        return;

    if (!ui->refresh_btn->isVisible())
        ui->refresh_btn->setVisible(true);
}

void MainWindow::create_ui()
{
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->addWidget(ui->contact_list_widget);
    m_splitter->addWidget(ui->contact_info_widget);
    m_splitter->setStyleSheet("QSplitter { border: 1px; } QSplitter::handle { width: 1px; height: 1px; background: lightgray; border-radius: 2px; }");
    setCentralWidget(m_splitter);

    ui->contact_list->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->filter_edit->addAction(QIcon(":/images/find.png"), QLineEdit::LeadingPosition);
    ui->contact_info_widget->setEnabled(false);
    ui->ready_btn->setVisible(false);
    ui->refresh_btn->setVisible(false);
    ui->remove_contact_btn->setVisible(false);
    ui->id->setVisible(false);
}

Contact MainWindow::get_fields() const
{
    Contact c;
    c.name = ui->name_edit->text();
    c.private_number = ui->private_number_edit->text();
    c.office_number = ui->office_number_edit->text();
    c.address = ui->address_edit->text();
    c.email = ui->email_edit->text();
    c.comment = ui->comment_edit->text();
    c.icon = contactbook::save_to_data(ui->image_btn->icon());
    c.id = ui->id->value();
    return c;
}

void MainWindow::set_fields(const Contact &c)
{   
    ui->name_edit->blockSignals(true);
    ui->private_number_edit->blockSignals(true);
    ui->office_number_edit->blockSignals(true);
    ui->address_edit->blockSignals(true);
    ui->email_edit->blockSignals(true);
    ui->comment_edit->blockSignals(true);

    /* Выставление значений ---> */
    ui->name_edit->setText(c.name);
    ui->private_number_edit->setText(c.private_number);
    ui->office_number_edit->setText(c.office_number);
    ui->address_edit->setText(c.address);
    ui->email_edit->setText(c.email);
    ui->comment_edit->setText(c.comment);
    ui->image_btn->setIcon(contactbook::load_from_data(c.icon));
    ui->id->setValue(c.id);
    /* <--- Выставление значений */

    ui->name_edit->blockSignals(false);
    ui->private_number_edit->blockSignals(false);
    ui->office_number_edit->blockSignals(false);
    ui->address_edit->blockSignals(false);
    ui->email_edit->blockSignals(false);
    ui->comment_edit->blockSignals(false);
}

void MainWindow::clear_fields()
{
    ui->remove_contact_btn->setVisible(false);
    ui->image_btn->setIcon(QIcon(":/images/dummy.png"));
    ui->name_edit->clear();
    ui->private_number_edit->clear();
    ui->office_number_edit->clear();
    ui->address_edit->clear();
    ui->email_edit->clear();
    ui->comment_edit->clear();
    ui->id->setValue(0);
}

void MainWindow::add_contact_to_list(const Contact &c)
{
    QVariant var;
    var.setValue(c);

    QListWidgetItem *item = new QListWidgetItem;
    item->setText(c.name);
    item->setIcon(contactbook::load_from_data(c.icon));
    item->setData(Qt::UserRole, var);

    ui->contact_list->addItem(item);
}

