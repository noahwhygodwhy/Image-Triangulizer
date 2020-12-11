#include "Delaunay.hpp"


using namespace std;
using namespace glm;

float sign(vec2 a, vec2 b, vec2 p)
{
    return (p.x - b.x) * (a.y - b.y) - (a.x - b.x) * (p.y - b.y);
}

int inTriangle(const Triangle& t, vec2 point)
{
    float x = sign(point, t.points[0], t.points[1]);
    float y = sign(point, t.points[1], t.points[2]);
    float z = sign(point, t.points[2], t.points[0]);
    if (x == 0 || y == 0 || z == 0)
    {
        return -1;
    }
    bool neg = x < 0 || y < 0 || z < 0;
    bool pos = x > 0 || y > 0 || z > 0;

    return !(pos && neg); //not all of them, and not none of them
}

vector<Triangle> findContainer(const vector<Triangle>& triangles, vec2 point)
{
    vector<Triangle> toReturn;
    for (const Triangle& t : triangles)
    {
        int result = inTriangle(t, point);
        printf("it's in %i triangles\n", result);
        if (result == 1)
        {
            toReturn.push_back(t);
            return toReturn;
        }
        else if (result == -1)
        {
            toReturn.push_back(t);
            if (toReturn.size() == 2)
            {
                return toReturn;
            }
        }
    }
    return toReturn; //just in case
}




vector<Triangle> splitTriangle(vector<Triangle> tris, vec2 p)
{
    printf("splitting %i triangles with point %f,%f\n", tris.size(), p.x, p.y);
    vector<Triangle> newTriangles;



    for (const Triangle& t : tris)
    {

        t.print("splitting");

        for (int i = 0; i < 3; i++)
        {

            Triangle theNewOne = { t.points };
            theNewOne.points[i] = p;

            float s = sign(theNewOne.points[(i + 1) % 3], theNewOne.points[(i + 2) % 3], theNewOne.points[i]);
            printf("s is %f\n", s);
            if (s != 0)
            {
                theNewOne.print("adding a triangle");
                newTriangles.push_back(theNewOne);
            }
            else
            {
                printf("not adding a triangle\n");
            }
        }
    }
    return newTriangles;
}

bool adjacent(const Triangle& a, const Triangle& b)
{
    int shared = 0;
    for (int i = 0; i < 3; i++)
    {
        shared += a.points[i] == b.points[i] ? 1 : 0;
    }
    return (shared == 2);
}

float distance(const vec2& p1, const vec2& p2)
{
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    return std::sqrt((dx * dx) + (dy * dy));
}

void checkMiddleEdge(Triangle& a, Triangle& b)
{
    vector<vec2> points;
    points.insert(points.end(), a.points.begin(), a.points.end());
    points.insert(points.end(), b.points.begin(), b.points.end());
    vector<vec2> uniquePoints;
    vector<vec2> sharedPoints;
    for (vec2 v : points)
    {
        if (count(points.begin(), points.end(), v) == 1)
        {
            uniquePoints.push_back(v);
        }
        else
        {
            sharedPoints.push_back(v);
        }
    }


    if (uniquePoints.size() != 2 || sharedPoints.size() != 4)
    {

        printf("ERROR IN checkMiddleEdge() in Delanuay.hpp\n");
        a.print("triangle 1 ");
        b.print("triangle 2");
        printf("unique points:\n");
        for (vec2 v : uniquePoints)
        {
            printf("%f, %f\n", v.x, v.y);
        }
        printf("shared points:\n");
        for (vec2 v : sharedPoints)
        {
            printf("%f, %f\n", v.x, v.y);
        }
        exit(-1);
    }
    float curDistance = distance(sharedPoints[0], sharedPoints[1]);
    float posDistance = distance(uniquePoints[0], uniquePoints[1]);
    //if it's illegal, flip the edge.
    if (posDistance < curDistance)
    {
        a.points = { {sharedPoints[0], sharedPoints[1], points[0]} };
        b.points = { {sharedPoints[0], sharedPoints[1], points[1]} };
    }
}


