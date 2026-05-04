#include "dashboard/dashboard_controller.h"
#include "dashboard/dashboard_summary_model.h"
#include "dashboard/dashboard_activity_model.h"
#include "dashboard/dashboard_service.h"
#include "auth/auth_utils.h"
#include "common/string_utils.h"
#include "common/errors.h"

#include <sstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <variant>

using ActivityData = std::variant<ActivitySale, ActivityUserCreated, ActivityStockAdded>;

namespace dashboard_controller {

	namespace {
		crow::json::wvalue toJsonDashboardActivities(const DashboardActivity& act) {
			crow::json::wvalue json;

			switch (act.type) {
			case DashboardActivityType::sale:
				json["type"] = "sale";
				break;
				case DashboardActivityType::userCreated:
				json["type"] = "userCreated";
				break;
				case DashboardActivityType::stockAdded:
				json["type"] = "stockAdded";
				break;
			}

			if (act.timestamp) {
				json["timestamp"] = utils::toIsoString(*act.timestamp);
			}
			else {
				json["timestamp"] = nullptr;
			}


			if (!act.data) {
				json["data"] = nullptr;
				return json;
			}

			std::visit([&](const auto& d) {
				using T = std::decay_t<decltype(d)>;

				if constexpr (std::is_same_v<T, ActivitySale>) {
					std::ostringstream oss;
					oss << std::fixed << std::setprecision(2) << d.total / 100.0;
					json["data"]["total"] = oss.str();
				}
				else if constexpr (std::is_same_v<T, ActivityUserCreated>) {
					json["data"]["role"]["id"] = d.role.id;
					if (auto nameOpt = utils::utf16_to_utf8_opt(d.role.name)) {
						json["data"]["role"]["name"] = *nameOpt;
					}
					else {
						json["data"]["role"]["name"] = "";
					}
				}
				else if constexpr (std::is_same_v<T, ActivityStockAdded>) {
					json["data"]["warehouseItem"]["id"] = d.warehouseItem.id;
					if (auto nameOpt = utils::utf16_to_utf8_opt(d.warehouseItem.name)) {
						json["data"]["warehouseItem"]["name"] = *nameOpt;
					}
					else {
						json["data"]["warehouseItem"]["name"] = "";
					};

					json["data"]["amount"] = d.amount;

					json["data"]["UOM"]["id"] = d.uom.id;
					if (auto nameOpt = utils::utf16_to_utf8_opt(d.uom.name)) {
						json["data"]["UOM"]["name"] = *nameOpt;
					}
					else {
						json["data"]["UOM"]["name"] = "";
					};
				}
				}, *act.data);

			return json;
		}

	}

	void setupRoutes(crow::App<crow::CORSHandler>& app) {

		CROW_ROUTE(app, "/dashboard/summary").methods(crow::HTTPMethod::Get) (
			[](const crow::request& req) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role != auth::Role::Admin) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					DashboardSummary dashSum;
					if (auto err = dashboard_service::getDashboardSummary(dashSum)) {
						return common::errors::makeError(*err);
					}

					crow::json::wvalue respJson;
					std::ostringstream oss;
					oss << std::fixed << std::setprecision(2) << dashSum.salesToday / 100.0;
					respJson["salesToday"] = oss.str();
					respJson["productsCount"] = dashSum.productsCount;
					respJson["usersCount"] = dashSum.usersCount;
					respJson["ordersToday"] = dashSum.ordersToday;

					return crow::response{ respJson };
				}
				catch (const std::exception& e) {
					std::cerr << "Register error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}

			});

		CROW_ROUTE(app, "/dashboard/activity").methods(crow::HTTPMethod::Get) (
			[](const crow::request& req) {
				try {
					auth::Context authCtx;
					if (auto err = auth::fromRequest(req, authCtx)) {
						return common::errors::makeError(*err);
					}

					if (authCtx.role != auth::Role::Admin) {
						return common::errors::makeError(common::errors::ErrorCode::Forbidden);
					}

					std::vector<DashboardActivity> dashActs(3);
					if (auto err = dashboard_service::getDashboardActivity(dashActs)) {
						return common::errors::makeError(*err);
					}

					crow::json::wvalue arr(crow::json::type::List);

					size_t i = 0;
					for (const auto& act : dashActs) {						
						arr[i++] = toJsonDashboardActivities(act);
					}

					crow::json::wvalue respJson;
					respJson["dashboardActivity"] = std::move(arr);

					return crow::response{ respJson };

				}
				catch (const std::exception& e) {
					std::cerr << "Register error: " << e.what() << std::endl;
					return common::errors::makeError(common::errors::ErrorCode::InternalError);
				}

			});

	}

}