/**
 * \file CbmLitTofQa.cxx
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2011
 */
#include "CbmLitTofQa.h"
#include "qa/base/CbmLitAcceptanceFunction.h"
#include "CbmLitTofQaReport.h"
#include "CbmHistManager.h"
#include "CbmGlobalTrack.h"
#include "CbmTrackMatchNew.h"
#include "CbmStsTrack.h"
#include "CbmTofHit.h"
#include "CbmTofTrack.h"
#include "CbmTofPoint.h"
#include "CbmMCTrack.h"
#include "CbmKFVertex.h"
#include "CbmKFParticleInterface.h"
#include "TClonesArray.h"
#include "TH2F.h"
#include "TParticlePDG.h"
#include "TDatabasePDG.h"
#include <boost/assign/list_of.hpp>
#include <vector>
#include <cmath>
#include "utils/CbmLitConverter.h"
#include "data/CbmLitPixelHit.h"
using std::vector;
using std::pair;
using boost::assign::list_of;
using std::min;
using std::sqrt;
using std::map;
using std::list;

CbmLitTofQa::CbmLitTofQa():
   fIsFixedBounds(true),
   fOutputDir("./test/"),
   fHM(NULL),
   fPRangeMin(0.),
   fPRangeMax(15.),
   fPRangeBins(400),
   fGlobalTracks(NULL),
   fStsTracks(NULL),
   fStsTrackMatches(NULL),
   fTofHits(NULL),
   fTofDigiMatches(NULL),
   fTofDigiMatchPoints(NULL),
   fTofPoints(NULL),
   fTofTracks(NULL),
   fMCTracks(NULL),
   fPrimVertex(NULL),
   fKFFitter(),
   fTrackCategories(),
   fTrackAcceptanceFunctions(),
   fMCTrackIdForTofHits(),
   fMCTrackIdForTofPoints()
{
   FillTrackCategoriesAndAcceptanceFunctions();
}

CbmLitTofQa::~CbmLitTofQa()
{
   if (fHM) delete fHM;
}

InitStatus CbmLitTofQa::Init()
{
   fHM = new CbmHistManager();
   CreateHistograms();
   ReadDataBranches();
   fKFFitter.Init();
   CbmLitToolFactory* factory = CbmLitToolFactory::Instance();
   fFieldPropagator = factory->CreateTrackPropagator("lit");
   fLinePropagator = factory->CreateTrackPropagator("line");
   fFilter = factory->CreateTrackUpdate("kalman");
   return kSUCCESS;
}

void CbmLitTofQa::Exec(
   Option_t* opt)
{
   static Int_t nofEvents = 0;
   std::cout << "CbmLitTofQa::Exec: event=" << nofEvents++ << std::endl;
   ProcessMC();
   ProcessGlobalTracks();
   ProcessTofHits();
   ProcessTofTracks();
}

void CbmLitTofQa::Finish()
{
   FitHistograms();
   fHM->WriteToFile();
   CbmSimulationReport* report = new CbmLitTofQaReport();
   report->Create(fHM, fOutputDir);
   delete report;
}

static TClonesArray* globalTofMatches = 0;

void CbmLitTofQa::ReadDataBranches()
{
   FairRootManager* ioman = FairRootManager::Instance();
   assert(ioman != NULL);

   fGlobalTracks = (TClonesArray*) ioman->GetObject("GlobalTrack");
   fStsTracks = (TClonesArray*) ioman->GetObject("StsTrack");
   fStsTrackMatches = (TClonesArray*) ioman->GetObject("StsTrackMatch");
   fTofHits = (TClonesArray*) ioman->GetObject("TofHit");
   fTofDigiMatches = (TClonesArray*) ioman->GetObject("TofDigiMatch");
   fTofDigiMatchPoints = (TClonesArray*) ioman->GetObject("TofDigiMatchPoints");
   fTofPoints = (TClonesArray*) ioman->GetObject("TofPoint");
   fTofTracks = (TClonesArray*) ioman->GetObject("TofTrack");
   fMCTracks = (TClonesArray*) ioman->GetObject("MCTrack");
   fPrimVertex = (CbmVertex*) ioman->GetObject("PrimaryVertex");
   
   globalTofMatches = (TClonesArray*) ioman->GetObject("TofHitMatch");
}

