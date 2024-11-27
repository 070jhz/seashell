#pragma once

#include "../view/ShellGUI.h"
#include "ShellController.h"
#include <memory>

class ApplicationController {
public:
    ApplicationController();
    void start();

private:
    ShellGUI gui;
    ShellController shell;

    void handleInput(const std::string& input);
    void updatePrompt();
    void processEvents();
};