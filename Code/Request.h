#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <iostream>

enum class Priority {
  PRIVATE,       // Частный (низший)
  CORPORATE,    // Корпоративный (средний)
  WARRANTY     // Гарантийный (высший)
};

enum class RequestStatus {
  NEW,          // Новая
  IN_BUFFER,    // В буфере
  PROCESSING,   // Обрабатывается
  COMPLETED,    // Завершена
  REJECTED      // Отклонена
};

class Request {
private:
  int requestId;        // Уникальный ID заявки
  int sourceId;         // ID источника, который сгенерировал заявку
  double creationTime;  // Время создания заявки
  double timeEnteredBuffer; // Время, когда заявка поступила в буфер
  Priority priority;    // Приоритет заявки
  RequestStatus status; // Cтатус заявки
  std::string description; // Описание

public:
  Request();

  Request(int reqId, int srcId, double time, Priority pri);

  int getRequestId() const { return requestId; }
  int getSourceId() const { return sourceId; }
  double getCreationTime() const { return creationTime; }
  double getTimeEnteredBuffer() const { return timeEnteredBuffer; }
  Priority getPriority() const { return priority; }
  RequestStatus getStatus() const { return status; }
  std::string getDescription() const { return description; }

  // Метод для получения строки с ID заявки
  std::string getIdString() const;

  // Метод для обновления статуса
  void updateStatus(RequestStatus newStatus) { status = newStatus; }

  // Метод для обновления времени поступления в буфер
  void setTimeEnteredBuffer(double time) { timeEnteredBuffer = time; }

  // Метод для получения строки с описанием приоритета
  static std::string priorityToString(Priority p);
};

#endif