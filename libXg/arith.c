/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>

Point
add(Point a, Point b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}

Point
sub(Point a, Point b)
{
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

Rectangle
inset(Rectangle r, int n)
{
    r.min.x += n;
    r.min.y += n;
    r.max.x -= n;
    r.max.y -= n;
    return r;
}

Point
divpt(Point a, int b)
{
    a.x /= b;
    a.y /= b;
    return a;
}

Rectangle
rsubp(Rectangle r, Point p)
{
    r.min.x -= p.x;
    r.min.y -= p.y;
    r.max.x -= p.x;
    r.max.y -= p.y;
    return r;
}

Rectangle
raddp(Rectangle r, Point p)
{
    r.min.x += p.x;
    r.min.y += p.y;
    r.max.x += p.x;
    r.max.y += p.y;
    return r;
}

int
eqpt(Point p, Point q)
{
    return p.x==q.x && p.y==q.y;
}

int
rectXrect(Rectangle r, Rectangle s)
{
    return r.min.x<s.max.x && s.min.x<r.max.x &&
           r.min.y<s.max.y && s.min.y<r.max.y;
}

int
ptinrect(Point p, Rectangle r)
{
    return p.x>=r.min.x && p.x<r.max.x &&
           p.y>=r.min.y && p.y<r.max.y;
}

Rectangle
rcanon(Rectangle r)
{
    int t;
    if (r.max.x < r.min.x) {
        t = r.min.x;
        r.min.x = r.max.x;
        r.max.x = t;
    }
    if (r.max.y < r.min.y) {
        t = r.min.y;
        r.min.y = r.max.y;
        r.max.y = t;
    }
    return r;
}

Rectangle
Rect(int x1, int y1, int x2, int y2)
{
    Rectangle r;

    r.min.x = x1;
    r.min.y = y1;
    r.max.x = x2;
    r.max.y = y2;
    return r;
}

Rectangle
Rpt(Point p1, Point p2)
{
    Rectangle r;

    r.min = p1;
    r.max = p2;
    return r;
}

Point
Pt(int x, int y)
{
    Point p;

    p.x = x;
    p.y = y;
    return p;
}
