#pragma once

#include "models/role_model.h"

#include <string>

struct UserShort
{
	std::wstring email;
	Role role;
};