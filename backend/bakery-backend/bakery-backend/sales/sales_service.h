#pragma once

#include "sales/sale_short_model.h"
#include "sales/sale_model.h"
#include "common/errors.h"

#include <optional>

namespace sales_service {
	using common::errors::ErrorCode;

	std::optional<ErrorCode> getSaleShort(std::vector<SaleShort>& sales);

	std::optional<ErrorCode> getSale(const int saleId, Sale& sale);

}