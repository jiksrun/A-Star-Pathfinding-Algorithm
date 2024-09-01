#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <queue>
#include "header.hpp"

unsigned int WINDOW_WIDTH_HEIGHT = 900; // an unsigned is an integer that can never be negative.
const int NUM_BOXES = 50*50;
bool gotStart = false, gotEnd = false;
std::vector<sf::RectangleShape> lines;
bool solved = false;

void makeGrid(std::vector<sf::RectangleShape>& lines);
void drawGrid(sf::RenderWindow& window, const std::vector<sf::RectangleShape>& lines);
std::vector<std::vector<Box>> makeBoxes();
void drawBoxes(sf::RenderWindow& window, const std::vector<std::vector<Box>>& boxes);
void getRowCol(const sf::Vector2i& pos, int& row, int& col);
void isMousePressed(sf::RenderWindow& window, std::vector<std::vector<Box>>& boxes);
void reset(std::vector<std::vector<Box>>& boxes);
int h(Box*& current, Box*& end);
void findEnd(std::vector<std::vector<Box>>& boxes, Box*& end);
void findStart(std::vector<std::vector<Box>>& boxes, Box*& start);
void reconstruct(sf::RenderWindow& window, Box* start, Box* end);
void algorithm(sf::RenderWindow& window, std::vector<std::vector<Box>>& boxes);
void isKeyPressed(sf::RenderWindow& window, std::vector<std::vector<Box>>& boxes);

int main() {
	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH_HEIGHT, WINDOW_WIDTH_HEIGHT), "A* Pathfinding Algorithm");
	
	makeGrid(lines);
	std::vector<std::vector<Box>> boxes = makeBoxes();

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) window.close();
		}
		window.clear(sf::Color::White);
		drawBoxes(window, boxes);
		drawGrid(window, lines);
		isMousePressed(window, boxes);
		isKeyPressed(window, boxes);
		window.display();
	}

	return 0;
}


void makeGrid(std::vector<sf::RectangleShape>& lines) {
	const int BOXES_ROW_COL = sqrt(NUM_BOXES);
	const int GAP = WINDOW_WIDTH_HEIGHT / BOXES_ROW_COL;
	sf::RectangleShape line;
	
	// horizontal line
	for (int i = 0; i < BOXES_ROW_COL; ++i) {
		line.setSize(sf::Vector2f(WINDOW_WIDTH_HEIGHT, 1));
		line.setPosition(sf::Vector2f(0, static_cast<float>(i*GAP)));
		line.setFillColor(sf::Color::Black);
		lines.push_back(line);
	}

	// vertical line
	for (int i = 0; i < BOXES_ROW_COL; ++i) {
		line.setSize(sf::Vector2f(1, WINDOW_WIDTH_HEIGHT));
		line.setPosition(sf::Vector2f(static_cast<float>(i * GAP), 0));
		line.setFillColor(sf::Color::Black);
		lines.push_back(line);
	}
	lines.shrink_to_fit();
}


void drawGrid(sf::RenderWindow& window, const std::vector<sf::RectangleShape>& lines) {
	for (auto& line : lines) {
		window.draw(line);
	}
}


std::vector<std::vector<Box>> makeBoxes() {
	Box box;
	std::vector<Box> boxes_col;
	std::vector<std::vector<Box>> boxes_row_col;
	int num_row_col = sqrt(NUM_BOXES);

	for (int i = 0; i < num_row_col; ++i) {
		boxes_col.push_back(box);
	}
	for (int i = 0; i < num_row_col; ++i) {
		boxes_row_col.push_back(boxes_col);
	}
	for (int i = 0; i < num_row_col; ++i) {
		for (int j = 0; j < num_row_col; ++j) {
			boxes_row_col[i][j].row = i;
			boxes_row_col[i][j].col = j;
		}
	}
	return boxes_row_col;
}


void drawBoxes(sf::RenderWindow& window, const std::vector<std::vector<Box>>& boxes) {
	for (auto& box_row : boxes) {
		for (auto& box_row_col : box_row) {
			if (box_row_col.color != sf::Color::White) box_row_col.draw(window, WINDOW_WIDTH_HEIGHT, NUM_BOXES);
		}
	}
}


void getRowCol(const sf::Vector2i& pos, int& row, int& col) {
	// initialize the gap
	const int GAP = WINDOW_WIDTH_HEIGHT / sqrt(NUM_BOXES);

	// get the position
	row = pos.y;
	col = pos.x;

	// fix the position based on the gap
	row = row / GAP;
	col = col / GAP;
}


