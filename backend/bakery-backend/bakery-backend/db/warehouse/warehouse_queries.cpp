#include "db/warehouse/warehouse_queries.h"

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

	bool getWarehouse(SQLHDBC dbc, std::vector<Warehouse>& warehouse) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT c.id, c.Name, c.Quantity, c.MinQuantity, u.id, u.Symbol, "
            "ISNULL((SELECT MAX(cr.ReplenishmentAt) FROM ConsumableReplenishments cr WHERE cr.ConsumableId = c.id), GETDATE()) AS ReplenishmentAt "
            "FROM Consumables c "
            "JOIN UnitOfMeasure u ON c.UOMId = u.id "
            "WHERE c.is_deleted = 0";

        TIMESTAMP_STRUCT sd;
        int warehouseId = 0, quantity = 0, minQuantity = 0, UOMId = 0;
        wchar_t warehouseName[101]{};
        wchar_t UOMName[11]{};
        SQLLEN indWarehouseId, indWarehouseName, indQuantity, indMinQuantity, indUOMId, indUOMName, indSupplyDate;

        if (!SQL_SUCCEEDED(SQLExecDirectA(stmt, (SQLCHAR*)query, SQL_NTS))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_LONG, &warehouseId, 0, &indWarehouseId);
        SQLBindCol(stmt, 2, SQL_C_WCHAR, warehouseName, sizeof(warehouseName), &indWarehouseName);
        SQLBindCol(stmt, 3, SQL_C_LONG, &quantity, 0, &indQuantity);
        SQLBindCol(stmt, 4, SQL_C_LONG, &minQuantity, 0, &indMinQuantity);
        SQLBindCol(stmt, 5, SQL_C_LONG, &UOMId, 0, &indUOMId);
        SQLBindCol(stmt, 6, SQL_C_WCHAR, UOMName, sizeof(UOMName), &indUOMName);
        SQLBindCol(stmt, 7, SQL_C_TYPE_TIMESTAMP, &sd, sizeof(sd), &indSupplyDate);


        SQLRETURN ret;
        while ((ret = SQLFetch(stmt)) != SQL_NO_DATA) {
            if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
                Warehouse w;
                w.id = warehouseId;
                w.name = warehouseName;
                w.quantity = quantity;
                w.minQuantity = minQuantity;
                w.uom.id = UOMId;
                w.uom.name = UOMName;
                
                if (indSupplyDate == SQL_NULL_DATA) {
                    return false;
                }

                std::tm tm_struct{};
                tm_struct.tm_year = sd.year - 1900;
                tm_struct.tm_mon = sd.month - 1;
                tm_struct.tm_mday = sd.day;
                tm_struct.tm_hour = sd.hour;
                tm_struct.tm_min = sd.minute;
                tm_struct.tm_sec = sd.second;

                auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm_struct));

                w.supplyDate = tp;

                w.isInStock = w.quantity > w.minQuantity ? true : false;

                warehouse.push_back(std::move(w));
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

    bool warehouseExistsByName(SQLHDBC dbc, const std::wstring& name) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT 1 FROM Consumables WHERE Name = ? AND is_deleted = 0";

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

    bool insertWarehouseItem(SQLHDBC dbc, int& warehouseId, const WarehouseItemAdd& warehouseItem) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "INSERT INTO Consumables "
            "(Name, Quantity, MinQuantity, UOMId) "
            "OUTPUT INSERTED.id "
            "VALUES (?, ?, ?, ?)";

        SQLRETURN ret = SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLLEN nameLen = SQL_NTS;
        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0,
            (SQLPOINTER)warehouseItem.name.c_str(), 0, &nameLen);

        int quantity_copy = warehouseItem.quantity;
        SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
            &quantity_copy, 0, NULL);

        int minQuantity_copy = warehouseItem.minQuantity;
        SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
            &minQuantity_copy, 0, NULL);

        int uomId_copy = warehouseItem.uom.id;
        SQLBindParameter(stmt, 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
            &uomId_copy, 0, NULL);

        int newWarehouseId = 0;
        SQLLEN indNewWarehouseId;

        ret = SQLExecute(stmt);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_LONG, &newWarehouseId, 0, &indNewWarehouseId);

        ret = SQLFetch(stmt);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        warehouseId = newWarehouseId;

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);

        return true;
    }

    bool insertWarehouseReplenishments(SQLHDBC dbc, const int warehouseId, const int quantity) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "INSERT INTO ConsumableReplenishments "
            "(ConsumableId, Quantity) "
            "VALUES (?, ?)";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int productId_copy = warehouseId;
        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0,
            &productId_copy, 0, NULL);

        int quantity_copy = quantity;
        SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0,
            &quantity_copy, 0, NULL);

        SQLRETURN ret = SQLExecute(stmt);
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);

        return SQL_SUCCEEDED(ret);
    }

    bool warehouseExistsById(SQLHDBC dbc, const int id) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT 1 FROM Consumables WHERE id = ? AND is_deleted = 0";

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

    bool getWarehouseItemShort(SQLHDBC dbc, const int id, WarehouseItemShort& warehouseItem) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT c.Name, c.Quantity, u.id, u.Symbol "
            "FROM Consumables c "
            "JOIN UnitOfMeasure u ON c.UOMId = u.id "
            "WHERE c.id = ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int id_copy = id;
        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0,
            &id_copy, 0, NULL);

        int quantity = 0, UOMId = 0;
        wchar_t warehouseName[101]{};
        wchar_t UOMName[11]{};
        SQLLEN indWarehouseName, indQuantity, indUOMId, indUOMName;

        if (!SQL_SUCCEEDED(SQLExecute(stmt))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_WCHAR, warehouseName, sizeof(warehouseName), &indWarehouseName);
        SQLBindCol(stmt, 2, SQL_C_LONG, &quantity, 0, &indQuantity);
        SQLBindCol(stmt, 3, SQL_C_LONG, &UOMId, 0, &indUOMId);
        SQLBindCol(stmt, 4, SQL_C_WCHAR, UOMName, sizeof(UOMName), &indUOMName);
        
        SQLRETURN ret = SQLFetch(stmt);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        warehouseItem.id = id;
        warehouseItem.name = warehouseName;
        warehouseItem.quantity = quantity;
        warehouseItem.uom.id = UOMId;
        warehouseItem.uom.name = UOMName;

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return true;
    }

    bool warehouseItemUsedInProduct(SQLHDBC dbc, const int id) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT 1 FROM Ingredients WHERE ConsumableId = ?";

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

    bool deleteWarehouseItem(SQLHDBC dbc, const int id) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "UPDATE Consumables SET is_deleted = 1 WHERE id = ?";

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

    bool warehouseItemAddStock(SQLHDBC dbc, const int id, const int quantity) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "UPDATE Consumables SET Quantity = Quantity + ? WHERE id = ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int quantity_copy = quantity;
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &quantity_copy, 0, NULL
        );

        int id_copy = id;
        SQLBindParameter(
            stmt, 2, SQL_PARAM_INPUT,
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

    bool warehouseRemoveQuantityIsMoreLimit(SQLHDBC dbc, const int id, const int quantity) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT 1 FROM Consumables WHERE id = ? AND Quantity >= ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int id_copy = id;
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &id_copy, 0, NULL
        );
        int quantity_copy = quantity;
        SQLBindParameter(
            stmt, 2, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &quantity_copy, 0, NULL
        );

        SQLExecute(stmt);

        SQLRETURN ret = SQLFetch(stmt);
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);

        return ret == SQL_NO_DATA;
    }

    bool warehouseItemRemoveStock(SQLHDBC dbc, const int id, const int quantity) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "UPDATE Consumables SET Quantity = Quantity - ? WHERE id = ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int quantity_copy = quantity;
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &quantity_copy, 0, NULL
        );

        int id_copy = id;
        SQLBindParameter(
            stmt, 2, SQL_PARAM_INPUT,
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