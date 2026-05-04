#pragma once

#include "products/product_model.h"
#include "products/product_item_full_model.h"
#include "models/add_stock_model.h"
#include "common/errors.h"

#include <optional>
#include <vector>

namespace products_service {
	using common::errors::ErrorCode;

	std::optional<ErrorCode> getProducts(std::vector<Product>& products);

	std::optional<ErrorCode> insertProductItem(const ProductItemFull& prodItem);

	std::optional<ErrorCode> saveProductImage(int productId, const std::string& body, const std::string& contentType);

	std::optional<ErrorCode> getImage(const std::string& path, std::string& body);

	std::optional<ErrorCode> getProductItemFull(const int productId, ProductItemFull& prodItem);

	std::optional<ErrorCode> deleteProductItem(const int productId);

	std::optional<ErrorCode> addStock(const int productId, const AddStock& addStock);

}