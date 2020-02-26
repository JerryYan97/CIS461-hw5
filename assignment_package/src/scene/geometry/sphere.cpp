#include "sphere.h"
#include <iostream>
#include <math.h>
#include "warpfunctions.h"

float Sphere::Area() const
{
    //TODO later
    glm::vec3 tempScale = transform.getScale();
    return glm::pi<float>() * 4.f * tempScale.x * tempScale.x;
}

void Sphere::ComputeTBN(const Point3f& P, Normal3f* nor, Vector3f* tan, Vector3f* bit) const
{
    *nor = glm::normalize(transform.invTransT() * glm::normalize(P));
    CoordinateSystem(*nor, tan, bit);
}

bool Sphere::Intersect(const Ray &ray, Intersection *isect) const
{
    //Transform the ray
    Ray r_loc = ray.GetTransformedCopy(transform.invT());

    float A = pow(r_loc.direction.x, 2.f) + pow(r_loc.direction.y, 2.f) + pow(r_loc.direction.z, 2.f);
    float B = 2*(r_loc.direction.x*r_loc.origin.x + r_loc.direction.y * r_loc.origin.y + r_loc.direction.z * r_loc.origin.z);
    float C = pow(r_loc.origin.x, 2.f) + pow(r_loc.origin.y, 2.f) + pow(r_loc.origin.z, 2.f) - 1.f;//Radius is 1.f
    float discriminant = B*B - 4*A*C;
    //If the discriminant is negative, then there is no real root
    if(discriminant < 0){
        return false;
    }
    float t = (-B - sqrt(discriminant))/(2*A);
    if(t < 0)
    {
        t = (-B + sqrt(discriminant))/(2*A);
    }
    if(t >= 0)
    {
        Point3f P = glm::vec3(r_loc.origin + t*r_loc.direction);
        InitializeIntersection(isect, t, P);
        return true;
    }
    return false;
}

Point2f Sphere::GetUVCoordinates(const Point3f &point) const
{
    Point3f p = glm::normalize(point);
    float phi = atan2f(p.z, p.x);
    if(phi < 0)
    {
        phi += TwoPi;
    }
    float theta = glm::acos(p.y);
    return Point2f(1 - phi/TwoPi, 1 - theta / Pi);
}

Intersection Sphere::Sample(const Intersection &ref, const Point2f &xi, float *pdf) const
{
    // Compute coordinate system for sphere sampling
    Point3f pCenter = transform.position();
    Vector3f wc = glm::normalize(pCenter - ref.point);
    Vector3f wcX, wcY;
    CoordinateSystem(wc, &wcX, &wcY);
    glm::vec3 scale = transform.getScale();
    float radius = scale.x;

    // Sample uniformly on sphere if  is inside it
    Point3f pOrigin = OffsetRayOrigin(ref.point, ref.normalGeometric, pCenter - ref.point);
    if(DistanceSquared(pOrigin, pCenter) <= radius * radius)
    {
        return Sample(xi, pdf);
    }

    // Sample shpere uniformly inside subtended cone
    float sinThetaMax2 = radius * radius / DistanceSquared(ref.point, pCenter);
    float cosThetaMax = std::sqrt(std::max((Float)0, 1 - sinThetaMax2));
    float cosTheta = (1 - xi[0]) + xi[0] * cosThetaMax;
    float sinTheta = std::sqrt(std::max(0.f, 1.f - cosTheta * cosTheta));
    float phi = xi[1] * 2 * glm::pi<float>();

    // Compute angle alpha from center of sphere to sampled point on surface:
    Float dc = Distance(ref.point, pCenter);
    Float ds = dc * cosTheta - std::sqrt(std::max(0.f, radius * radius - dc * dc * sinTheta * sinTheta));
    Float cosAlpha = (dc * dc + radius * radius - ds * ds) / (2.f * dc * radius);
    Float sinAlpha = std::sqrt(std::max(0.f, 1.f - cosAlpha * cosAlpha));

    // Compute surface normal and sampled point on sphere
    Vector3f nObj = SphericalDirection(sinAlpha, cosAlpha, phi, -wcX, -wcY, -wc);
    Point3f pObj = radius * Point3f(nObj.x, nObj.y, nObj.z);

    // Return Interaction for sampled point on sphere
    Intersection it;
    // Reproject pObj to sphere surface.
    pObj *= radius / Distance(pObj, Point3f(0, 0, 0));

    it.point = glm::vec3(transform.T() * glm::vec4(pObj, 1.f));
    it.normalGeometric = glm::normalize(transform.invTransT() * Normal3f(pObj.x, pObj.y, pObj.z));
    return it;
}

Intersection Sphere::Sample(const Point2f &xi, Float *pdf) const
{
    glm::vec3 tempScale = transform.getScale();

    Point3f pObj = Point3f(0.f) + tempScale.x * WarpFunctions::squareToSphereUniform(xi);

    Intersection it;
    it.normalGeometric = glm::normalize(transform.invTransT() * Normal3f(pObj.x, pObj.y, pObj.z));
    it.point = glm::vec3(transform.T() * glm::vec4(pObj, 1.f));

    *pdf = 1.f / Area();
    return it;
}
