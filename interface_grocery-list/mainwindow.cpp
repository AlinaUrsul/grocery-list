#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>


MainWindow::~MainWindow()
{
    delete ui;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setting_main_widgets();
    fill_up_from_struct();
}

// Измененяет переменную класса TABLE_NAME
void MainWindow::set_table_name(const QString& tableName) {
    TABLE_NAME = tableName;
    emit tableNameChanged();
}

// Окрашивает активную кнопку категории, если она есть, в светло-зеленый, остальные - в светло-серый
void MainWindow::highlightActiveButton() {
    for (auto pair : category_buttons) {
        // Если имя кнопки совпадает с TABLE_NAME, окрашиваем в светло-зеленый
        if (pair.second == TABLE_NAME) {
            pair.first->setStyleSheet(green);
        }else {
            // Иначе окрашиваем в светло-серый
            pair.first->setStyleSheet(grey);
        }
    }
}

// Окрашивает активную кнопку в scrollArea_2 в светло-голубой, остальные - в светло-серый
void MainWindow::highlightActiveButton_hid(QPushButton* active_button) {
    QWidget *scrollAreaWidget = ui->scrollArea_2->widget();
    if (!scrollAreaWidget) {
        return;
    }
    // Получаем список всех кнопок внутри содержимого QScrollArea
    QList<QPushButton *> buttons = scrollAreaWidget->findChildren<QPushButton *>();

    // Изменяем цвет каждой найденной кнопки
    for (QPushButton *button : buttons) {
        // Oкрашиваем в светло-серый
        button->setStyleSheet(grey);
    }
    // Окрашиваем активную кнопку в светло-голубой цвет
    if (active_button){

        active_button->setStyleSheet(blue);
        ui->title_line->setStyleSheet(blue);
    }
}

// Устанавливает основные настройки окна, слоты и сигналы, вызывает разово функции
void MainWindow::setting_main_widgets() {

    connect(this, &MainWindow::tableNameChanged, this, &MainWindow::highlightActiveButton);
//    connect(this, &MainWindow::hiddenButtonClick, this, &MainWindow::highlightActiveButton_hid);
    connect(this, &MainWindow::hiddenButtonClick, this, [=](QPushButton* active_button) {
        highlightActiveButton_hid(active_button);
    });

    ui->title_line->setMaxLength(25);

    category_buttons.append(qMakePair(ui->user_lists, QString("user_lists")));
    category_buttons.append(qMakePair(ui->supermarket, QString("supermarket")));
    category_buttons.append(qMakePair(ui->consumables, QString("consumables")));
    category_buttons.append(qMakePair(ui->large_purchases, QString("large_purchases")));
    category_buttons.append(qMakePair(ui->holidays, QString("holidays")));
    category_buttons.append(qMakePair(ui->recipes, QString("recipes")));

    set_table_name("no");

    connect(ui->default_struct, &QAction::triggered, this, [this]() {
        default_struct_click();
    });
}

// Чистит область чек-листа scrollArea и заголовка title_line
void MainWindow::clear_check_list() {
     QWidget *widget = ui->scrollArea->takeWidget(); // Удаление текущего виджета из QScrollArea
         delete widget; // Освобождение памяти
     ui->title_line->clear();
     ui->title_line->setStyleSheet("background-color: rgb(220, 220, 220);");
}

// Чистит область scrollArea_2 от кнопок
void MainWindow::clear_list_buttons() {
    QWidget *widget = ui->scrollArea_2->takeWidget();
        delete widget;
}

// Возвращает заголовок для таблицы, взятый из имени файла
QString MainWindow::extract_table_name(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName(); // Получаем имя файла с расширением
    int pos = fileName.lastIndexOf('.'); // Находим позицию последней точки (расширения)
    if (pos != -1) {
        return fileName.left(pos); // Возвращаем часть имени файла до последней точки (без расширения)
    } else {
        return fileName; // Если точка не найдена, возвращаем полное имя файла
    }
}

