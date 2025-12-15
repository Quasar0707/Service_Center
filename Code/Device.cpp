#include "Device.h"
#include <chrono>

Device::Device(int id, double meanTime)
  : deviceId(id), isBusy(false), meanServiceTime(meanTime), serviceStartTime(0.0) {
  // Инициализируем генератор случайных чисел
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  generator.seed(seed + id); // Используем ID прибора для разных потоков

  // Настройка экспоненциального распределения
  distribution = std::exponential_distribution<double>(1.0 / meanServiceTime);
}

void Device::startService(const Request& req, double startTime) {
  isBusy = true;
  currentRequest = req;
  serviceStartTime = startTime;
  currentRequest.updateStatus(RequestStatus::PROCESSING);
}

void Device::completeService() {
  isBusy = false;
  currentRequest.updateStatus(RequestStatus::COMPLETED);
}

bool Device::isAvailable() const {
  return !isBusy;
}

double Device::getServiceTime() {
  return distribution(generator);
}