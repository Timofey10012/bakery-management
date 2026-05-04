#pragma once

#include <crow.h>
#include <crow/middlewares/cors.h>

namespace products_controller {

	void setupRoutes(crow::App<crow::CORSHandler>& app);

}