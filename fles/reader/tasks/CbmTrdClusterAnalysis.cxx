#include "CbmTrdClusterAnalysis.h"
#include "CbmTrdDigi.h"
#include "CbmTrdCluster.h"
#include "CbmTrdHit.h"
#include "CbmHistManager.h"
#include "CbmBeamDefaults.h"

#include "FairLogger.h"

#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"

#include "TString.h"
#include "TStyle.h"

#include <cmath>
#include <map>
#include <vector>

CbmTrdClusterAnalysis::CbmTrdClusterAnalysis()
  :FairTask("CbmTrdClusterAnalysis"),
   fDigis(NULL),
   fClusters(NULL),  
   fHits(NULL),
   fHM(new CbmHistManager())
{
 LOG(DEBUG) << "Default Constructor of CbmTrdClusterAnalysis" << FairLogger::endl;
}
CbmTrdClusterAnalysis::~CbmTrdClusterAnalysis()
{
  fDigis->Delete();
  delete fDigis;
  fClusters->Delete();
  delete fClusters;
  fHits->Delete();
  delete fHits;
  LOG(DEBUG) << "Destructor of CbmTrdClusterAnalysis" << FairLogger::endl;
}
void CbmTrdClusterAnalysis::SetParContainers()
{
 LOG(DEBUG) << "SetParContainers of CbmTrdClusterAnalysis" << FairLogger::endl;
}
InitStatus CbmTrdClusterAnalysis::Init()
{
  FairRootManager* ioman = FairRootManager::Instance();
  fDigis = static_cast<TClonesArray*>(ioman->GetObject("TrdDigi"));
  if ( ! fDigis) {
    LOG(FATAL) << "No CbmTrdDigi TClonesArray!\n CbmTrdClusterAnalysis will be inactive" << FairLogger::endl;
    return kERROR;
  }
  fClusters = static_cast<TClonesArray*>(ioman->GetObject("TrdCluster"));
  if ( ! fClusters) {
    LOG(FATAL) << "No CbmTrdCluster TClonesArray!\n CbmTrdClusterAnalysis will be inactive" << FairLogger::endl;
    return kERROR;
  }
  fHits = new TClonesArray("CbmTrdHit", 100);
  ioman->Register("TrdHit", "TRD Hits", fDigis, kTRUE);

  CbmTrdClusterAnalysis::CreateHistograms();

  return kSUCCESS;
}
InitStatus CbmTrdClusterAnalysis::ReInit()
{
  LOG(DEBUG) << "Initilization of CbmTrdClusterAnalysis" << FairLogger::endl;
  return kSUCCESS;
}
void CbmTrdClusterAnalysis::Exec(Option_t*)
{
  // Exec runs on the TimeSliceContainer level. The TClonesArrys are not available at this time. Therefore we do the analysis in the finish function.
}
void CbmTrdClusterAnalysis::Finish()
{
  Int_t nDigis = fDigis->GetEntriesFast();
  Int_t nClusters = fClusters->GetEntriesFast();
  for (Int_t iCluster=0; iCluster < nClusters; ++iCluster) {
    CbmTrdCluster *cluster = (CbmTrdCluster*) fClusters->At(iCluster);
    Int_t nofDigis = cluster->GetNofDigis();
    Int_t moduleAddress = 0;
    Int_t secId(-1), colId(-1), rowId(-1);
    TVector3 hit_posV; // hit position in global chamber coordinates
    TVector3 local_pad_dposV; // Error in local pad coordinates
    Double_t totalCharge = 0;
    for (Int_t iDigi = 0; iDigi < nofDigis; iDigi++) {
      Int_t digiId = cluster->GetDigi(iDigi);
      const CbmTrdDigi* digi = static_cast<const CbmTrdDigi*>(fDigis->At(digiId));
      Int_t digiAddress = digi->GetAddress();
      secId = CbmTrdAddress::GetSectorId(digiAddress);
      colId = CbmTrdAddress::GetColumnId(digiAddress);
      rowId = CbmTrdAddress::GetRowId(digiAddress);
      //moduleInfo = fDigiPar->GetModule(CbmTrdAddress::GetModuleAddress(digiAddress));
      moduleAddress = CbmTrdAddress::GetModuleAddress(digi->GetAddress());
    }  
    Int_t nofHits = fHits->GetEntriesFast();
    new ((*fHits)[nofHits]) CbmTrdHit(moduleAddress, hit_posV, local_pad_dposV, 0, iCluster, 0, 0, totalCharge);

    /**
     * \brief Standard constructor CbmTrdHit.
     *\param address Unique detector ID
     *\param pos Position in global c.s. [cm]
     *\param dpos Errors of position in global c.s. [cm]
     *\param dxy XY correlation of the hit
     *\param refId Index of digi or cluster
     *\param eLossTR Energy losses of electron via TR
     *\param eLossdEdx Energy deposition in the detector without TR
     *\param eLoss TR + dEdx
     **/
  }


  LOG(DEBUG) << "Finish of CbmTrdRawBeamProfile" << FairLogger::endl;
  // Write to file
  fHM->WriteToFile();
  // Update Histos and Canvases
  LOG(INFO) << "CbmTrdClusterAnalysis::Finish Digis:               " << fDigis->GetEntriesFast() << FairLogger::endl;
  LOG(INFO) << "CbmTrdClusterAnalysis::Finish Clusters:            " << fClusters->GetEntriesFast() << FairLogger::endl;
  LOG(INFO) << "CbmTrdClusterAnalysis::Finish Hits:                " << fHits->GetEntriesFast() << FairLogger::endl;
}
void CbmTrdClusterAnalysis::CreateHistograms()
{
  gStyle->SetNumberContours(99);
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);
  fHM->Add("Test", new TH1I("Test", "Test", 1,-0.5,0.5));
}
ClassImp(CbmTrdClusterAnalysis)
