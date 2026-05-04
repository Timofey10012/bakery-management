const errorMap = {
  USER_NOT_FOUND: 'Пользователь не найден',
  ROLE_NOT_FOUND: 'Роль не найдена',
  UOM_NOT_FOUND: 'Единица измерения не найдена',

  AUTH_LOGIN_EMAIL_NOT_FOUND: 'Пользователь с указаным email не найден',
  AUTH_LOGIN_INVALID_PASSWORD: 'Неверный пароль',
  AUTH_REGISTER_EMAIL_ALREADY_EXISTS: 'Пользователь с таким email уже зарегистрирован',
  AUTH_REGISTER_PASSWORD_NOT_VALID: 'Пароль не соответствует требованиям безопасности',

  PRODUCT_NOT_FOUND: 'Карточка товара не найдена',
  PRODUCT_ALREADY_EXISTS: 'Карточка товара с таким названием уже существует',
  PRODUCT_INSUFFICIENT_STOCK: 'Количество товара на складе недостаточно для выполнения операции',

  WAREHOUSE_ITEM_NOT_FOUND: 'Позиция склада не найдена',
  WAREHOUSE_ITEM_ALREADY_EXISTS: 'Позиция склада уже существует',
  WAREHOUSE_ITEM_USED_IN_PRODUCT: 'Позиция склада используется в карточке товара',
  WAREHOUSE_ITEM_INSUFFICIENT_AMOUNT: 'Недостаточное количество позиции на складе',

  SALE_NOT_FOUND: 'Продажа не найдена',
  SALE_ITEM_INSUFFICIENT_STOCK: 'Количество продаваемого товара превышает доступное на складе',

  ORDERS_INSUFFICIENT_STOCK_AT_CHECKOUT: 'Недостаточное количество товара на складе на момент оформления покупки',

  CART_ITEM_INSUFFICIENT_STOCK: 'Недостаточное количество товара в корзине',

  MANUFACTURER_NOT_FOUND: 'Производитель не найден',
  MANUFACTURER_ALREADY_EXISTS: 'Производитель с таким названием уже существует'
};

export function mapError(data, status) {
  const code = data?.error?.code;

  if (code && errorMap[code]) {
    return errorMap[code];
  }

  switch (status) {
    case 400:
      return 'Неверный запрос';
    case 401:
      return 'Не авторизован';
    case 403:
      return 'Нет доступа';
    case 404:
      return 'Не найдено';
    case 409:
      return 'Конфликт данных';
    case 500:
      return 'Ошибка сервера';
    default:
      return 'Неизвестная ошибка';
  }
}