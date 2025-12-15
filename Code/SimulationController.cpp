#include "SimulationController.h"
#include <iomanip>
#include <cmath>

extern volatile sig_atomic_t g_signalRaised;

SimulationController::SimulationController()
  : buffer(5), // размер буфера 5
  dispatcher(&buffer, {}), //  диспетчер
  currentTime(0.0),
  simulationEndTime(1000.0), // длительность симуляции
  bufferSize(5),
  meanServiceTime(10.0), // среднее время обслуживания
  nextRequestId(1) { // счётчик заявок

  initializeSystem();
}

void SimulationController::initializeSystem() {
  // Источник 1: Гарантийный (высший приоритет)
  sources.emplace_back(1, 10.0, Priority::WARRANTY); // Среднее время между заявками 10
  // Источник 2: Корпоративный (средний приоритет)
  sources.emplace_back(2, 7.0, Priority::CORPORATE);
  // Источник 3: Частный (низший приоритет)
  sources.emplace_back(3, 5.0, Priority::PRIVATE);

  // Три прибора
  for (int i = 1; i <= 3; ++i) {
    devices.emplace_back(i, meanServiceTime);
  }

  // диспетчер с буфером и приборами
  std::vector<Device*> devicePtrs;
  for (auto& device : devices) {
    devicePtrs.push_back(&device);
  }
  dispatcher = Dispatcher(&buffer, devicePtrs);

  // Генерируем первые события для каждого источника
  for (auto& source : sources) {
    double nextGenTime = source.getNextGenerationTime(currentTime);
    Request firstRequest = source.generateRequest(nextGenTime, nextRequestId++);
    firstRequest.setTimeEnteredBuffer(nextGenTime);
    Event genEvent(nextGenTime, "GENERATION", source.getSourceId(), firstRequest.getRequestId(), firstRequest);
    eventQueue.push(genEvent);
  }

  // Инициализируем статистику
  totalRequestsGenerated = 0;
  totalRequestsRejected = 0;
  totalRequestsCompleted = 0;
  for (int i = 1; i <= 3; ++i) {
    requestsBySource[i] = 0;
    rejectedBySource[i] = 0;
    completedBySource[i] = 0;
    totalTimeInSystem[i] = 0.0;
    totalTimeWaiting[i] = 0.0;
    totalTimeProcessing[i] = 0.0;
    sumSqDiffWaitingTime[i] = 0.0;
    sumSqDiffProcessingTime[i] = 0.0;
  }
}

// Пошаговый режим (ОД1)
void SimulationController::runSimulationStepByStep() {
  std::cout << "=== Запуск пошаговой симуляции сервисного центра ===" << std::endl;
  std::cout << "Начальное состояние системы:" << std::endl;
  printCurrentState();
  std::cout << "\nНажмите Enter для выполнения следующего шага... (Ctrl+C для выхода)" << std::endl;
  std::cin.get();
  if (g_signalRaised == SIGINT) return;

  while (stepSimulation()) {
    printCurrentState(); // Печатаем состояние после каждого шага
    std::cout << "\n--- Нажмите Enter для следующего шага (или Ctrl+C для выхода) ---" << std::endl;
    std::cin.get();
    if (g_signalRaised == SIGINT) {
      printSummary();
      return;
    }
  }

  std::cout << "\nСимуляция завершена." << std::endl;
}

// Автоматический режим (ОР1)
void SimulationController::runSimulationAutomatic() {

  while (stepSimulation()) {
    // Цикл работает до конца, без ввода
  }

  printSummary();
}

bool SimulationController::stepSimulation() {
  if (eventQueue.empty()) {
    return false;
  }

  Event currentEvent = eventQueue.top();
  eventQueue.pop();

  if (currentEvent.time > simulationEndTime) {
    return false;
  }

  currentTime = currentEvent.time;

  // Обрабатываем событие
  if (currentEvent.type == "GENERATION") {
    handleGenerationEvent(currentEvent);
  }
  else if (currentEvent.type == "SERVICE_COMPLETE") {
    handleServiceCompleteEvent(currentEvent);
  }

  dispatcher.assignToDevice(currentTime);

  return true;
}

