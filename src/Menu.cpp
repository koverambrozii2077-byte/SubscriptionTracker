#include "Menu.h"
#include "FileIO.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <sstream>

Menu::Menu(const std::string& dataPath) : dataFilePath(dataPath) {
    FileIO::loadFromFile(dataFilePath, manager);
}

void Menu::run() {
    int choice;
    do {
        displayMainMenu();
        choice = getIntInput("Выберите действие", 0, 8);

        switch (choice) {
        case 1: addSubscriptionMenu(); break;
        case 2: editSubscriptionMenu(); break;
        case 3: deleteSubscriptionMenu(); break;
        case 4: searchMenu(); break;
        case 5: filterMenu(); break;
        case 6: sortMenu(); break;
        case 7: analyticsMenu(); break;
        case 8:
            if (FileIO::saveToFile(dataFilePath, manager)) {
                std::cout << "Данные сохранены в " << dataFilePath << std::endl;
            }
            std::cout << "До свидания!\n";
            break;
        case 0: break;
        }
    } while (choice != 0 && choice != 8);
}

void Menu::displayMainMenu() {
    std::cout << "\n========================================\n";
    std::cout << "   СИСТЕМА УПРАВЛЕНИЯ ПОДПИСКАМИ\n";
    std::cout << "========================================\n";
    std::cout << "1. Добавить подписку\n";
    std::cout << "2. Редактировать подписку\n";
    std::cout << "3. Удалить подписку\n";
    std::cout << "4. Поиск по названию\n";
    std::cout << "5. Фильтрация\n";
    std::cout << "6. Сортировка\n";
    std::cout << "7. Аналитика\n";
    std::cout << "8. Сохранить и выйти\n";
    std::cout << "0. Выход без сохранения\n";
    std::cout << "========================================\n";
    std::cout << "Всего подписок: " << manager.getSize() << "\n";
}

void Menu::displaySubscriptions(const std::vector<Subscription>& subs) {
    if (subs.empty()) {
        std::cout << "Подписки не найдены.\n";
        return;
    }

    std::cout << "\n--------------------------------------------------------------------------------------------------------\n";
    std::cout << std::left << std::setw(5) << "ID"
        << std::setw(20) << "Название"
        << std::setw(15) << "Категория"
        << std::setw(12) << "Стоимость/мес"
        << std::setw(12) << "Цикл"
        << std::setw(15) << "Дата платежа"
        << std::setw(10) << "Статус" << "\n";
    std::cout << "--------------------------------------------------------------------------------------------------------\n";

    for (const auto& sub : subs) {
        std::cout << std::left << std::setw(5) << sub.getId()
            << std::setw(20) << sub.getServiceName()
            << std::setw(15) << sub.getCategoryString()
            << std::setw(12) << std::fixed << std::setprecision(2) << sub.getEffectiveMonthlyCost()
            << std::setw(12) << sub.getBillingCycleString()
            << std::setw(15) << sub.getNextPaymentDateString()
            << std::setw(10) << (sub.isActiveSubscription() ? "Активна" : "Неактивна") << "\n";
    }
    std::cout << "--------------------------------------------------------------------------------------------------------\n";
}

void Menu::addSubscriptionMenu() {
    std::cout << "\n--- ДОБАВЛЕНИЕ ПОДПИСКИ ---\n";

    std::string name = getInput("Название сервиса");
    if (name.empty()) {
        std::cout << "Ошибка: название не может быть пустым.\n";
        return;
    }

    std::cout << "Категория:\n";
    std::cout << "1. Развлечения\n2. Софт\n3. Обучение\n4. Коммунальные\n5. Другое\n";
    int catChoice = getIntInput("Выберите категорию", 1, 5);
    Category category = static_cast<Category>(catChoice - 1);

    double cost = getDoubleInput("Стоимость (в рублях за период)", 0.01);

    std::cout << "Цикл оплаты:\n";
    std::cout << "1. Ежемесячно\n2. Ежегодно\n3. Еженедельно\n4. Ежеквартально\n";
    int cycleChoice = getIntInput("Выберите цикл", 1, 4);
    BillingCycle cycle = static_cast<BillingCycle>(cycleChoice - 1);

    std::string dateStr = getInput("Дата следующего платежа (ГГГГ-ММ-ДД)");
    std::time_t nextDate = parseDate(dateStr);
    if (nextDate == -1) {
        std::cout << "Ошибка: неверный формат даты.\n";
        return;
    }

    Subscription newSub(manager.getNextId(), name, category, cost, cycle, nextDate, true);
    if (manager.addSubscription(newSub)) {
        manager.incrementNextId();
        std::cout << "Подписка успешно добавлена с ID: " << newSub.getId() << "\n";
    }
    else {
        std::cout << "Ошибка при добавлении подписки.\n";
    }
}

