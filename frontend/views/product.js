import { BASE_URL, getProductById, patchProductAddStock, uploadProductImage, addToCart, errorMessage } from '../api.js';
import { renderError, clearError } from '../main.js';
import { getUser } from '../state.js';

export function renderProduct(container, id, loadPage) {
  container.innerHTML = `
    <div class="product-detail-container">
      <button class="back-btn" id="back-btn">← Назад к списку</button>
      <div id="product-detail-content" class="product-detail-content">
        <div class="loading-spinner">Загрузка...</div>
      </div>
    </div>
  `;

  document.getElementById('back-btn').addEventListener('click', () => loadPage('products'));
  loadProduct(id);
}

async function loadProduct(id) {
  const el = document.getElementById('product-detail-content');
  clearError();

  const data = await getProductById(id).catch(() => {
    renderError(errorMessage);
    el.innerHTML = `<div class="error-state">❌ Ошибка загрузки товара</div>`;
    return null;
  });

  if (!data) {
    el.innerHTML = `<div class="error-state">📭 Товар не найден</div>`;
    return;
  }

  const user = getUser();
  const role = user?.role?.name;
  const canAddToCart = role === 'Seller' || role === 'User';
  const canManageStock = role === 'Admin' || role === 'Seller';
  const canUploadImage = role === 'Admin';

  const ingredientsList = (data.ingredients || []).map(ing => `
    <li><span class="ingredient-name">${ing.warehouseItem?.name ?? '—'}</span> — ${ing.quantity ?? 0} ${ing.UOM?.name ?? ''}</li>
  `).join('');

  el.innerHTML = `
    <div class="product-detail-card">
      <div class="product-detail-header">
      <div class="product-image-large">
        <img id="product-img" src="${data.imgURL ? `${BASE_URL}/${data.imgURL}?v=${Date.now()}` : 'https://via.placeholder.com/180x180?text=🥐'}" alt="${data.name}">
      </div>
      <div class="product-info-large">
         <h1>${data.name}</h1>
         <div class="product-badge">${data.UOM?.name ?? ''}</div>
         <div class="product-price-large">${data.price} ₽</div>
         <div class="product-stock-large ${data.inStock < 10 ? 'low-stock' : ''}">
            ${data.inStock > 0 ? `✅ В наличии: ${data.inStock} шт.` : '❌ Нет в наличии'}
         </div>
        </div>
      </div>
      
      <div class="product-detail-section">
        <h3>📝 Описание</h3>
        <p>${data.description || 'Нет описания'}</p>
      </div>
      
      <div class="product-detail-section">
        <h3>📊 Характеристики</h3>
        <div class="specs-grid">
          <div class="spec-item"><span class="spec-label">Количество:</span> ${data.quantityPerUnit} ${data.UOM?.name ?? ''}</div>
          <div class="spec-item"><span class="spec-label">Калорийность:</span> ${data.caloriesPer100g ?? '—'} ккал/100г</div>
          <div class="spec-item"><span class="spec-label">Срок годности:</span> ${data.expiration ?? '—'} дней</div>
          <div class="spec-item"><span class="spec-label">Температура:</span> ${data.tempStorage ?? '—'}°C</div>
          <div class="spec-item"><span class="spec-label">Производитель:</span> ${data.manufacturer?.name ?? '—'}</div>
        </div>
      </div>
      
      <div class="product-detail-section">
        <h3>🥣 Состав</h3>
        <ul class="ingredients-list">${ingredientsList || '<li>Нет данных</li>'}</ul>
      </div>
      
      ${canAddToCart ? `
        <div class="product-detail-section">
          <h3>🛒 Добавить в корзину</h3>
          <div class="action-group">
            <input type="number" id="cart-quantity" class="quantity-input" min="1" value="1">
            <button id="btn-add-to-cart" class="btn-primary">В корзину</button>
          </div>
        </div>
      ` : ''}
      
      ${canManageStock ? `
        <div class="product-detail-section">
          <h3>📦 Управление складом</h3>
          <div class="action-group">
            <input type="number" id="add-stock-quantity" class="quantity-input" placeholder="Количество" min="1">
            <button id="btn-add-stock" class="btn-secondary">➕ Пополнить</button>
          </div>
        </div>
      ` : ''}
      
      ${canUploadImage ? `
        <div class="product-detail-section">
          <h3>🖼️ Изображение</h3>
          <form id="upload-image-form" class="upload-form">
            <div class="custom-file-input">
              <input type="file" name="image" accept="image/*" id="image-file">
              <label for="image-file" class="custom-file-label">📁 Выбрать файл</label>
            </div>
            <button type="submit" class="btn-secondary">Загрузить</button>
            <p id="upload-status" class="upload-status"></p>
          </form>
        </div>
      ` : ''}
    </div>
  `;

  // Добавление в корзину
  const cartBtn = document.getElementById('btn-add-to-cart');
  if (cartBtn) {
    cartBtn.onclick = async () => {
      const qtyInput = document.getElementById('cart-quantity');
      const quantity = Number(qtyInput.value);
      if (!quantity || quantity <= 0) {
        renderError('Введите корректное количество');
        return;
      }
      clearError();
      try {
        await addToCart(id, quantity);
        renderError('✅ Товар добавлен в корзину');
        setTimeout(() => clearError(), 2000);
        qtyInput.value = '1';
      } catch (_) {
        renderError(errorMessage);
      }
    };
  }

  // Пополнение склада
  const stockBtn = document.getElementById('btn-add-stock');
  if (stockBtn) {
    stockBtn.onclick = async () => {
      const qtyInput = document.getElementById('add-stock-quantity');
      const quantity = Number(qtyInput.value);
      if (!quantity || quantity <= 0) {
        renderError('Введите корректное количество');
        return;
      }
      clearError();
      try {
        await patchProductAddStock(id, { quantity });
        renderError('✅ Склад пополнен');
        qtyInput.value = '';
        loadProduct(id);
      } catch (_) {
        renderError(errorMessage);
      }
    };
  }

  // Загрузка изображения
  const uploadForm = document.getElementById('upload-image-form');
  if (uploadForm) {
    const statusEl = document.getElementById('upload-status');
    const imgEl = document.getElementById('product-img');
    uploadForm.onsubmit = async (e) => {
      e.preventDefault();
      statusEl.textContent = 'Загрузка...';
      clearError();
      const fileInput = uploadForm.querySelector('[name="image"]');
      if (!fileInput.files.length) {
        renderError('Выберите файл');
        statusEl.textContent = '';
        return;
      }
      const formData = new FormData();
      formData.append('image', fileInput.files[0]);
      try {
        await uploadProductImage(id, formData);
        statusEl.textContent = '✅ Изображение загружено';
        const updatedData = await getProductById(id);
        imgEl.src = `${BASE_URL}/${updatedData.imgURL}?v=${Date.now()}`;
        fileInput.value = '';
        setTimeout(() => { statusEl.textContent = ''; }, 3000);
      } catch (err) {
        renderError(err.message || errorMessage);
        statusEl.textContent = '❌ Ошибка загрузки';
      }
    };
  }
}