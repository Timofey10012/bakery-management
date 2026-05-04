#pragma once

#include "models/role_model.h"
#include "models/UOM_model.h"
#include "models/warehouse_item_model.h"
#include "models/manufacturer_model.h"
#include "manufacturers/manufacturer_add_model.h"
#include "common/errors.h"

#include <optional>

namespace dictionaries_service {
	using common::errors::ErrorCode;

	std::optional<ErrorCode> getRoles(std::vector<Role>& roles);

	std::optional<ErrorCode> getUOM(std::vector<UOM>& uoms);

	std::optional<ErrorCode> getWarehouseItems(std::vector<WarehouseItem>& warehouseItems);

	std::optional<ErrorCode> getManufacturer(std::vector<Manufacturer>& manufacturers);

	std::optional<ErrorCode> insertManufacturer(const ManufacturerAdd& manufacturerAdd);

	std::optional<ErrorCode> deleteManufacturer(const int id);

}