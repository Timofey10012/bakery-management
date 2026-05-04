#pragma once

#include <string>
#include <optional>

struct WarehouseItem
{
	int id;
	std::optional<std::wstring> name;
};