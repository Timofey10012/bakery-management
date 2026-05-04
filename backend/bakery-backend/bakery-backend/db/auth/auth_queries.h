#pragma once

#include "db/db.h"
#include "models/user_model.h"

namespace db {

    bool userExistsByEmail(SQLHDBC dbc, const std::wstring& email);

    bool getPasswordHashByEmail(SQLHDBC dbc, const std::wstring& email, std::string& passwordHash);

    bool insertUser(
        SQLHDBC dbc,
        const std::wstring& firstName,
        const std::wstring& lastName,
        const std::wstring& email,
        const std::string& passwordHash,
        int roleId
    );

    bool getUserByEmail(
        SQLHDBC dbc,
        const std::wstring& email,
        User& outUser
    );

    bool getUserById(
        SQLHDBC dbc,
        const int& id,
        User& outUser
    );

}
