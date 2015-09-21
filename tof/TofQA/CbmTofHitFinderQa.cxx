/** @file CbmTofHitFinderQa.cxx
 ** @author Pierre-Alain Loizeau <loizeau@physi.uni-heidelberg.de>
 ** @date 27/08/2015
 **/
 
#include "CbmTofHitFinderQa.h"

// TOF Classes and includes
#include "CbmTofPoint.h"      // in cbmdata/tof
#include "CbmTofDigi.h"       // in cbmdata/tof
#include "CbmTofDigiExp.h"    // in cbmdata/tof
#include "CbmTofHit.h"        // in cbmdata/tof
#include "CbmTofGeoHandler.h" // in tof/TofTools
#include "CbmTofDetectorId_v12b.h" // in cbmdata/tof
#include "CbmTofDetectorId_v14a.h" // in cbmdata/tof
#include "CbmTofCell.h"       // in tof/TofData
#include "CbmTofDigiPar.h"    // in tof/TofParam
#include "CbmTofDigiBdfPar.h" // in tof/TofParam
#include "CbmTofAddress.h"    // in cbmdata/tof

// CBMroot classes and includes
#include "CbmMCTrack.h"
#include "CbmMatch.h"

// FAIR classes and includes
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairRootManager.h"
#include "FairLogger.h"

// ROOT Classes and includes
#include "Riostream.h"
#include "TClonesArray.h"
#include "TH1.h"
#include "TH2.h"
#include "TString.h"
#include "TFile.h"
#include "TMath.h"
#include "TRandom.h"
#include "TROOT.h"

//___________________________________________________________________
// Constants definitions: Particles list
const Int_t   kiNbPart = 13;
const TString ksPartTag[kiNbPart] = 
   { "others",
     "ep", "em",   "pip", "pim", "kp", "km", 
     "p",  "pbar", "d",   "t",   "he",  "a" };
const Int_t   kiPartPdgCode[kiNbPart] = 
   {   0,
      21,   -21,    211,   -211,  321,  -321, 
    2212, -2212, 1000010020, 1000010030, 1000020030, 1000020040 };
const TString ksPartName[kiNbPart] = 
   { "any other part.", 
     "e-", "e+",   "#pi+", "#pi-", "k+", "k-", 
     "p",  "anti-p", "d",    "t",    "he", "#alpha"};
//___________________________________________________________________


//___________________________________________________________________
//
// CbmTofHitFinderQa
//
// Task for QA of TOF event based hit finders and digitizers
//
// ------------------------------------------------------------------
CbmTofHitFinderQa::CbmTofHitFinderQa()
  : FairTask("TofHitFinderQa"),
    fEvents(0),
    fMCEventHeader(NULL),
    fGeoHandler(new CbmTofGeoHandler()),
    fTofId(NULL),
    fChannelInfo(NULL),
    iNbSmTot(0),
    fvTypeSmOffs(),
    iNbRpcTot(0),
    fvSmRpcOffs(),
    iNbChTot(0),
    fvRpcChOffs(),
    fDigiPar(NULL),
    fDigiBdfPar(NULL),
    fTofPointsColl(NULL),
    fMcTracksColl(NULL),
    fTofDigisColl(NULL),
    fTofDigiMatchPointsColl(NULL),
    fTofHitsColl(NULL),
    fTofDigiMatchColl(NULL),
    fbNormHistGenMode( kFALSE ),
    fsHistoInNormCartFilename(""),  
    fsHistoInNormAngFilename(""),
    fsHistoInNormSphFilename(""),
    fsHistoOutFilename("./tofQa.hst.root"),
    fdWallPosZ(1000),
    fhTrackMapXY(NULL),  // Only when creating normalization histos
    fhTrackMapXZ(NULL),  // Only when creating normalization histos
    fhTrackMapYZ(NULL),  // Only when creating normalization histos
    fhTrackMapAng(NULL), // Only when creating normalization histos
    fhTrackMapSph(NULL), // Only when creating normalization histos
    fhPointMapXY(NULL),
    fhPointMapXZ(NULL),
    fhPointMapYZ(NULL),
    fhPointMapAng(NULL),
    fhPointMapSph(NULL),
    fhDigiMapXY(NULL),
    fhDigiMapXZ(NULL),
    fhDigiMapYZ(NULL),
    fhDigiMapAng(NULL),
    fhDigiMapSph(NULL),
    fhHitMapXY(NULL),
    fhHitMapXZ(NULL),
    fhHitMapYZ(NULL),
    fhHitMapAng(NULL),
    fhHitMapSph(NULL),
    fhLeftRightDigiMatch(NULL),
    fhNbPointsInHit(NULL),
    fhNbTracksInHit(NULL),
    fhHitMapSingPntXY(NULL),
    fhHitMapSingPntXZ(NULL),
    fhHitMapSingPntYZ(NULL),
    fhHitMapSingPntAng(NULL),
    fhHitMapSingPntSph(NULL),
    fhHitMapMultPntXY(NULL),
    fhHitMapMultPntXZ(NULL),
    fhHitMapMultPntYZ(NULL),
    fhHitMapMultPntAng(NULL),
    fhHitMapMultPntSph(NULL),
    fhHitMapSingTrkXY(NULL),
    fhHitMapSingTrkXZ(NULL),
    fhHitMapSingTrkYZ(NULL),
    fhHitMapSingTrkAng(NULL),
    fhHitMapSingTrkSph(NULL),
    fhHitMapMultTrkXY(NULL),
    fhHitMapMultTrkXZ(NULL),
    fhHitMapMultTrkYZ(NULL),
    fhHitMapMultTrkAng(NULL),
    fhHitMapMultTrkSph(NULL),
    fhSinglePointHitDeltaX(NULL),
    fhSinglePointHitDeltaY(NULL),
    fhSinglePointHitDeltaZ(NULL),
    fhSinglePointHitDeltaR(NULL),
    fhSinglePointHitDeltaT(NULL),
    fhSinglePointHitPullX(NULL),
    fhSinglePointHitPullY(NULL),
    fhSinglePointHitPullZ(NULL),
    fhSinglePointHitPullR(NULL),
    fhMultiPntHitClosestDeltaX(NULL),
    fhMultiPntHitClosestDeltaY(NULL),
    fhMultiPntHitClosestDeltaZ(NULL),
    fhMultiPntHitClosestDeltaR(NULL),
    fhMultiPntHitClosestDeltaT(NULL),
    fhMultiPntHitClosestPullX(NULL),
    fhMultiPntHitClosestPullY(NULL),
    fhMultiPntHitClosestPullZ(NULL),
    fhMultiPntHitClosestPullR(NULL),
    fhMultiPntHitFurthestDeltaX(NULL),
    fhMultiPntHitFurthestDeltaY(NULL),
    fhMultiPntHitFurthestDeltaZ(NULL),
    fhMultiPntHitFurthestDeltaR(NULL),
    fhMultiPntHitFurthestDeltaT(NULL),
    fhMultiPntHitFurthestPullX(NULL),
    fhMultiPntHitFurthestPullY(NULL),
    fhMultiPntHitFurthestPullZ(NULL),
    fhMultiPntHitFurthestPullR(NULL),
    fhMultiPntHitMeanDeltaX(NULL),
    fhMultiPntHitMeanDeltaY(NULL),
    fhMultiPntHitMeanDeltaZ(NULL),
    fhMultiPntHitMeanDeltaR(NULL),
    fhMultiPntHitMeanDeltaT(NULL),
    fhMultiPntHitMeanPullX(NULL),
    fhMultiPntHitMeanPullY(NULL),
    fhMultiPntHitMeanPullZ(NULL),
    fhMultiPntHitMeanPullR(NULL),
    fhSingleTrackHitDeltaX(NULL),
    fhSingleTrackHitDeltaY(NULL),
    fhSingleTrackHitDeltaZ(NULL),
    fhSingleTrackHitDeltaR(NULL),
    fhSingleTrackHitDeltaT(NULL),
    fhSingleTrackHitPullX(NULL),
    fhSingleTrackHitPullY(NULL),
    fhSingleTrackHitPullZ(NULL),
    fhSingleTrackHitPullR(NULL),
    fhSingTrkMultiPntHitDeltaX(NULL),
    fhSingTrkMultiPntHitDeltaY(NULL),
    fhSingTrkMultiPntHitDeltaZ(NULL),
    fhSingTrkMultiPntHitDeltaR(NULL),
    fhSingTrkMultiPntHitDeltaT(NULL),
    fhSingTrkMultiPntHitPullX(NULL),
    fhSingTrkMultiPntHitPullY(NULL),
    fhSingTrkMultiPntHitPullZ(NULL),
    fhSingTrkMultiPntHitPullR(NULL),
    fhMultiTrkHitClosestDeltaX(NULL),
    fhMultiTrkHitClosestDeltaY(NULL),
    fhMultiTrkHitClosestDeltaZ(NULL),
    fhMultiTrkHitClosestDeltaR(NULL),
    fhMultiTrkHitClosestDeltaT(NULL),
    fhMultiTrkHitClosestPullX(NULL),
    fhMultiTrkHitClosestPullY(NULL),
    fhMultiTrkHitClosestPullZ(NULL),
    fhMultiTrkHitClosestPullR(NULL),
    fhMultiTrkHitFurthestDeltaX(NULL),
    fhMultiTrkHitFurthestDeltaY(NULL),
    fhMultiTrkHitFurthestDeltaZ(NULL),
    fhMultiTrkHitFurthestDeltaR(NULL),
    fhMultiTrkHitFurthestDeltaT(NULL),
    fhMultiTrkHitFurthestPullX(NULL),
    fhMultiTrkHitFurthestPullY(NULL),
    fhMultiTrkHitFurthestPullZ(NULL),
    fhMultiTrkHitFurthestPullR(NULL),
    fhMultiTrkHitMeanDeltaX(NULL),
    fhMultiTrkHitMeanDeltaY(NULL),
    fhMultiTrkHitMeanDeltaZ(NULL),
    fhMultiTrkHitMeanDeltaR(NULL),
    fhMultiTrkHitMeanDeltaT(NULL),
    fhMultiTrkHitMeanPullX(NULL),
    fhMultiTrkHitMeanPullY(NULL),
    fhMultiTrkHitMeanPullZ(NULL),
    fhMultiTrkHitMeanPullR(NULL),
    fvhPtmRapGenTrk(),
    fvhPtmRapStsPnt(),
    fvhPtmRapTofPnt(),
    fvhPtmRapTofHit(),
    fvhPtmRapTofHitSinglePnt(),
    fvhPtmRapTofHitSingleTrk(),
    fvhPlabGenTrk(),
    fvhPlabStsPnt(),
    fvhPlabTofPnt(),
    fvhPlabTofHit(),
    fvhPlabTofHitSinglePnt(),
    fvhPlabTofHitSingleTrk(),
    fvhPtmRapGenTrkTofPnt(),
    fvhPtmRapGenTrkTofHit(),
    fvhPlabGenTrkTofPnt(),
    fvhPlabGenTrkTofhit()
{
  cout << "CbmTofHitFinderQa: Task started " << endl;
}
// ------------------------------------------------------------------

// ------------------------------------------------------------------
CbmTofHitFinderQa::CbmTofHitFinderQa(const char* name, Int_t verbose)
  : FairTask(name, verbose),
    fEvents(0),
    fMCEventHeader(NULL),
    fGeoHandler(new CbmTofGeoHandler()),
    fTofId(NULL),
    fChannelInfo(NULL),
    iNbSmTot(0),
    fvTypeSmOffs(),
    iNbRpcTot(0),
    fvSmRpcOffs(),
    iNbChTot(0),
    fvRpcChOffs(),
    fDigiPar(NULL),
    fDigiBdfPar(NULL),
    fTofPointsColl(NULL),
    fMcTracksColl(NULL),
    fTofDigisColl(NULL),
    fTofDigiMatchPointsColl(NULL),
    fTofHitsColl(NULL),
    fTofDigiMatchColl(NULL),
    fbNormHistGenMode( kFALSE ),
    fsHistoInNormCartFilename(""),  
    fsHistoInNormAngFilename(""),
    fsHistoInNormSphFilename(""),
    fsHistoOutFilename("./tofQa.hst.root"),
    fdWallPosZ(1000),
    fhTrackMapXY(NULL),  // Only when creating normalization histos
    fhTrackMapXZ(NULL),  // Only when creating normalization histos
    fhTrackMapYZ(NULL),  // Only when creating normalization histos
    fhTrackMapAng(NULL), // Only when creating normalization histos
    fhTrackMapSph(NULL), // Only when creating normalization histos
    fhPointMapXY(NULL),
    fhPointMapXZ(NULL),
    fhPointMapYZ(NULL),
    fhPointMapAng(NULL),
    fhPointMapSph(NULL),
    fhDigiMapXY(NULL),
    fhDigiMapXZ(NULL),
    fhDigiMapYZ(NULL),
    fhDigiMapAng(NULL),
    fhDigiMapSph(NULL),
    fhHitMapXY(NULL),
    fhHitMapXZ(NULL),
    fhHitMapYZ(NULL),
    fhHitMapAng(NULL),
    fhHitMapSph(NULL),
    fhLeftRightDigiMatch(NULL),
    fhNbPointsInHit(NULL),
    fhNbTracksInHit(NULL),
    fhHitMapSingPntXY(NULL),
    fhHitMapSingPntXZ(NULL),
    fhHitMapSingPntYZ(NULL),
    fhHitMapSingPntAng(NULL),
    fhHitMapSingPntSph(NULL),
    fhHitMapMultPntXY(NULL),
    fhHitMapMultPntXZ(NULL),
    fhHitMapMultPntYZ(NULL),
    fhHitMapMultPntAng(NULL),
    fhHitMapMultPntSph(NULL),
    fhHitMapSingTrkXY(NULL),
    fhHitMapSingTrkXZ(NULL),
    fhHitMapSingTrkYZ(NULL),
    fhHitMapSingTrkAng(NULL),
    fhHitMapSingTrkSph(NULL),
    fhHitMapMultTrkXY(NULL),
    fhHitMapMultTrkXZ(NULL),
    fhHitMapMultTrkYZ(NULL),
    fhHitMapMultTrkAng(NULL),
    fhHitMapMultTrkSph(NULL),
    fhSinglePointHitDeltaX(NULL),
    fhSinglePointHitDeltaY(NULL),
    fhSinglePointHitDeltaZ(NULL),
    fhSinglePointHitDeltaR(NULL),
    fhSinglePointHitDeltaT(NULL),
    fhSinglePointHitPullX(NULL),
    fhSinglePointHitPullY(NULL),
    fhSinglePointHitPullZ(NULL),
    fhSinglePointHitPullR(NULL),
    fhMultiPntHitClosestDeltaX(NULL),
    fhMultiPntHitClosestDeltaY(NULL),
    fhMultiPntHitClosestDeltaZ(NULL),
    fhMultiPntHitClosestDeltaR(NULL),
    fhMultiPntHitClosestDeltaT(NULL),
    fhMultiPntHitClosestPullX(NULL),
    fhMultiPntHitClosestPullY(NULL),
    fhMultiPntHitClosestPullZ(NULL),
    fhMultiPntHitClosestPullR(NULL),
    fhMultiPntHitFurthestDeltaX(NULL),
    fhMultiPntHitFurthestDeltaY(NULL),
    fhMultiPntHitFurthestDeltaZ(NULL),
    fhMultiPntHitFurthestDeltaR(NULL),
    fhMultiPntHitFurthestDeltaT(NULL),
    fhMultiPntHitFurthestPullX(NULL),
    fhMultiPntHitFurthestPullY(NULL),
    fhMultiPntHitFurthestPullZ(NULL),
    fhMultiPntHitFurthestPullR(NULL),
    fhMultiPntHitMeanDeltaX(NULL),
    fhMultiPntHitMeanDeltaY(NULL),
    fhMultiPntHitMeanDeltaZ(NULL),
    fhMultiPntHitMeanDeltaR(NULL),
    fhMultiPntHitMeanDeltaT(NULL),
    fhMultiPntHitMeanPullX(NULL),
    fhMultiPntHitMeanPullY(NULL),
    fhMultiPntHitMeanPullZ(NULL),
    fhMultiPntHitMeanPullR(NULL),
    fhSingleTrackHitDeltaX(NULL),
    fhSingleTrackHitDeltaY(NULL),
    fhSingleTrackHitDeltaZ(NULL),
    fhSingleTrackHitDeltaR(NULL),
    fhSingleTrackHitDeltaT(NULL),
    fhSingleTrackHitPullX(NULL),
    fhSingleTrackHitPullY(NULL),
    fhSingleTrackHitPullZ(NULL),
    fhSingleTrackHitPullR(NULL),
    fhSingTrkMultiPntHitDeltaX(NULL),
    fhSingTrkMultiPntHitDeltaY(NULL),
    fhSingTrkMultiPntHitDeltaZ(NULL),
    fhSingTrkMultiPntHitDeltaR(NULL),
    fhSingTrkMultiPntHitDeltaT(NULL),
    fhSingTrkMultiPntHitPullX(NULL),
    fhSingTrkMultiPntHitPullY(NULL),
    fhSingTrkMultiPntHitPullZ(NULL),
    fhSingTrkMultiPntHitPullR(NULL),
    fhMultiTrkHitClosestDeltaX(NULL),
    fhMultiTrkHitClosestDeltaY(NULL),
    fhMultiTrkHitClosestDeltaZ(NULL),
    fhMultiTrkHitClosestDeltaR(NULL),
    fhMultiTrkHitClosestDeltaT(NULL),
    fhMultiTrkHitClosestPullX(NULL),
    fhMultiTrkHitClosestPullY(NULL),
    fhMultiTrkHitClosestPullZ(NULL),
    fhMultiTrkHitClosestPullR(NULL),
    fhMultiTrkHitFurthestDeltaX(NULL),
    fhMultiTrkHitFurthestDeltaY(NULL),
    fhMultiTrkHitFurthestDeltaZ(NULL),
    fhMultiTrkHitFurthestDeltaR(NULL),
    fhMultiTrkHitFurthestDeltaT(NULL),
    fhMultiTrkHitFurthestPullX(NULL),
    fhMultiTrkHitFurthestPullY(NULL),
    fhMultiTrkHitFurthestPullZ(NULL),
    fhMultiTrkHitFurthestPullR(NULL),
    fhMultiTrkHitMeanDeltaX(NULL),
    fhMultiTrkHitMeanDeltaY(NULL),
    fhMultiTrkHitMeanDeltaZ(NULL),
    fhMultiTrkHitMeanDeltaR(NULL),
    fhMultiTrkHitMeanDeltaT(NULL),
    fhMultiTrkHitMeanPullX(NULL),
    fhMultiTrkHitMeanPullY(NULL),
    fhMultiTrkHitMeanPullZ(NULL),
    fhMultiTrkHitMeanPullR(NULL),
    fvhPtmRapGenTrk(),
    fvhPtmRapStsPnt(),
    fvhPtmRapTofPnt(),
    fvhPtmRapTofHit(),
    fvhPtmRapTofHitSinglePnt(),
    fvhPtmRapTofHitSingleTrk(),
    fvhPlabGenTrk(),
    fvhPlabStsPnt(),
    fvhPlabTofPnt(),
    fvhPlabTofHit(),
    fvhPlabTofHitSinglePnt(),
    fvhPlabTofHitSingleTrk(),
    fvhPtmRapGenTrkTofPnt(),
    fvhPtmRapGenTrkTofHit(),
    fvhPlabGenTrkTofPnt(),
    fvhPlabGenTrkTofhit()
{
}
// ------------------------------------------------------------------

