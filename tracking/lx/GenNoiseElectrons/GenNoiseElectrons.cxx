#include "GenNoiseElectrons.h"
#include "FairLogger.h"
#include "TClonesArray.h"
#include "CbmMuchPoint.h"
#include "TH1F.h"
#include "TRandom3.h"
#include "CbmMuchGeoScheme.h"
#include "TGeoManager.h"

ClassImp(LxGenNoiseElectrons)

LxGenNoiseElectrons::LxGenNoiseElectrons() : fNofNoiseE(1), fNofStations(4), fMCTracks(0), fMuchPoints(0), fTrdPoints(0)
{
}

static Double_t x_rmss[4][3];
static Double_t y_rmss[4][3];

static Double_t GetRMS(const char* name)
{
   Double_t result = -1.;
   char fileName[128];
   sprintf(fileName, "%s.root", name);
   TFile* curFile = TFile::CurrentFile();
   TFile* f = new TFile(fileName);
   
   if (!f->IsZombie())
   {
      TH1F* h = static_cast<TH1F*> (f->Get(name));
   
      if (0 != h)
         result = h->GetRMS();
   }
   
   delete f;
   TFile::CurrentFile() = curFile;
   return result;
}

InitStatus LxGenNoiseElectrons::Init()
{
   FairRootManager* ioman = FairRootManager::Instance();
    
   if (0 == ioman)
      fLogger->Fatal(MESSAGE_ORIGIN, "No FairRootManager");
   
   fMCTracks = static_cast<TClonesArray*> (ioman->GetObject("MCTrack"));
   fMuchPoints = static_cast<TClonesArray*> (ioman->GetObject("MuchPoint"));
   fTrdPoints = static_cast<TClonesArray*> (ioman->GetObject("TrdPoint"));
   
   if (0 == fMCTracks || 0 == fMuchPoints)
      fLogger->Fatal(MESSAGE_ORIGIN, "No MC tracks or points");
   
   char name[128];
   
   for (int i = 0; i < 4; ++i)
   {
      for (int j = 0; j < 3; ++j)
      {
         sprintf(name, "noise_e_x_%d_%d", i, j);
         x_rmss[i][j] = GetRMS(name);
         
         if (x_rmss[i][j] < 0)
         {
            sprintf(name, "Couldn't read noise_e_x_%d_%d", i, j);
            fLogger->Fatal(MESSAGE_ORIGIN, name);
         }
         
         sprintf(name, "noise_e_y_%d_%d", i, j);
         y_rmss[i][j] = GetRMS(name);
         
         if (y_rmss[i][j] < 0)
         {
            sprintf(name, "Couldn't read noise_e_y_%d_%d", i, j);
            fLogger->Fatal(MESSAGE_ORIGIN, name);
         }
      }
   }
   
   return kSUCCESS;
}

void LxGenNoiseElectrons::Exec(Option_t* opt)
{
   Int_t nofMuchPoints = fMuchPoints->GetEntriesFast();
   TClonesArray& clref = *fMuchPoints;
   Int_t ind = nofMuchPoints;
   
   for (Int_t i = 0; i < nofMuchPoints; ++i)
   {
      const CbmMuchPoint* pMuchPt = static_cast<const CbmMuchPoint*> (fMuchPoints->At(i));
      Int_t stationNumber = CbmMuchGeoScheme::GetStationIndex(pMuchPt->GetDetectorId());
      Int_t layerNumber = CbmMuchGeoScheme::GetLayerIndex(pMuchPt->GetDetectorId());
      
      for (int j = 0; j < fNofNoiseE; ++j)
      {
         CbmMuchPoint noiseMuchPt(*pMuchPt);
         TVector3 posIn;
         TVector3 posOut;
         pMuchPt->PositionIn(posIn);
         pMuchPt->PositionOut(posOut);
         TVector3 posDelta(gRandom->Gaus(0, x_rmss[stationNumber][layerNumber]), gRandom->Gaus(0, y_rmss[stationNumber][layerNumber]), 0);
         posIn += posDelta;
         posOut += posDelta;
         Double_t x = (posIn.X() + posOut.X()) / 2;
         Double_t y = (posIn.Y() + posOut.Y()) / 2;
         Double_t z = (posIn.Z() + posOut.Z()) / 2;
         TGeoNode* node = gGeoManager->FindNode(x, y, z);
         
         if (0 == node)
            continue;
         
         TString nodeName(node->GetName());
         
         if (!nodeName.Contains("factive", TString::kIgnoreCase))
            continue;
         
         noiseMuchPt.SetPosition(posIn);
         noiseMuchPt.SetPositionOut(posOut);
         new(clref[ind++]) CbmMuchPoint(noiseMuchPt);
      }
   }
}

void LxGenNoiseElectrons::Finish()
{
}
