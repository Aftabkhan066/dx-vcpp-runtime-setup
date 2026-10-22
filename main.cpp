#include <iostream>
#include <fstream>
#include <vector>
#include <string>

int main() {
    std::ifstream file("config.json");
    if (!file.is_open()) {
        std::cerr << "Failed to open config" << std::endl;
        return 1;
    }
    std::cout << "Config loaded" << std::endl;
    return 0;
}
