#include "warehouse/warehouse_service.h"
#include "db/warehouse/warehouse_queries.h"

namespace warehouse_service {

	std::optional<ErrorCode> getWarehouse(std::vector<Warehouse>& warehouse) {
		if (!db::getWarehouse(db::connection(), warehouse)) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}

	std::optional<ErrorCode> insertWarehouseItem(const WarehouseItemAdd& warehouseItem) {
		if (warehouseItem.name.empty()) {
			return ErrorCode::ValidationError;
		}

		if (warehouseItem.quantity < 0 || warehouseItem.minQuantity < 0) {
			return ErrorCode::ValidationError;
		}

		if (db::warehouseExistsByName(db::connection(), warehouseItem.name)) {
			return ErrorCode::WarehouseItemAlreadyExists;
		}

		if (!db::UOMExistsByID(db::connection(), warehouseItem.uom.id)) {
			return ErrorCode::ValidationError;
		}

		if (!db::beginTransaction(db::connection())) {
			return ErrorCode::InternalError;
		}
		bool success = true;

		int warehouseId;
		if (!db::insertWarehouseItem(db::connection(), warehouseId, warehouseItem)) {
			success = false;
		}
		else if (!db::insertWarehouseReplenishments(db::connection(), warehouseId, warehouseItem.quantity)) {
			success = false;
		}

		if (!db::endTransaction(db::connection(), success)) {
			return ErrorCode::InternalError;
		}

		if (!success) return ErrorCode::InternalError;

		return std::nullopt;
	}

	std::optional<ErrorCode> getWarehouseItemShort(const int warehouseId, WarehouseItemShort& warehouseItem) {
		if (!db::warehouseExistsById(db::connection(), warehouseId)) {
			return ErrorCode::WarehouseItemNotFound;
		}

		if (!db::getWarehouseItemShort(db::connection(), warehouseId, warehouseItem)) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}

	std::optional<ErrorCode> deleteWarehouseItem(const int warehouseId) {
		if (!db::warehouseExistsById(db::connection(), warehouseId)) {
			return ErrorCode::WarehouseItemNotFound;
		}

		if (db::warehouseItemUsedInProduct(db::connection(), warehouseId)) {
			return ErrorCode::WarehouseItemAlreadyUsedInProduct;
		}

		if (!db::deleteWarehouseItem(db::connection(), warehouseId)) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}

	std::optional<ErrorCode> addStock(const int warehouseId, const AddStock& addStock) {
		if (!db::warehouseExistsById(db::connection(), warehouseId)) {
			return ErrorCode::WarehouseItemNotFound;
		}

		if (addStock.quantity < 0) {
			return ErrorCode::ValidationError;
		}

		if (!db::beginTransaction(db::connection())) {
			return ErrorCode::InternalError;
		}
		bool success = true;

		if (!db::warehouseItemAddStock(db::connection(), warehouseId, addStock.quantity)) {
			success = false;
		}
		else if (!db::insertWarehouseReplenishments(db::connection(), warehouseId, addStock.quantity)) {
			success = false;
		}

		if (!db::endTransaction(db::connection(), success)) {
			return ErrorCode::InternalError;
		}

		if (!success) return ErrorCode::InternalError;

		return std::nullopt;
	}

	std::optional<ErrorCode> removeStock(const int warehouseId, const RemoveStock& removeStock) {
		if (!db::warehouseExistsById(db::connection(), warehouseId)) {
			return ErrorCode::WarehouseItemNotFound;
		}

		if (removeStock.quantity < 0) {
			return ErrorCode::ValidationError;
		}

		if (db::warehouseRemoveQuantityIsMoreLimit(db::connection(), warehouseId, removeStock.quantity)) {
			return ErrorCode::WarehouseItemInsufficientAmount;
		}

		if (!db::warehouseItemRemoveStock(db::connection(), warehouseId, removeStock.quantity)) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}

}