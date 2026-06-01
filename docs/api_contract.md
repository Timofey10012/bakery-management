# Endpoints

| Method   | URL                              | Body model (Input)   | Headers                     | Response model (Output)   | Description                                                                        |
|:---------|:---------------------------------|:---------------------|:----------------------------|:--------------------------|:-----------------------------------------------------------------------------------|
| POST     | /auth/login                      | loginBody            | -                           | authResponse              | User authentication                                                                |
| POST     | /auth/register                   | registerBody         | -                           | authResponse              | User registration with the default role                                            |
| GET      | /auth/me                         | -                    | Authorization: Bearer token | user                      | Current user data                                                                  |
| GET      | /dashboard/summary               | -                    | Authorization: Bearer token | dashboardSummary          | Summary information for the main page                                              |
| GET      | /dashboard/activity              | -                    | Authorization: Bearer token | dashboardActivity[]       | Returns 3 activity items (a sale, a warehouse restock, a user addition)            |
| GET      | /products                        | -                    | -                           | product[]                 | List of products with abbreviated data                                             |
| POST     | /products                        | productItemFull      | Authorization: Bearer token | -                         | Add a product card                                                                 |
| POST     | /products/{id}/image             | productImage         | Authorization: Bearer token | -                         | Add a product image                                                                |
| GET      | /products/{id}                   | -                    | -                           | productItemFull           | Product card for a specific item                                                   |
| DELETE   | /products/{id}                   | -                    | Authorization: Bearer token | -                         | Delete a product card                                                              |
| PATCH    | /products/{id}/add-stock         | addStock             | Authorization: Bearer token | -                         | Increase the available stock quantity of a product                                 |
| GET      | /warehouse                       | -                    | Authorization: Bearer token | warehouseItemFull[]       | Warehouse items                                                                    |
| POST     | /warehouse                       | warehouseItemAdd     | Authorization: Bearer token | -                         | Add a warehouse item                                                               |
| GET      | /warehouse/{id}                  | -                    | Authorization: Bearer token | warehouseItemShort        | Brief information about a warehouse item                                           |
| DELETE   | /warehouse/{id}                  | -                    | Authorization: Bearer token | -                         | Delete a warehouse item                                                            |
| PATCH    | /warehouse/{id}/add-stock        | addStock             | Authorization: Bearer token | -                         | Increase the available quantity of a warehouse item                                |
| PATCH    | /warehouse/{id}/remove-stock     | removeStock          | Authorization: Bearer token | -                         | Write off quantity from a warehouse item                                           |
| GET      | /users                           | -                    | Authorization: Bearer token | user[]                    | List of users                                                                      |
| POST     | /users                           | userAdd              | Authorization: Bearer token | -                         | Add a user as an admin (a role can be assigned)                                    |
| GET      | /users/{id}                      | -                    | Authorization: Bearer token | userShort                 | Brief information about a user                                                     |
| DELETE   | /users/{id}                      | -                    | Authorization: Bearer token | -                         | Delete a user                                                                      |
| GET      | /sales                           | -                    | Authorization: Bearer token | saleShort[]               | List of sales with brief information                                               |
| GET      | /sales/{id}                      | -                    | Authorization: Bearer token | sale                      | Get full information about a specific sale                                         |
| POST     | /orders                          | -                    | Authorization: Bearer token | -                         | Purchase                                                                           |
| GET      | /cart                            | -                    | Authorization: Bearer token | cartItem[]                | Cart items                                                                         |
| POST     | /cart                            | cartItem             | Authorization: Bearer token | -                         | Add a product to the cart                                                          |
| DELETE   | /cart                            | productItem          | Authorization: Bearer token | -                         | Remove a product from the cart                                                     |
| PATCH    | /cart/add-stock                  | cartItem             | Authorization: Bearer token | -                         | Increase the quantity of a product in the cart                                     |
| PATCH    | /cart/remove-stock               | cartItem             | Authorization: Bearer token | -                         | Decrease the quantity of a product in the cart                                     |
| GET      | /dictionaries/roles              | -                    | Authorization: Bearer token | role[]                    | Roles dictionary                                                                   |
| GET      | /dictionaries/uoms               | -                    | Authorization: Bearer token | UOM[]                     | Units of measurement dictionary                                                    |
| GET      | /dictionaries/warehouseItems     | -                    | Authorization: Bearer token | warehouseItem[]           | Ingredients dictionary                                                             |
| GET      | /dictionaries/manufacturers      | -                    | Authorization: Bearer token | manufacturer[]            | Manufacturers dictionary                                                           |
| POST     | /dictionaries/manufacturers      | manufacturerAdd      | Authorization: Bearer token | -                         | Add a manufacturer                                                                 |
| DELETE   | /dictionaries/manufacturers/{id} | -                    | Authorization: Bearer token | -                         | Delete a manufacturer                                                              |

