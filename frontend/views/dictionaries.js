import {
  getRoles,
  getUOMs,
  getWarehouseItems,
  getManufacturers,
  addManufacturer,
  deleteManufacturer,
  errorMessage
} from '../api.js';
import { renderError, clearError } from '../main.js';

export const state = { roles: [], uoms: [], warehouseItems: [], manufacturers: [] };

export async function loadRoles() {
  clearError();
  try {
    state.roles = (await getRoles()).role || [];
  } catch (_) {
    renderError(errorMessage);
  }
}

export async function loadUOMs() {
  clearError();
  try {
    state.uoms = (await getUOMs()).UOM || [];
  } catch (_) {
    renderError(errorMessage);
  }
}

export async function loadWarehouseItems() {
  clearError();
  try {
    state.warehouseItems = (await getWarehouseItems()).warehouseItem || [];
  } catch (_) {
    renderError(errorMessage);
  }
}

export async function loadManufacturers() {
  clearError();
  try {
    const response = await getManufacturers();
    console.log('API Response:', response); // Для отладки
    state.manufacturers = response.manufacturer || [];
    console.log('State manufacturers:', state.manufacturers); // Для отладки
  } catch (_) {
    renderError(errorMessage);
  }
}

export async function loadAllDictionaries() {
  await Promise.all([loadRoles(), loadUOMs(), loadWarehouseItems(), loadManufacturers()]);
}

export async function createManufacturer(name) {
  clearError();
  try {
    const newM = await addManufacturer(name);
    console.log('New manufacturer response:', newM); // Для отладки
    
    // Проверяем структуру ответа
    let manufacturerData = newM;
    
    // Если ответ обернут в объект с полем manufacturer
    if (newM && newM.manufacturer) {
      manufacturerData = newM.manufacturer;
    }
    
    if (manufacturerData && manufacturerData.id) {
      // Добавляем в state
      state.manufacturers.push(manufacturerData);
      console.log('Updated state manufacturers:', state.manufacturers); // Для отладки
      return manufacturerData;
    } else {
      // Если не получили данные, перезагружаем список
      await loadManufacturers();
      // Ищем по имени (временное решение)
      const found = state.manufacturers.find(m => m.name === name);
      return found || null;
    }
  } catch (error) {
    console.error('Error creating manufacturer:', error);
    renderError(errorMessage);
    return null;
  }
}

export async function removeManufacturer(id) {
  clearError();
  try {
    await deleteManufacturer(id);
    state.manufacturers = state.manufacturers.filter(m => m.id !== id);
    return true;
  } catch (_) {
    renderError(errorMessage);
    return false;
  }
}