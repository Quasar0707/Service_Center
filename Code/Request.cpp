#include "Request.h"
#include <sstream>

Request::Request()
  : requestId(0), sourceId(0), creationTime(0.0), timeEnteredBuffer(0.0), priority(Priority::PRIVATE), status(RequestStatus::NEW) {
  description = "Заявка по умолчанию";
}

Request::Request(int reqId, int srcId, double time, Priority pri)
  : requestId(reqId), sourceId(srcId), creationTime(time), timeEnteredBuffer(0.0),
  priority(pri), status(RequestStatus::NEW) {
  std::ostringstream oss;
  oss << "Заявка #" << requestId << " от Источника " << srcId
    << " (Приоритет: " << priorityToString(priority) << ")";
  description = oss.str();
}

std::string Request::getIdString() const {
  std::ostringstream oss;
  oss << "#" << requestId;
  return oss.str();
}

std::string Request::priorityToString(Priority p) {
  switch (p) {
  case Priority::WARRANTY: return "Гарантийный";
  case Priority::CORPORATE: return "Корпоративный";
  case Priority::PRIVATE: return "Частный";
  default: return "Неизвестный";
  }
}