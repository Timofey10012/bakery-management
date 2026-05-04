#include "db/db.h"

#define WIN32_LEAN_AND_MEAN
#define NOMIMAX

#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <iostream>

namespace db
{
	namespace
	{
		SQLHENV g_env = SQL_NULL_HENV;
		SQLHDBC g_dbc = SQL_NULL_HDBC;
		bool g_initialized = false;

		void logError(SQLSMALLINT handleType, SQLHANDLE handle) {
			SQLCHAR sqlState[6];
			SQLCHAR message[256];
			SQLINTEGER nativeError;
			SQLSMALLINT textLength;

			if (SQLGetDiagRecA(
				handleType,
				handle,
				1,
				sqlState,
				&nativeError,
				message,
				sizeof(message),
				&textLength
			) == SQL_SUCCESS) {

				std::cerr
					<< "[ODBC ERROR] "
					<< sqlState << " | "
					<< message
					<< std::endl;
			}
		}
	}

	bool db::init(const std::string& connectionString) {
		if (g_initialized) {
			std::cerr << "[DB] Already initialized\n";
			return false;
		}

		if (connectionString.empty()) {
			std::cerr << "[DB] Connection string is empty\n";
			return false;
		}

		if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &g_env) != SQL_SUCCESS) {
			std::cerr << "Failed to allocate ODBC environment\n";
			return false;
		}

		SQLSetEnvAttr(
			g_env,
			SQL_ATTR_ODBC_VERSION,
			(void*)SQL_OV_ODBC3,
			0
		);

		if (SQLAllocHandle(SQL_HANDLE_DBC, g_env, &g_dbc) != SQL_SUCCESS) {
			std::cerr << "Failed to allocate ODBC connection\n";
			return false;
		}

		SQLCHAR outConnStr[1024];
		SQLSMALLINT outConnStrLen;

		SQLRETURN ret = SQLDriverConnectA(
			g_dbc,
			NULL,
			(SQLCHAR*)connectionString.c_str(),
			SQL_NTS,
			outConnStr,
			sizeof(outConnStr),
			&outConnStrLen,
			SQL_DRIVER_NOPROMPT
		);

		if (!SQL_SUCCEEDED(ret)) {
			std::cerr << "Failed to connect to database\n";
			logError(SQL_HANDLE_DBC, g_dbc);
			return false;
		}


		g_initialized = true;
		return true;
	}

	void db::shutdown() {
		if (!g_initialized) {
			return;
		}

		SQLDisconnect(g_dbc);
		SQLFreeHandle(SQL_HANDLE_DBC, g_dbc);
		SQLFreeHandle(SQL_HANDLE_ENV, g_env);

		g_dbc = SQL_NULL_HDBC;
		g_env = SQL_NULL_HENV;
		g_initialized = false;

		std::cout << "[DB] Shutdown\n";
		g_initialized = false;
	}

	SQLHDBC connection() {
		return g_dbc;
	}

	bool beginTransaction(SQLHDBC dbc) {
		return SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0) == SQL_SUCCESS;
	}

	bool endTransaction(SQLHDBC dbc, bool success) {
		SQLRETURN ret = success ? SQLEndTran(SQL_HANDLE_DBC, dbc, SQL_COMMIT)
			: SQLEndTran(SQL_HANDLE_DBC, dbc, SQL_ROLLBACK);

		SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0);

		return SQL_SUCCEEDED(ret);
	}
}