void Menu::editSubscriptionMenu() {
    std::cout << "\n--- РЕДАКТИРОВАНИЕ ПОДПИСКИ ---\n";
    int id = getIntInput("Введите ID подписки для редактирования", 1, 999999);

    Subscription* sub = manager.findSubscriptionById(id);
    if (!sub) {
        std::cout << "Подписка с ID " << id << " не найдена.\n";
        return;
    }

    std::cout << "Текущая информация:\n";
    displaySubscriptions({ *sub });

    std::string name = getInput("Новое название (оставьте пустым для сохранения)");
    if (!name.empty()) sub->setServiceName(name);

    std::cout << "Новая категория (0-пропустить, 1-Развлечения,2-Софт,3-Обучение,4-Коммунальные,5-Другое): ";
    int catChoice = getIntInput("", 0, 5);
    if (catChoice > 0) sub->setCategory(static_cast<Category>(catChoice - 1));

    double cost = getDoubleInput("Новая стоимость (0 для пропуска)", 0);
    if (cost > 0) sub->setMonthlyCost(cost);

    std::cout << "Новый цикл оплаты (0-пропустить,1-месяц,2-год,3-неделя,4-квартал): ";
    int cycleChoice = getIntInput("", 0, 4);
    if (cycleChoice > 0) sub->setBillingCycle(static_cast<BillingCycle>(cycleChoice - 1));

    std::string dateStr = getInput("Новая дата платежа (ГГГГ-ММ-ДД) (оставьте пустым для пропуска)");
    if (!dateStr.empty()) {
        std::time_t newDate = parseDate(dateStr);
        if (newDate != -1) sub->setNextPaymentDate(newDate);
    }

    std::cout << "Изменить статус? (1-Активна, 2-Неактивна, 0-Пропустить): ";
    int statusChoice = getIntInput("", 0, 2);
    if (statusChoice == 1) sub->setActive(true);
    else if (statusChoice == 2) sub->setActive(false);

    std::cout << "Подписка обновлена.\n";
}

void Menu::deleteSubscriptionMenu() {
    std::cout << "\n--- УДАЛЕНИЕ ПОДПИСКИ ---\n";
    std::cout << "1. Удалить по ID\n";
    std::cout << "2. Удалить по названию\n";
    int choice = getIntInput("Выберите способ", 1, 2);

    if (choice == 1) {
        int id = getIntInput("Введите ID", 1, 999999);
        if (manager.deleteSubscriptionById(id)) {
            std::cout << "Подписка удалена.\n";
        }
        else {
            std::cout << "Подписка не найдена.\n";
        }
    }
    else {
        std::string name = getInput("Введите название сервиса");
        if (manager.deleteSubscriptionByName(name)) {
            std::cout << "Подписка(и) удалена.\n";
        }
        else {
            std::cout << "Подписка не найдена.\n";
        }
    }
}

void Menu::searchMenu() {
    std::cout << "\n--- ПОИСК ПО НАЗВАНИЮ ---\n";
    std::string query = getInput("Введите название для поиска");
    auto results = manager.searchByName(query);
    displaySubscriptions(results);
}

