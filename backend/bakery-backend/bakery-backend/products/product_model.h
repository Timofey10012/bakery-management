#pragma once

#include "models/UOM_model.h"

#include <string>
#include <optional>

struct Product
{
	int id;
	std::wstring name;
	std::optional<std::string> imgURL;
	int quantityPerUnit;
	UOM uom;
	int price;
	int stockQuantity;
};