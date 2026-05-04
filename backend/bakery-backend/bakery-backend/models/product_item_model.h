#pragma once

#include <string>
#include <optional>

struct ProductItem
{
	int id;
	std::optional<std::wstring> name;
};