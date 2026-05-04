#pragma once

#include "db/db.h"
#include "warehouse/warehouse_model.h"
#include "warehouse/warehouse_item_add_model.h"
#include "warehouse/warehouse_item_short_model.h"

#include <vector>

namespace db {

	bool getWarehouse(SQLHDBC dbc, std::vector<Warehouse>& warehouse);

	bool warehouseExistsByName(SQLHDBC dbc, const std::wstring& name);

	bool UOMExistsByID(SQLHDBC dbc, const int& id);

	bool insertWarehouseItem(SQLHDBC dbc, int& warehouseId, const WarehouseItemAdd& warehouseItem);

	bool insertWarehouseReplenishments(SQLHDBC dbc, const int warehouseId, const int quantity);

	bool warehouseExistsById(SQLHDBC dbc, const int id);

	bool getWarehouseItemShort(SQLHDBC dbc, const int id, WarehouseItemShort& warehouseItem);

	bool warehouseItemUsedInProduct(SQLHDBC dbc, const int id);

	bool deleteWarehouseItem(SQLHDBC dbc, const int id);

	bool warehouseItemAddStock(SQLHDBC dbc, const int id, const int quantity);

	bool warehouseRemoveQuantityIsMoreLimit(SQLHDBC dbc, const int id, const int quantity);

	bool warehouseItemRemoveStock(SQLHDBC dbc, const int id, const int quantity);

}