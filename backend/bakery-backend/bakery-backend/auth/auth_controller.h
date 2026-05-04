#pragma once

#include "auth/register_body_model.h"
#include "auth/auth_response_model.h"
#include "auth/login_body_model.h"
#include "auth/auth_service.h"

#include <crow.h>
#include <crow/middlewares/cors.h>

namespace auth_controller {

    void setupRoutes(crow::App<crow::CORSHandler>& app);

}
