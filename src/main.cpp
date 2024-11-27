#include "controller/ApplicationController.h"
#include <exception>
#include <iostream>

int main() {
    try {
        ApplicationController app;
        app.start();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}