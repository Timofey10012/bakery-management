#define _CRT_SECURE_NO_WARNINGS

#include "server/server.h"
#include "db/db.h"

#include <Windows.h>
#include <clocale>
#include <iostream>
#include <cstdlib>
#include <openssl/crypto.h>

int main()
{
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    setlocale(LC_ALL, "Russian");
    const char* env = std::getenv("DB_CONNECTION_STRING");
    if (!env) {
        std::cerr << "[FATAL] DB_CONNECTION_STRING is not set\n";
        return 1;
    }

    std::string connectionString(env);
    if (!db::init(connectionString)) {
        std::cerr << "[FATAL] Database initialization failed\n";
        return 1;
    }

    OPENSSL_init_crypto(0, nullptr);

    run_server();
    
    db::shutdown();
    return 0;
}