// Возвращает список из заголовков тиблицы(первую строку) из файла
QStringList MainWindow::extract_headers(QFile *file){
    // Прочитать первую строку файла для получения заголовков столбцов
    QStringList res, splitline;
    QString line;
    if (file->atEnd()) {
        qDebug() << "Ошибка чтения заголовков столбцов";
    } else {
        line = QString::fromStdString(file->readLine().toStdString());
        qDebug() << "Строка с наименованиями столбцов: " << line;
        // Разделить заголовки столбцов
        splitline = line.split(',');
        for (const QString& str : splitline) {
                res << str.trimmed(); // Удаляем лишние пробелы и символы новой строки
            }
    }
    return res;
}

// Выполняет запрос в бд, ничего не возвращает
void MainWindow::executeQuery(const QString *query) {
    PGconn *conn = PQconnectdb(CONNINFO.toUtf8().constData());
    if (PQstatus(conn) != CONNECTION_OK) {
        qDebug() << "Ошибка подключения к базе данных: " << PQerrorMessage(conn);
    } else {
        qDebug() << "Успешное подключение к базе данных!";
        PGresult *res = PQexec(conn, query->toUtf8().constData());
        if (PQresultStatus(res) != PGRES_COMMAND_OK && PQresultStatus(res) != PGRES_TUPLES_OK) {
            qDebug() << "Ошибка выполнения запроса: " << PQerrorMessage(conn);
        } else {
            qDebug() << "Запрос успешно выполнен!" << query->toUtf8().constData();
            int numRows = PQntuples(res);
            int numCols = PQnfields(res);
            for (int i = 0; i < numRows; ++i) {
                for (int j = 0; j < numCols; ++j) {
                    qDebug() << "Value[" << i << "][" << j << "]: " << PQgetvalue(res, i, j);
                }
            }
        }
        PQclear(res);
        PQfinish(conn);
        qDebug() << "Отключение от базы данных\n\n";

    }
}

// Возвращает true или false о наличии таблицы в бд(выполняет запрос)
bool MainWindow::table_exists(const QString& table_name) {
    // Установка соединения с базой данных
    PGconn *conn = PQconnectdb(CONNINFO.toUtf8().constData());
    bool exists = false;

    // Проверка состояния соединения
    if (PQstatus(conn) != CONNECTION_OK) {
        qDebug() << "Ошибка подключения к базе данных: " << PQerrorMessage(conn);
    } else {
        // Формирование SQL-запроса для проверки наличия таблицы
        QString query = "SELECT EXISTS (SELECT 1 FROM information_schema.tables WHERE table_schema = 'public' AND table_name = '"
                + table_name + "') AS table_exists;";

        // Выполнение SQL-запроса
        PGresult *res = PQexec(conn, query.toUtf8().constData());

        // Проверка статуса результата запроса
        if (PQresultStatus(res) != PGRES_COMMAND_OK && PQresultStatus(res) != PGRES_TUPLES_OK) {
            qDebug() << "Ошибка выполнения запроса: " << PQerrorMessage(conn);
        } else {
            // Извлечение результата запроса
            exists = strcmp(PQgetvalue(res, 0, 0), "t") == 0;
            qDebug() << "Таблица с именем" << table_name << (exists ? "существует" : "не существует");
                    }

        // Освобождение результата запроса и закрытие соединения
        PQclear(res);
        PQfinish(conn);
    }

    return exists;
}

// Создает теблицу в бд из имени таблицы и заголовков столбцов
void MainWindow::create_table(const QString& table_name, const QStringList& headers) {
    QString query;
    query = "DROP TABLE IF EXISTS " + table_name + " CASCADE;";
    executeQuery(&query);
    query = "CREATE TABLE IF NOT EXISTS " + table_name + " (id SERIAL PRIMARY KEY, " + headers.join(" VARCHAR(30), ") + " VARCHAR(30));";
    executeQuery(&query);
}

// Вставляет данные в таблицу из файла
void MainWindow::insert_data_from_file(const QString& table_name, QFile* file, QStringList headers) {
    QTextStream in(file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList values = line.split(',');
        QString insertQuery = "INSERT INTO " + table_name + "(" + headers.join(", ") + ") VALUES ('" + values.join("', '") + "');";
        executeQuery(&insertQuery);
    }
}

