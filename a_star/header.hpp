#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

class Box {
public:
	bool isStart = false;
	bool isEnd = false;
	bool isBarrier = false;
	bool isOpen = false;
	bool isClosed = false;
	float g = INFINITY;
	int f = 0;
	int h = 0;
	int row, col;
	sf::Color color = sf::Color::White;
	std::vector<Box*> neighbors;
	Box* previous = nullptr;

	void makeStart();
	void makeEnd();
	void makeBarrier();
	void makeOpen();
	void makeClosed();

	void updateNeighbors(std::vector<std::vector<Box>>& boxes, const int& max_row_col);
	void reset();
	void draw(sf::RenderWindow& window, const unsigned int& WINDOW_WIDTH_HEIGHT, const int& NUM_BOXES) const;

	int getG() { return g; }
};


class myComparator {
public:
	bool operator() (Box* b1, Box* b2) {
		if (b1->f == b2->f) return b1->h > b2->h;
		return b1->f > b2->f;
	}
};


void Box::makeStart() {
	color = sf::Color::Yellow;
}


void Box::makeEnd() {
	color = sf::Color::Blue;
}


void Box::makeBarrier() {
	color = sf::Color::Black;
}


void Box::makeOpen() {
	if (!isStart && !isEnd) color = sf::Color::Green;
	isOpen = true;
}


void Box::makeClosed() {
	if (!isStart && !isEnd) color = sf::Color::Red;
	isOpen = false;
	isClosed = true;
}


void Box::reset() {
	color = sf::Color::White;
	isStart = false;
	isEnd = false;
	isBarrier = false;
	isOpen = false;
	isClosed = false;
	neighbors.clear();
}


void Box::updateNeighbors(std::vector<std::vector<Box>>& boxes, const int& max_row_col) {
	neighbors.clear();

	// check the right box
	if (col < max_row_col - 1 && !boxes[row][col + 1].isBarrier) {
		neighbors.push_back(&boxes[row][col + 1]);
	}
	// check the left box
	if (col > 0 && !boxes[row][col - 1].isBarrier) {
		neighbors.push_back(&boxes[row][col - 1]);
	}
	// check the top box
	if (row > 0 && !boxes[row - 1][col].isBarrier) {
		neighbors.push_back(&boxes[row - 1][col]);
	}
	//check the bottom box
	if (row < max_row_col - 1 && !boxes[row + 1][col].isBarrier) {
		neighbors.push_back(&boxes[row + 1][col]);
	}
}


void Box::draw (sf::RenderWindow& window, const unsigned int& WINDOW_WIDTH_HEIGHT, const int& NUM_BOXES) const {
	int size_int = WINDOW_WIDTH_HEIGHT / sqrt(NUM_BOXES);
	float size = static_cast<float>(size_int);

	const int GAP = WINDOW_WIDTH_HEIGHT / sqrt(NUM_BOXES);

	sf::RectangleShape box;
	box.setFillColor(color);
	box.setSize(sf::Vector2f(size, size));
	sf::Vector2f pos;
	pos.x = col * GAP;
	pos.y = row * GAP;
	box.setPosition(pos);
	window.draw(box);
}
