#include "server/server.h"
#include "auth/auth_controller.h"
#include "dashboard/dashboard_controller.h"
#include "products/products_controller.h"
#include "warehouse/warehouse_controller.h"
#include "users/users_controller.h"
#include "sales/sales_controller.h"
#include "cart/cart_controller.h"
#include "dictionaries/dictionaries_controller.h"

#include <crow.h>
#include <crow/middlewares/cors.h>
#include <iostream>

void run_server() {
	crow::App<crow::CORSHandler> app;

	auto& cors = app.get_middleware<crow::CORSHandler>();
	cors.global()
		.headers("X-Custom-Header", "Upgrade-Insecure-Requests", "Content-Type", "Authorization")
		.methods("POST"_method, "GET"_method, "PATCH"_method, "DELETE"_method, "OPTIONS"_method)
		.origin("*");

	auth_controller::setupRoutes(app);
	dashboard_controller::setupRoutes(app);
	products_controller::setupRoutes(app);
	warehouse_controller::setupRoutes(app);
	users_controller::setupRoutes(app);
	sales_controller::setupRoutes(app);
	cart_controller::setupRoutes(app);
	dictionaries_controller::setupRoutes(app);

	std::cout << "Server started at http://localhost:8080\n";

	app.port(8080).multithreaded().run();
}