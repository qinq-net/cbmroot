#include "CbmStsHodoCorrelations.h"

#include "CbmStsDigi.h"
#include "CbmStsAddress.h"
#include "CbmFiberHodoCluster.h"
#include "CbmFiberHodoAddress.h"

#include "FairLogger.h"

#include "TClonesArray.h"
#include "TH2F.h"

#include <set>

using std::set;

// ---- Default constructor -------------------------------------------
CbmStsHodoCorrelations::CbmStsHodoCorrelations()
  : FairTask("CbmStsHodoCorrelations"),
    fStsDigi(NULL),
    fHodoCluster(NULL),
    hodo1_pos_sts0(NULL),
    hodo2_pos_sts0(NULL),
    hodo1_pos_sts1(NULL),
    hodo2_pos_sts1(NULL)
{
}

// ---- Destructor ----------------------------------------------------
CbmStsHodoCorrelations::~CbmStsHodoCorrelations()
{
}

// ----  Initialisation  ----------------------------------------------
void CbmStsHodoCorrelations::SetParContainers()
{
  fLogger->Debug(MESSAGE_ORIGIN,"SetParContainers of CbmStsHodoCorrelations");
  // Load all necessary parameter containers from the runtime data base
  /*
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb=ana->GetRuntimeDb();

  <CbmStsHodoCorrelationsDataMember> = (<ClassPointer>*)
    (rtdb->getContainer("<ContainerName>"));
  */
}

// ---- Init ----------------------------------------------------------
InitStatus CbmStsHodoCorrelations::Init()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of CbmStsHodoCorrelations");

  // Get a handle from the IO manager
  FairRootManager* ioman = FairRootManager::Instance();

  // Get a pointer to the previous already existing data level
  fStsDigi = (TClonesArray*) ioman->GetObject("StsDigi");
  if ( ! fStsDigi ) {
    LOG(ERROR) << "No branch StsDigi found!" << FairLogger::endl;
    LOG(ERROR) << "Task CbmStsHodoCorrelations will be inactive" << FairLogger::endl;
    return kERROR;
  }

  fHodoCluster = (TClonesArray*) ioman->GetObject("FiberHodoCluster");
  if ( ! fHodoCluster ) {
    LOG(ERROR) << "No branch FiberHodoCluster found!" << FairLogger::endl;
    LOG(ERROR) << "Task CbmStsHodoCorrelations will be inactive" << FairLogger::endl;
    return kERROR;
  }
  

  // Create the TClonesArray for the output data and register
  // it in the IO manager
  /*
    <OutputDataLevel> = new TClonesArray("OutputDataLevelName", 100);
    ioman->Register("OutputDataLevelName","OutputDataLevelName",<OutputDataLevel>,kTRUE);
  */

  // Do whatever else is needed at the initilization stage
  // Create histograms to be filled
  // initialize variables

  hodo1_pos_sts0 = new TH2F("xy1_sts0", "y vs. x Hodo 1, if sts0", 64, 0., 64., 64, 0., 64.);
  hodo2_pos_sts0 = new TH2F("xy2_sts0", "y vs. x Hodo 2, if sts0", 64, 0., 64., 64, 0., 64.);

  hodo1_pos_sts1 = new TH2F("xy1_sts1", "y vs. x Hodo 1, if sts1", 64, 0., 64., 64, 0., 64.);
  hodo2_pos_sts1 = new TH2F("xy2_sts1", "y vs. x Hodo 2, if sts1", 64, 0., 64., 64, 0., 64.);

  return kSUCCESS;

}

// ---- ReInit  -------------------------------------------------------
InitStatus CbmStsHodoCorrelations::ReInit()
{
  return kSUCCESS;
}

// ---- Exec ----------------------------------------------------------
void CbmStsHodoCorrelations::Exec(Option_t* option)
{

  UInt_t nofStsDigis = fStsDigi->GetEntriesFast();
  UInt_t nofHodoClusters = fHodoCluster->GetEntriesFast();
  
  UInt_t St0S0 = 0;
  UInt_t St0S1 = 0;
  UInt_t St1S0 = 0;
  UInt_t St1S1 = 0;
 
  // check if there is a coincidence between p- and n-side of sts0
  CbmStsDigi* stsDigi = NULL;
  for ( UInt_t iDigi=0; iDigi < nofStsDigis; ++iDigi) {
    stsDigi = static_cast<CbmStsDigi*>(fStsDigi->At(iDigi));
    UInt_t stsAddress = stsDigi->GetAddress();
    UInt_t station = CbmStsAddress::GetElementId(stsAddress, kStsStation);
    UInt_t side = CbmStsAddress::GetElementId(stsAddress, kStsSide);

    if ( 0 == station && 0 == side ) St0S0++; 
    if ( 0 == station && 1 == side ) St0S1++; 
    if ( 1 == station && 0 == side ) St1S0++; 
    if ( 1 == station && 1 == side ) St1S1++; 
  }

  if ( 4 == nofHodoClusters ) {
    // check if there is a cluster from each layer
    set<Int_t> layerset; 
    Double_t x1;
    Double_t x2;
    Double_t y1;
    Double_t y2;
    for(Int_t iclust=0; iclust<4; ++iclust ) {
      Int_t layerID = CbmFiberHodoAddress::GetLayerAddress(static_cast<CbmFiberHodoCluster*>(fHodoCluster->At(iclust))->GetAddress());
      if ( layerset.find(layerID) == layerset.end() ) {
	layerset.insert(layerID);
	switch (layerID) {
	case 0:
	  x1=64-static_cast<CbmFiberHodoCluster*>(fHodoCluster->At(iclust))->GetMean();
	  break;
	case 16:
	  y1=static_cast<CbmFiberHodoCluster*>(fHodoCluster->At(iclust))->GetMean();
	  break;
	case 1:
	  x2=static_cast<CbmFiberHodoCluster*>(fHodoCluster->At(iclust))->GetMean();
	  break;
	case 17:
	  y2=static_cast<CbmFiberHodoCluster*>(fHodoCluster->At(iclust))->GetMean();
	  break;
	}
      } else {
	return;
      }
      
    }
    if ( St0S0 > 0 && St0S1 > 0 ) {
      hodo1_pos_sts0->Fill(x1, y1);
      hodo2_pos_sts0->Fill(x2, y2);
    }  
    
    if ( St1S0 > 0 && St1S1 > 0 ) {
      hodo1_pos_sts1->Fill(x1, y1);
      hodo2_pos_sts1->Fill(x2, y2);
    }  
  }
  // check if there is a coincidence between p- and n-side of sts1

    //  LOG(INFO) << nofStsDigis << " Sts Digis and " << nofHodoClusters << " hodo clusters found." << FairLogger::endl; 
}

// ---- Finish --------------------------------------------------------
void CbmStsHodoCorrelations::Finish()
{
  hodo1_pos_sts0->Write();
  hodo2_pos_sts0->Write();
  hodo1_pos_sts1->Write();
  hodo2_pos_sts1->Write();
}

ClassImp(CbmStsHodoCorrelations)
