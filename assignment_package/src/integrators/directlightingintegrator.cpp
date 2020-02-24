#include "directlightingintegrator.h"


Color3f DirectLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    //TODO
    Color3f L(0.f);

    // Find closest ray intersection or return background
    // radiance.
    Intersection isect;

    if(!scene.Intersect(ray, &isect))
    {
        return L;
    }

    // Compute emitted and reflected light at ray intersection point

    // Initialize common variable for Whitted integrator
    Normal3f n = isect.normalGeometric;
    Vector3f wo = - ray.direction;

    // Compute scattering functions for surface interaction

    // Compute emitted light if ray hit an area light source
    Color3f lightSource = isect.Le(wo);
    if(!IsBlack(lightSource))
    {
        L += lightSource;
        return L;
    }

    // Ask _objectHit_ to produce a BSDF
    // based on other data stored in this
    // Intersection, e.g. the surface normal
    // and UV coordinates
    isect.ProduceBSDF();


    // Randomly select a light source from scene.lights and call its Sample_Li function:
    int nLights = int(scene.lights.size());
    if (nLights == 0) return Color3f(0.f);
    int lightNum = std::min((int)(sampler->Get1D() * nLights), nLights - 1);
    const std::shared_ptr<Light> &light = scene.lights[lightNum];

    Vector3f wi;
    float pdf;
    Color3f Li = light->Sample_Li(isect, sampler->Get2D(), &wi, &pdf);

    // Shadow Test:
    Ray tempRay = Ray(isect.point + isect.normalGeometric * 1e-4f, wi);
    Intersection tempInsect;
    if(!scene.Intersect(tempRay, &tempInsect))
    {
        return Color3f(0.f);
    }
    if(light.get() != tempInsect.objectHit->GetAreaLight())
    {
        return Color3f(0.f);
    }



    // evaluate the remaining components of the Light Transport Equation (you already have Li):
    Color3f f = isect.bsdf->f(wo, wi);
    if(pdf <= 0.f)
    {
        return Color3f(0.f);
    }
    L += f * Li * AbsDot(wi, n) / (pdf / nLights);

    return L;
}
