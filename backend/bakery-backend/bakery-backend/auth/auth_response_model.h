#pragma once

#include <string>
#include "models/user_model.h"

struct AuthResponse
{
	std::string token;
	User user;
};