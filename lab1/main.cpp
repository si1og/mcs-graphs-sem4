#include "cliui.h"

int main() {
    try {
        CLIUI ui;
        ui.run();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
