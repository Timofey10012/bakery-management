#pragma once

#include <string>
#include <optional>

struct Role
{
	int id;
	std::optional<std::wstring> name;
};