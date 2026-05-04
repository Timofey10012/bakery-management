import { BASE_URL, getProducts, addProduct, deleteProduct as apiDeleteProduct, addToCart, errorMessage } from '../api.js';
import { state, loadAllDictionaries, createManufacturer, removeManufacturer } from './dictionaries.js';
import { renderError, clearError } from '../main.js';
import { getUser } from '../state.js';
import { showConfirm } from '../dialog.js';

export function renderProducts(container) {
  const user = getUser();
  const role = user?.role?.name;
  const canAddProduct = role === 'Admin' || role === 'Seller';
  
  container.innerHTML = `
    <div class="products-header">
      <h1>🥐 Товары</h1>
      ${canAddProduct ? '<button id="btn-add-product" class="btn-primary">+ Добавить товар</button>' : ''}
    </div>
    <div id="products-list" class="products-grid">
      <div class="loading-placeholder">Загрузка товаров...</div>
    </div>
  `;

  loadProducts();
  
  if (canAddProduct) {
    document.getElementById('btn-add-product').onclick = () => renderAddProductForm();
  }
}

async function loadProducts() {
  clearError();
  const el = document.getElementById('products-list');

  const data = await getProducts().catch(() => {
    renderError(errorMessage);
    el.innerHTML = `<div class="error-placeholder">❌ Ошибка загрузки товаров</div>`;
    return null;
  });

  if (!data || !Array.isArray(data.product) || data.product.length === 0) {
    el.innerHTML = `<div class="empty-placeholder">📭 Нет товаров. Добавьте первый!</div>`;
    return;
  }

  el.innerHTML = data.product.map(product => renderProductItem(product)).join('');
  
  // Обработчики для карточек
  document.querySelectorAll('.product-card').forEach(card => {
    card.addEventListener('click', (e) => {
      if (e.target.closest('button') || e.target.closest('input')) return;
      const id = card.dataset.id;
      if (id && window.viewProduct) window.viewProduct(id);
    });
  });
  
  // Обработчики для кнопок +/-
  document.querySelectorAll('.btn-qty-plus').forEach(btn => {
    btn.addEventListener('click', (e) => {
      e.stopPropagation();
      const id = btn.dataset.id;
      const input = document.getElementById(`qty-${id}`);
      if (input) input.value = parseInt(input.value) + 1;
    });
  });
  
  document.querySelectorAll('.btn-qty-minus').forEach(btn => {
    btn.addEventListener('click', (e) => {
      e.stopPropagation();
      const id = btn.dataset.id;
      const input = document.getElementById(`qty-${id}`);
      if (input && parseInt(input.value) > 1) input.value = parseInt(input.value) - 1;
    });
  });
}

function renderProductItem(product) {
  const user = getUser();
  const role = user?.role?.name;
  const canAddToCart = role === 'Seller' || role === 'User';
  const canDelete = role === 'Admin';
  
  return `
    <div class="product-card" data-id="${product.id}">
      <div class="product-card-image">
        <img src="${product.imgURL ? `${BASE_URL}/${product.imgURL}?v=${Date.now()}` : 'https://via.placeholder.com/200x160?text=🥐'}" alt="${product.name}">
        ${product.stockQuantity < 10 ? '<span class="stock-badge low">Мало</span>' : ''}
      </div>
      <div class="product-card-body">
        <h3 class="product-card-title">${product.name}</h3>
        <div class="product-card-price">${product.price} ₽</div>
        <div class="product-card-meta">
          <span>📦 ${product.stockQuantity} шт.</span>
          <span>📏 ${product.quantityPerUnit} ${product.UOM?.name || ''}</span>
        </div>
        <div class="product-card-actions">
          ${canAddToCart ? `
            <div class="cart-control">
              <button class="btn-qty-minus" data-id="${product.id}">−</button>
              <input type="number" id="qty-${product.id}" class="qty-input-mini" min="1" value="1" step="1">
              <button class="btn-qty-plus" data-id="${product.id}">+</button>
              <button class="btn-cart" data-id="${product.id}">🛒</button>
            </div>
          ` : ''}
          ${canDelete ? `<button class="btn-delete-card" data-id="${product.id}" data-name="${product.name}">🗑️</button>` : ''}
        </div>
      </div>
    </div>
  `;
}

