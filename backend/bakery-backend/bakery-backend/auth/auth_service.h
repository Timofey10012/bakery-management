#pragma once
#include "auth/register_body_model.h"
#include "auth/auth_response_model.h"
#include "auth/login_body_model.h"
#include "common/errors.h"

#include <string>
#include <optional>

namespace auth_service {

    using common::errors::ErrorCode;

    std::optional<ErrorCode> registerUser(const RegisterBody& body, AuthResponse& out);

    std::optional<ErrorCode> loginUser(const LoginBody& body, AuthResponse& out);

    std::optional<ErrorCode> getMe(const int& userId, User& out);

}
