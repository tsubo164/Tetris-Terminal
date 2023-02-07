#ifndef POINT_H
#define POINT_H

struct Point {
    int x, y;

    Point() : x(0), y(0) {}
    Point(int xx, int yy) : x(xx), y(yy) {}

    const Point &operator+=(Point a)
    {
        x += a.x;
        y += a.y;
        return *this;
    }

    const Point &operator-=(Point a)
    {
        x -= a.x;
        y -= a.y;
        return *this;
    }
};

inline Point operator+(Point a, Point b)
{
    return {a.x + b.x, a.y + b.y};
}

inline Point operator-(Point a, Point b)
{
    return {a.x - b.x, a.y - b.y};
}

#endif
