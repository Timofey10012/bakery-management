#pragma once

#include <crow/json.h>
#include <vector>
#include <string>

inline bool jsonNotHasKeysAndisNull(const crow::json::rvalue& j, const std::vector<std::string>& keys) {
    const crow::json::rvalue* current = &j;

    for (const auto& key : keys) {
        if (!current->count(key)) {
            return true;
        }

        const auto& next = (*current)[key];
        if (next.t() == crow::json::type::Null) {
            return true;
        }

        current = &next;
    }

    return false;
}