// ------------------------------------------------------------------
CbmTofHitFinderQa::~CbmTofHitFinderQa()
{
    // Destructor
}
// ------------------------------------------------------------------
/************************************************************************************/
// FairTasks inherited functions
InitStatus CbmTofHitFinderQa::Init()
{
   if( kFALSE == RegisterInputs() )
      return kFATAL;

   // Initialize the TOF GeoHandler
   Bool_t isSimulation=kFALSE;
   Int_t iGeoVersion = fGeoHandler->Init(isSimulation);
   LOG(INFO)<<"CbmTofSimpClusterizer::InitParameters with GeoVersion "<<iGeoVersion<<FairLogger::endl;

   if( k12b > iGeoVersion )
   {
      LOG(ERROR)<<"CbmTofSimpClusterizer::InitParameters => Only compatible with geometries after v12b !!!"
                <<FairLogger::endl;
      return kFATAL;
   } // if( k12b > iGeoVersion )
   
   if(NULL != fTofId) 
     LOG(INFO)<<"CbmTofSimpClusterizer::InitParameters with GeoVersion "<<fGeoHandler->GetGeoVersion()<<FairLogger::endl;
   else
   {
      switch(iGeoVersion)
      {
         case k12b: 
            fTofId = new CbmTofDetectorId_v12b();
            break;
         case k14a:
            fTofId = new CbmTofDetectorId_v14a();
            break;
         default:
            LOG(ERROR)<<"CbmTofSimpClusterizer::InitParameters => Invalid geometry!!!"<<iGeoVersion
                      <<FairLogger::endl;
         return kFATAL;
      } // switch(iGeoVersion)
   } // else of if(NULL != fTofId) 

   if( kFALSE == LoadGeometry() )
      return kFATAL;

   if( kFALSE == CreateHistos() )
      return kFATAL;

   return kSUCCESS;
}

void CbmTofHitFinderQa::SetParContainers()
{
   LOG(INFO)<<" CbmTofHitFinderQa => Get the digi parameters for tof"<<FairLogger::endl;

   // Get Base Container
   FairRunAna* ana = FairRunAna::Instance();
   FairRuntimeDb* rtdb=ana->GetRuntimeDb();

   fDigiPar = (CbmTofDigiPar*) (rtdb->getContainer("CbmTofDigiPar"));

   fDigiBdfPar = (CbmTofDigiBdfPar*) (rtdb->getContainer("CbmTofDigiBdfPar"));
}

void CbmTofHitFinderQa::Exec(Option_t * /*option*/)
{
   // Task execution

   LOG(DEBUG)<<" CbmTofHitFinderQa => New event"<<FairLogger::endl;

   FillHistos();

   if( 0 == ( fEvents%100 ) && 0 < fEvents )
   {
      cout << "-I- CbmTofHitFinderQa::Exec : "
           << "event " << fEvents << " processed." << endl;
   }
   fEvents += 1;
}

void CbmTofHitFinderQa::Finish()
{
   // Normalisations
   cout << "CbmTofHitFinderQa::Finish up with " << fEvents << " analyzed events " << endl;

   if( kFALSE == fbNormHistGenMode )
      NormalizeMapHistos();
      else NormalizeNormHistos();

   WriteHistos();
   // Prevent them from being sucked in by the CbmHadronAnalysis WriteHistograms method
   DeleteHistos();
}

