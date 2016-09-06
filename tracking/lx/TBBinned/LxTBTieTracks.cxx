/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "LxTBTieTracks.h"
#include "data/CbmLitTrackParam.h"
#include "utils/CbmLitConverter.h"
#include "base/CbmLitToolFactory.h"

LxTBBinnedDetector gDetector;

void LxTBBinnedDetector::AddStsTrack(const FairTrackParam& par, Double_t time/*, Int_t selfId, Int_t eventId, Int_t fileId*/)
{
   LxTBBinnedStsTrack stsTrack = { par, time/*, selfId, eventId, fileId*/ };
   fStsTracks.push_back(stsTrack);
}

struct TieHandlePoint : public LxTBBinndedLayer::PointHandler
{
   TrackUpdatePtr fFilter;
   CbmLitTrackParam fPar;
   CbmLitTrackParam fOutPar;
   litfloat fChiSq;
   litfloat fZ;
   const LxTbBinnedPoint* fPoint;
   
   void HandlePoint(const LxTbBinnedPoint& point)
   {
      litfloat chiSq = std::numeric_limits<litfloat>::max();
      CbmLitPixelHit litHit;
      litHit.SetX(point.x);
      litHit.SetY(point.y);
      litHit.SetZ(fZ);
      litHit.SetDx(point.dx);
      litHit.SetDy(point.dy);
      litHit.SetDz(0);
      litHit.SetDxy(0);
      litHit.SetRefId(0);        
      CbmLitTrackParam par = fPar;
      fFilter->Update(&par, &litHit, chiSq);
      
      if (chiSq < 25 && chiSq < fChiSq)
      {
         fChiSq = chiSq;
         fPoint = &point;
         fOutPar = par;
      }
   }
};

void LxTBBinnedDetector::TieTracks(LxTbBinnedFinder& fFinder)
{
   //for (int i = 0; i < fFinder.nofTrackBins; ++i)
   //{
      //list<LxTbBinnedFinder::Chain*>& recoTracksBin = fFinder.recoTracks[i];
      
      //for (list<LxTbBinnedFinder::Chain*>::const_iterator j = recoTracksBin.begin(); j != recoTracksBin.end(); ++j)
         //recoTracks.push_back(*j);
   //}
   TrackPropagatorPtr propagator = CbmLitToolFactory::CreateTrackPropagator("lit");
   TrackUpdatePtr filter = CbmLitToolFactory::CreateTrackUpdate("kalman");
   //TrackFitterPtr fFitter = CbmLitToolFactory::CreateTrackFitter("lit_kalman");
   
   for (list<LxTBBinnedStsTrack>::const_iterator i = fStsTracks.begin(); i != fStsTracks.end(); ++i)
   {
      const LxTBBinnedStsTrack& stsTrack = *i;
      CbmLitTrackParam par;
      CbmLitConverterFairTrackParam::FairTrackParamToCbmLitTrackParam(&stsTrack.fPar, &par);
      CbmLitTrackParam prevPar;
      timetype t = stsTrack.fTime;
      
      for (int j = 0; j < fNofLayers; ++j)
      {
         prevPar = par;
         
         if (propagator->Propagate(&par, fLayers[j].z, 13) == kLITERROR)
            break;
         
         scaltype x = par.GetX();
         scaltype y = par.GetY();
         timetype length = sqrt((par.GetX() - prevPar.GetX()) * (par.GetX() - prevPar.GetX()) + (par.GetY() - prevPar.GetY()) * (par.GetY() - prevPar.GetY()) +
            (par.GetZ() - prevPar.GetZ()) * (par.GetZ() - prevPar.GetZ()));
         t += 1.e9 * length / speedOfLight;
         scaltype sigmaXSq = par.GetCovariance(0);
         scaltype sigmaYSq = par.GetCovariance(5);
         TieHandlePoint pointHandler;
         pointHandler.fFilter = filter;
         pointHandler.fPar = par;
         pointHandler.fZ = fLayers[j].z;
         pointHandler.fChiSq = std::numeric_limits<litfloat>::max();
         pointHandler.fPoint = 0;
         fLayers[j].Search(x, sigmaXSq, y, sigmaYSq, t, &pointHandler);
         
         if (pointHandler.fPoint)
         {
            par = pointHandler.fOutPar;
         }
      }
   }
}
