#pragma once

#include "db/db.h"
#include "models/role_model.h"
#include "models/UOM_model.h"
#include "models/warehouse_item_model.h"
#include "models/manufacturer_model.h"

#include <vector>
#include <string>

namespace db {

	bool getRoles(SQLHDBC dbc, std::vector<Role>& roles);

	bool getUOM(SQLHDBC dbc, std::vector<UOM>& uoms);

	bool getWarehouseItems(SQLHDBC dbc, std::vector<WarehouseItem>& warehouseItems);

	bool getManufacturer(SQLHDBC dbc, std::vector<Manufacturer>& manufacturers);

	bool manufacturerExistsByName(SQLHDBC dbc, const std::wstring& name);

	bool insertManufacturer(SQLHDBC dbc, const std::wstring& name);

	bool manufacturerExistsById(SQLHDBC dbc, const int id);

	bool deleteManufacturer(SQLHDBC dbc, const int id);

}