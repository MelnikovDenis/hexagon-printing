#include <SFML/Graphics.hpp>
#include <iostream>
#include <list>
#include <cmath>
#define M_PI 3.14159265358979323846f
using namespace sf;

const int WINDOW_WIDTH = 600; //ширина главного окна
const int WINDOW_HEIGHT = 600; //высота главного окна


class Track : public Drawable {
private:
	Vertex line[2];	
public:
	Track(const Vector2f& startPoint, const Vector2f& endPoint) {
		line[0] = startPoint;
		line[1] = endPoint;
		line[0].color = Color::Magenta;
		line[1].color = Color::Magenta;
	}
	void setEndPoint(const Vector2f& endPoint) {
		line[1] = endPoint;
		line[1].color = Color::Magenta;
	}
	Vector2f getEndPoint() {
		return line[1].position;
	}
	void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		target.draw(line, 2, Lines, states);
	}
};


class Printhead : public Drawable {
private:
	CircleShape head; 
	std::list<Track> trackList; //список, хранящий в себе линии, по которым двигался экструктор
	
	bool isFinished = false; //было ли закончено рисование текущей линии
	//получение длины вектора
	float getLength(const Vector2f& vec) {
		return sqrtf(pow(vec.x, 2) + pow(vec.y, 2));
	}
	//получение единичного вектора
	Vector2f getUnitVector(const Vector2f& vec) {
		return Vector2f(vec.x / getLength(vec), vec.y / getLength(vec));
	}
	//получение координат вектора на основе точек начала и конца
	Vector2f getVector(const Vector2f& startPoint, const Vector2f& endPoint) {
		return Vector2f(endPoint.x - startPoint.x, endPoint.y - startPoint.y);
	}
	
public:
	Printhead(const Vector2f& startPoint, const float& radius) {
		head.setRadius(radius);
		head.setFillColor(Color::Yellow);
		setCentralPos(startPoint);
	}
	Vector2f getCenter() const {
		return Vector2f(head.getPosition().x + (head.getLocalBounds().width / 2),
			head.getPosition().y + (head.getLocalBounds().height / 2));
	}
	void setCentralPos(const Vector2f& pos) {
		head.setPosition(pos.x - (head.getLocalBounds().width / 2), pos.y - (head.getLocalBounds().height / 2));
	}	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		for (auto iter = trackList.begin(); iter != trackList.end(); ++iter) {
			target.draw(*iter, states);
		}
		target.draw(head, states);	
	}
	float getRadius() {
		return head.getRadius();
	}
	void move(std::list<Vector2f>& points, const float& speed, Clock& clock, std::list<Vector2f>::iterator& nextPoint) {
		if (nextPoint != points.end()) {
			std::cout << getCenter().x << ':' << getCenter().y << '\n';
			if (trackList.empty() || isFinished) {
				trackList.push_back(Track(getCenter(), getCenter()));
			}
			else {
				trackList.back().setEndPoint(getCenter());
			}
			isFinished = false;
			if ((getLength(getVector(*nextPoint, getCenter())) > 1.f) && !isFinished) {
				//вектор направления
				Vector2f vec = getVector(getCenter(), *nextPoint);
				//единичный вектор направления
				Vector2f unitVec = getUnitVector(vec);

				float time = clock.getElapsedTime().asMilliseconds();
				clock.restart();
				
				Vector2f motionVec(unitVec.x * (speed / 1000 * time), unitVec.y * (speed / 1000 * time));
				setCentralPos(Vector2f(getCenter().x + motionVec.x, getCenter().y + motionVec.y));				
			}
			else {
				std::cout << "\tТОЧКА СМЕНЫ НАПРАВЛЕНИЯ\n";
				trackList.back().setEndPoint(*nextPoint);
				setCentralPos(*nextPoint);
				++nextPoint;
				points.pop_front();
				isFinished = true;
			}
		}
	}
};


int main() {
	setlocale(LC_ALL, "ru");
	ContextSettings settings;
	settings.antialiasingLevel = 8;
	RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Practice", sf::Style::Close | sf::Style::Titlebar, settings);
	
	Printhead head(Vector2f(300.f, 300.f), 5.f);
	Clock clock;
	float curr_radius = head.getRadius() * 2.f;
	float shape_radius = 40.f;
	int angles = 6;
	std::list<Vector2f> points;
	
	while (curr_radius < shape_radius) {
		for (int i = 0; i < angles - 1; ++i) {
			points.push_back(Vector2f(300.f + curr_radius * cosf(2.f * i * M_PI / angles), 300.f + curr_radius * sinf(2.f * i * M_PI / angles)));
		}
		if(curr_radius + head.getRadius() * 2.f >= shape_radius)
			points.push_back(Vector2f(300.f  + 2.f + curr_radius * cosf(2.f * (angles - 1) * M_PI / angles), 300.f + curr_radius * sinf(2.f * (angles - 1) * M_PI / angles)));
		else points.push_back(Vector2f(300.f + head.getRadius() * 2.f + curr_radius * cosf(2.f * (angles - 1) * M_PI / angles), 300.f + curr_radius * sinf(2.f * (angles - 1) * M_PI / angles)));
		curr_radius += head.getRadius() * 2.f;		
	}
	


	auto iter = points.begin();


	//главный цикл отрисовки окна
	while (window.isOpen()) {
		Event event;	
		//цикл обработки событий
		while (window.pollEvent(event)) {
			if (event.type == Event::Closed) window.close();
		}
		
		head.move(points, 200.f, clock, iter);

		window.clear();
		window.draw(head);
		window.display();
	}
	return 0;
}