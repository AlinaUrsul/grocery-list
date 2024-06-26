# GroceryList_app
GroceryList - это десктопное приложение, написанное на C++ с использованием фреймворка Qt 5.15. Планируется портирование приложения на платформу Android и предоставление некоторого функционала через Telegram Bot API.
Приложение предназначено для упрощения составления списка покупок продуктов и других товаров. Оно основывается на уже готовых списках товаров по умолчанию из супермаркетов, позволяя пользователю легко составлять свой список покупок.
Для хранения и доступа к данным приложение использует базу данных PostgreSQL, которая запускается в контейнере Docker. Для взаимодействия с базой данных используется библиотека libpq-fe.h.
Проект собирается с использованием Makefile или CMake, обеспечивая гибкость и удобство в процессе разработки.

## Цели приложения:
- Упростить составление списка продуктов с помощью готовых списков товаров из супермаркетов.
- Хранить и организовывать список продуктов и других товаров.
- Разбивать списки на различные смысловые категории.
- Разделять списки по критериям срочности и важности.
- Хранить списки в качестве отдельных сущностей для рецептов.
- Позволять пользователям составлять собственные списки товаров.

## Дополнительные цели (планируемые в дальнейшем):
- Совместное использование списков: Создание возможности создания и совместного использования списков покупок с семьей или друзьями.
- Подсказки и рекомендации: Реализация функционала подсказок и рекомендаций по составлению списка на основе предыдущих покупок и предпочтений пользователей.
- Уведомления и напоминания: Внедрение уведомлений и напоминаний о планируемых покупках и списках покупок.
