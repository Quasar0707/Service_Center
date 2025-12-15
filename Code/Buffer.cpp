#include "Buffer.h"
#include <stdexcept>

Buffer::Buffer(int cap) : capacity(cap), ringPointer(0) {
  slots.resize(capacity);
  occupied.resize(capacity, false);
}

bool Buffer::addRequest(const Request& req, Request& replacedReq) {
  bool wasReplaced = false;

  if (isFull()) {
    // применяем дисциплину вытеснения D1004
    // Вытесняем последнюю поступившую заявку
    int replaceIndex = findRequestForReplacement();
    if (replaceIndex != -1) {
      // Сохраняем вытесняемую заявку
      replacedReq = slots[replaceIndex];
      wasReplaced = true; // Флаг, что вытеснение произошло
      replacedReq.updateStatus(RequestStatus::REJECTED); // Помечаем вытеснённую как REJECTED
      // Перезаписываем слот с новой заявкой
      slots[replaceIndex] = req;
      occupied[replaceIndex] = true; // Убедимся, что слот помечен как занятый
    }
    else {
      return false;
    }
  }
  else {
    // Есть место, добавляем по кольцу (D1031)
    // Найдём первое свободное место, начиная с ringPointer
    int insertIndex = -1;
    for (int i = 0; i < capacity; ++i) {
      int idx = (ringPointer + i) % capacity;
      if (!occupied[idx]) {
        insertIndex = idx;
        break;
      }
    }
    if (insertIndex != -1) {
      slots[insertIndex] = req; // Копируем req с его timeEnteredBuffer
      occupied[insertIndex] = true;
      // Обновляем указатель кольца на следующую позицию после вставленного
      ringPointer = (insertIndex + 1) % capacity;
    }
    else {
      return false;
    }
  }
  return true;
}

void Buffer::markSlotFree(int index) {
  if (index >= 0 && index < capacity) {
    occupied[index] = false;
  }
}

int Buffer::findNextFreeSlot() {
  for (int i = 0; i < capacity; ++i) {
    int idx = (ringPointer + i) % capacity;
    if (!occupied[idx]) {
      return idx;
    }
  }
  // Буфер полон
  return -1;
}

int Buffer::findRequestForReplacement() {
  if (occupied.empty()) {
    return -1;
  }
  // D1004: выбираем последнюю заявку
  int lastAddedIndex = (ringPointer - 1 + capacity) % capacity;
  if (occupied[lastAddedIndex]) {
    return lastAddedIndex;
  }
  // Если не занят, ищем последнюю занятую
  for (int i = capacity - 1; i >= 0; --i) {
    int idx = (ringPointer - 1 - i + capacity) % capacity;
    if (occupied[idx]) {
      return idx;
    }
  }
  return -1;
}

bool Buffer::isFull() const {
  for (bool occ : occupied) {
    if (!occ) return false;
  }
  return true;
}

bool Buffer::isEmpty() const {
  for (bool occ : occupied) {
    if (occ) return false;
  }
  return true;
}

int Buffer::getCurrentSize() const {
  int count = 0;
  for (bool occ : occupied) {
    if (occ) count++;
  }
  return count;
}