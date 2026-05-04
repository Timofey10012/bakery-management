#pragma once

#include "models/product_item_model.h"

struct SalesItem
{
	ProductItem productItem;
	int quantityItems;
	int pricePerUnit;
	int totalPricePerProduct;
};