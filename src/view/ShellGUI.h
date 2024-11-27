#pragma once
#include <SFML/Graphics.hpp>
#include <deque>
#include <string>

class ShellGUI {
public:
	ShellGUI();
	void update();
	void render();
	void handleEvent(const sf::Event& event);
	void addOutputLine(const std::string& line);
	void setPrompt(const std::string& prompt);
	std::string getPrompt();
	void clearInput();
	std::string getCurrentInput() const;
	bool isWindowOpen() const;
	sf::RenderWindow& getWindow();
	void scrollUp();
	void scrollDown();
private:
	sf::RenderWindow window;
	sf::Font font;
	sf::Text inputText;
	sf::RectangleShape cursor;
	sf::RectangleShape statusBar;

	std::deque<std::string> outputHistory;
	std::string currentInput;
	std::string prompt;
	size_t cursorPos;
	float lineHeight;
	float charWidth;
	int scrollOffset;

	void updateCursorPosition();
	void drawOutput();
	void drawInput();
	void handleTextInput(sf::Uint32 unicode);
	void handleSpecialKeys(const sf::Event::KeyEvent& key);
};