// Наполняет бд в соответствии с переданным файлом и статусом программы
void MainWindow::fill_up_from_file(const QString *sourse, bool check_drop){
    // Открываем файл и проверяем успешность открытия
    QFile file(sourse->toUtf8().constData());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Ошибка открытия файла: " << sourse;
        return;
    } else {
        qDebug() << "Открыт файл: " << *sourse;
    }

    // Получаем имя таблицы из имени файла
    QString table_name = extract_table_name(sourse->toUtf8().constData());
    qDebug() << "Название таблицы:" << table_name;

    // Получаем заголовки столбцов из файла
    QStringList headers = extract_headers(&file);
    qDebug() << "Разделенные заголовки столбцов:";
    for (const QString& header : headers) {
        qDebug() << header;
    }

    // Проверяем существование таблицы в базе данных и создаем/пересоздаем при необходимости
    if (!table_exists(table_name) || check_drop) {
        create_table(table_name, headers);
        // Вносим данные из файла в таблицу
        insert_data_from_file(table_name, &file, headers);    }
    else {
        qDebug() << "Таблица " << table_name << " уже существует, пропускаем создание.";
    }

    // Закрываем файл
    file.close();
}

void MainWindow::default_struct_click() {
    clear_check_list();
    clear_list_buttons();
    set_table_name();
    fill_up_from_struct(true);
}

// Передает все файлы из списка в fill_up_from_file для наполнения бд
void MainWindow::fill_up_from_struct(bool check_drop){
    for (QString &folderPath : dirs) {
        folderPath = prefix_path + folderPath;
        QDir folder(folderPath);
        QFileInfoList files = folder.entryInfoList(QDir::Files);

        for (const QFileInfo& fileInfo : files) {
            QString filePath = fileInfo.absoluteFilePath();
            fill_up_from_file(&filePath, check_drop);
        }

    }
    qDebug() << "Таблицы созданы!";
}

QStringList MainWindow::get_list_tables_from_file(const QString& filepath) {
    QStringList list_tables;

    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Ошибка при открытии файла" << filepath;
        return list_tables;
    }

    QTextStream in(&file);
    bool skipHeader = true; // Переменная для пропуска строки с заголовками
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (skipHeader) {
            skipHeader = false;
            continue; // Пропускаем строку с заголовками
        }
        QStringList fields = line.split(","); // Разбиваем строку на поля по разделителю ","
        if (fields.size() > 0) {
            list_tables.append(fields[0]); // Добавляем первое поле (первую колонку) в список
        }
    }

    file.close();
    return list_tables;
}


QString MainWindow::query_for_main_list(const QString column){
    QString query;
    QList<QString> main_check_list, category_tables,
                in_postgres_files = {"user_lists", "supermarket", "consumables"};
    for (auto category: in_postgres_files) { // category = user_lists
        QString category_filepath = prefix_path + category + ".csv"; // filepath = user_lists.csv
        category_tables = get_list_tables_from_file(category_filepath);
        main_check_list.append(category_tables);
        qDebug() << "Добавляемые поля" << category_tables;
    }

    // ТУТ СОЗДАЮ ЗАПРОС ИЗ main_check_list И id, name, opt, ГДЕ opt = "t"
        for (auto table : main_check_list) {
            query += "SELECT " + column + " FROM " + table + " WHERE opt='t' UNION ALL ";
        }
        int lengthToRemove = QString(" UNION ALL ").length();
        query = query.chopped(lengthToRemove);
        query += ";";
        executeQuery(&query);
        qDebug() << "ПОЛУЧИЛСЯ ТАКОЙ ЗАПРОС: " << query;
        return query;
}

// Возвращает ответ из бд в качкстве отсортированного списка для создания кнопок и чек-листа
PGresult* MainWindow::get_grocery_from_table(const QString table_name, const QString column) {
    // Формируем SQL-запрос для получения списка названий
    QString query;
    if (column.contains("opt")) {









        // ЕСЛИ СНИМАТЬ ГАЛОЧКУ С ЧЕК-ЛИСТА В  main_list ЗАПРОС ОТПРАВЛЯЕТСЯ С ТАБЛИЦЕЙ main_list, А НЕ В ОРИГИНАЛЬНУЮ ТАБЛИЦУ!






        if (table_name == "main_list") {
            query = query_for_main_list(column);
        } else {
            query = "SELECT " + column + " FROM " + table_name + " ORDER BY opt DESC, id ASC;";
        }
    } else {
        query = "SELECT " + column + " FROM " + table_name + " ORDER BY id ASC;";
    }

    PGconn *conn = PQconnectdb(CONNINFO.toUtf8().constData());
    if (PQstatus(conn) != CONNECTION_OK) {
        qDebug() << "Ошибка подключения к базе данных: " << PQerrorMessage(conn);
        return nullptr;
    } else {
        qDebug() << "Успешное подключение к базе данных!";
        PGresult *res = PQexec(conn, query.toUtf8().constData());
        PQfinish(conn);
        qDebug() << "Отключение от базы данных!";
        return res;
    }
}

