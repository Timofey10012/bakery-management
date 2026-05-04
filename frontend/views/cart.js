import { getCart, deleteCartItem, patchCartAddStock, patchCartRemoveStock, createOrder, errorMessage } from '../api.js';
import { renderError, clearError } from '../main.js';
import { showConfirm } from '../dialog.js';

export function renderCart(container) {
  container.innerHTML = `
    <div class="cart-container">
      <h1>🛒 Корзина</h1>
      <div id="cart-items" class="cart-items-list">
        <div class="loading-placeholder">Загрузка...</div>
      </div>
      <div id="cart-total" class="cart-summary"></div>
      <button id="btn-purchase" class="btn-purchase">✅ Оформить заказ</button>
    </div>
  `;

  loadCart();

  async function loadCart() {
    const el = document.getElementById('cart-items');
    const totalEl = document.getElementById('cart-total');
    clearError();

    const data = await getCart().catch(() => {
      renderError(errorMessage);
      el.innerHTML = `<div class="error-placeholder">❌ Ошибка загрузки корзины</div>`;
      return null;
    });

    const cartItems = Array.isArray(data) ? data : (data?.cartItem || []);

    if (!cartItems.length) {
      el.innerHTML = `<div class="empty-placeholder">🛒 Корзина пуста</div>`;
      totalEl.innerHTML = '';
      return;
    }

    el.innerHTML = cartItems.map((item) => {
      const product = item.productItem;
      const quantity = item.quantityItems || 0;
      
      return `
        <div class="cart-item" data-id="${product?.id}">
          <div class="cart-item-info">
            <div class="cart-item-name">${product?.name || 'Товар'}</div>
            <div class="cart-item-quantity">📦 Количество: ${quantity} шт</div>
          </div>
          <div class="cart-item-controls">
            <div class="qty-control">
              <input type="number" class="quantity-input" id="qty-${product?.id}" min="1" value="1" style="width: 60px;">
              <button class="btn-increase" data-id="${product?.id}">+</button>
              <button class="btn-decrease" data-id="${product?.id}">−</button>
            </div>
            <button class="btn-delete" data-id="${product?.id}">🗑️</button>
          </div>
        </div>
      `;
    }).join('');

    totalEl.innerHTML = `
      <div class="summary-card">
        <div class="summary-row">
          <span>Всего позиций:</span>
          <span class="summary-total">${cartItems.length} шт</span>
        </div>
        <div class="summary-hint">Для оформления заказа нажмите кнопку ниже</div>
      </div>
    `;

    // Привязываем события
    document.querySelectorAll('.btn-increase').forEach(btn => {
      btn.onclick = async () => {
        const productId = parseInt(btn.dataset.id);
        const qtyInput = document.getElementById(`qty-${productId}`);
        const quantity = parseInt(qtyInput?.value || '1');
        
        if (quantity <= 0) {
          renderError('Введите корректное количество');
          return;
        }
        
        clearError();
        try {
          await patchCartAddStock(productId, quantity);
          await loadCart();
        } catch (err) {
          renderError(errorMessage);
        }
      };
    });

    document.querySelectorAll('.btn-decrease').forEach(btn => {
      btn.onclick = async () => {
        const productId = parseInt(btn.dataset.id);
        const qtyInput = document.getElementById(`qty-${productId}`);
        const quantity = parseInt(qtyInput?.value || '1');
        
        if (quantity <= 0) {
          renderError('Введите корректное количество');
          return;
        }
        
        clearError();
        try {
          await patchCartRemoveStock(productId, quantity);
          await loadCart();
        } catch (err) {
          renderError(errorMessage);
        }
      };
    });

    document.querySelectorAll('.btn-delete').forEach(btn => {
      btn.onclick = async () => {
        const productId = parseInt(btn.dataset.id);
        if (!await showConfirm('Удалить товар из корзины?')) return;
        
        clearError();
        try {
          await deleteCartItem(productId);
          await loadCart();
        } catch (err) {
          renderError(errorMessage);
        }
      };
    });
  }

  document.getElementById('btn-purchase').onclick = async () => {
    if (!await showConfirm('Оформить заказ?')) return;
    clearError();
    try {
      await createOrder();
      renderError('✅ Заказ оформлен! Спасибо за покупку!');
      setTimeout(() => clearError(), 3000);
      await loadCart();
    } catch (_) {
      renderError(errorMessage);
    }
  };
}