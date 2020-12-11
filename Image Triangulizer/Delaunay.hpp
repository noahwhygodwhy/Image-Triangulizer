#pragma once
#ifndef DELAUNAY_H
#define DELAUNAY_H

#include <vector>
#include <array>
#include <glm/glm.hpp>
#include <unordered_set>

using namespace std;
using namespace glm;

struct Edge
{
    array<vec2, 2> points;

    void print(string prefix = "") const
    {
        printf("%s (%f,%f), (%f,%f)\n", prefix.c_str(), points[0].x, points[0].y, points[1].x, points[1].y);
    }


    template<typename T>
    bool operator==(const T& other)
    {
        try
        {
            const Edge* otherEdge = dynamic_cast<const Edge*>(&other);
            if (otherEdge == 0)
            {
                return false;
            }
            if (this->points[0] == otherEdge->points[0] && this->points[1] == otherEdge->points[1])
            {
                return true;
            }
            if (this->points[0] == otherEdge->points[1] && this->points[1] == otherEdge->points[0])
            {
                return true;
            }
            return false;
        }
        catch (exception e)
        {
            return false;
        }
        printf("END OF == OPERATOR BAD BAD BAD\n\n\n");
        return false;
    }
};

struct Triangle
{
    array<vec2, 3> points;

    void print(string prefix = "") const
    {
        printf("%s (%f,%f), (%f,%f), (%f,%f)\n", prefix.c_str(), points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y);
    }

    bool hasEdge(const Edge& e) const
    {
        int shared = 0;
        for (vec2 p : points)
        {
            for (vec2 v : e.points)
            {
                if (p == v)
                {
                    shared += 1;
                }
            }
        }
        if (shared > 2)
        {
            printf("TRIANGLE HAS EDGE BROKE\n");
            exit(0);
        }
        return shared == 2;
    }
    /*bool hasPoint(vec2 p)
    {
        for (vec2 v : points)
        {
            if (v == p)
            {
                return true;
            }
        }
        return false;
    }*/

        template<typename T>
    bool operator==(const T& other) const
    {
        try
        {
            const Triangle* otherTri = dynamic_cast<const Triangle*>(&other);
            if (otherTri == 0)
            {
                return false;
            }
            for (int i = 0; i < 3; i++)
            {
                if (this->points[i] != otherTri->points[i])
                {
                    return false;
                }
            }
            return true;
        }
        catch (exception e)
        {
            return false;
        }
        printf("END OF == OPERATOR BAD BAD BAD\n\n\n");
        return false;
    }
};


namespace std {
    template<>
    struct hash<Edge> {
        inline size_t operator()(const Edge& x) const
        {
            hash<int> ih;

            size_t value = 0;

            for (vec2 p : x.points)
            {
                value = value ^ ih(p.x) & ih(p.y);
            }

            return value;
        }
    };
}


using namespace std;
using namespace glm;

float sign(vec2 a, vec2 b, vec2 p);

int inTriangle(const Triangle& t, vec2 point);

vector<Triangle> findContainer(const vector<Triangle>& triangles, vec2 point);


vector<Triangle> splitTriangle(vector<Triangle> tris, vec2 p);

bool adjacent(const Triangle& a, const Triangle& b);

float distance(const vec2& p1, const vec2& p2);

void checkMiddleEdge(Triangle& a, Triangle& b);

vec2 getCenter(const Triangle& t);

void fixIllegalTriangles(vector<Triangle>& tris, const vector<vec2>& points);

bool shareAPoint(Triangle& t, Triangle& superTriangle);

vector<Triangle> delaunay(const vector<vec2>& pointsIn, vec2 bottomLeft = vec2(0, 0), vec2 topRight = vec2(0, 0));

#endif