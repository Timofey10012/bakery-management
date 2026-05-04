import { login, errorMessage } from '../api.js';
import { renderError, clearError } from '../main.js';

export function renderLogin(container, onSuccess, loadPage) {
  container.innerHTML = `
    <div class="auth-container">
      <div class="auth-card">
        <div class="auth-header">
          <span class="auth-emoji">🥐</span>
          <h1>Добро пожаловать</h1>
          <p>Войдите в систему управления пекарней</p>
        </div>
        
        <form id="login-form" class="auth-form">
          <div class="form-group">
            <label for="email">📧 Email</label>
            <input type="email" id="email" name="email" placeholder="example@bakery.com" autocomplete="email" required>
          </div>
          
          <div class="form-group">
            <label for="password">🔒 Пароль</label>
            <input type="password" id="password" name="password" placeholder="••••••••" autocomplete="current-password" required>
          </div>
          
          <button type="submit" id="loginBtn" class="auth-btn">
            <span>Войти</span>
          </button>
          
          <div class="auth-footer">
            <p>Нет аккаунта? 
              <button type="button" id="toRegister" class="link-btn">Зарегистрироваться</button>
            </p>
          </div>
        </form>
        
        <div id="login-error" class="auth-error" style="display:none;"></div>
      </div>
    </div>
  `;

  const form = document.getElementById('login-form');
  const emailInput = document.getElementById('email');
  const passwordInput = document.getElementById('password');
  const errorDiv = document.getElementById('login-error');

  // Очищаем ошибку при вводе
  const clearErrorOnInput = () => {
    if (errorDiv.style.display === 'block') {
      errorDiv.style.display = 'none';
      clearError();
    }
  };
  
  emailInput.addEventListener('input', clearErrorOnInput);
  passwordInput.addEventListener('input', clearErrorOnInput);

  form.addEventListener('submit', async (e) => {
    e.preventDefault();
    
    const email = emailInput.value.trim();
    const password = passwordInput.value;
    
    if (!email) {
      renderError('Введите email');
      errorDiv.textContent = 'Введите email';
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
    
    const btn = form.querySelector('#loginBtn');
    const originalText = btn.innerHTML;
    btn.innerHTML = '<span class="btn-loader"></span> Вход...';
    btn.disabled = true;
    
    try {
      const data = await login(email, password);
      if (data?.token && data?.user) {
        onSuccess(data);
      }
    } catch (err) {
      const errorText = errorMessage || 'Ошибка входа. Проверьте email и пароль.';
      renderError(errorText);
      errorDiv.textContent = errorText;
      errorDiv.style.display = 'block';
    } finally {
      btn.innerHTML = originalText;
      btn.disabled = false;
    }
  });

  document.getElementById('toRegister').addEventListener('click', (e) => {
    e.preventDefault();
    loadPage('register');
  });
}