---

# Models

| Model                            | Field                | Type                  | Required   | Description                                                                                          |
|:---------------------------------|:---------------------|:----------------------|:-----------|:-----------------------------------------------------------------------------------------------------|
| loginBody                        | email                | string                | yes        | User email                                                                                           |
|                                  | password             | string                | yes        | User password                                                                                        |
| registerBody                     | firstName            | string                | yes        | First name                                                                                           |
|                                  | lastName             | string                | yes        | Last name                                                                                            |
|                                  | email                | string                | yes        | Email                                                                                                |
|                                  | password             | string                | yes        | Password                                                                                             |
| authResponse                     | token                | string                | yes        | JWT token                                                                                            |
|                                  | user                 | user                  | yes        | User data                                                                                            |
| user                             | id                   | int                   | yes        | ID                                                                                                   |
|                                  | firstName            | string                | yes        | First name                                                                                           |
|                                  | lastName             | string                | yes        | Last name                                                                                            |
|                                  | email                | string                | yes        | Email                                                                                                |
|                                  | role                 | role                  | yes        | Role                                                                                                 |
| userShort                        | email                | string                | yes        | Email                                                                                                |
|                                  | role                 | role                  | yes        | Role                                                                                                 |
| role                             | id                   | int                   | yes        | ID                                                                                                   |
|                                  | name                 | string                | no         | Role (may be omitted in request body)                                                                |
| dashboardSummary                 | salesToday           | decimal               | yes        | Total sales amount for today                                                                         |
|                                  | productsCount        | int                   | yes        | Number of products in the catalog                                                                    |
|                                  | usersCount           | int                   | yes        | Number of users                                                                                      |
|                                  | ordersToday          | int                   | yes        | Number of orders placed today                                                                        |
| dashboardActivity                | type                 | dashboardActivityType | yes        | Activity type                                                                                        |
|                                  | timestamp            | datetime              | no         | Event time \| null if absent                                                                         |
|                                  | data                 | dashboardActivityData | no         | Activity data (depends on type) \| null if absent                                                   |
| dashboardActivityType            | sale                 | string                | yes        | Sale                                                                                                 |
|                                  | userCreated          | string                | yes        | User addition                                                                                        |
|                                  | stockAdded           | string                | yes        | Warehouse stock change                                                                               |
| dashboardActivityDataSale        | total                | decimal               | yes        | Sale amount                                                                                          |
| dashboardActivityDataUserCreated | role                 | role                  | yes        | User role                                                                                            |
| dashboardActivityDataStockAdded  | warehouseItem        | warehouseItem         | yes        | Warehouse item being restocked                                                                       |
|                                  | amount               | int                   | yes        | Change amount                                                                                        |
|                                  | UOM                  | UOM                   | yes        | Unit of measurement                                                                                  |
| UOM                              | id                   | int                   | yes        | Unit of measurement ID                                                                               |
|                                  | name                 | string                | no         | Unit of measurement label (may be omitted in request body)                                           |
| product                          | id                   | int                   | yes        | Product ID                                                                                           |
|                                  | name                 | string                | yes        | Product name                                                                                         |
|                                  | imgURL               | string                | no         | Product image URL \| null if absent                                                                  |
|                                  | quantityPerUnit      | int                   | yes        | Product quantity relative to the unit of measurement                                                 |
|                                  | UOM                  | UOM                   | yes        | Unit of measurement                                                                                  |
|                                  | price                | decimal               | yes        | Price                                                                                                |
|                                  | stockQuantity        | int                   | yes        | Total in stock                                                                                       |
| productItemFull                  | name                 | string                | yes        | Product name                                                                                         |
|                                  | imgURL               | string                | no         | Product image URL \| null if absent (not provided in POST /products)                                 |
|                                  | quantityPerUnit      | int                   | yes        | Product quantity relative to the unit of measurement                                                 |
|                                  | UOM                  | UOM                   | yes        | Unit of measurement                                                                                  |
|                                  | price                | decimal               | yes        | Price                                                                                                |
|                                  | inStock              | int                   | yes        | Total stock quantity                                                                                  |
|                                  | description          | string                | yes        | Description                                                                                          |
|                                  | caloriesPer100g      | int                   | yes        | Calories per 100 grams                                                                               |
|                                  | expiration           | int                   | yes        | Shelf life depending on temperature (in days)                                                        |
|                                  | tempStorage          | int                   | yes        | Storage temperature                                                                                  |
|                                  | manufacturer         | manufacturer          | yes        | Manufacturer                                                                                         |
|                                  | ingredients[]        | ingredients           | yes        | Ingredients                                                                                          |
| productImage                     | img                  | file                  | yes        | Product image                                                                                        |
| productItem                      | id                   | int                   | yes        | Product ID                                                                                           |
|                                  | name                 | string                | no         | Product name (may be omitted in request body)                                                        |
| manufacturer                     | id                   | int                   | yes        | Manufacturer ID                                                                                      |
|                                  | name                 | string                | no         | Manufacturer name (may be omitted in request body)                                                   |
| manufacturerAdd                  | name                 | string                | yes        | Name of the manufacturer to add                                                                      |
| ingredients                      | warehouseItem        | warehouseItem         | yes        | Warehouse item                                                                                       |
|                                  | quantity             | int                   | yes        | Warehouse item quantity relative to the unit of measurement                                          |
|                                  | UOM                  | UOM                   | no         | Unit of measurement (may be omitted when adding a product card)                                      |
| addStock                         | quantity             | int                   | yes        | Quantity to add                                                                                      |
| removeStock                      | quantity             | int                   | yes        | Quantity to write off                                                                                |
| warehouseItem                    | id                   | int                   | yes        | Warehouse item ID                                                                                    |
|                                  | name                 | string                | no         | Warehouse item name (may be omitted in request body)                                                 |
| warehouseItemFull                | id                   | int                   | yes        | Warehouse item ID                                                                                    |
|                                  | name                 | string                | yes        | Item name                                                                                            |
|                                  | quantity             | int                   | yes        | Available quantity                                                                                   |
|                                  | minQuantity          | int                   | yes        | Minimum allowed quantity                                                                             |
|                                  | UOM                  | UOM                   | yes        | Unit of measurement                                                                                  |
|                                  | supplyDate           | datetime              | yes        | Date of last change                                                                                  |
|                                  | isInStock            | bool                  | yes        | true if the available quantity of the warehouse item is at or above the minimum threshold, otherwise false |
| warehouseItemShort               | id                   | int                   | yes        | Warehouse item ID                                                                                    |
|                                  | name                 | string                | yes        | Item name                                                                                            |
|                                  | quantity             | int                   | yes        | Available quantity                                                                                   |
|                                  | UOM                  | UOM                   | yes        | Unit of measurement                                                                                  |
| warehouseItemAdd                 | name                 | string                | yes        | Item name                                                                                            |
|                                  | quantity             | int                   | yes        | Available quantity                                                                                   |
|                                  | minQuantity          | int                   | yes        | Minimum allowed quantity                                                                             |
|                                  | UOM                  | UOM                   | yes        | Unit of measurement                                                                                  |
| userAdd                          | firstName            | string                | yes        | First name                                                                                           |
|                                  | lastName             | string                | yes        | Last name                                                                                            |
|                                  | email                | string                | yes        | Email                                                                                                |
|                                  | password             | string                | yes        | Password                                                                                             |
|                                  | role                 | role                  | yes        | Role                                                                                                 |
| saleShort                        | id                   | int                   | yes        | Sale ID                                                                                              |
|                                  | user                 | user                  | no         | User who made the purchase; no — if the purchase was made offline, user is not returned              |
|                                  | total                | decimal               | yes        | Total sale amount                                                                                    |
| sale                             | user                 | user                  | no         | User who made the purchase; no — if the purchase was made offline, user is not returned              |
|                                  | salesItem[]          | salesItem             | yes        | Sold products                                                                                        |
|                                  | total                | decimal               | yes        | Total sale amount                                                                                    |
| salesItem                        | productItem          | productItem           | yes        | Purchased product                                                                                    |
|                                  | quantityItems        | int                   | yes        | Quantity of the purchased product                                                                    |
|                                  | pricePerUnit         | decimal               | yes        | Price per unit of the purchased product                                                              |
|                                  | totalPricePerProduct | decimal               | yes        | Total price for the entire purchased quantity                                                        |
| cartItem                         | productItem          | productItem           | yes        | Product being added to the cart                                                                      |
|                                  | quantityItems        | int                   | yes        | Product quantity                                                                                     |

