#include "Dispatcher.h"
#include <algorithm>
#include <stdexcept>
#include <iostream>

Dispatcher::Dispatcher(Buffer* buf, std::vector<Device*> devs)
  : buffer(buf), devices(devs), ringPointerBuffer(0), ringPointerDevice(0) {}

bool Dispatcher::acceptRequest(Request& req, Request& replacedReq) {
  // Проверяем, есть ли свободные места в буфере
  if (!buffer->isFull()) {
    addToBuffer(req);
    return true;
  }
  else {
    // Буфер полон, применяем дисциплину вытеснения D1004
    bool added = buffer->addRequest(req, replacedReq);
    if (added) {
      return true;
    }
    else {
      std::cout << "Ошибка: buffer->addRequest вернул false при полоном буфере в D1004." << std::endl;
      return false;
    }
  }
}

void Dispatcher::addToBuffer(Request& req) {
  // Добавляем заявку в буфер по кольцу (D1031)
  Request dummyReplacedReq; // Временная заявка, не используется, если буфер не полон
  bool added = buffer->addRequest(req, dummyReplacedReq);
  if (!added) {
    std::cout << "Ошибка: buffer->addRequest вернул false при добавлении в неполный буфер." << std::endl;
  }
}

AssignmentResult Dispatcher::assignToDevice(double currentTime) {
  if (buffer->isEmpty()) {
    return AssignmentResult();
  }

  // Выбираем свободный прибор по кольцу (D2P2)
  Device* selectedDevice = selectFreeDevice();

  if (!selectedDevice || !selectedDevice->isAvailable()) {
    return AssignmentResult();
  }

  // Выбираем заявку с наивысшим приоритетом (D2Б4)
  Request selectedReq = selectRequestForService();

  // Назначаем заявку на прибор
  selectedDevice->startService(selectedReq, currentTime);

  // Удаляем заявку из буфера - помечаем слот как свободный
  auto& slots = buffer->getSlots();
  auto& occupied = buffer->getOccupancy();
  for (size_t i = 0; i < slots.size(); ++i) {
    if (occupied[i] && slots[i].getRequestId() == selectedReq.getRequestId()) {
      buffer->markSlotFree(i);
      break;
    }
  }

  return AssignmentResult(true, selectedReq.getRequestId(), selectedDevice->getDeviceId(), currentTime);
}

Request Dispatcher::selectRequestForService() {
  // D2Б4: выбрать заявку с наивысшим приоритетом
  // Приоритет (WARRANTY > CORPORATE > PRIVATE)

  auto& slots = buffer->getSlots();
  auto& occupied = buffer->getOccupancy();

  const Request* maxReq = nullptr;
  // Используем время поступления в буфер для сравнения при равенстве приоритетов
  double maxTimeEntered = -1.0;

  for (size_t i = 0; i < slots.size(); ++i) {
    // Игнорируем пустые слоты
    if (!occupied[i]) {
      continue;
    }
    auto& slot = slots[i];

    bool shouldUpdate = false;
    if (!maxReq) {
      shouldUpdate = true;
    }
    else {
      if (slot.getPriority() > maxReq->getPriority()) {
        shouldUpdate = true;
      }
      else if (slot.getPriority() == maxReq->getPriority()) {
        // Если приоритет равен, выбираем с большим timeEnteredBuffer
        if (slot.getTimeEnteredBuffer() > maxTimeEntered) {
          shouldUpdate = true;
        }
      }
    }

    if (shouldUpdate) {
      maxReq = &slot;
      maxTimeEntered = slot.getTimeEnteredBuffer(); // Обновляем время
    }
  }

  if (maxReq) {
    return *maxReq;
  }

  throw std::runtime_error("Буфер пуст или все заявки назначены, нельзя выбрать заявку.");
}

Device* Dispatcher::selectFreeDevice() {
  // D2P2: выбрать свободный прибор по кольцу
  if (devices.empty()) {
    return nullptr;
  }

  // Начинаем с текущего указателя
  int start = ringPointerDevice;
  do {
    if (devices[ringPointerDevice]->isAvailable()) {
      Device* freeDevice = devices[ringPointerDevice];
      ringPointerDevice = (ringPointerDevice + 1) % devices.size();
      return freeDevice;
    }
    ringPointerDevice = (ringPointerDevice + 1) % devices.size();
  } while (ringPointerDevice != start);

  return nullptr;
}