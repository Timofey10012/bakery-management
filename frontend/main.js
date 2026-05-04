import { renderDashboard } from './views/dashboard.js';
import { renderProducts } from './views/products.js';
import { renderProduct } from './views/product.js';
import { renderWarehouse } from './views/warehouse.js';
import { renderWarehouseItem } from './views/warehouseItem.js';
import { renderUsers } from './views/users.js';
import { renderSales } from './views/sales.js';
import { renderSale } from './views/sale.js';
import { renderCart } from './views/cart.js';
import { renderRegister } from './views/register.js';
import { renderLogin } from './views/login.js';
import { setUser, getUser, clearUser } from './state.js';
import { setToken, clearToken } from './api.js';

let globalLoadPage = null;

document.addEventListener('DOMContentLoaded', async () => {
  const content = document.getElementById('content');
  const nav = document.getElementById('nav');
  const userInfoEl = document.getElementById('userInfo');

  function loadPage(page, id) {
    if (page === 'login' || page === 'register') {
      nav.style.display = 'none';
      document.querySelector('.mobile-menu-btn')?.style.setProperty('display', 'none', 'important');
    } else {
      nav.style.display = 'flex';
      document.querySelector('.mobile-menu-btn')?.style.removeProperty('display');
    }
    
    if (content) {
      content.style.opacity = '0';
      content.style.transform = 'translateX(8px)';
    }
    
    setTimeout(() => {
      switch(page) {
        case 'dashboard':
          renderDashboard(content);
          break;
        case 'products':
          renderProducts(content);
          break;
        case 'product':
          renderProduct(content, id, loadPage);
          break;
        case 'warehouse':
          renderWarehouse(content, loadPage);
          break;
        case 'warehouse-item':
          renderWarehouseItem(content, id, loadPage);
          break;
        case 'users':
          renderUsers(content);
          break;
        case 'sales':
          renderSales(content, loadPage);
          break;
        case 'sale':
          renderSale(content, id, loadPage);
          break;
        case 'cart':
          renderCart(content);
          break;
        case 'register':
          renderRegister(content, onLoginSuccess, loadPage);
          break;
        case 'login':
          renderLogin(content, onLoginSuccess, loadPage);
          break;
        default:
          renderProducts(content);
      }
      
      setTimeout(() => {
        if (content) {
          content.style.opacity = '1';
          content.style.transform = 'translateX(0)';
        }
      }, 50);
      
      // Подсветка активного пункта меню
      document.querySelectorAll('#nav .nav-item').forEach(item => {
        const btn = item.querySelector('button');
        if (btn && btn.dataset.page === page) {
          item.classList.add('active');
        } else {
          item.classList.remove('active');
        }
      });
    }, 150);
  }

  globalLoadPage = loadPage;

  // Глобальная функция для открытия товара
  window.viewProduct = (id) => {
    loadPage('product', id);
  };

  function logout() {
    clearToken();
    clearUser();
    nav.style.display = 'none';
    loadPage('login');
  }

  function updateNavByRole() {
    const user = getUser();
    if (!user) return;

    const role = user.role?.name;
    
    const navItems = document.querySelectorAll('#nav .nav-item');
    navItems.forEach(item => item.style.display = 'none');
    
    let allowedPages = [];
    
    if (role === 'Admin') {
      allowedPages = ['dashboard', 'products', 'warehouse', 'users', 'sales'];
    } else if (role === 'Seller') {
      allowedPages = ['products', 'warehouse', 'cart'];
    } else { 
      allowedPages = ['products', 'cart'];
    }
    
    navItems.forEach(item => {
      const btn = item.querySelector('button');
      if (btn && allowedPages.includes(btn.dataset.page)) {
        item.style.display = 'flex';
      }
    });
  }

  function onLoginSuccess(data) {
    setToken(data.token);
    setUser(data.user);

    nav.style.display = 'flex';
    updateNavByRole();
    updateUserUI();
    
    const role = data.user.role?.name;
    if (role === 'Admin') {
      loadPage('dashboard');
    } else {
      loadPage('products');
    }
  }

  function updateUserUI() {
    const user = getUser();
    if (!user || !userInfoEl) return;

    const initials = `${user.firstName?.[0] || ''}${user.lastName?.[0] || ''}`.toUpperCase();
    const roleName = user.role?.name === 'Admin' ? 'Администратор' : 
                     user.role?.name === 'Seller' ? 'Продавец' : 'Покупатель';
    
    userInfoEl.innerHTML = `
      <div class="user-sidebar">
        <div class="user-avatar-sidebar">
          <span class="avatar-initials">${initials || '👤'}</span>
        </div>
        <div class="user-details">
          <div class="user-name-sidebar">${user.firstName} ${user.lastName}</div>
          <div class="user-role-sidebar">${roleName}</div>
        </div>
        <button id="logout-btn" class="logout-sidebar-btn">🚪 Выйти</button>
      </div>
    `;
    
    const logoutBtn = document.getElementById('logout-btn');
    if (logoutBtn) {
      logoutBtn.addEventListener('click', logout);
    }
  }

  // Построение бокового меню
  function buildSidebarMenu() {
    const menuItems = [
      { page: 'dashboard', icon: '📊', label: 'Главная', roles: ['Admin'] },
      { page: 'products', icon: '🥐', label: 'Товары', roles: ['Admin', 'Seller', 'User'] },
      { page: 'warehouse', icon: '📦', label: 'Склад', roles: ['Admin', 'Seller'] },
      { page: 'users', icon: '👥', label: 'Пользователи', roles: ['Admin'] },
      { page: 'sales', icon: '💰', label: 'Продажи', roles: ['Admin'] },
      { page: 'cart', icon: '🛒', label: 'Корзина', roles: ['Seller', 'User'] }
    ];
    
    const menuContainer = document.getElementById('nav-menu');
    if (!menuContainer) return;
    
    menuContainer.innerHTML = menuItems.map(item => `
      <div class="nav-item" data-page="${item.page}" style="display: none;">
        <button data-page="${item.page}" class="nav-button">
          <span class="nav-icon">${item.icon}</span>
          <span class="nav-label">${item.label}</span>
        </button>
      </div>
    `).join('');
    
    // Добавляем обработчики
    document.querySelectorAll('#nav-menu .nav-button').forEach(btn => {
      btn.addEventListener('click', () => loadPage(btn.dataset.page));
    });
  }
  
  buildSidebarMenu();

  // Проверка авторизации при загрузке
  const savedToken = localStorage.getItem('token');
  const savedUser = getUser();
  
  if (savedToken && savedUser) {
    setToken(savedToken);
    setUser(savedUser);
    nav.style.display = 'flex';
    updateNavByRole();
    updateUserUI();
    
    const role = savedUser.role?.name;
    if (role === 'Admin') {
      loadPage('dashboard');
    } else {
      loadPage('products');
    }
  } else {
    // Если нет сохранённых данных — показываем логин
    nav.style.display = 'none';
    loadPage('login');
  }
});

export function renderError(errorMessage) {
  const el = document.getElementById('error');
  if (!el) return;
  
  if (errorMessage) {
    el.textContent = errorMessage;
    el.style.display = 'block';
    setTimeout(() => {
      if (el.textContent === errorMessage) {
        el.style.display = 'none';
      }
    }, 5000);
  } else {
    el.style.display = 'none';
  }
}

export function clearError() {
  const el = document.getElementById('error');
  if (el) {
    el.style.display = 'none';
    el.textContent = '';
  }
}