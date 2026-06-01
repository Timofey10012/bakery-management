# Endpoints

| Method   | URL                              | Body model (Входные данные)   | Headers                     | Response model (Выходные данные)   | Описание                                                                               |
|:---------|:---------------------------------|:------------------------------|:----------------------------|:-----------------------------------|:---------------------------------------------------------------------------------------|
| POST     | /auth/login                      | loginBody                     | -                           | authResponse                       | Авторизация пользователя                                                               |
| POST     | /auth/register                   | registerBody                  | -                           | authResponse                       | Регистрация пользователя с ролью по умолчанию                                          |
| GET      | /auth/me                         | -                             | Authorization: Bearer token | user                               | Данные текущего пользователя                                                           |
| GET      | /dashboard/summary               | -                             | Authorization: Bearer token | dashboardSummary                   | Сводка информации для главной страницы                                                 |
| GET      | /dashboard/activity              | -                             | Authorization: Bearer token | dashboardActivity[]                | Возвращает 3 элемента активности (продажу, пополнение склада, добавление пользователя) |
| GET      | /products                        | -                             | -                           | product[]                          | Список товаров с сокращенными данными                                                  |
| POST     | /products                        | productItemFull               | Authorization: Bearer token | -                                  | Добавление карточки товара                                                             |
| POST     | /products/{id}/image             | productImage                  | Authorization: Bearer token | -                                  | Добавление картинки товара                                                             |
| GET      | /products/{id}                   | -                             | -                           | productItemFull                    | Карточка конкретного товара                                                            |
| DELETE   | /products/{id}                   | -                             | Authorization: Bearer token | -                                  | Удаление карточки товара                                                               |
| PATCH    | /products/{id}/add-stock         | addStock                      | Authorization: Bearer token | -                                  | Увеличение объема товара в наличии                                                     |
| GET      | /warehouse                       | -                             | Authorization: Bearer token | warehouseItemFull[]                | Позиции склада                                                                         |
| POST     | /warehouse                       | warehouseItemAdd              | Authorization: Bearer token | -                                  | Добавление позиции склада                                                              |
| GET      | /warehouse/{id}                  | -                             | Authorization: Bearer token | warehouseItemShort                 | Краткая информация о позиции склада                                                    |
| DELETE   | /warehouse/{id}                  | -                             | Authorization: Bearer token | -                                  | Удаление позиции склада                                                                |
| PATCH    | /warehouse/{id}/add-stock        | addStock                      | Authorization: Bearer token | -                                  | Увеличение объема позиции в наличии                                                    |
| PATCH    | /warehouse/{id}/remove-stock     | removeStock                   | Authorization: Bearer token | -                                  | Списание объема позиции в наличии                                                      |
| GET      | /users                           | -                             | Authorization: Bearer token | user[]                             | Список пользователей                                                                   |
| POST     | /users                           | userAdd                       | Authorization: Bearer token | -                                  | Добавление пользователя админом (можно назначить роль)                                 |
| GET      | /users/{id}                      | -                             | Authorization: Bearer token | userShort                          | Краткая информация о пользователе                                                      |
| DELETE   | /users/{id}                      | -                             | Authorization: Bearer token | -                                  | Удаление пользователя                                                                  |
| GET      | /sales                           | -                             | Authorization: Bearer token | saleShort[]                        | Список продаж  с краткой информацией                                                   |
| GET      | /sales/{id}                      | -                             | Authorization: Bearer token | sale                               | Получение полной информации о конкретной продаже                                       |
| POST     | /orders                          | -                             | Authorization: Bearer token | -                                  | Покупка                                                                                |
| GET      | /cart                            | -                             | Authorization: Bearer token | cartItem[]                         | Элементы корзины                                                                       |
| POST     | /cart                            | cartItem                      | Authorization: Bearer token | -                                  | Добавить товар в корзину                                                               |
| DELETE   | /cart                            | productItem                   | Authorization: Bearer token | -                                  | Удаление товара из корзины                                                             |
| PATCH    | /cart/add-stock                  | cartItem                      | Authorization: Bearer token | -                                  | Увеличение объема товара в корзине                                                     |
| PATCH    | /cart/remove-stock               | cartItem                      | Authorization: Bearer token | -                                  | Списание объема товара в корзине                                                       |
| GET      | /dictionaries/roles              | -                             | Authorization: Bearer token | role[]                             | Словарь ролей                                                                          |
| GET      | /dictionaries/uoms               | -                             | Authorization: Bearer token | UOM[]                              | Словарь единиц измерения                                                               |
| GET      | /dictionaries/warehouseItems     | -                             | Authorization: Bearer token | warehouseItem[]                    | Словарь ингредиентов                                                                   |
| GET      | /dictionaries/manufacturers      | -                             | Authorization: Bearer token | manufacturer[]                     | Словарь производителей                                                                 |
| POST     | /dictionaries/manufacturers      | manufacturerAdd               | Authorization: Bearer token | -                                  | Добавление производителя                                                               |
| DELETE   | /dictionaries/manufacturers/{id} | -                             | Authorization: Bearer token | -                                  | Удаление производителя                                                                 |

