#include "dictionaries/dictionaries_service.h"
#include "db/dictionaries/dictionaries_queries.h"

namespace dictionaries_service {
	std::optional<ErrorCode> getRoles(std::vector<Role>& roles) {
		if (!db::getRoles(db::connection(), roles)) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}

	std::optional<ErrorCode> getUOM(std::vector<UOM>& uoms) {
		if (!db::getUOM(db::connection(), uoms)) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}

	std::optional<ErrorCode> getWarehouseItems(std::vector<WarehouseItem>& warehouseItems) {
		if (!db::getWarehouseItems(db::connection(), warehouseItems)) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}

	std::optional<ErrorCode> getManufacturer(std::vector<Manufacturer>& manufacturers) {
		if (!db::getManufacturer(db::connection(), manufacturers)) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}

	std::optional<ErrorCode> insertManufacturer(const ManufacturerAdd& manufacturerAdd) {
		if (manufacturerAdd.name.empty()) {
			return ErrorCode::ValidationError;
		}

		if (db::manufacturerExistsByName(db::connection(), manufacturerAdd.name)) {
			return ErrorCode::ManufacturerAlreadyExists;
		}

		if (!db::insertManufacturer(db::connection(), manufacturerAdd.name)) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}

	std::optional<ErrorCode> deleteManufacturer(const int id) {
		if (!db::manufacturerExistsById(db::connection(), id)) {
			return ErrorCode::ManufacturerNotFound;
		}

		if (!db::deleteManufacturer(db::connection(), id)) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}

}