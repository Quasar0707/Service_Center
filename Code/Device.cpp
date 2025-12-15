#include "Device.h"
#include <chrono>

Device::Device(int id, double meanTime)
  : deviceId(id), isBusy(false), meanServiceTime(meanTime), serviceStartTime(0.0), totalTimeBusy(0.0) {
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  generator.seed(seed + id);
  distribution = std::exponential_distribution<double>(1.0 / meanServiceTime);
}

void Device::startService(const Request& req, double startTime) {
  isBusy = true;
  currentRequest = req;
  serviceStartTime = startTime;
  currentRequest.updateStatus(RequestStatus::PROCESSING);
}

void Device::completeService(double endTime) { // Принимает время завершения
  if (isBusy) {
    totalTimeBusy += (endTime - serviceStartTime); // Обновляем totalTimeBusy
  }
  isBusy = false;
  currentRequest.updateStatus(RequestStatus::COMPLETED);
}

bool Device::isAvailable() const {
  return !isBusy;
}

int Device::getDeviceId() const { return deviceId; }
bool Device::getIsBusy() const { return isBusy; }
const Request& Device::getCurrentRequest() const { return currentRequest; }
double Device::getServiceStartTime() const { return serviceStartTime; }
double Device::getTotalTimeBusy() const { return totalTimeBusy; }
double Device::getServiceTime() { return distribution(generator); }