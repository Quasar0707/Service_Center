#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include "Request.h"
#include <optional>

class Buffer {
private:
  int capacity;                       // Емкость буфера
  std::vector<Request> slots;         // Массив слотов для хранения заявок
  std::vector<bool> occupied;         // Массив для отслеживания занятости слотов
  int ringPointer;                    // Указатель для кольцевого буфера

public:
  Buffer(int cap);

  // Метод для добавления заявки в буфер по кольцу  D1031
  bool addRequest(const Request& req, Request& replacedReq);

  // Метод для удаления заявки по индексу  D2Б4
  void markSlotFree(int index);

  // Метод для поиска следующего свободного слота  D1031
  int findNextFreeSlot();

  // Метод для поиска индекса заявки, которая будет вытеснена D1004
  int findRequestForReplacement();

  bool isFull() const;
  bool isEmpty() const;

  int getCapacity() const { return capacity; }
  int getCurrentSize() const;
  const std::vector<Request>& getSlots() const { return slots; }
  const std::vector<bool>& getOccupancy() const { return occupied; }

  int getRingPointer() const { return ringPointer; }
};

#endif