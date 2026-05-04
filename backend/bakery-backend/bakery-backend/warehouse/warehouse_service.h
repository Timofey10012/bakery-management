#pragma once

#include "warehouse/warehouse_model.h"
#include "warehouse/warehouse_item_add_model.h"
#include "warehouse/warehouse_item_short_model.h"
#include "models/add_stock_model.h"
#include "models/remove_stock_model.h"
#include "common/errors.h"

#include <optional>

namespace warehouse_service {
	using common::errors::ErrorCode;

	std::optional<ErrorCode> getWarehouse(std::vector<Warehouse>& warehouse);

	std::optional<ErrorCode> insertWarehouseItem(const WarehouseItemAdd& warehouseItem);

	std::optional<ErrorCode> getWarehouseItemShort(const int warehouseId, WarehouseItemShort& warehouseItem);

	std::optional<ErrorCode> deleteWarehouseItem(const int warehouseId);

	std::optional<ErrorCode> addStock(const int warehouseId, const AddStock& addStock);

	std::optional<ErrorCode> removeStock(const int warehouseId, const RemoveStock& removeStock);

}