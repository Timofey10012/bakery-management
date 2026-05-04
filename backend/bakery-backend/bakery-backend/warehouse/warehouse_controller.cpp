#include "warehouse/warehouse_controller.h"
#include "warehouse/warehouse_service.h"
#include "warehouse/warehouse_model.h"
#include "warehouse/warehouse_item_add_model.h"
#include "warehouse/warehouse_item_short_model.h"
#include "models/add_stock_model.h"
#include "models/remove_stock_model.h"
#include "common/errors.h"
#include "common/string_utils.h"
#include "auth/auth_utils.h"
#include "common/utils.h"

namespace warehouse_controller  {
	void setupRoutes(crow::App<crow::CORSHandler>& app) {
		CROW_ROUTE(app, "/warehouse").methods(crow::HTTPMethod::Get) (
			[](const crow::request& req) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role == auth::Role::User) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					std::vector<Warehouse> warehouse;
					if (auto err = warehouse_service::getWarehouse(warehouse)) {
						return common::errors::makeError(*err);
					}

					crow::json::wvalue arr(crow::json::type::List);

					size_t i = 0;
					for (const auto& warehouseItem : warehouse) {
						arr[i]["id"] = warehouseItem.id;
						arr[i]["name"] = utils::utf16_to_utf8(warehouseItem.name);
						arr[i]["quantity"] = warehouseItem.quantity;
						arr[i]["minQuantity"] = warehouseItem.minQuantity;
						arr[i]["UOM"]["id"] = warehouseItem.uom.id;
						if (auto UOMNameOpt = utils::utf16_to_utf8_opt(warehouseItem.uom.name)) arr[i]["UOM"]["name"] = *UOMNameOpt;
						else arr[i]["UOM"]["name"] = nullptr;
						arr[i]["supplyDate"] = utils::toIsoString(warehouseItem.supplyDate);
						arr[i]["isInStock"] = warehouseItem.isInStock;
						i++;
					}

					crow::json::wvalue respJson;
					respJson["warehouseItemFull"] = std::move(arr);

					return crow::response{ respJson };
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/warehouse").methods(crow::HTTPMethod::Post) (
			[](const crow::request& req) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role != auth::Role::Admin) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					auto bodyJson = crow::json::load(req.body);
					if (!bodyJson) {
						return common::errors::makeError(common::errors::ErrorCode::ValidationError);
					}

					if (jsonNotHasKeysAndisNull(bodyJson, { "name" }) || jsonNotHasKeysAndisNull(bodyJson, { "quantity" }) ||
						jsonNotHasKeysAndisNull(bodyJson, { "minQuantity" }) || jsonNotHasKeysAndisNull(bodyJson, { "UOM", "id" })) {
						return common::errors::makeError(common::errors::ErrorCode::ValidationError);
					}

					WarehouseItemAdd warehouseItem;
					warehouseItem.name = utils::utf8_to_utf16(bodyJson["name"].s());
					warehouseItem.quantity = bodyJson["quantity"].i();
					warehouseItem.minQuantity = bodyJson["minQuantity"].i();
					warehouseItem.uom.id = bodyJson["UOM"]["id"].i();

					if (auto err = warehouse_service::insertWarehouseItem(warehouseItem)) {
						return common::errors::makeError(*err);
					}

					return crow::response(201);
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/warehouse/<int>").methods(crow::HTTPMethod::Get) (
			[](const crow::request& req, int id) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role == auth::Role::User) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					WarehouseItemShort warehouseItem;

					if (auto err = warehouse_service::getWarehouseItemShort(id, warehouseItem)) {
						return common::errors::makeError(*err);
					}

					crow::json::wvalue respJson;
					respJson["id"] = warehouseItem.id;
					respJson["name"] = utils::utf16_to_utf8(warehouseItem.name);
					respJson["quantity"] = warehouseItem.quantity;
					respJson["UOM"]["id"] = warehouseItem.uom.id;
					if (auto UOMNameOpt = utils::utf16_to_utf8_opt(warehouseItem.uom.name)) {
						respJson["UOM"]["name"] = *UOMNameOpt;
					}
					else {
						respJson["UOM"]["name"] = nullptr;
					}

					return crow::response{ respJson };
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/warehouse/<int>").methods(crow::HTTPMethod::Delete) (
			[](const crow::request& req, int id) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role != auth::Role::Admin) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					if (auto err = warehouse_service::deleteWarehouseItem(id)) {
						return common::errors::makeError(*err);
					}

					return crow::response(204);
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/warehouse/<int>/add-stock").methods(crow::HTTPMethod::Patch) (
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

					if (auto err = warehouse_service::addStock(id, addStock)) {
						return common::errors::makeError(*err);
					}

					return crow::response(204);
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/warehouse/<int>/remove-stock").methods(crow::HTTPMethod::Patch) (
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

					RemoveStock removeStock;
					removeStock.quantity = bodyJson["quantity"].i();

					if (auto err = warehouse_service::removeStock(id, removeStock)) {
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