void CbmLitTofQa::FillTrackCategoriesAndAcceptanceFunctions()
{
   vector<string> tmp = list_of("All")("Positive")("Negative")("Primary")("Secondary")("Electron")("Muon")("Proton")("AntiProton")("Pion")("PionPlus")("PionMinus")("Kaon")("KaonPlus")("KaonMinus");
   fTrackCategories =  tmp;
   // List of all supported track categories
   fTrackAcceptanceFunctions["All"] = CbmLitAcceptanceFunction::AllTrackAcceptanceFunction;
   fTrackAcceptanceFunctions["Positive"] = CbmLitAcceptanceFunction::PositiveTrackAcceptanceFunction;
   fTrackAcceptanceFunctions["Negative"] = CbmLitAcceptanceFunction::NegativeTrackAcceptanceFunction;
   fTrackAcceptanceFunctions["Primary"] = CbmLitAcceptanceFunction::PrimaryTrackAcceptanceFunction;
   fTrackAcceptanceFunctions["Secondary"] = CbmLitAcceptanceFunction::SecondaryTrackAcceptanceFunction;
   fTrackAcceptanceFunctions["Electron"] = CbmLitAcceptanceFunction::ElectronTrackAcceptanceFunction;
   fTrackAcceptanceFunctions["Muon"] = CbmLitAcceptanceFunction::MuonTrackAcceptanceFunction;
   fTrackAcceptanceFunctions["Proton"] = CbmLitAcceptanceFunction::ProtonTrackAcceptanceFunction;
   fTrackAcceptanceFunctions["AntiProton"] = CbmLitAcceptanceFunction::AntiProtonTrackAcceptanceFunction;
   fTrackAcceptanceFunctions["Pion"] = CbmLitAcceptanceFunction::PionTrackAcceptanceFunction;
   fTrackAcceptanceFunctions["PionPlus"] = CbmLitAcceptanceFunction::PionPlusTrackAcceptanceFunction;
   fTrackAcceptanceFunctions["PionMinus"] = CbmLitAcceptanceFunction::PionMinusTrackAcceptanceFunction;
   fTrackAcceptanceFunctions["Kaon"] = CbmLitAcceptanceFunction::KaonTrackAcceptanceFunction;
   fTrackAcceptanceFunctions["KaonPlus"] = CbmLitAcceptanceFunction::KaonPlusTrackAcceptanceFunction;
   fTrackAcceptanceFunctions["KaonMinus"] = CbmLitAcceptanceFunction::KaonMinusTrackAcceptanceFunction;
}

