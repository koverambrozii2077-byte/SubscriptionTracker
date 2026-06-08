#include "FileIO.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <filesystem>

bool FileIO::loadFromFile(const std::string& filename, SubscriptionManager& manager) {
    std::filesystem::path filePath(filename);
    std::filesystem::path dirPath = filePath.parent_path();

    if (!dirPath.empty() && !std::filesystem::exists(dirPath)) {
        std::filesystem::create_directories(dirPath);
        std::cout << "Создана папка: " << dirPath << std::endl;
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Файл не найден: " << filename << std::endl;
        std::cerr << "Создаю новый файл при сохранении..." << std::endl;
        return false;
    }

    manager.clear();
    std::string line;
    int maxId = 0;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string idStr, serviceName, categoryStr, monthlyCostStr, billingCycleStr, nextPaymentDateStr, statusStr;

        std::getline(ss, idStr, ',');
        std::getline(ss, serviceName, ',');
        std::getline(ss, categoryStr, ',');
        std::getline(ss, monthlyCostStr, ',');
        std::getline(ss, billingCycleStr, ',');
        std::getline(ss, nextPaymentDateStr, ',');
        std::getline(ss, statusStr, ',');

        try {
            int id = std::stoi(idStr);
            double monthlyCost = std::stod(monthlyCostStr);
            bool isActive = (statusStr == "active");

            std::tm tm = {};
            std::istringstream dateStream(nextPaymentDateStr);
            dateStream >> std::get_time(&tm, "%Y-%m-%d");

            if (dateStream.fail()) {
                std::cerr << "Ошибка парсинга даты: " << nextPaymentDateStr << std::endl;
                continue;
            }

            std::time_t nextPaymentDate = std::mktime(&tm);

            Category category = Subscription::stringToCategory(categoryStr);
            BillingCycle billingCycle = Subscription::stringToBillingCycle(billingCycleStr);

            Subscription sub(id, serviceName, category, monthlyCost, billingCycle, nextPaymentDate, isActive);
            manager.addSubscription(sub);

            if (id > maxId) maxId = id;
        }
        catch (const std::exception& e) {
            std::cerr << "Ошибка при чтении строки: " << line << " - " << e.what() << std::endl;
        }
    }

    file.close();

    if (maxId > 0) {
        manager.setNextId(maxId + 1);
    }

    std::cout << "Загружено " << manager.getSize() << " подписок из файла" << std::endl;
    return true;
}

bool FileIO::saveToFile(const std::string& filename, const SubscriptionManager& manager) {
    std::filesystem::path filePath(filename);
    std::filesystem::path dirPath = filePath.parent_path();

    if (!dirPath.empty() && !std::filesystem::exists(dirPath)) {
        std::filesystem::create_directories(dirPath);
        std::cout << "Создана папка: " << dirPath << std::endl;
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка: Не удалось открыть файл для записи " << filename << std::endl;
        return false;
    }

    for (const auto& sub : manager.getAllSubscriptions()) {
        file << sub.getId() << ","
            << sub.getServiceName() << ","
            << sub.getCategoryString() << ","
            << sub.getMonthlyCost() << ","
            << sub.getBillingCycleString() << ","
            << sub.getNextPaymentDateString() << ","
            << (sub.isActiveSubscription() ? "active" : "inactive") << "\n";
    }

    file.close();
    std::cout << "Сохранено " << manager.getAllSubscriptions().size() << " подписок в " << filename << std::endl;
    return true;
}