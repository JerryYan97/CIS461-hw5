#include "fresnel.h"

Color3f FresnelDielectric::Evaluate(float cosThetaI) const
{
    //TODO
    cosThetaI = glm::clamp(cosThetaI, -1.f, 1.f);

    // Potentially swap indices of refraction.
    float currEtaI = etaI;
    float currEtaT = etaT;
    bool entering = (cosThetaI > 0.f);
    // if(!entering)
    if(!entering)
    {
        std::swap(currEtaI, currEtaT);
        cosThetaI = std::abs(cosThetaI);
    }

    // Compute cosThetaT using Snell's law.
    float sinThetaI = std::sqrt(std::max(0.f, 1.f - cosThetaI * cosThetaI));

    float sinThetaT = currEtaI / currEtaT * sinThetaI;

    // Handle total internal reflection.
    if(sinThetaT >= 1.f)
    {
       return Color3f(1.f);
    }

    float cosThetaT = std::sqrt(std::max(0.f, 1.f - sinThetaT * sinThetaT));

    float Rparl = ((currEtaT * cosThetaI) - (currEtaI * cosThetaT)) / ((currEtaT * cosThetaI) + (currEtaI * cosThetaT));
    float Rperp = ((currEtaI * cosThetaI) - (currEtaT * cosThetaT)) / ((currEtaI * cosThetaI) + (currEtaT * cosThetaT));

    return Color3f((Rparl * Rparl + Rperp * Rperp) / 2.f);
    // return Color3f(1.f);
}
