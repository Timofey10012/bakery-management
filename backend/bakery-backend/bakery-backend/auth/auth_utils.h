#pragma once

#include <optional>

#include "common/errors.h"

namespace auth {

    enum class Role {
        User,
        Seller,
        Admin
    };

    struct Context {
        int userId;
        Role role;
    };

    std::optional<common::errors::ErrorCode> fromRequest(const crow::request& req, Context& outCtx);

    std::string hashPassword(const std::string& password);
}