// Обработчики через делегирование
document.addEventListener('click', async (e) => {
  // Добавление в корзину
  if (e.target.classList.contains('btn-cart')) {
    e.stopPropagation();
    const productId = parseInt(e.target.dataset.id);
    const qtyInput = document.getElementById(`qty-${productId}`);
    const quantity = parseInt(qtyInput?.value || '1');
    
    if (quantity <= 0) {
      renderError('Введите корректное количество');
      return;
    }
    
    clearError();
    try {
      await addToCart(productId, quantity);
      renderError('✅ Товар добавлен в корзину');
      setTimeout(() => clearError(), 2000);
    } catch (_) {
      renderError(errorMessage);
    }
  }
  
  // Удаление товара
  if (e.target.classList.contains('btn-delete-card')) {
    e.stopPropagation();
    const productId = parseInt(e.target.dataset.id);
    const productName = e.target.dataset.name;
    
    if (await showConfirm(`Удалить товар "${productName}"?`)) {
      clearError();
      try {
        await apiDeleteProduct(productId);
        renderError(`✅ Товар "${productName}" удалён`);
        loadProducts();
      } catch (_) {
        renderError(errorMessage);
      }
    }
  }
});

// Функция закрытия всех модалок
function closeAllModals() {
  document.querySelectorAll('.modal-overlay').forEach(modal => modal.remove());
}

// Модалка для добавления производителя
function renderAddManufacturerModal(onSuccess) {
  const modal = document.createElement('div');
  modal.className = 'modal-overlay';
  modal.id = 'manufacturer-modal';
  
  modal.innerHTML = `
    <div class="modal-content">
      <div class="modal-header">
        <h2>➕ Добавить производителя</h2>
        <button class="modal-close">✕</button>
      </div>
      <form id="add-manufacturer-form" class="modal-form">
        <div class="form-group">
          <label>Название производителя *</label>
          <input type="text" name="name" required placeholder="Например: Хлебозавод №1" autofocus>
        </div>
        
        <div class="form-actions">
          <button type="submit" class="btn-primary">Добавить</button>
          <button type="button" class="btn-secondary cancel-btn">Отмена</button>
        </div>
      </form>
    </div>
  `;
  
  document.body.appendChild(modal);

  const form = modal.querySelector('#add-manufacturer-form');
  const nameInput = modal.querySelector('[name="name"]');
  const closeBtn = modal.querySelector('.modal-close');
  const cancelBtn = modal.querySelector('.cancel-btn');

  const closeModal = () => {
    modal.remove();
    document.removeEventListener('keydown', escHandler);
  };
  
  closeBtn.addEventListener('click', closeModal);
  cancelBtn.addEventListener('click', closeModal);
  modal.addEventListener('click', (e) => {
    if (e.target === modal) closeModal();
  });

  const escHandler = (e) => {
    if (e.key === 'Escape') {
      closeModal();
    }
  };
  document.addEventListener('keydown', escHandler);
  
  setTimeout(() => nameInput.focus(), 100);

  form.onsubmit = async (e) => {
    e.preventDefault();
    const name = nameInput.value.trim();
    
    if (!name) {
      renderError('Введите название производителя');
      return;
    }
    
    clearError();
    try {
      console.log('Creating manufacturer:', name); // Для отладки
      const newManufacturer = await createManufacturer(name);
      console.log('Created manufacturer result:', newManufacturer); // Для отладки
      
      if (newManufacturer) {
        // Сначала закрываем модалку
        closeModal();
        
        // Показываем сообщение об успехе
        renderError('✅ Производитель добавлен');
        setTimeout(() => clearError(), 2000);
        
        // Потом вызываем onSuccess для обновления родительского селекта
        if (onSuccess) {
          await onSuccess(newManufacturer);
        }
      } else {
        renderError('❌ Не удалось создать производителя');
      }
    } catch (error) {
      console.error('Error in form submit:', error);
      renderError(errorMessage);
    }
  };
}

