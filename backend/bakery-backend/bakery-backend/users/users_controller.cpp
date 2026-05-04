#include "users/users_controller.h"
#include "users/users_service.h"
#include "models/user_model.h"
#include "users/user_add_model.h"
#include "users/user_short_model.h"
#include "common/errors.h"
#include "common/string_utils.h"
#include "auth/auth_utils.h"
#include "common/utils.h"

#include <vector>

namespace users_controller {

	void setupRoutes(crow::App<crow::CORSHandler>& app) {
		CROW_ROUTE(app, "/users").methods(crow::HTTPMethod::Get) (
			[](const crow::request& req) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role != auth::Role::Admin) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					std::vector<User> users;
					if (auto err = users_service::getUsers(users)) {
						return common::errors::makeError(*err);
					}

					crow::json::wvalue arr(crow::json::type::List);

					size_t i = 0;
					for (const auto& user : users) {
						arr[i]["id"] = user.id;
						arr[i]["firstName"] = utils::utf16_to_utf8(user.firstName);
						arr[i]["lastName"] = utils::utf16_to_utf8(user.lastName);
						arr[i]["email"] = utils::utf16_to_utf8(user.email);
						arr[i]["role"]["id"] = user.role.id;
						if (auto roleNameOpt = utils::utf16_to_utf8_opt(user.role.name)) arr[i]["role"]["name"] = *roleNameOpt;
						else arr[i]["role"]["name"] = nullptr;
						i++;
					}

					crow::json::wvalue respJson;
					respJson["user"] = std::move(arr);

					return crow::response{ respJson };
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/users").methods(crow::HTTPMethod::Post) (
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

					if (jsonNotHasKeysAndisNull(bodyJson, { "firstName" }) || jsonNotHasKeysAndisNull(bodyJson, { "lastName" }) ||
						jsonNotHasKeysAndisNull(bodyJson, { "email" }) || jsonNotHasKeysAndisNull(bodyJson, { "password" }) ||
						jsonNotHasKeysAndisNull(bodyJson, { "role", "id" })) {
						return common::errors::makeError(common::errors::ErrorCode::ValidationError);
					}

					UserAdd userAdd;
					userAdd.firstName = utils::utf8_to_utf16(bodyJson["firstName"].s());
					userAdd.lastName = utils::utf8_to_utf16(bodyJson["lastName"].s());
					userAdd.email = utils::utf8_to_utf16(bodyJson["email"].s());
					userAdd.password = bodyJson["password"].s();
					userAdd.role.id = bodyJson["role"]["id"].i();

					if (auto err = users_service::insertUser(userAdd)) {
						return common::errors::makeError(*err);
					}

					return crow::response(201);
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/users/<int>").methods(crow::HTTPMethod::Get) (
			[](const crow::request& req, int id) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role != auth::Role::Admin) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					UserShort userShort;

					if (auto err = users_service::getUserShort(id, userShort)) {
						return common::errors::makeError(*err);
					}

					crow::json::wvalue respJson;
					respJson["email"] = utils::utf16_to_utf8(userShort.email);
					respJson["role"]["id"] = userShort.role.id;
					if (auto roleNameOpt = utils::utf16_to_utf8_opt(userShort.role.name)) {
						respJson["role"]["name"] = *roleNameOpt;
					}
					else {
						respJson["role"]["name"] = nullptr;
					}

					return crow::response{ respJson };
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/users/<int>").methods(crow::HTTPMethod::Delete) (
			[](const crow::request& req, int id) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role != auth::Role::Admin) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					if (auto err = users_service::deleteUser(id)) {
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