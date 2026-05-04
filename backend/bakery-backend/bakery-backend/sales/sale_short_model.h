#pragma once

#include "models/user_model.h"

#include <optional>

struct SaleShort
{
	int id;
	std::optional<User> user;
	int total;
};