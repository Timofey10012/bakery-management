import { getWarehouse, addWarehouseItem, deleteWarehouseItem, errorMessage } from '../api.js';
import { renderError, clearError } from '../main.js';
import { state, loadAllDictionaries } from './dictionaries.js';
import { getUser } from '../state.js';
import { showConfirm } from '../dialog.js';

let currentLoadPage = null;

export function renderWarehouse(container, loadPage) {
  currentLoadPage = loadPage;
  
  container.innerHTML = `
    <div class="warehouse-header">
      <h1>📦 Склад</h1>
      <button id="btn-add-warehouse-item" class="btn-primary">+ Добавить позицию</button>
    </div>
    <div id="warehouse-list" class="warehouse-grid">
      <div class="loading-placeholder">Загрузка...</div>
    </div>
  `;

  loadWarehouse();
  document.getElementById('btn-add-warehouse-item').onclick = () => renderAddWarehouseForm();
}

async function loadWarehouse() {
  const el = document.getElementById('warehouse-list');
  clearError();

  const data = await getWarehouse().catch(() => {
    renderError(errorMessage);
    el.innerHTML = `<div class="error-placeholder">❌ Ошибка загрузки склада</div>`;
    return null;
  });

  const items = data?.warehouseItemFull || [];

  if (!items.length) {
    el.innerHTML = `<div class="empty-placeholder">📭 Склад пуст. Добавьте первую позицию!</div>`;
    return;
  }

  el.innerHTML = items.map(item => renderWarehouseItemRow(item)).join('');
  
  // Обработчики для карточек
  document.querySelectorAll('.warehouse-card').forEach(card => {
    card.addEventListener('click', (e) => {
      if (e.target.closest('button')) return;
      const id = card.dataset.id;
      if (id && currentLoadPage) currentLoadPage('warehouse-item', id);
    });
  });
}

function renderWarehouseItemRow(item) {
  const user = getUser();
  const canDelete = user?.role?.name === 'Admin';
  const isLowStock = item.quantity < item.minQuantity;
  
  return `
    <div class="warehouse-card" data-id="${item.id}">
      <div class="warehouse-card-header">
        <h3 class="warehouse-card-title">${item.name}</h3>
        ${isLowStock ? '<span class="stock-badge low">Мало</span>' : ''}
      </div>
      <div class="warehouse-card-stock">
        <span class="stock-amount">${item.quantity}</span>
        <span class="stock-unit">${item.UOM?.name || ''}</span>
      </div>
      <div class="warehouse-card-details">
        <div class="detail-item">
          <span class="detail-label">Минимум:</span>
          <span>${item.minQuantity} ${item.UOM?.name || ''}</span>
        </div>
        <div class="detail-item">
          <span class="detail-label">Обновлён:</span>
          <span>${formatDate(item.supplyDate)}</span>
        </div>
        <div class="detail-item">
          <span class="detail-label">Статус:</span>
          <span class="${item.isInStock ? 'status-in-stock' : 'status-out-stock'}">
            ${item.isInStock ? '✅ В наличии' : '❌ Нет'}
          </span>
        </div>
      </div>
      <div class="warehouse-card-actions">
        <button class="btn-outline" data-action="open" data-id="${item.id}">📋 Изменить количество</button>
        ${canDelete ? `<button class="btn-delete" data-action="delete" data-id="${item.id}">🗑️ Удалить</button>` : ''}
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
    hour: '2-digit',
    minute: '2-digit'
  });
}

// Делегирование для кнопок в карточках склада
document.addEventListener('click', async (e) => {
  const btn = e.target.closest('button');
  if (!btn) return;
  
  const action = btn.dataset.action;
  const id = btn.dataset.id;
  
  if (action === 'open' && id && currentLoadPage) {
    e.stopPropagation();
    currentLoadPage('warehouse-item', id);
  }
  
  if (action === 'delete' && id) {
    e.stopPropagation();
    if (!await showConfirm('Вы уверены, что хотите удалить эту позицию?')) return;
    clearError();
    try {
      await deleteWarehouseItem(id);
      renderError('✅ Позиция удалена');
      setTimeout(() => clearError(), 2000);
      loadWarehouse();
    } catch (_) {
      renderError(errorMessage);
    }
  }
});

// Функция закрытия всех модалок
function closeAllModals() {
  document.querySelectorAll('.modal-overlay').forEach(modal => modal.remove());
}

// Форма добавления позиции на склад
async function renderAddWarehouseForm() {
  await loadAllDictionaries();
  
  // Закрываем все существующие модалки
  closeAllModals();
  
  // Создаём модальное окно прямо в body
  const modal = document.createElement('div');
  modal.className = 'modal-overlay';
  modal.id = 'warehouse-modal';
  
  modal.innerHTML = `
    <div class="modal-content">
      <div class="modal-header">
        <h2>➕ Добавить позицию на склад</h2>
        <button class="modal-close">✕</button>
      </div>
      <form id="add-warehouse-form" class="modal-form">
        <div class="form-group">
          <label>Название *</label>
          <input type="text" name="name" required placeholder="Например: Мука пшеничная">
        </div>
        
        <div class="form-row">
          <div class="form-group">
            <label>Количество *</label>
            <input type="number" name="quantity" required placeholder="0" min="0" step="1">
          </div>
          <div class="form-group">
            <label>Минимальное количество *</label>
            <input type="number" name="minQuantity" required placeholder="0" min="0" step="1">
          </div>
        </div>
        
        <div class="form-group">
          <label>Единица измерения *</label>
          <select name="UOM" required>
            ${state.uoms.map(u => `<option value="${u.id}">${u.name}</option>`).join('')}
          </select>
        </div>
        
        <div class="form-actions">
          <button type="submit" class="btn-primary">Добавить</button>
          <button type="button" class="btn-secondary cancel-btn">Отмена</button>
        </div>
      </form>
    </div>
  `;
  
  document.body.appendChild(modal);

  const form = modal.querySelector('#add-warehouse-form');
  const closeBtn = modal.querySelector('.modal-close');
  const cancelBtn = modal.querySelector('.cancel-btn');

  const closeModal = () => modal.remove();
  closeBtn.addEventListener('click', closeModal);
  cancelBtn.addEventListener('click', closeModal);
  modal.addEventListener('click', (e) => {
    if (e.target === modal) closeModal();
  });

  // Закрытие по Escape
  const escHandler = (e) => {
    if (e.key === 'Escape') {
      closeModal();
      document.removeEventListener('keydown', escHandler);
    }
  };
  document.addEventListener('keydown', escHandler);

  form.onsubmit = async (e) => {
    e.preventDefault();
    const formData = new FormData(form);
    const body = {
      name: formData.get('name'),
      quantity: Number(formData.get('quantity')),
      minQuantity: Number(formData.get('minQuantity')),
      UOM: { id: Number(formData.get('UOM')) }
    };

    clearError();
    try {
      await addWarehouseItem(body);
      closeModal();
      document.removeEventListener('keydown', escHandler);
      loadWarehouse();
      renderError('✅ Позиция добавлена на склад');
      setTimeout(() => clearError(), 2000);
    } catch (_) {
      renderError(errorMessage);
    }
  };
}