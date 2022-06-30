#include <SFML/Graphics.hpp>
#include <iostream>
#include <list>
#include <cmath>
#define M_PI 3.14159265358979323846f
#define NOVALUE Vector2f(FLT_MIN, FLT_MIN)
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
	static float getLength(const Vector2f& vec) {
		return sqrtf(pow(vec.x, 2) + pow(vec.y, 2));
	}
	//получение единичного вектора (орта)
	static Vector2f getUnitVector(const Vector2f& vec) {
		return Vector2f(vec.x / getLength(vec), vec.y / getLength(vec));
	}
	//получение координат точки пересечения двух прямых, заданных общим уравнением прямой (если не пересекаются вернёт Vector2f(FLT_MIN, FLT_MIN))
	//Ax + By = -C
	static Vector2f getIntersection(Vector3f coef1, Vector3f coef2) {
		float det = coef1.x * coef2.y - coef1.y * coef2.x;
		if (det == 0) return Vector2f(FLT_MIN, FLT_MIN);
		float det1 = coef1.z * coef2.y - coef1.y * coef2.z;
		float det2 = coef1.x * coef2.z - coef1.z * coef2.x;
		return Vector2f(det1 / det, det2 / det);
	}
	//получение коэффициентов общего уравнения врямой через 2 точки (Ax + By = -C), если точки совпадают вернёт Vector3f(FLT_MIN, FLT_MIN, FLT_MIN)
	static Vector3f getEquation(const Vector2f& startPoint, const Vector2f& endPoint) {
		if (startPoint == endPoint) return Vector3f(FLT_MIN, FLT_MIN, FLT_MIN);
		Vector2f vec = getVector(startPoint, endPoint);
		return Vector3f(vec.y, -1 * vec.x, endPoint.x * vec.y - endPoint.y * vec.x);
	}
	//получение коэфициентов уравнения горизонтальной прямой расположенной на определённой высоте
	static Vector3f getHorizontal(const float& height) {
		return Vector3f(0, 1, height);
	}
	//получение координат вектора на основе точек начала и конца
	static Vector2f getVector(const Vector2f& startPoint, const Vector2f& endPoint) {
		return Vector2f(endPoint.x - startPoint.x, endPoint.y - startPoint.y);
	}
	//первый сосед точки в фигуре (его номер)
	static int getFirstNeigh(const ConvexShape& shape, int num) {
		if (num == 0)  return shape.getPointCount() - 1;
		else return num - 1;
	}
	//второй сосед точки в фигуре (его номер)
	static int getSecondNeigh(const ConvexShape& shape, const int& num) {
		if (num == shape.getPointCount() - 1)  return 0;
		else return num + 1;
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
		target.draw(head);
	}
	float getRadius() {
		return head.getRadius();
	}
	void move(std::list<Vector2f>& points, const float& speed, Clock& clock, std::list<Vector2f>::iterator& nextPoint){
		if (nextPoint != points.end()) {
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

				//время исользуется для более плавного движения и сохранения постоянной скорости вне зависимости от того, на каком компьютере запущена программа
				float time = clock.getElapsedTime().asMilliseconds();
				clock.restart();
				
				Vector2f motionVec(unitVec.x * (speed / 1000 * time), unitVec.y * (speed / 1000 * time));
				setCentralPos(Vector2f(getCenter().x + motionVec.x, getCenter().y + motionVec.y));
			}
			else {
				trackList.back().setEndPoint(*nextPoint);
				setCentralPos(*nextPoint);
				++nextPoint;
				points.pop_front();
				isFinished = true;
			}
		}
	}
	//получить путь заполнения спиралью правильного шестиугольника
	void makeSpiralPath(std::list<Vector2f>& points, const float& shape_radius) {
		float curr_radius = head.getRadius() * 2.f;
		int angles = 6;

		while (curr_radius < shape_radius) {
			for (int i = 0; i < angles - 1; ++i) {
				points.push_back(Vector2f(300.f + curr_radius * cosf(2.f * i * M_PI / angles), 300.f + curr_radius * sinf(2.f * i * M_PI / angles)));
			}
			if (curr_radius + head.getRadius() * 2.f >= shape_radius)
				points.push_back(Vector2f(300.f + 2.f + curr_radius * cosf(2.f * (angles - 1) * M_PI / angles), 300.f + curr_radius * sinf(2.f * (angles - 1) * M_PI / angles)));
			else points.push_back(Vector2f(300.f + head.getRadius() * 2.f + curr_radius * cosf(2.f * (angles - 1) * M_PI / angles), 300.f + curr_radius * sinf(2.f * (angles - 1) * M_PI / angles)));
			curr_radius += head.getRadius() * 2.f;
		}
	}
	//получить путь заполнения змейкой любого выпуклого многоугольника
	void makeSnakePath(std::list<Vector2f>& points, const ConvexShape& shape) {
		float diam = head.getRadius() * 2.f;
		int p1 = getHighest(shape);
		int p2 = p1;
		float curr_height = shape.getPoint(p1).y + diam;	
		Vector2f inters_p1 = shape.getPoint(p1), inters_p2 = shape.getPoint(p1);
		std::cout << "Высочайшая точка: " << shape.getPoint(p1).x << "; " << shape.getPoint(p1).y << '\n';
		std::cout << "Нижайшая точка: " << shape.getPoint(getLowest(shape)).x << "; " << shape.getPoint(getLowest(shape)).y << '\n';
		for (int i = 0; curr_height < shape.getPoint(getLowest(shape)).y; ++i, curr_height += diam) {
			//std::cout << "\ncurr_height: " << curr_height << '\n';			
			Vector3f horizontal = getHorizontal(curr_height);
			//std::cout << "horizontal: " << horizontal.x << "; " << horizontal.y << "; " << horizontal.z << '\n';
			
			Vector3f line1 = getEquation(shape.getPoint(p1), shape.getPoint(getFirstNeigh(shape, p1)));			
			inters_p1 = getIntersection(line1, horizontal);
			if (inters_p1.y >= shape.getPoint(getFirstNeigh(shape, p1)).y) {
				//std::cout << "------------СМЕНА ТОЧКИ1----------------\n";
				p1 = getFirstNeigh(shape, p1);
				line1 = getEquation(shape.getPoint(p1), shape.getPoint(getFirstNeigh(shape, p1)));
				inters_p1 = getIntersection(line1, horizontal);
			}
			if (inters_p1 == NOVALUE) {
				//std::cout << "------------СМЕНА ТОЧКИ1/1----------------\n";
				p1 = getFirstNeigh(shape, p1);
				line1 = getEquation(shape.getPoint(p1), shape.getPoint(getFirstNeigh(shape, p1)));
				
				inters_p1 = getIntersection(line1, horizontal);
			}
			//std::cout << "Первая линия: " << line1.x << "; " << line1.y << "; " << line1.z << '\n';
			Vector3f line2 = getEquation(shape.getPoint(p2), shape.getPoint(getSecondNeigh(shape, p2)));
			inters_p2 = getIntersection(line2, horizontal);

			if (inters_p2.y >= shape.getPoint(getSecondNeigh(shape, p2)).y) {
				//std::cout << "------------СМЕНА ТОЧКИ2----------------\n";
				p2 = getSecondNeigh(shape, p2);
				line2 = getEquation(shape.getPoint(p2), shape.getPoint(getSecondNeigh(shape, p2)));
				inters_p2 = getIntersection(line2, horizontal);
			}
			if (inters_p2 == NOVALUE) {
				//std::cout << "------------СМЕНА ТОЧКИ2/2----------------\n";
				p2 = getSecondNeigh(shape, p2);
				line2 = getEquation(shape.getPoint(p2), shape.getPoint(getSecondNeigh(shape, p2)));
				inters_p2 = getIntersection(line2, horizontal);
			}
			//std::cout << "p1: " << p1 << "\tp2: " << p2 << '\n';
			if (i % 2 == 0) {
				points.push_back(inters_p2);
				points.push_back(inters_p1);
			}
			else {
				points.push_back(inters_p1);
				points.push_back(inters_p2);
			}	
			//std::cout << "inters_p1: " << inters_p1.x << "; " << inters_p1.y << '\n';
			//std::cout << "inters_p2: " << inters_p2.x << "; " << inters_p2.x << '\n';
		}
		points.push_back(shape.getPoint(getLowest(shape)));
	}
	//самая низкая точка данной фигуры
	static int getLowest(const ConvexShape& shape) {
		Vector2f lowest = Vector2f(FLT_MIN, FLT_MIN);
		int index = -1;
		for (int i = 0; i < shape.getPointCount(); ++i) {
			if (lowest.y < shape.getPoint(i).y) {
				lowest = shape.getPoint(i);
				index = i;
			};
		}
		return index;
	}
	//самая высокая точка данной фигуры
	static int getHighest(const ConvexShape& shape) {
		Vector2f highest = Vector2f(FLT_MAX, FLT_MAX);
		int index = -1;
		for (int i = 0; i < shape.getPointCount(); ++i) {
			if (highest.y > shape.getPoint(i).y) {
				highest = shape.getPoint(i);
				index = i;
			};
		}
		return index;
	}
};
	

