#define _CRT_SECURE_NO_WARNINGS

#include "auth/auth_service.h"
#include "db/auth/auth_queries.h"
#include "auth/auth_utils.h"
#include "models/user_model.h"

#include "windows.h"
#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/traits.h>
#include <openssl/evp.h>
#include <openssl/core_names.h>
#include <openssl/err.h>
#include <string>
#include <stdexcept>
#include <chrono>
#include <optional>

namespace auth_service {

    using jwt_traits = jwt::traits::nlohmann_json;

    static bool verifyPassword(const std::string& password, const std::string& passwordHash) {
        const unsigned char salt[16] = "bakerysalt12345";

        unsigned char hash[32];
        PKCS5_PBKDF2_HMAC(password.c_str(), (int)password.size(), salt, 16, 100000, EVP_sha256(), 32, hash);

        std::string computed;
        char hex[3];
        for (int i = 0; i < 32; i++) {
            snprintf(hex, sizeof(hex), "%02x", hash[i]);
            computed += hex;
        }

        return computed == passwordHash;
    }

    static bool generateToken(int userId, int roleId, std::string& token) {
        using namespace std::chrono;

        const char* jwtSecretKey = std::getenv("JWT_SECRET");
        if (!jwtSecretKey) {
            std::cerr << "JWT_SECRET is not set\n";
            return false;
        }

        token = jwt::create<jwt_traits>()
            .set_type("JWT")
            .set_issuer("bakery-backend")
            .set_issued_at(system_clock::now())
            .set_expires_at(system_clock::now() + hours(24))
            .set_payload_claim("userId", jwt::basic_claim<jwt_traits>(std::to_string(userId)))
            .set_payload_claim("roleId", jwt::basic_claim<jwt_traits>(std::to_string(roleId)))
            .sign(jwt::algorithm::hs256{ jwtSecretKey });

        return true;
    }

    using common::errors::ErrorCode;

    std::optional<ErrorCode> registerUser(const RegisterBody& body, AuthResponse& out) {
        if (body.firstName.empty() || body.lastName.empty())
            return ErrorCode::ValidationError;

        if (body.email.empty())
            return ErrorCode::ValidationError;

        if (body.password.size() < 8)
            return ErrorCode::AuthRegisterPasswordNotValid;

        if (db::userExistsByEmail(db::connection(), body.email))
            return ErrorCode::AuthRegisterEmailAlreadyExists;

        std::string passwordHash = auth::hashPassword(body.password);

        int roleId = 1;

        if (!db::insertUser(
            db::connection(),
            body.firstName,
            body.lastName,
            body.email,
            passwordHash,
            roleId
        )) {
            return ErrorCode::InternalError;
        }

        User user;
        if (!db::getUserByEmail(db::connection(), body.email, user)) {
            return ErrorCode::InternalError;
        }

        if (!generateToken(user.id, user.role.id, out.token)) {
            return ErrorCode::InternalError;
        }

        out.user = user;

        return std::nullopt;
    }

    std::optional<ErrorCode> loginUser(const LoginBody& body, AuthResponse& out) {
        if (body.email.empty())
            return ErrorCode::ValidationError;

        if (body.password.size() < 8)
            return ErrorCode::AuthRegisterPasswordNotValid;

        if (!db::userExistsByEmail(db::connection(), body.email))
            return ErrorCode::AuthLoginEmailNotFound;

        std::string passwordHash;

        if (!db::getPasswordHashByEmail(db::connection(), body.email, passwordHash)) {
            return ErrorCode::InternalError;
        }

        if (!verifyPassword(body.password, passwordHash))
            return ErrorCode::AuthLoginInvalidPassword;


        User user;
        if (!db::getUserByEmail(db::connection(), body.email, user)) {
            return ErrorCode::InternalError;
        }

        if (!generateToken(user.id, user.role.id, out.token)) {
            return ErrorCode::InternalError;
        }

        out.user = user;

        return std::nullopt;

    }

    std::optional<ErrorCode> getMe(const int& userId, User& out) {
        if (!userId)
            return ErrorCode::Unauthorized;

        if (!db::getUserById(db::connection(), userId, out)) {
            return ErrorCode::InternalError;
        }

        return std::nullopt;
    }
}