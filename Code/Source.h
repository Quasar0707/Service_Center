#ifndef SOURCE_H
#define SOURCE_H

#include <random>
#include "Request.h"

class Source {
private:
  int sourceId;
  double generationInterval; // Среднее время между генерацией заявок
  Priority priority;      // Приоритет заявок
  std::default_random_engine generator; // Генератор случайных чисел
  std::uniform_real_distribution<double> distribution; // Равномерное распределение

public:
  Source(int id, double interval, Priority pri);

  // Метод для генерации новой заявки
  Request generateRequest(double currentTime, int uniqueId);

  // Метод для получения времени следующей генерации
  double getNextGenerationTime(double currentTime);

  int getSourceId() const { return sourceId; }
  Priority getPriority() const { return priority; }
};

#endif