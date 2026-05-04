#include "cart/cart_controller.h"
#include "cart/cart_service.h"
#include "cart/cart_item_model.h"
#include "models/product_item_model.h"
#include "models/sales_item_model.h" 
#include "common/errors.h"
#include "common/string_utils.h"
#include "auth/auth_utils.h"
#include "common/utils.h"

namespace cart_controller {
	void setupRoutes(crow::App<crow::CORSHandler>& app) {
		CROW_ROUTE(app, "/cart").methods(crow::HTTPMethod::Get) (
			[](const crow::request& req) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role == auth::Role::Admin) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					std::vector<CartItem> cartItems;
					if (auto err = cart_service::getCartItems(authCtx.userId, cartItems)) {
						return common::errors::makeError(*err);
					}

					crow::json::wvalue arr(crow::json::type::List);

					size_t i = 0;
					for (const auto& cartItem : cartItems) {
						arr[i]["productItem"]["id"] = cartItem.productItem.id;

						if (auto productItemeNameOpt = utils::utf16_to_utf8_opt(cartItem.productItem.name)) arr[i]["productItem"]["name"] = *productItemeNameOpt;
						else arr[i]["productItem"]["name"] = nullptr;

						arr[i]["quantityItems"] = cartItem.quantityItems;

						i++;
					}

					crow::json::wvalue respJson;
					respJson["cartItem"] = std::move(arr);

					return crow::response{ respJson };
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/cart").methods(crow::HTTPMethod::Post) (
			[](const crow::request& req) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role == auth::Role::Admin) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					auto bodyJson = crow::json::load(req.body);
					if (!bodyJson) {
						return common::errors::makeError(common::errors::ErrorCode::ValidationError);
					}

					if (jsonNotHasKeysAndisNull(bodyJson, { "productItem", "id" }) || jsonNotHasKeysAndisNull(bodyJson, { "quantityItems" })) {
						return common::errors::makeError(common::errors::ErrorCode::ValidationError);
					}

					CartItem cartItem;
					cartItem.productItem.id = bodyJson["productItem"]["id"].i();
					cartItem.quantityItems = bodyJson["quantityItems"].i();

					if (auto err = cart_service::insertCartItem(authCtx.userId, cartItem)) {
						return common::errors::makeError(*err);
					}

					return crow::response(201);
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/cart").methods(crow::HTTPMethod::Delete) (
			[](const crow::request& req) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role == auth::Role::Admin) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					auto bodyJson = crow::json::load(req.body);
					if (!bodyJson) {
						return common::errors::makeError(common::errors::ErrorCode::ValidationError);
					}

					if (jsonNotHasKeysAndisNull(bodyJson, { "id" })) {
						return common::errors::makeError(common::errors::ErrorCode::ValidationError);
					}

					ProductItem productItem;
					productItem.id = bodyJson["id"].i();

					if (auto err = cart_service::deleteCartItem(authCtx.userId, productItem.id)) {
						return common::errors::makeError(*err);
					}

					return crow::response(204);
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/cart/add-stock").methods(crow::HTTPMethod::Patch) (
			[](const crow::request& req) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role == auth::Role::Admin) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					auto bodyJson = crow::json::load(req.body);
					if (!bodyJson) {
						return common::errors::makeError(common::errors::ErrorCode::ValidationError);
					}

					if (jsonNotHasKeysAndisNull(bodyJson, { "productItem", "id" }) || jsonNotHasKeysAndisNull(bodyJson, { "quantityItems" })) {
						return common::errors::makeError(common::errors::ErrorCode::ValidationError);
					}

					CartItem cartItem;
					cartItem.productItem.id = bodyJson["productItem"]["id"].i();
					cartItem.quantityItems = bodyJson["quantityItems"].i();

					if (auto err = cart_service::addStock(authCtx.userId, cartItem)) {
						return common::errors::makeError(*err);
					}

					return crow::response(204);
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/cart/remove-stock").methods(crow::HTTPMethod::Patch) (
			[](const crow::request& req) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role == auth::Role::Admin) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					auto bodyJson = crow::json::load(req.body);
					if (!bodyJson) {
						return common::errors::makeError(common::errors::ErrorCode::ValidationError);
					}

					if (jsonNotHasKeysAndisNull(bodyJson, { "productItem", "id" }) || jsonNotHasKeysAndisNull(bodyJson, { "quantityItems" })) {
						return common::errors::makeError(common::errors::ErrorCode::ValidationError);
					}

					CartItem cartItem;
					cartItem.productItem.id = bodyJson["productItem"]["id"].i();
					cartItem.quantityItems = bodyJson["quantityItems"].i();

					if (auto err = cart_service::removeStock(authCtx.userId, cartItem)) {
						return common::errors::makeError(*err);
					}

					return crow::response(204);
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/orders").methods(crow::HTTPMethod::Post) (
			[](const crow::request& req) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role == auth::Role::Admin) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					if (auto err = cart_service::orders(authCtx)) {
						return common::errors::makeError(*err);
					}

					return crow::response(201);
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

	}
}