---

# Models

| Model                            | Field                | Type                  | Required   | Описание                                                                                   |
|:---------------------------------|:---------------------|:----------------------|:-----------|:-------------------------------------------------------------------------------------------|
| loginBody                        | email                | string                | yes        | Почта пользователя                                                                         |
|                                  | password             | string                | yes        | Пароль пользователя                                                                        |
| registerBody                     | firstName            | string                | yes        | Имя                                                                                        |
|                                  | lastName             | string                | yes        | Фамилия                                                                                    |
|                                  | email                | string                | yes        | Почта                                                                                      |
|                                  | password             | string                | yes        | Пароль                                                                                     |
| authResponse                     | token                | string                | yes        | JWT токен                                                                                  |
|                                  | user                 | user                  | yes        | Данные пользователя                                                                        |
| user                             | id                   | int                   | yes        | ID                                                                                         |
|                                  | firstName            | string                | yes        | Имя                                                                                        |
|                                  | lastName             | string                | yes        | Фамилия                                                                                    |
|                                  | email                | string                | yes        | Почта                                                                                      |
|                                  | role                 | role                  | yes        | Роль                                                                                       |
| userShort                        | email                | string                | yes        | Почта                                                                                      |
|                                  | role                 | role                  | yes        | Роль                                                                                       |
| role                             | id                   | int                   | yes        | ID                                                                                         |
|                                  | name                 | string                | no         | Роль (во входных данных можно не указывать)                                                |
| dashboardSummary                 | salesToday           | decimal               | yes        | Сумма продаж сделанных сегодня                                                             |
|                                  | productsCount        | int                   | yes        | Количество товаров в каталоге                                                              |
|                                  | usersCount           | int                   | yes        | Количество пользователей                                                                   |
|                                  | ordersToday          | int                   | yes        | Количество заказов сделанных сегодня                                                       |
| dashboardActivity                | type                 | dashboardActivityType | yes        | Тип активности                                                                             |
|                                  | timestamp            | datetime              | no         | Время события | null при отсутсвии                                                         |
|                                  | data                 | dashboardActivityData | no         | Данные активности (зависят от type) | null при отсутсвии                                   |
| dashboardActivityType            | sale                 | string                | yes        | Продажа                                                                                    |
|                                  | userCreated          | string                | yes        | Добавление пользователя                                                                    |
|                                  | stockAdded           | string                | yes        | Изменение объема склада                                                                    |
| dashboardActivityDataSale        | total                | decimal               | yes        | Сумма продажи                                                                              |
| dashboardActivityDataUserCreated | role                 | role                  | yes        | Роль пользователя                                                                          |
| dashboardActivityDataStockAdded  | warehouseItem        | warehouseItem         | yes        | Пополняемая позиция склада                                                                 |
|                                  | amount               | int                   | yes        | Объем изменений                                                                            |
|                                  | UOM                  | UOM                   | yes        | Единица измерения                                                                          |
| UOM                              | id                   | int                   | yes        | ID обозначения единицы измерения                                                           |
|                                  | name                 | string                | no         | Обозначение единицы измерения (во входных данных можно не указывать)                       |
| product                          | id                   | int                   | yes        | ID товара                                                                                  |
|                                  | name                 | string                | yes        | Название товара                                                                            |
|                                  | imgURL               | string                | no         | URL картинки товара | null при отсутсвии                                                   |
|                                  | quantityPerUnit      | int                   | yes        | Количество товара в зависимости от единицы измерения                                       |
|                                  | UOM                  | UOM                   | yes        | Единица измерения                                                                          |
|                                  | price                | decimal               | yes        | Цена                                                                                       |
|                                  | stockQuantity        | int                   | yes        | Всего в наличии                                                                            |
| productItemFull                  | name                 | string                | yes        | Название товара                                                                            |
|                                  | imgURL               | string                | no         | URL картинки товара | null при отсутсвии (при POST /products не указывается)               |
|                                  | quantityPerUnit      | int                   | yes        | Количество товара в зависимости от единицы измерения                                       |
|                                  | UOM                  | UOM                   | yes        | Единица измерения                                                                          |
|                                  | price                | decimal               | yes        | Цена                                                                                       |
|                                  | inStock              | int                   | yes        | Всего товара                                                                               |
|                                  | description          | string                | yes        | Описание                                                                                   |
|                                  | caloriesPer100g      | int                   | yes        | Калорийность на 100 грамм                                                                  |
|                                  | expiration           | int                   | yes        | Срок годности в зависимости от температуры (в днях)                                        |
|                                  | tempStorage          | int                   | yes        | Температура хранения                                                                       |
|                                  | manufacturer         | manufacturer          | yes        | Производитель                                                                              |
|                                  | ingredients[]        | ingredients           | yes        | Состав                                                                                     |
| productImage                     | img                  | file                  | yes        | Картинка товара                                                                            |
| productItem                      | id                   | int                   | yes        | ID товара                                                                                  |
|                                  | name                 | string                | no         | Название товара (во входных данных можно не указывать)                                     |
| manufacturer                     | id                   | int                   | yes        | ID производителя                                                                           |
|                                  | name                 | string                | no         | Название производителя (во входных данных можно не указывать)                              |
| manufacturerAdd                  | name                 | string                | yes        | Название добавляемого производителя                                                        |
| ingredients                      | warehouseItem        | warehouseItem         | yes        | Позиция склада                                                                             |
|                                  | quantity             | int                   | yes        | Количество позиции склада в зависимости от единицы измерения                               |
|                                  | UOM                  | UOM                   | no         | Единица измерения (при добавлении карточки товара можно не указывать)                      |
| addStock                         | quantity             | int                   | yes        | Количество для добавления                                                                  |
| removeStock                      | quantity             | int                   | yes        | Количество для списания                                                                    |
| warehouseItem                    | id                   | int                   | yes        | ID позиции склада                                                                          |
|                                  | name                 | string                | no         | Название позиции склада (во входных данных можно не указывать)                             |
| warehouseItemFull                | id                   | int                   | yes        | ID позиции на складе                                                                       |
|                                  | name                 | string                | yes        | Название позиции                                                                           |
|                                  | quantity             | int                   | yes        | Количество в наличии                                                                       |
|                                  | minQuantity          | int                   | yes        | Минимальное допустимое количество                                                          |
|                                  | UOM                  | UOM                   | yes        | Единица измерения                                                                          |
|                                  | supplyDate           | datetime              | yes        | Дата последних изменений                                                                   |
|                                  | isInStock            | bool                  | yes        | true, если количество в наличии позиции склада не меньше минимального порога, иначе false  |
| warehouseItemShort               | id                   | int                   | yes        | ID позиции на складе                                                                       |
|                                  | name                 | string                | yes        | Название позиции                                                                           |
|                                  | quantity             | int                   | yes        | Количество в наличии                                                                       |
|                                  | UOM                  | UOM                   | yes        | Единица измерения                                                                          |
| warehouseItemAdd                 | name                 | string                | yes        | Название позиции                                                                           |
|                                  | quantity             | int                   | yes        | Количество в наличии                                                                       |
|                                  | minQuantity          | int                   | yes        | Минимальное допустимое количество                                                          |
|                                  | UOM                  | UOM                   | yes        | Единица измерения                                                                          |
| userAdd                          | firstName            | string                | yes        | Имя                                                                                        |
|                                  | lastName             | string                | yes        | Фамилия                                                                                    |
|                                  | email                | string                | yes        | Почта                                                                                      |
|                                  | password             | string                | yes        | Пароль                                                                                     |
|                                  | role                 | role                  | yes        | Роль                                                                                       |
| saleShort                        | id                   | int                   | yes        | ID продажи                                                                                 |
|                                  | user                 | user                  | no         | Пользователь совершивший покупку, no - если покупка совершена оффлайн - user не передается |
|                                  | total                | decimal               | yes        | Сумма сделанной продажи                                                                    |
| sale                             | user                 | user                  | no         | Пользователь совершивший покупку, no - если покупка совершена оффлайн - user не передается |
|                                  | salesItem[]          | salesItem             | yes        | Проданные товары                                                                           |
|                                  | total                | decimal               | yes        | Сумма сделанной продажи                                                                    |
| salesItem                        | productItem          | productItem           | yes        | Купленный товар                                                                            |
|                                  | quantityItems        | int                   | yes        | Количество купленного товара                                                               |
|                                  | pricePerUnit         | decimal               | yes        | Цена за единицу купленного товара                                                          |
|                                  | totalPricePerProduct | decimal               | yes        | Цена за весь купленный товар                                                               |
| cartItem                         | productItem          | productItem           | yes        | Товар добавляемый в корзину                                                                |
|                                  | quantityItems        | int                   | yes        | Количество товара                                                                          |

