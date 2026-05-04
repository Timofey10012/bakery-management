#pragma once

#include "models/user_model.h"
#include "users/user_add_model.h"
#include "users/user_short_model.h"
#include "common/errors.h"

#include <optional>

namespace users_service {
	using common::errors::ErrorCode;

	std::optional<ErrorCode> getUsers(std::vector<User>& users);

	std::optional<ErrorCode> insertUser(const UserAdd& userAdd);

	std::optional<ErrorCode> getUserShort(const int userId, UserShort& userShort);

	std::optional<ErrorCode> deleteUser(const int userId);

}