// Форма добавления товара
async function renderAddProductForm() {
  const user = getUser();
  const role = user?.role?.name;
  const canAddProduct = role === 'Admin' || role === 'Seller';
  
  if (!canAddProduct) {
    renderError('⛔ Нет прав для добавления товара');
    return;
  }
  
  await loadAllDictionaries();
  
  // Закрываем все существующие модалки
  closeAllModals();
  
  // Создаём модальное окно прямо в body
  const modal = document.createElement('div');
  modal.className = 'modal-overlay';
  modal.id = 'product-modal';
  
  modal.innerHTML = `
    <div class="modal-content modal-large">
      <div class="modal-header">
        <h2>➕ Добавить товар</h2>
        <button class="modal-close">✕</button>
      </div>
      <form id="add-product-form" class="modal-form">
        <div class="form-row">
          <div class="form-group">
            <label>Название *</label>
            <input type="text" name="name" required placeholder="Например: Круассан">
          </div>
          <div class="form-group">
            <label>Цена *</label>
            <input type="number" step="0.01" name="price" required placeholder="₽">
          </div>
        </div>
        
        <div class="form-row">
          <div class="form-group">
            <label>Количество на единицу *</label>
            <input type="number" name="quantityPerUnit" required placeholder="1">
          </div>
          <div class="form-group">
            <label>Единица измерения *</label>
            <select name="UOM" required>
              ${state.uoms.map(u => `<option value="${u.id}">${u.name}</option>`).join('')}
            </select>
          </div>
        </div>
        
        <div class="form-row">
          <div class="form-group">
            <label>В наличии *</label>
            <input type="number" name="inStock" required placeholder="0">
          </div>
          <div class="form-group">
            <label>Производитель *</label>
            <div class="select-with-buttons">
              <select name="manufacturer" id="manufacturer-select"></select>
              <button type="button" id="add-manufacturer-btn" class="btn-icon" title="Добавить производителя">+</button>
              <button type="button" id="delete-manufacturer-btn" class="btn-icon" title="Удалить производителя">−</button>
            </div>
          </div>
        </div>
        
        <div class="form-group">
          <label>Описание *</label>
          <textarea name="description" rows="3" placeholder="Описание товара..."></textarea>
        </div>
        
        <div class="form-row">
          <div class="form-group">
            <label>Калорийность (на 100г) *</label>
            <input type="number" name="caloriesPer100g" placeholder="ккал">
          </div>
          <div class="form-group">
            <label>Срок годности (дней) *</label>
            <input type="number" name="expiration" placeholder="дней">
          </div>
          <div class="form-group">
            <label>Температура хранения *</label>
            <input type="number" name="tempStorage" placeholder="°C">
          </div>
        </div>
        
        <div class="form-section">
          <h3>📋 Ингредиенты</h3>
          <div id="ingredients-container" class="ingredients-list"></div>
          <button type="button" id="add-ingredient" class="btn-secondary">+ Добавить ингредиент</button>
        </div>
        
        <div class="form-actions">
          <button type="submit" class="btn-primary">Создать товар</button>
          <button type="button" class="btn-secondary cancel-btn">Отмена</button>
        </div>
      </form>
    </div>
  `;
  
  document.body.appendChild(modal);

  const form = modal.querySelector('#add-product-form');
  const ingredientsContainer = modal.querySelector('#ingredients-container');
  const manufacturerSelect = modal.querySelector('#manufacturer-select');
  const closeBtn = modal.querySelector('.modal-close');
  const cancelBtn = modal.querySelector('.cancel-btn');

  // Функция закрытия
  const closeModal = () => {
    modal.remove();
    document.removeEventListener('keydown', escHandler);
  };
  
  closeBtn.addEventListener('click', closeModal);
  cancelBtn.addEventListener('click', closeModal);
  modal.addEventListener('click', (e) => {
    if (e.target === modal) closeModal();
  });

  // Закрытие по Escape
  const escHandler = (e) => {
    if (e.key === 'Escape') {
      closeModal();
    }
  };
  document.addEventListener('keydown', escHandler);

  async function renderManufacturers() {
  console.log('Rendering manufacturers, current state:', state.manufacturers); // Для отладки
  await loadAllDictionaries();
  console.log('After loading, state manufacturers:', state.manufacturers); // Для отладки
  
  if (!manufacturerSelect) {
    console.error('Manufacturer select not found!');
    return;
  }
  
  if (state.manufacturers.length === 0) {
    manufacturerSelect.innerHTML = '<option value="">Нет производителей</option>';
    return;
  }
  
  manufacturerSelect.innerHTML = state.manufacturers.map(m => 
    `<option value="${m.id}">${m.name}</option>`
  ).join('');
  
  if (state.manufacturers.length > 0) {
    manufacturerSelect.value = state.manufacturers[0].id;
  }
}

// Вызов при инициализации
await renderManufacturers();

  // Добавление ингредиента
  modal.querySelector('#add-ingredient').onclick = () => {
    const row = document.createElement('div');
    row.className = 'ingredient-row';
    row.innerHTML = `
      <select name="warehouseItem" class="ingredient-select">
        ${state.warehouseItems.map(w => `<option value="${w.id}">${w.name}</option>`).join('')}
      </select>
      <input type="number" name="quantity" placeholder="Кол-во" class="ingredient-quantity" required>
      <button type="button" class="remove-ingredient btn-icon">✕</button>
    `;
    row.querySelector('.remove-ingredient').onclick = () => row.remove();
    ingredientsContainer.appendChild(row);
  };

  // Открытие модалки добавления производителя
  modal.querySelector('#add-manufacturer-btn').onclick = () => {
  console.log('Opening add manufacturer modal'); // Для отладки
  renderAddManufacturerModal(async (newManufacturer) => {
    console.log('Manufacturer added, new manufacturer:', newManufacturer); // Для отладки
    // Обновляем список производителей
    await renderManufacturers();
    console.log('After renderManufacturers, select options:', manufacturerSelect.innerHTML); // Для отладки
    // Устанавливаем только что добавленного производителя
    if (newManufacturer && newManufacturer.id) {
      manufacturerSelect.value = newManufacturer.id;
      console.log('Set select value to:', newManufacturer.id); // Для отладки
    }
  });
};


  // Удаление производителя
  modal.querySelector('#delete-manufacturer-btn').onclick = async () => {
    const id = Number(manufacturerSelect.value);
    if (!id) return;
    
    const selectedOption = manufacturerSelect.selectedOptions[0];
    const manufacturerName = selectedOption?.text || 'этого производителя';
    
    if (await showConfirm(`Удалить производителя "${manufacturerName}"?`)) {
      const ok = await removeManufacturer(id);
      if (ok) {
        renderManufacturers();
        renderError('✅ Производитель удалён');
        setTimeout(() => clearError(), 2000);
      }
    }
  };

  // Отправка формы
  form.onsubmit = async (e) => {
    e.preventDefault();
    const formData = new FormData(form);
    
    const body = {
      name: formData.get('name'),
      quantityPerUnit: Number(formData.get('quantityPerUnit')),
      UOM: { id: Number(formData.get('UOM')) },
      price: Number(formData.get('price')),
      inStock: Number(formData.get('inStock')),
      description: formData.get('description') || null,
      caloriesPer100g: Number(formData.get('caloriesPer100g')) || null,
      expiration: Number(formData.get('expiration')) || null,
      tempStorage: Number(formData.get('tempStorage')) || null,
      manufacturer: { id: Number(formData.get('manufacturer')) },
      ingredients: Array.from(ingredientsContainer.querySelectorAll('.ingredient-row')).map(row => ({
        warehouseItem: { id: Number(row.querySelector('[name="warehouseItem"]').value) },
        quantity: Number(row.querySelector('[name="quantity"]').value)
      }))
    };

    clearError();
    try {
      await addProduct(body);
      closeModal();
      loadProducts();
      renderError('✅ Товар успешно создан');
      setTimeout(() => clearError(), 3000);
    } catch (_) {
      renderError(errorMessage);
    }
  };
}

window.viewProduct = (id) => {
  if (window.loadPage) window.loadPage('product', id);
};