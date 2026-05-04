#include "db/sales/sales_queries.h"

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

    bool getSaleShort(SQLHDBC dbc, std::vector<SaleShort>& sales) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT s.id, u.id, u.FirstName, u.LastName, u.Email, r.id, r.Name, (SELECT COALESCE(SUM(si.QuantityItems * si.PriceAtPurchase), 0) "
            "FROM SalesItem si "
            "WHERE si.SalesId = s.id) "
            "FROM Sales s "
            "LEFT JOIN Users u ON s.UserId = u.id "
            "LEFT JOIN Roles r ON u.RoleId = r.id";


        double total = 0;
        int saleId = 0, userId = 0, roleId = 0;
        wchar_t firstName[101]{};
        wchar_t lastName[101]{};
        wchar_t email[321]{};
        wchar_t roleName[51]{};
        SQLLEN indSaleId, indUserId, indFirstName, indLastName, indEmail, indRoleId, indRoleName, indTotal;

        if (!SQL_SUCCEEDED(SQLExecDirectA(stmt, (SQLCHAR*)query, SQL_NTS))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_LONG, &saleId, 0, &indSaleId);
        SQLBindCol(stmt, 2, SQL_C_LONG, &userId, 0, &indUserId);
        SQLBindCol(stmt, 3, SQL_C_WCHAR, firstName, sizeof(firstName), &indFirstName);
        SQLBindCol(stmt, 4, SQL_C_WCHAR, lastName, sizeof(lastName), &indLastName);
        SQLBindCol(stmt, 5, SQL_C_WCHAR, email, sizeof(email), &indEmail);
        SQLBindCol(stmt, 6, SQL_C_LONG, &roleId, 0, &indRoleId);
        SQLBindCol(stmt, 7, SQL_C_WCHAR, roleName, sizeof(roleName), &indRoleName);
        SQLBindCol(stmt, 8, SQL_C_DOUBLE, &total, 0, &indTotal);


        SQLRETURN ret;
        while ((ret = SQLFetch(stmt)) != SQL_NO_DATA) {
            if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
                SaleShort s;
                s.id = saleId;
                if (indUserId != SQL_NULL_DATA) {
                    s.user.emplace();
                    s.user->id = userId;
                    s.user->firstName = firstName;
                    s.user->lastName = lastName;
                    s.user->email = email;
                    s.user->role.id = roleId;
                    s.user->role.name = roleName;
                }
                else s.user = std::nullopt;
                s.total = static_cast<int>(total * 100.0 + 0.5);

                sales.push_back(std::move(s));
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

    bool saleExistsById(SQLHDBC dbc, const int id) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT 1 FROM Sales WHERE id = ?";

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

    bool getSaleUser(SQLHDBC dbc, const int id, Sale& sale) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT u.id, u.FirstName, u.LastName, u.Email, r.id, r.Name "
            "FROM Sales s "
            "LEFT JOIN Users u ON s.UserId = u.id "
            "LEFT JOIN Roles r ON u.RoleId = r.id "
            "WHERE s.id = ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int id_copy = id;
        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0,
            &id_copy, 0, NULL);

        int userId = 0, roleId = 0;
        wchar_t firstName[101]{};
        wchar_t lastName[101]{};
        wchar_t email[321]{};
        wchar_t roleName[51]{};
        SQLLEN indUserId, indFirstName, indLastName, indEmail, indRoleId, indRoleName;

        if (!SQL_SUCCEEDED(SQLExecute(stmt))) {
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

        SQLRETURN ret = SQLFetch(stmt);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        if (indFirstName != SQL_NULL_DATA) {
            sale.user.emplace();
            sale.user->id = userId;
            sale.user->firstName = firstName;
            sale.user->lastName = lastName;
            sale.user->email = email;
            sale.user->role.id = roleId;
            sale.user->role.name = roleName;
        }
        else sale.user = std::nullopt;

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return true;
    }

    bool getSaleItemsAndTotal(SQLHDBC dbc, const int id, std::vector<SalesItem>& salesItems, int& total) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT p.id, p.Name, si.QuantityItems, si.PriceAtPurchase, (si.QuantityItems * si.PriceAtPurchase) "
            "FROM SalesItem si "
            "JOIN Products p ON si.ProductId = p.id "
            "WHERE si.SalesId = ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int id_copy = id;
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &id_copy, 0, NULL
        );

        double priceAtPurchase = 0, totalPricePerProduct = 0;
        int productId = 0, quantityItems = 0;
        wchar_t productName[101]{};
        SQLLEN indProductId, indProductName, indQuantityItems, indPriceAtPurchase, indTotalPricePerProduct;

        if (!SQL_SUCCEEDED(SQLExecute(stmt))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_LONG, &productId, 0, &indProductId);
        SQLBindCol(stmt, 2, SQL_C_WCHAR, productName, sizeof(productName), &indProductName);
        SQLBindCol(stmt, 3, SQL_C_LONG, &quantityItems, 0, &indQuantityItems);
        SQLBindCol(stmt, 4, SQL_C_DOUBLE, &priceAtPurchase, 0, &indPriceAtPurchase);
        SQLBindCol(stmt, 5, SQL_C_DOUBLE, &totalPricePerProduct, 0, &indTotalPricePerProduct);

        SQLRETURN ret;
        total = 0;
        while ((ret = SQLFetch(stmt)) != SQL_NO_DATA) {
            if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
                SalesItem si;
                si.productItem.id = productId;
                si.productItem.name = productName;
                si.quantityItems = quantityItems;
                si.pricePerUnit = static_cast<int>(priceAtPurchase * 100.0 + 0.5);
                si.totalPricePerProduct = static_cast<int>(totalPricePerProduct * 100.0 + 0.5);
                total += si.totalPricePerProduct;

                salesItems.push_back(std::move(si));
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

}