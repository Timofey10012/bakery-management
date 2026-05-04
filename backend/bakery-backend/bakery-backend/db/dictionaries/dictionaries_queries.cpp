#include "db/dictionaries/dictionaries_queries.h"

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

    bool getRoles(SQLHDBC dbc, std::vector<Role>& roles) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT id, Name "
            "FROM Roles";

        int id = 0;
        wchar_t name[51]{};
        SQLLEN indId, indName;

        if (!SQL_SUCCEEDED(SQLExecDirectA(stmt, (SQLCHAR*)query, SQL_NTS))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_LONG, &id, 0, &indId);
        SQLBindCol(stmt, 2, SQL_C_WCHAR, name, sizeof(name), &indName);


        SQLRETURN ret;
        while ((ret = SQLFetch(stmt)) != SQL_NO_DATA) {
            if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
                Role r;
                r.id = id;
                r.name = name;

                roles.push_back(std::move(r));
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

    bool getUOM(SQLHDBC dbc, std::vector<UOM>& uoms) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT id, Symbol "
            "FROM UnitOfMeasure";

        int id = 0;
        wchar_t name[11]{};
        SQLLEN indId, indName;

        if (!SQL_SUCCEEDED(SQLExecDirectA(stmt, (SQLCHAR*)query, SQL_NTS))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_LONG, &id, 0, &indId);
        SQLBindCol(stmt, 2, SQL_C_WCHAR, name, sizeof(name), &indName);


        SQLRETURN ret;
        while ((ret = SQLFetch(stmt)) != SQL_NO_DATA) {
            if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
                UOM u;
                u.id = id;
                u.name = name;

                uoms.push_back(std::move(u));
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

    bool getWarehouseItems(SQLHDBC dbc, std::vector<WarehouseItem>& warehouseItems) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT id, Name "
            "FROM Consumables "
            "WHERE is_deleted = 0";

        int id = 0;
        wchar_t name[101]{};
        SQLLEN indId, indName;

        if (!SQL_SUCCEEDED(SQLExecDirectA(stmt, (SQLCHAR*)query, SQL_NTS))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_LONG, &id, 0, &indId);
        SQLBindCol(stmt, 2, SQL_C_WCHAR, name, sizeof(name), &indName);


        SQLRETURN ret;
        while ((ret = SQLFetch(stmt)) != SQL_NO_DATA) {
            if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
                WarehouseItem wi;
                wi.id = id;
                wi.name = name;

                warehouseItems.push_back(std::move(wi));
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

    bool getManufacturer(SQLHDBC dbc, std::vector<Manufacturer>& manufacturers) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT id, Name "
            "FROM Manufacturer "
            "WHERE is_deleted = 0";

        int id = 0;
        wchar_t name[101]{};
        SQLLEN indId, indName;

        if (!SQL_SUCCEEDED(SQLExecDirectA(stmt, (SQLCHAR*)query, SQL_NTS))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_LONG, &id, 0, &indId);
        SQLBindCol(stmt, 2, SQL_C_WCHAR, name, sizeof(name), &indName);


        SQLRETURN ret;
        while ((ret = SQLFetch(stmt)) != SQL_NO_DATA) {
            if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
                Manufacturer m;
                m.id = id;
                m.name = name;

                manufacturers.push_back(std::move(m));
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

    bool manufacturerExistsByName(SQLHDBC dbc, const std::wstring& name) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT 1 FROM Manufacturer WHERE Name = ? AND is_deleted = 0";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_WCHAR, SQL_VARCHAR,
            100, 0,
            (SQLPOINTER)name.c_str(), 0, NULL
        );

        SQLExecute(stmt);

        SQLRETURN ret = SQLFetch(stmt);
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);

        return ret != SQL_NO_DATA;
    }

    bool insertManufacturer(SQLHDBC dbc, const std::wstring& name) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "INSERT INTO Manufacturer "
            "(Name) "
            "VALUES (?)";
        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);

        SQLLEN nameLen = SQL_NTS;
        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0,
            (SQLPOINTER)name.c_str(), 0, &nameLen);

        SQLRETURN ret = SQLExecute(stmt);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);

        return true;
    }

    bool manufacturerExistsById(SQLHDBC dbc, const int id) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT 1 FROM Manufacturer WHERE id = ? AND is_deleted = 0";

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

    bool deleteManufacturer(SQLHDBC dbc, const int id) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "UPDATE Manufacturer SET is_deleted = 1 WHERE id = ?";

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