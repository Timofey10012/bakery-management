#pragma once

#include "models/warehouse_item_model.h"
#include "models/UOM_model.h"

#include <optional>

struct Ingredients
{
	WarehouseItem warehouseItem;
	int quantity;
	std::optional<UOM> uom;
};