#include "sales/sales_controller.h"
#include "sales/sales_service.h"
#include "sales/sale_short_model.h"
#include "sales/sale_model.h"
#include "common/errors.h"
#include "common/string_utils.h"
#include "auth/auth_utils.h"
#include "common/utils.h"

#include <sstream>
#include <iomanip>
#include <vector>

namespace sales_controller {

	void setupRoutes(crow::App<crow::CORSHandler>& app) {
		CROW_ROUTE(app, "/sales").methods(crow::HTTPMethod::Get) (
			[](const crow::request& req) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role != auth::Role::Admin) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					std::vector<SaleShort> sales;
					if (auto err = sales_service::getSaleShort(sales)) {
						return common::errors::makeError(*err);
					}

					crow::json::wvalue arr(crow::json::type::List);

					size_t i = 0;
					for (const auto& sale : sales) {
						arr[i]["id"] = sale.id;
						if (auto userOpt = (sale.user)) {
							arr[i]["user"]["id"] = userOpt->id;
							arr[i]["user"]["firstName"] = utils::utf16_to_utf8(userOpt->firstName);
							arr[i]["user"]["lastName"] = utils::utf16_to_utf8(userOpt->lastName);
							arr[i]["user"]["email"] = utils::utf16_to_utf8(userOpt->email);
							arr[i]["user"]["role"]["id"] = userOpt->role.id;
							if (auto userRoleNameOpt = utils::utf16_to_utf8_opt(userOpt->role.name)) arr[i]["user"]["role"]["name"] = *userRoleNameOpt;
							else arr[i]["user"]["role"]["name"] = nullptr;
						}
						else {
							arr[i]["user"] = nullptr;
						}
						std::ostringstream oss;
						oss << std::fixed << std::setprecision(2) << sale.total / 100.0;
						arr[i]["total"] = oss.str();
						i++;
					}

					crow::json::wvalue respJson;
					respJson["saleShort"] = std::move(arr);

					return crow::response{ respJson };
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

		CROW_ROUTE(app, "/sales/<int>").methods(crow::HTTPMethod::Get) (
			[](const crow::request& req, int id) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role != auth::Role::Admin) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					Sale sale;
					if (auto err = sales_service::getSale(id, sale)) {
						return common::errors::makeError(*err);
					}

					crow::json::wvalue respJson;

					if (auto userOpt = (sale.user)) {
						respJson["user"]["id"] = userOpt->id;
						respJson["user"]["firstName"] = utils::utf16_to_utf8(userOpt->firstName);
						respJson["user"]["lastName"] = utils::utf16_to_utf8(userOpt->lastName);
						respJson["user"]["email"] = utils::utf16_to_utf8(userOpt->email);
						respJson["user"]["role"]["id"] = userOpt->role.id;
						if (auto userRoleNameOpt = utils::utf16_to_utf8_opt(userOpt->role.name)) respJson["user"]["role"]["name"] = *userRoleNameOpt;
						else respJson["user"]["role"]["name"] = nullptr;
					}
					else {
						respJson["user"] = nullptr;
					}

					crow::json::wvalue arr(crow::json::type::List);

					size_t i = 0;
					for (const auto& salesItem : sale.salesItems) {
						arr[i]["productItem"]["id"] = salesItem.productItem.id;
						if (auto productItemeNameOpt = utils::utf16_to_utf8_opt(salesItem.productItem.name)) arr[i]["productItem"]["name"] = *productItemeNameOpt;
						else arr[i]["productItem"]["name"] = nullptr;

						arr[i]["quantityItems"] = salesItem.quantityItems;

						std::ostringstream pricePerUnit;
						pricePerUnit << std::fixed << std::setprecision(2) << salesItem.pricePerUnit / 100.0;
						arr[i]["pricePerUnit"] = pricePerUnit.str();

						std::ostringstream totalPricePerProduct;
						totalPricePerProduct << std::fixed << std::setprecision(2) << salesItem.totalPricePerProduct / 100.0;
						arr[i]["totalPricePerProduct"] = totalPricePerProduct.str();
						i++;
					}

					respJson["salesItem"] = std::move(arr);

					std::ostringstream oss;
					oss << std::fixed << std::setprecision(2) << sale.total / 100.0;
					respJson["total"] = oss.str();

					return crow::response{ respJson };
				}
				catch (const std::exception& e) {
					std::cerr << "Login error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}
			});

	}
}