#pragma once

#include "cart/cart_item_model.h"
#include "models/sales_item_model.h"
#include "auth/auth_utils.h"
#include "common/errors.h"

#include <optional>

namespace cart_service {
	using common::errors::ErrorCode;

	std::optional<ErrorCode> getCartItems(const int userId, std::vector<CartItem>& cartItems);

	std::optional<ErrorCode> insertCartItem(const int userId, const CartItem& cartItem);

	std::optional<ErrorCode> deleteCartItem(const int userId, const int productId);

	std::optional<ErrorCode> addStock(const int userId, const CartItem& cartItem);

	std::optional<ErrorCode> removeStock(const int userId, const CartItem& cartItem);

	std::optional<ErrorCode> orders(const auth::Context& authCtx);

}