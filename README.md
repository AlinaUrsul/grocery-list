# GroceryList_app
GroceryList is a desktop application written in C++ using the Qt 5.15 framework. There are plans to port the application to the Android platform and provide some functionality via the Telegram Bot API.

The application is designed to simplify the process of creating shopping lists for groceries and other items. It is based on pre-made default lists of items from supermarkets, allowing the user to easily create their own shopping list.

For data storage and access, the application uses a PostgreSQL database, which is run in a Docker container. The libpq-fe.h library is used for interacting with the database.

The project is built using Makefile or CMake, providing flexibility and convenience in the development process.

## Application Objectives:
- Simplify the process of creating grocery lists using pre-made lists of items from supermarkets.
- Store and organize lists of groceries and other items.
- Divide lists into various meaningful categories.
- Sort lists based on urgency and importance criteria.
- Store lists as separate entities for recipes.
- Allow users to create their own lists of items.

## Additional Objectives (planned for the future):
- Shared list usage: Creating the ability to create and share shopping lists with family or friends.
- Tips and recommendations: Implementing functionality for tips and recommendations for list composition based on previous purchases and user preferences.
- Notifications and reminders: Integrating notifications and reminders about planned purchases and shopping lists.
