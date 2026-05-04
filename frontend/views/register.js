import { register, errorMessage } from '../api.js';
import { renderError, clearError } from '../main.js';

export function renderRegister(container, onSuccess, loadPage) {
  container.innerHTML = `
    <div class="auth-container">
      <div class="auth-card">
        <div class="auth-header">
          <span class="auth-emoji">🥐</span>
          <h1>Создать аккаунт</h1>
          <p>Зарегистрируйтесь в системе управления пекарней</p>
        </div>
        
        <form id="register-form" class="auth-form">
          <div class="form-row">
            <div class="form-group">
              <label for="firstName">👤 Имя</label>
              <input type="text" id="firstName" name="firstName" placeholder="Иван" autocomplete="given-name" required>
            </div>
            
            <div class="form-group">
              <label for="lastName">📛 Фамилия</label>
              <input type="text" id="lastName" name="lastName" placeholder="Петров" autocomplete="family-name" required>
            </div>
          </div>
          
          <div class="form-group">
            <label for="email">📧 Email</label>
            <input type="email" id="email" name="email" placeholder="example@bakery.com" autocomplete="email" required>
          </div>
          
          <div class="form-group">
            <label for="password">🔒 Пароль</label>
            <input type="password" id="password" name="password" placeholder="••••••••" autocomplete="new-password" required>
            <small class="form-hint">Минимум 8 символов</small>
          </div>
          
          <button type="submit" id="registerBtn" class="auth-btn">
            <span>Зарегистрироваться</span>
          </button>
          
          <div class="auth-footer">
            <p>Уже есть аккаунт? 
              <button type="button" id="toLogin" class="link-btn">Войти</button>
            </p>
          </div>
        </form>
        
        <div id="register-error" class="auth-error" style="display:none;"></div>
      </div>
    </div>
  `;

  const form = document.getElementById('register-form');
  const firstNameInput = document.getElementById('firstName');
  const lastNameInput = document.getElementById('lastName');
  const emailInput = document.getElementById('email');
  const passwordInput = document.getElementById('password');
  const errorDiv = document.getElementById('register-error');

  const clearErrorOnInput = () => {
    if (errorDiv.style.display === 'block') {
      errorDiv.style.display = 'none';
      clearError();
    }
  };
  
  firstNameInput.addEventListener('input', clearErrorOnInput);
  lastNameInput.addEventListener('input', clearErrorOnInput);
  emailInput.addEventListener('input', clearErrorOnInput);
  passwordInput.addEventListener('input', clearErrorOnInput);

  form.addEventListener('submit', async (e) => {
    e.preventDefault();
    
    const firstName = firstNameInput.value.trim();
    const lastName = lastNameInput.value.trim();
    const email = emailInput.value.trim();
    const password = passwordInput.value;
    
    if (!firstName) {
      renderError('Введите имя');
      errorDiv.textContent = 'Введите имя';
      errorDiv.style.display = 'block';
      firstNameInput.focus();
      return;
    }
    
    if (!lastName) {
      renderError('Введите фамилию');
      errorDiv.textContent = 'Введите фамилию';
      errorDiv.style.display = 'block';
      lastNameInput.focus();
      return;
    }
    
    if (!email) {
      renderError('Введите email');
      errorDiv.textContent = 'Введите email';
      errorDiv.style.display = 'block';
      emailInput.focus();
      return;
    }
    
    if (!email.includes('@')) {
      renderError('Введите корректный email');
      errorDiv.textContent = 'Введите корректный email (пример: name@domain.com)';
      errorDiv.style.display = 'block';
      emailInput.focus();
      return;
    }
    
    if (!password) {
      renderError('Введите пароль');
      errorDiv.textContent = 'Введите пароль';
      errorDiv.style.display = 'block';
      passwordInput.focus();
      return;
    }
    
    if (password.length < 8) {
      renderError('Пароль должен быть не менее 8 символов');
      errorDiv.textContent = 'Пароль должен содержать минимум 8 символов';
      errorDiv.style.display = 'block';
      passwordInput.focus();
      return;
    }
    
    const btn = form.querySelector('#registerBtn');
    const originalText = btn.innerHTML;
    btn.innerHTML = '<span class="btn-loader"></span> Регистрация...';
    btn.disabled = true;
    
    try {
      const data = await register(firstName, lastName, email, password);
      if (data?.token && data?.user) {
        onSuccess(data);
      }
    } catch (err) {
      const errorText = errorMessage || 'Ошибка регистрации. Попробуйте другой email.';
      renderError(errorText);
      errorDiv.textContent = errorText;
      errorDiv.style.display = 'block';
    } finally {
      btn.innerHTML = originalText;
      btn.disabled = false;
    }
  });

  document.getElementById('toLogin').addEventListener('click', (e) => {
    e.preventDefault();
    loadPage('login');
  });
}