#ifndef DEVICE_H
#define DEVICE_H

#include "Request.h"
#include <random>
#include <chrono>

class Device {
private:
  int deviceId;
  bool isBusy;
  Request currentRequest;
  std::default_random_engine generator;
  std::exponential_distribution<double> distribution;
  double meanServiceTime;
  double serviceStartTime;
  double totalTimeBusy; // <-- Новое поле

public:
  Device(int id, double meanTime);
  void startService(const Request& req, double startTime);
  void completeService(double endTime); // <-- Изменённая сигнатура
  bool isAvailable() const;
  int getDeviceId() const;
  bool getIsBusy() const;
  const Request& getCurrentRequest() const;
  double getServiceStartTime() const;
  double getTotalTimeBusy() const; // <-- Новый геттер
  double getServiceTime();
};

#endif // DEVICE_H