// Создание чек-бокса и поля ввода, их настройка и соединение с помощью сигналов и слотов
void MainWindow::customization_checkBoxLineEditPairs(int i, const QString& table_name, PGresult *res) {
     QString id = PQgetvalue(res, i, 0), product = PQgetvalue(res, i, 1), opt = PQgetvalue(res, i, 2);

    // Создание чек-бокса на основе ID
    QCheckBox *checkBox = new QCheckBox(id, this);
    if (opt == "t") {
        checkBox->setChecked(true);
    }
    checkBox->setStyleSheet("font-size: 1px;");// Установка минимального размера текста
    checkBox->setLayoutDirection(Qt::RightToLeft); // Поменять местами checkBox с его текстом
    // Создание поля ввода с названием
    QLineEdit *lineEdit = new QLineEdit(product);
    lineEdit->setMaxLength(30);

    // Связывание сигнала returnPressed() от QLineEdit с переключением checkBox
    connect(lineEdit, &QLineEdit::returnPressed, this, [checkBox, table_name, id, this]() {
        checkBox->setChecked(true);
        QString query = "UPDATE " + table_name + " SET opt = 't'" + " WHERE id = " + id + ";";
        executeQuery(&query);
    });

    // Связывание сигнала stateChanged() от QCheckBox с добавлением текста к lineEdit
    connect(checkBox, &QCheckBox::stateChanged, this, [lineEdit, checkBox, table_name, id, this](int state) {
        if (state == Qt::Checked) {
            lineEdit->setText(lineEdit->text() + checkBox->text());
            QString query = "UPDATE " + table_name + " SET opt = 't'" + " WHERE id = " + id + ";";
            executeQuery(&query);
        } else {
            // Если галочка снята, удалите текст из lineEdit
            QString text = lineEdit->text();
            text.remove(checkBox->text());
            lineEdit->setText(text);
            QString query = "UPDATE " + table_name + " SET opt = 'f'" + " WHERE id = " + id + ";";
            executeQuery(&query);
        }
    });
    // Добавление чек-бокса и поля ввода в структуру данных
    checkBoxLineEditPairs.append(qMakePair(checkBox, lineEdit));
}



//_____________________________________________________________//

// Заполнение области scrollArea чек-боксами
void MainWindow::add_check_list(const QString table_name) {
    QVBoxLayout *v_layout = new QVBoxLayout;

//    qDebug() << "TABLE_NAME: " << TABLE_NAME;
    PGresult *res = get_grocery_from_table(table_name, "id, name, opt");

    // Проверяем успешность выполнения запроса
    if (res != nullptr && PQntuples(res) > 0) {
        // Создаем строки с названиями из результата запроса
        for (int i = 0; i < PQntuples(res); ++i) {
            customization_checkBoxLineEditPairs(i, table_name, res);
        }
        // Добавление отсортированных виджетов в вертикальный макет
        for (auto pair : checkBoxLineEditPairs) {
            QHBoxLayout *h_layout = new QHBoxLayout;
            h_layout->addWidget(pair.first);
            h_layout->addWidget(pair.second);
            v_layout->addLayout(h_layout);
        }
    } else {
        // Обработка случая, если запрос не вернул результатов или произошла ошибка
        qDebug() << "Ошибка выполнения запроса или таблица" << table_name << " пуста";
    }

        PQclear(res);
    // Создание нового виджета для размещения в ScrollArea
    QWidget *widget = new QWidget;
    widget->setLayout(v_layout);

    // Установка виджета в ScrollArea
    ui->scrollArea->setWidget(widget);
}

