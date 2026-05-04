#pragma once

#include "models/user_model.h"
#include "models/sales_item_model.h"

#include <optional>
#include <vector>

struct Sale
{
	std::optional<User> user;
	std::vector<SalesItem> salesItems;
	int total;
};