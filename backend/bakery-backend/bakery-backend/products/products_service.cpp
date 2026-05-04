#include "products/products_service.h"
#include "db/products/products_queries.h"

#include <fstream>
#include <string>
#include <sstream>

namespace products_service {

	namespace {
		bool getImageFile(const std::string& body, const std::string& contentType, std::string& fileData) {
			auto pos = contentType.find("boundary=");
			if (pos == std::string::npos) return false;

			std::string boundary = "--" + contentType.substr(pos + 9);

			size_t start = body.find(boundary);
			if (start == std::string::npos) return false;
			start += boundary.size() + 2;

			size_t end = body.find(boundary, start);
			if (end == std::string::npos) return false;

			std::string part = body.substr(start, end - start);

			size_t headerEnd = part.find("\r\n\r\n");
			if (headerEnd == std::string::npos) return false;
			headerEnd += 4;

			fileData = part.substr(headerEnd);
			if (fileData.size() >= 2 && fileData[fileData.size() - 2] == '\r' && fileData[fileData.size() - 1] == '\n') {
				fileData.resize(fileData.size() - 2);
			}

			return true;
		}

		bool insertImageFile(const int productId, const std::string fileData, std::wstring& savePath) {
			std::filesystem::create_directories("uploads/images");

			std::wstring filename = std::to_wstring(productId) + L".jpg";
			savePath = L"uploads/images/" + filename;

			std::ofstream out(savePath, std::ios::binary);
			if (!out.is_open()) return false;

			out.write(fileData.data(), fileData.size());
			out.close();

			return true;
		}
	}

	using common::errors::ErrorCode;

	std::optional<ErrorCode> getProducts(std::vector<Product>& products) {

		if (!db::getProducts(db::connection(), products)) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}

	std::optional<ErrorCode> getImage(const std::string& path, std::string& body) {
		
		std::ifstream file(path, std::ios::binary);
		if (!file.is_open()) {
			return ErrorCode::ValidationError;
		}

		std::ostringstream buffer;
		buffer << file.rdbuf();
		body = buffer.str();

		return std::nullopt;
	}

	std::optional<ErrorCode> insertProductItem(const ProductItemFull& prodItem) {
		if (prodItem.name.empty() || prodItem.description.empty()) {
			return ErrorCode::ValidationError;
		}

		if (prodItem.price < 0 || prodItem.price > 9999999999) {
			return ErrorCode::ValidationError;
		}

		if (db::productExistsByName(db::connection(), prodItem.name)) {
			return ErrorCode::ProductAlreadyExists;
		}

		if ((!db::UOMExistsByID(db::connection(), prodItem.uom.id)) || (!db::manufacturerExistsById(db::connection(), prodItem.manufacturer.id))) {
			return ErrorCode::ValidationError;
		}

		for (auto ingredient : prodItem.ingredients) {
			if (!db::ingredientsExistsById(db::connection(), ingredient.warehouseItem.id)) {
				return ErrorCode::ValidationError;
			}

			if (db::ingredientsQuantityIsMoreLimit(db::connection(), prodItem.inStock, ingredient)) {
				return ErrorCode::WarehouseItemInsufficientAmount;
			}
		}

		if (!db::beginTransaction(db::connection())) {
			return ErrorCode::InternalError;
		}
		bool success = true;
		
		int productId;
		if (!db::insertProductItem(db::connection(), productId, prodItem)) {
			success = false;
		}

		if (success) {
			for (auto ingredient : prodItem.ingredients) {
				if ((!db::insertIngredients(db::connection(), productId, ingredient.warehouseItem.id, ingredient.quantity)) || (!db::ingredientsQuantityRemoveStock(db::connection(), prodItem.inStock, ingredient))) {
					success = false;
					break;
				}
			}
		}

		if (!db::endTransaction(db::connection(), success)) {
			return ErrorCode::InternalError;
		}

		if (!success) return ErrorCode::InternalError;

		return std::nullopt;
	}

	std::optional<ErrorCode> saveProductImage(const int productId, const std::string& body, const std::string& contentType) {
		
		std::string fileData;
		if (!getImageFile(body, contentType, fileData)) {
			return ErrorCode::ValidationError;
		}

		std::wstring savePath;
		if (!insertImageFile(productId, fileData, savePath)) {
			return ErrorCode::InternalError;
		}

		if (!db::updateProductImageURL(db::connection(), productId, savePath)) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}

	std::optional<ErrorCode> getProductItemFull(const int productId, ProductItemFull& prodItem) {

		if (!db::productExistsById(db::connection(), productId)) {
			return ErrorCode::ProductNotFound;
		}

		if (!db::beginTransaction(db::connection())) {
			return ErrorCode::InternalError;
		}
		bool success = true;

		if ((!db::getProductItemFull(db::connection(), productId, prodItem)) || (!db::getIngredients(db::connection(), productId, prodItem.ingredients))) {
			success = false;
		}

		if (!db::endTransaction(db::connection(), success)) {
			return ErrorCode::InternalError;
		}

		if (!success) return ErrorCode::InternalError;

		return std::nullopt;
	}

	std::optional<ErrorCode> deleteProductItem(const int productId) {
		if (!db::productExistsById(db::connection(), productId)) {
			return ErrorCode::ProductNotFound;
		}
		
		if (!db::beginTransaction(db::connection())) {
			return ErrorCode::InternalError;
		}
		bool success = true;

		if ((!db::deleteCartItem(db::connection(), productId))) {
			success = false;
		}
		
		if ((!db::deleteIngredients(db::connection(), productId)) || (!db::deleteProductItem(db::connection(), productId))) {
			success = false;
		}

		if (!db::endTransaction(db::connection(), success)) {
			return ErrorCode::InternalError;
		}

		if (!success) return ErrorCode::InternalError;

		return std::nullopt;
	}

	std::optional<ErrorCode> addStock(const int productId, const AddStock& addStock) {
		if (!db::productExistsById(db::connection(), productId)) {
			return ErrorCode::ProductNotFound;
		}
		
		if (addStock.quantity < 0) {
			return ErrorCode::ValidationError;
		}

		std::vector<Ingredients> ingredients;

		if (!db::getIngredients(db::connection(), productId, ingredients)) {
			return ErrorCode::InternalError;
		}

		for (auto ingredient : ingredients) {
			if (db::ingredientsQuantityIsMoreLimit(db::connection(), addStock.quantity, ingredient)) {
				return ErrorCode::WarehouseItemInsufficientAmount;
			}
		}

		if (!db::beginTransaction(db::connection())) {
			return ErrorCode::InternalError;
		}
		bool success = true;

		for (auto ingredient : ingredients) {
			if (!db::ingredientsQuantityRemoveStock(db::connection(), addStock.quantity, ingredient)) {
				success = false;
			}
		}

		if (!db::productItemAddStock(db::connection(), productId, addStock.quantity)) {
			success = false;
		}

		if (!db::endTransaction(db::connection(), success)) {
			return ErrorCode::InternalError;
		}

		if (!success) return ErrorCode::InternalError;

		return std::nullopt;
	}

}