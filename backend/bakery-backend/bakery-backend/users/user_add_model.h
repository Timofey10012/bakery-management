#pragma once

#include "models/role_model.h"

#include <string>

struct UserAdd
{
	std::wstring firstName;
	std::wstring lastName;
	std::wstring email;
	std::string password;
	Role role;
};