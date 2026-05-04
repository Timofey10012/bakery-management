#include "sales/sales_service.h"
#include "db/sales/sales_queries.h"

namespace sales_service {

	std::optional<ErrorCode> getSaleShort(std::vector<SaleShort>& sales) {
		if (!db::getSaleShort(db::connection(), sales)) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}

	std::optional<ErrorCode> getSale(const int saleId, Sale& sale) {
		if (!db::saleExistsById(db::connection(), saleId)) {
			return ErrorCode::SaleNotFound;
		}
		
		if (!db::getSaleUser(db::connection(), saleId, sale)) {
			return ErrorCode::InternalError;
		}
		
		if (!db::getSaleItemsAndTotal(db::connection(), saleId, sale.salesItems, sale.total)) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}

}