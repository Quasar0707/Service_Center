#ifndef SIMULATIONCONTROLLER_H
#define SIMULATIONCONTROLLER_H

#include "Source.h"
#include "Buffer.h"
#include "Device.h"
#include "Dispatcher.h"
#include <vector>
#include <queue>
#include <map>
#include <string>
#include <iostream>
#include <csignal>

struct Event {
  double time;          // Время события
  std::string type;     // Тип события (GENERATION, SERVICE_COMPLETE)
  int sourceId;         // ID источника (для GENERATION)
  int deviceId;         // ID прибора (для SERVICE_COMPLETE)
  int requestId;        // ID заявки
  Request request;      // Сама заявка (для GENERATION)

  // Конструктор для события генерации
  Event(double t, const std::string& ty, int srcId, int reqId, const Request& req)
    : time(t), type(ty), sourceId(srcId), deviceId(-1), requestId(reqId), request(req) {}

  // Конструктор для события завершения обслуживания
  Event(double t, const std::string& ty, int devId, int reqId)
    : time(t), type(ty), sourceId(-1), deviceId(devId), requestId(reqId) {}

  // Оператор сравнения для приоритетной очереди (меньшее время - выше приоритет)
  bool operator>(const Event& other) const {
    return time > other.time;
  }
};

class SimulationController {
private:
  std::vector<Source> sources;      // Вектор источников
  Buffer buffer;                    // Буфер
  std::vector<Device> devices;      // Вектор приборов
  Dispatcher dispatcher;            // Диспетчер

  // Статистика
  int totalRequestsGenerated;
  int totalRequestsRejected;
  int totalRequestsCompleted;
  std::map<int, int> requestsBySource;      // Количество заявок по источникам
  std::map<int, int> rejectedBySource;      // Количество отклоненных заявок по источникам
  std::map<int, int> completedBySource;     // Количество завершенных заявок по источникам
  std::map<int, double> totalTimeInSystem;  // Общее время в системе по источникам
  std::map<int, double> totalTimeWaiting;   // Общее время ожидания по источникам
  std::map<int, double> totalTimeProcessing; // Общее время обработки по источникам

  // Сумма квадратов отклонений для времени ожидания (T БП)
  std::map<int, double> sumSqDiffWaitingTime;
  // Сумма квадратов отклонений для времени обслуживания (T обсл)
  std::map<int, double> sumSqDiffProcessingTime;

  // Календарь событий
  std::priority_queue<Event, std::vector<Event>, std::greater<Event>> eventQueue;

  // Текущее модельное время
  double currentTime;

  // Параметры симуляции
  double simulationEndTime; // Время окончания симуляции
  int bufferSize;           // Размер буфера
  double meanServiceTime;   // Среднее время обслуживания

  // Счётчик для уникального ID заявок
  int nextRequestId;

public:
  SimulationController();

  void runSimulationStepByStep(); // Пошаговый режим (ОД1)
  void runSimulationAutomatic();  // Автоматический режим (ОР1)

  // Метод для выполнения одного шага симуляции
  bool stepSimulation();

  // Метод для печати текущего состояния системы (ОД1)
  void printCurrentState();

  // Метод для печати сводной таблицы результатов (ОР1)
  void printSummary();

  // Метод для инициализации системы
  void initializeSystem();

  // Методы для обработки различных типов событий
  void handleGenerationEvent(const Event& event);
  void handleServiceCompleteEvent(const Event& event);

};

#endif