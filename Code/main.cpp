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
  std::signal(SIGINT, signalHandler);

  cout << "Выберите режим работы симуляции:" << endl;
  cout << "1. Пошаговый режим (ОД1)" << endl;
  cout << "2. Автоматический режим (ОР1)" << endl;
  cout << "Введите 1 или 2: ";

  int mode_choice;
  cin >> mode_choice;
  cin.ignore();

  SimulationController simController;

  if (mode_choice == 1) {
    cout << "\nЗапуск пошаговой симуляции..." << endl;
    simController.runSimulationStepByStep();
  }
  else if (mode_choice == 2) {
    cout << "\nЗапуск автоматической симуляции..." << endl;
    simController.runSimulationAutomatic();
  }
  else {
    cout << "Неверный выбор. Запускаю пошаговый режим по умолчанию." << endl;
    simController.runSimulationStepByStep();
  }

  // Проверяем, был ли вызван сигнал завершения
  if (g_signalRaised == SIGINT && mode_choice == 1) {
    std::cout << "\nПолучен сигнал SIGINT (Ctrl+C). Завершение программы." << std::endl;
    return 0;
  }

  return 0;
}