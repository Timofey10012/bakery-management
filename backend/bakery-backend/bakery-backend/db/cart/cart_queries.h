#pragma once

#include "db/db.h"
#include "cart/cart_item_model.h"
#include "models/sales_item_model.h"

#include <vector>

namespace db {

	bool userExistsById(SQLHDBC dbc, const int id);

	bool getCartItems(SQLHDBC dbc, const int userId, std::vector<CartItem>& cartItems);

	bool productExistsById(SQLHDBC dbc, const int id);

	bool productQuantityIsMoreLimit(SQLHDBC dbc, const CartItem& cartItem);

	bool cartItemExistsByProductId(SQLHDBC dbc, const int userId, const int productId);

	bool insertCartItem(SQLHDBC dbc, const int userId, const CartItem& cartItem);

	bool deleteCartItem(SQLHDBC dbc, const int userId, const int productId);

	bool cartItemAddStock(SQLHDBC dbc, const int userId, const CartItem& cartItem);

	bool cartItemQuantityIsMoreLimit(SQLHDBC dbc, const int userId, const CartItem& cartItem);

	bool cartItemRemoveStock(SQLHDBC dbc, const int userId, const CartItem& cartItem);

	bool getSalesItemsFromCart(SQLHDBC dbc, const int userId, std::vector<SalesItem>& salesItems);

	bool salesItemQuantityIsMoreLimit(SQLHDBC dbc, const SalesItem& salesItem);

	bool insertSales(SQLHDBC dbc, const int userId, int& salesId);

	bool insertSales(SQLHDBC dbc, int& salesId);

	bool insertSalesItem(SQLHDBC dbc, const int salesId, const SalesItem& salesItem);

	bool productRemoveStock(SQLHDBC dbc, const int productId, const int quantity);

}