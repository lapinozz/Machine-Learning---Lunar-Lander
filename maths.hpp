#pragma once

#include <random>
#include <assert.h>

#include <SFML/System.hpp>

#ifdef  _USE_MATH_DEFINES

/* Define _USE_MATH_DEFINES before including math.h to expose these macro
 * definitions for common math constants.  These are placed under an #ifdef
 * since these commonly-defined names are not part of the C/C++ standards.
 */

/* Definitions of useful mathematical constants
 * M_E        - e
 * M_LOG2E    - log2(e)
 * M_LOG10E   - log10(e)
 * M_LN2      - ln(2)
 * M_LN10     - ln(10)
 * M_PI       - pi
 * M_PI_2     - pi/2
 * M_PI_4     - pi/4
 * M_1_PI     - 1/pi
 * M_2_PI     - 2/pi
 * M_2_SQRTPI - 2/sqrt(pi)
 * M_SQRT2    - sqrt(2)
 * M_SQRT1_2  - 1/sqrt(2)
 */

#define M_E        2.71828182845904523536
#define M_LOG2E    1.44269504088896340736
#define M_LOG10E   0.434294481903251827651
#define M_LN2      0.693147180559945309417
#define M_LN10     2.30258509299404568402
#define M_PI       3.14159265358979323846
#define M_PI_2     1.57079632679489661923
#define M_PI_4     0.785398163397448309616
#define M_1_PI     0.318309886183790671538
#define M_2_PI     0.636619772367581343076
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2    1.41421356237309504880
#define M_SQRT1_2  0.707106781186547524401

#endif  /* _USE_MATH_DEFINES */

static std::default_random_engine utilityRandEngine;

template<class T> T rand(T min, T max)
{
    std::uniform_int_distribution<T> dist(min, max);
    return dist(utilityRandEngine);
}

template<class T> T rand(T max)
{
    return rand((T)0, max);
}

inline float rand(float max)
{
    std::uniform_real_distribution<float> dist(0.f, max);
    return dist(utilityRandEngine);
}

inline float rand(float min, float max)
{
    std::uniform_real_distribution<float> dist(min, max);
    return dist(utilityRandEngine);
}

inline double rand(double min, double max)
{
    std::uniform_real_distribution<double> dist(min, max);
    return dist(utilityRandEngine);
}

template<class T = bool> bool randBool()
{
    std::bernoulli_distribution dist;
    return dist(utilityRandEngine);
}

inline bool probability(int pourcent)
{
    return pourcent >= rand(100);
}

inline bool probability(float chance)
{
    return chance >= rand(1.f);
}

template<class T> void seed(T seed = time(nullptr))
{
    utilityRandEngine.seed(seed);
}

template<class T> T getDistance(sf::Vector2<T> v1, sf::Vector2<T> v2)
{
    T kx = v1.x - v2.x;
    T ky = v1.y - v2.y;

    return sqrt(pow(kx, 2) + pow(ky, 2));
}

template<class T, class T2, class T3> bool isBetween(T val, T2 p1, T3 p2)
{
    return (val > p1 && val < p2);
}

template<class T> T toRad(T val)
{
    return val * M_PI / 180;
}

template<class T> T toDeg(T val)
{
    return val * 180 / M_PI;
}

template<class T> T lenght(sf::Vector2<T> vector)
{
    return sqrt(vector.x * vector.x + vector.y * vector.y);
}

template<class T> T normalize(T vector)
{
    float length = lenght(vector);
    if (length == 0)
        return sf::Vector2f(0, 0);
    vector.x = vector.x / length;
    vector.y = vector.y / length;

    return vector;
}

template<class T> sf::Vector2<T> angleToVector(T val)
{
    return normalize(sf::Vector2<T>(sin(toRad(val)), -cos(toRad(val))));
}

template<class T> T vectorToAngle(sf::Vector2<T> val)
{
    float angle = toDeg(atan2(val.x, -val.y));
    //    if(angle < 0.f)
    //        angle = 360.f + angle;
    return angle;
}

template<class T> T dot(sf::Vector2<T> vector1, sf::Vector2<T> vector2)
{
    return vector1.x*vector2.x + vector1.y*vector2.y;
}

template<class T> T perpDot(sf::Vector2<T> vector1, sf::Vector2<T> vector2)
{
    return (vector1.y*vector2.x) - (vector1.x*vector2.y);
}

template<class T> sf::Vector2<T> crossProduct(T val, sf::Vector2<T> vector)
{
    return sf::Vector2<T>(-val * vector.y, val * vector.x);
}

template<typename T> T lerp(T value1, T value2, float interpolation)
{
    return (value1 * (1.0f - interpolation)) + (value2 * interpolation);
}

template<typename T> T perpandicular(T vector)
{
    return T(-vector.y, vector.x);
}

const float tolerance = 0.000001f;//1e-15;

template<typename T> T roundNearZero(T val)
{
    return std::fabs(val) < tolerance ? 0 : val;
}

template<typename T> sf::Vector2<T> roundNearZero(sf::Vector2<T> vector)
{
    vector.x = roundNearZero(vector.x);
    vector.y = roundNearZero(vector.y);

    return vector;
}

template<typename T> sf::Vector2<T> abs(sf::Vector2<T> vector)
{
    return {std::abs(vector.x), std::abs(vector.y)};
}

inline float rayVsSegment(const sf::Vector2f& rayPos, const sf::Vector2f& rayDir, const sf::Vector2f& segPos, const sf::Vector2f& segSize)
{
    //if parallel return -1
    if(roundNearZero(lenght(normalize(abs(rayDir)) - normalize(abs(segSize)))) == 0)
        return -1.f;

    float T2 = (rayDir.x*(segPos.y-rayPos.y) + rayDir.y*(rayPos.x-segPos.x))/(segSize.x*rayDir.y - segSize.y*rayDir.x);
    if(T2 > 0.f && T2 < 1.f)
    {
        float T1;
        if(roundNearZero(rayDir.x) == 0)
            T1 = (segPos.y+segSize.y*T2-rayPos.y)/rayDir.y;
        else
            T1 = (segPos.x+segSize.x*T2-rayPos.x)/rayDir.x;

        if(T1 > 0.f)
        {
            return T1;
        }
    }

    return -1;
}

template<typename Container>
void projectOntoAxis(const Container& points, const sf::Vector2f& axis, float& min, float& max)
{
    assert(!points.empty());

    min = dot(points[0], axis);
    max = min;
    for(const sf::Vector2f& point : points)
    {
        float projection = dot(point, axis);

        if(projection < min)
            min = projection;

        if(projection > max)
            max = projection;
    }
}
