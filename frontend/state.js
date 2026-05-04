let currentUser = null;

export function setUser(user) {
  currentUser = user;
  if (user) {
    localStorage.setItem('user', JSON.stringify(user));
  } else {
    localStorage.removeItem('user');
  }
}

export function getUser() {
  if (!currentUser) {
    const saved = localStorage.getItem('user');
    if (saved) {
      currentUser = JSON.parse(saved);
    }
  }
  return currentUser;
}

export function clearUser() {
  currentUser = null;
  localStorage.removeItem('user');
}