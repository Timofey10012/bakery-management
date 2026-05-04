#pragma once

#include "models/role_model.h"
#include "models/UOM_model.h"
#include "models/warehouse_item_model.h"

#include <chrono>
#include <variant>

using TimePoint = std::chrono::system_clock::time_point;

enum DashboardActivityType 
{
	sale,
	userCreated,
	stockAdded
};

struct ActivitySale
{
	int total;

	ActivitySale(int total) : total(total) {}
};

struct ActivityUserCreated
{
	Role role;

	ActivityUserCreated(Role role) : role(role) {}
};

struct ActivityStockAdded
{
	WarehouseItem warehouseItem;
	int amount;
	UOM uom;

	ActivityStockAdded(WarehouseItem warehouseItem, int amount, UOM uom) : warehouseItem(warehouseItem), amount(amount), uom(uom) {}
};

using ActivityData = std::variant<ActivitySale, ActivityUserCreated, ActivityStockAdded>;

struct DashboardActivity
{
	DashboardActivityType type;
	std::optional<TimePoint> timestamp;
	std::optional<ActivityData> data;
};