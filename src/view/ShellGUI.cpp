#include "ShellGUI.h"
#include <iostream>

ShellGUI::ShellGUI() : window(sf::VideoMode(800, 600), "SeaShell", sf::Style::Default), scrollOffset(0) {
	if (!font.loadFromFile("resources/FiraCode.ttf")) {
		throw std::runtime_error("failed to load font");
	}
	window.setVerticalSyncEnabled(true);

	inputText.setFont(font);
	inputText.setCharacterSize(14);
	inputText.setFillColor(sf::Color::White);

	cursor.setSize(sf::Vector2f(2, 16));
	cursor.setFillColor(sf::Color::White);

	statusBar.setSize(sf::Vector2f(800, 24));
	statusBar.setFillColor(sf::Color(50, 50, 50));
	statusBar.setPosition(0, window.getSize().y - 24);

	lineHeight = 20.f;
	charWidth = 8.f;
	cursorPos = 0;
	prompt = ">>> ";
}

void ShellGUI::update() {
	updateCursorPosition();
}

void ShellGUI::render() {
	window.clear(sf::Color::Black);

	drawOutput();
	drawInput();

	window.draw(statusBar);

	window.display();
}

void ShellGUI::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::TextEntered) {
        // only handle actual text input (not control characters)
        if (event.text.unicode >= 32 && event.text.unicode < 128) {
            handleTextInput(event.text.unicode);
        }
    }
    else if (event.type == sf::Event::KeyPressed) {
        handleSpecialKeys(event.key);
    }
    else if (event.type == sf::Event::MouseWheelScrolled) {
        if (event.mouseWheelScroll.delta > 0) {
            scrollDown();
        } else {
            scrollUp();
        }
    }
}


void ShellGUI::addOutputLine(const std::string& line) {
    outputHistory.push_back(line);
    if (outputHistory.size() > 1000) {
        outputHistory.pop_front();
        if (scrollOffset > 0) scrollOffset--;
    }

    float availableHeight = window.getSize().y - lineHeight - 24;
    int visibleLines = static_cast<int>(availableHeight / lineHeight);
    int maxScrollOffset = std::max(0, static_cast<int>(outputHistory.size()) - visibleLines);
    
    // If scrollOffset is near bottom (within 3 lines), stick to bottom
    if (scrollOffset >= maxScrollOffset - 3) {
        scrollOffset = maxScrollOffset;
    }
}

void ShellGUI::setPrompt(const std::string& newPrompt) {
	prompt = newPrompt;
}

std::string ShellGUI::getCurrentInput() const {
	return currentInput;
}

std::string ShellGUI::getPrompt() {
	return prompt;
}

bool ShellGUI::isWindowOpen() const {
	return window.isOpen();
}

sf::RenderWindow& ShellGUI::getWindow() {
	return window;
}

void ShellGUI::updateCursorPosition() {
	float xPos = (prompt.length() + cursorPos) * charWidth;
	float yPos = window.getSize().y - lineHeight - 24;
	cursor.setPosition(xPos, yPos);
}

void ShellGUI::drawOutput() {
    float yPos = 0;
    float availableHeight = window.getSize().y - lineHeight - 24;
    int visibleLines = static_cast<int>(availableHeight / lineHeight);
    
    // If we have more lines than can fit, adjust scroll offset to show latest content
    if (scrollOffset == 0 && outputHistory.size() > visibleLines) {
        scrollOffset = outputHistory.size() - visibleLines;
    }
    
    // Calculate start index based on scroll offset
    int startIndex = scrollOffset;
    int endIndex = std::min(startIndex + visibleLines, static_cast<int>(outputHistory.size()));
    
    // Draw from top to bottom
    for (int i = startIndex; i < endIndex; ++i) {
        sf::Text text(outputHistory[i], font, 14);
        text.setFillColor(sf::Color::White);
        text.setPosition(0, yPos);
        window.draw(text);
        yPos += lineHeight;
    }
}

void ShellGUI::drawInput() {
	sf::Text promptText(prompt + currentInput, font, 14);
	promptText.setFillColor(sf::Color::White);
	promptText.setPosition(0, window.getSize().y - lineHeight - 24);
	window.draw(promptText);
	window.draw(cursor);
}

void ShellGUI::handleTextInput(sf::Uint32 unicode) {
	currentInput.insert(cursorPos, 1, static_cast<char>(unicode));
	cursorPos++;
	updateCursorPosition();
}

void ShellGUI::handleSpecialKeys(const sf::Event::KeyEvent& key) {
    if (key.code == sf::Keyboard::BackSpace && cursorPos > 0) {
        currentInput.erase(cursorPos - 1, 1);
        cursorPos--;
        updateCursorPosition();
    }
	else if (key.code == sf::Keyboard::Left && cursorPos > 0) {
		cursorPos--;
		updateCursorPosition();
	}
	else if (key.code == sf::Keyboard::Right && cursorPos < currentInput.length()) {
		cursorPos++;
		updateCursorPosition();
	}
}

void ShellGUI::clearInput() {
	currentInput.clear();
	cursorPos = 0;
	updateCursorPosition();
}

void ShellGUI::scrollDown() {
	if (scrollOffset > 0) {
		scrollOffset--;
	}
}

void ShellGUI::scrollUp() {
    float availableHeight = window.getSize().y - lineHeight - 24;
    int visibleLines = static_cast<int>(availableHeight / lineHeight);
    int maxScrollOffset = std::max(0, static_cast<int>(outputHistory.size()) - visibleLines);
    
    // Only increment scroll offset if we haven't reached the max
    if (scrollOffset < maxScrollOffset) {
        scrollOffset++;
    }
}