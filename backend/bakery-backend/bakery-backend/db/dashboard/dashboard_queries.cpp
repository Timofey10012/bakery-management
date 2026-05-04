#include "db/dashboard/dashboard_queries.h"

#define WIN32_LEAN_AND_MEAN
#define NOMIMAX

#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <iostream>
#include <chrono>
#include <ctime>

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


	bool getDashboardSummary(SQLHDBC dbc, DashboardSummary& out) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT "
            "(SELECT COALESCE(SUM(si.QuantityItems * si.PriceAtPurchase), 0) "
            " FROM Sales s "
            " JOIN SalesItem si ON s.Id = si.SalesId "
            " WHERE CAST(s.CreatedAt AS date) = CAST(GETDATE() AS date)) AS salesToday, "
            "(SELECT COUNT(*) FROM Products WHERE is_deleted = 0) AS productsCount, "
            "(SELECT COUNT(*) FROM Users WHERE is_deleted = 0) AS usersCount, "
            "(SELECT COUNT(*) FROM Sales "
            " WHERE CAST(CreatedAt AS date) = CAST(GETDATE() AS date)) AS ordersToday";


        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);

        double salesToday = 0;
        int productsCount = 0, usersCount = 0, ordersToday = 0;
        SQLLEN indSalesToday, indProductsCount, indUsersCount, indOrdersToday;

        if (!SQL_SUCCEEDED(SQLExecute(stmt))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_DOUBLE, &salesToday, 0, &indSalesToday);
        SQLBindCol(stmt, 2, SQL_C_LONG, &productsCount, 0, &indProductsCount);
        SQLBindCol(stmt, 3, SQL_C_LONG, &usersCount, 0, &indUsersCount);
        SQLBindCol(stmt, 4, SQL_C_LONG, &ordersToday, 0, &indOrdersToday);

        SQLRETURN ret = SQLFetch(stmt);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        out.salesToday = static_cast<int>(salesToday * 100.0 + 0.5);
        out.productsCount = productsCount;
        out.usersCount = usersCount;
        out.ordersToday = ordersToday;

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return true;
	}

    bool getDashboardActivitySale(SQLHDBC dbc, DashboardActivity& out) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT s.CreatedAt, COALESCE(SUM(si.QuantityItems * si.PriceAtPurchase), 0) "
            "FROM Sales s "
            "JOIN SalesItem si ON s.Id = si.SalesId "
            "WHERE s.Id = (SELECT TOP 1 Id FROM Sales ORDER BY CreatedAt DESC) "
            "GROUP BY s.CreatedAt";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);

        TIMESTAMP_STRUCT ts;
        double amount = 0;
        SQLLEN indTimestamp, indAmount;

        if (!SQL_SUCCEEDED(SQLExecute(stmt))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_TYPE_TIMESTAMP, &ts, sizeof(ts), &indTimestamp);
        SQLBindCol(stmt, 2, SQL_C_DOUBLE, &amount, 0, &indAmount);

        SQLRETURN ret = SQLFetch(stmt);

        if (ret == SQL_NO_DATA) {
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return true;
        }

        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        if (indTimestamp == SQL_NULL_DATA) {
            return false;
        }

        std::tm tm_struct{};
        tm_struct.tm_year = ts.year - 1900;
        tm_struct.tm_mon = ts.month - 1;
        tm_struct.tm_mday = ts.day;
        tm_struct.tm_hour = ts.hour;
        tm_struct.tm_min = ts.minute;
        tm_struct.tm_sec = ts.second;

        auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm_struct));

        out.timestamp = tp;
        out.data = ActivitySale{static_cast<int>(amount * 100.0 + 0.5)};

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return true;
    }

    bool getDashboardActivityUserCreated(SQLHDBC dbc, DashboardActivity& out) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT TOP 1 u.CreatedAt, r.id, r.Name "
            " FROM Users u "
            " JOIN Roles r ON u.RoleId = r.id "
            " ORDER BY u.CreatedAt DESC";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);

        TIMESTAMP_STRUCT ts;
        int roleId = 0;
        wchar_t roleName[101] = {};
        SQLLEN indTimestamp, indRoleId, indRoleName;

        if (!SQL_SUCCEEDED(SQLExecute(stmt))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_TYPE_TIMESTAMP, &ts, sizeof(ts), &indTimestamp);
        SQLBindCol(stmt, 2, SQL_C_LONG, &roleId, 0, &indRoleId);
        SQLBindCol(stmt, 3, SQL_C_WCHAR, roleName, sizeof(roleName), &indRoleName);

        SQLRETURN ret = SQLFetch(stmt);

        if (ret == SQL_NO_DATA) {
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return true;
        }

        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        if (indTimestamp == SQL_NULL_DATA) {
            return false;
        }

        std::tm tm_struct{};
        tm_struct.tm_year = ts.year - 1900;
        tm_struct.tm_mon = ts.month - 1;
        tm_struct.tm_mday = ts.day;
        tm_struct.tm_hour = ts.hour;
        tm_struct.tm_min = ts.minute;
        tm_struct.tm_sec = ts.second;

        auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm_struct));

        out.timestamp = tp;

        Role role;
        role.id = roleId;
        role.name = roleName;
        out.data = ActivityUserCreated{role};

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return true;
    }

    bool getDashboardActivityStockAdded(SQLHDBC dbc, DashboardActivity& out) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT TOP 1 cr.ReplenishmentAt, cr.ConsumableId, c.Name, cr.Quantity, c.UOMId, uom.Symbol "
            " FROM Consumables c "
            " JOIN ConsumableReplenishments cr ON c.Id = cr.ConsumableId "
            " JOIN UnitOfMeasure uom ON c.UOMId = uom.id "
            " ORDER BY cr.ReplenishmentAt DESC";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);

        TIMESTAMP_STRUCT ts;
        int warehouseItemId = 0, amount = 0, UOMId = 0;
        wchar_t warehouseItemName[101] = {};
        wchar_t UOMName[11] = {};
        SQLLEN indTimestamp, indWarehouseItemId, indAmount, indUOMId, indWarehouseItemName, indUOMName;

        if (!SQL_SUCCEEDED(SQLExecute(stmt))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_TYPE_TIMESTAMP, &ts, sizeof(ts), &indTimestamp);
        SQLBindCol(stmt, 2, SQL_C_LONG, &warehouseItemId, 0, &indWarehouseItemId);
        SQLBindCol(stmt, 3, SQL_C_WCHAR, warehouseItemName, sizeof(warehouseItemName), &indWarehouseItemName);
        SQLBindCol(stmt, 4, SQL_C_LONG, &amount, 0, &indAmount);
        SQLBindCol(stmt, 5, SQL_C_LONG, &UOMId, 0, &indUOMId);
        SQLBindCol(stmt, 6, SQL_C_WCHAR, UOMName, sizeof(UOMName), &indUOMName);

        SQLRETURN ret = SQLFetch(stmt);

        if (ret == SQL_NO_DATA) {
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return true;
        }

        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        if (indTimestamp == SQL_NULL_DATA) {
            return false;
        }

        std::tm tm_struct{};
        tm_struct.tm_year = ts.year - 1900;
        tm_struct.tm_mon = ts.month - 1;
        tm_struct.tm_mday = ts.day;
        tm_struct.tm_hour = ts.hour;
        tm_struct.tm_min = ts.minute;
        tm_struct.tm_sec = ts.second;

        auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm_struct));

        out.timestamp = tp;
        
        WarehouseItem warehouseItem;
        warehouseItem.id = warehouseItemId;
        warehouseItem.name = warehouseItemName;
        UOM uom;
        uom.id = UOMId;
        uom.name = UOMName;
        out.data = ActivityStockAdded{ warehouseItem, amount, uom };

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return true;
    }
}