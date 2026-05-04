#pragma once

#include "models/UOM_model.h"
#include "models/manufacturer_model.h"
#include "models/ingredients_model.h"

#include <string>
#include <optional>
#include <vector>

struct ProductItemFull
{
	std::wstring name;
	std::optional<std::string> imgURL;
	int quantityPerUnit;
	UOM uom;
	int price;
	int inStock;
	std::wstring description;
	int caloriesPer100g;
	int expiration;
	int tempStorage;
	Manufacturer manufacturer;
	std::vector<Ingredients> ingredients;
};