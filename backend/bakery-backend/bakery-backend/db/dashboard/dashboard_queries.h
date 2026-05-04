#pragma once

#include "dashboard/dashboard_summary_model.h"
#include "dashboard/dashboard_activity_model.h"
#include "db/db.h"

namespace db {

	bool getDashboardSummary(SQLHDBC dbc, DashboardSummary& out);

	bool getDashboardActivitySale(SQLHDBC dbc, DashboardActivity& out);
	bool getDashboardActivityUserCreated(SQLHDBC dbc, DashboardActivity& out);
	bool getDashboardActivityStockAdded(SQLHDBC dbc, DashboardActivity& out);
}