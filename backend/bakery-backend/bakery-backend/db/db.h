#pragma once

#include <string>

typedef void* SQLHDBC;

namespace db
{
	bool init(const std::string& connectionString);

	void shutdown();

	SQLHDBC connection();

	bool beginTransaction(SQLHDBC dbc);

	bool endTransaction(SQLHDBC dbc, bool success);
}