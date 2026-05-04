#pragma once

#include "models/UOM_model.h"

#include <string>

struct WarehouseItemShort
{
	int id;
	std::wstring name;
	int quantity;
	UOM uom;
};