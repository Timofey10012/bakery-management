#include "auth/auth_controller.h"
#include "common/string_utils.h"
#include "auth/auth_utils.h"
#include "common/utils.h"

#include <iostream>

namespace auth_controller {

    void setupRoutes(crow::App<crow::CORSHandler>& app) {

        CROW_ROUTE(app, "/auth/register").methods(crow::HTTPMethod::Post)(
            [](const crow::request& req) {
                try {
                    auto bodyJson = crow::json::load(req.body);
                    if (!bodyJson) {
                        return common::errors::makeError(common::errors::ErrorCode::ValidationError);
                    }

                    if (jsonNotHasKeysAndisNull(bodyJson, { "firstName" }) || jsonNotHasKeysAndisNull(bodyJson, { "lastName" }) ||
                        jsonNotHasKeysAndisNull(bodyJson, { "email" }) || jsonNotHasKeysAndisNull(bodyJson, { "password" })) {
                        return common::errors::makeError(common::errors::ErrorCode::ValidationError);
                    }

                    RegisterBody body;
                    body.firstName = utils::utf8_to_utf16(bodyJson["firstName"].s());
                    body.lastName = utils::utf8_to_utf16(bodyJson["lastName"].s());
                    body.email = utils::utf8_to_utf16(bodyJson["email"].s());
                    body.password = bodyJson["password"].s();

                    AuthResponse res;
                    if (auto err = auth_service::registerUser(body, res)) {
                        return common::errors::makeError(*err);
                    }

                    crow::json::wvalue respJson;
                    respJson["token"] = res.token;
                    respJson["user"]["id"] = res.user.id;
                    respJson["user"]["firstName"] = utils::utf16_to_utf8(res.user.firstName);
                    respJson["user"]["lastName"] = utils::utf16_to_utf8(res.user.lastName);
                    respJson["user"]["email"] = utils::utf16_to_utf8(res.user.email);
                    respJson["user"]["role"]["id"] = res.user.role.id;
                    if (auto nameOpt = utils::utf16_to_utf8_opt(res.user.role.name)) {
                        respJson["user"]["role"]["name"] = *nameOpt;
                    }
                    else {
                        respJson["user"]["role"]["name"] = nullptr;
                    }


                    return crow::response{ respJson };

                }
                catch (const std::exception& e) {
                    std::cerr << "Register error: " << e.what() << std::endl;
                    return common::errors::makeError(common::errors::ErrorCode::InternalError);
                }
            }
            );

        CROW_ROUTE(app, "/auth/login").methods(crow::HTTPMethod::Post)(
            [](const crow::request& req) {
                try {
                    auto bodyJson = crow::json::load(req.body);
                    if (!bodyJson) {
                        return common::errors::makeError(common::errors::ErrorCode::ValidationError);
                    }

                    if (jsonNotHasKeysAndisNull(bodyJson, { "email" }) || jsonNotHasKeysAndisNull(bodyJson, { "password" })) {
                        return common::errors::makeError(common::errors::ErrorCode::ValidationError);
                    }

                    LoginBody body;
                    body.email = utils::utf8_to_utf16(bodyJson["email"].s());
                    body.password = bodyJson["password"].s();

                    AuthResponse res;
                    if (auto err = auth_service::loginUser(body, res)) {
                        return common::errors::makeError(*err);
                    }

                    crow::json::wvalue respJson;
                    respJson["token"] = res.token;
                    respJson["user"]["id"] = res.user.id;
                    respJson["user"]["firstName"] = utils::utf16_to_utf8(res.user.firstName);
                    respJson["user"]["lastName"] = utils::utf16_to_utf8(res.user.lastName);
                    respJson["user"]["email"] = utils::utf16_to_utf8(res.user.email);
                    respJson["user"]["role"]["id"] = res.user.role.id;
                    if (auto nameOpt = utils::utf16_to_utf8_opt(res.user.role.name)) {
                        respJson["user"]["role"]["name"] = *nameOpt;
                    }
                    else {
                        respJson["user"]["role"]["name"] = "";
                    }


                    return crow::response{ respJson };

                }
                catch (const std::exception& e) {
                    std::cerr << "Login error: " << e.what() << std::endl;
                    return common::errors::makeError(common::errors::ErrorCode::InternalError);
                }
            }
            );
            
        CROW_ROUTE(app, "/auth/me").methods(crow::HTTPMethod::Get) (
            [](const crow::request& req) {
                try {
                    auth::Context authCtx;
                    if (auto err = auth::fromRequest(req, authCtx)) {
                        return common::errors::makeError(*err);
                    }

                    User user;
                    if (auto err = auth_service::getMe(authCtx.userId, user)) {
                        return common::errors::makeError(*err);
                    }

                    crow::json::wvalue respJson;
                    respJson["user"]["id"] = user.id;
                    respJson["user"]["firstName"] = utils::utf16_to_utf8(user.firstName);
                    respJson["user"]["lastName"] = utils::utf16_to_utf8(user.lastName);
                    respJson["user"]["email"] = utils::utf16_to_utf8(user.email);
                    respJson["user"]["role"]["id"] = user.role.id;
                    if (auto nameOpt = utils::utf16_to_utf8_opt(user.role.name)) {
                        respJson["user"]["role"]["name"] = *nameOpt;
                    }
                    else {
                        respJson["user"]["role"]["name"] = "";
                    }


                    return crow::response{ respJson };
                }
                catch (const std::exception& e) {
                    std::cerr << "Login error: " << e.what() << std::endl;
                    return common::errors::makeError(common::errors::ErrorCode::InternalError);
                }
            });
    }
}
