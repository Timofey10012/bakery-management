import { getUsers, addUser, deleteUser, errorMessage } from '../api.js';
import { state, loadRoles } from './dictionaries.js';
import { renderError, clearError } from '../main.js';
import { getUser } from '../state.js';
import { showConfirm } from '../dialog.js';

export function renderUsers(container) {
  container.innerHTML = `
    <div class="users-header">
      <h1>👥 Пользователи</h1>
      <button id="btn-add-user" class="btn-primary">+ Добавить пользователя</button>
    </div>
    <div id="users-list" class="users-grid">
      <div class="loading-placeholder">Загрузка...</div>
    </div>
  `;

  loadUsers();
  document.getElementById('btn-add-user').onclick = () => renderAddUserForm();
}

async function loadUsers() {
  clearError();
  const el = document.getElementById('users-list');

  const data = await getUsers().catch(() => {
    renderError(errorMessage);
    el.innerHTML = `<div class="error-placeholder">❌ Ошибка загрузки пользователей</div>`;
    return null;
  });

  const users = data?.user || data?.users || [];
  
  if (!users.length) {
    el.innerHTML = `<div class="empty-placeholder">📭 Нет пользователей</div>`;
    return;
  }

  el.innerHTML = users.map(user => renderUserItem(user)).join('');
}

function renderUserItem(user) {
  const currentUser = getUser();
  const canDelete = currentUser?.role?.name === 'Admin' && currentUser?.id !== user.id;
  
  const initials = `${user.firstName?.[0] || ''}${user.lastName?.[0] || ''}`.toUpperCase();
  const roleName = getRoleName(user.role?.name);
  
  return `
    <div class="user-card" data-id="${user.id}">
      <div class="user-avatar">
        <span class="user-initials">${initials || '👤'}</span>
      </div>
      <div class="user-info">
        <div class="user-name">${user.firstName} ${user.lastName}</div>
        <div class="user-email">${user.email}</div>
        <div class="user-role-badge">${roleName}</div>
      </div>
      ${canDelete ? `
        <button class="btn-delete-user" data-id="${user.id}" data-name="${user.firstName} ${user.lastName}">
          🗑️
        </button>
      ` : ''}
    </div>
  `;
}

function getRoleName(role) {
  switch(role) {
    case 'Admin': return 'Администратор';
    case 'Seller': return 'Продавец';
    default: return 'Пользователь';
  }
}

// Обработчик удаления через делегирование
document.addEventListener('click', async (e) => {
  if (e.target.classList.contains('btn-delete-user')) {
    const userId = parseInt(e.target.dataset.id);
    const userName = e.target.dataset.name;
    
    if (await showConfirm(`Удалить пользователя "${userName}"?`)) {
      clearError();
      try {
        await deleteUser(userId);
        renderError(`✅ Пользователь "${userName}" удалён`);
        setTimeout(() => clearError(), 2000);
        loadUsers();
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

// Форма добавления пользователя
async function renderAddUserForm() {
  await loadRoles();
  
  // Закрываем все существующие модалки
  closeAllModals();
  
  // Создаём модальное окно прямо в body
  const modal = document.createElement('div');
  modal.className = 'modal-overlay';
  modal.id = 'user-modal';
  
  modal.innerHTML = `
    <div class="modal-content">
      <div class="modal-header">
        <h2>➕ Добавить пользователя</h2>
        <button class="modal-close">✕</button>
      </div>
      <form id="add-user-form" class="modal-form">
        <div class="form-row">
          <div class="form-group">
            <label>Имя *</label>
            <input type="text" name="firstName" required placeholder="Иван">
          </div>
          <div class="form-group">
            <label>Фамилия *</label>
            <input type="text" name="lastName" required placeholder="Петров">
          </div>
        </div>
        
        <div class="form-group">
          <label>Email *</label>
          <input type="email" name="email" required placeholder="user@example.com">
        </div>
        
        <div class="form-group">
          <label>Пароль *</label>
          <input type="password" name="password" required placeholder="••••••••">
        </div>
        
        <div class="form-group">
          <label>Роль *</label>
          <select name="role" required>
            ${state.roles.map(r => `<option value="${r.id}">${getRoleName(r.name)}</option>`).join('')}
          </select>
        </div>
        
        <div class="form-actions">
          <button type="submit" class="btn-primary">Создать</button>
          <button type="button" class="btn-secondary cancel-btn">Отмена</button>
        </div>
      </form>
    </div>
  `;
  
  document.body.appendChild(modal);

  const form = modal.querySelector('#add-user-form');
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
    const fd = new FormData(form);
    const body = {
      firstName: fd.get('firstName'),
      lastName: fd.get('lastName'),
      email: fd.get('email'),
      password: fd.get('password'),
      roleId: Number(fd.get('role'))
    };

    clearError();
    try {
      await addUser(body);
      closeModal();
      document.removeEventListener('keydown', escHandler);
      loadUsers();
      renderError('✅ Пользователь создан');
      setTimeout(() => clearError(), 2000);
    } catch (_) {
      renderError(errorMessage);
    }
  };
}