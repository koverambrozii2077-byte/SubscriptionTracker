#include "Menu.h"
#include <iostream>
#include <locale>
#include <clocale>

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
    #ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    #endif
    
    setlocale(LC_ALL, "ru_RU.UTF-8");
    
    std::cout << "Загрузка системы управления подписками...\n";
    
    Menu menu("data/subscriptions.txt");
    menu.run();
    
    return 0;
}