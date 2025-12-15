#include "Source.h"
#include <chrono>

Source::Source(int id, double interval, Priority pri)
  : sourceId(id), generationInterval(interval), priority(pri) {
  // генератор случайных чисел
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  generator.seed(seed + id);

  // Ќастройка равномерного распределени€ дл€ интервалов
  distribution = std::uniform_real_distribution<double>(0.0, 2.0 * generationInterval);
}

Request Source::generateRequest(double currentTime, int uniqueId) {
  Request req(uniqueId, sourceId, currentTime, priority);
  return req;
}

double Source::getNextGenerationTime(double currentTime) {
  // Cлучайное врем€ до следующей за€вки
  double nextInterval = distribution(generator);
  return currentTime + nextInterval;
}