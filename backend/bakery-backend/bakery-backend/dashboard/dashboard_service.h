#pragma once

#include "common/errors.h"
#include "dashboard/dashboard_summary_model.h"
#include "dashboard/dashboard_activity_model.h"

#include <optional>
#include <vector>

namespace dashboard_service {

	using common::errors::ErrorCode;

	std::optional<ErrorCode> getDashboardSummary(DashboardSummary& out);

	std::optional<ErrorCode> getDashboardActivity(std::vector<DashboardActivity>& out);

}