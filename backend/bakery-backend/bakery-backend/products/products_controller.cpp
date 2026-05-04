#include "products/products_controller.h"
#include "products/product_model.h"
#include "products/product_item_full_model.h"
#include "models/ingredients_model.h"
#include "models/add_stock_model.h"
#include "products/products_service.h"
#include "common/errors.h"
#include "common/string_utils.h"
#include "auth/auth_utils.h"
#include "common/utils.h"

#include <sstream>
#include <iomanip>
#include <optional>
#include <string>
#include <vector>

namespace products_controller {

	void setupRoutes(crow::App<crow::CORSHandler>& app) {
		CROW_ROUTE(app, "/products").methods(crow::HTTPMethod::Get) (
			[](const crow::request& req) {
				try {
					std::vector<Product> products;
					if (auto err = products_service::getProducts(products)) {
						return common::errors::makeError(*err);
					}

					crow::json::wvalue arr(crow::json::type::List);

					size_t i = 0;
					for (const auto& product : products) {
						arr[i]["id"] = product.id;
						arr[i]["name"] = utils::utf16_to_utf8(product.name);
						if (auto imgURLOpt = product.imgURL) arr[i]["imgURL"] = *imgURLOpt;
						else arr[i]["imgURL"] = nullptr;
						arr[i]["quantityPerUnit"] = product.quantityPerUnit;
						arr[i]["UOM"]["id"] = product.uom.id;
						if (auto UOMNameOpt = utils::utf16_to_utf8_opt(product.uom.name)) arr[i]["UOM"]["name"] = *UOMNameOpt;
						else arr[i]["UOM"]["name"] = nullptr;

						std::ostringstream oss;
						oss << std::fixed << std::setprecision(2) << product.price / 100.0;
						arr[i]["price"] = oss.str();
						arr[i]["stockQuantity"] = product.stockQuantity;
						i++;
					}

					crow::json::wvalue respJson;
					respJson["product"] = std::move(arr);

					return crow::response{ respJson };
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/uploads/images/<string>")
			([](const std::string& filename) {
				try {
					std::string path = "uploads/images/" + filename;

					std::string body;

					if (auto err = products_service::getImage(path, body)) {
						return common::errors::makeError(*err);
					}

					crow::response res;

					res.set_header("Content-Type", "image/jpeg");
					res.body = std::move(body);

					return res;
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/products").methods(crow::HTTPMethod::Post) (
			[](const crow::request& req) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role == auth::Role::User) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					auto bodyJson = crow::json::load(req.body);
					if (!bodyJson) {
						return common::errors::makeError(common::errors::ErrorCode::ValidationError);
					}

					if (jsonNotHasKeysAndisNull(bodyJson, { "name" }) || jsonNotHasKeysAndisNull(bodyJson, { "quantityPerUnit" }) ||
						jsonNotHasKeysAndisNull(bodyJson, { "UOM", "id" }) || jsonNotHasKeysAndisNull(bodyJson, { "price" }) ||
						jsonNotHasKeysAndisNull(bodyJson, { "inStock" }) || jsonNotHasKeysAndisNull(bodyJson, { "description" }) ||
						jsonNotHasKeysAndisNull(bodyJson, { "caloriesPer100g" }) || jsonNotHasKeysAndisNull(bodyJson, { "expiration" }) ||
						jsonNotHasKeysAndisNull(bodyJson, { "tempStorage" }) || jsonNotHasKeysAndisNull(bodyJson, { "manufacturer", "id" }) ||
						jsonNotHasKeysAndisNull(bodyJson, { "ingredients" })) {
						return common::errors::makeError(common::errors::ErrorCode::ValidationError);
					}

					ProductItemFull prodItem;
					prodItem.name = utils::utf8_to_utf16(bodyJson["name"].s());
					prodItem.quantityPerUnit = bodyJson["quantityPerUnit"].i();
					prodItem.uom.id = bodyJson["UOM"]["id"].i();
					prodItem.price = bodyJson["price"].d() * 100.0;
					prodItem.inStock = bodyJson["inStock"].i();
					prodItem.description = utils::utf8_to_utf16(bodyJson["description"].s());
					prodItem.caloriesPer100g = bodyJson["caloriesPer100g"].i();
					prodItem.expiration = bodyJson["expiration"].i();
					prodItem.tempStorage = bodyJson["tempStorage"].i();
					prodItem.manufacturer.id = bodyJson["manufacturer"]["id"].i();

					auto ingredients = bodyJson["ingredients"];
					for (int i = 0; i < ingredients.size(); i++) {
						auto& item = ingredients[i];

						Ingredients ingredient;
						ingredient.warehouseItem.id = item["warehouseItem"]["id"].i();
						ingredient.quantity = item["quantity"].i();
						
						prodItem.ingredients.push_back(ingredient);
					}

					if (auto err = products_service::insertProductItem(prodItem)) {
						return common::errors::makeError(*err);
					}

					return crow::response(201);
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/products/<int>/image").methods(crow::HTTPMethod::Post) (
			[](const crow::request& req, int id) {
				try {

					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role == auth::Role::User) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					auto contentType = req.get_header_value("Content-Type");
					if (contentType.find("multipart/form-data") == std::string::npos) {
						return common::errors::makeError(common::errors::ErrorCode::ValidationError);
					}

					if (auto err = products_service::saveProductImage(id, req.body, contentType)) {
						return common::errors::makeError(*err);
					}

					return crow::response(201);
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/products/<int>").methods(crow::HTTPMethod::Get) (
			[](const crow::request& req, int id) {
				try {
					ProductItemFull prodItem;

					if (auto err = products_service::getProductItemFull(id, prodItem)) {
						return common::errors::makeError(*err);
					}

					crow::json::wvalue respJson;
					respJson["name"] = utils::utf16_to_utf8(prodItem.name);
					if (auto imgURLOpt = prodItem.imgURL) {
						respJson["imgURL"] = *imgURLOpt;
					}
					else {
						respJson["imgURL"] = nullptr;
					}
					respJson["quantityPerUnit"] = prodItem.quantityPerUnit;
					respJson["UOM"]["id"] = prodItem.uom.id;
					if (auto UOMNameOpt = utils::utf16_to_utf8_opt(prodItem.uom.name)) {
						respJson["UOM"]["name"] = *UOMNameOpt;
					}
					else {
						respJson["UOM"]["name"] = nullptr;
					}
					std::ostringstream oss;
					oss << std::fixed << std::setprecision(2) << prodItem.price / 100.0;
					respJson["price"] = oss.str();
					respJson["inStock"] = prodItem.inStock;
					respJson["description"] = utils::utf16_to_utf8(prodItem.description);
					respJson["caloriesPer100g"] = prodItem.caloriesPer100g;
					respJson["expiration"] = prodItem.expiration;
					respJson["tempStorage"] = prodItem.tempStorage;
					respJson["manufacturer"]["id"] = prodItem.manufacturer.id;
					if (auto manufacturerNameOpt = utils::utf16_to_utf8_opt(prodItem.manufacturer.name)) {
						respJson["manufacturer"]["name"] = *manufacturerNameOpt;
					}
					else {
						respJson["manufacturer"]["name"] = nullptr;
					}
					crow::json::wvalue ingredients(crow::json::type::List);
					
					size_t i = 0;
					for (auto ingredient : prodItem.ingredients) {
						ingredients[i]["warehouseItem"]["id"] = ingredient.warehouseItem.id;
						if (auto warehouseItemNameOpt = utils::utf16_to_utf8_opt(ingredient.warehouseItem.name)) {
							ingredients[i]["warehouseItem"]["name"] = *warehouseItemNameOpt;
						}
						else {
							ingredients[i]["warehouseItem"]["name"] = nullptr;
						}
						ingredients[i]["quantity"] = ingredient.quantity;
						if (auto UOMOpt = ingredient.uom) {
							ingredients[i]["UOM"]["id"] = UOMOpt->id;
							if (auto UOMNameOpt = utils::utf16_to_utf8_opt(UOMOpt->name)) {
								ingredients[i]["UOM"]["name"] = *UOMNameOpt;
							}
							else {
								ingredients[i]["UOM"]["name"] = nullptr;
							}
						}
						else {
							ingredients[i]["UOM"]["id"] = nullptr;
							ingredients[i]["UOM"]["name"] = nullptr;
						}
						i++;
					}

					respJson["ingredients"] = std::move(ingredients);

					return crow::response{ respJson };
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/products/<int>").methods(crow::HTTPMethod::Delete) (
			[](const crow::request& req, int id) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role == auth::Role::User) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					if (auto err = products_service::deleteProductItem(id)) {
						return common::errors::makeError(*err);
					}

					return crow::response(204);
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/products/<int>/add-stock").methods(crow::HTTPMethod::Patch) (
			[](const crow::request& req, int id) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role == auth::Role::User) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					auto bodyJson = crow::json::load(req.body);
					if (!bodyJson) {
						return common::errors::makeError(common::errors::ErrorCode::ValidationError);
					}

					if (jsonNotHasKeysAndisNull(bodyJson, { "quantity" })) {
						return common::errors::makeError(common::errors::ErrorCode::ValidationError);
					}

					AddStock addStock;
					addStock.quantity = bodyJson["quantity"].i();

					if (auto err = products_service::addStock(id, addStock)) {
						return common::errors::makeError(*err);
					}

					return crow::response(204);
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

	}
}