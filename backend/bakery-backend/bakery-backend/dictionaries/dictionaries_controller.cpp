#include "dictionaries/dictionaries_controller.h"
#include "dictionaries/dictionaries_service.h"
#include "models/role_model.h"
#include "models/UOM_model.h"
#include "models/warehouse_item_model.h"
#include "models/manufacturer_model.h"
#include "manufacturers/manufacturer_add_model.h"
#include "common/errors.h"
#include "common/string_utils.h"
#include "auth/auth_utils.h"
#include "common/utils.h"

namespace dictionaries_controller {
	void setupRoutes(crow::App<crow::CORSHandler>& app) {
		CROW_ROUTE(app, "/dictionaries/roles").methods(crow::HTTPMethod::Get) (
			[](const crow::request& req) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role == auth::Role::User) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					std::vector<Role> roles;
					if (auto err = dictionaries_service::getRoles(roles)) {
						return common::errors::makeError(*err);
					}

					crow::json::wvalue arr(crow::json::type::List);

					size_t i = 0;
					for (const auto& role : roles) {
						arr[i]["id"] = role.id;

						if (auto RoleNameOpt = utils::utf16_to_utf8_opt(role.name)) arr[i]["name"] = *RoleNameOpt;
						else arr[i]["name"] = nullptr;

						i++;
					}

					crow::json::wvalue respJson;
					respJson["role"] = std::move(arr);

					return crow::response{ respJson };
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/dictionaries/uoms").methods(crow::HTTPMethod::Get) (
			[](const crow::request& req) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role == auth::Role::User) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					std::vector<UOM> uoms;
					if (auto err = dictionaries_service::getUOM(uoms)) {
						return common::errors::makeError(*err);
					}

					crow::json::wvalue arr(crow::json::type::List);

					size_t i = 0;
					for (const auto& uom : uoms) {
						arr[i]["id"] = uom.id;

						if (auto UOMNameOpt = utils::utf16_to_utf8_opt(uom.name)) arr[i]["name"] = *UOMNameOpt;
						else arr[i]["name"] = nullptr;

						i++;
					}

					crow::json::wvalue respJson;
					respJson["UOM"] = std::move(arr);

					return crow::response{ respJson };
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/dictionaries/warehouseItems").methods(crow::HTTPMethod::Get) (
			[](const crow::request& req) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role == auth::Role::User) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					std::vector<WarehouseItem> warehouseItems;
					if (auto err = dictionaries_service::getWarehouseItems(warehouseItems)) {
						return common::errors::makeError(*err);
					}

					crow::json::wvalue arr(crow::json::type::List);

					size_t i = 0;
					for (const auto& warehouseItem : warehouseItems) {
						arr[i]["id"] = warehouseItem.id;

						if (auto warehouseItemNameOpt = utils::utf16_to_utf8_opt(warehouseItem.name)) arr[i]["name"] = *warehouseItemNameOpt;
						else arr[i]["name"] = nullptr;

						i++;
					}

					crow::json::wvalue respJson;
					respJson["warehouseItem"] = std::move(arr);

					return crow::response{ respJson };
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/dictionaries/manufacturers").methods(crow::HTTPMethod::Get) (
			[](const crow::request& req) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role == auth::Role::User) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					std::vector<Manufacturer> manufacturers;
					if (auto err = dictionaries_service::getManufacturer(manufacturers)) {
						return common::errors::makeError(*err);
					}

					crow::json::wvalue arr(crow::json::type::List);

					size_t i = 0;
					for (const auto& manufacturer : manufacturers) {
						arr[i]["id"] = manufacturer.id;

						if (auto manufacturerNameOpt = utils::utf16_to_utf8_opt(manufacturer.name)) arr[i]["name"] = *manufacturerNameOpt;
						else arr[i]["name"] = nullptr;

						i++;
					}

					crow::json::wvalue respJson;
					respJson["manufacturer"] = std::move(arr);

					return crow::response{ respJson };
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/dictionaries/manufacturers").methods(crow::HTTPMethod::Post) (
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

					if (jsonNotHasKeysAndisNull(bodyJson, { "name" })) {
						return common::errors::makeError(common::errors::ErrorCode::ValidationError);
					}

					ManufacturerAdd manufacturerAdd;
					manufacturerAdd.name = utils::utf8_to_utf16(bodyJson["name"].s());

					if (auto err = dictionaries_service::insertManufacturer(manufacturerAdd)) {
						return common::errors::makeError(*err);
					}

					return crow::response(201);
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/dictionaries/manufacturers/<int>").methods(crow::HTTPMethod::Delete) (
			[](const crow::request& req, int id) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role != auth::Role::Admin) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					if (auto err = dictionaries_service::deleteManufacturer(id)) {
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