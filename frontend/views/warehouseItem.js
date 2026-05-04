import { 
  getWarehouseItemById, 
  patchWarehouseAddStock, 
  patchWarehouseRemoveStock, 
  errorMessage 
} from '../api.js';
import { renderError, clearError } from '../main.js';
import { getUser } from '../state.js';

export function renderWarehouseItem(container, id, loadPage) {
  container.innerHTML = `
    <div class="warehouse-item-container">
      <button class="back-btn" id="back-btn">← Назад к складу</button>
      <div id="warehouse-item-details" class="warehouse-item-details">
        <div class="loading-spinner">Загрузка...</div>
      </div>
    </div>
  `;

  document.getElementById('back-btn').addEventListener('click', () => loadPage('warehouse'));
  loadItem(id);
}

async function loadItem(id) {
  const el = document.getElementById('warehouse-item-details');
  clearError();

  const data = await getWarehouseItemById(id).catch(() => {
    renderError(errorMessage);
    el.innerHTML = `<div class="error-state">❌ Ошибка загрузки позиции</div>`;
    return null;
  });

  if (!data) {
    el.innerHTML = `<div class="error-state">📭 Позиция не найдена</div>`;
    return;
  }

  const user = getUser();
  const canManage = user?.role?.name === 'Admin' || user?.role?.name === 'Seller';

  el.innerHTML = `
    <div class="warehouse-item-card">
      <div class="warehouse-item-header">
        <h1>📦 ${data.name}</h1>
      </div>
      
      <div class="warehouse-item-stats">
        <div class="stat-block">
          <div class="stat-value">${data.quantity}</div>
          <div class="stat-label">${data.UOM?.name || ''}</div>
        </div>
      </div>
      
      ${canManage ? `
        <div class="warehouse-item-actions">
          <h3>📊 Управление количеством</h3>
          <div class="action-group">
            <input type="number" id="stock-quantity" class="quantity-input" placeholder="Количество" min="1">
            <button id="btn-add-stock" class="btn-primary">➕ Пополнить</button>
            <button id="btn-remove-stock" class="btn-secondary">➖ Списать</button>
          </div>
        </div>
      ` : ''}
    </div>
  `;

  if (!canManage) return;

  const qtyInput = document.getElementById('stock-quantity');

  document.getElementById('btn-add-stock').onclick = async () => {
    const quantity = Number(qtyInput.value);
    if (!quantity || quantity <= 0) {
      renderError('Введите корректное количество');
      return;
    }
    clearError();
    try {
      await patchWarehouseAddStock(id, { quantity });
      renderError('✅ Склад пополнен');
      qtyInput.value = '';
      loadItem(id);
    } catch (_) {
      renderError(errorMessage);
    }
  };

  document.getElementById('btn-remove-stock').onclick = async () => {
    const quantity = Number(qtyInput.value);
    if (!quantity || quantity <= 0) {
      renderError('Введите корректное количество');
      return;
    }
    clearError();
    try {
      await patchWarehouseRemoveStock(id, { quantity });
      renderError('✅ Товар списан');
      qtyInput.value = '';
      loadItem(id);
    } catch (_) {
      renderError(errorMessage);
    }
  };
}