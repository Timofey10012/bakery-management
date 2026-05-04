#pragma once

#include <crow.h>
#include <crow/middlewares/cors.h>

namespace users_controller {

	void setupRoutes(crow::App<crow::CORSHandler>& app);

}