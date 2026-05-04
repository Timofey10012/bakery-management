#pragma once

#include "models/UOM_model.h"

#include <string>

struct  WarehouseItemAdd
{
	std::wstring name;
	int quantity;
	int minQuantity;
	UOM uom;
};