---

# Errors

|   http статус | Ошибка                                | Описание ошибки                                                        |
|--------------:|:--------------------------------------|:-----------------------------------------------------------------------|
|           400 | VALIDATION_ERROR                      | Входные данные не прошли валидацию                                     |
|           401 | UNAUTHORIZED                          | Неуспешная аутентификация                                              |
|           403 | FORBIDDEN                             | Недостаточно прав для выполнения операции                              |
|           404 | ENTITY_NOT_FOUND                      | Запрашиваемая сущность не найдена                                      |
|           409 | CONFLICT                              | Конфликт данных при выполнении операции                                |
|           500 | INTERNAL_ERROR                        | Внутренняя ошибка сервера                                              |
|           404 | USER_NOT_FOUND                        | Пользователь с указанным id не найден                                  |
|           404 | ROLE_NOT_FOUND                        | Роль с указанным id не найдена                                         |
|           404 | UOM_NOT_FOUND                         | Единица измерения с указанным id не найдена                            |
|           404 | AUTH_LOGIN_EMAIL_NOT_FOUND            | Пользователь с указанным email не найден                               |
|           401 | AUTH_LOGIN_INVALID_PASSWORD           | Неверный пароль                                                        |
|           409 | AUTH_REGISTER_EMAIL_ALREADY_EXISTS    | Пользователь с таким email уже зарегистрирован                         |
|           400 | AUTH_REGISTER_PASSWORD_NOT_VALID      | Пароль не соответствует требованиям безопасности                       |
|           404 | PRODUCT_NOT_FOUND                     | Карточка товара с указанным id не найдена                              |
|           409 | PRODUCT_ALREADY_EXISTS                | Карточка товара с таким названием уже существует                       |
|           409 | PRODUCT_INSUFFICIENT_STOCK            | Количество товара на складе недостаточно для выполнения операции       |
|           404 | WAREHOUSE_ITEM_NOT_FOUND              | Позиция склада с указанным id не найдена                               |
|           409 | WAREHOUSE_ITEM_ALREADY_EXISTS         | Позиция склада уже существует                                          |
|           409 | WAREHOUSE_ITEM_USED_IN_PRODUCT        | Позиция склада используется в карточке товара                          |
|           409 | WAREHOUSE_ITEM_INSUFFICIENT_AMOUNT    | Недостаточное количество позиции на складе                             |
|           404 | SALE_NOT_FOUND                        | Продажа с указанным id не найдена                                      |
|           409 | SALE_ITEM_INSUFFICIENT_STOCK          | Количество продаваемого товара превышает доступное на складе           |
|           409 | ORDERS_INSUFFICIENT_STOCK_AT_CHECKOUT | Недостаточное количество товара на складе на момент оформления покупки |
|           409 | CART_ITEM_INSUFFICIENT_STOCK          | Недостаточное количество товара в корзине                              |
|           404 | MANUFACTURER_NOT_FOUND                | Производитель с указанным id не найден                                 |
|           409 | MANUFACTURER_ALREADY_EXISTS           | Производитель с таким названием уже существует                         |

---

