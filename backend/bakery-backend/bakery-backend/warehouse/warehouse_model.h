#pragma once

#include "models/UOM_model.h"

#include <string>
#include <chrono>

using TimePoint = std::chrono::system_clock::time_point;

struct Warehouse
{
	int id;
	std::wstring name;
	int quantity;
	int minQuantity;
	UOM uom;
	TimePoint supplyDate;
	bool isInStock;
};