int main() {
	setlocale(LC_ALL, "ru");
	ContextSettings settings;
	settings.antialiasingLevel = 8;
	RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Practice", sf::Style::Close | sf::Style::Titlebar, settings);
	
	
	Clock clock;	
	std::list<Vector2f> points;
	std::cout << "Введите радиус следа, оставляемого экструдером: "; float radius; std::cin >> radius;
	std::cout << "Введите кол-во вершин фигуры: "; int vertices; std::cin >> vertices;
	sf::ConvexShape polygon;
	polygon.setPointCount(vertices);
	Vector2f vertex;
	for (int i = 0; i < vertices; ++i) {
		std::cout << "Введите x координату вершины [" << i + 1 << "]: ";
		std::cin >> vertex.x;
		std::cout << "Введите y координату вершины [" << i + 1 << "]: ";
		std::cin >> vertex.y;
		polygon.setPoint(i, vertex);
	}
	Printhead head(polygon.getPoint(Printhead::getHighest(polygon)), radius);
	polygon.setFillColor(Color::Yellow);
	polygon.setOutlineColor(Color::Green);
	polygon.setOutlineThickness(2.f);
	head.makeSnakePath(points, polygon);
	clock.restart();
	auto iter = points.begin();
	//главный цикл отрисовки окна
	while (window.isOpen()) {
		Event event;	
		//цикл обработки событий
		while (window.pollEvent(event)) {
			if (event.type == Event::Closed) window.close();
		}
		
		head.move(points, 800.f, clock, iter);

		window.clear();
		window.draw(polygon);
		window.draw(head);
		window.display();
	}
	return 0;
}
//входные данные для тестов

/*2
5
200
400
140
333
200
333
200
265
300
341*/

/*2
4
200
350
200
250
350
250
350
350*/