---

# Errors

|   http status | Error                                 | Error description                                                              |
|--------------:|:--------------------------------------|:-------------------------------------------------------------------------------|
|           400 | VALIDATION_ERROR                      | Input data failed validation                                                   |
|           401 | UNAUTHORIZED                          | Authentication failed                                                          |
|           403 | FORBIDDEN                             | Insufficient permissions to perform the operation                              |
|           404 | ENTITY_NOT_FOUND                      | The requested entity was not found                                             |
|           409 | CONFLICT                              | Data conflict while performing the operation                                   |
|           500 | INTERNAL_ERROR                        | Internal server error                                                          |
|           404 | USER_NOT_FOUND                        | No user found with the specified id                                            |
|           404 | ROLE_NOT_FOUND                        | No role found with the specified id                                            |
|           404 | UOM_NOT_FOUND                         | No unit of measurement found with the specified id                             |
|           404 | AUTH_LOGIN_EMAIL_NOT_FOUND            | No user found with the specified email                                         |
|           401 | AUTH_LOGIN_INVALID_PASSWORD           | Incorrect password                                                             |
|           409 | AUTH_REGISTER_EMAIL_ALREADY_EXISTS    | A user with this email is already registered                                   |
|           400 | AUTH_REGISTER_PASSWORD_NOT_VALID      | Password does not meet security requirements                                   |
|           404 | PRODUCT_NOT_FOUND                     | No product card found with the specified id                                    |
|           409 | PRODUCT_ALREADY_EXISTS                | A product card with this name already exists                                   |
|           409 | PRODUCT_INSUFFICIENT_STOCK            | Insufficient product stock to perform the operation                            |
|           404 | WAREHOUSE_ITEM_NOT_FOUND              | No warehouse item found with the specified id                                  |
|           409 | WAREHOUSE_ITEM_ALREADY_EXISTS         | A warehouse item already exists                                                |
|           409 | WAREHOUSE_ITEM_USED_IN_PRODUCT        | The warehouse item is used in a product card                                   |
|           409 | WAREHOUSE_ITEM_INSUFFICIENT_AMOUNT    | Insufficient quantity of the warehouse item                                    |
|           404 | SALE_NOT_FOUND                        | No sale found with the specified id                                            |
|           409 | SALE_ITEM_INSUFFICIENT_STOCK          | The quantity of the item being sold exceeds what is available in the warehouse |
|           409 | ORDERS_INSUFFICIENT_STOCK_AT_CHECKOUT | Insufficient product stock at the time of checkout                             |
|           409 | CART_ITEM_INSUFFICIENT_STOCK          | Insufficient product quantity in the cart                                      |
|           404 | MANUFACTURER_NOT_FOUND                | No manufacturer found with the specified id                                    |
|           409 | MANUFACTURER_ALREADY_EXISTS           | A manufacturer with this name already exists                                   |

---
