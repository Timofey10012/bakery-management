import { getDashboardSummary, getDashboardActivity, errorMessage } from '../api.js';
import { renderError, clearError } from '../main.js';

export function renderDashboard(container) {
  container.innerHTML = `
    <div class="dashboard-container">
      <h1>📊 Панель управления</h1>
      
      <div class="stats-grid" id="stats-grid">
        <div class="stat-card">
          <div class="stat-icon">💰</div>
          <div class="stat-info">
            <span class="stat-value" id="sales-today">—</span>
            <span class="stat-label">Продажи сегодня</span>
          </div>
        </div>
        
        <div class="stat-card">
          <div class="stat-icon">🥐</div>
          <div class="stat-info">
            <span class="stat-value" id="products-count">—</span>
            <span class="stat-label">Товаров</span>
          </div>
        </div>
        
        <div class="stat-card">
          <div class="stat-icon">👥</div>
          <div class="stat-info">
            <span class="stat-value" id="users-count">—</span>
            <span class="stat-label">Пользователей</span>
          </div>
        </div>
        
        <div class="stat-card">
          <div class="stat-icon">📦</div>
          <div class="stat-info">
            <span class="stat-value" id="orders-today">—</span>
            <span class="stat-label">Заказов сегодня</span>
          </div>
        </div>
      </div>
      
      <div class="activity-section">
        <h2>📋 Последняя активность</h2>
        <div id="activity-list" class="activity-list">
          <div class="activity-placeholder">Загрузка...</div>
        </div>
      </div>
    </div>
  `;

  loadSummary();
  loadActivity();
}

async function loadSummary() {
  clearError();
  
  const salesEl = document.getElementById('sales-today');
  const productsEl = document.getElementById('products-count');
  const usersEl = document.getElementById('users-count');
  const ordersEl = document.getElementById('orders-today');

  const data = await getDashboardSummary().catch(() => {
    renderError(errorMessage);
    return null;
  });

  if (!data) {
    salesEl.textContent = '0';
    productsEl.textContent = '0';
    usersEl.textContent = '0';
    ordersEl.textContent = '0';
    return;
  }

  // Анимация появления чисел
  animateNumber(salesEl, 0, data.salesToday || 0);
  animateNumber(productsEl, 0, data.productsCount || 0);
  animateNumber(usersEl, 0, data.usersCount || 0);
  animateNumber(ordersEl, 0, data.ordersToday || 0);
}

function animateNumber(element, start, end) {
  if (!element) return;
  const duration = 500;
  const step = Math.ceil((end - start) / 20);
  let current = start;
  
  element.textContent = start;
  
  if (start === end) return;
  
  const timer = setInterval(() => {
    current += step;
    if (current >= end) {
      element.textContent = end;
      clearInterval(timer);
    } else {
      element.textContent = current;
    }
  }, duration / 20);
}

async function loadActivity() {
  const container = document.getElementById('activity-list');
  if (!container) return;

  const data = await getDashboardActivity().catch(() => {
    renderError(errorMessage);
    container.innerHTML = `<div class="activity-placeholder error">❌ Ошибка загрузки активности</div>`;
    return null;
  });

  if (!data || !data.dashboardActivity || data.dashboardActivity.length === 0) {
    container.innerHTML = `<div class="activity-placeholder">📭 Нет активности</div>`;
    return;
  }

  const items = data.dashboardActivity.map(renderActivityItem).join('');
  container.innerHTML = items;
}

function renderActivityItem(item) {
  if (!item) return '';

  const date = formatDate(item.timestamp);
  
  switch (item.type) {
    case 'sale':
      return `
        <div class="activity-item sale">
          <div class="activity-icon">💰</div>
          <div class="activity-content">
            <div class="activity-title">Продажа</div>
            <div class="activity-details">Сумма: ${item.data?.total ?? 'нет данных'} ₽</div>
            <div class="activity-time">${date}</div>
          </div>
        </div>
      `;

    case 'userCreated':
      const roleName = item.data?.role?.name === 'Admin' ? 'Администратор' :
                       item.data?.role?.name === 'Seller' ? 'Продавец' : 'Пользователь';
      return `
        <div class="activity-item user">
          <div class="activity-icon">👤</div>
          <div class="activity-content">
            <div class="activity-title">Новый пользователь</div>
            <div class="activity-details">Роль: ${roleName}</div>
            <div class="activity-time">${date}</div>
          </div>
        </div>
      `;

    case 'stockAdded':
      return `
        <div class="activity-item stock">
          <div class="activity-icon">📦</div>
          <div class="activity-content">
            <div class="activity-title">Пополнение склада</div>
            <div class="activity-details">
              ${item.data?.warehouseItem?.name ?? 'Товар'}: 
              ${item.data?.amount ?? '0'} ${item.data?.UOM?.name ?? ''}
            </div>
            <div class="activity-time">${date}</div>
          </div>
        </div>
      `;

    default:
      return `
        <div class="activity-item unknown">
          <div class="activity-icon">❓</div>
          <div class="activity-content">
            <div class="activity-title">Неизвестная активность</div>
            <div class="activity-time">${date}</div>
          </div>
        </div>
      `;
  }
}

function formatDate(date) {
  if (!date) return '';
  const d = new Date(date);
  return d.toLocaleString('ru-RU', {
    day: '2-digit',
    month: '2-digit',
    hour: '2-digit',
    minute: '2-digit'
  });
}