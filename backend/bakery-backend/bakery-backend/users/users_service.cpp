#include "users/users_service.h"
#include "db/users/users_queries.h"
#include "auth/auth_utils.h"

namespace users_service {

	std::optional<ErrorCode> getUsers(std::vector<User>& users) {

		if (!db::getUsers(db::connection(), users)) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}

	std::optional<ErrorCode> insertUser(const UserAdd& userAdd) {
		if (userAdd.firstName.empty() || userAdd.lastName.empty() || userAdd.email.empty() || userAdd.password.empty()) {
			return ErrorCode::ValidationError;
		}

		if (userAdd.password.size() < 8) {
			return ErrorCode::AuthRegisterPasswordNotValid;
		}

		if (db::userExistsByEmail(db::connection(), userAdd.email)) {
			return ErrorCode::AuthRegisterEmailAlreadyExists;
		}

		std::string passwordHash = auth::hashPassword(userAdd.password);

		if (!db::insertUser(
			db::connection(),
			userAdd.firstName,
			userAdd.lastName,
			userAdd.email,
			passwordHash,
			userAdd.role.id
		)) {
			return ErrorCode::InternalError;
		}
		
		return std::nullopt;
	}

	std::optional<ErrorCode> getUserShort(const int userId, UserShort& userShort) {
		if (!db::userExistsById(db::connection(), userId)) {
			return ErrorCode::UserNotFound;
		}

		if (!db::getUserShort(db::connection(), userId, userShort)) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}

	std::optional<ErrorCode> deleteUser(const int userId) {
		if (!db::userExistsById(db::connection(), userId)) {
			return ErrorCode::UserNotFound;
		}

		if (!db::beginTransaction(db::connection())) {
			return ErrorCode::InternalError;
		}
		bool success = true;

		if (!db::deleteCart(db::connection(), userId)) {
			success = false;
		}

		if (!db::deleteUser(db::connection(), userId)) {
			success = false;
		}

		if (!db::endTransaction(db::connection(), success)) {
			return ErrorCode::InternalError;
		}

		if (!success) return ErrorCode::InternalError;

		return std::nullopt;
	}

}