/************************************************************************************/
// Functions common for all clusters approximations
Bool_t   CbmTofHitFinderQa::RegisterInputs()
{
   FairRootManager *fManager = FairRootManager::Instance();
   
   fTofPointsColl  = (TClonesArray *) fManager->GetObject("TofPoint");
   if( NULL == fTofPointsColl)
   {
      LOG(ERROR)<<"CbmTofHitFinderQa::RegisterInputs => Could not get the TofPoint TClonesArray!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fTofPointsColl)

   fMcTracksColl   = (TClonesArray *) fManager->GetObject("MCTrack");
   if( NULL == fMcTracksColl)
   {
      LOG(ERROR)<<"CbmTofHitFinderQa::RegisterInputs => Could not get the MCTrack TClonesArray!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fMcTracksColl)

   fTofDigisColl   = (TClonesArray *) fManager->GetObject("TofDigi");
   if( NULL == fTofDigisColl)
   {
      LOG(ERROR)<<"CbmTofHitFinderQa::RegisterInputs => Could not get the TofDigi TClonesArray!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fTofDigisColl)
   fTofDigiMatchPointsColl   = (TClonesArray *) fManager->GetObject("TofDigiMatchPoints");
   if( NULL == fTofDigiMatchPointsColl)
   {
      LOG(ERROR)<<"CbmTofHitFinderQa::RegisterInputs => Could not get the TofDigiMatchPoints TClonesArray!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fTofDigiMatchPointsColl)

   fTofHitsColl   = (TClonesArray *) fManager->GetObject("TofHit");
   if( NULL == fTofHitsColl)
   {
      LOG(ERROR)<<"CbmTofHitFinderQa::RegisterInputs => Could not get the TofHit TClonesArray!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fTofHitsColl)
   fTofDigiMatchColl   = (TClonesArray *) fManager->GetObject("TofDigiMatch");
   if( NULL == fTofDigiMatchColl)
   {
      LOG(ERROR)<<"CbmTofHitFinderQa::RegisterInputs => Could not get the TofDigiMatch TClonesArray!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fTofDigiMatchColl)

   return kTRUE;
}
/************************************************************************************/
Bool_t   CbmTofHitFinderQa::LoadGeometry()
{
   /*
     Type 0: 5 RPC/SM,  24 SM, 32 ch/RPC =>  3840 ch          , 120 RPC         ,
     Type 1: 5 RPC/SM, 142 SM, 32 ch/RPC => 22720 ch => 26560 , 710 RPC =>  830 , => 166
     Type 3: 3 RPC/SM,  50 SM, 56 ch/RPC =>  8400 ch => 34960 , 150 RPC =>  980 , => 216
     Type 4: 5 RPC/SM,   8 SM, 96 ch/RPC =>  3840 ch => 38800 ,  40 RPC => 1020 , => 224
     Type 5: 5 RPC/SM,   8 SM, 96 ch/RPC =>  3840 ch => 42640 ,  40 RPC => 1060 , => 232
     Type 6: 2 RPC/SM,  10 SM, 96 ch/RPC =>  1920 ch => 44560 ,  20 RPC => 1080 , => 242
   */

   // Count the total number of channels and
   // generate an array with the global channel index of the first channe in each RPC
   Int_t iNbSmTypes = fDigiBdfPar->GetNbSmTypes();
   fvTypeSmOffs.resize( iNbSmTypes );
   fvSmRpcOffs.resize( iNbSmTypes );
   fvRpcChOffs.resize( iNbSmTypes );
   iNbSmTot  = 0;
   iNbRpcTot = 0;
   iNbChTot  = 0;
   for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
   {
      Int_t iNbSm  = fDigiBdfPar->GetNbSm(  iSmType);
      Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType);

      fvTypeSmOffs[iSmType] = iNbSmTot;
      iNbSmTot += iNbSm;

      fvSmRpcOffs[iSmType].resize( iNbSm );
      fvRpcChOffs[iSmType].resize( iNbSm );

      for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
      {
         fvSmRpcOffs[iSmType][iSm] = iNbRpcTot;
         iNbRpcTot += iNbRpc;

         fvRpcChOffs[iSmType][iSm].resize( iNbRpc );
         for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ )
         {
            fvRpcChOffs[iSmType][iSm][iRpc] = iNbChTot;
            iNbChTot += fDigiBdfPar->GetNbChan( iSmType, iRpc );
         } // for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ )
      } // for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
   } // for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )

   return kTRUE;
}
/************************************************************************************/
// ------------------------------------------------------------------
Bool_t CbmTofHitFinderQa::SetWallPosZ( Double_t dWallPosCm)
{
   fdWallPosZ = dWallPosCm;
   LOG(INFO)<<"CbmTofHitFinderQa::SetWallPosZ => Change histograms center on Z axis to "<<dWallPosCm<<" cm"<<FairLogger::endl;
   return kTRUE;
}
Bool_t CbmTofHitFinderQa::CreateHistos()
{
  // Create histogramms

   TDirectory * oldir = gDirectory; // <= To prevent histos from being sucked in by the param file of the TRootManager!
   gROOT->cd(); // <= To prevent histos from being sucked in by the param file of the TRootManager !
/*
   Double_t ymin=-1.;
   Double_t ymax=4.;
   Double_t ptmmax=2.5;
   Int_t    ptm_nbx=30;
   Int_t    ptm_nby=30;

   Double_t v1_nbx=20.;
   Double_t v1_nby=20.;
   Double_t yvmax=1.3;
*/
   // xy - hit densities and rates
   Int_t nbinx=1500;
   Int_t nbiny=1000;
   Int_t nbinz=1500;
   Double_t xrange=750.;
   Double_t yrange=500.;
   Double_t zmin  = fdWallPosZ -  50.;
   Double_t zmax  = fdWallPosZ + 100.;

   // angular densities for overlap check
   Int_t iNbBinThetaX  = 1200;
   Double_t dThetaXMin = -  60.0;
   Double_t dThetaXMax =    60.0;
   Int_t iNbBinThetaY  = 800;
   Double_t dThetaYMin = -  40.0;
   Double_t dThetaYMax =    40.0;

   Int_t iNbBinTheta   = 180;
   Double_t dThetaMin  =   0;
   Double_t dThetaMax  =  TMath::Pi()*90/180;
   Int_t iNbBinPhi     = 180;
   Double_t dPhiMin    = - TMath::Pi();
   Double_t dPhiMax    =   TMath::Pi();
   
   // Range hit deviation from MC
   Int_t    iNbBinsDeltaPos  =  600;
   Double_t dDeltaPosRange   =   30; // cm
   Int_t    iNbBinsDeltaTime = 1000;
   Double_t dDeltaTimeRange  = 1000; // ps 
   Int_t    iNbBinsMulti     =   18;
   Double_t iMinMulti        =    2 - 0.5;
   Double_t iMaxMulti        =   19 + 0.5;
   // Range pulls from MC
   Int_t    iNbBinsPullPos  =  500;
   Double_t dPullPosRange   =    5; 

   // Mapping
      // tracks: Only when creating normalization histos
   if( kTRUE == fbNormHistGenMode )
   {
      fhTrackMapXY     = new TH2D("TofTests_TracksMapXY",  "Position of the MC Tracks assuming along Z axis; X[cm]; Y[cm]; # [Tracks]",
                                 nbinx,-xrange,xrange,nbiny,-yrange,yrange);
      fhTrackMapXZ     = new TH2D("TofTests_TracksMapXZ",  "Position of the MC Tracks assuming along Z axis; X[cm]; Z[cm]; # [Tracks]",
                                 nbinx,-xrange,xrange,nbinz,zmin,zmax);
      fhTrackMapYZ     = new TH2D("TofTests_TracksMapYZ",  "Position of the MC Tracks assuming along Z axis; Y[cm]; Z[cm]; # [Tracks]",
                                 nbiny,-yrange,yrange,nbinz,zmin,zmax);
      fhTrackMapAng    = new TH2D("TofTests_TracksMapAng",  "Position of the MC Tracks assuming from origin; #theta_{x}[Deg.]; #theta_{y}[Deg.]; # [Tracks]",
                                 iNbBinThetaX, dThetaXMin, dThetaXMax,
                                 iNbBinThetaY, dThetaYMin, dThetaYMax);
      fhTrackMapSph    = new TH2D("TofTests_TracksMapSph",  "Position of the MC Tracks assuming from origin; #theta[rad.]; #phi[rad.]; # [Tracks]",
                                 iNbBinTheta, dThetaMin, dThetaMax,
                                 iNbBinPhi,   dPhiMin,   dPhiMax);
   } // if( kTRUE == fbNormHistGenMode )
      // points
   fhPointMapXY     = new TH2D("TofTests_PointsMapXY",  "Position of the Tof Points; X[cm]; Y[cm]; # [Points]",
                              nbinx,-xrange,xrange,nbiny,-yrange,yrange);
   fhPointMapXZ     = new TH2D("TofTests_PointsMapXZ",  "Position of the Tof Points; X[cm]; Z[cm]; # [Points]",
                              nbinx,-xrange,xrange,nbinz,zmin,zmax);
   fhPointMapYZ     = new TH2D("TofTests_PointsMapYZ",  "Position of the Tof Points; Y[cm]; Z[cm]; # [Points]",
                              nbiny,-yrange,yrange,nbinz,zmin,zmax);
   fhPointMapAng    = new TH2D("TofTests_PointsMapAng",  "Position of the Tof Points; #theta_{x}[Deg.]; #theta_{y}[Deg.]; # [Points]",
                              iNbBinThetaX, dThetaXMin, dThetaXMax,
                              iNbBinThetaY, dThetaYMin, dThetaYMax);
   fhPointMapSph    = new TH2D("TofTests_PointsMapSph",  "Position of the Tof Points; #theta[rad.]; #phi[rad.]; # [Points]",
                              iNbBinTheta, dThetaMin, dThetaMax,
                              iNbBinPhi,   dPhiMin,   dPhiMax);
      // Digis
   fhDigiMapXY     = new TH2D("TofTests_DigisMapXY",  "Position of the Tof Digis; X[cm]; Y[cm]; # [Digi]",
                              nbinx,-xrange,xrange,nbiny,-yrange,yrange);
   fhDigiMapXZ     = new TH2D("TofTests_DigisMapXZ",  "Position of the Tof Digis; X[cm]; Z[cm]; # [Digi]",
                              nbinx,-xrange,xrange,nbinz,zmin,zmax);
   fhDigiMapYZ     = new TH2D("TofTests_DigisMapYZ",  "Position of the Tof Digis; Y[cm]; Z[cm]; # [Digi]",
                              nbiny,-yrange,yrange,nbinz,zmin,zmax);
   fhDigiMapAng    = new TH2D("TofTests_DigisMapAng",  "Position of the Tof Digis; #theta_{x}[Deg.]; #theta_{y}[Deg.]; # [Digi]",
                              iNbBinThetaX, dThetaXMin, dThetaXMax,
                              iNbBinThetaY, dThetaYMin, dThetaYMax);
   fhDigiMapSph    = new TH2D("TofTests_DigisMapSph",  "Position of the Tof Digis; #theta[rad.]; #phi[rad.]; # [Points]",
                              iNbBinTheta, dThetaMin, dThetaMax,
                              iNbBinPhi,   dPhiMin,   dPhiMax);

      // Hits
   fhHitMapXY      = new TH2D("TofTests_HitsMapXY",   "Position of the Tof Hits; X[cm]; Y[cm]; # [Hits]",
                              nbinx,-xrange,xrange,nbiny,-yrange,yrange);
   fhHitMapXZ      = new TH2D("TofTests_HitsMapXZ",   "Position of the Tof Hits; X[cm]; Z[cm]; # [Hits]",
                              nbinx,-xrange,xrange,nbinz,zmin,zmax);
   fhHitMapYZ      = new TH2D("TofTests_HitsMapYZ",   "Position of the Tof Hits; Y[cm]; Z[cm]; # [Hits]",
                              nbiny,-yrange,yrange,nbinz,zmin,zmax);
   fhHitMapAng     = new TH2D("TofTests_HitsMapAng",  "Position of the Tof Hits; #theta_{x}[Deg.]; #theta_{y}[Deg.]; # [Hits]",
                              iNbBinThetaX, dThetaXMin, dThetaXMax,
                              iNbBinThetaY, dThetaYMin, dThetaYMax);
   fhHitMapSph     = new TH2D("TofTests_HitsMapSph",  "Position of the Tof Hits; #theta[rad.]; #phi[rad.]; # [Points]",
                              iNbBinTheta, dThetaMin, dThetaMax,
                              iNbBinPhi,   dPhiMin,   dPhiMax);

   // L/R digis missmatch
   fhLeftRightDigiMatch = new TH2D("TofTests_DigiMatchLR","Matching between the left an right digi for strips added to hit; Global Strip Index []",
                              iNbChTot, 0, iNbChTot,
                              3, -0.5, 2.5);
   fhLeftRightDigiMatch->GetYaxis()->SetBinLabel( 1 , "Same MC Point");
   fhLeftRightDigiMatch->GetYaxis()->SetBinLabel( 2 , "Same MC Track");
   fhLeftRightDigiMatch->GetYaxis()->SetBinLabel( 3 , "Diff MC Track");
   
   // Nb different MC Points and Tracks in Hit
   fhNbPointsInHit = new TH1D("TofTests_NbPointsInHit", "Number of different MC Points in Hit; # [MC Points]",
                              100, -0.5, 99.5 );
   fhNbTracksInHit = new TH1D("TofTests_NbTracksInHit", "Number of different MC Tracks in Hit; # [MC Tracks]",
                              100, -0.5, 99.5 );

      // Mapping of position for hits coming from a single MC Point
   fhHitMapSingPntXY      = new TH2D("TofTests_HitsMapSingPntXY",   "Position of the Tof Hits from a single MC Point; X[cm]; Y[cm]; # [Hits]",
                              nbinx,-xrange,xrange,nbiny,-yrange,yrange);
   fhHitMapSingPntXZ      = new TH2D("TofTests_HitsMapSingPntXZ",   "Position of the Tof Hits from a single MC Point; X[cm]; Z[cm]; # [Hits]",
                              nbinx,-xrange,xrange,nbinz,zmin,zmax);
   fhHitMapSingPntYZ      = new TH2D("TofTests_HitsMapSingPntYZ",   "Position of the Tof Hits from a single MC Point; Y[cm]; Z[cm]; # [Hits]",
                              nbiny,-yrange,yrange,nbinz,zmin,zmax);
   fhHitMapSingPntAng     = new TH2D("TofTests_HitsMapSingPntAng",  "Position of the Tof Hits from a single MC Point; #theta_{x}[Deg.]; #theta_{y}[Deg.]; # [Hits]",
                              iNbBinThetaX, dThetaXMin, dThetaXMax,
                              iNbBinThetaY, dThetaYMin, dThetaYMax);
   fhHitMapSingPntSph     = new TH2D("TofTests_HitsMapSingPntSph",  "Position of the Tof Hits from a single MC Point; #theta[rad.]; #phi[rad.]; # [Points]",
                              iNbBinTheta, dThetaMin, dThetaMax,
                              iNbBinPhi,   dPhiMin,   dPhiMax);
      // Mapping of position for hits coming from multiple MC Points
   fhHitMapMultPntXY      = new TH2D("TofTests_HitsMapMultPntXY",   "Position of the Tof Hits from multiple MC Points; X[cm]; Y[cm]; # [Hits]",
                              nbinx,-xrange,xrange,nbiny,-yrange,yrange);
   fhHitMapMultPntXZ      = new TH2D("TofTests_HitsMapMultPntXZ",   "Position of the Tof Hits from multiple MC Points; X[cm]; Z[cm]; # [Hits]",
                              nbinx,-xrange,xrange,nbinz,zmin,zmax);
   fhHitMapMultPntYZ      = new TH2D("TofTests_HitsMapMultPntYZ",   "Position of the Tof Hits from multiple MC Points; Y[cm]; Z[cm]; # [Hits]",
                              nbiny,-yrange,yrange,nbinz,zmin,zmax);
   fhHitMapMultPntAng     = new TH2D("TofTests_HitsMapMultPntAng",  "Position of the Tof Hits from multiple MC Points; #theta_{x}[Deg.]; #theta_{y}[Deg.]; # [Hits]",
                              iNbBinThetaX, dThetaXMin, dThetaXMax,
                              iNbBinThetaY, dThetaYMin, dThetaYMax);
   fhHitMapMultPntSph     = new TH2D("TofTests_HitsMapMultPntSph",  "Position of the Tof Hits from multiple MC Points; #theta[rad.]; #phi[rad.]; # [Points]",
                              iNbBinTheta, dThetaMin, dThetaMax,
                              iNbBinPhi,   dPhiMin,   dPhiMax);
      // Mapping of position for hits coming from a single MC Track
   fhHitMapSingTrkXY      = new TH2D("TofTests_HitsMapSingTrkXY",   "Position of the Tof Hits from a single MC Track; X[cm]; Y[cm]; # [Hits]",
                              nbinx,-xrange,xrange,nbiny,-yrange,yrange);
   fhHitMapSingTrkXZ      = new TH2D("TofTests_HitsMapSingTrkXZ",   "Position of the Tof Hits from a single MC Track; X[cm]; Z[cm]; # [Hits]",
                              nbinx,-xrange,xrange,nbinz,zmin,zmax);
   fhHitMapSingTrkYZ      = new TH2D("TofTests_HitsMapSingTrkYZ",   "Position of the Tof Hits from a single MC Track; Y[cm]; Z[cm]; # [Hits]",
                              nbiny,-yrange,yrange,nbinz,zmin,zmax);
   fhHitMapSingTrkAng     = new TH2D("TofTests_HitsMapSingTrkAng",  "Position of the Tof Hits from a single MC Track; #theta_{x}[Deg.]; #theta_{y}[Deg.]; # [Hits]",
                              iNbBinThetaX, dThetaXMin, dThetaXMax,
                              iNbBinThetaY, dThetaYMin, dThetaYMax);
   fhHitMapSingTrkSph     = new TH2D("TofTests_HitsMapSingTrkSph",  "Position of the Tof Hits from a single MC Track; #theta[rad.]; #phi[rad.]; # [Points]",
                              iNbBinTheta, dThetaMin, dThetaMax,
                              iNbBinPhi,   dPhiMin,   dPhiMax);
      // Mapping of position for hits coming from multiple MC Tracks
   fhHitMapMultTrkXY      = new TH2D("TofTests_HitsMapMultTrkXY",   "Position of the Tof Hits from multiple MC Tracks; X[cm]; Y[cm]; # [Hits]",
                              nbinx,-xrange,xrange,nbiny,-yrange,yrange);
   fhHitMapMultTrkXZ      = new TH2D("TofTests_HitsMapMultTrkXZ",   "Position of the Tof Hits from multiple MC Tracks; X[cm]; Z[cm]; # [Hits]",
                              nbinx,-xrange,xrange,nbinz,zmin,zmax);
   fhHitMapMultTrkYZ      = new TH2D("TofTests_HitsMapMultTrkYZ",   "Position of the Tof Hits from multiple MC Tracks; Y[cm]; Z[cm]; # [Hits]",
                              nbiny,-yrange,yrange,nbinz,zmin,zmax);
   fhHitMapMultTrkAng     = new TH2D("TofTests_HitsMapMultTrkAng",  "Position of the Tof Hits from multiple MC Tracks; #theta_{x}[Deg.]; #theta_{y}[Deg.]; # [Hits]",
                              iNbBinThetaX, dThetaXMin, dThetaXMax,
                              iNbBinThetaY, dThetaYMin, dThetaYMax);
   fhHitMapMultTrkSph     = new TH2D("TofTests_HitsMapMultTrkSph",  "Position of the Tof Hits from multiple MC Tracks; #theta[rad.]; #phi[rad.]; # [Points]",
                              iNbBinTheta, dThetaMin, dThetaMax,
                              iNbBinPhi,   dPhiMin,   dPhiMax);

                              
   // Hit Quality for Hits coming from a single MC Point
   fhSinglePointHitDeltaX = new TH1D("TofTests_SinglePointHitDeltaX", 
                              "Quality of the Tof Hits position on X axis, for hit coming from a single MC Point; X(Hit) - X(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSinglePointHitDeltaY = new TH1D("TofTests_SinglePointHitDeltaY", 
                              "Quality of the Tof Hits position on Y axis, for hit coming from a single MC Point; Y(Hit) - Y(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSinglePointHitDeltaZ = new TH1D("TofTests_SinglePointHitDeltaZ", 
                              "Quality of the Tof Hits position on Z axis, for hit coming from a single MC Point; Z(Hit) - Z(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSinglePointHitDeltaR = new TH1D("TofTests_SinglePointHitDeltaR", 
                              "Quality of the Tof Hits position, for hit coming from a single MC Point; R(Hit -> Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSinglePointHitDeltaT = new TH1D("TofTests_SinglePointHitDeltaT", 
                              "Quality of the Tof Hits Time, for hit coming from a single MC Point; T(hit) - T(Point) [ps]; # [Hits]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange);
   fhSinglePointHitPullX = new TH1D("TofTests_SinglePointHitPullX", 
                              "Quality of the Tof Hits position error on X axis, for hit coming from a single MC Point; Pull X(Hit -> Point) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhSinglePointHitPullY = new TH1D("TofTests_SinglePointHitPullY", 
                              "Quality of the Tof Hits position error on Y axis, for hit coming from a single MC Point; Pull Y(Hit -> Point) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhSinglePointHitPullZ = new TH1D("TofTests_SinglePointHitPullZ", 
                              "Quality of the Tof Hits position error on Z axis, for hit coming from a single MC Point; Pull Z(Hit -> Point) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhSinglePointHitPullR = new TH1D("TofTests_SinglePointHitPullR", 
                              "Quality of the Tof Hits position error, for hit coming from a single MC Point; Pull R(Hit -> Point) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);

   // Hit Quality for Hits coming from a multiple MC Points
      // To Point closest to Hit
   fhMultiPntHitClosestDeltaX  = new TH2D("TofTests_MultiPntHitClosestDeltaX", 
                "Quality of the Tof Hits position on X axis relative to closest Point, for hit coming from multiple MC Point; X(Hit) - X(Point) [cm]; # [Hits]; Multi [Pnt]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitClosestDeltaY  = new TH2D("TofTests_MultiPntHitClosestDeltaY", 
                "Quality of the Tof Hits position on Y axis relative to closest Point, for hit coming from multiple MC Point; Y(Hit) - Y(Point) [cm]; # [Hits]; Multi [Pnt]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitClosestDeltaZ  = new TH2D("TofTests_MultiPntHitClosestDeltaZ", 
                "Quality of the Tof Hits position on Z axis relative to closest Point, for hit coming from multiple MC Point; Z(Hit) - Z(Point) [cm]; # [Hits]; Multi [Pnt]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitClosestDeltaR  = new TH2D("TofTests_MultiPntHitClosestDeltaR", 
                "Quality of the Tof Hits position relative to closest Point, for hit coming from multiple MC Point; R(Hit -> Point) [cm]; # [Hits]; Multi [Pnt]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitClosestDeltaT = new TH2D("TofTests_MultiPntHitClosestDeltaT", 
                "Quality of the Tof Hits Time relative to closest Point, for hit coming from multiple MC Point; T(hit) - T(Point) [ps]; # [Hits]; Multi [Pnt]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitClosestPullX  = new TH2D("TofTests_MultiPntHitClosestPullX", 
                "Quality of the Tof Hits position error on X axis relative to closest Point, for hit coming from multiple MC Point; Pull X(Hit -> Point) []; # [Hits]; Multi [Pnt]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitClosestPullY  = new TH2D("TofTests_MultiPntHitClosestPullY", 
                "Quality of the Tof Hits position error on Y axis relative to closest Point, for hit coming from multiple MC Point; Pull Y(Hit -> Point) []; # [Hits]; Multi [Pnt]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitClosestPullZ  = new TH2D("TofTests_MultiPntHitClosestPullZ", 
                "Quality of the Tof Hits position error on Z axis relative to closest Point, for hit coming from multiple MC Point; Pull Z(Hit -> Point) []; # [Hits]; Multi [Pnt]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitClosestPullR  = new TH2D("TofTests_MultiPntHitClosestPullR", 
                "Quality of the Tof Hits position error relative to closest Point, for hit coming from multiple MC Point; Pull R(Hit -> Point) []; # [Hits]; Multi [Pnt]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
      // To Point furthest from Hit
   fhMultiPntHitFurthestDeltaX  = new TH2D("TofTests_MultiPntHitFurthestDeltaX", 
                "Quality of the Tof Hits position on X axis relative to furthest Point, for hit coming from multiple MC Point; X(Hit) - X(Point) [cm]; # [Hits]; Multi [Pnt]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitFurthestDeltaY  = new TH2D("TofTests_MultiPntHitFurthestDeltaY", 
                "Quality of the Tof Hits position on Y axis relative to furthest Point, for hit coming from multiple MC Point; Y(Hit) - Y(Point) [cm]; # [Hits]; Multi [Pnt]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitFurthestDeltaZ  = new TH2D("TofTests_MultiPntHitFurthestDeltaZ", 
                "Quality of the Tof Hits position on Z axis relative to furthest Point, for hit coming from multiple MC Point; Z(Hit) - Z(Point) [cm]; # [Hits]; Multi [Pnt]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitFurthestDeltaR  = new TH2D("TofTests_MultiPntHitFurthestDeltaR", 
                "Quality of the Tof Hits position relative to furthest Point, for hit coming from multiple MC Point; R(Hit -> Point) [cm]; # [Hits]; Multi [Pnt]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitFurthestDeltaT = new TH2D("TofTests_MultiPntHitFurthestDeltaT", 
                "Quality of the Tof Hits Time relative to furthest Point, for hit coming from multiple MC Point; T(hit) - T(Point) [ps]; # [Hits]; Multi [Pnt]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitFurthestPullX  = new TH2D("TofTests_MultiPntHitFurthestPullX", 
                "Quality of the Tof Hits position error on X axis relative to furthest Point, for hit coming from multiple MC Point; Pull X(Hit -> Point) []; # [Hits]; Multi [Pnt]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitFurthestPullY  = new TH2D("TofTests_MultiPntHitFurthestPullY", 
                "Quality of the Tof Hits position error on Y axis relative to furthest Point, for hit coming from multiple MC Point; Pull Y(Hit -> Point) []; # [Hits]; Multi [Pnt]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitFurthestPullZ  = new TH2D("TofTests_MultiPntHitFurthestPullZ", 
                "Quality of the Tof Hits position error on Z axis relative to furthest Point, for hit coming from multiple MC Point; Pull Z(Hit -> Point) []; # [Hits]; Multi [Pnt]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitFurthestPullR  = new TH2D("TofTests_MultiPntHitFurthestPullR", 
                "Quality of the Tof Hits position error relative to furthest Point, for hit coming from multiple MC Point; Pull R(Hit -> Point) []; # [Hits]; Multi [Pnt]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
      // To mean Point position
   fhMultiPntHitMeanDeltaX  = new TH2D("TofTests_MultiPntHitMeanDeltaX", 
                "Quality of the Tof Hits position on X axis relative to mean Point position, for hit coming from multiple MC Point; X(Hit) - X(Point) [cm]; # [Hits]; Multi [Pnt]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitMeanDeltaY  = new TH2D("TofTests_MultiPntHitMeanDeltaY", 
                "Quality of the Tof Hits position on Y axis relative to mean Point position, for hit coming from multiple MC Point; Y(Hit) - Y(Point) [cm]; # [Hits]; Multi [Pnt]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitMeanDeltaZ  = new TH2D("TofTests_MultiPntHitMeanDeltaZ", 
                "Quality of the Tof Hits position on Z axis relative to mean Point position, for hit coming from multiple MC Point; Z(Hit) - Z(Point) [cm]; # [Hits]; Multi [Pnt]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitMeanDeltaR  = new TH2D("TofTests_MultiPntHitMeanDeltaR", 
                "Quality of the Tof Hits position relative to mean Point position, for hit coming from multiple MC Point; R(Hit -> Point) [cm]; # [Hits]; Multi [Pnt]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitMeanDeltaT = new TH2D("TofTests_MultiPntHitMeanDeltaT", 
                "Quality of the Tof Hits Time relative to mean Point time, for hit coming from multiple MC Point; T(hit) - T(Point) [ps]; # [Hits]; Multi [Pnt]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitMeanPullX  = new TH2D("TofTests_MultiPntHitMeanPullX", 
                "Quality of the Tof Hits position error on X axis relative to mean Point position, for hit coming from multiple MC Point; Pull X(Hit -> Point) []; # [Hits]; Multi [Pnt]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitMeanPullY  = new TH2D("TofTests_MultiPntHitMeanPullY", 
                "Quality of the Tof Hits position error on Y axis relative to mean Point position, for hit coming from multiple MC Point; Pull Y(Hit -> Point) []; # [Hits]; Multi [Pnt]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitMeanPullZ  = new TH2D("TofTests_MultiPntHitMeanPullZ", 
                "Quality of the Tof Hits position error on Z axis relative to mean Point position, for hit coming from multiple MC Point; Pull Z(Hit -> Point) []; # [Hits]; Multi [Pnt]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiPntHitMeanPullR  = new TH2D("TofTests_MultiPntHitMeanPullR", 
                "Quality of the Tof Hits position error relative to mean Point position, for hit coming from multiple MC Point; Pull R(Hit -> Point) []; # [Hits]; Multi [Pnt]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );

   // Hit Quality for Hits coming from a single MC Track
   fhSingleTrackHitDeltaX = new TH1D("TofTests_SingleTrackHitDeltaX", 
                              "Quality of the Tof Hits position on X axis, for hit coming from a single MC Track; X(Hit) - X(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSingleTrackHitDeltaY = new TH1D("TofTests_SingleTrackHitDeltaY", 
                              "Quality of the Tof Hits position on Y axis, for hit coming from a single MC Track; Y(Hit) - X(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSingleTrackHitDeltaZ = new TH1D("TofTests_SingleTrackHitDeltaZ", 
                              "Quality of the Tof Hits position on Z axis, for hit coming from a single MC Track; Z(Hit) - X(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSingleTrackHitDeltaR = new TH1D("TofTests_SingleTrackHitDeltaR", 
                              "Quality of the Tof Hits position, for hit coming from a single MC Track; R(Hit -> Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSingleTrackHitDeltaT = new TH1D("TofTests_SingleTrackHitDeltaT", 
                              "Quality of the Tof Hits Time, for hit coming from a single MC Track; T(hit) - T(Track) [ps]; # [Hits]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange);
   fhSingleTrackHitPullX = new TH1D("TofTests_SingleTrackHitPullX", 
                              "Quality of the Tof Hits position error on X axis, for hit coming from a single MC Track; Pull X(Hit -> Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhSingleTrackHitPullY = new TH1D("TofTests_SingleTrackHitPullY", 
                              "Quality of the Tof Hits position error on Y axis, for hit coming from a single MC Track; Y(Hit) - X(Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhSingleTrackHitPullZ = new TH1D("TofTests_SingleTrackHitPullZ", 
                              "Quality of the Tof Hits position error on Z axis, for hit coming from a single MC Track; Z(Hit) - X(Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhSingleTrackHitPullR = new TH1D("TofTests_SingleTrackHitPullR", 
                              "Quality of the Tof Hits position error, for hit coming from a single MC Track; Pull R(Hit -> Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);

   // Hit Quality for Hits coming from a single MC Track but multiple points
   fhSingTrkMultiPntHitDeltaX = new TH1D("TofTests_SingTrkMultiPntHitDeltaX",
                              "Quality of the Tof Hits position on X axis, for hit coming from a single MC Track; X(Hit) - X(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSingTrkMultiPntHitDeltaY = new TH1D("TofTests_SingTrkMultiPntHitDeltaY",
                              "Quality of the Tof Hits position on Y axis, for hit coming from a single MC Track; Y(Hit) - X(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSingTrkMultiPntHitDeltaZ = new TH1D("TofTests_SingTrkMultiPntHitDeltaZ",
                              "Quality of the Tof Hits position on Z axis, for hit coming from a single MC Track; Z(Hit) - X(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSingTrkMultiPntHitDeltaR = new TH1D("TofTests_SingTrkMultiPntHitDeltaR",
                              "Quality of the Tof Hits position, for hit coming from a single MC Track; R(Hit -> Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSingTrkMultiPntHitDeltaT = new TH1D("TofTests_SingTrkMultiPntHitDeltaT",
                              "Quality of the Tof Hits Time, for hit coming from a single MC Track; T(hit) - T(Track) [ps]; # [Hits]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange);
   fhSingTrkMultiPntHitPullX  = new TH1D("TofTests_SingTrkMultiPntHitPullX",
                              "Quality of the Tof Hits position error on X axis, for hit coming from a single MC Track; Pull X(Hit -> Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhSingTrkMultiPntHitPullY  = new TH1D("TofTests_SingTrkMultiPntHitPullY",
                              "Quality of the Tof Hits position error on Y axis, for hit coming from a single MC Track; Y(Hit) - X(Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhSingTrkMultiPntHitPullZ  = new TH1D("TofTests_SingTrkMultiPntHitPullZ",
                              "Quality of the Tof Hits position error on Z axis, for hit coming from a single MC Track; Z(Hit) - X(Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhSingTrkMultiPntHitPullR  = new TH1D("TofTests_SingTrkMultiPntHitPullR",
                              "Quality of the Tof Hits position error, for hit coming from a single MC Track; Pull R(Hit -> Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);

   // Hit Quality for Hits coming from a multiple MC Tracks
      // To Point closest to Hit
   fhMultiTrkHitClosestDeltaX  = new TH2D("TofTests_MultiTrkHitClosestDeltaX", 
                "Quality of the Tof Hits position on X axis relative to closest Track, for hit coming from multiple MC Track; X(Hit) - X(Track) [cm]; # [Hits]; Multi [Trk]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitClosestDeltaY  = new TH2D("TofTests_MultiTrkHitClosestDeltaY", 
                "Quality of the Tof Hits position on Y axis relative to closest Track, for hit coming from multiple MC Track; Y(Hit) - Y(Track) [cm]; # [Hits]; Multi [Trk]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitClosestDeltaZ  = new TH2D("TofTests_MultiTrkHitClosestDeltaZ", 
                "Quality of the Tof Hits position on Z axis relative to closest Track, for hit coming from multiple MC Track; Z(Hit) - Z(Track) [cm]; # [Hits]; Multi [Trk]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitClosestDeltaR  = new TH2D("TofTests_MultiTrkHitClosestDeltaR", 
                "Quality of the Tof Hits position relative to closest Track, for hit coming from multiple MC Track; R(Hit -> Track) [cm]; # [Hits]; Multi [Trk]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitClosestDeltaT = new TH2D("TofTests_MultiTrkHitClosestDeltaT", 
                "Quality of the Tof Hits Time relative to closest Track, for hit coming from multiple MC Track; T(hit) - T(Track) [ps]; # [Hits]; Multi [Trk]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitClosestPullX  = new TH2D("TofTests_MultiTrkHitClosestPullX", 
                "Quality of the Tof Hits position error on X axis relative to closest Track, for hit coming from multiple MC Track; Pull X(Hit -> Track) []; # [Hits]; Multi [Trk]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitClosestPullY  = new TH2D("TofTests_MultiTrkHitClosestPullY", 
                "Quality of the Tof Hits position error on Y axis relative to closest Track, for hit coming from multiple MC Track; Pull Y(Hit -> Track) []; # [Hits]; Multi [Trk]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitClosestPullZ  = new TH2D("TofTests_MultiTrkHitClosestPullZ", 
                "Quality of the Tof Hits position error on Z axis relative to closest Track, for hit coming from multiple MC Track; Pull Z(Hit -> Track) []; # [Hits]; Multi [Trk]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitClosestPullR  = new TH2D("TofTests_MultiTrkHitClosestPullR", 
                "Quality of the Tof Hits position error relative to closest Track, for hit coming from multiple MC Track; Pull R(Hit -> Track) []; # [Hits]; Multi [Trk]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
      // To Track furthest from Hit
   fhMultiTrkHitFurthestDeltaX  = new TH2D("TofTests_MultiTrkHitFurthestDeltaX", 
                "Quality of the Tof Hits position on X axis relative to furthest Track, for hit coming from multiple MC Track; X(Hit) - X(Track) [cm]; # [Hits]; Multi [Trk]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitFurthestDeltaY  = new TH2D("TofTests_MultiTrkHitFurthestDeltaY", 
                "Quality of the Tof Hits position on Y axis relative to furthest Track, for hit coming from multiple MC Track; Y(Hit) - Y(Track) [cm]; # [Hits]; Multi [Trk]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitFurthestDeltaZ  = new TH2D("TofTests_MultiTrkHitFurthestDeltaZ", 
                "Quality of the Tof Hits position on Z axis relative to furthest Track, for hit coming from multiple MC Track; Z(Hit) - Z(Track) [cm]; # [Hits]; Multi [Trk]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitFurthestDeltaR  = new TH2D("TofTests_MultiTrkHitFurthestDeltaR", 
                "Quality of the Tof Hits position relative to furthest Track, for hit coming from multiple MC Track; R(Hit -> Track) [cm]; # [Hits]; Multi [Trk]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitFurthestDeltaT = new TH2D("TofTests_MultiTrkHitFurthestDeltaT", 
                "Quality of the Tof Hits Time relative to furthest Track, for hit coming from multiple MC Track; T(hit) - T(Track) [ps]; # [Hits]; Multi [Trk]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitFurthestPullX  = new TH2D("TofTests_MultiTrkHitFurthestPullX", 
                "Quality of the Tof Hits position error on X axis relative to furthest Track, for hit coming from multiple MC Track; Pull X(Hit -> Track) []; # [Hits]; Multi [Trk]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitFurthestPullY  = new TH2D("TofTests_MultiTrkHitFurthestPullY", 
                "Quality of the Tof Hits position error on Y axis relative to furthest Track, for hit coming from multiple MC Track; Pull Y(Hit -> Track) []; # [Hits]; Multi [Trk]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitFurthestPullZ  = new TH2D("TofTests_MultiTrkHitFurthestPullZ", 
                "Quality of the Tof Hits position error on Z axis relative to furthest Track, for hit coming from multiple MC Track; Pull Z(Hit -> Track) []; # [Hits]; Multi [Trk]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitFurthestPullR  = new TH2D("TofTests_MultiTrkHitFurthestPullR", 
                "Quality of the Tof Hits position error relative to furthest Track, for hit coming from multiple MC Track; Pull R(Hit -> Track) []; # [Hits]; Multi [Trk]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
      // To mean Track position
   fhMultiTrkHitMeanDeltaX  = new TH2D("TofTests_MultiTrkHitMeanDeltaX", 
                "Quality of the Tof Hits position on X axis relative to mean Track position, for hit coming from multiple MC Track; X(Hit) - X(Track) [cm]; # [Hits]; Multi [Trk]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitMeanDeltaY  = new TH2D("TofTests_MultiTrkHitMeanDeltaY", 
                "Quality of the Tof Hits position on Y axis relative to mean Track position, for hit coming from multiple MC Track; Y(Hit) - Y(Track) [cm]; # [Hits]; Multi [Trk]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitMeanDeltaZ  = new TH2D("TofTests_MultiTrkHitMeanDeltaZ", 
                "Quality of the Tof Hits position on Z axis relative to mean Track position, for hit coming from multiple MC Track; Z(Hit) - Z(Track) [cm]; # [Hits]; Multi [Trk]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitMeanDeltaR  = new TH2D("TofTests_MultiTrkHitMeanDeltaR", 
                "Quality of the Tof Hits position relative to mean Track position, for hit coming from multiple MC Track; R(Hit -> Track) [cm]; # [Hits]; Multi [Trk]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitMeanDeltaT = new TH2D("TofTests_MultiTrkHitMeanDeltaT", 
                "Quality of the Tof Hits Time relative to mean Track time, for hit coming from multiple MC Track; T(hit) - T(Track) [ps]; # [Hits]; Multi [Trk]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitMeanPullX  = new TH2D("TofTests_MultiTrkHitMeanPullX", 
                "Quality of the Tof Hits position error on X axis relative to mean Track position, for hit coming from multiple MC Track; Pull X(Hit -> Track) []; # [Hits]; Multi [Trk]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitMeanPullY  = new TH2D("TofTests_MultiTrkHitMeanPullY", 
                "Quality of the Tof Hits position error on Y axis relative to mean Track position, for hit coming from multiple MC Track; Pull Y(Hit -> Track) []; # [Hits]; Multi [Trk]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitMeanPullZ  = new TH2D("TofTests_MultiTrkHitMeanPullZ", 
                "Quality of the Tof Hits position error on Z axis relative to mean Track position, for hit coming from multiple MC Track; Pull Z(Hit -> Track) []; # [Hits]; Multi [Trk]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   fhMultiTrkHitMeanPullR  = new TH2D("TofTests_MultiTrkHitMeanPullR", 
                "Quality of the Tof Hits position error relative to mean Track position, for hit coming from multiple MC Track; Pull R(Hit -> Track) []; # [Hits]; Multi [Trk]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange,
                              iNbBinsMulti, iMinMulti, iMaxMulti );
   
      // Physics coord mapping, 1 per particle type
         // Phase space
   Int_t    iNbBinsY   = 30;
   Double_t dMinY      = -1.;
   Double_t dMaxY      =  4.;
   Int_t    iNbBNinsPtm = 30;
   Double_t dMinPtm    =  0.0;
   Double_t dMaxPtm    =  2.5;
   fvhPtmRapGenTrk.resize(kiNbPart);
   fvhPtmRapStsPnt.resize(kiNbPart);
   fvhPtmRapTofPnt.resize(kiNbPart);
   fvhPtmRapTofHit.resize(kiNbPart);
   fvhPtmRapTofHitSinglePnt.resize(kiNbPart);
   fvhPtmRapTofHitSingleTrk.resize(kiNbPart);
         // PLab
   Int_t    iNbBinsPlab = 100;
   Double_t dMinPlab    =   0.0;
   Double_t dMaxPlab    =  10.0;      
   fvhPlabGenTrk.resize(kiNbPart);
   fvhPlabStsPnt.resize(kiNbPart);
   fvhPlabTofPnt.resize(kiNbPart);
   fvhPlabTofHit.resize(kiNbPart);
   fvhPlabTofHitSinglePnt.resize(kiNbPart);
   fvhPlabTofHitSingleTrk.resize(kiNbPart);
         // MC Tracks losses      
   fvhPtmRapGenTrkTofPnt.resize(kiNbPart);
   fvhPtmRapGenTrkTofHit.resize(kiNbPart);
   fvhPlabGenTrkTofPnt.resize(kiNbPart);
   fvhPlabGenTrkTofhit.resize(kiNbPart);
   for( Int_t iPartIdx = 0; iPartIdx < kiNbPart; iPartIdx++)
   {
         // Phase space
      fvhPtmRapGenTrk[iPartIdx] = new TH2D( Form("TofTests_PtmRapGenTrk_%s", ksPartTag[iPartIdx].Data() ) ,  
                              Form("P_{t}/M vs y distribution for MC tracks, %s; y; P_{t}/M; # []", ksPartName[iPartIdx].Data()  ),
                              iNbBinsY,    dMinY,   dMaxY,
                              iNbBNinsPtm, dMinPtm, dMaxPtm);
      fvhPtmRapStsPnt[iPartIdx] = new TH2D( Form("TofTests_PtmRapStsPnt_%s", ksPartTag[iPartIdx].Data() ) ,
                              Form("P_{t}/M vs y distribution from MC Track with STS points, %s; y; P_{t}/M; # []", ksPartName[iPartIdx].Data()  ),
                              iNbBinsY,    dMinY,   dMaxY,
                              iNbBNinsPtm, dMinPtm, dMaxPtm);
      fvhPtmRapTofPnt[iPartIdx] = new TH2D( Form("TofTests_PtmRapTofPnt_%s", ksPartTag[iPartIdx].Data() ) ,
                              Form("P_{t}/M vs y distribution from MC Track for TOF points, %s; y; P_{t}/M; # []", ksPartName[iPartIdx].Data()  ),
                              iNbBinsY,    dMinY,   dMaxY,
                              iNbBNinsPtm, dMinPtm, dMaxPtm);
      fvhPtmRapTofHit[iPartIdx] = new TH2D( Form("TofTests_PtmRapTofHit_%s", ksPartTag[iPartIdx].Data() ) ,
                              Form("P_{t}/M vs y distribution from MC Track for TOF Hits, %s; y; P_{t}/M; # []", ksPartName[iPartIdx].Data()  ),
                              iNbBinsY,    dMinY,   dMaxY,
                              iNbBNinsPtm, dMinPtm, dMaxPtm);
      fvhPtmRapTofHitSinglePnt[iPartIdx] = new TH2D( Form("TofTests_PtmRapTofHitSinglePnt_%s", ksPartTag[iPartIdx].Data() ),
                              Form("P_{t}/M vs y distribution from MC Track for TOF Hits from a single TOF Point, %s; y; P_{t}/M; # []", ksPartName[iPartIdx].Data()  ),
                              iNbBinsY,    dMinY,   dMaxY,
                              iNbBNinsPtm, dMinPtm, dMaxPtm);
      fvhPtmRapTofHitSingleTrk[iPartIdx] = new TH2D( Form("TofTests_PtmRapTofHitSingleTrk_%s", ksPartTag[iPartIdx].Data() ),
                              Form("P_{t}/M vs y distribution from MC Track for TOF Hits from a single MC track, %s; y; P_{t}/M; # []", ksPartName[iPartIdx].Data()  ),
                              iNbBinsY,    dMinY,   dMaxY,
                              iNbBNinsPtm, dMinPtm, dMaxPtm);
         // PLab
      fvhPlabGenTrk[iPartIdx] = new TH1D( Form("TofTests_PlabGenTrk_%s", ksPartTag[iPartIdx].Data() ), 
                              Form("P_{lab} distribution for MC tracks, %s; P_{lab} [GeV/c]; # []", ksPartName[iPartIdx].Data()  ),
                              iNbBinsPlab, dMinPlab, dMaxPlab);
      fvhPlabStsPnt[iPartIdx] = new TH1D( Form("TofTests_PlabStsPnt_%s", ksPartTag[iPartIdx].Data() ),
                              Form("P_{lab} distribution from MC Track with STS points, %s; P_{lab} [GeV/c]; # []", ksPartName[iPartIdx].Data()  ),
                              iNbBinsPlab, dMinPlab, dMaxPlab);
      fvhPlabTofPnt[iPartIdx] = new TH1D( Form("TofTests_PlabTofPnt_%s", ksPartTag[iPartIdx].Data() ),
                              Form("P_{lab} distribution from MC Track for TOF points, %s; P_{lab} [GeV/c]; # []", ksPartName[iPartIdx].Data()  ),
                              iNbBinsPlab, dMinPlab, dMaxPlab);
      fvhPlabTofHit[iPartIdx] = new TH1D( Form("TofTests_PlabTofHit_%s", ksPartTag[iPartIdx].Data() ),
                              Form("P_{lab} distribution from MC Track for TOF Hits, %s; P_{lab} [GeV/c]; # []", ksPartName[iPartIdx].Data()  ),
                              iNbBinsPlab, dMinPlab, dMaxPlab);
      fvhPlabTofHitSinglePnt[iPartIdx] = new TH1D( Form("TofTests_PlabTofHitSinglePnt_%s", ksPartTag[iPartIdx].Data() ), 
                              Form("P_{lab} distribution from MC Track for TOF Hits from a single TOF Point, %s; P_{lab} [GeV/c]; # []", ksPartName[iPartIdx].Data()  ),
                              iNbBinsPlab, dMinPlab, dMaxPlab);
      fvhPlabTofHitSingleTrk[iPartIdx] = new TH1D( Form("TofTests_PlabTofHitSingleTrk_%s", ksPartTag[iPartIdx].Data() ),
                              Form("P_{lab} distribution from MC Track for TOF Hits from a single MC tracks, %s; P_{lab} [GeV/c]; # []", ksPartName[iPartIdx].Data()  ),
                              iNbBinsPlab, dMinPlab, dMaxPlab);
                              
                              
         // MC Tracks losses      
      fvhPtmRapGenTrkTofPnt[iPartIdx] = new TH2D( Form("TofTests_PtmRapGenTrkTofPnt_%s", ksPartTag[iPartIdx].Data() ) ,  
                              Form("P_{t}/M vs y distribution for MC tracks with TOF Point(s), %s; y; P_{t}/M; # []", ksPartName[iPartIdx].Data()  ),
                              iNbBinsY,    dMinY,   dMaxY,
                              iNbBNinsPtm, dMinPtm, dMaxPtm);
      fvhPtmRapGenTrkTofHit[iPartIdx] = new TH2D( Form("TofTests_PtmRapGenTrkTofHit_%s", ksPartTag[iPartIdx].Data() ) ,  
                              Form("P_{t}/M vs y distribution for MC tracks with TOF Hit(s), %s; y; P_{t}/M; # []", ksPartName[iPartIdx].Data()  ),
                              iNbBinsY,    dMinY,   dMaxY,
                              iNbBNinsPtm, dMinPtm, dMaxPtm);
      
      fvhPlabGenTrkTofPnt[iPartIdx] = new TH1D( Form("TofTests_PlabGenTrkTofPnt_%s", ksPartTag[iPartIdx].Data() ), 
                              Form("P_{lab} distribution for MC tracks with TOF Point(s), %s; P_{lab} [GeV/c]; # []", ksPartName[iPartIdx].Data()  ),
                              iNbBinsPlab, dMinPlab, dMaxPlab);
      fvhPlabGenTrkTofhit[iPartIdx] = new TH1D( Form("TofTests_PlabGenTrkTofHit_%s", ksPartTag[iPartIdx].Data() ),
                              Form("P_{lab} distribution from MC Track with TOF Hit(s), %s; P_{lab} [GeV/c]; # []", ksPartName[iPartIdx].Data()  ),
                              iNbBinsPlab, dMinPlab, dMaxPlab);
   } // for( Int_t iPartIdx = 0; iPartIdx < kiNbPart; iPartIdx++)
   
   gDirectory->cd( oldir->GetPath() ); // <= To prevent histos from being sucked in by the param file of the TRootManager!

   return kTRUE;
}

// ------------------------------------------------------------------
Bool_t CbmTofHitFinderQa::FillHistos()
{
   // Declare variables outside the loop
   CbmMCTrack  * pMcTrk;
   CbmTofPoint * pTofPoint;
   CbmMatch    * pMatchDigiPnt;
   CbmMatch    * pMatchDigiPntB;
   CbmTofHit   * pTofHit;
   CbmMatch    * pMatchHitDigi;

   Int_t iNbTracks, iNbTofPts, iNbTofDigis, 
         iNbTofDigisMatch, iNbTofHits, iNbTofHitsMatch;

   iNbTracks        = fMcTracksColl->GetEntriesFast();
   iNbTofPts        = fTofPointsColl->GetEntriesFast();
   iNbTofDigis      = fTofDigisColl->GetEntriesFast();
   iNbTofDigisMatch = fTofDigiMatchPointsColl->GetEntriesFast();
   iNbTofHits       = fTofHitsColl->GetEntriesFast();
   iNbTofHitsMatch  = fTofDigiMatchColl->GetEntriesFast();

   if( iNbTofDigis != iNbTofDigisMatch )
      LOG(FATAL)<<"CbmTofHitFinderQa::FillHistos => Nb entries in TofDigiMatchPoints TClonesArray doe not match nb entries in TofDigi!!!"<<FairLogger::endl;
   if( iNbTofHits != iNbTofHitsMatch )
      LOG(FATAL)<<"CbmTofHitFinderQa::FillHistos => Nb entries in TofDigiMatch TClonesArray doe not match nb entries in TofHit!!! "
                <<iNbTofHits<<" VS "<<iNbTofHitsMatch
                <<" (Prev step: "<<iNbTofDigis<<" VS "<<iNbTofDigisMatch<<" )"<<FairLogger::endl;      
   
   // Tracks Info
   Int_t iNbTofTracks     = 0;
   Int_t iNbTofTracksPrim = 0;
   std::vector<Bool_t> vbTrackHasHit(iNbTracks, kFALSE);
   for(Int_t iTrkInd = 0; iTrkInd < iNbTracks; iTrkInd++)
   {
      pMcTrk = (CbmMCTrack*) fMcTracksColl->At( iTrkInd );

      if( 0 < pMcTrk->GetNPoints(kTOF) )
      {
         iNbTofTracks++;
      }
      if( 0 < pMcTrk->GetNPoints(kTOF) && -1 == pMcTrk->GetMotherId() )
         iNbTofTracksPrim++;
         
      // tracks mapping: Only when creating normalization histos
      // Assume only TOF in setup, no field (only straight tracks)
      // and all tracks reach TOF (protons)
      if( kTRUE == fbNormHistGenMode )
      {
         // XYZ mapping: assume tracks along Z axis
         if( pMcTrk->GetPz() == pMcTrk->GetP() )
         {
            fhTrackMapXY->Fill( pMcTrk->GetStartX(), pMcTrk->GetStartY() );
//            fhTrackMapXZ->Fill( pMcTrk->GetStartX(), fdWallPosZ ); // Not sure how to get Z here
//            fhTrackMapYZ->Fill( pMcTrk->GetStartY(), fdWallPosZ ); // Not sure how to get Z here
         } // if( pMcTrk->GetPz() == pMcTrk->GetP() )
         
         // Angular mapping: assume tracks all coming from origin and not necess. along Z axis
         if( 0 != pMcTrk->GetPz() && ( (0 != pMcTrk->GetPx() ) || (0 != pMcTrk->GetPy() ) ))
            fhTrackMapAng->Fill( TMath::ATan2( pMcTrk->GetPx(), pMcTrk->GetPz() )*180.0/TMath::Pi(),
                                 TMath::ATan2( pMcTrk->GetPy(), pMcTrk->GetPz() )*180.0/TMath::Pi() );
                                 
         // Spherical mapping: assume tracks all coming from origin and not necess. along Z axis
         if( 0 != pMcTrk->GetPz() && 0 != pMcTrk->GetPx() )
            fhTrackMapSph->Fill( TMath::ATan2( pMcTrk->GetPt(), pMcTrk->GetPz() ),
                                 TMath::ATan2( pMcTrk->GetPy(), pMcTrk->GetPx() ) );
      } // if( kTRUE == fbNormHistGenMode )
      
         // Physics coord mapping, 1 per particle type
	   Int_t iPdgCode = pMcTrk->GetPdgCode();
      Int_t iPartIdx = -1;
      for( Int_t iPart = 0; iPart < kiNbPart; iPart ++)
         if( kiPartPdgCode[iPart] == iPdgCode )
         {
            iPartIdx = iPart;
            break;
         } // if( kiPartPdgCode[iPart] == iPdgCode )
      if( -1 < iPartIdx )
      {
         // Phase space
         fvhPtmRapGenTrk[iPartIdx]->Fill( pMcTrk->GetRapidity(), pMcTrk->GetPt()/pMcTrk->GetMass());
         // PLab
         fvhPlabGenTrk[iPartIdx]->Fill( pMcTrk->GetP() );
         // Do the same for tracks within STS acceptance
         if( 0 < pMcTrk->GetNPoints(kSTS) )
         {
            fvhPtmRapStsPnt[iPartIdx]->Fill( pMcTrk->GetRapidity(), pMcTrk->GetPt()/pMcTrk->GetMass());
            fvhPlabStsPnt[iPartIdx]->Fill( pMcTrk->GetP() );
         } // if( 0 < pMcTrk->GetNPoints(kSTS) )
         // Do the same for tracks within STS acceptance
         if( 0 < pMcTrk->GetNPoints(kTOF) )
         {
            fvhPtmRapGenTrkTofPnt[iPartIdx]->Fill( pMcTrk->GetRapidity(), pMcTrk->GetPt()/pMcTrk->GetMass());
            fvhPlabGenTrkTofPnt[iPartIdx]->Fill( pMcTrk->GetP() );
         } // if( 0 < pMcTrk->GetNPoints(kTOF) )
      } // if( -1 < iPartIdx )
         else 
         {
            // Phase space
            fvhPtmRapGenTrk[0]->Fill( pMcTrk->GetRapidity(), pMcTrk->GetPt()/pMcTrk->GetMass());
            // PLab
            fvhPlabGenTrk[0]->Fill( pMcTrk->GetP() );
            // Do the same for tracks within STS acceptance
            if( 0 < pMcTrk->GetNPoints(kSTS) )
            {
               fvhPtmRapStsPnt[0]->Fill( pMcTrk->GetRapidity(), pMcTrk->GetPt()/pMcTrk->GetMass());
               fvhPlabStsPnt[0]->Fill( pMcTrk->GetP() );
            } // if( 0 < pMcTrk->GetNPoints(kSTS) )
            if( 0 < pMcTrk->GetNPoints(kTOF) )
            {
               fvhPtmRapGenTrkTofPnt[0]->Fill( pMcTrk->GetRapidity(), pMcTrk->GetPt()/pMcTrk->GetMass());
               fvhPlabGenTrkTofPnt[0]->Fill( pMcTrk->GetP() );
            } // if( 0 < pMcTrk->GetNPoints(kTOF) )
         } // else of if( -1 < iPartIdx )
   } // for(Int_t iTrkInd = 0; iTrkInd < nMcTracks; iTrkInd++)
   
   // Loop over Points and map them?
   for (Int_t iPntInd = 0; iPntInd < iNbTofPts; iPntInd++ )
   {
      // Get a pointer to the TOF point
      pTofPoint = (CbmTofPoint*) fTofPointsColl->At( iPntInd );
      // Get a pointer to the corresponding MC Track
      pMcTrk = (CbmMCTrack*) fMcTracksColl->At( pTofPoint->GetTrackID() );

      // Obtain position
      TVector3 vPntPos;
      pTofPoint->Position( vPntPos );

      Double_t dX = vPntPos.X();
      Double_t dY = vPntPos.Y();
      Double_t dZ = vPntPos.Z();

      // tracks mapping: Only when creating normalization histos
      // Assume only TOF in setup, no field (only straight tracks)
      // and all tracks reach TOF (protons)
      if( kTRUE == fbNormHistGenMode )
      {
         // XYZ mapping: assume tracks along Z axis
         if( pMcTrk->GetPz() == pMcTrk->GetP() &&
             pMcTrk->GetStartX() == dX &&
             pMcTrk->GetStartY() == dY )
         {
            fhTrackMapXZ->Fill( dX, dZ ); // only way to get Z here?
            fhTrackMapYZ->Fill( dY, dZ ); // only way to get Z here?
         } // if( pMcTrk->GetPz() == pMcTrk->GetP() )
      } // if( kTRUE == fbNormHistGenMode )

      fhPointMapXY->Fill( dX, dY );
      fhPointMapXZ->Fill( dX, dZ );
      fhPointMapYZ->Fill( dY, dZ );

      Double_t dThetaX = TMath::ATan2( dX, dZ )*180.0/TMath::Pi();
      Double_t dThetaY = TMath::ATan2( dY, dZ )*180.0/TMath::Pi();
      fhPointMapAng->Fill( dThetaX, dThetaY );

      Double_t dTheta  = TMath::ATan2( TMath::Sqrt( dX*dX + dY*dY ), dZ );//*180.0/TMath::Pi();
      Double_t dPhi    = TMath::ATan2( dY, dX );//*180.0/TMath::Pi();
      fhPointMapSph->Fill( dTheta, dPhi );
      
         // Physics coord mapping, 1 per particle type
	   Int_t iPdgCode = pMcTrk->GetPdgCode();
      Int_t iPartIdx = -1;
      for( Int_t iPart = 0; iPart < kiNbPart; iPart ++)
         if( kiPartPdgCode[iPart] == iPdgCode )
         {
            iPartIdx = iPart;
            break;
         } // if( kiPartPdgCode[iPart] == iPdgCode )
      if( -1 < iPartIdx )
      {
         // Phase space
         fvhPtmRapTofPnt[iPartIdx]->Fill( pMcTrk->GetRapidity(), pMcTrk->GetPt()/pMcTrk->GetMass());
         // PLab
         fvhPlabTofPnt[iPartIdx]->Fill( pMcTrk->GetP() );
      } // if( -1 < iPartIdx )
         else 
         {
            // Phase space
            fvhPtmRapTofPnt[0]->Fill( pMcTrk->GetRapidity(), pMcTrk->GetPt()/pMcTrk->GetMass());
            // PLab
            fvhPlabTofPnt[0]->Fill( pMcTrk->GetP() );
         } // else of if( -1 < iPartIdx )
   } // for (Int_t iPntInd = 0; iPntInd < nTofPoint; iPntInd++ )
 
   // Loop over Digis and map them?
   if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
   {
      CbmTofDigiExp *pTofDigi;
      for( Int_t iDigInd = 0; iDigInd < iNbTofDigis; iDigInd++ )
      {
         pTofDigi = (CbmTofDigiExp*) fTofDigisColl->At( iDigInd );

         Int_t iSmType = pTofDigi->GetType();
         Int_t iSm     = pTofDigi->GetSm();
         Int_t iRpc    = pTofDigi->GetRpc();
         Int_t iCh     = pTofDigi->GetChannel();
         // First Get X/Y position info
         CbmTofDetectorInfo xDetInfo(kTOF, iSmType, iSm, iRpc, 0, iCh + 1);
         Int_t iChId =  fTofId->SetDetectorInfo( xDetInfo );
         fChannelInfo = fDigiPar->GetCell( iChId );

         Double_t dX = fChannelInfo->GetX();
         Double_t dY = fChannelInfo->GetY();
         Double_t dZ = fChannelInfo->GetZ();

         fhDigiMapXY->Fill(   dX, dY );
         fhDigiMapXZ->Fill(   dX, dZ );
         fhDigiMapYZ->Fill(   dY, dZ );

         Double_t dThetaX = TMath::ATan2( dX, dZ )*180.0/TMath::Pi();
         Double_t dThetaY = TMath::ATan2( dY, dZ )*180.0/TMath::Pi();
         fhDigiMapAng->Fill( dThetaX, dThetaY );

         Double_t dTheta  = TMath::ATan2( TMath::Sqrt( dX*dX + dY*dY ), dZ );//*180.0/TMath::Pi();
         Double_t dPhi    = TMath::ATan2( dY, dX );//*180.0/TMath::Pi();
         fhDigiMapSph->Fill( dTheta, dPhi );
      } // for( Int_t iDigInd = 0; iDigInd < iNbTofDigis; iDigInd++ )
   } // if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
      else
      {
         CbmTofDigi *pTofDigi;
         for( Int_t iDigInd = 0; iDigInd < iNbTofDigis; iDigInd++ )
         {
            pTofDigi = (CbmTofDigi*) fTofDigisColl->At( iDigInd );

            Int_t iSmType = pTofDigi->GetType();
            Int_t iSm     = pTofDigi->GetSm();
            Int_t iRpc    = pTofDigi->GetRpc();
            Int_t iCh     = pTofDigi->GetChannel();
            // First Get X/Y position info
            CbmTofDetectorInfo xDetInfo(kTOF, iSmType, iSm, iRpc, 0, iCh + 1);
            Int_t iChId = fTofId->SetDetectorInfo( xDetInfo );
            fChannelInfo = fDigiPar->GetCell( iChId );

            Double_t dX = fChannelInfo->GetX();
            Double_t dY = fChannelInfo->GetY();
            Double_t dZ = fChannelInfo->GetZ();

            fhDigiMapXY->Fill(   dX, dY );
            fhDigiMapXZ->Fill(   dX, dZ );
            fhDigiMapYZ->Fill(   dY, dZ );

            Double_t dThetaX = TMath::ATan2( dX, dZ )*180.0/TMath::Pi();
            Double_t dThetaY = TMath::ATan2( dY, dZ )*180.0/TMath::Pi();
            fhDigiMapAng->Fill( dThetaX, dThetaY );

            Double_t dTheta  = TMath::ATan2( TMath::Sqrt( dX*dX + dY*dY ), dZ );//*180.0/TMath::Pi();
            Double_t dPhi    = TMath::ATan2( dY, dX );//*180.0/TMath::Pi();
            fhDigiMapSph->Fill( dTheta, dPhi );
         } // for( Int_t iDigInd = 0; iDigInd < iNbTofDigis; iDigInd++ )
      } // else of if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
     
   // Loop Over Hits
   for( Int_t iHitInd = 0; iHitInd < iNbTofHits; iHitInd++ )
   {
      std::vector<Int_t> vTofPointsId;
      std::vector<Int_t> vTofTracksId;
      Double_t dPntMeanPosX = 0;
      Double_t dPntMeanPosY = 0;
      Double_t dPntMeanPosZ = 0;
      Double_t dPntMeanTime = 0;
      Double_t dTrkMeanPosX = 0;
      Double_t dTrkMeanPosY = 0;
      Double_t dTrkMeanPosZ = 0;
      Double_t dTrkMeanTime = 0;
      Int_t    iClosestPntIdx  = -1;
      Double_t dClosestPntDr   = 1e18;
      Int_t    iFurthestPntIdx = -1;
      Double_t dFurthestPntDr  = -1;
      Int_t    iClosestTrkIdx  = -1;
      Double_t dClosestTrkDr   = 1e18;
      Int_t    iFurthestTrkIdx = -1;
      Double_t dFurthestTrkDr  = -1;
         
      pTofHit       = (CbmTofHit*) fTofHitsColl->At( iHitInd );
      pMatchHitDigi = (CbmMatch*) fTofDigiMatchColl->At( iHitInd );
      
      Double_t dX = pTofHit->GetX();
      Double_t dY = pTofHit->GetY();
      Double_t dZ = pTofHit->GetZ();
      Double_t dErrX = pTofHit->GetDx();
      Double_t dErrY = pTofHit->GetDy();
      Double_t dErrZ = pTofHit->GetDz();
      Double_t dErrR = TMath::Sqrt( dErrX*dErrX + dErrY*dErrY + dErrZ*dErrZ );

      fhHitMapXY->Fill(   dX, dY );
      fhHitMapXZ->Fill(   dX, dZ );
      fhHitMapYZ->Fill(   dY, dZ );

      Double_t dThetaX = TMath::ATan2( dX, dZ )*180.0/TMath::Pi();
      Double_t dThetaY = TMath::ATan2( dY, dZ )*180.0/TMath::Pi();
      fhHitMapAng->Fill( dThetaX, dThetaY );

      Double_t dTheta  = TMath::ATan2( TMath::Sqrt( dX*dX + dY*dY ), dZ );//*180.0/TMath::Pi();
      Double_t dPhi    = TMath::ATan2( dY, dX );//*180.0/TMath::Pi();
      fhHitMapSph->Fill( dTheta, dPhi );
      
      if( kFALSE == fbNormHistGenMode )
      {
         Int_t iNbDigisHit = pMatchHitDigi->GetNofLinks();
         if( 0 != iNbDigisHit%2 )
            LOG(FATAL)<<"CbmTofHitFinderQa::FillHistos => Nb of digis matching Hit #"
                      <<iHitInd<<" in event #"<<fEvents
                      <<" is not a multiple of 2 => should not happen as both ends of strp required!!!"<<FairLogger::endl;
            
         // Loop over Digis inside Hit
         if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
         {
            CbmTofDigiExp *pTofDigi;
            for( Int_t iDigi = 0; iDigi < iNbDigisHit; iDigi++)
            {
               CbmLink lDigi    = pMatchHitDigi->GetLink(iDigi); 
               Int_t   iDigiIdx = lDigi.GetIndex();
               
               if( iNbTofDigis <= iDigiIdx )
               {
                  LOG(ERROR)<<"CbmTofHitFinderQa::FillHistos => Digi index from Hit #"
                      <<iHitInd<<" in event #"<<fEvents
                      <<" is bigger than nb entries in Digis arrays => ignore it!!!"<<FairLogger::endl;
                  continue;
               } // if( iNbTofDigis <= iDigiIdx )
               
               pTofDigi      = (CbmTofDigiExp*) fTofDigisColl->At( iDigiIdx );
               pMatchDigiPnt = (CbmMatch*) fTofDigiMatchPointsColl->At( iDigiIdx );

               CbmLink lPt    = pMatchDigiPnt->GetLink(0); 
               Int_t   iPtIdx = lPt.GetIndex();
               Int_t   iTrkId = ((CbmTofPoint*) fTofPointsColl->At(iPtIdx))->GetTrackID();
                  
               Int_t iSmType = pTofDigi->GetType();
               Int_t iSm     = pTofDigi->GetSm();
               Int_t iRpc    = pTofDigi->GetRpc();
               Int_t iCh     = pTofDigi->GetChannel();
               Int_t iGlobalChan = iCh  + fvRpcChOffs[iSmType][iSm][iRpc];
               
               // MC Track losses
               if( kFALSE == vbTrackHasHit[iTrkId] )
                  vbTrackHasHit[iTrkId] = kTRUE;
               
               // Check Left-Right MC missmatch (digis match always stored by pairs)
               if( 0 == iDigi%2 )
               {
                  // Get Info about the other end of the strip
                  pMatchDigiPntB = (CbmMatch*) fTofDigiMatchPointsColl->At( iDigiIdx + 1 );

                  CbmLink lPtB    = pMatchDigiPntB->GetLink(0); 
                  Int_t   iPtIdxB = lPtB.GetIndex();
                  
                  // Check Left-Right missmatch for MC Point
                  if( iPtIdx != iPtIdxB )
                  {
                     // Check Left-Right missmatch for MC Track
                     if( iTrkId != ((CbmTofPoint*) fTofPointsColl->At(iPtIdxB))->GetTrackID() )
                     {
                        fhLeftRightDigiMatch->Fill(iGlobalChan, 2);
                     } // if( iTrkId != ((CbmTofPoint*) fTofPointsColl->At(iPtIdxB))->GetTrackID() )
                        else fhLeftRightDigiMatch->Fill(iGlobalChan, 1);
                  } // if( iPtIdx != iPtIdxB )
                     else fhLeftRightDigiMatch->Fill(iGlobalChan, 0);

               } // if( 0 == iDigi%2 )
               
               // Count Nb different MC Points in Hit
               Bool_t bPointFound = kFALSE;
               for( UInt_t uPrevPtIdx = 0; uPrevPtIdx < vTofPointsId.size(); uPrevPtIdx++)
                  if( iPtIdx == vTofPointsId[uPrevPtIdx] )
                  {
                     bPointFound = kTRUE;
                     break;
                  } // if( iPtIdx == vTofPointsId[uPrevPtIdx] )
               if( kFALSE == bPointFound )
               {
                  vTofPointsId.push_back(iPtIdx);
                  
                  // Obtain Point position
                  pTofPoint = (CbmTofPoint*) fTofPointsColl->At(iPtIdx);
                  TVector3 vPntPos;
                  pTofPoint->Position( vPntPos );
            
                  // Compute mean MC Points position (X, Y, Z, T)
                  dPntMeanPosX += vPntPos.X();
                  dPntMeanPosY += vPntPos.Y();
                  dPntMeanPosZ += vPntPos.Z();
                  dPntMeanTime += pTofPoint->GetTime();
                  
                  // Check if closest MC Point to Hit position
                  Double_t dPntDeltaR = TMath::Sqrt(
                        (dX - vPntPos.X())*(dX - vPntPos.X()) 
                      + (dY - vPntPos.Y())*(dY - vPntPos.Y()) 
                      + (dZ - vPntPos.Z())*(dZ - vPntPos.Z()) );
                  if( dPntDeltaR < dClosestPntDr )
                  {
                     iClosestPntIdx  = iPtIdx;
                     dClosestPntDr   = dPntDeltaR;
                  } // if( dPntDeltaR < dClosestPntDr )
                  // Check if furthest MC Point to Hit position
                  if( dFurthestPntDr < dPntDeltaR )
                  {
                     iFurthestPntIdx  = iPtIdx;
                     dFurthestPntDr   = dPntDeltaR;
                  } // if( dFurthestPntDr < dPntDeltaR )
               } // if( kFALSE == bPointFound )
                  
               // Count Nb different MC Tracks in Hit
               Bool_t bTrackFound = kFALSE;
               for( UInt_t uPrevTrkIdx = 0; uPrevTrkIdx < vTofTracksId.size(); uPrevTrkIdx++)
                  if( iTrkId == vTofTracksId[uPrevTrkIdx] )
                  {
                     bTrackFound = kTRUE;
                     break;
                  } // if( iTrkId == vTofPointsId[uPrevTrkIdx] )
               if( kFALSE == bTrackFound )
               {
                  vTofTracksId.push_back(iTrkId);
                  
                  // Obtain Point position (Consider 1st Pnt of each Trk is approximate coord)
                  pTofPoint = (CbmTofPoint*) fTofPointsColl->At(iPtIdx);
                  TVector3 vPntPos;
                  pTofPoint->Position( vPntPos );
            
                  // Compute mean MC Tracks position (X, Y, Z, T)
                  dTrkMeanPosX += vPntPos.X();
                  dTrkMeanPosY += vPntPos.Y();
                  dTrkMeanPosZ += vPntPos.Z();
                  dTrkMeanTime += pTofPoint->GetTime();
                  
                  // Check if closest MC track to Hit position
                  Double_t dTrkDeltaR = TMath::Sqrt(
                        (dX - vPntPos.X())*(dX - vPntPos.X()) 
                      + (dY - vPntPos.Y())*(dY - vPntPos.Y()) 
                      + (dZ - vPntPos.Z())*(dZ - vPntPos.Z()) );
                  if( dTrkDeltaR < dClosestTrkDr )
                  {
                     iClosestTrkIdx  = iPtIdx;
                     dClosestTrkDr   = dTrkDeltaR;
                  } // if( dTrkDeltaR < dClosestTrkDr )
                  // Check if furthest MC track to Hit position
                  if( dFurthestTrkDr < dTrkDeltaR )
                  {
                     iFurthestTrkIdx  = iPtIdx;
                     dFurthestTrkDr   = dTrkDeltaR;
                  } // if( dFurthestTrkDr < dTrkDeltaR )
               } // if( kFALSE == bTrackFound )
            } // for( Int_t iDigiIdx = 0; iDigiIdx < iNbDigisHit; iDigiIdx++)
         } // if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
            else
            {
               CbmTofDigi *pTofDigi;
               for( Int_t iDigi = 0; iDigi < iNbDigisHit; iDigi++)
               {
                  CbmLink lDigi    = pMatchHitDigi->GetLink(iDigi); 
                  Int_t   iDigiIdx = lDigi.GetIndex();
                  
                  if( iNbTofDigis <= iDigiIdx )
                  {
                     LOG(ERROR)<<"CbmTofHitFinderQa::FillHistos => Digi index from Hit #"
                         <<iHitInd<<" in event #"<<fEvents
                         <<" is bigger than nb entries in Digis arrays => ignore it!!!"<<FairLogger::endl;
                     continue;
                  } // if( iNbTofDigis <= iDigiIdx )
                  
                  pTofDigi      = (CbmTofDigi*) fTofDigisColl->At( iDigiIdx );
                  pMatchDigiPnt = (CbmMatch*) fTofDigiMatchPointsColl->At( iDigiIdx );

                  CbmLink lPt    = pMatchDigiPnt->GetLink(0); 
                  Int_t   iPtIdx = lPt.GetIndex();
                  Int_t   iTrkId = ((CbmTofPoint*) fTofPointsColl->At(iPtIdx))->GetTrackID();
                     
                  Int_t iSmType = pTofDigi->GetType();
                  Int_t iSm     = pTofDigi->GetSm();
                  Int_t iRpc    = pTofDigi->GetRpc();
                  Int_t iCh     = pTofDigi->GetChannel();
                  Int_t iGlobalChan = iCh  + fvRpcChOffs[iSmType][iSm][iRpc];
               
                  // MC Track losses
                  if( kFALSE == vbTrackHasHit[iTrkId] )
                     vbTrackHasHit[iTrkId] = kTRUE;
                  
                  // Check Left-Right MC missmatch (digis match always stored by pairs)
                  if( 0 == iDigi%2 )
                  {
                     // Get Info about the other end of the strip
                     pMatchDigiPntB = (CbmMatch*) fTofDigiMatchPointsColl->At( iDigiIdx + 1 );

                     CbmLink lPtB    = pMatchDigiPntB->GetLink(0); 
                     Int_t   iPtIdxB = lPtB.GetIndex();
                     
                     // Check Left-Right missmatch for MC Point
                     if( iPtIdx != iPtIdxB )
                     {
                        // Check Left-Right missmatch for MC Track
                        if( iTrkId != ((CbmTofPoint*) fTofPointsColl->At(iPtIdxB))->GetTrackID() )
                        {
                           fhLeftRightDigiMatch->Fill(iGlobalChan, 2);
                        } // if( iTrkId != ((CbmTofPoint*) fTofPointsColl->At(iPtIdxB))->GetTrackID() )
                           else fhLeftRightDigiMatch->Fill(iGlobalChan, 1);
                     } // if( iPtIdx != iPtIdxB )
                        else fhLeftRightDigiMatch->Fill(iGlobalChan, 0);

                  } // if( 0 == iDigi%2 )
                  
                  // Count Nb different MC Points in Hit
                  Bool_t bPointFound = kFALSE;
                  for( UInt_t uPrevPtIdx = 0; uPrevPtIdx < vTofPointsId.size(); uPrevPtIdx++)
                     if( iPtIdx == vTofPointsId[uPrevPtIdx] )
                     {
                        bPointFound = kTRUE;
                        break;
                     } // if( iPtIdx == vTofPointsId[uPrevPtIdx] )
                  if( kFALSE == bPointFound )
                  {
                     vTofPointsId.push_back(iPtIdx);
                     
                     // Obtain Point position
                     pTofPoint = (CbmTofPoint*) fTofPointsColl->At(iPtIdx);
                     TVector3 vPntPos;
                     pTofPoint->Position( vPntPos );
               
                     // Compute mean MC Points position (X, Y, Z, T)
                     dPntMeanPosX += vPntPos.X();
                     dPntMeanPosY += vPntPos.Y();
                     dPntMeanPosZ += vPntPos.Z();
                     dPntMeanTime += pTofPoint->GetTime();
                  
                     // Check if closest MC Point to Hit position
                     Double_t dPntDeltaR = TMath::Sqrt(
                           (dX - vPntPos.X())*(dX - vPntPos.X()) 
                         + (dY - vPntPos.Y())*(dY - vPntPos.Y()) 
                         + (dZ - vPntPos.Z())*(dZ - vPntPos.Z()) );
                     if( dPntDeltaR < dClosestPntDr )
                     {
                        iClosestPntIdx  = iPtIdx;
                        dClosestPntDr   = dPntDeltaR;
                     } // if( dPntDeltaR < dClosestPntDr )
                     // Check if furthest MC Point to Hit position
                     if( dFurthestPntDr < dPntDeltaR )
                     {
                        iFurthestPntIdx  = iPtIdx;
                        dFurthestPntDr   = dPntDeltaR;
                     } // if( dFurthestPntDr < dPntDeltaR )
                  } // if( kFALSE == bPointFound )
                     
                  // Count Nb different MC Tracks in Hit
                  Bool_t bTrackFound = kFALSE;
                  for( UInt_t uPrevTrkIdx = 0; uPrevTrkIdx < vTofTracksId.size(); uPrevTrkIdx++)
                     if( iTrkId == vTofTracksId[uPrevTrkIdx] )
                     {
                        bTrackFound = kTRUE;
                        break;
                     } // if( iTrkId == vTofPointsId[uPrevTrkIdx] )
                  if( kFALSE == bTrackFound )
                  {
                     vTofTracksId.push_back(iTrkId);
                     
                     // Obtain Point position (Consider 1st Pnt of each Trk is approximate coord)
                     pTofPoint = (CbmTofPoint*) fTofPointsColl->At(iPtIdx);
                     TVector3 vPntPos;
                     pTofPoint->Position( vPntPos );
               
                     // Compute mean MC Tracks position (X, Y, Z, T)
                     dTrkMeanPosX += vPntPos.X();
                     dTrkMeanPosY += vPntPos.Y();
                     dTrkMeanPosZ += vPntPos.Z();
                     dTrkMeanTime += pTofPoint->GetTime();
                  
                     // Check if closest MC track to Hit position
                     Double_t dTrkDeltaR = TMath::Sqrt(
                           (dX - vPntPos.X())*(dX - vPntPos.X()) 
                         + (dY - vPntPos.Y())*(dY - vPntPos.Y()) 
                         + (dZ - vPntPos.Z())*(dZ - vPntPos.Z()) );
                     if( dTrkDeltaR < dClosestTrkDr )
                     {
                        iClosestTrkIdx  = iPtIdx;
                        dClosestTrkDr   = dTrkDeltaR;
                     } // if( dTrkDeltaR < dClosestTrkDr )
                     // Check if furthest MC track to Hit position
                     if( dFurthestTrkDr < dTrkDeltaR )
                     {
                        iFurthestTrkIdx  = iPtIdx;
                        dFurthestTrkDr   = dTrkDeltaR;
                     } // if( dFurthestTrkDr < dTrkDeltaR )
                  } // if( kFALSE == bTrackFound )
               } // for( Int_t iDigiIdx = 0; iDigiIdx < iNbDigisHit; iDigiIdx++)
            } // else of if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
            
         // Count Nb different MC Points in Hit
         UInt_t uNbPointsInHit = vTofPointsId.size();
         fhNbPointsInHit->Fill( uNbPointsInHit );
         // Count Nb different MC Tracks in Hit
         UInt_t uNbTracksInHit = vTofTracksId.size();
         fhNbTracksInHit->Fill( uNbTracksInHit );
         
         // Check Hit Quality for Hits coming from a single MC Point
         if( 1 == uNbPointsInHit )
         {
            fhHitMapSingPntXY->Fill(   dX, dY );
            fhHitMapSingPntXZ->Fill(   dX, dZ );
            fhHitMapSingPntYZ->Fill(   dY, dZ );
            fhHitMapSingPntAng->Fill( dThetaX, dThetaY );
            fhHitMapSingPntSph->Fill( dTheta, dPhi );
         
            pTofPoint = (CbmTofPoint*) fTofPointsColl->At(vTofPointsId[0]);
            
            // Obtain Point position
            TVector3 vPntPos;
            pTofPoint->Position( vPntPos );
            
            Double_t dDeltaX = dX - vPntPos.X();
            Double_t dDeltaY = dY - vPntPos.Y();
            Double_t dDeltaZ = dZ - vPntPos.Z();
            Double_t dDeltaR = TMath::Sqrt(   dDeltaX*dDeltaX 
                                            + dDeltaY*dDeltaY 
                                            + dDeltaZ*dDeltaZ ); // Not sure Z should be in?
         
            fhSinglePointHitDeltaX->Fill( dDeltaX );
            fhSinglePointHitDeltaY->Fill( dDeltaY );
            fhSinglePointHitDeltaZ->Fill( dDeltaZ );
            fhSinglePointHitDeltaR->Fill( dDeltaR );
            fhSinglePointHitDeltaT->Fill( 1000.0*(pTofHit->GetTime() - pTofPoint->GetTime()) );
            fhSinglePointHitPullX->Fill( dDeltaX/dErrX );
            fhSinglePointHitPullY->Fill( dDeltaY/dErrY );
            fhSinglePointHitPullZ->Fill( dDeltaZ/dErrZ ); // Offset(s) bec. hit assigned to middle of det.
            fhSinglePointHitPullR->Fill( dDeltaR/dErrR );
         } // if( 1 == vTofPointsId.size() )
         // For Hits with multiple MC Points
            else
            {
               fhHitMapMultPntXY->Fill(   dX, dY );
               fhHitMapMultPntXZ->Fill(   dX, dZ );
               fhHitMapMultPntYZ->Fill(   dY, dZ );
               fhHitMapMultPntAng->Fill( dThetaX, dThetaY );
               fhHitMapMultPntSph->Fill( dTheta, dPhi );
            
               // Check Hit Quality relative to mean of all MC Points
               dPntMeanPosX /= vTofPointsId.size();
               dPntMeanPosY /= vTofPointsId.size();
               dPntMeanPosZ /= vTofPointsId.size();
               dPntMeanTime /= vTofPointsId.size();
               
               Double_t dDeltaMeanX = dX - dPntMeanPosX;
               Double_t dDeltaMeanY = dY - dPntMeanPosY;
               Double_t dDeltaMeanZ = dZ - dPntMeanPosZ;
               Double_t dDeltaMeanR = TMath::Sqrt(   dDeltaMeanX*dDeltaMeanX 
                                                   + dDeltaMeanY*dDeltaMeanY 
                                                   + dDeltaMeanZ*dDeltaMeanZ ); // Not sure Z should be in?

               fhMultiPntHitMeanDeltaX->Fill( dDeltaMeanX, uNbPointsInHit );
               fhMultiPntHitMeanDeltaY->Fill( dDeltaMeanY, uNbPointsInHit );
               fhMultiPntHitMeanDeltaZ->Fill( dDeltaMeanZ, uNbPointsInHit );
               fhMultiPntHitMeanDeltaR->Fill( dDeltaMeanR, uNbPointsInHit );
               fhMultiPntHitMeanDeltaT->Fill( 1000.0*(pTofHit->GetTime() - dPntMeanTime), uNbPointsInHit );
               fhMultiPntHitMeanPullX->Fill( dDeltaMeanX/dErrX, uNbPointsInHit );
               fhMultiPntHitMeanPullY->Fill( dDeltaMeanY/dErrY, uNbPointsInHit );
               fhMultiPntHitMeanPullZ->Fill( dDeltaMeanZ/dErrZ, uNbPointsInHit ); // Offset(s) bec. hit assigned to middle of det.
               fhMultiPntHitMeanPullR->Fill( dDeltaMeanR/dErrR, uNbPointsInHit );
               
               // Check Hit Quality relative to closest MC Point
               pTofPoint = (CbmTofPoint*) fTofPointsColl->At(iClosestPntIdx);
               
                  // Obtain Point position
               TVector3 vPntPosClo;
               pTofPoint->Position( vPntPosClo );
            
               Double_t dDeltaCloX = dX - vPntPosClo.X();
               Double_t dDeltaCloY = dY - vPntPosClo.Y();
               Double_t dDeltaCloZ = dZ - vPntPosClo.Z();
               Double_t dDeltaCloR = TMath::Sqrt(   dDeltaCloX*dDeltaCloX 
                                                  + dDeltaCloY*dDeltaCloY 
                                                  + dDeltaCloZ*dDeltaCloZ ); // Not sure Z should be in?
            
               fhMultiPntHitClosestDeltaX->Fill( dDeltaCloX, uNbPointsInHit );
               fhMultiPntHitClosestDeltaY->Fill( dDeltaCloY, uNbPointsInHit );
               fhMultiPntHitClosestDeltaZ->Fill( dDeltaCloZ, uNbPointsInHit );
               fhMultiPntHitClosestDeltaR->Fill( dDeltaCloR, uNbPointsInHit );
               fhMultiPntHitClosestDeltaT->Fill( 1000.0*(pTofHit->GetTime() - pTofPoint->GetTime()), uNbPointsInHit );
               fhMultiPntHitClosestPullX->Fill( dDeltaCloX/dErrX, uNbPointsInHit );
               fhMultiPntHitClosestPullY->Fill( dDeltaCloY/dErrY, uNbPointsInHit );
               fhMultiPntHitClosestPullZ->Fill( dDeltaCloZ/dErrZ, uNbPointsInHit ); // Offset(s) bec. hit assigned to middle of det.
               fhMultiPntHitClosestPullR->Fill( dDeltaCloR/dErrR, uNbPointsInHit );
            
               // Check Hit Quality relative to furthest MC Point
               pTofPoint = (CbmTofPoint*) fTofPointsColl->At(iFurthestPntIdx);
               
                  // Obtain Point position
               TVector3 vPntPosFar;
               pTofPoint->Position( vPntPosFar );
            
               Double_t dDeltaFarX = dX - vPntPosFar.X();
               Double_t dDeltaFarY = dY - vPntPosFar.Y();
               Double_t dDeltaFarZ = dZ - vPntPosFar.Z();
               Double_t dDeltaFarR = TMath::Sqrt(   dDeltaFarX*dDeltaFarX 
                                                  + dDeltaFarY*dDeltaFarY 
                                                  + dDeltaFarZ*dDeltaFarZ ); // Not sure Z should be in?
            
               fhMultiPntHitFurthestDeltaX->Fill( dDeltaFarX, uNbPointsInHit );
               fhMultiPntHitFurthestDeltaY->Fill( dDeltaFarY, uNbPointsInHit );
               fhMultiPntHitFurthestDeltaZ->Fill( dDeltaFarZ, uNbPointsInHit );
               fhMultiPntHitFurthestDeltaR->Fill( dDeltaFarR, uNbPointsInHit );
               fhMultiPntHitFurthestDeltaT->Fill( 1000.0*(pTofHit->GetTime() - pTofPoint->GetTime()), uNbPointsInHit );
               fhMultiPntHitFurthestPullX->Fill( dDeltaFarX/dErrX, uNbPointsInHit );
               fhMultiPntHitFurthestPullY->Fill( dDeltaFarY/dErrY, uNbPointsInHit );
               fhMultiPntHitFurthestPullZ->Fill( dDeltaFarZ/dErrZ, uNbPointsInHit ); // Offset(s) bec. hit assigned to middle of det.
               fhMultiPntHitFurthestPullR->Fill( dDeltaFarR/dErrR, uNbPointsInHit );
            } // else of if( 1 == vTofPointsId.size() )
            
         // Check Hit Quality for Hits coming from a single MC Track
         if( 1 == uNbTracksInHit )
         {
            fhHitMapSingTrkXY->Fill(   dX, dY );
            fhHitMapSingTrkXZ->Fill(   dX, dZ );
            fhHitMapSingTrkYZ->Fill(   dY, dZ );
            fhHitMapSingTrkAng->Fill( dThetaX, dThetaY );
            fhHitMapSingTrkSph->Fill( dTheta, dPhi );
            
            // => If all points come from the same track, any of them should give
            //    the approximate position and time of the track at the detector level
            pTofPoint = (CbmTofPoint*) fTofPointsColl->At(vTofPointsId[0]);
            
            // Obtain Point position
            TVector3 vPntPos;
            pTofPoint->Position( vPntPos );
            
            Double_t dDeltaX = dX - vPntPos.X();
            Double_t dDeltaY = dY - vPntPos.Y();
            Double_t dDeltaZ = dZ - vPntPos.Z();
            Double_t dDeltaR = TMath::Sqrt(   dDeltaX*dDeltaX 
                                            + dDeltaY*dDeltaY 
                                            + dDeltaZ*dDeltaZ ); // Not sure Z should be in?
         
            fhSingleTrackHitDeltaX->Fill( dDeltaX );
            fhSingleTrackHitDeltaY->Fill( dDeltaY );
            fhSingleTrackHitDeltaZ->Fill( dDeltaZ );
            fhSingleTrackHitDeltaR->Fill( dDeltaR );
            fhSingleTrackHitDeltaT->Fill( 1000.0*(pTofHit->GetTime() - pTofPoint->GetTime()) );
            fhSingleTrackHitPullX->Fill( dDeltaX/dErrX );
            fhSingleTrackHitPullY->Fill( dDeltaY/dErrY );
            fhSingleTrackHitPullZ->Fill( dDeltaZ/dErrZ ); // Offset(s) bec. hit assigned to middle of det.
            fhSingleTrackHitPullR->Fill( dDeltaR/dErrR );

            if( 1 < uNbPointsInHit )
            {
               fhSingTrkMultiPntHitDeltaX->Fill( dDeltaX );
               fhSingTrkMultiPntHitDeltaY->Fill( dDeltaY );
               fhSingTrkMultiPntHitDeltaZ->Fill( dDeltaZ );
               fhSingTrkMultiPntHitDeltaR->Fill( dDeltaR );
               fhSingTrkMultiPntHitDeltaT->Fill( 1000.0*(pTofHit->GetTime() - pTofPoint->GetTime()) );
               fhSingTrkMultiPntHitPullX->Fill( dDeltaX/dErrX );
               fhSingTrkMultiPntHitPullY->Fill( dDeltaY/dErrY );
               fhSingTrkMultiPntHitPullZ->Fill( dDeltaZ/dErrZ ); // Offset(s) bec. hit assigned to middle of det.
               fhSingTrkMultiPntHitPullR->Fill( dDeltaR/dErrR );
            } // if( 1 < uNbPointsInHit )
         } // if( 1 == vTofTracksId.size() )
         // For Hits with multiple MC Tracks
            else
            {
               fhHitMapMultTrkXY->Fill(   dX, dY );
               fhHitMapMultTrkXZ->Fill(   dX, dZ );
               fhHitMapMultTrkYZ->Fill(   dY, dZ );
               fhHitMapMultTrkAng->Fill( dThetaX, dThetaY );
               fhHitMapMultTrkSph->Fill( dTheta, dPhi );
               
               // Check Hit Quality relative to mean of all MC Tracks            
               dTrkMeanPosX /= vTofTracksId.size();
               dTrkMeanPosY /= vTofTracksId.size();
               dTrkMeanPosZ /= vTofTracksId.size();
               dTrkMeanTime /= vTofTracksId.size();
               
               Double_t dDeltaMeanX = dX - dTrkMeanPosX;
               Double_t dDeltaMeanY = dY - dTrkMeanPosY;
               Double_t dDeltaMeanZ = dZ - dTrkMeanPosZ;
               Double_t dDeltaMeanR = TMath::Sqrt(   dDeltaMeanX*dDeltaMeanX 
                                                   + dDeltaMeanY*dDeltaMeanY 
                                                   + dDeltaMeanZ*dDeltaMeanZ ); // Not sure Z should be in?
               fhMultiTrkHitMeanDeltaX->Fill( dDeltaMeanX, uNbTracksInHit );
               fhMultiTrkHitMeanDeltaY->Fill( dDeltaMeanY, uNbTracksInHit );
               fhMultiTrkHitMeanDeltaZ->Fill( dDeltaMeanZ, uNbTracksInHit );
               fhMultiTrkHitMeanDeltaR->Fill( dDeltaMeanR, uNbTracksInHit );
               fhMultiTrkHitMeanDeltaT->Fill( 1000.0*(pTofHit->GetTime() - dTrkMeanTime), uNbTracksInHit );
               fhMultiTrkHitMeanPullX->Fill( dDeltaMeanX/dErrX, uNbTracksInHit );
               fhMultiTrkHitMeanPullY->Fill( dDeltaMeanY/dErrY, uNbTracksInHit );
               fhMultiTrkHitMeanPullZ->Fill( dDeltaMeanZ/dErrZ, uNbTracksInHit ); // Offset(s) bec. hit assigned to middle of det.
               fhMultiTrkHitMeanPullR->Fill( dDeltaMeanR/dErrR, uNbTracksInHit );
               
               // Check Hit Quality relative to closest MC Tracks
                  // Obtain Point position (Consider 1st Pnt of each Trk is approximate coord)
               pTofPoint = (CbmTofPoint*) fTofPointsColl->At(iClosestTrkIdx);
               TVector3 vPntPosClo;
               pTofPoint->Position( vPntPosClo );
            
               Double_t dDeltaCloX = dX - vPntPosClo.X();
               Double_t dDeltaCloY = dY - vPntPosClo.Y();
               Double_t dDeltaCloZ = dZ - vPntPosClo.Z();
               Double_t dDeltaCloR = TMath::Sqrt(   dDeltaCloX*dDeltaCloX 
                                                  + dDeltaCloY*dDeltaCloY 
                                                  + dDeltaCloZ*dDeltaCloZ ); // Not sure Z should be in?
            
               fhMultiTrkHitClosestDeltaX->Fill( dDeltaCloX, uNbTracksInHit );
               fhMultiTrkHitClosestDeltaY->Fill( dDeltaCloY, uNbTracksInHit );
               fhMultiTrkHitClosestDeltaZ->Fill( dDeltaCloZ, uNbTracksInHit );
               fhMultiTrkHitClosestDeltaR->Fill( dDeltaCloR, uNbTracksInHit );
               fhMultiTrkHitClosestDeltaT->Fill( 1000.0*(pTofHit->GetTime() - pTofPoint->GetTime()), uNbTracksInHit );
               fhMultiTrkHitClosestPullX->Fill( dDeltaCloX/dErrX, uNbTracksInHit );
               fhMultiTrkHitClosestPullY->Fill( dDeltaCloY/dErrY, uNbTracksInHit );
               fhMultiTrkHitClosestPullZ->Fill( dDeltaCloZ/dErrZ, uNbTracksInHit ); // Offset(s) bec. hit assigned to middle of det.
               fhMultiTrkHitClosestPullR->Fill( dDeltaCloR/dErrR, uNbTracksInHit );
            
               // Check Hit Quality relative to furthest MC Tracks
                  // Obtain Point position (Consider 1st Pnt of each Trk is approximate coord)
               pTofPoint = (CbmTofPoint*) fTofPointsColl->At(iFurthestTrkIdx);
               TVector3 vPntPosFar;
               pTofPoint->Position( vPntPosFar );
            
               Double_t dDeltaFarX = dX - vPntPosFar.X();
               Double_t dDeltaFarY = dY - vPntPosFar.Y();
               Double_t dDeltaFarZ = dZ - vPntPosFar.Z();
               Double_t dDeltaFarR = TMath::Sqrt(   dDeltaFarX*dDeltaFarX 
                                                  + dDeltaFarY*dDeltaFarY 
                                                  + dDeltaFarZ*dDeltaFarZ ); // Not sure Z should be in?
            
               fhMultiTrkHitFurthestDeltaX->Fill( dDeltaFarX, uNbTracksInHit );
               fhMultiTrkHitFurthestDeltaY->Fill( dDeltaFarY, uNbTracksInHit );
               fhMultiTrkHitFurthestDeltaZ->Fill( dDeltaFarZ, uNbTracksInHit );
               fhMultiTrkHitFurthestDeltaR->Fill( dDeltaFarR, uNbTracksInHit );
               fhMultiTrkHitFurthestDeltaT->Fill( 1000.0*(pTofHit->GetTime() - pTofPoint->GetTime()), uNbTracksInHit );
               fhMultiTrkHitFurthestPullX->Fill( dDeltaFarX/dErrX, uNbTracksInHit );
               fhMultiTrkHitFurthestPullY->Fill( dDeltaFarY/dErrY, uNbTracksInHit );
               fhMultiTrkHitFurthestPullZ->Fill( dDeltaFarZ/dErrZ, uNbTracksInHit ); // Offset(s) bec. hit assigned to middle of det.
               fhMultiTrkHitFurthestPullR->Fill( dDeltaFarR/dErrR, uNbTracksInHit );
            } // if( 1 == vTofTracksId.size() )
         
         // Physics coord mapping, 1 per particle type
            // Do as in CbmHadronAna: Take the MC Point of the first Digi matched with the hit
         pMatchHitDigi = (CbmMatch*) fTofDigiMatchColl->At( iHitInd );
            // Get index of first digi 
         CbmLink lDigi    = pMatchHitDigi->GetLink(0);
         Int_t   iDigiIdx = lDigi.GetIndex();
         if( iNbTofDigis <= iDigiIdx )
         {
            LOG(ERROR)<<"CbmTofHitFinderQa::FillHistos => Digi index from Hit #"
            <<iHitInd<<" in event #"<<fEvents
            <<" is bigger than nb entries in Digis arrays => ignore it!!!"<<FairLogger::endl;
            continue;
         } // if( iNbTofDigis <= iDigiIdx )
            // Get pointer on match of first digi
         pMatchDigiPnt = (CbmMatch*) fTofDigiMatchPointsColl->At( iDigiIdx );
            // Get index of tof point corresponding to the first digi
         CbmLink lPt    = pMatchDigiPnt->GetLink(0); 
         Int_t   iPtIdx = lPt.GetIndex();
            // Get index of corresponding MC track
         Int_t   iTrkId = ((CbmTofPoint*) fTofPointsColl->At(iPtIdx))->GetTrackID();         
            // Get a pointer to the corresponding MC Track
         pMcTrk = (CbmMCTrack*) fMcTracksColl->At( iTrkId );
               
         Int_t iPdgCode = pMcTrk->GetPdgCode();
         Int_t iPartIdx = -1;
         for( Int_t iPart = 0; iPart < kiNbPart; iPart ++)
            if( kiPartPdgCode[iPart] == iPdgCode )
            {
               iPartIdx = iPart;
               break;
            } // if( kiPartPdgCode[iPart] == iPdgCode )
         if( -1 < iPartIdx )
         {
            // Phase space
            fvhPtmRapTofHit[iPartIdx]->Fill( pMcTrk->GetRapidity(), pMcTrk->GetPt()/pMcTrk->GetMass());
            // PLab
            fvhPlabTofHit[iPartIdx]->Fill( pMcTrk->GetP() );
            if( 1 == vTofPointsId.size() )
            {
               fvhPtmRapTofHitSinglePnt[iPartIdx]->Fill( pMcTrk->GetRapidity(), pMcTrk->GetPt()/pMcTrk->GetMass());
               fvhPlabTofHitSinglePnt[iPartIdx]->Fill( pMcTrk->GetP() );
            } // if( 1 == vTofPointsId.size() )
            if( 1 == vTofTracksId.size() )
            {
               fvhPtmRapTofHitSingleTrk[iPartIdx]->Fill( pMcTrk->GetRapidity(), pMcTrk->GetPt()/pMcTrk->GetMass());
               fvhPlabTofHitSingleTrk[iPartIdx]->Fill( pMcTrk->GetP() );
            } // if( 1 == vTofTracksId.size() )
         } // if( -1 < iPartIdx )
            else 
            {
               // Phase space
               fvhPtmRapTofHit[0]->Fill( pMcTrk->GetRapidity(), pMcTrk->GetPt()/pMcTrk->GetMass());
               // PLab
               fvhPlabTofHit[0]->Fill( pMcTrk->GetP() );
               if( 1 == vTofPointsId.size() )
               {
                  fvhPtmRapTofHitSinglePnt[0]->Fill( pMcTrk->GetRapidity(), pMcTrk->GetPt()/pMcTrk->GetMass());
                  fvhPlabTofHitSinglePnt[0]->Fill( pMcTrk->GetP() );
               } // if( 1 == vTofPointsId.size() )
               if( 1 == vTofTracksId.size() )
               {
                  fvhPtmRapTofHitSingleTrk[0]->Fill( pMcTrk->GetRapidity(), pMcTrk->GetPt()/pMcTrk->GetMass());
                  fvhPlabTofHitSingleTrk[0]->Fill( pMcTrk->GetP() );
               } // if( 1 == vTofTracksId.size() )
            } // else of if( -1 < iPartIdx )
         
         // clear storage of point and track IDs
         vTofPointsId.clear();
         vTofTracksId.clear();
      } // if( kFALSE == fbNormHistGenMode )
   } // for( Int_t iHitInd = 0; iHitInd < iNbTofHits; iHitInd++ )
   
   // MC Tracks losses
   for(Int_t iTrkInd = 0; iTrkInd < iNbTracks; iTrkInd++)
   {
      pMcTrk = (CbmMCTrack*) fMcTracksColl->At( iTrkInd );
   
      if( kTRUE == vbTrackHasHit[iTrkInd] )
      {
            // Physics coord mapping, 1 per particle type
         Int_t iPdgCode = pMcTrk->GetPdgCode();
         Int_t iPartIdx = -1;
         for( Int_t iPart = 0; iPart < kiNbPart; iPart ++)
            if( kiPartPdgCode[iPart] == iPdgCode )
            {
               iPartIdx = iPart;
               break;
            } // if( kiPartPdgCode[iPart] == iPdgCode )
         if( -1 < iPartIdx )
         {
               fvhPtmRapGenTrkTofHit[iPartIdx]->Fill( pMcTrk->GetRapidity(), pMcTrk->GetPt()/pMcTrk->GetMass());
               fvhPlabGenTrkTofhit[iPartIdx]->Fill( pMcTrk->GetP() );
         } // if( -1 < iPartIdx )
            else 
            {
                  fvhPtmRapGenTrkTofHit[0]->Fill( pMcTrk->GetRapidity(), pMcTrk->GetPt()/pMcTrk->GetMass());
                  fvhPlabGenTrkTofhit[0]->Fill( pMcTrk->GetP() );
            } // else of if( -1 < iPartIdx )
      } // if( kTRUE == vbTrackHasHit[iTrkId] )
   } // for(Int_t iTrkInd = 0; iTrkInd < nMcTracks; iTrkInd++)
   vbTrackHasHit.clear();
   
   return kTRUE;
}
// ------------------------------------------------------------------

Bool_t CbmTofHitFinderQa::SetHistoFileNameCartCoordNorm( TString sFilenameIn )
{
   fsHistoInNormCartFilename = sFilenameIn;
   return kTRUE;
}
Bool_t CbmTofHitFinderQa::SetHistoFileNameAngCoordNorm( TString sFilenameIn )
{
   fsHistoInNormAngFilename = sFilenameIn;
   return kTRUE;
}
Bool_t CbmTofHitFinderQa::SetHistoFileNameSphCoordNorm( TString sFilenameIn )
{
   fsHistoInNormSphFilename = sFilenameIn;
   return kTRUE;
}
Bool_t CbmTofHitFinderQa::NormalizeMapHistos()
{
   // TIP: Normalization files can (should?) be generated using the FairBoxGenerator
   //      to get uniform distributions with the right density in the desired coords.
   // Normalization of Cartesian coordinates maps 
   if( "" != fsHistoInNormCartFilename )
   {
      // Open file and set folders properly to avoid creating objects in uncontrolled places
      TDirectory * oldir = gDirectory;
      TFile *fHistNorm = new TFile( fsHistoInNormCartFilename,"READ");
      if( NULL == fHistNorm )
      {
         LOG(ERROR)<<"CbmTofHitFinderQa::NormalizeMapHistos => Could not open file"
                    <<" with Normalization histos of Cartesian coordinates maps with name "
                    << fsHistoInNormCartFilename
                    <<" => Stopping normalization here!"<<FairLogger::endl;
         return kFALSE;           
      } // if( NULL == fHistNorm )
      gROOT->cd();
      
      // Prepare pointers to access normalization histos
      TH2 * fhPointMapNormXY = NULL;
      TH2 * fhPointMapNormXZ = NULL;
      TH2 * fhPointMapNormYZ = NULL;
      TH2 * fhDigiMapNormXY  = NULL;
      TH2 * fhDigiMapNormXZ  = NULL;
      TH2 * fhDigiMapNormYZ  = NULL;
      TH2 * fhHitMapNormXY   = NULL;
      TH2 * fhHitMapNormXZ   = NULL;
      TH2 * fhHitMapNormYZ   = NULL;
      
      // Get pointers for norm histos from file if they exist
      fhPointMapNormXY = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_PointsMapXY"));
      fhPointMapNormXZ = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_PointsMapXZ"));
      fhPointMapNormYZ = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_PointsMapYZ"));

      fhDigiMapNormXY = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_DigisMapXY"));
      fhDigiMapNormXZ = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_DigisMapXZ"));
      fhDigiMapNormYZ = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_DigisMapYZ"));

      fhHitMapNormXY = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_HitsMapXY"));
      fhHitMapNormXZ = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_HitsMapXZ"));
      fhHitMapNormYZ = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_HitsMapYZ"));
      LOG(INFO)<<"CbmTofHitFinderQa::NormalizeMapHistos => XY norm histos: "
                    <<fhPointMapNormXY<<" "
                    <<fhDigiMapNormXY<<" "
                    <<fhHitMapNormXY<<" "
                    <<FairLogger::endl;
      LOG(INFO)<<"CbmTofHitFinderQa::NormalizeMapHistos => XZ norm histos: "
                    <<fhPointMapNormXZ<<" "
                    <<fhDigiMapNormXZ<<" "
                    <<fhHitMapNormXZ<<" "
                    <<FairLogger::endl;
      LOG(INFO)<<"CbmTofHitFinderQa::NormalizeMapHistos => YZ norm histos: "
                    <<fhPointMapNormYZ<<" "
                    <<fhDigiMapNormYZ<<" "
                    <<fhHitMapNormYZ<<" "
                    <<FairLogger::endl;

      // If norm histo found, normalize corresponding map
      if( NULL != fhPointMapNormXY )
      {
         fhPointMapXY->Divide( fhPointMapNormXY );
      } // if( NULL != fhPointMapNormXY )
      if( NULL != fhPointMapNormXZ )
      {
         fhPointMapXZ->Divide( fhPointMapNormXZ );
      } // if( NULL != fhPointMapNormXZ )
      if( NULL != fhPointMapNormYZ )
      {
         fhPointMapYZ->Divide( fhPointMapNormYZ );
      } // if( NULL != fhPointMapNormYZ )
      if( NULL != fhDigiMapNormXY  )
      {
         fhDigiMapXY->Divide( fhDigiMapNormXY );
      } // if( NULL != fhDigiMapNormXY  )
      if( NULL != fhDigiMapNormXZ  )
      {
         fhDigiMapXZ->Divide( fhDigiMapNormXZ );
      } // if( NULL != fhDigiMapNormXZ  )
      if( NULL != fhDigiMapNormYZ  )
      {
         fhDigiMapYZ->Divide( fhDigiMapNormYZ );
      } // if( NULL != fhDigiMapNormYZ  )
      if( NULL != fhHitMapNormXY   )
      {
         fhHitMapXY->Divide( fhHitMapNormXY );
         fhHitMapSingPntXY->Divide( fhHitMapNormXY );
         fhHitMapMultPntXY->Divide( fhHitMapNormXY );
         fhHitMapSingTrkXY->Divide( fhHitMapNormXY );
         fhHitMapMultTrkXY->Divide( fhHitMapNormXY );
      } // if( NULL != fhHitMapNormXY   )
      if( NULL != fhHitMapNormXZ   )
      {
         fhHitMapXZ->Divide( fhHitMapNormXZ );
         fhHitMapSingPntXZ->Divide( fhHitMapNormXZ );
         fhHitMapMultPntXZ->Divide( fhHitMapNormXZ );
         fhHitMapSingTrkXZ->Divide( fhHitMapNormXZ );
         fhHitMapMultTrkXZ->Divide( fhHitMapNormXZ );
      } // if( NULL != fhHitMapNormXZ   )
      if( NULL != fhHitMapNormYZ   )
      {
         fhHitMapYZ->Divide( fhHitMapNormYZ );
         fhHitMapSingPntYZ->Divide( fhHitMapNormYZ );
         fhHitMapMultPntYZ->Divide( fhHitMapNormYZ );
         fhHitMapSingTrkYZ->Divide( fhHitMapNormYZ );
         fhHitMapMultTrkYZ->Divide( fhHitMapNormYZ );
      } // if( NULL != fhHitMapNormYZ   )
      
      // Go back to original folder, whereever it is
      gDirectory->cd( oldir->GetPath() );

      fHistNorm->Close();
   } // if( "" != fsHistoInNormCartFilename )
      else LOG(INFO)<<"CbmTofHitFinderQa::NormalizeMapHistos => Name of input file for"
                    <<" Normalization of Cartesian coordinates maps not given"
                    <<" => no normalization for these maps!"<<FairLogger::endl;
   // Normalization of Angular   coordinates maps 
   if( "" != fsHistoInNormAngFilename )
   {
      // Open file and set folders properly to avoid creating objects in uncontrolled places
      TDirectory * oldir = gDirectory;
      TFile *fHistNorm = new TFile( fsHistoInNormAngFilename,"READ");
      if( NULL == fHistNorm )
      {
         LOG(ERROR)<<"CbmTofHitFinderQa::NormalizeMapHistos => Could not open file"
                    <<" with Normalization histos of Angular   coordinates maps with name "
                    << fsHistoInNormAngFilename
                    <<" => Stopping normalization here!"<<FairLogger::endl;
         return kFALSE;
      } // if( NULL == fHistNorm )
      gROOT->cd();
      
      // Prepare pointers to access normalization histos
      TH2 * fhPointMapNormAng = NULL;
      TH2 * fhDigiMapNormAng  = NULL;
      TH2 * fhHitMapNormAng   = NULL;
      
      // Get pointers for norm histos from file if they exist
      fhPointMapNormAng = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_PointsMapAng"));
      fhDigiMapNormAng = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_DigisMapAng"));
      fhHitMapNormAng = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_HitsMapAng"));
      LOG(INFO)<<"CbmTofHitFinderQa::NormalizeMapHistos => Angular norm histos: "
                    <<fhPointMapNormAng<<" "
                    <<fhDigiMapNormAng<<" "
                    <<fhHitMapNormAng<<" "
                    <<FairLogger::endl;

      // If norm histo found, normalize corresponding map
      if( NULL != fhPointMapNormAng )
         fhPointMapAng->Divide( fhPointMapNormAng );
      if( NULL != fhDigiMapNormAng )
         fhDigiMapAng->Divide( fhDigiMapNormAng );
      if( NULL != fhHitMapNormAng )
      {
         fhHitMapAng->Divide( fhHitMapNormAng );
         fhHitMapSingPntAng->Divide( fhHitMapNormAng );
         fhHitMapMultPntAng->Divide( fhHitMapNormAng );
         fhHitMapSingTrkAng->Divide( fhHitMapNormAng );
         fhHitMapMultTrkAng->Divide( fhHitMapNormAng );
      }

      // Go back to original folder, whereever it is
      gDirectory->cd( oldir->GetPath() );

      fHistNorm->Close();
   } // if( "" != fsHistoInNormAngFilename )
      else LOG(INFO)<<"CbmTofHitFinderQa::NormalizeMapHistos => Name of input file for"
                    <<" Normalization of Angular   coordinates maps not given"
                    <<" => no normalization for these maps!"<<FairLogger::endl;
   // Normalization of Spherical coordinates maps 
   if( "" != fsHistoInNormSphFilename )
   {
      // Open file and set folders properly to avoid creating objects in uncontrolled places
      TDirectory * oldir = gDirectory;
      TFile *fHistNorm = new TFile( fsHistoInNormSphFilename,"READ");
      if( NULL == fHistNorm )
      {
         LOG(ERROR)<<"CbmTofHitFinderQa::NormalizeMapHistos => Could not open file"
                    <<" with Normalization histos of Spherical coordinates maps with name "
                    << fsHistoInNormSphFilename
                    <<" => Stopping normalization here!"<<FairLogger::endl;
         return kFALSE;           
      } // if( NULL == fHistNorm )
      gROOT->cd();
      
      // Prepare pointers to access normalization histos
      TH2 * fhPointMapNormSph = NULL;
      TH2 * fhDigiMapNormSph  = NULL;
      TH2 * fhHitMapNormSph   = NULL;
      
      // Get pointers for norm histos from file if they exist
      fhPointMapNormSph = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_PointsMapSph"));
      fhDigiMapNormSph = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_DigisMapSph"));
      fhHitMapNormSph = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_HitsMapSph"));
      LOG(INFO)<<"CbmTofHitFinderQa::NormalizeMapHistos => Spherical norm histos: "
                    <<fhPointMapNormSph<<" "
                    <<fhDigiMapNormSph<<" "
                    <<fhHitMapNormSph<<" "
                    <<FairLogger::endl;

      // If norm histo found, normalize corresponding map
      if( NULL != fhPointMapNormSph )
         fhPointMapSph->Divide( fhPointMapNormSph );
      if( NULL != fhDigiMapNormSph )
         fhDigiMapSph->Divide( fhDigiMapNormSph );
      if( NULL != fhHitMapNormSph )
      {
         fhHitMapSph->Divide( fhHitMapNormSph );
         fhHitMapSingPntSph->Divide( fhHitMapNormSph );
         fhHitMapMultPntSph->Divide( fhHitMapNormSph );
         fhHitMapSingTrkSph->Divide( fhHitMapNormSph );
         fhHitMapMultTrkSph->Divide( fhHitMapNormSph );
      }

      // Go back to original folder, whereever it is
      gDirectory->cd( oldir->GetPath() );

      fHistNorm->Close();
   } // if( "" != fsHistoInNormSphFilename )
      else LOG(INFO)<<"CbmTofHitFinderQa::NormalizeMapHistos => Name of input file for"
                    <<" Normalization of Spherical coordinates maps not given"
                    <<" => no normalization for these maps!"<<FairLogger::endl;
   return kTRUE;
}
Bool_t CbmTofHitFinderQa::NormalizeNormHistos()
{
   if( 0 < fhTrackMapXY->GetEntries())
   {
      fhPointMapXY->Divide( fhTrackMapXY );
      fhDigiMapXY ->Divide( fhTrackMapXY );
      fhHitMapXY  ->Divide( fhTrackMapXY );
   } // if( 0 < fhTrackMapXY->GetEntries())
   if( 0 < fhTrackMapXZ->GetEntries())
   {
      fhPointMapXZ->Divide( fhTrackMapXZ );
      fhDigiMapXZ ->Divide( fhTrackMapXZ );
      fhHitMapXZ  ->Divide( fhTrackMapXZ );
   } // if( 0 < fhTrackMapXZ->GetEntries())
   if( 0 < fhTrackMapYZ->GetEntries())
   {
      fhPointMapYZ->Divide( fhTrackMapYZ );
      fhDigiMapYZ ->Divide( fhTrackMapYZ );
      fhHitMapYZ  ->Divide( fhTrackMapYZ );
   } // if( 0 < fhTrackMapYZ->GetEntries())
   if( 0 < fhTrackMapAng->GetEntries())
   {
      fhPointMapAng->Divide( fhTrackMapAng );
      fhDigiMapAng ->Divide( fhTrackMapAng );
      fhHitMapAng  ->Divide( fhTrackMapAng );
   } // if( 0 < fhTrackMapAng->GetEntries())
   if( 0 < fhTrackMapSph->GetEntries())
   {
      fhPointMapSph->Divide( fhTrackMapSph );
      fhDigiMapSph ->Divide( fhTrackMapSph );
      fhHitMapSph  ->Divide( fhTrackMapSph );
   } // if( 0 < fhTrackMapSph->GetEntries()) 

   return kTRUE;
}
// ------------------------------------------------------------------

Bool_t CbmTofHitFinderQa::SetHistoFileName( TString sFilenameIn )
{
   fsHistoOutFilename = sFilenameIn;
   return kTRUE;
}
Bool_t CbmTofHitFinderQa::WriteHistos()
{
   // TODO: add sub-folders ?

   // Write histogramms to the file
   TDirectory * oldir = gDirectory;
   TFile *fHist = new TFile( fsHistoOutFilename,"RECREATE");
   fHist->cd();
   
      // Mapping
   TDirectory *cdGeomMap = fHist->mkdir( "GeomMap" );
   cdGeomMap->cd();    // make the "GeomMap" directory the current directory
   if( kTRUE == fbNormHistGenMode )
   {
      fhTrackMapXY->Write();
      fhTrackMapXZ->Write();
      fhTrackMapYZ->Write();
      fhTrackMapAng->Write();
      fhTrackMapSph->Write();
   } // if( kTRUE == fbNormHistGenMode )
   fhPointMapXY->Write();
   fhPointMapXZ->Write();
   fhPointMapYZ->Write();
   fhPointMapAng->Write();
   fhPointMapSph->Write();
   fhDigiMapXY->Write();
   fhDigiMapXZ->Write();
   fhDigiMapYZ->Write();
   fhDigiMapAng->Write();
   fhDigiMapSph->Write();
   fhHitMapXY->Write();
   fhHitMapXZ->Write();
   fhHitMapYZ->Write();
   fhHitMapAng->Write();
   fhHitMapSph->Write();
   
   if( kFALSE == fbNormHistGenMode )
   {
      fHist->cd(); // make the file root the current directory
         // L/R digis missmatch
      fhLeftRightDigiMatch->Write();
         // Nb different MC Points and Tracks in Hit
      fhNbPointsInHit->Write();
      fhNbTracksInHit->Write();
      
      TDirectory *cdMixMap = fHist->mkdir( "MixMap" );
      cdMixMap->cd();    // make the "MixMap" directory the current directory
         // Mapping of position for hits coming from a single MC Point
      fhHitMapSingPntXY->Write();
      fhHitMapSingPntXZ->Write();
      fhHitMapSingPntYZ->Write();
      fhHitMapSingPntAng->Write();
      fhHitMapSingPntSph->Write();
         // Mapping of position for hits coming from multiple MC Points
      fhHitMapMultPntXY->Write();
      fhHitMapMultPntXZ->Write();
      fhHitMapMultPntYZ->Write();
      fhHitMapMultPntAng->Write();
      fhHitMapMultPntSph->Write();
         // Mapping of position for hits coming from a single MC Track
      fhHitMapSingTrkXY->Write();
      fhHitMapSingTrkXZ->Write();
      fhHitMapSingTrkYZ->Write();
      fhHitMapSingTrkAng->Write();
      fhHitMapSingTrkSph->Write();
         // Mapping of position for hits coming from multiple MC Tracks
      fhHitMapMultTrkXY->Write();
      fhHitMapMultTrkXZ->Write();
      fhHitMapMultTrkYZ->Write();
      fhHitMapMultTrkAng->Write();
      fhHitMapMultTrkSph->Write();
      
      TDirectory *cdHitQual = fHist->mkdir( "HitQual" );
      cdHitQual->cd();    // make the "HitQual" directory the current directory
         // Hit Quality for Hits coming from a single MC Point
      fhSinglePointHitDeltaX->Write();
      fhSinglePointHitDeltaY->Write();
      fhSinglePointHitDeltaZ->Write();
      fhSinglePointHitDeltaR->Write();
      fhSinglePointHitDeltaT->Write();
      fhSinglePointHitPullX->Write();
      fhSinglePointHitPullY->Write();
      fhSinglePointHitPullZ->Write();
      fhSinglePointHitPullR->Write();
         // Hit Quality for Hits coming from a multiple MC Points
            // To Point closest to Hit
      fhMultiPntHitClosestDeltaX->Write();
      fhMultiPntHitClosestDeltaY->Write();
      fhMultiPntHitClosestDeltaZ->Write();
      fhMultiPntHitClosestDeltaR->Write();
      fhMultiPntHitClosestDeltaT->Write();
      fhMultiPntHitClosestPullX->Write();
      fhMultiPntHitClosestPullY->Write();
      fhMultiPntHitClosestPullZ->Write();
      fhMultiPntHitClosestPullR->Write();
            // To Point furthest from Hit
      fhMultiPntHitFurthestDeltaX->Write();
      fhMultiPntHitFurthestDeltaY->Write();
      fhMultiPntHitFurthestDeltaZ->Write();
      fhMultiPntHitFurthestDeltaR->Write();
      fhMultiPntHitFurthestDeltaT->Write();
      fhMultiPntHitFurthestPullX->Write();
      fhMultiPntHitFurthestPullY->Write();
      fhMultiPntHitFurthestPullZ->Write();
      fhMultiPntHitFurthestPullR->Write();
            // To mean Point position
      fhMultiPntHitMeanDeltaX->Write();
      fhMultiPntHitMeanDeltaY->Write();
      fhMultiPntHitMeanDeltaZ->Write();
      fhMultiPntHitMeanDeltaR->Write();
      fhMultiPntHitMeanDeltaT->Write();
      fhMultiPntHitMeanPullX->Write();
      fhMultiPntHitMeanPullY->Write();
      fhMultiPntHitMeanPullZ->Write();
      fhMultiPntHitMeanPullR->Write();
         // Hit Quality for Hits coming from a single MC Track
      fhSingleTrackHitDeltaX->Write();
      fhSingleTrackHitDeltaY->Write();
      fhSingleTrackHitDeltaZ->Write();
      fhSingleTrackHitDeltaR->Write();
      fhSingleTrackHitDeltaT->Write();
      fhSingleTrackHitPullX->Write();
      fhSingleTrackHitPullY->Write();
      fhSingleTrackHitPullZ->Write();
      fhSingleTrackHitPullR->Write();
         // Hit Quality for Hits coming from a single MC Track but multiple points
      fhSingTrkMultiPntHitDeltaX->Write();
      fhSingTrkMultiPntHitDeltaY->Write();
      fhSingTrkMultiPntHitDeltaZ->Write();
      fhSingTrkMultiPntHitDeltaR->Write();
      fhSingTrkMultiPntHitDeltaT->Write();
      fhSingTrkMultiPntHitPullX->Write();
      fhSingTrkMultiPntHitPullY->Write();
      fhSingTrkMultiPntHitPullZ->Write();
      fhSingTrkMultiPntHitPullR->Write();
         // Hit Quality for Hits coming from a multiple MC Points
            // To Track closest to Hit
      fhMultiTrkHitClosestDeltaX->Write();
      fhMultiTrkHitClosestDeltaY->Write();
      fhMultiTrkHitClosestDeltaZ->Write();
      fhMultiTrkHitClosestDeltaR->Write();
      fhMultiTrkHitClosestDeltaT->Write();
      fhMultiTrkHitClosestPullX->Write();
      fhMultiTrkHitClosestPullY->Write();
      fhMultiTrkHitClosestPullZ->Write();
      fhMultiTrkHitClosestPullR->Write();
            // To Track furthest from Hit
      fhMultiTrkHitFurthestDeltaX->Write();
      fhMultiTrkHitFurthestDeltaY->Write();
      fhMultiTrkHitFurthestDeltaZ->Write();
      fhMultiTrkHitFurthestDeltaR->Write();
      fhMultiTrkHitFurthestDeltaT->Write();
      fhMultiTrkHitFurthestPullX->Write();
      fhMultiTrkHitFurthestPullY->Write();
      fhMultiTrkHitFurthestPullZ->Write();
      fhMultiTrkHitFurthestPullR->Write();
            // To mean Track position
      fhMultiTrkHitMeanDeltaX->Write();
      fhMultiTrkHitMeanDeltaY->Write();
      fhMultiTrkHitMeanDeltaZ->Write();
      fhMultiTrkHitMeanDeltaR->Write();
      fhMultiTrkHitMeanDeltaT->Write();
      fhMultiTrkHitMeanPullX->Write();
      fhMultiTrkHitMeanPullY->Write();
      fhMultiTrkHitMeanPullZ->Write();
      fhMultiTrkHitMeanPullR->Write();
      
      TDirectory *cdPhysMap = fHist->mkdir( "PhysMap" );
      cdPhysMap->cd();    // make the "PhysMap" directory the current directory
         // Physics coord mapping, 1 per particle type
      for( Int_t iPartIdx = 0; iPartIdx < kiNbPart; iPartIdx++)
      {
            // Phase space
         fvhPtmRapGenTrk[iPartIdx]->Write();
         fvhPtmRapStsPnt[iPartIdx]->Write();
         fvhPtmRapTofPnt[iPartIdx]->Write();
         fvhPtmRapTofHit[iPartIdx]->Write();
         fvhPtmRapTofHitSinglePnt[iPartIdx]->Write();
         fvhPtmRapTofHitSingleTrk[iPartIdx]->Write();
            // PLab
         fvhPlabGenTrk[iPartIdx]->Write();
         fvhPlabStsPnt[iPartIdx]->Write();
         fvhPlabTofPnt[iPartIdx]->Write();
         fvhPlabTofHit[iPartIdx]->Write();
         fvhPlabTofHitSinglePnt[iPartIdx]->Write();
         fvhPlabTofHitSingleTrk[iPartIdx]->Write();
         // MC Tracks losses 
         fvhPtmRapGenTrkTofPnt[iPartIdx]->Write();
         fvhPtmRapGenTrkTofHit[iPartIdx]->Write();
         fvhPlabGenTrkTofPnt[iPartIdx]->Write();
         fvhPlabGenTrkTofhit[iPartIdx]->Write();
      } // for( Int_t iPartIdx = 0; iPartIdx < kiNbPart; iPartIdx++)
   } // if( kFALSE == fbNormHistGenMode )

   gDirectory->cd( oldir->GetPath() );

   fHist->Close();

   return kTRUE;
}
Bool_t   CbmTofHitFinderQa::DeleteHistos()
{
   // Mapping
   if( kTRUE == fbNormHistGenMode )
   {
      delete fhTrackMapXY;
      delete fhTrackMapXZ;
      delete fhTrackMapYZ;
      delete fhTrackMapAng;
      delete fhTrackMapSph;
   } // if( kTRUE == fbNormHistGenMode )
   delete fhPointMapXY;
   delete fhPointMapXZ;
   delete fhPointMapYZ;
   delete fhPointMapAng;
   delete fhPointMapSph;
   delete fhDigiMapXY;
   delete fhDigiMapXZ;
   delete fhDigiMapYZ;
   delete fhDigiMapAng;
   delete fhDigiMapSph;
   delete fhHitMapXY;
   delete fhHitMapXZ;
   delete fhHitMapYZ;
   delete fhHitMapAng;
   delete fhHitMapSph;
      // L/R digis missmatch
   delete fhLeftRightDigiMatch;
      // Nb different MC Points and Tracks in Hit
   delete fhNbPointsInHit;
   delete fhNbTracksInHit;
         // Mapping of position for hits coming from a single MC Point
   delete fhHitMapSingPntXY;
   delete fhHitMapSingPntXZ;
   delete fhHitMapSingPntYZ;
   delete fhHitMapSingPntAng;
   delete fhHitMapSingPntSph;
         // Mapping of position for hits coming from multiple MC Points
   delete fhHitMapMultPntXY;
   delete fhHitMapMultPntXZ;
   delete fhHitMapMultPntYZ;
   delete fhHitMapMultPntAng;
   delete fhHitMapMultPntSph;
         // Mapping of position for hits coming from a single MC Track
   delete fhHitMapSingTrkXY;
   delete fhHitMapSingTrkXZ;
   delete fhHitMapSingTrkYZ;
   delete fhHitMapSingTrkAng;
   delete fhHitMapSingTrkSph;
         // Mapping of position for hits coming from multiple MC Tracks
   delete fhHitMapMultTrkXY;
   delete fhHitMapMultTrkXZ;
   delete fhHitMapMultTrkYZ;
   delete fhHitMapMultTrkAng;
   delete fhHitMapMultTrkSph;
      // Hit Quality for Hits coming from a single MC Point
   delete fhSinglePointHitDeltaX;
   delete fhSinglePointHitDeltaY;
   delete fhSinglePointHitDeltaZ;
   delete fhSinglePointHitDeltaR;
   delete fhSinglePointHitDeltaT;
   delete fhSinglePointHitPullX;
   delete fhSinglePointHitPullY;
   delete fhSinglePointHitPullZ;
   delete fhSinglePointHitPullR;
      // Hit Quality for Hits coming from a multiple MC Points
         // To Point closest to Hit
   delete fhMultiPntHitClosestDeltaX;
   delete fhMultiPntHitClosestDeltaY;
   delete fhMultiPntHitClosestDeltaZ;
   delete fhMultiPntHitClosestDeltaR;
   delete fhMultiPntHitClosestDeltaT;
   delete fhMultiPntHitClosestPullX;
   delete fhMultiPntHitClosestPullY;
   delete fhMultiPntHitClosestPullZ;
   delete fhMultiPntHitClosestPullR;
         // To Point furthest from Hit
   delete fhMultiPntHitFurthestDeltaX;
   delete fhMultiPntHitFurthestDeltaY;
   delete fhMultiPntHitFurthestDeltaZ;
   delete fhMultiPntHitFurthestDeltaR;
   delete fhMultiPntHitFurthestDeltaT;
   delete fhMultiPntHitFurthestPullX;
   delete fhMultiPntHitFurthestPullY;
   delete fhMultiPntHitFurthestPullZ;
   delete fhMultiPntHitFurthestPullR;
         // To mean Point position
   delete fhMultiPntHitMeanDeltaX;
   delete fhMultiPntHitMeanDeltaY;
   delete fhMultiPntHitMeanDeltaZ;
   delete fhMultiPntHitMeanDeltaR;
   delete fhMultiPntHitMeanDeltaT;
   delete fhMultiPntHitMeanPullX;
   delete fhMultiPntHitMeanPullY;
   delete fhMultiPntHitMeanPullZ;
   delete fhMultiPntHitMeanPullR;
      // Hit Quality for Hits coming from a single MC Track
   delete fhSingleTrackHitDeltaX;
   delete fhSingleTrackHitDeltaY;
   delete fhSingleTrackHitDeltaZ;
   delete fhSingleTrackHitDeltaR;
   delete fhSingleTrackHitDeltaT;
   delete fhSingleTrackHitPullX;
   delete fhSingleTrackHitPullY;
   delete fhSingleTrackHitPullZ;
   delete fhSingleTrackHitPullR;
         // Hit Quality for Hits coming from a single MC Track but multiple points
   delete fhSingTrkMultiPntHitDeltaX;
   delete fhSingTrkMultiPntHitDeltaY;
   delete fhSingTrkMultiPntHitDeltaZ;
   delete fhSingTrkMultiPntHitDeltaR;
   delete fhSingTrkMultiPntHitDeltaT;
   delete fhSingTrkMultiPntHitPullX;
   delete fhSingTrkMultiPntHitPullY;
   delete fhSingTrkMultiPntHitPullZ;
   delete fhSingTrkMultiPntHitPullR;

      // Hit Quality for Hits coming from a multiple MC Points
         // To Track closest to Hit
   delete fhMultiTrkHitClosestDeltaX;
   delete fhMultiTrkHitClosestDeltaY;
   delete fhMultiTrkHitClosestDeltaZ;
   delete fhMultiTrkHitClosestDeltaR;
   delete fhMultiTrkHitClosestDeltaT;
   delete fhMultiTrkHitClosestPullX;
   delete fhMultiTrkHitClosestPullY;
   delete fhMultiTrkHitClosestPullZ;
   delete fhMultiTrkHitClosestPullR;
         // To Track furthest from Hit
   delete fhMultiTrkHitFurthestDeltaX;
   delete fhMultiTrkHitFurthestDeltaY;
   delete fhMultiTrkHitFurthestDeltaZ;
   delete fhMultiTrkHitFurthestDeltaR;
   delete fhMultiTrkHitFurthestDeltaT;
   delete fhMultiTrkHitFurthestPullX;
   delete fhMultiTrkHitFurthestPullY;
   delete fhMultiTrkHitFurthestPullZ;
   delete fhMultiTrkHitFurthestPullR;
         // To mean Track position
   delete fhMultiTrkHitMeanDeltaX;
   delete fhMultiTrkHitMeanDeltaY;
   delete fhMultiTrkHitMeanDeltaZ;
   delete fhMultiTrkHitMeanDeltaR;
   delete fhMultiTrkHitMeanDeltaT;
   delete fhMultiTrkHitMeanPullX;
   delete fhMultiTrkHitMeanPullY;
   delete fhMultiTrkHitMeanPullZ;
   delete fhMultiTrkHitMeanPullR;
   
      // Physics coord mapping, 1 per particle type
   for( Int_t iPartIdx = 0; iPartIdx < kiNbPart; iPartIdx++)
   {
         // Phase space
      delete fvhPtmRapGenTrk[iPartIdx];
      delete fvhPtmRapStsPnt[iPartIdx];
      delete fvhPtmRapTofPnt[iPartIdx];
      delete fvhPtmRapTofHit[iPartIdx];
      delete fvhPtmRapTofHitSinglePnt[iPartIdx];
      delete fvhPtmRapTofHitSingleTrk[iPartIdx];
         // PLab
      delete fvhPlabGenTrk[iPartIdx];
      delete fvhPlabStsPnt[iPartIdx];
      delete fvhPlabTofPnt[iPartIdx];
      delete fvhPlabTofHit[iPartIdx];
      delete fvhPlabTofHitSinglePnt[iPartIdx];
      delete fvhPlabTofHitSingleTrk[iPartIdx];
         // MC Tracks losses 
      delete fvhPtmRapGenTrkTofPnt[iPartIdx];
      delete fvhPtmRapGenTrkTofHit[iPartIdx];    
      delete fvhPlabGenTrkTofPnt[iPartIdx];
      delete fvhPlabGenTrkTofhit[iPartIdx]; 
   } // for( Int_t iPartIdx = 0; iPartIdx < kiNbPart; iPartIdx++)
         // Phase space
   fvhPtmRapGenTrk.clear();
   fvhPtmRapStsPnt.clear();
   fvhPtmRapTofPnt.clear();
   fvhPtmRapTofHit.clear();
   fvhPtmRapTofHitSinglePnt.clear();
   fvhPtmRapTofHitSingleTrk.clear();
         // PLab 
   fvhPlabGenTrk.clear();
   fvhPlabStsPnt.clear();
   fvhPlabTofPnt.clear();
   fvhPlabTofHit.clear();
   fvhPlabTofHitSinglePnt.clear();
   fvhPlabTofHitSingleTrk.clear();
         // MC Tracks losses      
   fvhPtmRapGenTrkTofPnt.clear();
   fvhPtmRapGenTrkTofHit.clear();
   fvhPlabGenTrkTofPnt.clear();
   fvhPlabGenTrkTofhit.clear();
   
   return kTRUE;
}


ClassImp(CbmTofHitFinderQa);
