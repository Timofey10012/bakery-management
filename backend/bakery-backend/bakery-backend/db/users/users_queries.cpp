#include "db/users/users_queries.h"
#include "models/user_model.h"

#define WIN32_LEAN_AND_MEAN
#define NOMIMAX

#include <windows.h>
#include <sql.h>
#include <sqlext.h>

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

    bool getUsers(SQLHDBC dbc, std::vector<User>& users) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT u.id, u.FirstName, u.LastName, u.Email, r.id, r.Name "
            "FROM Users u "
            "JOIN Roles r ON u.RoleId = r.id "
            "WHERE is_deleted = 0";

        int userId = 0, roleId = 0;
        wchar_t firstName[101]{};
        wchar_t lastName[101]{};
        wchar_t email[321]{};
        wchar_t roleName[51]{};
        SQLLEN indUserId, indFirstName, indLastName, indEmail, indRoleId, indRoleName;

        if (!SQL_SUCCEEDED(SQLExecDirectA(stmt, (SQLCHAR*)query, SQL_NTS))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_LONG, &userId, 0, &indUserId);
        SQLBindCol(stmt, 2, SQL_C_WCHAR, firstName, sizeof(firstName), &indFirstName);
        SQLBindCol(stmt, 3, SQL_C_WCHAR, lastName, sizeof(lastName), &indLastName);
        SQLBindCol(stmt, 4, SQL_C_WCHAR, email, sizeof(email), &indEmail);
        SQLBindCol(stmt, 5, SQL_C_LONG, &roleId, 0, &indRoleId);
        SQLBindCol(stmt, 6, SQL_C_WCHAR, roleName, sizeof(roleName), &indRoleName);


        SQLRETURN ret;
        while ((ret = SQLFetch(stmt)) != SQL_NO_DATA) {
            if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
                User u;
                u.id = userId;
                u.firstName = firstName;
                u.lastName = lastName;
                u.email = email;
                u.role.id = roleId;
                u.role.name = roleName;

                users.push_back(std::move(u));
            }
            else {
                logError(SQL_HANDLE_STMT, stmt);
                SQLFreeHandle(SQL_HANDLE_STMT, stmt);
                return false;
            }
        }

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return true;
    }

    bool userExistsById(SQLHDBC dbc, const int id) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT 1 FROM Users WHERE id = ? AND is_deleted = 0";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int id_copy = id;
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &id_copy, 0, NULL
        );

        SQLExecute(stmt);

        SQLRETURN ret = SQLFetch(stmt);
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);

        return ret != SQL_NO_DATA;
    }

    bool getUserShort(SQLHDBC dbc, const int id, UserShort& userShort) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT u.Email, r.id, r.Name "
            "FROM Users u "
            "JOIN Roles r ON u.RoleId = r.id "
            "WHERE u.id = ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int id_copy = id;
        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0,
            &id_copy, 0, NULL);

        int roleId = 0;
        wchar_t email[321]{};
        wchar_t roleName[51]{};
        SQLLEN indEmail, indRoleId, indRoleName;

        if (!SQL_SUCCEEDED(SQLExecute(stmt))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_WCHAR, email, sizeof(email), &indEmail);
        SQLBindCol(stmt, 2, SQL_C_LONG, &roleId, 0, &indRoleId);
        SQLBindCol(stmt, 3, SQL_C_WCHAR, roleName, sizeof(roleName), &indRoleName);

        SQLRETURN ret = SQLFetch(stmt);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        userShort.email = email;
        userShort.role.id = roleId;
        userShort.role.name = roleName;

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return true;
    }

    bool deleteCart(SQLHDBC dbc, const int userId) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "DELETE FROM Cart WHERE UserId = ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int userId_copy = userId;
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &userId_copy, 0, NULL
        );

        SQLRETURN ret = SQLExecute(stmt);
        if (!SQL_SUCCEEDED(ret) && ret != SQL_NO_DATA) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);

        return true;
    }

    bool deleteUser(SQLHDBC dbc, const int id) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "UPDATE Users SET is_deleted = 1 WHERE id = ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int id_copy = id;
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &id_copy, 0, NULL
        );

        SQLRETURN ret = SQLExecute(stmt);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);

        return true;
    }

}