#ifndef DEVICE_H
#define DEVICE_H

#include "Request.h"
#include <random>
#include <chrono>

// Класс прибора (мастера)
class Device {
private:
  int deviceId;             // Уникальный ID прибора
  bool isBusy;              // Флаг занятости
  Request currentRequest;   // Текущая заявка, которую обслуживает прибор
  std::default_random_engine generator; // Генератор случайных чисел
  std::exponential_distribution<double> distribution; // Экспоненциальное распределение

  // Параметры для экспоненциального распределения
  double meanServiceTime;   // Среднее время обслуживания
  double serviceStartTime;  // Время начала обслуживания текущей заявки

public:
  Device(int id, double meanTime);

  // Метод для начала обслуживания заявки
  void startService(const Request& req, double startTime);

  // Метод для завершения обслуживания
  void completeService();

  // Метод для проверки доступности
  bool isAvailable() const;

  int getDeviceId() const { return deviceId; }
  bool getIsBusy() const { return isBusy; }
  const Request& getCurrentRequest() const { return currentRequest; }
  double getServiceStartTime() const { return serviceStartTime; }

  // Метод для получения времени обслуживания
  double getServiceTime();
};

#endif