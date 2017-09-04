/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "Station3D.h"
#include "Tracker.h"

void CbmBinned3DStation::SearchHits(const CbmPixelHit* searchHit, std::function<void(CbmTBin::HitHolder&)> handleHit)
{
   Double_t searchX = searchHit->GetX();
   Double_t searchY = searchHit->GetY();
   Double_t searchZ = searchHit->GetZ();
   Double_t searchT = searchHit->GetTime();

   Double_t searchDx = searchHit->GetDx();
   Double_t searchDxSq = searchDx * searchDx;
   Double_t searchDy = searchHit->GetDy();
   Double_t searchDySq = searchDy * searchDy;
   Double_t dt = searchHit->GetTimeError();
   Double_t dtSq = dt * dt;

   Double_t tx = searchX / searchZ;
   Double_t ty = searchY / searchZ;

   Double_t beamDxSq = CbmBinnedTracker::Instance()->GetBeamDxSq();
   Double_t beamDySq = CbmBinnedTracker::Instance()->GetBeamDySq();
   Double_t wX = cbmBinnedSigma * std::sqrt((1.5 - 2 * fMaxZ / searchZ) * (1.5 - 2 * fMaxZ / searchZ) * beamDxSq +
      (2 * fMaxZ / searchZ - 0.5) * (2 * fMaxZ / searchZ - 0.5) * searchDxSq + fDxSq);
   Double_t wY = cbmBinnedSigma * std::sqrt((1.5 - 2 * fMaxZ / searchZ) * (1.5 - 2 * fMaxZ / searchZ) * beamDySq +
      (2 * fMaxZ / searchZ - 0.5) * (2 * fMaxZ / searchZ - 0.5) * searchDySq + fDySq);

   Double_t xMin = tx > 0 ? tx * fMinZ - wX : tx * fMaxZ - wX;
   Double_t xMax = tx > 0 ? tx * fMaxZ + wX : tx * fMinZ + wX;

   Double_t yMin = ty > 0 ? ty * fMinZ - wY : ty * fMaxZ - wY;
   Double_t yMax = ty > 0 ? ty * fMaxZ + wY : ty * fMinZ + wY;

   Double_t tCoeff = std::sqrt(1 + tx * tx + ty * ty) / cbmBinnedSOL;
   Double_t wT = cbmBinnedSigma * std::sqrt(dtSq + fDtSq);
   Double_t tMin = searchT + tCoeff * (fMinZ - searchZ) - wT;
   Double_t tMax = searchT + tCoeff * (fMaxZ - searchZ) + wT;

   int lowerXind = GetXInd(xMin);
   int upperXind = GetXInd(xMax);
   int lowerYind = GetYInd(yMin);
   int upperYind = GetYInd(yMax);
   int lowerTind = GetTInd(tMin);
   int upperTind = GetTInd(tMax);

   for (int i = lowerYind; i <= upperYind; ++i)
   {
      CbmYBin& yBin = fYBins[i];

      for (int j = lowerXind; j <= upperXind; ++j)
      {
         CbmXBin& xBin = yBin[j];

         for (int k = lowerTind; k <= upperTind; ++k)
         {
            CbmTBin& tBin = xBin[k];
            std::list<CbmTBin::HitHolder>::iterator hitIter = tBin.HitsBegin();
            std::list<CbmTBin::HitHolder>::iterator hitIterEnd = tBin.HitsEnd();

            for (; hitIter != hitIterEnd; ++hitIter)
            {
               const CbmPixelHit* hit = hitIter->hit;
               Double_t hitZ = hit->GetZ();
               Double_t y = ty * hitZ;
               Double_t deltaY = hit->GetY() - y;

               if (deltaY * deltaY > cbmBinnedSigmaSq * ((1.5 - 2 * hitZ / searchZ) * (1.5 - 2 * hitZ / searchZ) * beamDySq +
                  (2 * hitZ / searchZ - 0.5) * (2 * hitZ / searchZ - 0.5) * searchDySq + hit->GetDy() * hit->GetDy()))
                  continue;

               Double_t x = tx * hitZ;
               Double_t deltaX = hit->GetX() - x;

               if (deltaX * deltaX > cbmBinnedSigmaSq * ((1.5 - 2 * hitZ / searchZ) * (1.5 - 2 * hitZ / searchZ) * beamDxSq +
                  (2 * hitZ / searchZ - 0.5) * (2 * hitZ / searchZ - 0.5) * searchDxSq + hit->GetDx() * hit->GetDx()))
                  continue;

               Double_t t = searchT + tCoeff * (hitZ - searchZ);
               Double_t deltaT = hit->GetTime() - t;

               if (deltaT * deltaT > cbmBinnedSigmaSq * (dtSq + hit->GetTimeError() * hit->GetTimeError()))
                  continue;

               handleHit(*hitIter);
            }
         }
      }
   }
}