void SimulationController::printCurrentState() {
  std::cout << "\n=== ТЕКУЩЕЕ СОСТОЯНИЕ СИСТЕМЫ ===" << std::endl;
  std::cout << "Текущее модельное время: " << currentTime << std::endl;

  std::cout << "Сделано (завершено): " << totalRequestsCompleted << std::endl;
  std::cout << "Отказано (вытеснено): " << totalRequestsRejected << std::endl;

  std::cout << "\n--- ВЕРОЯТНОСТЬ ОТКАЗА (P отк) ---" << std::endl;
  for (int i = 1; i <= 3; ++i) {
    double p_otk = (requestsBySource[i] > 0) ? static_cast<double>(rejectedBySource[i]) / requestsBySource[i] : 0.0;
    std::cout << "  Источник " << i << ": " << std::fixed << std::setprecision(4) << p_otk << std::endl;
  }

  std::cout << "\n--- УКАЗАТЕЛИ ---" << std::endl;
  std::cout << "  ringPointer (буфер): " << buffer.getRingPointer() << std::endl;
  std::cout << "  ringPointerDevice (диспетчер): " << dispatcher.getRingPointerDevice() << std::endl;

  std::cout << "\n--- БУФЕР ---" << std::endl;
  std::cout << "Ёмкость: " << buffer.getCapacity() << ", Занято: " << buffer.getCurrentSize() << std::endl;
  const auto& slots = buffer.getSlots();
  const auto& occupied = buffer.getOccupancy();
  for (size_t i = 0; i < slots.size(); ++i) {
    if (occupied[i]) {
      std::cout << "  Позиция " << i << ": Заявка " << slots[i].getIdString()
        << " (Источник " << slots[i].getSourceId() << ", Приоритет: " << Request::priorityToString(slots[i].getPriority()) << ")" << std::endl;
    }
    else {
      std::cout << "  Позиция " << i << ": ПУСТО" << std::endl;
    }
  }

  std::cout << "\n--- ПРИБОРЫ ---" << std::endl;
  for (size_t i = 0; i < devices.size(); ++i) {
    const Device& dev = devices[i];
    std::cout << "  Прибор " << dev.getDeviceId() << ": ";
    if (dev.getIsBusy()) {
      std::cout << "ЗАНЯТ (Заявка " << dev.getCurrentRequest().getIdString()
        << ", Источник " << dev.getCurrentRequest().getSourceId() << ", Время начала: " << dev.getServiceStartTime() << ")" << std::endl;
    }
    else {
      std::cout << "СВОБОДЕН" << std::endl;
    }
  }

  std::cout << "\n--- КАЛЕНДАРЬ СОБЫТИЙ (оставшиеся) ---" << std::endl;
  auto tempQueue = eventQueue;
  if (tempQueue.empty()) {
    std::cout << "  (пусто)" << std::endl;
  }
  else {
    // Таблица
    std::cout << std::setw(10) << "Время" << " | " << std::setw(15) << "Тип" << " | " << std::setw(10) << "Источник" << " | " << std::setw(10) << "Прибор" << " | " << std::setw(10) << "Заявка" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    while (!tempQueue.empty()) {
      const Event& e = tempQueue.top();
      std::cout << std::setw(10) << e.time << " | " << std::setw(15) << e.type;
      if (e.type == "GENERATION") {
        std::cout << " | " << std::setw(10) << e.sourceId << " | " << std::setw(10) << "-" << " | " << std::setw(10) << e.requestId << std::endl;
      }
      else if (e.type == "SERVICE_COMPLETE") {
        std::cout << " | " << std::setw(10) << "-" << " | " << std::setw(10) << e.deviceId << " | " << std::setw(10) << e.requestId << std::endl;
      }
      tempQueue.pop();
    }
  }
  std::cout << "=================================" << std::endl;
}

