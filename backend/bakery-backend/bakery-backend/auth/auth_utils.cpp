#define _CRT_SECURE_NO_WARNINGS

#include "auth/auth_utils.h"

#include "windows.h"
#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/traits.h>
#include <openssl/evp.h>
#include <cstring>
#include <cstdio>

namespace auth {

    using jwt_traits = jwt::traits::nlohmann_json;

    namespace {
        std::optional<common::errors::ErrorCode>
            extractToken(const crow::request & req, std::string & outToken) {
            auto authHeader = req.get_header_value("Authorization");
            if (authHeader.empty()) {
                return common::errors::ErrorCode::Unauthorized;
            }

            const std::string prefix = "Bearer ";
            if (!authHeader.starts_with(prefix)) {
                return common::errors::ErrorCode::Unauthorized;
            }

            outToken = authHeader.substr(prefix.size());
            return std::nullopt;
        }

        std::optional<common::errors::ErrorCode>
            parseToken(const std::string& token, Context& outCtx) {
            try {
                const char* jwtSecretKey = std::getenv("JWT_SECRET");
                if (!jwtSecretKey) {
                    std::cerr << "JWT_SECRET is not set\n";
                    return common::errors::ErrorCode::InternalError;
                }

                auto decoded = jwt::decode<jwt_traits>(token);

                auto verifier = jwt::verify<jwt_traits>()
                    .allow_algorithm(jwt::algorithm::hs256{ jwtSecretKey })
                    .with_issuer("bakery-backend");

                verifier.verify(decoded);

                auto userIdClaim = decoded.get_payload_claim("userId");
                auto roleIdClaim = decoded.get_payload_claim("roleId");

                int userId = std::stoi(userIdClaim.as_string());
                int roleId = std::stoi(roleIdClaim.as_string());

                outCtx.userId = userId;

                switch (roleId) {
                case 1: outCtx.role = Role::User; break;
                case 2: outCtx.role = Role::Seller; break;
                case 3: outCtx.role = Role::Admin; break;
                default:
                    return common::errors::ErrorCode::Forbidden;
                }

                return std::nullopt;
            }
            catch (const std::exception& e) {
                std::cerr << "JWT parse error: " << e.what() << std::endl;
                return common::errors::ErrorCode::Unauthorized;
            }
        }
    }

    std::optional<common::errors::ErrorCode> fromRequest(const crow::request& req, Context& outCtx) {
        std::string token;

        if (auto err = extractToken(req, token))
            return err;

        if (auto err = parseToken(token, outCtx))
            return err;

        return std::nullopt;
    }

    std::string hashPassword(const std::string& password) {
        const unsigned char salt[16] = "bakerysalt12345";

        unsigned char hash[32];
        PKCS5_PBKDF2_HMAC(password.c_str(), (int)password.size(), salt, 16, 100000, EVP_sha256(), 32, hash);

        std::string result;
        char hex[3];
        for (int i = 0; i < 32; i++) {
            snprintf(hex, sizeof(hex), "%02x", hash[i]);
            result += hex;
        }
        return result;
    }

}