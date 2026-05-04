#include "db/products/products_queries.h"

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

	bool getProducts(SQLHDBC dbc, std::vector<Product>& products) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT p.id, p.Name, p.imgURL, p.QuantityPerUnit, u.id, u.Symbol, p.Price, p.StockQuantity "
            "FROM Products p "
            "JOIN UnitOfMeasure u ON p.UOMId = u.id "
            "WHERE p.is_deleted = 0";

        int productId = 0, quantityPerUnit = 0, UOMId = 0, stockQuantity = 0;
        double price = 0;
        wchar_t productName[101]{};
        wchar_t UOMName[11]{};
        char imgURL[256]{};
        SQLLEN indProductId, indProductName, indImgURL, indQuantityPerUnit, indUOMId, indUOMName, indPrice, indStockQuantity;

        if (!SQL_SUCCEEDED(SQLExecDirectA(stmt, (SQLCHAR*)query, SQL_NTS))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_LONG, &productId, 0, &indProductId);
        SQLBindCol(stmt, 2, SQL_C_WCHAR, productName, sizeof(productName), &indProductName);
        SQLBindCol(stmt, 3, SQL_C_CHAR, &imgURL, sizeof(imgURL), &indImgURL);
        SQLBindCol(stmt, 4, SQL_C_LONG, &quantityPerUnit, 0, &indQuantityPerUnit);
        SQLBindCol(stmt, 5, SQL_C_LONG, &UOMId, 0, &indUOMId);
        SQLBindCol(stmt, 6, SQL_C_WCHAR, UOMName, sizeof(UOMName), &indUOMName);
        SQLBindCol(stmt, 7, SQL_C_DOUBLE, &price, 0, &indPrice);
        SQLBindCol(stmt, 8, SQL_C_LONG, &stockQuantity, 0, &indStockQuantity);

        SQLRETURN ret;
        while ((ret = SQLFetch(stmt)) != SQL_NO_DATA) {
            if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
                Product p;
                p.id = productId;
                p.name = productName;
                if (indImgURL == SQL_NULL_DATA) p.imgURL = std::nullopt;
                else p.imgURL = imgURL;
                p.quantityPerUnit = quantityPerUnit;
                p.uom.id = UOMId;
                p.uom.name = UOMName;
                p.price = static_cast<int>(price * 100);
                p.stockQuantity = stockQuantity;

                products.push_back(std::move(p));
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

    bool productExistsByName(SQLHDBC dbc, const std::wstring& name) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT 1 FROM Products WHERE Name = ? AND is_deleted = 0";

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

    bool UOMExistsByID(SQLHDBC dbc, const int& id) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT 1 FROM UnitOfMeasure WHERE id = ?";

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

    bool ingredientsExistsById(SQLHDBC dbc, const int id) {
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

    bool ingredientsQuantityIsMoreLimit(SQLHDBC dbc, const int inStock, const Ingredients& ingredient) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT 1 FROM Consumables WHERE id = ? AND Quantity >= ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int id_copy = ingredient.warehouseItem.id;
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &id_copy, 0, NULL
        );
        int quantity = ingredient.quantity * inStock;
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

    bool insertProductItem(SQLHDBC dbc, int& productId, const ProductItemFull& prodItem) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "INSERT INTO Products "
            "(Name, Price, QuantityPerUnit, UOMId, StockQuantity, Description, CaloriesPer100g, Expiration, TempStorage, ManufacturerId) "
            "OUTPUT INSERTED.id "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

        SQLRETURN ret = SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLLEN nameLen = SQL_NTS;
        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0,
            (SQLPOINTER)prodItem.name.c_str(), 0, &nameLen);

        double price = prodItem.price / 100.0;
        SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 10, 2,
            &price, 0, NULL);

        int quantityPerUnit_copy = prodItem.quantityPerUnit;
        SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
            &quantityPerUnit_copy, 0, NULL);

        int uomId_copy = prodItem.uom.id;
        SQLBindParameter(stmt, 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
            &uomId_copy, 0, NULL);

        int inStock_copy = prodItem.inStock;
        SQLBindParameter(stmt, 5, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
            &inStock_copy, 0, NULL);

        SQLLEN descLen = SQL_NTS;
        SQLBindParameter(stmt, 6, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 4000, 0,
            (SQLPOINTER)prodItem.description.c_str(), 0, &descLen);

        int caloriesPer100g_copy = prodItem.caloriesPer100g;
        SQLBindParameter(stmt, 7, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
            &caloriesPer100g_copy, 0, NULL);

        int expiration_copy = prodItem.expiration;
        SQLBindParameter(stmt, 8, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
            &expiration_copy, 0, NULL);

        int tempStorage_copy = prodItem.tempStorage;
        SQLBindParameter(stmt, 9, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
            &tempStorage_copy, 0, NULL);

        int manufacturer_copy = prodItem.manufacturer.id;
        SQLBindParameter(stmt, 10, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
            &manufacturer_copy, 0, NULL);

        int newProductId = 0;
        SQLLEN indNewProductId;

        ret = SQLExecute(stmt);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_LONG, &newProductId, 0, &indNewProductId);

        ret = SQLFetch(stmt);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        productId = newProductId;

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);

        return true;
    }

    bool insertIngredients(SQLHDBC dbc, const int productId, const int ingredientsId, const int quantity) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "INSERT INTO Ingredients "
            "(ProductId, ConsumableId, Quantity) "
            "VALUES (?, ?, ?)";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int productId_copy = productId;
        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0,
            &productId_copy, 0, NULL);
        int ingredientsId_copy = ingredientsId;
        SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0,
            &ingredientsId_copy, 0, NULL);
        int quantity_copy = quantity;
        SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0,
            &quantity_copy, 0, NULL);

        SQLRETURN ret = SQLExecute(stmt);
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);

        return SQL_SUCCEEDED(ret);
    }

    bool ingredientsQuantityRemoveStock(SQLHDBC dbc, const int inStock, const Ingredients& ingredient) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "UPDATE Consumables SET Quantity = Quantity - ? WHERE id = ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int quantity = ingredient.quantity * inStock;
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &quantity, 0, NULL
        );

        int id_copy = ingredient.warehouseItem.id;
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

    bool updateProductImageURL(SQLHDBC dbc, const int productId, const std::wstring& savePath) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "UPDATE Products SET imgURL = ? WHERE id = ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_WCHAR, SQL_WVARCHAR,
            255, 0,
            (SQLPOINTER)savePath.c_str(), 0, NULL
        );

        int id_copy = productId;
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

    bool productExistsById(SQLHDBC dbc, const int id) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT 1 FROM Products WHERE id = ? AND is_deleted = 0";

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

    bool getProductItemFull(SQLHDBC dbc, const int id, ProductItemFull& prodItem) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT p.Name, p.imgURL, p.QuantityPerUnit, u.id, u.Symbol, p.Price, p.StockQuantity, "
            "p.Description, p.CaloriesPer100g, p.Expiration, p.TempStorage, m.id, m.Name "
            "FROM Products p "
            "JOIN UnitOfMeasure u ON p.UOMId = u.id "
            "JOIN Manufacturer m ON p.ManufacturerId = m.id "
            "WHERE p.id = ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int id_copy = id;
        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0,
            &id_copy, 0, NULL);

        int quantityPerUnit = 0, UOMId = 0, stockQuantity = 0,
            caloriesPer100g = 0, expiration = 0, tempStorage = 0, manufacturerId = 0;
        double price = 0;
        wchar_t productName[101]{};
        wchar_t UOMName[11]{};
        wchar_t description[4001]{};
        wchar_t manufacturerName[101]{};
        char imgURL[256]{};
        SQLLEN indProductId, indProductName, indImgURL, indQuantityPerUnit, indUOMId, indUOMName, indPrice, indStockQuantity,
            indDescription, indCaloriesPer100g, indExpiration, indTempStorage, indManufacturerId, indManufacturerName;

        if (!SQL_SUCCEEDED(SQLExecute(stmt))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_WCHAR, productName, sizeof(productName), &indProductName);
        SQLBindCol(stmt, 2, SQL_C_CHAR, &imgURL, sizeof(imgURL), &indImgURL);
        SQLBindCol(stmt, 3, SQL_C_LONG, &quantityPerUnit, 0, &indQuantityPerUnit);
        SQLBindCol(stmt, 4, SQL_C_LONG, &UOMId, 0, &indUOMId);
        SQLBindCol(stmt, 5, SQL_C_WCHAR, UOMName, sizeof(UOMName), &indUOMName);
        SQLBindCol(stmt, 6, SQL_C_DOUBLE, &price, 0, &indPrice);
        SQLBindCol(stmt, 7, SQL_C_LONG, &stockQuantity, 0, &indStockQuantity);
        SQLBindCol(stmt, 8, SQL_C_WCHAR, description, sizeof(description), &indDescription);
        SQLBindCol(stmt, 9, SQL_C_LONG, &caloriesPer100g, 0, &indCaloriesPer100g);
        SQLBindCol(stmt, 10, SQL_C_LONG, &expiration, 0, &indExpiration);
        SQLBindCol(stmt, 11, SQL_C_LONG, &tempStorage, 0, &indTempStorage);
        SQLBindCol(stmt, 12, SQL_C_LONG, &manufacturerId, 0, &indManufacturerId);
        SQLBindCol(stmt, 13, SQL_C_WCHAR, manufacturerName, sizeof(manufacturerName), &indManufacturerName);

        SQLRETURN ret = SQLFetch(stmt);
        if (!SQL_SUCCEEDED(ret)) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        prodItem.name = productName;
        if (indImgURL == SQL_NULL_DATA) prodItem.imgURL = std::nullopt;
        else prodItem.imgURL = imgURL;
        prodItem.imgURL = imgURL;
        prodItem.quantityPerUnit = quantityPerUnit;
        prodItem.uom.id = UOMId;
        prodItem.uom.name = UOMName;
        prodItem.price = price * 100.0;
        prodItem.inStock = stockQuantity;
        prodItem.description = description;
        prodItem.caloriesPer100g = caloriesPer100g;
        prodItem.expiration = expiration;
        prodItem.tempStorage = tempStorage;
        prodItem.manufacturer.id = manufacturerId;
        prodItem.manufacturer.name = manufacturerName;

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return true;
    }

    bool getIngredients(SQLHDBC dbc, const int id, std::vector<Ingredients>& ingredients) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "SELECT c.id, c.Name, i.Quantity, u.id, u.Symbol "
            "FROM Consumables c "
            "JOIN Ingredients i ON c.id = i.ConsumableId "
            "JOIN UnitOfMeasure u ON c.UOMId = u.id "
            "WHERE i.ProductId = ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int id_copy = id;
        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0,
            &id_copy, 0, NULL);

        int consumablesId = 0, quantity = 0, UOMId = 0;
        wchar_t consumablesName[101]{};
        wchar_t UOMName[11]{};
        SQLLEN indConsumablesId, indConsumablesName, indQuantity, indUOMId, indUOMName;

        if (!SQL_SUCCEEDED(SQLExecute(stmt))) {
            logError(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return false;
        }

        SQLBindCol(stmt, 1, SQL_C_LONG, &consumablesId, 0, &indConsumablesId);
        SQLBindCol(stmt, 2, SQL_C_WCHAR, consumablesName, sizeof(consumablesName), &indConsumablesName);
        SQLBindCol(stmt, 3, SQL_C_LONG, &quantity, 0, &indQuantity);
        SQLBindCol(stmt, 4, SQL_C_LONG, &UOMId, 0, &indUOMId);
        SQLBindCol(stmt, 5, SQL_C_WCHAR, UOMName, sizeof(UOMName), &indUOMName);

        SQLRETURN ret;
        while ((ret = SQLFetch(stmt)) != SQL_NO_DATA) {
            if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
                Ingredients i;
                i.warehouseItem.id = consumablesId;
                i.warehouseItem.name = consumablesName;
                i.quantity = quantity;
                i.uom.emplace();
                i.uom->id = UOMId;
                i.uom->name = UOMName;

                ingredients.push_back(std::move(i));
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

    bool deleteCartItem(SQLHDBC dbc, const int productId) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "DELETE FROM Cart WHERE ProductId = ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int productId_copy = productId;
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &productId_copy, 0, NULL
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

    bool deleteIngredients(SQLHDBC dbc, const int id) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "DELETE FROM Ingredients WHERE ProductId = ?";

        SQLPrepareA(stmt, (SQLCHAR*)query, SQL_NTS);
        int id_copy = id;
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT,
            SQL_C_LONG, SQL_INTEGER,
            0, 0,
            &id_copy, 0, NULL
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

    bool deleteProductItem(SQLHDBC dbc, const int id) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "UPDATE Products SET is_deleted = 1 WHERE id = ?";

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

    bool productItemAddStock(SQLHDBC dbc, const int id, const int quantity) {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        const char* query =
            "UPDATE Products SET StockQuantity = StockQuantity + ? WHERE id = ?";

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