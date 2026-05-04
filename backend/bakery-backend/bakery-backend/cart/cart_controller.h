#pragma once

#include <crow.h>
#include <crow/middlewares/cors.h>

namespace cart_controller {

	void setupRoutes(crow::App<crow::CORSHandler>& app);

}