void SimulationController::handleGenerationEvent(const Event& event) {
  Request req = event.request;
  int sourceId = event.sourceId;

  totalRequestsGenerated++;
  requestsBySource[sourceId]++;

  // Устанавливаем время поступления в буфер
  req.setTimeEnteredBuffer(currentTime);

  Request replacedReq; // Создаём объект для вытеснённой заявки
  bool accepted = dispatcher.acceptRequest(req, replacedReq);

  if (accepted) {
    // Проверяем, была ли она сразу назначена.
    AssignmentResult assignment = dispatcher.assignToDevice(currentTime);
    if (assignment.success) {
      // Планируем событие завершения для только что назначенной заявки
      Device& assignedDevice = devices[assignment.assignedDeviceId - 1];
      double serviceDuration = assignedDevice.getServiceTime();
      double serviceCompletionTime = assignment.serviceStartTime + serviceDuration;
      Event serviceCompleteEvent(serviceCompletionTime, "SERVICE_COMPLETE", assignment.assignedDeviceId, assignment.assignedRequestId);
      eventQueue.push(serviceCompleteEvent);
    }

    // Проверяем, была ли вытеснена заявка (D1004)
    if (replacedReq.getStatus() == RequestStatus::REJECTED) {
      // Обновляем статистику для вытеснённой заявки
      totalRequestsRejected++;
      rejectedBySource[replacedReq.getSourceId()]++;
    }

    double nextGenTime = sources[sourceId - 1].getNextGenerationTime(currentTime);
    Request nextRequest = sources[sourceId - 1].generateRequest(nextGenTime, nextRequestId++);
    nextRequest.setTimeEnteredBuffer(nextGenTime);
    Event nextGenEvent(nextGenTime, "GENERATION", sourceId, nextRequest.getRequestId(), nextRequest);
    eventQueue.push(nextGenEvent);
  }
  else {
    std::cout << "Ошибка: acceptRequest вернул false." << std::endl;
  }
}

void SimulationController::handleServiceCompleteEvent(const Event& event) {
  int deviceId = event.deviceId;
  int requestId = event.requestId;

  Device& device = devices[deviceId - 1];

  if (device.getIsBusy() == false || device.getCurrentRequest().getRequestId() != requestId) {
    std::cout << "Предупреждение: Прибор " << deviceId << " не обслуживает заявку " << requestId << " при попытке завершить обслуживание." << std::endl;
    return;
  }

  Request completedReq = device.getCurrentRequest();
  double serviceStartTime = device.getServiceStartTime();
  double serviceCompletionTime = currentTime;
  double serviceDuration = serviceCompletionTime - serviceStartTime;
  double totalTimeInSystemValue = currentTime - completedReq.getCreationTime();
  double waitTime = totalTimeInSystemValue - serviceDuration;
  int sourceId = completedReq.getSourceId();

  device.completeService(currentTime); // Передаём время завершения

  totalRequestsCompleted++;
  completedBySource[sourceId]++;

  // Обновляем общие суммы
  totalTimeInSystem[sourceId] += totalTimeInSystemValue;
  totalTimeWaiting[sourceId] += waitTime;
  totalTimeProcessing[sourceId] += serviceDuration;

  // Обновляем суммы квадратов отклонений для дисперсии
  // Для времени ожидания (T БП)
  int n_prev_wait = completedBySource[sourceId] - 1; // Количество до этого завершения
  double old_mean_wait = (n_prev_wait > 0) ? totalTimeWaiting[sourceId - 1] / n_prev_wait : 0.0; // Старое среднее T БП
  double new_value_wait = waitTime; // Новое значение T БП для этой заявки
  if (n_prev_wait == 0) {
    // Если это первая завершённая заявка для источника i, сумма квадратов = 0
    sumSqDiffWaitingTime[sourceId] = 0.0;
  }
  else {
    // Обновляем сумму квадратов отклонений
    double old_sum_sq_diff = sumSqDiffWaitingTime[sourceId];
    // Вычисляем новое среднее после добавления этой заявки
    double new_mean_wait = totalTimeWaiting[sourceId] / completedBySource[sourceId];
    sumSqDiffWaitingTime[sourceId] = old_sum_sq_diff + (new_value_wait - old_mean_wait) * (new_value_wait - new_mean_wait);
  }

  // Для времени обслуживания (T обсл)
  // Старое среднее T обсл
  double old_mean_proc = (n_prev_wait > 0) ? totalTimeProcessing[sourceId - 1] / n_prev_wait : 0.0;
  double new_value_proc = serviceDuration; // Новое значение T обсл для этой заявки
  if (n_prev_wait == 0) {
    // Если это первая завершённая заявка для источника i, сумма квадратов = 0
    sumSqDiffProcessingTime[sourceId] = 0.0;
  }
  else {
    // Обновляем сумму квадратов отклонений
    double old_sum_sq_diff = sumSqDiffProcessingTime[sourceId];
    // Вычисляем новое среднее после добавления этой заявки
    double new_mean_proc = totalTimeProcessing[sourceId] / completedBySource[sourceId];
    sumSqDiffProcessingTime[sourceId] = old_sum_sq_diff + (new_value_proc - old_mean_proc) * (new_value_proc - new_mean_proc);
  }


  AssignmentResult assignment = dispatcher.assignToDevice(currentTime);
  if (assignment.success) {
    Device& assignedDevice = devices[assignment.assignedDeviceId - 1];
    double serviceDuration = assignedDevice.getServiceTime();
    double serviceCompletionTime = assignment.serviceStartTime + serviceDuration;
    Event serviceCompleteEvent(serviceCompletionTime, "SERVICE_COMPLETE", assignment.assignedDeviceId, assignment.assignedRequestId);
    eventQueue.push(serviceCompleteEvent);
  }
}