// Вызывает слот на окрашивание активной кнопки в scrollArea_2 и метод по заполнению scrollArea чек-боксами
void MainWindow::hidden_button_click(const QString table_name, QPushButton *button){
    // Очистка структуры для хранения информации о чек-боксах и полях ввода
    emit hiddenButtonClick(button);
    checkBoxLineEditPairs.clear();
    add_check_list(table_name);
}

// Настраивает отображение имени кнопки в заголовке (и наоборот) и соединяет кнопку со слотом на окрашивание и вывод чек-боксов
void MainWindow::add_button_func(QPushButton *button, const QString& table_name) {
    // вывод чек-боксов
    connect(button, &QPushButton::clicked, this, [=]() { hidden_button_click(table_name, button); });

    // отображение текста с кнопки
    QObject::connect(button, &QPushButton::clicked, this, [=]() {
        ui->title_line->setText(button->text());
    });
    // изменение текста на кнопке
//    QObject::connect(ui->title_line, &QLineEdit::textChanged, this, [=](const QString &text) {
//        button->setText(text);
//    });

//    QObject::connect(ui->title_line, &QLineEdit::textChanged, [&](const QString &text) {
//        QPushButton *currentButton = qobject_cast<QPushButton*>(sender()); // Получаем указатель на текущую кнопку
//        if (currentButton) {
//            currentButton->setText(text);
//        }
//    });

    button->setFixedSize(210, 70);
//    add_context_menu(button);
}


// Создает и размещает кнопки в scrollArea_2
void MainWindow::create_list_buttons() {
    clear_check_list();
    QLayout *existingLayout = ui->scrollArea_2->layout();
    if (existingLayout) {
        while (QLayoutItem* item = existingLayout->takeAt(0)) {
            delete item->widget();
            delete item;
        }
        delete existingLayout;
    }

    // Получаем список наименований и таблиц из базы данных
    PGresult *res = get_grocery_from_table(TABLE_NAME, "table_name, name");

    // Создаем QGridLayout для размещения кнопок
    QGridLayout *layout = new QGridLayout;

    // Вычисляем количество кнопок (строк) для отображения
    int numRows = PQntuples(res);
    // Переменная, определяющая количество столбцов
    int numColumns;
    if (numRows != 4) {
       numColumns = 3;
    } else {
       numColumns = 2;
    }

    // Создаем кнопки и размещаем их в layout
    for (int row = 0; row < numRows; ++row) {
        // Получаем наименование и таблицу
        QString table_name = QString(PQgetvalue(res, row, 0)).trimmed();
        QString name = QString(PQgetvalue(res, row, 1)).trimmed();


        // Создаем кнопку
        QPushButton *button = new QPushButton(name);
        add_button_func(button, table_name);

        // Вычисляем позицию кнопки в layout
        int rowIndex = row / numColumns;
        int columnIndex = row % numColumns;

        // Добавляем кнопку в layout
        layout->addWidget(button, rowIndex, columnIndex, Qt::AlignCenter);
    }

    // Освобождаем память, выделенную под результаты запросов
    PQclear(res);

    // Создаем виджет, в который поместим layout
    QWidget *widget = new QWidget;
    widget->setLayout(layout);

    // Устанавливаем layout в scrollArea_2
    ui->scrollArea_2->setWidget(widget);

    // Устанавливаем рамку (необязательно)
    ui->scrollArea_2->setFrameStyle(QFrame::Box | QFrame::Raised);
}


//_____________________________________________________________//

void MainWindow::on_user_lists_clicked()
{
    set_table_name("user_lists");
    create_list_buttons ();

}

void MainWindow::on_supermarket_clicked()
{
    set_table_name("supermarket");
    create_list_buttons ();

}

void MainWindow::on_consumables_clicked()
{
    set_table_name("consumables");
    create_list_buttons ();
}

void MainWindow::on_large_purchases_clicked()
{
    set_table_name("large_purchases");
    create_list_buttons ();
}

void MainWindow::on_holidays_clicked()
{
    set_table_name("holidays");
    create_list_buttons ();
}

void MainWindow::on_recipes_clicked()
{
    set_table_name("recipes");
    create_list_buttons ();
}

void MainWindow::on_new_list_clicked()
{
    // Кнопка будет добавлять список в категорию

}

