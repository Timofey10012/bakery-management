#include "db/auth/auth_queries.h"
#include "models/user_model.h"
#include "models/role_model.h"

#define WIN32_LEAN_AND_MEAN
#define NOMIMAX

#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <iostream>

namespace db {

    namespace {
        void logError(SQLSMALLINT handleType, SQLHANDLE handle) {
            SQLWCHAR sqlState[6];
            SQLINTEGER nativeError;
            SQLWCHAR message[512];
            SQLSMALLINT textLength;

            SQLGetDiagRecW(handleType, handle, 1, sqlState, &nativeError, message, _countof(message), &textLength);
            wprintf(L"ODBC Error: %s (%d) - %s\n", sqlState, nativeError, message);
        }

    }

    bool userExistsByEmail(SQLHDBC dbc, const std::wstring& email) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT 1 FROM Users WHERE Email = ? AND is_deleted = 0";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_WCHAR, SQL_VARCHAR,
            320, 0,
            (SQLPOINTER)email.c_str(), 0, NULL
        );

        SQLExecute(stmt);

        SQLRETURN ret = SQLFetch(stmt);
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);

        return ret != SQL_NO_DATA;
    }

    bool getPasswordHashByEmail(SQLHDBC dbc, const std::wstring& email, std::string& passwordHash) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT PasswordHash FROM Users WHERE Email = ? AND is_deleted = 0";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_WCHAR, SQL_VARCHAR,
            320, 0,
            (SQLPOINTER)email.c_str(), 0, NULL
        );

        char passwordHashBuf[256] = {};
        SQLLEN indPasswordHashBuf;

        if (!SQL_SUCCEEDED(SQLExecute(stmt))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_CHAR, passwordHashBuf, sizeof(passwordHashBuf), &indPasswordHashBuf);

        SQLRETURN ret = SQLFetch(stmt);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        passwordHash = passwordHashBuf;

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return true;
    }

    bool insertUser(
        SQLHDBC dbc,
        const std::wstring& firstName,
        const std::wstring& lastName,
        const std::wstring& email,
        const std::string& passwordHash,
        int roleId
    ) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "INSERT INTO Users "
            "(FirstName, LastName, Email, PasswordHash, RoleId) "
            "VALUES (?, ?, ?, ?, ?)";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);

        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, 100, 0,
            (SQLPOINTER)firstName.c_str(), 0, NULL);
        SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, 100, 0,
            (SQLPOINTER)lastName.c_str(), 0, NULL);
        SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, 320, 0,
            (SQLPOINTER)email.c_str(), 0, NULL);
        SQLBindParameter(stmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0,
            (SQLPOINTER)passwordHash.c_str(), 0, NULL);
        SQLBindParameter(stmt, 5, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0,
            &roleId, 0, NULL);

        SQLRETURN ret = SQLExecute(stmt);
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);

        return SQL_SUCCEEDED(ret);
    }

    bool getUserByEmail(
        SQLHDBC dbc,
        const std::wstring& email,
        User& outUser
    ) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT "
            "u.Id, u.FirstName, u.LastName, u.Email, "
            "r.Id, r.Name "
            "FROM Users u "
            "JOIN Roles r ON r.Id = u.RoleId "
            "WHERE u.Email = ? AND u.is_deleted = 0";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        SQLBindParameter(
            stmt,
            1,
            SQL_PARAM_INPUT,
            SQL_C_WCHAR,
            SQL_WVARCHAR,
            320,
            0,
            (SQLPOINTER)email.c_str(),
            0,
            nullptr
        );


        int userId = 0, roleId = 0;
        wchar_t firstName[101] = {};
        wchar_t lastName[101] = {};
        wchar_t emailBuf[321] = {};
        wchar_t roleName[101] = {};

        SQLLEN indUserId, indFirstName, indLastName, indEmail, indRoleId, indRoleName;

        if (!SQL_SUCCEEDED(SQLExecute(stmt))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_SLONG, &userId, 0, &indUserId);
        SQLBindCol(stmt, 2, SQL_C_WCHAR, firstName, sizeof(firstName), &indFirstName);
        SQLBindCol(stmt, 3, SQL_C_WCHAR, lastName, sizeof(lastName), &indLastName);
        SQLBindCol(stmt, 4, SQL_C_WCHAR, emailBuf, sizeof(emailBuf), &indEmail);
        SQLBindCol(stmt, 5, SQL_C_SLONG, &roleId, 0, &indRoleId);
        SQLBindCol(stmt, 6, SQL_C_WCHAR, roleName, sizeof(roleName), &indRoleName);

        SQLRETURN ret = SQLFetch(stmt);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        outUser.id = userId;
        outUser.firstName = firstName;
        outUser.lastName = lastName;
        outUser.email = emailBuf;
        outUser.role.id = roleId;
        outUser.role.name = roleName;

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return true;
    }

    bool getUserById(
        SQLHDBC dbc,
        const int& id,
        User& outUser
    ) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT "
            "u.Id, u.FirstName, u.LastName, u.Email, "
            "r.Id, r.Name "
            "FROM Users u "
            "JOIN Roles r ON r.Id = u.RoleId "
            "WHERE u.Id = ? AND u.is_deleted = 0";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);

        SQLINTEGER idValue = id;
        SQLBindParameter(stmt,
            1,
            SQL_PARAM_INPUT,
            SQL_C_SLONG,
            SQL_INTEGER,
            0,
            0,
            &idValue,
            0,
            nullptr);


        int userId = 0, roleId = 0;
        wchar_t firstName[101] = {};
        wchar_t lastName[101] = {};
        wchar_t emailBuf[321] = {};
        wchar_t roleName[101] = {};

        SQLLEN indUserId, indFirstName, indLastName, indEmail, indRoleId, indRoleName;

        if (!SQL_SUCCEEDED(SQLExecute(stmt))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_SLONG, &userId, 0, &indUserId);
        SQLBindCol(stmt, 2, SQL_C_WCHAR, firstName, sizeof(firstName), &indFirstName);
        SQLBindCol(stmt, 3, SQL_C_WCHAR, lastName, sizeof(lastName), &indLastName);
        SQLBindCol(stmt, 4, SQL_C_WCHAR, emailBuf, sizeof(emailBuf), &indEmail);
        SQLBindCol(stmt, 5, SQL_C_SLONG, &roleId, 0, &indRoleId);
        SQLBindCol(stmt, 6, SQL_C_WCHAR, roleName, sizeof(roleName), &indRoleName);

        SQLRETURN ret = SQLFetch(stmt);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        outUser.id = userId;
        outUser.firstName = firstName;
        outUser.lastName = lastName;
        outUser.email = emailBuf;
        outUser.role.id = roleId;
        outUser.role.name = roleName;

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return true;
    }

}
