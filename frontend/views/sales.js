import { getSales, errorMessage } from '../api.js';
import { renderError, clearError } from '../main.js';

let globalLoadPage = null;

export function renderSales(container, loadPage) {
  globalLoadPage = loadPage;
  
  container.innerHTML = `
    <div class="sales-header">
      <h1>💰 Продажи</h1>
    </div>
    <div id="sales-list" class="sales-grid">
      <div class="loading-placeholder">Загрузка...</div>
    </div>
  `;

  loadSales();

  async function loadSales() {
    clearError();
    const el = document.getElementById('sales-list');

    const data = await getSales().catch(() => {
      renderError(errorMessage);
      el.innerHTML = `<div class="error-placeholder">❌ Ошибка загрузки продаж</div>`;
      return null;
    });

    const sales = data?.saleShort || [];

    if (!sales.length) {
      el.innerHTML = `<div class="empty-placeholder">📭 Нет продаж</div>`;
      return;
    }

    el.innerHTML = sales.map(sale => renderSaleCard(sale)).join('');
    
    // Добавляем обработчики на карточки
    document.querySelectorAll('.sale-card').forEach(card => {
      card.addEventListener('click', (e) => {
        if (e.target.closest('button')) return;
        const id = card.dataset.id;
        if (id && globalLoadPage) {
          globalLoadPage('sale', id);
        }
      });
    });
  }
}

function renderSaleCard(sale) {
  const date = formatDate(sale.createdAt);
  const customerName = sale.user 
    ? `${sale.user.firstName || ''} ${sale.user.lastName || ''}`.trim() 
    : 'Оффлайн-покупатель';
  
  return `
    <div class="sale-card" data-id="${sale.id}">
      <div class="sale-card-header">
        <span class="sale-id">#${sale.id}</span>
        <span class="sale-date">${date}</span>
      </div>
      <div class="sale-card-body">
        <div class="sale-customer">
          <span class="sale-customer-icon">👤</span>
          <span>${customerName}</span>
        </div>
        <div class="sale-amount">
          <span class="sale-amount-value">${sale.total} ₽</span>
        </div>
      </div>
      <div class="sale-card-footer">
        <button class="btn-outline" data-id="${sale.id}">📋 Детали</button>
      </div>
    </div>
  `;
}

// Обработчик для кнопки "Детали" через делегирование
document.addEventListener('click', (e) => {
  const btn = e.target.closest('.btn-outline');
  if (btn && btn.closest('.sale-card')) {
    e.stopPropagation();
    const card = btn.closest('.sale-card');
    const id = card.dataset.id;
    if (id && globalLoadPage) {
      globalLoadPage('sale', id);
    }
  }
});

function formatDate(date) {
  if (!date) return '—';
  const d = new Date(date);
  return d.toLocaleString('ru-RU', {
    day: '2-digit',
    month: '2-digit',
    hour: '2-digit',
    minute: '2-digit'
  });
}