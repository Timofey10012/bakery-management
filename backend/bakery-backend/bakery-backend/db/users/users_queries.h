#pragma once

#include "db/db.h"
#include "models/user_model.h"
#include "users/user_short_model.h"

#include <vector>

namespace db {

	bool getUsers(SQLHDBC dbc, std::vector<User>& users);

	bool userExistsByEmail(SQLHDBC dbc, const std::wstring& email);

    bool insertUser(
        SQLHDBC dbc,
        const std::wstring& firstName,
        const std::wstring& lastName,
        const std::wstring& email,
        const std::string& passwordHash,
        int roleId
    );

    bool userExistsById(SQLHDBC dbc, const int id);

    bool getUserShort(SQLHDBC dbc, const int id, UserShort& userShort);

    bool deleteCart(SQLHDBC dbc, const int userId);

    bool deleteUser(SQLHDBC dbc, const int id);

}