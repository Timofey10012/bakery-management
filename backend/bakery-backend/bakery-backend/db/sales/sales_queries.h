#pragma once

#include "db/db.h"
#include "sales/sale_short_model.h"
#include "sales/sale_model.h"
#include "models/sales_item_model.h"

#include <vector>

namespace db {

	bool getSaleShort(SQLHDBC dbc, std::vector<SaleShort>& sales);

	bool saleExistsById(SQLHDBC dbc, const int id);

	bool getSaleUser(SQLHDBC dbc, const int id, Sale& sale);

	bool getSaleItemsAndTotal(SQLHDBC dbc, const int id, std::vector<SalesItem>& salesItems, int& total);

}