#include "db/cart/cart_queries.h"

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

    bool getCartItems(SQLHDBC dbc, const int userId, std::vector<CartItem>& cartItems) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT p.id, p.Name, c.Quantity "
            "FROM Cart c "
            "JOIN Products p ON c.ProductId = p.id "
            "WHERE UserId = ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int id_copy = userId;
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &id_copy, 0, NULL
        );

        int productId = 0, quantityItems = 0;
        wchar_t productName[101]{};
        SQLLEN indProductId, indProductName, indQuantityItems;

        if (!SQL_SUCCEEDED(SQLExecute(stmt))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_LONG, &productId, 0, &indProductId);
        SQLBindCol(stmt, 2, SQL_C_WCHAR, productName, sizeof(productName), &indProductName);
        SQLBindCol(stmt, 3, SQL_C_LONG, &quantityItems, 0, &indQuantityItems);

        SQLRETURN ret;
        while ((ret = SQLFetch(stmt)) != SQL_NO_DATA) {
            if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
                CartItem ci;
                ci.productItem.id = productId;
                ci.productItem.name = productName;
                ci.quantityItems = quantityItems;

                cartItems.push_back(std::move(ci));
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

    bool productQuantityIsMoreLimit(SQLHDBC dbc, const CartItem& cartItem) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT 1 FROM Products WHERE id = ? AND StockQuantity >= ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int id_copy = cartItem.productItem.id;
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &id_copy, 0, NULL
        );
        int quantity = cartItem.quantityItems;
        SQLBindParameter(
            stmt, 2, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &quantity, 0, NULL
        );

        SQLExecute(stmt);

        SQLRETURN ret = SQLFetch(stmt);
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);

        return ret == SQL_NO_DATA;
    }

    bool cartItemExistsByProductId(SQLHDBC dbc, const int userId, const int productId) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT 1 FROM Cart WHERE UserId = ? AND ProductId = ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int userId_copy = userId;
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &userId_copy, 0, NULL
        );

        int productId_copy = productId;
        SQLBindParameter(
            stmt, 2, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &productId_copy, 0, NULL
        );

        SQLExecute(stmt);

        SQLRETURN ret = SQLFetch(stmt);
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);

        return ret != SQL_NO_DATA;
    }

    bool insertCartItem(SQLHDBC dbc, const int userId, const CartItem& cartItem) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "INSERT INTO Cart "
            "(UserId, ProductId, Quantity) "
            "VALUES (?, ?, ?)";

        SQLRETURN ret = SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            return false;
        }

        int userId_copy = userId;
        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
            &userId_copy, 0, NULL);

        int productId_copy = cartItem.productItem.id;
        SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
            &productId_copy, 0, NULL);

        int quantityItems_copy = cartItem.quantityItems;
        SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
            &quantityItems_copy, 0, NULL);

        ret = SQLExecute(stmt);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);

        return true;
    }

    bool deleteCartItem(SQLHDBC dbc, const int userId, const int productId) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "DELETE FROM Cart WHERE UserId = ? AND ProductId = ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int userId_copy = userId;
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &userId_copy, 0, NULL
        );
        
        int productId_copy = productId;
        SQLBindParameter(
            stmt, 2, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &productId_copy, 0, NULL
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

    bool cartItemAddStock(SQLHDBC dbc, const int userId, const CartItem& cartItem) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "UPDATE Cart SET Quantity = Quantity + ? WHERE UserId = ? AND ProductId = ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int quantity_copy = cartItem.quantityItems;
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &quantity_copy, 0, NULL
        );

        int userId_copy = userId;
        SQLBindParameter(
            stmt, 2, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &userId_copy, 0, NULL
        );

        int productId_copy = cartItem.productItem.id;
        SQLBindParameter(
            stmt, 3, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &productId_copy, 0, NULL
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

    bool cartItemQuantityIsMoreLimit(SQLHDBC dbc, const int userId, const CartItem& cartItem) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT 1 FROM Cart WHERE UserId = ? AND ProductId = ? AND Quantity >= ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int userId_copy = userId;
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &userId_copy, 0, NULL
        );

        int productId_copy = cartItem.productItem.id;
        SQLBindParameter(
            stmt, 2, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &productId_copy, 0, NULL
        );

        int quantity = cartItem.quantityItems;
        SQLBindParameter(
            stmt, 3, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &quantity, 0, NULL
        );

        SQLExecute(stmt);

        SQLRETURN ret = SQLFetch(stmt);
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);

        return ret == SQL_NO_DATA;
    }

    bool cartItemRemoveStock(SQLHDBC dbc, const int userId, const CartItem& cartItem) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "UPDATE Cart SET Quantity = Quantity - ? WHERE UserId = ? AND ProductId = ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int quantity_copy = cartItem.quantityItems;
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &quantity_copy, 0, NULL
        );

        int userId_copy = userId;
        SQLBindParameter(
            stmt, 2, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &userId_copy, 0, NULL
        );

        int productId_copy = cartItem.productItem.id;
        SQLBindParameter(
            stmt, 3, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &productId_copy, 0, NULL
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

    bool getSalesItemsFromCart(SQLHDBC dbc, const int userId, std::vector<SalesItem>& salesItems) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT p.id, p.Name, c.Quantity, p.Price, (c.Quantity * p.Price) "
            "FROM Cart c "
            "JOIN Products p ON c.ProductId = p.id "
            "WHERE c.UserId = ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int userId_copy = userId;
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &userId_copy, 0, NULL
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
        while ((ret = SQLFetch(stmt)) != SQL_NO_DATA) {
            if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
                SalesItem si;
                si.productItem.id = productId;
                si.productItem.name = productName;
                si.quantityItems = quantityItems;
                si.pricePerUnit = static_cast<int>(priceAtPurchase * 100.0 + 0.5);
                si.totalPricePerProduct = static_cast<int>(totalPricePerProduct * 100.0 + 0.5);

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

    bool salesItemQuantityIsMoreLimit(SQLHDBC dbc, const SalesItem& salesItem) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT 1 FROM Products WHERE id = ? AND StockQuantity >= ? AND is_deleted = 0";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int id_copy = salesItem.productItem.id;
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &id_copy, 0, NULL
        );
        int quantity = salesItem.quantityItems;
        SQLBindParameter(
            stmt, 2, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &quantity, 0, NULL
        );

        SQLExecute(stmt);

        SQLRETURN ret = SQLFetch(stmt);
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);

        return ret == SQL_NO_DATA;
    }

    bool insertSales(SQLHDBC dbc, const int userId, int& salesId) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "INSERT INTO Sales "
            "(UserId) "
            "OUTPUT INSERTED.id "
            "VALUES (?)";

        SQLRETURN ret = SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            return false;
        }

        int userId_copy = userId;
        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
            &userId_copy, 0, NULL);

        int newSalesId = 0;
        SQLLEN indNewSalesId;

        ret = SQLExecute(stmt);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_LONG, &newSalesId, 0, &indNewSalesId);

        ret = SQLFetch(stmt);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        salesId = newSalesId;

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);

        return true;
    }

    bool insertSales(SQLHDBC dbc, int& salesId) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "INSERT INTO Sales "
            "(UserId) "
            "OUTPUT INSERTED.id "
            "VALUES (NULL)";

        int newSalesId = 0;
        SQLLEN indNewSalesId;

        if (!SQL_SUCCEEDED(SQLExecDirectA(stmt, (SQLCHAR*)query, SQL_NTS))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_LONG, &newSalesId, 0, &indNewSalesId);

        SQLRETURN ret = SQLFetch(stmt);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        salesId = newSalesId;

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);

        return true;
    }

    bool insertSalesItem(SQLHDBC dbc, const int salesId, const SalesItem& salesItem) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "INSERT INTO SalesItem "
            "(SalesId, ProductId, QuantityItems, PriceAtPurchase) "
            "VALUES (?, ?, ?, ?)";

        SQLRETURN ret = SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            return false;
        }

        int salesId_copy = salesId;
        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
            &salesId_copy, 0, NULL);

        int productId_copy = salesItem.productItem.id;
        SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
            &productId_copy, 0, NULL);

        int quantityItems_copy = salesItem.quantityItems;
        SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
            &quantityItems_copy, 0, NULL);

        double priceAtPurchase = salesItem.pricePerUnit / 100.0;
        SQLBindParameter(stmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 10, 2,
            &priceAtPurchase, 0, NULL);

        ret = SQLExecute(stmt);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);

        return true;
    }

    bool productRemoveStock(SQLHDBC dbc, const int productId, const int quantity) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "UPDATE Products SET StockQuantity = StockQuantity - ? WHERE id = ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int quantity_copy = quantity;
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &quantity_copy, 0, NULL
        );

        int productId_copy = productId;
        SQLBindParameter(
            stmt, 2, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &productId_copy, 0, NULL
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