void CbmLitTofQa::CreateHistograms()
{
   Int_t nofTrackCategories = fTrackCategories.size();
   for (Int_t iCat = 0; iCat < nofTrackCategories; iCat++) {
	   string name = "hmp_Tof_Reco_" + fTrackCategories[iCat] + "_m2p";
      fHM->Add(name, new TH2F(name.c_str(), string(name + ";P [GeV/c];M^{2} [(GeV/c)^{2}]").c_str(), fPRangeBins, fPRangeMin, fPRangeMax, 400, -0.2, 1.8));
      name = "hmp_Tof_RecoMCID_" + fTrackCategories[iCat] + "_m2p";
      fHM->Add(name, new TH2F(name.c_str(), string(name + ";P [GeV/c];M^{2} [(GeV/c)^{2}]").c_str(), fPRangeBins, fPRangeMin, fPRangeMax, 400, -0.2, 1.8));
      name = "hmp_Tof_RecoAccTof_" + fTrackCategories[iCat] + "_m2p";
      fHM->Add(name, new TH2F(name.c_str(), string(name + ";P [GeV/c];M^{2} [(GeV/c)^{2}]").c_str(), fPRangeBins, fPRangeMin, fPRangeMax, 400, -0.2, 1.8));
      name = "hmp_Tof_RecoMCIDAccTof_" + fTrackCategories[iCat] + "_m2p";
      fHM->Add(name, new TH2F(name.c_str(), string(name + ";P [GeV/c];M^{2} [(GeV/c)^{2}]").c_str(), fPRangeBins, fPRangeMin, fPRangeMax, 400, -0.2, 1.8));

      name = "hmp_TofTrack_" + fTrackCategories[iCat] + "_Distance";
      fHM->Add(name, new TH1F(name.c_str(), string(name + ";Distance [cm]").c_str(), 200, 0., 50.));
      name = "hmp_TofTrack_" + fTrackCategories[iCat] + "_NormDistance";
      fHM->Add(name, new TH1F(name.c_str(), string(name + ";Normalized distance").c_str(), 200, 0., 50.));
      name = "hmp_TofTrack_" + fTrackCategories[iCat] + "_Length";
      fHM->Add(name, new TH1F(name.c_str(), string(name + ";Length [cm]").c_str(), 1200, 0., 1200.));
      name = "hmp_TofTrack_" + fTrackCategories[iCat] + "_NofHitsPerGlobalTrack";
      fHM->Add(name, new TH1F(name.c_str(), string(name + ";number of hits per global track").c_str(), 11, -0.5, 10.5));
   }
   string name = "hmp_Tof_dTime";
   fHM->Add(name, new TH1F(name.c_str(), string(name + ";dt [ps];Counter").c_str(), 1000, -500., 500.));
   name = "hmp_Tof_TimeZero_a";
   fHM->Add(name, new TH1F(name.c_str(), string(name + ";Time [ns];Counter").c_str(), 2000, 0., 36.));
   name = "hmp_Tof_TimeZero_reco";
   fHM->Add(name, new TH1F(name.c_str(), string(name + ";Time [ns];Counter").c_str(), 2000, -5.,15. ));
   name = "hmp_Tof_TimeZero_mc";
   fHM->Add(name, new TH1F(name.c_str(), string(name + ";Time [ns];Counter").c_str(), 2000, -5.,15. ));
   name = "hmp_Tof_TimeZero_NofTracks";
   fHM->Add(name, new TH1F(name.c_str(), string(name + ";Number of tracks;Counter").c_str(), 100, 0., 100.));
   name = "hmp_Tof_Time_FirstTrack";
   fHM->Add(name, new TH1F(name.c_str(), string(name + ";Time [ns];Counter").c_str(), 2000, 0., 36.));
   name = "hmp_Tof_Residual_X";
   fHM->Add(name, new TH1F(name.c_str(), string(name + ";Distance [cm]").c_str(), 200, -10., 10.));
   name = "hmp_Tof_Residual_Y";
   fHM->Add(name, new TH1F(name.c_str(), string(name + ";Distance [cm]").c_str(), 200, -10., 10.));
   name = "hmp_Tof_Track_Chi2_MC";
   fHM->Add(name, new TH1F(name.c_str(), string(name + ";Units").c_str(), 500, 0., 500.));
   name = "hmp_Tof_Track_Chi2_Hit";
   fHM->Add(name, new TH1F(name.c_str(), string(name + ";Units").c_str(), 500, 0., 500.));
   name = "hmp_Tof_Track_Good_Chi2_MC";
   fHM->Add(name, new TH1F(name.c_str(), string(name + ";Units").c_str(), 100, 0., 10.));
   name = "hmp_Tof_Track_Good_Chi2_Hit";
   fHM->Add(name, new TH1F(name.c_str(), string(name + ";Units").c_str(), 100, 0., 10.));
   name = "hmp_Tof_Track_Good_Chi2_Norm";
   fHM->Add(name, new TH1F(name.c_str(), string(name + ";Units").c_str(), 100, 0., 10.));
}

void CbmLitTofQa::ProcessMC()
{
   fMCTrackIdForTofHits.clear();
   fMCTrackIdForTofPoints.clear();

   Int_t nofHits = fTofHits->GetEntriesFast();
   for (Int_t iHit = 0; iHit < nofHits; iHit++) {
      const CbmTofHit* tofHit = static_cast<const CbmTofHit*>(fTofHits->At(iHit));
      const CbmTofPoint* tofPoint = static_cast<const CbmTofPoint*>(fTofPoints->At(tofHit->GetRefId()));
      fMCTrackIdForTofHits.insert(tofPoint->GetTrackID());
   }

   Int_t nofPoints = fTofPoints->GetEntriesFast();
   for (Int_t iPoint = 0; iPoint < nofPoints; iPoint++) {
      const CbmTofPoint* tofPoint = static_cast<const CbmTofPoint*>(fTofPoints->At(iPoint));
      fMCTrackIdForTofPoints.insert(tofPoint->GetTrackID());
   }
}

struct TofPointInfo
{
    Double_t x;
    Double_t y;
    Double_t z;
    set<const CbmTofHit*> genHits;
};

