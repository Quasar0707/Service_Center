#include <iostream>
#include <vector>
#include <queue>
#include <random>
#include <algorithm>
#include <iomanip>
#include <csignal>
#include <cstdlib>

#include "SimulationController.h"

// Глобальная переменная для флага завершения
volatile sig_atomic_t g_signalRaised = 0;

void signalHandler(int signal) {
  g_signalRaised = signal;
}

using namespace std;

int main() {
  setlocale(LC_ALL, "rus");

  // Регистрируем обработчик сигнала SIGINT (Ctrl+C)
  std::signal(SIGINT, signalHandler);

  // Создаем контроллер симуляции
  SimulationController simController;

  // Запускаем симуляцию
  simController.runSimulation();

  // Проверяем, был ли вызван сигнал завершения
  if (g_signalRaised == SIGINT) {
    std::cout << "\nПолучен сигнал SIGINT (Ctrl+C). Завершение программы." << std::endl;
    return 0;
  }

  return 0;
}