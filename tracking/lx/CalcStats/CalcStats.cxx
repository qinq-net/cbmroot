#include "CalcStats.h"
#include "FairLogger.h"
#include "TClonesArray.h"
#include "CbmMuchPoint.h"
#include <list>
#include <map>
#include "CbmMuchGeoScheme.h"
#include "CbmMCTrack.h"

ClassImp(LxCalcStats)

using namespace std;

struct LxStatTrack
{
   list<Int_t> points[4][3];
};

LxCalcStats::LxCalcStats() : fMCTracks(0), fMuchPoints(0), fTrdPoints(0)
{
}

InitStatus LxCalcStats::Init()
{
   FairRootManager* ioman = FairRootManager::Instance();
    
   if (0 == ioman)
      fLogger->Fatal(MESSAGE_ORIGIN, "No FairRootManager");
   
   fMCTracks = static_cast<TClonesArray*> (ioman->GetObject("MCTrack"));
   fMuchPoints = static_cast<TClonesArray*> (ioman->GetObject("MuchPoint"));
   fTrdPoints = static_cast<TClonesArray*> (ioman->GetObject("TrdPoint"));
   
   if (0 == fMCTracks || 0 == fMuchPoints)
      fLogger->Fatal(MESSAGE_ORIGIN, "No MC tracks or points");
   
   char buf[128];
   
   for (int i = 0; i < 4; ++i)
   {
      for (int j = 0; j < 3; ++j)
      {
         sprintf(buf, "noise_e_x_%d_%d", i, j);
         xHistos[i][j] = new TH1F(buf, buf, 240, -30., 30.);
         sprintf(buf, "noise_e_y_%d_%d", i, j);
         yHistos[i][j] = new TH1F(buf, buf, 240, -30., 30.);
      }
   }
   
   return kSUCCESS;
}

void LxCalcStats::Exec(Option_t* opt)
{
   map<Int_t, LxStatTrack> tracks;
   Int_t nofMuchPoints = fMuchPoints->GetEntriesFast();
   
   for (Int_t i = 0; i < nofMuchPoints; ++i)
   {
      const CbmMuchPoint* point = static_cast<const CbmMuchPoint*> (fMuchPoints->At(i));
      Int_t stationNumber = CbmMuchGeoScheme::GetStationIndex(point->GetDetectorId());
      Int_t layerNumber = CbmMuchGeoScheme::GetLayerIndex(point->GetDetectorId());
      Int_t trackId = point->GetTrackID();
      map<Int_t, LxStatTrack>::iterator trIt = tracks.find(trackId);
      
      if (trIt == tracks.end())
         tracks[trackId] = LxStatTrack();
      
      tracks[trackId].points[stationNumber][layerNumber].push_back(i);
   }
   
   for (Int_t i = 0; i < nofMuchPoints; ++i)
   {
      const CbmMuchPoint* point = static_cast<const CbmMuchPoint*> (fMuchPoints->At(i));
      Int_t stationNumber = CbmMuchGeoScheme::GetStationIndex(point->GetDetectorId());
      Int_t layerNumber = CbmMuchGeoScheme::GetLayerIndex(point->GetDetectorId());
      Int_t trackId = point->GetTrackID();
      const CbmMCTrack* track = static_cast<const CbmMCTrack*> (fMCTracks->At(trackId));
      Int_t pdgCode = track->GetPdgCode();
      
      if (11 == pdgCode)
      {
         Double_t x = (point->GetXIn() + point->GetXOut()) / 2;
         Double_t y = (point->GetYIn() + point->GetYOut()) / 2;
         Int_t parentId = track->GetMotherId();
         
         while (parentId >= 0)
         {
            const CbmMCTrack* parent = static_cast<const CbmMCTrack*> (fMCTracks->At(parentId));
            const list<Int_t> parentPts = tracks[parentId].points[stationNumber][layerNumber];
            
            for (list<Int_t>::const_iterator j = parentPts.begin(); j != parentPts.end(); ++j)
            {
               Int_t parentPtId = *j;
               const CbmMuchPoint* parentPt = static_cast<const CbmMuchPoint*> (fMuchPoints->At(parentPtId));
               Double_t pX = (parentPt->GetXIn() + parentPt->GetXOut()) / 2;
               Double_t pY = (parentPt->GetYIn() + parentPt->GetYOut()) / 2;
               xHistos[stationNumber][layerNumber]->Fill(x - pX);
               yHistos[stationNumber][layerNumber]->Fill(y - pY);
            }
            
            parentId = parent->GetMotherId();
         }
      }
   }
}

static void SaveHisto(TH1F* h)
{
   TFile* curFile = TFile::CurrentFile();
   char name[128];
   sprintf(name, "%s.root", h->GetName());
   TFile fh(name, "RECREATE");
   h->Write();
   fh.Close();
   delete h;
   TFile::CurrentFile() = curFile;
}

void LxCalcStats::Finish()
{
   for (int i = 0; i < 4; ++i)
   {
      for (int j = 0; j < 3; ++j)
      {
         SaveHisto(xHistos[i][j]);
         SaveHisto(yHistos[i][j]);
      }
   }
}
