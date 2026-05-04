#include "cart/cart_service.h"
#include "db/cart/cart_queries.h"

namespace cart_service {

	std::optional<ErrorCode> getCartItems(const int userId, std::vector<CartItem>& cartItems) {
		if (!db::userExistsById(db::connection(), userId)) {
			return ErrorCode::UserNotFound;
		}

		if (!db::getCartItems(db::connection(), userId, cartItems)) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}

	std::optional<ErrorCode> insertCartItem(const int userId, const CartItem& cartItem) {
		if (!db::userExistsById(db::connection(), userId)) {
			return ErrorCode::UserNotFound;
		}

		if (cartItem.quantityItems < 0) {
			return ErrorCode::ValidationError;
		}

		if (!db::productExistsById(db::connection(), cartItem.productItem.id)) {
			return ErrorCode::ProductNotFound;
		}

		if (db::productQuantityIsMoreLimit(db::connection(), cartItem)) {
			return ErrorCode::ProductInsufficientStock;
		}
		
		if (db::cartItemExistsByProductId(db::connection(), userId, cartItem.productItem.id)) {
			if (!db::cartItemAddStock(db::connection(), userId, cartItem)) {
				return ErrorCode::InternalError;
			}
		}
		else {
			if (!db::insertCartItem(db::connection(), userId, cartItem)) {
				return ErrorCode::InternalError;
			}
		}

		return std::nullopt;
	}

	std::optional<ErrorCode> deleteCartItem(const int userId, const int productId) {
		if (!db::userExistsById(db::connection(), userId)) {
			return ErrorCode::UserNotFound;
		}

		if (!db::productExistsById(db::connection(), productId)) {
			return ErrorCode::ProductNotFound;
		}

		if (!db::cartItemExistsByProductId(db::connection(), userId, productId)) {
			return ErrorCode::ValidationError;
		}

		if (!db::deleteCartItem(db::connection(), userId, productId)) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}

	std::optional<ErrorCode> addStock(const int userId, const CartItem& cartItem) {
		if (!db::userExistsById(db::connection(), userId)) {
			return ErrorCode::UserNotFound;
		}

		if (cartItem.quantityItems < 0) {
			return ErrorCode::ValidationError;
		}
		
		if (!db::productExistsById(db::connection(), cartItem.productItem.id)) {
			return ErrorCode::ProductNotFound;
		}

		if (!db::cartItemExistsByProductId(db::connection(), userId, cartItem.productItem.id)) {
			return ErrorCode::ValidationError;
		}

		if (db::productQuantityIsMoreLimit(db::connection(), cartItem)) {
			return ErrorCode::ProductInsufficientStock;
		}

		if (!db::cartItemAddStock(db::connection(), userId, cartItem)) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}

	std::optional<ErrorCode> removeStock(const int userId, const CartItem& cartItem) {
		if (!db::userExistsById(db::connection(), userId)) {
			return ErrorCode::UserNotFound;
		}

		if (cartItem.quantityItems < 0) {
			return ErrorCode::ValidationError;
		}

		if (!db::productExistsById(db::connection(), cartItem.productItem.id)) {
			return ErrorCode::ProductNotFound;
		}

		if (!db::cartItemExistsByProductId(db::connection(), userId, cartItem.productItem.id)) {
			return ErrorCode::ValidationError;
		}

		if (db::cartItemQuantityIsMoreLimit(db::connection(), userId, cartItem)) {
			return ErrorCode::CartItemInsufficientStock;
		}

		if (!db::cartItemRemoveStock(db::connection(), userId, cartItem)) {
			return ErrorCode::InternalError;
		}

		return std::nullopt;
	}

	std::optional<ErrorCode> orders(const auth::Context& authCtx) {
		if (!db::userExistsById(db::connection(), authCtx.userId)) {
			return ErrorCode::UserNotFound;
		}

		std::vector<SalesItem> salesItems;
		
		if (!db::getSalesItemsFromCart(db::connection(), authCtx.userId, salesItems)) {
			return ErrorCode::InternalError;
		}
		
		if (salesItems.empty()) return ErrorCode::CartItemInsufficientStock;

		for (auto salesItem : salesItems) {
			if (db::salesItemQuantityIsMoreLimit(db::connection(), salesItem)) {
				return ErrorCode::SaleItemInsufficientStock;
			}
		}

		if (!db::beginTransaction(db::connection())) {
			return ErrorCode::InternalError;
		}
		bool success = true;
		
		int salesId;
		if (authCtx.role == auth::Role::User) {
			if (!db::insertSales(db::connection(), authCtx.userId, salesId)) {
				success = false;
			}
		}
		else {
			if (!db::insertSales(db::connection(), salesId)) {
				success = false;
			}
		}
		
		for (auto salesItem : salesItems) {
			if (!db::insertSalesItem(db::connection(), salesId, salesItem)) {
				success = false;
			}
			
			if (!db::deleteCartItem(db::connection(), authCtx.userId, salesItem.productItem.id)) {
				success = false;
			}

			if (!db::productRemoveStock(db::connection(), salesItem.productItem.id, salesItem.quantityItems)) {
				success = false;
			}
		}

		if (!db::endTransaction(db::connection(), success)) {
			return ErrorCode::InternalError;
		}

		if (!success) return ErrorCode::InternalError;

		return std::nullopt;
	}

}