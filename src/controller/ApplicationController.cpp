#include "ApplicationController.h"

ApplicationController::ApplicationController() {
    updatePrompt();
}

void ApplicationController::start() {
    while (gui.isWindowOpen()) {
        processEvents();
        gui.update();
        gui.render();
    }
}

void ApplicationController::processEvents() {
    sf::Event event;
    while (gui.getWindow().pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            gui.getWindow().close();
        }
        else {
            gui.handleEvent(event);
            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::Return) {
                std::string input = gui.getCurrentInput();
                if (!input.empty()) {
                    if (event.key.shift) {
                        shell.appendInput(input);
						shell.setMultiLine(true);
						gui.addOutputLine(gui.getPrompt() + input);
                        gui.clearInput();
                    }
                    else {
                        handleInput(input);
						shell.setMultiLine(false);
                        gui.clearInput();
                    }
                }
                updatePrompt();
            }
        }
    }
}

void ApplicationController::handleInput(const std::string& input) {
    if (input.empty()) {
        return;
    }

    shell.appendInput(input);

    // show the final line that triggered evaluation
    gui.addOutputLine(gui.getPrompt() + input);

    try {
        // evaluate and show result
        std::string result = shell.executeBuffer();
        if (!result.empty()) {
            gui.addOutputLine("=> " + result);
        }
    }
    catch (const std::exception& e) {
        gui.addOutputLine(std::string("Error: ") + e.what());
        shell.clearBuffer();
    }
}

void ApplicationController::updatePrompt() {
    gui.setPrompt(shell.isInMultiLine() ? "... " : ">>> ");
}