void CbmLitTofQa::ProcessGlobalTracks()
{
   Double_t timeZeroReco = 0.0;
   Double_t timeZeroMC = 0.0;
   Double_t timeFirstTrack = 100.; // ns
   Double_t timeZeroA = 0.; // ns
   Int_t nofTracksForTimeZero = 0;
   
   Int_t fPDG = 211;
   Double_t goodChi2Limit = 50.;
   
   map<Int_t, list<const CbmTofHit*> > mcTofPointHits;
   Double_t minTofHitZ = std::numeric_limits<Double_t>::max();
   Int_t nofTofHits = fTofHits->GetEntriesFast();
   
   for (Int_t iHit = 0; iHit < nofTofHits; ++iHit)
   {
       const CbmTofHit* tofHit = static_cast<const CbmTofHit*>(fTofHits->At(iHit));
       
       if (tofHit->GetZ() < minTofHitZ)
           minTofHitZ = tofHit->GetZ();
       
       const CbmMatch* tofDigiMatch = static_cast<const CbmMatch*>(fTofDigiMatches->At(iHit));
       const vector<CbmLink>& tofDigiLinks = tofDigiMatch->GetLinks();
       
       for (vector<CbmLink>::const_iterator digiIt = tofDigiLinks.begin(); digiIt != tofDigiLinks.end(); ++digiIt)
       {
           const CbmMatch* tofPointMatch = static_cast<const CbmMatch*>(fTofDigiMatchPoints->At(digiIt->GetIndex()));
           const vector<CbmLink>& tofPointLinks = tofPointMatch->GetLinks();
           
           for (vector<CbmLink>::const_iterator ptIt = tofPointLinks.begin(); ptIt != tofPointLinks.end(); ++ptIt)
               mcTofPointHits[ptIt->GetIndex()].push_back(tofHit);
       }
   }
   
   map<Int_t, map<Int_t, list<Int_t> > > mcTrackTofPoints;// TOF MC points grouped by track Id and then detector Id
   Int_t nofTofPoints = fTofPoints->GetEntriesFast();
   
   for (Int_t iPoint = 0; iPoint < nofTofPoints; ++iPoint)
   {
       const CbmTofPoint* tofPoint = static_cast<const CbmTofPoint*> (fTofPoints->At(iPoint));
       mcTrackTofPoints[tofPoint->GetTrackID()][tofPoint->GetDetectorID()].push_back(iPoint);
   }
   
   map<Int_t, list<TofPointInfo> > mcTrackAvgTofInfos;// Averaged by MRPC TOF MC points grouped by track Id
   
   for (map<Int_t, map<Int_t, list<Int_t> > >::const_iterator i = mcTrackTofPoints.begin(); i != mcTrackTofPoints.end(); ++i)
   {       
       for (map<Int_t, list<Int_t> >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
       {
            TofPointInfo avgInfo;
            Double_t avgX = 0;
            Double_t avgY = 0;
            Double_t avgZ = 0;
            Int_t ptCount = 0;
            
            for (list<Int_t>::const_iterator k = j->second.begin(); k != j->second.end(); ++k)
            {
                const CbmTofPoint* tofPoint = static_cast<const CbmTofPoint*> (fTofPoints->At(*k));
                avgX += tofPoint->GetX();
                avgY += tofPoint->GetY();
                avgZ += tofPoint->GetZ();
                ++ptCount;
                map<Int_t, list<const CbmTofHit*> >::const_iterator hitIt = mcTofPointHits.find(*k);
                
                if (hitIt != mcTofPointHits.end())
                    avgInfo.genHits.insert(hitIt->second.begin(), hitIt->second.end());
            }
            
            avgX /= ptCount;
            avgY /= ptCount;
            avgZ /= ptCount;
            avgInfo.x = avgX;
            avgInfo.y = avgY;
            avgInfo.z = avgZ;
            
            mcTrackAvgTofInfos[i->first].push_back(avgInfo);
       }
   }

   Int_t nofGlobalTracks = fGlobalTracks->GetEntriesFast();
   for (Int_t iTrack = 0; iTrack < nofGlobalTracks; iTrack++) {
      const CbmGlobalTrack* globalTrack = static_cast<const CbmGlobalTrack*>(fGlobalTracks->At(iTrack));
      Int_t stsId = globalTrack->GetStsTrackIndex();
      Int_t tofId = globalTrack->GetTofHitIndex();
      
      if (stsId < 0) continue; // We need both STS track further

      CbmStsTrack* stsTrack = static_cast<CbmStsTrack*>(fStsTracks->At(stsId));
      const CbmTrackMatchNew* stsMatch = static_cast<const CbmTrackMatchNew*>(fStsTrackMatches->At(stsId));
      Int_t stsMCTrackId = stsMatch->GetMatchedLink().GetIndex();
      
      bool goodChi2MC = false;
      bool goodChi2Hit = false;
      map<Int_t, list<TofPointInfo> >::const_iterator mcTofIter = mcTrackAvgTofInfos.find(stsMCTrackId);
      
      if (mcTofIter != mcTrackAvgTofInfos.end() && !mcTofIter->second.empty())
      {
          for (list<TofPointInfo>::const_iterator i = mcTofIter->second.begin(); i != mcTofIter->second.end(); ++i)
          {
                TofPointInfo tofInfo = *i;
                FairTrackParam parSts = FairTrackParam(*stsTrack->GetParamLast());
                CbmLitTrackParam par;
                CbmLitConverter::FairTrackParamToCbmLitTrackParam(&parSts, &par);
          
                if (fFieldPropagator->Propagate(&par, minTofHitZ, fPDG) != kLITERROR)
                {
                    fLinePropagator->Propagate(&par, tofInfo.z, fPDG);
                    Double_t dx = par.GetX() - tofInfo.x;
                    Double_t dy = par.GetY() - tofInfo.y;
                    fHM->H1("hmp_Tof_Residual_X")->Fill(dx);
                    fHM->H1("hmp_Tof_Residual_Y")->Fill(dy);
                    
                    for (set<const CbmTofHit*>::const_iterator j = tofInfo.genHits.begin(); j != tofInfo.genHits.end(); ++j)
                    {
                        Double_t chi = 0;
                        CbmLitTrackParam tpar(par);
                        CbmLitPixelHit hit;
                        CbmLitConverter::CbmPixelHitToCbmLitPixelHit(*j, 0, &hit);
                        hit.SetX(tofInfo.x);
                        hit.SetY(tofInfo.y);
                        hit.SetZ(tofInfo.z);
                        fFilter->Update(&tpar, &hit, chi);
                        fHM->H1("hmp_Tof_Track_Chi2_MC")->Fill(chi);
                        
                        if (chi < goodChi2Limit)
                            goodChi2MC = true;
                        
                        CbmLitTrackParam par2;
                        CbmLitConverter::FairTrackParamToCbmLitTrackParam(&parSts, &par2);
                        
                        if (fFieldPropagator->Propagate(&par2, minTofHitZ, fPDG) != kLITERROR)
                        {
                            fLinePropagator->Propagate(&par2, (*j)->GetZ(), fPDG);
                            chi = 0;
                            CbmLitTrackParam tpar2(par2);
                            CbmLitPixelHit hit2;
                            CbmLitConverter::CbmPixelHitToCbmLitPixelHit(*j, 0, &hit2);
                            fFilter->Update(&tpar2, &hit2, chi);
                            fHM->H1("hmp_Tof_Track_Chi2_Hit")->Fill(chi);
                            
                            if (chi < goodChi2Limit)
                                goodChi2Hit = true;
                        }
                    }
                }
          }
          
          const CbmMCTrack* mcTrack = static_cast<const CbmMCTrack*> (fMCTracks->At(stsMCTrackId));
          fHM->H1("hmp_Tof_Track_Good_Chi2_Norm")->Fill(mcTrack->GetP());
          
          if (goodChi2MC)
              fHM->H1("hmp_Tof_Track_Good_Chi2_MC")->Fill(mcTrack->GetP());
          
          if (goodChi2Hit)
              fHM->H1("hmp_Tof_Track_Good_Chi2_Hit")->Fill(mcTrack->GetP());
      }
      
      if (tofId < 0) continue; // Starting from here we need both STS track and TOF hit

      const CbmTofHit* tofHit = static_cast<const CbmTofHit*>(fTofHits->At(tofId));
      Int_t tofMCPointId = tofHit->GetRefId();
      const CbmTofPoint* tofPoint = static_cast<const CbmTofPoint*>(fTofPoints->At(tofMCPointId));
      Int_t tofMCTrackId = tofPoint->GetTrackID();

      
      FairTrackParam vtxTrack;
//       fKFFitter.FitToVertex(stsTrack, fPrimVertex, &vtxTrack);
//       Double_t chiSqPrimary = fKFFitter.GetChiToVertex(stsTrack, fPrimVertex);
      float chiSqPrimary = 0.f;
      CbmKFParticleInterface::ExtrapolateTrackToPV(stsTrack, fPrimVertex, &vtxTrack, chiSqPrimary);
      
      Bool_t chiSqPrimaryOk = chiSqPrimary < 3.;
      
//      Double_t ctCorrection = -0.007; 
      Double_t ctCorrection = 0.0;
      Double_t ctReco = 0.299792458 * tofHit->GetTime() + ctCorrection; // ToF time in ns -> transfrom to ct in m
      Double_t ctMC = 0.299792458 * tofPoint->GetTime() ; // mc time in ns -> transfrom to ct in m
      Double_t trackLengthReco = globalTrack->GetLength() / 100.; //global length
     // Double_t trackLengthMC = tofPoint->GetLength() / 100.; //mc length
      Double_t preco = (vtxTrack.GetQp() != 0) ? std::abs(1. / vtxTrack.GetQp()) : 0;
      Double_t t = (trackLengthReco != 0) ? (ctReco / trackLengthReco) : 0;
      Double_t m2reco = preco * preco * (t * t - 1);
      Double_t radialPos = sqrt(tofHit->GetX() * tofHit->GetX() + tofHit->GetY() * tofHit->GetY());

      if (chiSqPrimaryOk && radialPos < 50.) {
         nofTracksForTimeZero++;
         Double_t beta = 1.;
         if (m2reco > 0.6) beta = preco / sqrt(preco * preco + 0.93827231 * 0.93827231);
         if (m2reco <= 0.6) beta = preco / sqrt(preco * preco + 0.1395679 * 0.1395679);
         timeZeroReco += tofHit->GetTime() - trackLengthReco / (beta * 0.299792458);
         timeZeroMC += tofPoint->GetTime() - trackLengthReco / (beta * 0.299792458);
         timeZeroA += trackLengthReco / 0.299792458;
         timeFirstTrack = std::min(timeFirstTrack, trackLengthReco / 0.299792458);
      }

      Int_t nofTrackCategories = fTrackCategories.size();
      for (Int_t iCat = 0; iCat < nofTrackCategories; iCat++) {
    	  string category = fTrackCategories[iCat];
    	  LitTrackAcceptanceFunction function = fTrackAcceptanceFunctions.find(category)->second;
    	  Bool_t categoryOk = function(fMCTracks, stsMCTrackId);
    	  //Bool_t accTofOk = fMCTrackIdForTofPoints.find(stsMCTrackId) != fMCTrackIdForTofPoints.end();
    	  Bool_t accTofOk = fMCTrackIdForTofHits.find(stsMCTrackId) != fMCTrackIdForTofHits.end();

    	  if (categoryOk && chiSqPrimaryOk) {
    		  fHM->H1("hmp_Tof_Reco_" + category + "_m2p")->Fill(preco, m2reco);
    		  if (accTofOk) fHM->H1("hmp_Tof_RecoAccTof_" + category + "_m2p")->Fill(preco, m2reco);

    		  if (stsMCTrackId == tofMCTrackId) {
    		     fHM->H1("hmp_Tof_RecoMCID_" + category + "_m2p")->Fill(preco, m2reco);
    		     if (accTofOk) fHM->H1("hmp_Tof_RecoMCIDAccTof_" + category + "_m2p")->Fill(preco, m2reco);
    		  }
    	  }
      }
   }
   if (nofTracksForTimeZero > 0) {
      timeZeroReco /= nofTracksForTimeZero;
      timeZeroMC /= nofTracksForTimeZero;
      timeZeroA /= nofTracksForTimeZero;
   }
   fHM->H1("hmp_Tof_TimeZero_reco")->Fill(timeZeroReco);
   fHM->H1("hmp_Tof_TimeZero_mc")->Fill(timeZeroMC);
   fHM->H1("hmp_Tof_TimeZero_a")->Fill(timeZeroA);
   fHM->H1("hmp_Tof_TimeZero_NofTracks")->Fill(nofTracksForTimeZero);
   fHM->H1("hmp_Tof_Time_FirstTrack")->Fill(timeFirstTrack);
}

void CbmLitTofQa::ProcessTofHits()
{
   Int_t nofTofHits = fTofHits->GetEntriesFast();
   for (Int_t iHit = 0; iHit < nofTofHits; iHit++) {
      const CbmTofHit* tofHit = static_cast<const CbmTofHit*>(fTofHits->At(iHit));
      Int_t tofMCPointId = tofHit->GetRefId();
      const CbmTofPoint* tofPoint = static_cast<const CbmTofPoint*>(fTofPoints->At(tofMCPointId));
      Int_t tofMCTrackId = tofPoint->GetTrackID();

      fHM->H1("hmp_Tof_dTime")->Fill(1000*(tofPoint->GetTime() - tofHit->GetTime()));
      
      /*const CbmMatch* hitMatch = static_cast<const CbmMatch*> (globalTofMatches->At(iHit));
      const vector<CbmLink>& mcLinks = hitMatch->GetLinks();
      std::cout << "TOF hit x = " << tofHit->GetX() << ", y = " << tofHit->GetY() << ", z = " << tofHit->GetZ() << std::endl;
      
      for (vector<CbmLink>::const_iterator itMC = mcLinks.begin(); itMC != mcLinks.end(); ++itMC)
      {
          const CbmTofPoint* tofPoint = static_cast<const CbmTofPoint*>(fTofPoints->At(itMC->GetIndex()));
          std::cout << "TOF MC point x = " << tofPoint->GetX() << ", y = " << tofPoint->GetY() << ", z = " << tofPoint->GetZ() << std::endl;
      }*/
   }
}

void CbmLitTofQa::ProcessTofTracks()
{
   map<Int_t, Int_t> nofTofHitsPerGlobalTrack;
   Int_t nofTofTracks = fTofTracks->GetEntriesFast();
   for (Int_t iTrack = 0; iTrack < nofTofTracks; iTrack++) {
      const CbmTofTrack* tofTrack = static_cast<const CbmTofTrack*>(fTofTracks->At(iTrack));
      nofTofHitsPerGlobalTrack[tofTrack->GetTrackIndex()]++;
   }

   for (Int_t iTrack = 0; iTrack < nofTofTracks; iTrack++) {
      const CbmTofTrack* tofTrack = static_cast<const CbmTofTrack*>(fTofTracks->At(iTrack));
      const CbmTofHit* tofHit = static_cast<const CbmTofHit*>(fTofHits->At(tofTrack->GetTofHitIndex()));
      const FairMCPoint* tofPoint = static_cast<const FairMCPoint*>(fTofPoints->At(tofHit->GetRefId()));
      Int_t tofMCTrackId = tofPoint->GetTrackID();

      const FairTrackParam* par = tofTrack->GetTrackParameter();
      Double_t dx = par->GetX() - tofHit->GetX();
      Double_t dy = par->GetY() - tofHit->GetY();
      Double_t distance = sqrt(dx * dx + dy * dy);

      Int_t nofTrackCategories = fTrackCategories.size();
      for (Int_t iCat = 0; iCat < nofTrackCategories; iCat++) {
        string category = fTrackCategories[iCat];
        LitTrackAcceptanceFunction function = fTrackAcceptanceFunctions.find(category)->second;
        Bool_t categoryOk = function(fMCTracks, tofMCTrackId);
        if (categoryOk) {
           fHM->H1("hmp_TofTrack_" + category + "_Distance")->Fill(distance);
           fHM->H1("hmp_TofTrack_" + category + "_NormDistance")->Fill(tofTrack->GetDistance());
           fHM->H1("hmp_TofTrack_" + category + "_Length")->Fill(tofTrack->GetTrackLength());
           fHM->H1("hmp_TofTrack_" + category + "_NofHitsPerGlobalTrack")->Fill(nofTofHitsPerGlobalTrack[tofTrack->GetTrackIndex()]);
        }
      }
   }
}

void CbmLitTofQa::FitHistograms()
{
	fHM->H2("hmp_Tof_RecoMCID_Pion_m2p")->FitSlicesY();
	TH1* meanHist = (TH1*) gDirectory->Get("hmp_Tof_RecoMCID_Pion_m2p_1"); // mean
	TH1* sigmaHist = (TH1*) gDirectory->Get("hmp_Tof_RecoMCID_Pion_m2p_2"); // sigma
	Int_t nofBins = meanHist->GetNbinsX();
	for (Int_t iBin = 0; iBin <= nofBins; iBin++) {
		Double_t mean = meanHist->GetBinContent(iBin);
		Double_t sigma = sigmaHist->GetBinContent(iBin);
		std::cout << "mean=" << mean << " sigma=" << sigma << std::endl;
	}
}

ClassImp(CbmLitTofQa)
