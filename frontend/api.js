import { mapError } from './errorHandler.js';

export const BASE_URL = 'http://localhost:8080';

let token = null;
export let errorMessage = '';

export function setToken(t) {
  token = t;
  if (t) {
    localStorage.setItem('token', t);
  } else {
    localStorage.removeItem('token');
  }
}

export function clearToken() {
  token = null;
  localStorage.removeItem('token');
}

export function clearError() {
  errorMessage = '';
}

async function request(path, options = {}) {
  clearError();

  try {
    const res = await fetch(BASE_URL + path, {
      headers: {
        'Content-Type': 'application/json',
        ...(token ? { Authorization: `Bearer ${token}` } : {}),
        ...(options.headers || {})
      },
      ...options
    });

    let data = null;

   if (res.status !== 204) {
    try {
      data = await res.json();
    } catch (_) {
      
    }
   }

    if (!res.ok) {
      errorMessage = mapError(data, res.status);
      throw new Error(errorMessage);
    }

    return data;

  } catch (err) {
    if (!errorMessage) {
      errorMessage = 'Ошибка сети';
    }

    throw err;
  }
}

export async function uploadProductImage(productId, formData) {
  const res = await fetch(`${BASE_URL}/products/${productId}/image`, {
    method: 'POST',
    body: formData,
    headers: { Authorization: `Bearer ${token}` }
  });

  if (!res.ok) {
    const text = await res.text();
    throw new Error(text || 'Ошибка загрузки изображения');
  }

  return true;
}

export async function login(email, password) {
  return request('/auth/login', {
    method: 'POST',
    body: JSON.stringify({ email, password })
  });
}

export async function register(firstName, lastName, email, password) {
  return request('/auth/register', {
    method: 'POST',
    body: JSON.stringify({ firstName, lastName, email, password })
  });
}

export async function getMe() {
  return request('/auth/me', {
    method: 'GET'
  });
}

export function getRoles() {
  return request('/dictionaries/roles', { 
    method: 'GET' 
  });
}

export function getUOMs() {
  return request('/dictionaries/uoms', { 
    method: 'GET'
  });
}

export function getWarehouseItems() {
  return request('/dictionaries/warehouseItems', { 
    method: 'GET'
   });
}

export function getManufacturers() {
  return request('/dictionaries/manufacturers', {
     method: 'GET'
   });
}

export function addManufacturer(name) {
  return request('/dictionaries/manufacturers', {
    method: 'POST',
    body: JSON.stringify({ name })
  });
}

export function deleteManufacturer(id) {
  return request(`/dictionaries/manufacturers/${id}`, { 
    method: 'DELETE' 
  });
}

export async function getDashboardSummary() {
  return request('/dashboard/summary', {
    method: 'GET'
  });
}

export async function getDashboardActivity() {
  return request('/dashboard/activity', {
    method: 'GET'
  });
}

export async function getProducts() {
  return request('/products', {
    method: 'GET'
  });
}

export async function getProductById(id) {
  return request(`/products/${id}`, {
    method: 'GET'
  });
}

export async function addProduct(body) {
  return request('/products', {
    method: 'POST',
    body: JSON.stringify(body)
  });
}

export function deleteProduct(id) {
  return request(`/products/${id}`, { 
    method: 'DELETE' 
  });
}

export function patchProductAddStock(id, quantity) {
  return request(`/products/${id}/add-stock`, { 
    method: 'PATCH',
    body: JSON.stringify(quantity)
  });
}

export async function getWarehouse() {
  return request('/warehouse', { 
    method: 'GET' 
  });
}

export async function addWarehouseItem(body) {
  return request('/warehouse', { 
    method: 'POST',
    body: JSON.stringify(body),
  });
}

export async function deleteWarehouseItem(id) {
  return request(`/warehouse/${id}`, { 
    method: 'DELETE' 
  });
}

export async function getWarehouseItemById(id) {
  return request(`/warehouse/${id}`, { 
    method: 'GET' 
  });
}

export async function patchWarehouseAddStock(id, body) {
  return request(`/warehouse/${id}/add-stock`, {
    method: 'PATCH',
    body: JSON.stringify(body)
  });
}

export async function patchWarehouseRemoveStock(id, body) {
  return request(`/warehouse/${id}/remove-stock`, {
    method: 'PATCH',
    body: JSON.stringify(body)
  });
}

export async function getUsers() {
  return request('/users', { 
    method: 'GET' 
  });
}

export async function addUser({ firstName, lastName, email, password, roleId }) {
  return request('/users', {
    method: 'POST',
    body: JSON.stringify({
      firstName,
      lastName,
      email,
      password,
      role: { id: roleId }
    })
  });
}

export async function getUserById(id) {
  return request(`/users/${id}`, { 
    method: 'GET' 
  });
}

export async function deleteUser(id) {
  return request(`/users/${id}`, { 
    method: 'DELETE' 
  });
}

export async function getSales() {
  return request('/sales', { method: 'GET' });
}

export async function getSaleById(id) {
  return request(`/sales/${id}`, { method: 'GET' });
}

export async function getCart() {
  return request('/cart', { method: 'GET' });
}

export async function deleteCartItem(productId) {
  return request('/cart', {
    method: 'DELETE',
    body: JSON.stringify({ id: productId })
  });
}

export async function patchCartAddStock(productId, quantityItems) {
  return request(`/cart/add-stock`, {
    method: 'PATCH',
    body: JSON.stringify({ productItem: { id: productId }, quantityItems })
  });
}

export async function patchCartRemoveStock(productId, quantityItems) {
  return request(`/cart/remove-stock`, {
    method: 'PATCH',
    body: JSON.stringify({ productItem: { id: productId }, quantityItems })
  });
}

export async function createOrder() {
  return request('/orders', { method: 'POST' });
}

export async function addToCart(productId, quantity) {
  return request('/cart', {
    method: 'POST',
    body: JSON.stringify({ 
      productItem: { id: productId }, 
      quantityItems: quantity 
    })
  });
}