//TODO: what if the 3 points are inline
vec2 getCenter(const Triangle& t)
{
    bool debug = false;

    float middleX = NAN;
    float middleY = NAN;

    vec2 midAB = (t.points[0] + t.points[1]) / 2.0f;
    vec2 midBC = (t.points[1] + t.points[2]) / 2.0f;
    if (debug)
    {
        printf("A: %f,%f\n", t.points[0].x, t.points[0].y);
        printf("B: %f,%f\n", t.points[1].x, t.points[1].y);
        printf("C: %f,%f\n", t.points[2].x, t.points[2].y);
    }
    float slopeAB = ((t.points[0].y - t.points[1].y) / (t.points[0].x - t.points[1].x));
    float tangentAB;
    //if the slope is infinity, the tangent is 0, else, calculate the tangent
    if (slopeAB == INFINITY || slopeAB == -INFINITY)
    {
        tangentAB = 0;
    }
    else
    {
        tangentAB = -1 / slopeAB;
    }
    if (debug)
    {
        printf("slopeAB: %f\n", slopeAB);
        printf("tangentAB: %f\n", tangentAB);
    }
    //if the tangent is infinity, the middle X is the middle of points
    //if it's 0, middle Y is the middle of points
    //else calculate the y intercept
    float yIntTanAB = NAN;
    if (tangentAB == INFINITY || tangentAB == -INFINITY)
    {
        middleX = (t.points[0].x+t.points[1].x)/2;
    }
    else if (tangentAB == 0)
    {
        middleY = (t.points[0].y + t.points[1].y) / 2.0f;
    }
    else
    {
        yIntTanAB = midAB.y - (tangentAB * midAB.x);
    }

    float slopeBC = ((t.points[1].y - t.points[2].y) / (t.points[1].x - t.points[2].x));
    float tangentBC;
    //if the slope is infinity, the tangent is 0, else, calculate the tangent
    if (slopeBC == INFINITY || slopeBC == -INFINITY)
    {
        tangentBC = 0;
    }
    else
    {
        tangentBC = -1 / slopeBC;
    }
    if (debug)
    {
        printf("slopeBC: %f\n", slopeBC);
        printf("tangentBC: %f\n", tangentBC);
    }

    //TODO: run it againand fix the missing values

    //if the tangent is infinity, the middle X is the middle of points
    //if it's 0, middle Y is the middle of points
    //else calculate the y intercept
    float yIntTanBC = NAN;
    if (tangentBC == INFINITY || tangentBC == -INFINITY)
    {

        middleX = (t.points[1].x + t.points[2].x)/2.0f;
    }
    else if (tangentBC == 0)
    {
        middleY = (t.points[1].y + t.points[2].y) / 2.0f;
    }
    else
    {
        yIntTanBC = midBC.y - (tangentBC * midBC.x);
    }

    if (debug)
    {
        printf("middleX: %f\n", middleX);
        printf("middleY: %f\n", middleY);
        printf("tangentAB: %f\n", tangentAB);
        printf("tangentBC: %f\n", tangentBC);
        printf("yIntTanAB: %f\n", yIntTanAB);
        printf("yIntTanBC: %f\n", yIntTanBC);
    }
    if (isnan(middleX) && isnan(middleY))
    {
        if (debug)
            printf("both NAN\n");
        middleX = (yIntTanBC - yIntTanAB) / (tangentAB - tangentBC);
        middleY = (tangentAB * middleX) + yIntTanAB;
    }
    else if (isnan(middleX))
    { 
        if(debug)
            printf("x NAN\n");
        if (isnan(yIntTanAB) || isnan(tangentAB))
        {
            middleX = (middleY - yIntTanBC) / tangentBC;
        }
        else
        {
            middleX = (middleY - yIntTanAB) / tangentAB;
        }
    }
    else if (isnan(middleY))
    {
        if(debug)
            printf("y NAN\n");
        //middleY = (tangentAB * middleX) + yIntTanAB;
        if (isnan(yIntTanAB) || isnan(tangentAB))
        {
            middleY = (tangentBC * middleX) + yIntTanBC;
        }
        else
        {
            middleY = (tangentAB * middleX) + yIntTanAB;
        }
    }

    return vec2(middleX, middleY);
}

