export function showConfirm(message) {
  return new Promise((resolve) => {
    const overlay = document.createElement('div');
    overlay.className = 'modal-overlay';
    overlay.style.zIndex = '20000';
    
    const modal = document.createElement('div');
    modal.className = 'modal-content';
    modal.style.maxWidth = '400px';
    modal.style.textAlign = 'center';
    
    modal.innerHTML = `
      <div class="modal-header">
        <h2>Подтверждение</h2>
        <button class="modal-close">✕</button>
      </div>
      <div style="padding: 1.5rem;">
        <p style="margin-bottom: 1.5rem;">${message}</p>
        <div style="display: flex; gap: 1rem; justify-content: center;">
          <button id="confirm-yes" class="btn-primary">Да</button>
          <button id="confirm-no" class="btn-secondary">Нет</button>
        </div>
      </div>
    `;
    
    overlay.appendChild(modal);
    document.body.appendChild(overlay);
    
    const close = () => overlay.remove();
    
    modal.querySelector('.modal-close').onclick = () => {
      close();
      resolve(false);
    };
    
    modal.querySelector('#confirm-no').onclick = () => {
      close();
      resolve(false);
    };
    
    modal.querySelector('#confirm-yes').onclick = () => {
      close();
      resolve(true);
    };
    
    overlay.onclick = (e) => {
      if (e.target === overlay) {
        close();
        resolve(false);
      }
    };
    
    setTimeout(() => {
      modal.querySelector('#confirm-yes').focus();
    }, 50);
  });
}