void Menu::filterMenu() {
    std::cout << "\n--- ФИЛЬТРАЦИЯ ---\n";
    std::cout << "1. По категории\n";
    std::cout << "2. По диапазону стоимости\n";
    std::cout << "3. По диапазону дат платежа\n";
    int choice = getIntInput("Выберите тип фильтра", 1, 3);

    std::vector<Subscription> results;

    switch (choice) {
    case 1: {
        std::cout << "1. Развлечения\n2. Софт\n3. Обучение\n4. Коммунальные\n5. Другое\n";
        int catChoice = getIntInput("Выберите категорию", 1, 5);
        Category category = static_cast<Category>(catChoice - 1);
        results = manager.filterByCategory(category);
        break;
    }
    case 2: {
        double min = getDoubleInput("Минимальная стоимость", 0);
        double max = getDoubleInput("Максимальная стоимость", min);
        results = manager.filterByCostRange(min, max);
        break;
    }
    case 3: {
        std::string startStr = getInput("Начальная дата (ГГГГ-ММ-ДД)");
        std::string endStr = getInput("Конечная дата (ГГГГ-ММ-ДД)");
        std::time_t start = parseDate(startStr);
        std::time_t end = parseDate(endStr);
        if (start != -1 && end != -1) {
            results = manager.filterByDateRange(start, end);
        }
        break;
    }
    }

    displaySubscriptions(results);
}

void Menu::sortMenu() {
    std::cout << "\n--- СОРТИРОВКА ---\n";
    std::cout << "1. По стоимости (возрастание)\n";
    std::cout << "2. По стоимости (убывание)\n";
    std::cout << "3. По дате платежа\n";
    std::cout << "4. По названию\n";
    int choice = getIntInput("Выберите тип сортировки", 1, 4);

    switch (choice) {
    case 1: manager.sortByMonthlyCost(true); break;
    case 2: manager.sortByMonthlyCost(false); break;
    case 3: manager.sortByNextPaymentDate(); break;
    case 4: manager.sortByServiceName(); break;
    }

    std::cout << "Сортировка применена.\n";
    displaySubscriptions(manager.getAllSubscriptions());
}

void Menu::analyticsMenu() {
    std::cout << "\n--- АНАЛИТИКА ---\n";

    double total = manager.getTotalMonthlyExpenses();
    std::cout << "Суммарные ежемесячные расходы: " << std::fixed << std::setprecision(2) << total << " руб.\n";

    std::cout << "\nПодписки с оплатой в ближайшие 7 дней:\n";
    auto upcoming = manager.getUpcomingPayments(7);
    displaySubscriptions(upcoming);

    std::cout << "\nТоп-3 категории по суммарной стоимости:\n";
    auto topCategories = manager.getTopCategoriesByCost(3);
    int rank = 1;
    for (auto it = topCategories.begin(); it != topCategories.end(); ++it) {
        std::cout << rank++ << ". " << it->first << ": " << std::fixed << std::setprecision(2) << it->second << " руб.\n";
    }
}

std::time_t Menu::parseDate(const std::string& dateStr) {
    std::tm tm = {};
    std::stringstream ss(dateStr);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    if (ss.fail()) return -1;
    return std::mktime(&tm);
}

std::string Menu::getInput(const std::string& prompt) {
    std::cout << prompt << ": ";
    std::string input;
    std::getline(std::cin, input);
    return input;
}

int Menu::getIntInput(const std::string& prompt, int min, int max) {
    int value;
    while (true) {
        std::cout << prompt << " (" << min << "-" << max << "): ";
        std::cin >> value;
        if (std::cin.fail() || value < min || value > max) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Ошибка: введите число от " << min << " до " << max << "\n";
        }
        else {
            std::cin.ignore();
            return value;
        }
    }
}

double Menu::getDoubleInput(const std::string& prompt, double min) {
    double value;
    while (true) {
        std::cout << prompt << " (мин. " << min << "): ";
        std::cin >> value;
        if (std::cin.fail() || value < min) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Ошибка: введите число >= " << min << "\n";
        }
        else {
            std::cin.ignore();
            return value;
        }
    }
}