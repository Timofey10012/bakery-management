#pragma once

#include "db/db.h"
#include "products/product_model.h"
#include "products/product_item_full_model.h"

#include <vector>

namespace db {

	bool getProducts(SQLHDBC dbc, std::vector<Product>& products);

	bool productExistsByName(SQLHDBC dbc, const std::wstring& name);

	bool UOMExistsByID(SQLHDBC dbc, const int& id);

	bool manufacturerExistsById(SQLHDBC dbc, const int id);

	bool ingredientsExistsById(SQLHDBC dbc, const int id);

	bool ingredientsQuantityIsMoreLimit(SQLHDBC dbc, const int inStock, const Ingredients& ingredient);

	bool insertProductItem(SQLHDBC dbc, int& productId, const ProductItemFull& prodItem);

	bool insertIngredients(SQLHDBC dbc, const int productId, const int ingredientsId, const int quantity);

	bool ingredientsQuantityRemoveStock(SQLHDBC dbc, const int inStock, const Ingredients& ingredient);

	bool updateProductImageURL(SQLHDBC dbc, const int productId, const std::wstring& savePath);

	bool productExistsById(SQLHDBC dbc, const int id);

	bool getProductItemFull(SQLHDBC dbc, const int id, ProductItemFull& prodItem);

	bool getIngredients(SQLHDBC dbc, const int id, std::vector<Ingredients>& ingredients);

	bool deleteCartItem(SQLHDBC dbc, const int productId);

	bool deleteIngredients(SQLHDBC dbc, const int id);

	bool deleteProductItem(SQLHDBC dbc, const int id);

	bool productItemAddStock(SQLHDBC dbc, const int id, const int quantity);

}