#include "dashboard/dashboard_service.h"
#include "db/dashboard/dashboard_queries.h"

namespace dashboard_service {

	std::optional<ErrorCode> getDashboardSummary(DashboardSummary& out) {

		if (!db::getDashboardSummary(db::connection(), out)) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}

	std::optional<ErrorCode> getDashboardActivity(std::vector<DashboardActivity>& out) {
		out[0].type = DashboardActivityType::sale;
		if (!db::getDashboardActivitySale(db::connection(), out[0])) {
			return ErrorCode::InternalError;
		}

		out[1].type = DashboardActivityType::userCreated;
		if (!db::getDashboardActivityUserCreated(db::connection(), out[1])) {
			return ErrorCode::InternalError;
		}

		out[2].type = DashboardActivityType::stockAdded;
		if (!db::getDashboardActivityStockAdded(db::connection(), out[2])) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}
}