void fixIllegalTriangles(vector<Triangle>& tris, const vector<vec2>& usedPoints)
{

    for (vec2 p : usedPoints)
    {
        auto triIter = tris.begin();
        while (triIter != tris.begin())
        {
            vec2 center = getCenter(*triIter);
            float radius = distance(center, triIter->points[0]);
            if (distance(p, center) < radius)
            {
                tris.erase(triIter);

                for (int i = 0; i < triIter->points.size(); i++)
                {
                    tris.push_back({ p, triIter->points[i], triIter->points[(i + 1) % triIter->points.size()] });
                }
            }
            else
            {
                triIter++;
            }
        }
    }


}


bool shareAPoint(Triangle& t, Triangle& other)
{
    for (vec2 a : t.points)
    {
        for (vec2 b : other.points)
        {
            if (a == b)
            {
                return true;
            }
        }
    }
    return false;
}


void addAPoint(vector<Triangle>& triangles, vec2 point)
{

    vector<Triangle> badTriangles;

    for (const Triangle& t : triangles) //for each triangle in triangles
    {
        vec2 center = getCenter(t);
        float radius = distance(t.points[0], center);
        if (distance(center, point) <= radius)
        {
            badTriangles.push_back(t);
        }
    }
    vector<Edge> polygon;
    for (const Triangle& t : badTriangles)//for each triangle in bad triangles
    {
        for (int i = 0; i < t.points.size(); i++) //for each edge
        {
            bool shared = false;
            for (const Triangle& ot : badTriangles)
            {
                if (!(ot == t))
                {
                    if (ot.hasEdge({ t.points[i], t.points[(i + 1) % t.points.size()] }))
                    {
                        shared = true;
                        break;
                    }
                }
            }
            if (!shared)
            {
                polygon.push_back({ t.points[i], t.points[(i + 1) % t.points.size()] });
            }
        }
    }
    for (const Triangle& t : badTriangles)
    {
        triangles.erase(find(triangles.begin(), triangles.end(), t));
    }
    for (const Edge& e : polygon)
    {
        triangles.push_back({ e.points[0], e.points[1], point });
    }
}


vector<Triangle> delaunay(const vector<vec2>& pointsIn, vec2 bottomLeft, vec2 topRight)
{
    vector<Triangle> triangles;
    vec2 mins = vec2(INT64_MAX);
    vec2 maxs = vec2(INT64_MIN);
    for (vec2 point : pointsIn)
    {
        mins = glm::min(mins, point);
        maxs = glm::max(maxs, point);
    }

    Triangle superTriangle = { mins - vec2(1),
        vec2(mins.x - 10, mins.y + 10 + ((maxs.y - mins.y) * 2)),
        vec2(mins.x + 10 + ((maxs.x - mins.x) * 2), mins.y - 10) };
    if (bottomLeft == topRight)
    {
        triangles.push_back(superTriangle);
    }
    else
    {
        Triangle t1 = { bottomLeft, vec2(bottomLeft.x, topRight.y), topRight };
        Triangle t2 = { bottomLeft, vec2(topRight.x, bottomLeft.y), topRight };
        triangles.push_back(t1);
        triangles.push_back(t2);
    }
    
    for (vec2 point : pointsIn)
    {
        addAPoint(triangles, point);
    }

    if (bottomLeft == topRight)
    {
        auto triIter = triangles.begin();
        while (triIter != triangles.end())
        {
            if (shareAPoint(superTriangle, *triIter))
            {
                triangles.erase(triIter);
            }
            else
            {
                triIter++;
            }
        }
    }
    return triangles;
}
