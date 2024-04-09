#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <postgresql/libpq-fe.h>
#include <QDir>
#include <QLineEdit>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public slots:


public:
    MainWindow(QWidget *parent = nullptr);
    void create_list_buttons ();
    void add_check_list(const QString table_name);
    void add_button_func(QPushButton *button, const QString& name);
    void fill_up_from_file(const QString *sourse, bool check_drop = false);
    void fill_up_from_struct(bool check_drop = false);
    void set_table_name(const QString& tableName = nullptr);
    void setting_main_widgets();
    void clear_check_list();
    void clear_list_buttons();
    void default_struct_click();

    void executeQuery(const QString *query);
    QStringList extract_headers(QFile *file);
    QString extract_table_name(const QString &filePath);
    bool table_exists(const QString& table_name);
    QStringList get_list_tables_from_file(const QString& fileName);

    void create_table(const QString& table_name, const QStringList& headers);
    void insert_data_from_file(const QString& table_name, QFile* file, QStringList headers);
    QString query_for_main_list(const QString column);
    PGresult* get_grocery_from_table(const QString table_name, const QString column);
    void customization_checkBoxLineEditPairs(int i, const QString& table_name, PGresult *res);

    ~MainWindow();

signals:
    void tableNameChanged();
    void hiddenButtonClick(QPushButton *button);


private slots:
    void on_user_lists_clicked();
    void on_supermarket_clicked();
    void on_consumables_clicked();
    void on_large_purchases_clicked();
    void on_holidays_clicked();
    void on_recipes_clicked();
    void on_new_list_clicked();

    void hidden_button_click(const QString table_name, QPushButton *button);
    void highlightActiveButton();
    void highlightActiveButton_hid(QPushButton* active_button  = nullptr);

private:
    QList<QString> dirs = {"", "user_lists", "supermarket", "consumables", "large_purchases", "holidays", "recipes"};
    const QString prefix_path = "../postgres_files/";

    QString CONNINFO = "host=localhost port=5433 dbname=grocery_db user=grocery_user password=grocery_password";

    QString TABLE_NAME;
    QVector<QPair<QCheckBox*, QLineEdit*>> checkBoxLineEditPairs;
    QVector<QPair<QPushButton*, QString>> category_buttons;

    const QString grey = "background-color: rgb(220, 220, 220);";
    const QString green = "background-color: rgb(80, 200, 120);";
    const QString blue = "background-color: rgb(135, 206, 250);";

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