// Сводная таблица (ОР1)
void SimulationController::printSummary() {
  std::cout << "\n--------------- СВОДНАЯ ТАБЛИЦА РЕЗУЛЬТАТОВ (ОР1) ---------------\n" << std::endl;

  std::cout << "Таблица 1: Характеристики источников ВС." << std::endl;
  std::cout << std::setw(10) << "№ Источника  " << std::setw(15) << "Количество заявок" << std::setw(15) << "Pотк" << std::setw(15) << "Tпреб" << std::setw(15) << "TБП" << std::setw(15) << "Tобсл" << std::setw(15) << "ДБП" << std::setw(15) << "Добсл" << std::endl;

  for (int i = 1; i <= 3; ++i) {
    double p_otk = (requestsBySource[i] > 0) ? static_cast<double>(rejectedBySource[i]) / requestsBySource[i] : 0.0;
    double t_pr = (completedBySource[i] > 0) ? totalTimeInSystem[i] / completedBySource[i] : 0.0;
    double t_bp = (completedBySource[i] > 0) ? totalTimeWaiting[i] / completedBySource[i] : 0.0;
    double t_obsl = (completedBySource[i] > 0) ? totalTimeProcessing[i] / completedBySource[i] : 0.0;
    double d_bp = (completedBySource[i] > 1) ? sumSqDiffWaitingTime[i] / (completedBySource[i] - 1) : 0.0; // Несмещенная оценка
    double d_obsl = (completedBySource[i] > 1) ? sumSqDiffProcessingTime[i] / (completedBySource[i] - 1) : 0.0; // Несмещенная оценка

    std::cout << std::setw(10) << "И" << i << std::setw(15) << requestsBySource[i] << std::setw(15) << std::fixed << std::setprecision(4) << p_otk
      << std::setw(15) << t_pr << std::setw(15) << t_bp << std::setw(15) << t_obsl
      << std::setw(15) << d_bp << std::setw(15) << d_obsl << std::endl;
  }
  std::cout << std::endl;

  std::cout << "Таблица 2: Характеристики приборов ВС." << std::endl;
  std::cout << std::setw(10) << "№ Прибора  " << std::setw(25) << "Коэффициент использования" << std::endl;

  for (int i = 1; i <= 3; ++i) {
    double k_isp = devices[i - 1].getTotalTimeBusy() / simulationEndTime;
    std::cout << std::setw(10) << "П" << i << std::setw(25) << std::fixed << std::setprecision(4) << k_isp << std::endl;
  }

  std::cout << "\n-----------------------------------------------------------------\n" << std::endl;
}