#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "Buffer.h"
#include "Device.h"
#include "Request.h"
#include <vector>

// Структура для результата назначения
struct AssignmentResult {
  bool success;
  int assignedRequestId;
  int assignedDeviceId;
  double serviceStartTime;

  AssignmentResult() : success(false), assignedRequestId(-1), assignedDeviceId(-1), serviceStartTime(0.0) {}
  AssignmentResult(bool s, int reqId, int devId, double time) : success(s), assignedRequestId(reqId), assignedDeviceId(devId), serviceStartTime(time) {}
};

class Dispatcher {
private:
  Buffer* buffer;           // Указатель на буфер
  std::vector<Device*> devices; // Вектор указателей на приборы
  int ringPointerBuffer;    // Указатель для кольца в буфере
  int ringPointerDevice;    // Указатель для кольца приборов

public:
  Dispatcher(Buffer* buf, std::vector<Device*> devs);

  // Метод для принятия заявки от источника
  bool acceptRequest(Request& req, Request& replacedReq);

  // Метод для добавления заявки в буфер D1031
  void addToBuffer(Request& req);

  // Метод для назначения заявки на прибор D2P2
  // Возвращает результат назначения
  AssignmentResult assignToDevice(double currentTime);

  // Метод для выбора заявки из буфера по приоритету D2Б4
  Request selectRequestForService();

  // Метод для выбора свободного прибора по кольцу D2P2
  Device* selectFreeDevice();

  Buffer* getBuffer() const { return buffer; }
  const std::vector<Device*>& getDevices() const { return devices; }

  int getRingPointerDevice() const { return ringPointerDevice; }
};

#endif