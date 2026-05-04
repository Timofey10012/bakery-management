#pragma once

#include <string>
#include "models/role_model.h"

struct User
{
	int id;
	std::wstring firstName;
	std::wstring lastName;
	std::wstring email;
	Role role;
};