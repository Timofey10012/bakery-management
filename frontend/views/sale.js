import { getSaleById, errorMessage } from '../api.js';
import { renderError, clearError } from '../main.js';

export function renderSale(container, id, loadPage) {
  container.innerHTML = `
    <div class="sale-detail-container">
      <button class="back-btn" id="back-btn">← Назад к списку</button>
      <div id="sale-details" class="sale-details">
        <div class="loading-spinner">Загрузка...</div>
      </div>
    </div>
  `;

  document.getElementById('back-btn').addEventListener('click', () => loadPage('sales'));
  loadSale(id);
}

async function loadSale(id) {
  const el = document.getElementById('sale-details');
  clearError();

  const data = await getSaleById(id).catch(() => {
    renderError(errorMessage);
    el.innerHTML = `<div class="error-state">❌ Ошибка загрузки продажи</div>`;
    return null;
  });

  if (!data) {
    el.innerHTML = `<div class="error-state">📭 Продажа не найдена</div>`;
    return;
  }

  const customerName = data.user 
    ? `${data.user.firstName || ''} ${data.user.lastName || ''}`.trim() 
    : 'Оффлайн-покупатель';
  
  const date = formatDate(data.createdAt);
  
  const itemsList = (data.salesItem || []).map(item => `
    <div class="sale-item">
      <div class="sale-item-name">${item.productItem?.name || 'неизвестный продукт'}</div>
      <div class="sale-item-details">
        <span>${item.quantityItems} шт</span>
        <span>× ${item.pricePerUnit} ₽</span>
        <span class="sale-item-total">= ${item.totalPricePerProduct} ₽</span>
      </div>
    </div>
  `).join('');

  el.innerHTML = `
    <div class="sale-detail-card">
      <div class="sale-detail-header">
        <h1>Продажа</h1>
        <div class="sale-detail-date">${date}</div>
      </div>
      
      <div class="sale-detail-customer">
        <div class="customer-info">
          <span class="customer-icon">👤</span>
          <div>
            <div class="customer-name">${customerName}</div>
            <div class="customer-email">${data.user?.email || 'Без email'}</div>
          </div>
        </div>
        <div class="sale-total">
          <span class="total-label">Итого:</span>
          <span class="total-value">${data.total} ₽</span>
        </div>
      </div>
      
      <div class="sale-items-section">
        <h3>📋 Состав заказа</h3>
        <div class="sale-items-list">
          ${itemsList || '<div class="empty-items">Нет товаров в продаже</div>'}
        </div>
      </div>
    </div>
  `;
}

function formatDate(date) {
  if (!date) return '—';
  const d = new Date(date);
  return d.toLocaleString('ru-RU', {
    day: '2-digit',
    month: '2-digit',
    year: 'numeric',
    hour: '2-digit',
    minute: '2-digit'
  });
}