void isMousePressed(sf::RenderWindow& window, std::vector<std::vector<Box>>& boxes) {
	sf::Vector2i pos;
	int row, col;

	if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
		pos = sf::Mouse::getPosition(window);
		getRowCol(pos, row, col);

		if (row < 0 || col < 0 || row > sqrt(NUM_BOXES)-1 || col > sqrt(NUM_BOXES)-1) return;

		if (!gotStart && !boxes[row][col].isEnd) {
			boxes[row][col].makeStart();
			boxes[row][col].isBarrier = false;
			boxes[row][col].isStart = true;
			gotStart = true;
		}
		else if (!gotEnd && !boxes[row][col].isStart) {
			boxes[row][col].makeEnd();
			boxes[row][col].isBarrier = false;
			boxes[row][col].isEnd = true;
			gotEnd = true;
		}
		else if (!boxes[row][col].isStart && !boxes[row][col].isEnd) {
			boxes[row][col].makeBarrier();
			boxes[row][col].isBarrier = true;
		}
	}
	if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
		pos = sf::Mouse::getPosition(window);
		getRowCol(pos, row, col);

		if (row < 0 || col < 0 || row > sqrt(NUM_BOXES) - 1 || col > sqrt(NUM_BOXES) - 1) return;

		if (boxes[row][col].isStart) {
			gotStart = false;
		}
		else if (boxes[row][col].isEnd) {
			gotEnd = false;
		}
		boxes[row][col].reset();
	}
}



void reset(std::vector<std::vector<Box>>& boxes) {
	for (auto& box_row : boxes) {
		for (auto& box_row_col : box_row) {
			box_row_col.reset();
		}
	}
	gotStart = false;
	gotEnd = false;
	solved = false;
}


int h(Box*& current, Box*& end) {
	return abs(end->row - current->row) + abs(end->col - current->col);
}


void findEnd(std::vector<std::vector<Box>>& boxes, Box*& end) {
	for (auto& box_row : boxes) {
		for (auto& box_row_col : box_row) {
			if (box_row_col.isEnd) {
				end = &box_row_col;
				return;
			}
		}
	}
}


void findStart(std::vector<std::vector<Box>>& boxes, Box*& start) {
	for (auto& box_row : boxes) {
		for (auto& box_row_col : box_row) {
			if (box_row_col.isStart) {
				start = &box_row_col;
				return;
			}
		}
	}
}


void reconstruct(sf::RenderWindow& window, Box* start, Box* end) {
	// backtracking from the end box until the start box
	Box* last = end;
	while (last != start) {
		if (!last->isEnd) last->color = sf::Color::Cyan;
		last = last->previous;
	}
}


void algorithm(sf::RenderWindow& window, std::vector<std::vector<Box>>& boxes) {
	// initialize neccessary variable
	Box* start = nullptr;
	Box* end = nullptr;
	Box* current = nullptr;
	int max_row_col = sqrt(NUM_BOXES);
	std::priority_queue<Box*, std::vector<Box*>, myComparator> open;

	findStart(boxes, start);
	findEnd(boxes, end);
	open.push(start);

	while (!open.empty()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
				return;
			}
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R) {
				reset(boxes);
				return;
			}
		}

		current = open.top();
		open.pop();
		current->updateNeighbors(boxes, max_row_col);

		// stop the loop when we reach the end
		if (current == end) {
			reconstruct(window, start, end);
			return;
		}

		// look for each neighbor for the current box
		for (Box* neighbor : current->neighbors) {
			// we don't need to consider the closed box
			if (neighbor->isClosed) continue;

			int tentative_g = current->g + 1;

			// update the neighbor's field
			if (!neighbor->isOpen || neighbor->g > tentative_g) {
				neighbor->g = tentative_g;
				neighbor->h = h(neighbor, end);
				neighbor->f = neighbor->g + neighbor->h;
				neighbor->previous = current;

				if (!neighbor->isOpen) {
					neighbor->makeOpen();
					open.push(neighbor);
				}
			}
		}
		current->makeClosed();

		// draw each step
		drawBoxes(window, boxes);
		drawGrid(window, lines);
		window.display();
	}
}


void isKeyPressed(sf::RenderWindow& window, std::vector<std::vector<Box>>& boxes) {
	static bool pressing = false; // static so it retains its value across function calls
	sf::Event event;

	while (window.pollEvent(event)) {
		switch (event.type) {
		case sf::Event::Closed:
			window.close();
			break;
		case sf::Event::KeyPressed:
			if (event.key.code == sf::Keyboard::R && !pressing) {
				pressing = true;
				reset(boxes);
			}
			if (event.key.code == sf::Keyboard::Space && !pressing) {
				if (solved) {
					reset(boxes);
				}
				pressing = true;
				std::cout << "space is pressed" << std::endl;
				if (gotStart && gotEnd && !solved) {
					solved = true;
					algorithm(window, boxes);
				}
			}
			break;
		case sf::Event::KeyReleased:
			pressing = false;
			break;
		}
	}
}



