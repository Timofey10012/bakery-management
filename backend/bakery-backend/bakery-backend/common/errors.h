#pragma once

#include <crow.h>
#include <unordered_map>

namespace common::errors {

    enum class ErrorCode {
        ValidationError,
        Unauthorized,
        Forbidden,
        EntityNotFound,
        Conflict,
        InternalError,

        UserNotFound,
        RoleNotFound,
        UOMNotFound,

        AuthLoginEmailNotFound,
        AuthLoginInvalidPassword,
        AuthRegisterEmailAlreadyExists,
        AuthRegisterPasswordNotValid,

        ProductNotFound,
        ProductAlreadyExists,
        ProductInsufficientStock,

        WarehouseItemNotFound,
        WarehouseItemAlreadyExists,
        WarehouseItemAlreadyUsedInProduct,
        WarehouseItemInsufficientAmount,

        SaleNotFound,
        SaleItemInsufficientStock,

        OrdersInsufficientStockAtCheckout,

        CartItemInsufficientStock,

        ManufacturerNotFound,
        ManufacturerAlreadyExists
    };

    struct ErrorInfo {
        int httpStatus;
        const char* apiCode;
    };

    inline const std::unordered_map<ErrorCode, ErrorInfo> errorMap = {
        { ErrorCode::ValidationError,                          {400, "VALIDATION_ERROR"} },
        { ErrorCode::Unauthorized,                             {401, "UNAUTHORIZED"} },
        { ErrorCode::Forbidden,                                {403, "FORBIDDEN"} },
        { ErrorCode::EntityNotFound,                           {404, "ENTITY_NOT_FOUND"} },
        { ErrorCode::Conflict,                                 {409, "CONFLICT"} },
        { ErrorCode::InternalError,                            {500, "INTERNAL_ERROR"} },

        { ErrorCode::UserNotFound,                             {404, "USER_NOT_FOUND"} },
        { ErrorCode::RoleNotFound,                             {404, "ROLE_NOT_FOUND"} },
        { ErrorCode::UOMNotFound,                              {404, "UOM_NOT_FOUND"} },

        { ErrorCode::AuthLoginEmailNotFound,                   {404, "AUTH_LOGIN_EMAIL_NOT_FOUND"} },
        { ErrorCode::AuthLoginInvalidPassword,                 {401, "AUTH_LOGIN_INVALID_PASSWORD"} },
        { ErrorCode::AuthRegisterEmailAlreadyExists,            {409, "AUTH_REGISTER_EMAIL_ALREADY_EXISTS"} },
        { ErrorCode::AuthRegisterPasswordNotValid,              {400, "AUTH_REGISTER_PASSWORD_NOT_VALID"} },

        { ErrorCode::ProductNotFound,                          {404, "PRODUCT_NOT_FOUND"} },
        { ErrorCode::ProductAlreadyExists,                     {409, "PRODUCT_ALREADY_EXISTS"} },
        { ErrorCode::ProductInsufficientStock,                 {409, "PRODUCT_INSUFFICIENT_STOCK"} },

        { ErrorCode::WarehouseItemNotFound,                    {404, "WAREHOUSE_ITEM_NOT_FOUND"} },
        { ErrorCode::WarehouseItemAlreadyExists,               {409, "WAREHOUSE_ITEM_ALREADY_EXISTS"} },
        { ErrorCode::WarehouseItemAlreadyUsedInProduct,        {409, "WAREHOUSE_ITEM_USED_IN_PRODUCT"} },
        { ErrorCode::WarehouseItemInsufficientAmount,          {409, "WAREHOUSE_ITEM_INSUFFICIENT_AMOUNT"} },

        { ErrorCode::SaleNotFound,                             {404, "SALE_NOT_FOUND"} },
        { ErrorCode::SaleItemInsufficientStock,                {409, "SALE_ITEM_INSUFFICIENT_STOCK"} },

        { ErrorCode::OrdersInsufficientStockAtCheckout,        {409, "ORDERS_INSUFFICIENT_STOCK_AT_CHECKOUT"} },

        { ErrorCode::CartItemInsufficientStock,                {409, "CART_ITEM_INSUFFICIENT_STOCK"} },

        { ErrorCode::ManufacturerNotFound,                     {404, "MANUFACTURER_NOT_FOUND"} },
        { ErrorCode::ManufacturerAlreadyExists,                {409, "MANUFACTURER_ALREADY_EXISTS"} }
    };

    inline crow::response makeError(ErrorCode code) {
        auto it = errorMap.find(code);

        if (it == errorMap.end()) {
            crow::json::wvalue json;
            json["error"]["code"] = "INTERNAL_ERROR";
            return crow::response(500, json);
        }

        const auto& info = it->second;

        crow::json::wvalue json;
        json["error"]["code"] = info.apiCode;

        return crow::response(info.httpStatus, json);
    }

}