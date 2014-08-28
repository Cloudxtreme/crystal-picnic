#ifndef GENERAL_TYPES_H
#define GENERAL_TYPES_H

namespace General {

template<typename T> class Size {
public:
	T w;
	T h;
	Size(T w, T h) {
		this->w = w;
		this->h = h;
	}
	Size() {}
	void operator=(const Size<T> &from) {
		w = from.w;
		h = from.h;
	}
};

template<typename T> class Point {
public:
	T x;
	T y;
	Point() {
		x = y = 0;
	}
	Point(T x, T y) {
		this->x = x;
		this->y = y;
	}
	Point(const Point<float> &p) {
		this->x = p.x;
		this->y = p.y;
	}
	Point(const Point<int> &p) {
		this->x = p.x;
		this->y = p.y;
	}
	inline void operator=(const Point<float> &from) {
		x = from.x;
		y = from.y;
	}
	inline void operator=(const Point<int> &from) {
		x = from.x;
		y = from.y;
	}
	inline bool operator==(const Point<T> &rhs) {
		return this->x == rhs.x && this->y == rhs.y;
	}
	inline Point<T> operator+(const Point<float> &rhs) {
		Point<T> p;
		p.x = this->x + rhs.x;
		p.y = this->y + rhs.y;
		return p;
	}
	inline Point<T> operator-(const Point<float> &rhs) {
		Point<T> p;
		p.x = this->x - rhs.x;
		p.y = this->y - rhs.y;
		return p;
	}
	inline Point<T> operator+(const Point<int> &rhs) {
		Point<T> p;
		p.x = this->x + rhs.x;
		p.y = this->y + rhs.y;
		return p;
	}
	inline Point<T> operator-(const Point<int> &rhs) {
		Point<T> p;
		p.x = this->x - rhs.x;
		p.y = this->y - rhs.y;
		return p;
	}
	inline Point<T> operator+(const Size<float> &rhs) {
		Point<T> p;
		p.x = this->x + rhs.w;
		p.y = this->y + rhs.h;
		return p;
	}
	inline Point<T> operator-(const Size<float> &rhs) {
		Point<T> p;
		p.x = this->x - rhs.w;
		p.y = this->y - rhs.h;
		return p;
	}
	inline Point<T> operator+(const Size<int> &rhs) {
		Point<T> p;
		p.x = this->x + rhs.w;
		p.y = this->y + rhs.h;
		return p;
	}
	inline Point<T> operator-(const Size<int> &rhs) {
		Point<T> p;
		p.x = this->x - rhs.w;
		p.y = this->y - rhs.h;
		return p;
	}
	inline const Point<T> &operator/(const float &rhs) {
		this->x /= rhs;
		this->y /= rhs;
		return *this;
	}
};

template<typename T> struct Line {
	T x1, y1, x2, y2;
};

template<typename T> class Rectangle {
public:
	T x1;
	T y1;
	T x2;
	T y2;
	Rectangle(T x1, T y1, T x2, T y2) {
		this->x1 = x1;
		this->y1 = y1;
		this->x2 = x2;
		this->y2 = y2;
	}
	Rectangle() {}
	void operator=(const Rectangle<T> &from) {
		x1 = from.x1;
		y1 = from.y1;
		x2 = from.x2;
		y2 = from.y2;
	}
};

template<typename T> bool compare_points_x(const Point<T> &a, const Point<T> &b)
{
	return a.x < b.x;
}

} // end General namespace

#endif // GENERAL_TYPES_H
