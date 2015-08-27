/** @file CbmTofTestBeamClusterizer.cxx 
 ** author nh  
 ** adopted from
 ** @file CbmTofSimpClusterizer.cxx 
 ** @author Pierre-Alain Loizeau <loizeau@physi.uni-heidelberg.de>
 ** @date 23.08.2013
 **/

#include "CbmTofTestBeamClusterizer.h"

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
#include "CbmMatch.h"

//#include "TMbsMappingTof.h"   // in unpack/tof/mapping
#include "TMbsMappingTofPar.h"

// CBMroot classes and includes
#include "CbmMCTrack.h"

// FAIR classes and includes
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairLogger.h"

// ROOT Classes and includes
#include "TClonesArray.h"
#include "TMath.h"
#include "TLine.h"
#include "TRandom3.h"
#include "TF2.h"
#include "TVector3.h"
#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"
#include "TDirectory.h"
#include "TROOT.h"
#include "TGeoManager.h"

// Constants definitions
#include "CbmTofClusterizersDef.h"

// C++ Classes and includes

/************************************************************************************/
CbmTofTestBeamClusterizer::CbmTofTestBeamClusterizer():
   FairTask("CbmTofTestBeamClusterizer"),
   fGeoHandler(new CbmTofGeoHandler()),
   fTofId(NULL),
   fDigiPar(NULL),
   fChannelInfo(NULL),
   fDigiBdfPar(NULL),
   fMbsMappingPar(NULL),
   fTofPointsColl(NULL),
   fMcTracksColl(NULL),
   fTofDigisColl(NULL),
   fbWriteHitsInOut(kTRUE),
   fTofHitsColl(NULL),
   fTofDigiMatchColl(NULL),
   fiNbHits(0),
   fVerbose(1),
   fStorDigi(),
   fStorDigiExp(),
   fStorDigiInd(),
   fviClusterMul(),
   fviClusterSize(),
   fviTrkMul(),
   fvdX(),
   fvdY(),
   fvdDifX(),
   fvdDifY(),
   fvdDifCh(),
   fhClustBuildTime(NULL),
   fhHitsPerTracks(NULL),
   fhPtsPerHit(NULL),
   fhTimeResSingHits(NULL),
   fhTimeResSingHitsB(NULL),
   fhTimePtVsHits(NULL),
   fhClusterSize(NULL),
   fhClusterSizeType(NULL),
   fhTrackMul(NULL),
   fhClusterSizeMulti(NULL),
   fhTrk1MulPos(NULL),
   fhHiTrkMulPos(NULL),
   fhAllTrkMulPos(NULL),
   fhMultiTrkProbPos(NULL),
   fhDigSpacDifClust(NULL),
   fhDigTimeDifClust(NULL),
   fhDigDistClust(NULL),
   fhClustSizeDifX(NULL),
   fhClustSizeDifY(NULL),
   fhChDifDifX(NULL),
   fhChDifDifY(NULL),
   fhRpcDigiCor(),
   fhRpcCluMul(),
   fhRpcCluPosition(),
   fhRpcCluDelPos(),
   fhRpcCluDelMatPos(),
   fhRpcCluTOff(),
   fhRpcCluDelTOff(),
   fhRpcCluDelMatTOff(),
   fhRpcCluTrms(),
   fhRpcCluTot(),
   fhRpcCluSize(),
   fhRpcCluAvWalk(),
   fhRpcCluAvLnWalk(),
   fhRpcCluWalk(),
   fhSmCluPosition(),
   fhSmCluTOff(),
   fhTRpcCluMul(),
   fhTRpcCluPosition(),
   fhTRpcCluTOff(),
   fhTRpcCluTot(),
   fhTRpcCluSize(),
   fhTRpcCluAvWalk(),
   fhTRpcCluDelTof(),
   fhTRpcCludXdY(),
   fhTRpcCluWalk(),
   fhTSmCluPosition(),
   fhTSmCluTOff(),
   fhTSmCluTRun(),
   fhSeldT(),
   fvCPDelTof(),
   fvCPTOff(),
   fvCPTotGain(),
   fvCPTotOff(),
   fvCPWalk(),
   fiNbSameSide(0),
   fhNbSameSide(NULL),
   fhNbDigiPerChan(NULL),
   fStart(),
   fStop(),
   dTRef(0.),
   fdTRefMax(0.),
   fCalMode(0),
   fCalSel(0),
   fCalSmType(0),
   fdCaldXdYMax(0.),
   fiCluMulMax(0),
   fTRefMode(0),
   fTRefHits(0),
   fDutId(0),
   fSelId(0),
   fiBeamRefType(0),
   fiBeamRefSm(0),
   fiBeamRefDet(0),
   fiBeamAddRefMul(0),
   fSel2Id(0),
   fPosYMaxScal(0.),
   fTRefDifMax(0.),
   fTotMax(0.),
   fTotMin(0.),
   fTotOff(0.),
   fTotMean(0.),
   fTotPreRange(0.),
   fMaxTimeDist(0.),
   fCalParFileName(""),
   fOutHstFileName(""),
   fCalParFile(NULL),      
   fiNevtBuild(0),
   fiMsgCnt(100),
   fdTOTMax(5.E4),
   fdTOTMin(0.),
   fdTTotMean(5.E3), //2.E4),
   fdMaxTimeDist(0.),
   fdMaxSpaceDist(0.),
   fdEvent(0)
{

}

CbmTofTestBeamClusterizer::CbmTofTestBeamClusterizer(const char *name, Int_t verbose, Bool_t writeDataInOut):
   FairTask(TString(name),verbose),
   fGeoHandler(new CbmTofGeoHandler()),
   fTofId(NULL),
   fDigiPar(NULL),
   fChannelInfo(NULL),
   fDigiBdfPar(NULL),
   fMbsMappingPar(NULL),
   fTofPointsColl(NULL),
   fMcTracksColl(NULL),
   fTofDigisColl(NULL),
   fbWriteHitsInOut(writeDataInOut),
   fTofHitsColl(NULL),
   fTofDigiMatchColl(NULL),
   fiNbHits(0),
   fVerbose(verbose),
   fStorDigi(),
   fStorDigiExp(),
   fStorDigiInd(),
   fviClusterMul(),
   fviClusterSize(),
   fviTrkMul(),
   fvdX(),
   fvdY(),
   fvdDifX(),
   fvdDifY(),
   fvdDifCh(),
   fhClustBuildTime(NULL),
   fhHitsPerTracks(NULL),
   fhPtsPerHit(NULL),
   fhTimeResSingHits(NULL),
   fhTimeResSingHitsB(NULL),
   fhTimePtVsHits(NULL),
   fhClusterSize(NULL),
   fhClusterSizeType(NULL),
   fhTrackMul(NULL),
   fhClusterSizeMulti(NULL),
   fhTrk1MulPos(NULL),
   fhHiTrkMulPos(NULL),
   fhAllTrkMulPos(NULL),
   fhMultiTrkProbPos(NULL),
   fhDigSpacDifClust(NULL),
   fhDigTimeDifClust(NULL),
   fhDigDistClust(NULL),
   fhClustSizeDifX(NULL),
   fhClustSizeDifY(NULL),
   fhChDifDifX(NULL),
   fhChDifDifY(NULL),
   fhRpcDigiCor(),
   fhRpcCluMul(),
   fhRpcCluPosition(),
   fhRpcCluDelPos(),
   fhRpcCluDelMatPos(),
   fhRpcCluTOff(),
   fhRpcCluDelTOff(),
   fhRpcCluDelMatTOff(),
   fhRpcCluTrms(),
   fhRpcCluTot(),
   fhRpcCluSize(),
   fhRpcCluAvWalk(),
   fhRpcCluAvLnWalk(),
   fhRpcCluWalk(),
   fhSmCluPosition(),
   fhSmCluTOff(),
   fhTRpcCluMul(),
   fhTRpcCluPosition(),
   fhTRpcCluTOff(),
   fhTRpcCluTot(),
   fhTRpcCluSize(),
   fhTRpcCluAvWalk(),
   fhTRpcCluDelTof(),
   fhTRpcCludXdY(),
   fhTRpcCluWalk(),
   fhTSmCluPosition(),
   fhTSmCluTOff(),
   fhTSmCluTRun(),
   fhSeldT(),
   fvCPDelTof(),
   fvCPTOff(),
   fvCPTotGain(),
   fvCPTotOff(),
   fvCPWalk(),
   fiNbSameSide(0),
   fhNbSameSide(NULL),
   fhNbDigiPerChan(NULL),
   fStart(),
   fStop(),
   dTRef(0.),
   fdTRefMax(0.),
   fCalMode(0),
   fCalSel(0),
   fCalSmType(0),
   fdCaldXdYMax(0.),
   fiCluMulMax(0),
   fTRefMode(0),
   fTRefHits(0),
   fDutId(0),
   fSelId(0),
   fiBeamRefType(0),
   fiBeamRefSm(0),
   fiBeamRefDet(0),
   fiBeamAddRefMul(0),
   fSel2Id(0),
   fPosYMaxScal(0.),
   fTRefDifMax(0.),
   fTotMax(0.),
   fTotMin(0.),
   fTotOff(0.),
   fTotMean(0.),
   fTotPreRange(0.),
   fMaxTimeDist(0.),
   fCalParFileName(""),
   fOutHstFileName(""),
   fCalParFile(NULL),      
   fiNevtBuild(0),
   fiMsgCnt(100),
   fdTOTMax(5.E4),
   fdTOTMin(0.),
   fdTTotMean(5.E3), //2.E4),
   fdMaxTimeDist(0.),
   fdMaxSpaceDist(0.),
   fdEvent(0)
{
  //  LOG(INFO) << "CbmTofTestBeamClusterizer initializing..."<<FairLogger::endl;
}

CbmTofTestBeamClusterizer::~CbmTofTestBeamClusterizer()
{
   if( fGeoHandler )
      delete fGeoHandler;
//   DeleteHistos(); // <-- if needed  ?
}

/************************************************************************************/
// FairTasks inherited functions
InitStatus CbmTofTestBeamClusterizer::Init()
{
   if( kFALSE == RegisterInputs() )
      return kFATAL;

   if( kFALSE == RegisterOutputs() )
      return kFATAL;

   if( kFALSE == InitParameters() )
      return kFATAL;

   if( kFALSE == LoadGeometry() )
      return kFATAL;

   if( kFALSE == InitCalibParameter() )
      return kFATAL;

   if( kFALSE == CreateHistos() )
      return kFATAL;

   return kSUCCESS;
}


void CbmTofTestBeamClusterizer::SetParContainers()
{
   LOG(INFO)<<" CbmTofTestBeamClusterizer::SetParContainers => Get the digi parameters for tof"<<FairLogger::endl;
   LOG(WARNING)<<" CbmTofTestBeamClusterizer::SetParContainers: Return without action"<<FairLogger::endl;
   return;
   // Get Base Container
   FairRunAna* ana = FairRunAna::Instance();
   FairRuntimeDb* rtdb=ana->GetRuntimeDb();

   fDigiPar = (CbmTofDigiPar*) (rtdb->getContainer("CbmTofDigiPar"));

   LOG(INFO)<<"  CbmTofTestBeamClusterizer::SetParContainers found " 
            << fDigiPar->GetNrOfModules() << " cells " <<FairLogger::endl;
   fDigiBdfPar = (CbmTofDigiBdfPar*)
              (rtdb->getContainer("CbmTofDigiBdfPar"));
}


void CbmTofTestBeamClusterizer::Exec(Option_t * option)
{
   fTofHitsColl->Clear("C");
   //for (Int_t i=0; i<fTofDigiMatchColl->GetEntries(); i++) ((CbmMatch *)(fTofDigiMatchColl->At(i)))->ClearLinks();  // FIXME, try to tamper memory leak (did not help)
   //fTofDigiMatchColl->Clear("C+L");
   fTofDigiMatchColl->Delete();

   fiNbHits = 0;

   LOG(DEBUG)<<"CbmTofTestBeamClusterizer::Exec => New event"<<FairLogger::endl;
   fStart.Set();

   BuildClusters();

   MergeClusters();

   fStop.Set();
   
   fdEvent++;
   FillHistos();
}

/************************************************************************************/
void CbmTofTestBeamClusterizer::Finish()
{
   WriteHistos();
   // Prevent them from being sucked in by the CbmHadronAnalysis WriteHistograms method
   DeleteHistos();
}

void CbmTofTestBeamClusterizer::Finish(Double_t calMode)
{
   SetCalMode(calMode);
   WriteHistos();
}

/************************************************************************************/
// Functions common for all clusters approximations
Bool_t   CbmTofTestBeamClusterizer::RegisterInputs()
{
   FairRootManager *fManager = FairRootManager::Instance();
   /*
   fTofPointsColl  = (TClonesArray *) fManager->GetObject("TofPoint");
   if( NULL == fTofPointsColl)
   {
      LOG(ERROR)<<"CbmTofTestBeamClusterizer::RegisterInputs => Could not get the TofPoint TClonesArray!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fTofPointsColl)

   fMcTracksColl   = (TClonesArray *) fManager->GetObject("MCTrack");
   if( NULL == fMcTracksColl)
   {
      LOG(ERROR)<<"CbmTofTestBeamClusterizer::RegisterInputs => Could not get the MCTrack TClonesArray!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fMcTracksColl)
   */

   fTofDigisColl   = (TClonesArray *) fManager->GetObject("CbmTofDigiExp");

   if( NULL == fTofDigisColl)
      fTofDigisColl = (TClonesArray *) fManager->GetObject("CbmTofDigi");
  
   if( NULL == fTofDigisColl)
   {
      LOG(ERROR)<<"CbmTofTestBeamClusterizer::RegisterInputs => Could not get the CbmTofDigi TClonesArray!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fTofDigisColl)

   return kTRUE;
}
Bool_t   CbmTofTestBeamClusterizer::RegisterOutputs()
{
   FairRootManager* rootMgr = FairRootManager::Instance();

   fTofHitsColl = new TClonesArray("CbmTofHit");

   // Flag check to control whether digis are written in output root file
   rootMgr->Register( "TofHit","Tof", fTofHitsColl, fbWriteHitsInOut);

   fTofDigiMatchColl = new TClonesArray("CbmMatch",100);
   rootMgr->Register( "TofDigiMatch","Tof", fTofDigiMatchColl, fbWriteHitsInOut);

   return kTRUE;

}
Bool_t   CbmTofTestBeamClusterizer::InitParameters()
{

   // Initialize the TOF GeoHandler
   Bool_t isSimulation=kFALSE;
   LOG(INFO)<<"CbmTofTestBeamClusterizer::InitParameters - Geometry, Mapping, ...  ??"
             <<FairLogger::endl;

   // Get Base Container
   FairRun* ana = FairRun::Instance();
   FairRuntimeDb* rtdb=ana->GetRuntimeDb();   

   Int_t iGeoVersion = fGeoHandler->Init(isSimulation);
   if( k14a > iGeoVersion )
   {
      LOG(ERROR)<<"CbmTofTestBeamClusterizer::InitParameters => Only compatible with geometries after v14a !!!"
                <<FairLogger::endl;
      return kFALSE;
   }
   fTofId = new CbmTofDetectorId_v14a();
   
   // Mapping parameter
   fMbsMappingPar = (TMbsMappingTofPar*) (rtdb->getContainer("TMbsMappingTofPar"));
   if( 0 == fMbsMappingPar )
   {
      LOG(ERROR)<<"CbmTofTestBeamClusterizer::InitParameters => Could not obtain the TMbsMappingTofPar "<<FairLogger::endl;
      return kFALSE; 
   }

   fDigiPar = (CbmTofDigiPar*) (rtdb->getContainer("CbmTofDigiPar"));
   if( 0 == fDigiPar )
   {
      LOG(ERROR)<<"CbmTofTestBeamClusterizer::InitParameters => Could not obtain the CbmTofDigiPar "<<FairLogger::endl;
      return kFALSE; 
   }

   fDigiBdfPar = (CbmTofDigiBdfPar*) (rtdb->getContainer("CbmTofDigiBdfPar"));
   if( 0 == fDigiBdfPar )
   {
      LOG(ERROR)<<"CbmTofTestBeamClusterizer::InitParameters => Could not obtain the CbmTofDigiBdfPar "<<FairLogger::endl;
      return kFALSE; 
   }

   rtdb->initContainers(  ana->GetRunId() );

   LOG(INFO)<<"CbmTofTestBeamClusterizer::InitParameter: currently " 
            << fDigiPar->GetNrOfModules() << " digi cells " <<FairLogger::endl;

   
   fdMaxTimeDist  = fDigiBdfPar->GetMaxTimeDist()*1000.;  // in ps
   fdMaxSpaceDist = fDigiBdfPar->GetMaxDistAlongCh();     // in cm

   if(fMaxTimeDist!=fdMaxTimeDist) {
     fdMaxTimeDist=fMaxTimeDist;            // modify default
     fdMaxSpaceDist=fdMaxTimeDist*0.017*0.5; // cut consistently on positions (with default signal velocity)
   }

   LOG(INFO)<<" BuildCluster with MaxTimeDist "<<fdMaxTimeDist<<", MaxSpaceDist "
	     <<fdMaxSpaceDist
	     <<FairLogger::endl;

   if(fiCluMulMax==0) fiCluMulMax=100;
   if (fOutHstFileName==""){
     fOutHstFileName="./tofTestBeamClust.hst.root";
   }

   LOG(INFO)<<" Hst Output filename = "<<fOutHstFileName
	     <<FairLogger::endl;

   if(fiBeamRefType == 0) {  // initialize defaults of sep14
     fiBeamRefType  = 5;
     fiBeamRefSm    = 1;
     fiBeamRefDet   = 0;
     fiBeamAddRefMul= 0;
   }
   if(fSelId == 0) {  // initialize defaults of sep14
     fSelId=4;
   }

   LOG(INFO)<<"<I>  BeamRefType = "<<fiBeamRefType<<", SM "<<fiBeamRefSm
	     <<FairLogger::endl;

   return kTRUE;
}
/************************************************************************************/
Bool_t   CbmTofTestBeamClusterizer::InitCalibParameter()
{
  // dimension and initialize calib parameter
  Int_t iNbDet=fMbsMappingPar->GetNbMappedDet();
  Int_t iNbSmTypes = fDigiBdfPar->GetNbSmTypes();

  if (fTotMean !=0.) fdTTotMean=fTotMean;   // adjust target mean for TTT

  fvCPTOff.resize( iNbSmTypes );
  fvCPTotGain.resize( iNbSmTypes );
  fvCPTotOff.resize( iNbSmTypes );
  fvCPWalk.resize( iNbSmTypes );
  fvCPDelTof.resize( iNbSmTypes );
  for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
  {
      Int_t iNbSm  = fDigiBdfPar->GetNbSm(  iSmType);
      Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType);
      fvCPTOff[iSmType].resize( iNbSm*iNbRpc );
      fvCPTotGain[iSmType].resize( iNbSm*iNbRpc );
      fvCPTotOff[iSmType].resize( iNbSm*iNbRpc );
      fvCPWalk[iSmType].resize( iNbSm*iNbRpc );
      fvCPDelTof[iSmType].resize( iNbSm*iNbRpc );
      for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
      {
        for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ )
        {
	  //	  LOG(INFO)<<Form(" fvCPDelTof resize for SmT %d, R %d, B %d ",iSmType,iNbSm*iNbRpc,nbClDelTofBinX)
	  //	   <<FairLogger::endl;
	  fvCPDelTof[iSmType][iSm*iNbRpc+iRpc].resize( nbClDelTofBinX );
	  for(Int_t iBx=0; iBx<nbClDelTofBinX; iBx++){ 
	    // LOG(INFO)<<Form(" fvCPDelTof for SmT %d, R %d, B %d",iSmType,iSm*iNbRpc+iRpc,iBx)<<FairLogger::endl;
  	    fvCPDelTof[iSmType][iSm*iNbRpc+iRpc][iBx].resize( iNSel );
            for(Int_t iSel=0; iSel<iNSel; iSel++)
      	    fvCPDelTof[iSmType][iSm*iNbRpc+iRpc][iBx][iSel]=0.;  // initialize
	  }

          Int_t iNbChan = fDigiBdfPar->GetNbChan( iSmType, iRpc );
	  fvCPTOff[iSmType][iSm*iNbRpc+iRpc].resize( iNbChan );
	  fvCPTotGain[iSmType][iSm*iNbRpc+iRpc].resize( iNbChan );
	  fvCPTotOff[iSmType][iSm*iNbRpc+iRpc].resize( iNbChan );
	  fvCPWalk[iSmType][iSm*iNbRpc+iRpc].resize( iNbChan );
	  Int_t nbSide  =2 - fDigiBdfPar->GetChanType( iSmType, iRpc );
	  for (Int_t iCh=0; iCh<iNbChan; iCh++)
	  {
	    fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh].resize( nbSide );
	    fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh].resize( nbSide );
	    fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh].resize( nbSide );
	    fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh].resize( nbSide );	  
	    for(Int_t iSide=0; iSide<nbSide; iSide++){
	      fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][iSide]=0.;      //initialize
	      fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide]=1.;   //initialize
	      fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][iSide]=0.;   //initialize
              fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][iSide].resize( nbClWalkBinX );
	      for(Int_t iWx=0; iWx<nbClWalkBinX; iWx++){
		fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][iSide][iWx]=0.;
	      }
	    }    
	  }
	}
      }
  }

  LOG(INFO)<<"CbmTofTestBeamClusterizer::InitCalibParameter: defaults set"
	   <<FairLogger::endl;

  TDirectory * oldir = gDirectory; // <= To prevent histos from being sucked in by the param file of the TRootManager!
  /*
  gROOT->cd(); // <= To prevent histos from being sucked in by the param file of the TRootManager !
  */

  if(0<fCalMode){
    LOG(INFO) << "CbmTofTestBeamClusterizer::InitCalibParameter: read histos from "
		 << "file " << fCalParFileName << FairLogger::endl;

  // read parameter from histos
    if(fCalParFileName.IsNull()) return kTRUE;

    fCalParFile = new TFile(fCalParFileName,"");
    if(NULL == fCalParFile) {
      LOG(ERROR) << "CbmTofTestBeamClusterizer::InitCalibParameter: "
		 << "file " << fCalParFileName << " does not exist!" << FairLogger::endl;
      return kTRUE;
    }
    /*
    gDirectory->Print();
    fCalParFile->cd();
    fCalParFile->ls();
    */

    for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
    {
      Int_t iNbSm  = fDigiBdfPar->GetNbSm(  iSmType );
      Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType );
      for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
        for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ )
        {
	  TH2F *htempPos_pfx =(TH2F*) gDirectory->FindObjectAny( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Pos_pfx",iSmType,iSm,iRpc));
	  TH2F *htempTOff_pfx=(TH2F*) gDirectory->FindObjectAny( Form("cl_CorSmT%01d_sm%03d_rpc%03d_TOff_pfx",iSmType,iSm,iRpc));
	  TH1D *htempTot_Mean =(TH1D*) gDirectory->FindObjectAny( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Tot_Mean",iSmType,iSm,iRpc));
	  TH1D *htempTot_Off  =(TH1D*) gDirectory->FindObjectAny( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Tot_Off",iSmType,iSm,iRpc));
          if(NULL != htempPos_pfx && NULL != htempTOff_pfx && NULL != htempTot_Mean && NULL != htempTot_Off)   
	  {
	    Int_t iNbCh = fDigiBdfPar->GetNbChan( iSmType, iRpc );
            Int_t iNbinTot = htempTot_Mean->GetNbinsX();
	    for( Int_t iCh = 0; iCh < iNbCh; iCh++ )
	      {
		Double_t YMean=((TProfile *)htempPos_pfx)->GetBinContent(iCh+1);  //nh +1 empirical(?)
		Double_t TMean=((TProfile *)htempTOff_pfx)->GetBinContent(iCh+1);
                //Double_t dTYOff=YMean/fDigiBdfPar->GetSignalSpeed() ;
                Double_t dTYOff=YMean/fDigiBdfPar->GetSigVel(iSmType,iRpc) ;
		fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0] += -dTYOff + TMean ;
		fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1] += +dTYOff + TMean ;
 
		for(Int_t iSide=0; iSide<2; iSide++){
		  Double_t TotMean=htempTot_Mean->GetBinContent(iCh*2+1+iSide);  //nh +1 empirical(?)
		  if(1<TotMean){
		    fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide] *= fdTTotMean / TotMean;
		  }
		  fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][iSide]=htempTot_Off->GetBinContent(iCh*2+1+iSide);
		}

		if(5 == iSmType || 8 == iSmType){
		  fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]=fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0];
		  fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][1] = fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][0];
		  fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]=fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][0];
		}

		LOG(DEBUG)<<"CbmTofTestBeamClusterizer::InitCalibParameter:" 
			   <<" SmT "<< iSmType<<" Sm "<<iSm<<" Rpc "<<iRpc<<" Ch "<<iCh
			 << Form(": YMean %f, TMean %f",YMean, TMean)
			 <<" -> " << Form(" %f, %f, %f, %f ",fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0],
				    fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1],
				    fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][0],
				    fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][1]) 
			   <<", NbinTot "<< iNbinTot
			   <<FairLogger::endl;

		TH1D *htempWalk0=(TH1D*)gDirectory->FindObjectAny( 
			         Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S0_Walk_px", iSmType, iSm, iRpc, iCh ));
		TH1D *htempWalk1=(TH1D*)gDirectory->FindObjectAny( 
			         Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S1_Walk_px", iSmType, iSm, iRpc, iCh ));
		if(NULL != htempWalk0 && NULL != htempWalk1 ) { // reinitialize Walk array 
		  LOG(DEBUG)<<"Initialize Walk correction for "
			    <<Form(" SmT%01d_sm%03d_rpc%03d_Ch%03d",iSmType, iSm, iRpc, iCh)
			    <<FairLogger::endl;
                  if(htempWalk0->GetNbinsX() != nbClWalkBinX) 
		    LOG(ERROR)<<"CbmTofTestBeamClusterizer::InitCalibParameter: Inconsistent Walk histograms"
			      <<FairLogger::endl;
                   for( Int_t iBin = 0; iBin < nbClWalkBinX; iBin++ )
		   {
		     fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iBin]=htempWalk0->GetBinContent(iBin+1);
		     fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iBin]=htempWalk1->GetBinContent(iBin+1);
		     LOG(DEBUG1)<<Form(" SmT%01d_sm%03d_rpc%03d_Ch%03d bin %d walk %f ",iSmType, iSm, iRpc, iCh, iBin,
				       fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iBin])
			      <<FairLogger::endl;
		     if(5 == iSmType || 8 == iSmType){
		       fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iBin]=fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iBin];
		     }
		   }
		}
	    }
	  }
	  else {
	    LOG(WARNING)<<" Calibration histos " << Form("cl_SmT%01d_sm%03d_rpc%03d_XXX", iSmType, iSm, iRpc) 
		      << " not found. "
		      <<FairLogger::endl;
	  }
	  for(Int_t iSel=0; iSel<iNSel; iSel++){
	   TH1D *htmpDelTof =(TH1D*) gDirectory->FindObjectAny( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));
	   if (NULL==htmpDelTof) {
	    LOG(DEBUG)<<" Histos " << Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof", iSmType, iSm, iRpc, iSel) << " not found. "
		      <<FairLogger::endl;
	    continue;
	   }
	   LOG(DEBUG)<<" Load DelTof from histos "<< Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel)<<"."
	            <<FairLogger::endl;
           for(Int_t iBx=0; iBx<nbClDelTofBinX; iBx++){
	    fvCPDelTof[iSmType][iSm*iNbRpc+iRpc][iBx][iSel] += htmpDelTof->GetBinContent(iBx+1);
	   }

	   // copy Histo to memory
	   TDirectory * curdir = gDirectory;
	   gDirectory->cd( oldir->GetPath() );
	   TH1D *h1DelTof=(TH1D *)htmpDelTof->Clone(Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));

	   LOG(DEBUG)<<" copy histo "
	 	     <<h1DelTof->GetName()
	 	     <<" to directory "
		     <<oldir->GetName()
		     <<FairLogger::endl;

	   gDirectory->cd( curdir->GetPath() );
	  }
	}
    }
  }
  //   fCalParFile->Delete();
  gDirectory->cd( oldir->GetPath() ); // <= To prevent histos from being sucked in by the param file of the TRootManager!
  LOG(INFO)<<"CbmTofTestBeamClusterizer::InitCalibParameter: initialization done"
	   <<FairLogger::endl; 
  return kTRUE;
}
/************************************************************************************/
Bool_t   CbmTofTestBeamClusterizer::LoadGeometry()
{
   LOG(INFO)<<"CbmTofTestBeamClusterizer::LoadGeometry starting for  "
	    <<fMbsMappingPar->GetNbMappedDet() << " mapped detectors, "
            <<fDigiPar->GetNrOfModules() << " geometrically known ones "
            <<FairLogger::endl;

   //fTRefMode=0x00002016;  // use plastic 1 as time reference 

   Int_t iNrOfCells = fDigiPar->GetNrOfModules();
   LOG(INFO)<<"Digi Parameter container contains "<<iNrOfCells<<" cells."<<FairLogger::endl;
   for (Int_t icell = 0; icell < iNrOfCells; ++icell) {

     Int_t cellId = fDigiPar->GetCellId(icell); // cellId is assigned in CbmTofCreateDigiPar
     fChannelInfo = fDigiPar->GetCell(cellId);

     Int_t smtype  = fGeoHandler->GetSMType(cellId);
     Int_t smodule = fGeoHandler->GetSModule(cellId);
     Int_t module  = fGeoHandler->GetCounter(cellId);
     Int_t cell    = fGeoHandler->GetCell(cellId);

     Double_t x = fChannelInfo->GetX();
     Double_t y = fChannelInfo->GetY();
     Double_t z = fChannelInfo->GetZ();
     Double_t dx = fChannelInfo->GetSizex();
     Double_t dy = fChannelInfo->GetSizey();
     LOG(DEBUG)
       << "-I- InitPar "<<icell<<" Id: "<< Form("0x%08x",cellId)
	    << " "<< cell << " tmcs: "<< smtype <<" "<<smodule<<" "<<module<<" "<<cell   
            << " x="<<Form("%6.2f",x)<<" y="<<Form("%6.2f",y)<<" z="<<Form("%6.2f",z)
            <<" dx="<<dx<<" dy="<<dy<<FairLogger::endl;

      TGeoNode *fNode=        // prepare local->global trafo
      gGeoManager->FindNode(fChannelInfo->GetX(),fChannelInfo->GetY(),fChannelInfo->GetZ());
      LOG(DEBUG2)<<Form(" Node at (%6.1f,%6.1f,%6.1f) : 0x%p",
		          fChannelInfo->GetX(),fChannelInfo->GetY(),fChannelInfo->GetZ(),fNode)
		 <<FairLogger::endl;
      //      fNode->Print();	

   }

   Int_t iNbDet=fMbsMappingPar->GetNbMappedDet();
   for(Int_t iDetIndx=0; iDetIndx<iNbDet; iDetIndx++){
       Int_t iUniqueId = fMbsMappingPar->GetMappedDetUId( iDetIndx );
       Int_t iSmType   = CbmTofAddress::GetSmType( iUniqueId );
       Int_t iSmId     = CbmTofAddress::GetSmId( iUniqueId );
       Int_t iRpcId    = CbmTofAddress::GetRpcId( iUniqueId );
       LOG(INFO) << " DetIndx "<<iDetIndx<<", SmType "<<iSmType<<", SmId "<<iSmId
                 << ", RpcId "<<iRpcId<<" => UniqueId "<<Form("0x%08x ",iUniqueId)
		 << Form(" Svel %6.3f ",fDigiBdfPar->GetSigVel(iSmType,iRpcId))
                 <<FairLogger::endl;
       Int_t iCell=-1; 
       while (kTRUE){
	 Int_t iUCellId= CbmTofAddress::GetUniqueAddress(iSmId,iRpcId,++iCell,0,iSmType);
         fChannelInfo = fDigiPar->GetCell(iUCellId);
         if (NULL == fChannelInfo) break;
	 LOG(DEBUG) << " Cell " << iCell << Form(" 0x%08x ",iUCellId)
                   << Form(", fCh 0x%p ",fChannelInfo)
                   << ", x: " << fChannelInfo->GetX()
                   << ", y: " << fChannelInfo->GetY()
                   << ", z: " << fChannelInfo->GetZ()
                   << FairLogger::endl; 
	 }

   }

   //   return kTRUE;

   Int_t iNbSmTypes = fDigiBdfPar->GetNbSmTypes();

   if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
   {
      fStorDigiExp.resize( iNbSmTypes );
      fStorDigiInd.resize( iNbSmTypes );
      fviClusterSize.resize( iNbSmTypes );
      fviTrkMul.resize( iNbSmTypes );
      fvdX.resize( iNbSmTypes );
      fvdY.resize( iNbSmTypes );
      fvdDifX.resize( iNbSmTypes );
      fvdDifY.resize( iNbSmTypes );
      fvdDifCh.resize( iNbSmTypes );
      fviClusterMul.resize( iNbSmTypes );
      for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
      {
         Int_t iNbSm  = fDigiBdfPar->GetNbSm(  iSmType);
         Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType);
         fStorDigiExp[iSmType].resize( iNbSm*iNbRpc );
         fStorDigiInd[iSmType].resize( iNbSm*iNbRpc );
         fviClusterSize[iSmType].resize( iNbRpc );
         fviTrkMul[iSmType].resize( iNbRpc );
         fvdX[iSmType].resize( iNbRpc );
         fvdY[iSmType].resize( iNbRpc );
         fvdDifX[iSmType].resize( iNbRpc );
         fvdDifY[iSmType].resize( iNbRpc );
         fvdDifCh[iSmType].resize( iNbRpc );
         for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
         {
           fviClusterMul[iSmType].resize( iNbSm );
           for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ )
            {
 	       fviClusterMul[iSmType][iSm].resize( iNbRpc );
               Int_t iNbChan = fDigiBdfPar->GetNbChan( iSmType, iRpc );
               LOG(DEBUG)<<"CbmTofTestBeamClusterizer::LoadGeometry: StoreDigi with "
			 << Form(" %3d %3d %3d %3d %5d ",iSmType,iSm,iNbRpc,iRpc,iNbChan)
                         << FairLogger::endl;
               fStorDigiExp[iSmType][iSm*iNbRpc+iRpc].resize( iNbChan );
               fStorDigiInd[iSmType][iSm*iNbRpc+iRpc].resize( iNbChan );
            } // for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ )
         } // for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
      } // for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
   } // if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
      else
      {
         fStorDigi.resize( iNbSmTypes );
         fStorDigiInd.resize( iNbSmTypes );
         fviClusterSize.resize( iNbSmTypes );
         fviTrkMul.resize( iNbSmTypes );
         fvdX.resize( iNbSmTypes );
         fvdY.resize( iNbSmTypes );
         fvdDifX.resize( iNbSmTypes );
         fvdDifY.resize( iNbSmTypes );
         fvdDifCh.resize( iNbSmTypes );
         for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
         {
            Int_t iNbSm  = fDigiBdfPar->GetNbSm(  iSmType);
            Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType);
            fStorDigi[iSmType].resize( iNbSm*iNbRpc );
            fStorDigiInd[iSmType].resize( iNbSm*iNbRpc );
            fviClusterSize[iSmType].resize( iNbRpc );
            fviTrkMul[iSmType].resize( iNbRpc );
            fvdX[iSmType].resize( iNbRpc );
            fvdY[iSmType].resize( iNbRpc );
            fvdDifX[iSmType].resize( iNbRpc );
            fvdDifY[iSmType].resize( iNbRpc );
            fvdDifCh[iSmType].resize( iNbRpc );
            for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
            {
               for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ )
               {
                  Int_t iNbChan = fDigiBdfPar->GetNbChan( iSmType, iRpc ); 
                  fStorDigi[iSmType][iSm*iNbRpc+iRpc].resize( iNbChan );
                  fStorDigiInd[iSmType][iSm*iNbRpc+iRpc].resize( iNbChan );
              } // for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ )
            } // for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
         } // for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
      } // else of if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
   return kTRUE;
}
Bool_t   CbmTofTestBeamClusterizer::DeleteGeometry()
{
   LOG(INFO)<<"CbmTofTestBeamClusterizer::DeleteGeometry starting"
             <<FairLogger::endl;
   return kTRUE;
   Int_t iNbSmTypes = fDigiBdfPar->GetNbSmTypes();
   if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
   {
      for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
      {
         Int_t iNbSm  = fDigiBdfPar->GetNbSm(  iSmType);
         Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType);
         for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
         {
	   for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ ) {
               fStorDigiExp[iSmType][iSm*iNbRpc+iRpc].clear();
               fStorDigiInd[iSmType][iSm*iNbRpc+iRpc].clear();
	   }
         } // for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
         fStorDigiExp[iSmType].clear();
         fStorDigiInd[iSmType].clear();
      } // for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
      fStorDigiExp.clear();
      fStorDigiInd.clear();
   } // if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
      else
      {
         for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
         {
            Int_t iNbSm  = fDigiBdfPar->GetNbSm(  iSmType);
            Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType);
            for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
            {
	      for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ ){
                 fStorDigi[iSmType][iSm*iNbRpc+iRpc].clear();
                 fStorDigiInd[iSmType][iSm*iNbRpc+iRpc].clear();
	      }
            } // for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
            fStorDigi[iSmType].clear();
            fStorDigiInd[iSmType].clear();
         } // for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
         fStorDigi.clear();
         fStorDigiInd.clear();
      } // else of if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
   return kTRUE;
}
/************************************************************************************/
// Histogramming functions
Bool_t   CbmTofTestBeamClusterizer::CreateHistos()
{
   TDirectory * oldir = gDirectory; // <= To prevent histos from being sucked in by the param file of the TRootManager!
   gROOT->cd(); // <= To prevent histos from being sucked in by the param file of the TRootManager !
   fhClustBuildTime = new TH1I( "TofTestBeamClus_ClustBuildTime", "Time needed to build clusters in each event; Time [s]",4000, 0.0, 4.0 );

   TH2* hTemp = 0;

   // Sm related distributions 
   fhSmCluPosition.resize( fDigiBdfPar->GetNbSmTypes() );
   fhSmCluTOff.resize( fDigiBdfPar->GetNbSmTypes() );
   fhTSmCluPosition.resize( fDigiBdfPar->GetNbSmTypes() );
   fhTSmCluTOff.resize( fDigiBdfPar->GetNbSmTypes() );
   fhTSmCluTRun.resize( fDigiBdfPar->GetNbSmTypes() );

   for (Int_t iS=0; iS< fDigiBdfPar->GetNbSmTypes(); iS++){
     Double_t YSCAL=50.;
     if (fPosYMaxScal !=0.) YSCAL=fPosYMaxScal;
     Int_t iUCellId  = CbmTofAddress::GetUniqueAddress(0,0,0,0,iS);
     fChannelInfo = fDigiPar->GetCell(iUCellId);
     if(NULL == fChannelInfo){
	 LOG(WARNING)<<"CbmTofTestBeamClusterizer::CreateHistos: No DigiPar for SmType "
                   <<Form("%d, 0x%08x", iS, iUCellId)
		   <<FairLogger::endl;
	 continue;     
     }
     Double_t YDMAX=TMath::Max(2.,fChannelInfo->GetSizey())*YSCAL;

     fhSmCluPosition[iS] =  new TH2F( 
          Form("cl_SmT%01d_Pos", iS),
          Form("Clu position of SmType %d; Sm+Rpc# []; ypos [cm]", iS ),
          fDigiBdfPar->GetNbSm(iS)*fDigiBdfPar->GetNbRpc(iS), 0, fDigiBdfPar->GetNbSm(iS)*fDigiBdfPar->GetNbRpc(iS),
          99, -YDMAX,YDMAX);
 
     Double_t TSumMax=1.E3;
     if (fTRefDifMax !=0.) TSumMax=fTRefDifMax;
     fhSmCluTOff[iS] =  new TH2F( 
          Form("cl_SmT%01d_TOff", iS),
          Form("Clu TimeZero in SmType %d; Sm+Rpc# []; TOff [ps]", iS ),
          fDigiBdfPar->GetNbSm(iS)*fDigiBdfPar->GetNbRpc(iS), 0, fDigiBdfPar->GetNbSm(iS)*fDigiBdfPar->GetNbRpc(iS),
          99, -TSumMax,TSumMax );    

     fhTSmCluPosition[iS].resize( iNSel );
     fhTSmCluTOff[iS].resize( iNSel );
     fhTSmCluTRun[iS].resize( iNSel );
     for (Int_t iSel=0; iSel<iNSel; iSel++){  // Loop over selectors 
       fhTSmCluPosition[iS][iSel] =  new TH2F( 
	  Form("cl_TSmT%01d_Sel%02d_Pos", iS, iSel),
          Form("Clu position of SmType %d under Selector %02d; Sm+Rpc# []; ypos [cm]", iS, iSel ),
          fDigiBdfPar->GetNbSm(iS)*fDigiBdfPar->GetNbRpc(iS), 0, fDigiBdfPar->GetNbSm(iS)*fDigiBdfPar->GetNbRpc(iS),
          99, -YDMAX,YDMAX); 
       fhTSmCluTOff[iS][iSel] =  new TH2F( 
	  Form("cl_TSmT%01d_Sel%02d_TOff", iS, iSel),
          Form("Clu TimeZero in SmType %d under Selector %02d; Sm+Rpc# []; TOff [ps]", iS, iSel ),
          fDigiBdfPar->GetNbSm(iS)*fDigiBdfPar->GetNbRpc(iS), 0, fDigiBdfPar->GetNbSm(iS)*fDigiBdfPar->GetNbRpc(iS),
          99, -TSumMax,TSumMax );    
       fhTSmCluTRun[iS][iSel] =  new TH2F( 
	  Form("cl_TSmT%01d_Sel%02d_TRun", iS, iSel),
          Form("Clu TimeZero in SmType %d under Selector %02d; Event# []; TMean [ps]", iS, iSel ),
          100, 0, MaxNbEvent,
          99, -TSumMax,TSumMax );    
     }
   }

   // RPC related distributions
   Int_t iNbDet=fMbsMappingPar->GetNbMappedDet();
   LOG(INFO)<<" Define Clusterizer histos for "<<iNbDet<<" detectors "<<FairLogger::endl;

   fhRpcDigiCor.resize( iNbDet  );
   fhRpcCluMul.resize( iNbDet  );
   fhRpcCluPosition.resize( iNbDet  );
   fhRpcCluDelPos.resize( iNbDet  );
   fhRpcCluDelMatPos.resize( iNbDet  );
   fhRpcCluTOff.resize( iNbDet  );
   fhRpcCluDelTOff.resize( iNbDet  );
   fhRpcCluDelMatTOff.resize( iNbDet  );
   fhRpcCluTrms.resize( iNbDet  );
   fhRpcCluTot.resize( iNbDet  );
   fhRpcCluSize.resize( iNbDet  );
   fhRpcCluAvWalk.resize( iNbDet );
   fhRpcCluAvLnWalk.resize( iNbDet );
   fhRpcCluWalk.resize( iNbDet );

   for(Int_t iDetIndx=0; iDetIndx<iNbDet; iDetIndx++){
       Int_t iUniqueId = fMbsMappingPar->GetMappedDetUId( iDetIndx );
       Int_t iSmType   = CbmTofAddress::GetSmType( iUniqueId );
       Int_t iSmId     = CbmTofAddress::GetSmId( iUniqueId );
       Int_t iRpcId    = CbmTofAddress::GetRpcId( iUniqueId );
       Int_t iUCellId  = CbmTofAddress::GetUniqueAddress(iSmId,iRpcId,0,0,iSmType);
       fChannelInfo = fDigiPar->GetCell(iUCellId);
       if (NULL==fChannelInfo) {
	 LOG(WARNING)<<"CbmTofTestBeamClusterizer::CreateHistos: No DigiPar for Det "
                     <<Form("0x%08x", iUCellId)
                     <<FairLogger::endl;
	 continue;
       }      
       LOG(INFO) << "CbmTofTestBeamClusterizer::CreateHistos: DetIndx "<<iDetIndx<<", SmType "<<iSmType<<", SmId "<<iSmId
	 	 << ", RpcId "<<iRpcId<<" => UniqueId "<<Form("(0x%08x, 0x%08x)",iUniqueId,iUCellId)
                 <<", dx "<<fChannelInfo->GetSizex()
                 <<", dy "<<fChannelInfo->GetSizey()
                 <<Form(" poi: 0x%p ",fChannelInfo)
	 	 <<", nbCh "<<fMbsMappingPar->GetSmTypeNbCh(iSmType)
                 <<FairLogger::endl;

       fhRpcDigiCor[iDetIndx] =  new TH2I(
          Form("cl_SmT%01d_sm%03d_rpc%03d_DigiCor", iSmType, iSmId, iRpcId ),
          Form("Digi Correlation of Rpc #%03d in Sm %03d of type %d; digi 0; digi 1", iRpcId, iSmId, iSmType ),
          fMbsMappingPar->GetSmTypeNbCh(iSmType),0,fMbsMappingPar->GetSmTypeNbCh(iSmType),
	  fMbsMappingPar->GetSmTypeNbCh(iSmType),0,fMbsMappingPar->GetSmTypeNbCh(iSmType));

       fhRpcCluMul[iDetIndx] =  new TH1I(
          Form("cl_SmT%01d_sm%03d_rpc%03d_Mul", iSmType, iSmId, iRpcId ),
          Form("Clu multiplicity of Rpc #%03d in Sm %03d of type %d; M []; Cnts", iRpcId, iSmId, iSmType ),
          fMbsMappingPar->GetSmTypeNbCh(iSmType),1,fMbsMappingPar->GetSmTypeNbCh(iSmType)+1);

       Double_t YSCAL=50.;
       if (fPosYMaxScal !=0.) YSCAL=fPosYMaxScal;
       Double_t YDMAX=TMath::Max(2.,fChannelInfo->GetSizey())*YSCAL;
       fhRpcCluPosition[iDetIndx] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_Pos", iSmType, iSmId, iRpcId ),
          Form("Clu position of Rpc #%03d in Sm %03d of type %d; Strip []; ypos [cm]", iRpcId, iSmId, iSmType ),
          fMbsMappingPar->GetSmTypeNbCh(iSmType), 0, fMbsMappingPar->GetSmTypeNbCh(iSmType),
          99, -YDMAX,YDMAX); 

       fhRpcCluDelPos[iDetIndx] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_DelPos", iSmType, iSmId, iRpcId ),
          Form("Clu position difference of Rpc #%03d in Sm %03d of type %d; Strip []; #Deltaypos(clu) [cm]", iRpcId, iSmId, iSmType ),
          fMbsMappingPar->GetSmTypeNbCh(iSmType), 0, fMbsMappingPar->GetSmTypeNbCh(iSmType),
          99, -10.,10.); 

       fhRpcCluDelMatPos[iDetIndx] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_DelMatPos", iSmType, iSmId, iRpcId ),
          Form("Matched Clu position difference of Rpc #%03d in Sm %03d of type %d; Strip []; #Deltaypos(mat) [cm]", iRpcId, iSmId, iSmType ),
          fMbsMappingPar->GetSmTypeNbCh(iSmType), 0, fMbsMappingPar->GetSmTypeNbCh(iSmType),
          99, -5.,5.); 

       Double_t TSumMax=1.E4;
       if (fTRefDifMax !=0.) TSumMax=fTRefDifMax;
       fhRpcCluTOff[iDetIndx] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_TOff", iSmType, iSmId, iRpcId ),
          Form("Clu TimeZero of Rpc #%03d in Sm %03d of type %d; Strip []; TOff [ps]", iRpcId, iSmId, iSmType ),
          fMbsMappingPar->GetSmTypeNbCh(iSmType), 0, fMbsMappingPar->GetSmTypeNbCh(iSmType),
          99, -TSumMax,TSumMax ); 

       fhRpcCluDelTOff[iDetIndx] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_DelTOff", iSmType, iSmId, iRpcId ),
          Form("Clu TimeZero Difference of Rpc #%03d in Sm %03d of type %d; Strip []; #DeltaTOff(clu) [ps]", iRpcId, iSmId, iSmType ),
          fMbsMappingPar->GetSmTypeNbCh(iSmType), 0, fMbsMappingPar->GetSmTypeNbCh(iSmType),
          99, -600.,600.); 

       fhRpcCluDelMatTOff[iDetIndx] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_DelMatTOff", iSmType, iSmId, iRpcId ),
          Form("Clu TimeZero Difference of Rpc #%03d in Sm %03d of type %d; Strip []; #DeltaTOff(mat) [ps]", iRpcId, iSmId, iSmType ),
          fMbsMappingPar->GetSmTypeNbCh(iSmType), 0, fMbsMappingPar->GetSmTypeNbCh(iSmType),
          99, -600.,600.); 

       fhRpcCluTrms[iDetIndx] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_Trms", iSmType, iSmId, iRpcId ),
          Form("Clu Time RMS of Rpc #%03d in Sm %03d of type %d; Strip []; Trms [ps]", iRpcId, iSmId, iSmType ),
          fMbsMappingPar->GetSmTypeNbCh(iSmType), 0, fMbsMappingPar->GetSmTypeNbCh(iSmType),
          99, 0., 500. ); 

       if (fTotMax !=0.) fdTOTMax=fTotMax;
       if (fTotMin !=0.) fdTOTMin=fTotMin;
       fhRpcCluTot[iDetIndx] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_Tot", iSmType, iSmId, iRpcId ),
          Form("Clu Tot of Rpc #%03d in Sm %03d of type %d; StripSide []; TOT [ps]", iRpcId, iSmId, iSmType ),
          fMbsMappingPar->GetSmTypeNbCh(iSmType)*2, 0, fMbsMappingPar->GetSmTypeNbCh(iSmType)*2,
          100, fdTOTMin, fdTOTMax);

       fhRpcCluSize[iDetIndx] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_Size", iSmType, iSmId, iRpcId ),
          Form("Clu size of Rpc #%03d in Sm %03d of type %d; Strip []; size [strips]", iRpcId, iSmId, iSmType ),
          fMbsMappingPar->GetSmTypeNbCh(iSmType), 0, fMbsMappingPar->GetSmTypeNbCh(iSmType),
          12, 0., 12.); 

       // Walk histos 
       fhRpcCluAvWalk[iDetIndx] = new TH2F( 
			  Form("cl_SmT%01d_sm%03d_rpc%03d_AvWalk", iSmType, iSmId, iRpcId),
			  Form("Walk in SmT%01d_sm%03d_rpc%03d_AvWalk", iSmType, iSmId, iRpcId),
			  nbClWalkBinX,fdTOTMin,fdTOTMax,nbClWalkBinY,-TSumMax,TSumMax);
   
       fhRpcCluAvLnWalk[iDetIndx] = new TH2F( 
			  Form("cl_SmT%01d_sm%03d_rpc%03d_AvLnWalk", iSmType, iSmId, iRpcId),
			  Form("Walk in SmT%01d_sm%03d_rpc%03d_AvLnWalk", iSmType, iSmId, iRpcId),
			  nbClWalkBinX,6.,TMath::Log(fdTOTMax),nbClWalkBinY,-TSumMax,TSumMax);

       fhRpcCluWalk[iDetIndx].resize( fMbsMappingPar->GetSmTypeNbCh(iSmType) );
       for( Int_t iCh=0; iCh<fMbsMappingPar->GetSmTypeNbCh(iSmType); iCh++){
	 fhRpcCluWalk[iDetIndx][iCh].resize( 2 );
	 for (Int_t iSide=0; iSide<2; iSide++)
	 {
	   fhRpcCluWalk[iDetIndx][iCh][iSide]= new TH2F( 
			  Form("cl_SmT%01d_sm%03d_rpc%03d_Ch%03d_S%01d_Walk", iSmType, iSmId, iRpcId, iCh, iSide ),
			  Form("Walk in SmT%01d_sm%03d_rpc%03d_Ch%03d_S%01d_Walk", iSmType, iSmId, iRpcId, iCh, iSide ),
			  nbClWalkBinX,fdTOTMin,fdTOTMax,nbClWalkBinY,-TSumMax,TSumMax );
	 }
       /*
         (fhRpcCluWalk[iDetIndx]).push_back( hTemp );
       */
       }
   }

   // Clusterizing monitoring 

   LOG(INFO)<<" Define Clusterizer monitoring histos "<<FairLogger::endl;
 
   fhDigSpacDifClust = new TH1I( "Clus_DigSpacDifClust",
         "Space difference along channel direction between Digi pairs on adjacent channels; PosCh i - Pos Ch i+1 [cm]",
         5000, -10.0, 10.0 );
   fhDigTimeDifClust = new TH1I( "Clus_DigTimeDifClust",
         "Time difference between Digi pairs on adjacent channels; 0.5*(tDigiA + tDigiA)chi - 0.5*(tDigiA + tDigiA)chi+1 [ns]",
         5000, -5.0, 5.0 );
   fhDigDistClust = new TH2I( "Clus_DigDistClust",
         "Distance between Digi pairs on adjacent channels; PosCh i - Pos Ch i+1 [cm along ch]; 0.5*(tDigiA + tDigiA)chi - 0.5*(tDigiA + tDigiA)chi+1 [ns]",
         5000, -10.0, 10.0,    2000, -5.0, 5.0 );
   fhClustSizeDifX = new TH2I( "Clus_ClustSizeDifX", "Position difference between Point and Hit as function of Cluster Size; Cluster Size [Strips]; dX [cm]",
         100, 0.5, 100.5,       500,-50,50);
   fhClustSizeDifY = new TH2I( "Clus_ClustSizeDifY", "Position difference between Point and Hit as function of Cluster Size; Cluster Size [Strips]; dY [cm]",
         100, 0.5, 100.5,       500,-50,50);
   fhChDifDifX = new TH2I( "Clus_ChDifDifX", "Position difference between Point and Hit as function of Channel dif; Ch Dif [Strips]; dX [cm]",
         101, -50.5, 50.5,      500,-50,50);
   fhChDifDifY = new TH2I( "Clus_ChDifDifY", "Position difference between Point and Hit as function of Channel Dif; Ch Dif [Strips]; dY [cm]",
         101, -50.5, 50.5,      500,-50,50);
   fhNbSameSide = new TH1I( "Clus_NbSameSide",
         "How many time per event the 2 digis on a channel were of the same side ; Counts/Event []",
         500, 0.0, 500.0 );
   fhNbDigiPerChan = new TH1I( "Clus_NbDigiPerChan",
         "Nb of Digis per channel; Nb Digis []",
         100, 0.0, 100.0 );

   // Trigger selected histograms 
   if (0<iNSel){
   
   fhSeldT.resize( iNSel );
   for (Int_t iSel=0; iSel<iNSel; iSel++){
       fhSeldT[iSel] =  new TH1F(  Form("cl_dt_Sel%02d", iSel ),
				   Form("Selector time %02d; dT [ps]",iSel ),
				   99, -DelTofMax*10., DelTofMax*10.); 
   }
 
   fhTRpcCluMul.resize( iNbDet  );
   fhTRpcCluPosition.resize( iNbDet  );
   fhTRpcCluTOff.resize( iNbDet  );
   fhTRpcCluTot.resize( iNbDet  );
   fhTRpcCluSize.resize( iNbDet  );
   fhTRpcCluAvWalk.resize( iNbDet );
   fhTRpcCluDelTof.resize( iNbDet );
   fhTRpcCludXdY.resize( iNbDet );
   fhTRpcCluWalk.resize( iNbDet );
   for(Int_t iDetIndx=0; iDetIndx<iNbDet; iDetIndx++){
       Int_t iUniqueId = fMbsMappingPar->GetMappedDetUId( iDetIndx );
       Int_t iSmType   = CbmTofAddress::GetSmType( iUniqueId );
       Int_t iSmId     = CbmTofAddress::GetSmId( iUniqueId );
       Int_t iRpcId    = CbmTofAddress::GetRpcId( iUniqueId );
       Int_t iUCellId  = CbmTofAddress::GetUniqueAddress(iSmId,iRpcId,0,0,iSmType);
       fChannelInfo = fDigiPar->GetCell(iUCellId);
       if (NULL==fChannelInfo) {
	 LOG(WARNING)<<"CbmTofTestBeamClusterizer::CreateHistos: No DigiPar for Det "
                     <<Form("0x%08x", iUCellId)
                 <<FairLogger::endl;
	 break;
       }      
       LOG(DEBUG1) << "CbmTofTestBeamClusterizer::CreateHistos: DetIndx "<<iDetIndx<<", SmType "<<iSmType<<", SmId "<<iSmId
	 	 << ", RpcId "<<iRpcId<<" => UniqueId "<<Form("(0x%08x, 0x%08x)",iUniqueId,iUCellId)
                 <<", dx "<<fChannelInfo->GetSizex()
                 <<", dy "<<fChannelInfo->GetSizey()
                 <<Form(" poi: 0x%p ",fChannelInfo)
	 	 <<", nbCh "<<fMbsMappingPar->GetSmTypeNbCh(iSmType)
                 <<FairLogger::endl;
       fhTRpcCluMul[iDetIndx].resize( iNSel  );
       fhTRpcCluPosition[iDetIndx].resize( iNSel  );
       fhTRpcCluTOff[iDetIndx].resize( iNSel  );
       fhTRpcCluTot[iDetIndx].resize( iNSel  );
       fhTRpcCluSize[iDetIndx].resize( iNSel  );
       fhTRpcCluAvWalk[iDetIndx].resize( iNSel  );
       fhTRpcCluDelTof[iDetIndx].resize( iNSel  );
       fhTRpcCludXdY[iDetIndx].resize( iNSel  );
       fhTRpcCluWalk[iDetIndx].resize( iNSel  );

       for (Int_t iSel=0; iSel<iNSel; iSel++){
       fhTRpcCluMul[iDetIndx][iSel] =  new TH1I(
	  Form("cl_SmT%01d_sm%03d_rpc%03d_Sel%02d_Mul", iSmType, iSmId, iRpcId, iSel ),
	  Form("Clu multiplicity of Rpc #%03d in Sm %03d of type %d under Selector %02d; M []; cnts", iRpcId, iSmId, iSmType, iSel ),
          fMbsMappingPar->GetSmTypeNbCh(iSmType),1.,fMbsMappingPar->GetSmTypeNbCh(iSmType)+1);

       Double_t YSCAL=50.;
       if (fPosYMaxScal !=0.) YSCAL=fPosYMaxScal;
       Double_t YDMAX=TMath::Max(2.,fChannelInfo->GetSizey())*YSCAL;
       fhTRpcCluPosition[iDetIndx][iSel] =  new TH2F( 
	  Form("cl_SmT%01d_sm%03d_rpc%03d_Sel%02d_Pos", iSmType, iSmId, iRpcId, iSel ),
          Form("Clu position of Rpc #%03d in Sm %03d of type %d under Selector %02d; Strip []; ypos [cm]", iRpcId, iSmId, iSmType, iSel ),
          fMbsMappingPar->GetSmTypeNbCh(iSmType), 0, fMbsMappingPar->GetSmTypeNbCh(iSmType),
          100, -YDMAX,YDMAX ); 

       Double_t TSumMax=1.E4;
       if (fTRefDifMax !=0.) TSumMax=fTRefDifMax;
       fhTRpcCluTOff[iDetIndx][iSel] =  new TH2F( 
	  Form("cl_SmT%01d_sm%03d_rpc%03d_Sel%02d_TOff", iSmType, iSmId, iRpcId, iSel ),
	  Form("Clu TimeZero of Rpc #%03d in Sm %03d of type %d under Selector %02d; Strip []; TOff [ps]", iRpcId, iSmId, iSmType, iSel ),
          fMbsMappingPar->GetSmTypeNbCh(iSmType), 0, fMbsMappingPar->GetSmTypeNbCh(iSmType),
          99, -TSumMax,TSumMax ); 

       if (fTotMax !=0.) fdTOTMax=fTotMax;
       fhTRpcCluTot[iDetIndx][iSel]  =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_Sel%02d_Tot", iSmType, iSmId, iRpcId, iSel ),
          Form("Clu Tot of Rpc #%03d in Sm %03d of type %d under Selector %02d; StripSide []; TOT [ps]", iRpcId, iSmId, iSmType, iSel ),
          fMbsMappingPar->GetSmTypeNbCh(iSmType)*2, 0, fMbsMappingPar->GetSmTypeNbCh(iSmType)*2,
          100, fdTOTMin, fdTOTMax);

       fhTRpcCluSize[iDetIndx][iSel]  =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_Sel%02d_Size", iSmType, iSmId, iRpcId, iSel ),
          Form("Clu size of Rpc #%03d in Sm %03d of type %d under Selector %02d; Strip []; size [strips]", iRpcId, iSmId, iSmType, iSel ),
          fMbsMappingPar->GetSmTypeNbCh(iSmType), 0, fMbsMappingPar->GetSmTypeNbCh(iSmType),
          12, 0., 12.); 

       // Walk histos 
       fhTRpcCluAvWalk[iDetIndx][iSel]  = new TH2F( 
			  Form("cl_SmT%01d_sm%03d_rpc%03d_Sel%02d_AvWalk", iSmType, iSmId, iRpcId, iSel),
			  Form("Walk in SmT%01d_sm%03d_rpc%03d_Sel%02d_AvWalk; TOT; T-TSel", iSmType, iSmId, iRpcId, iSel),
			  nbClWalkBinX,fdTOTMin,fdTOTMax,nbClWalkBinY,-TSumMax,TSumMax);

       // Tof Histos 
       fhTRpcCluDelTof[iDetIndx][iSel] = new TH2F( 
	  Form("cl_SmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof", iSmType, iSmId, iRpcId, iSel),
	  Form("SmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof; TRef-TSel; T-TSel", iSmType, iSmId, iRpcId, iSel),
	  nbClDelTofBinX,-DelTofMax,DelTofMax,nbClDelTofBinY,-TSumMax,TSumMax);
      
       // Position deviation histos  
       fhTRpcCludXdY[iDetIndx][iSel] = new TH2F( 
	  Form("cl_SmT%01d_sm%03d_rpc%03d_Sel%02d_dXdY", iSmType, iSmId, iRpcId, iSel),
	  Form("SmT%01d_sm%03d_rpc%03d_Sel%02d_dXdY; #Delta x [cm]; #Delta y [cm];", iSmType, iSmId, iRpcId, iSel),
	  nbCldXdYBinX,-dXdYMax,dXdYMax,nbCldXdYBinY,-dXdYMax,dXdYMax);

       fhTRpcCluWalk[iDetIndx][iSel].resize( fMbsMappingPar->GetSmTypeNbCh(iSmType) );
       for( Int_t iCh=0; iCh<fMbsMappingPar->GetSmTypeNbCh(iSmType); iCh++){
	 fhTRpcCluWalk[iDetIndx][iSel][iCh].resize( 2 );
	 for (Int_t iSide=0; iSide<2; iSide++)
	 {
	   fhTRpcCluWalk[iDetIndx][iSel][iCh][iSide]= new TH2F( 
           Form("cl_SmT%01d_sm%03d_rpc%03d_Ch%03d_S%01d_Sel%02d_Walk", iSmType, iSmId, iRpcId, iCh, iSide, iSel ),
	   Form("Walk in SmT%01d_sm%03d_rpc%03d_Ch%03d_S%01d_Sel%02d_Walk", iSmType, iSmId, iRpcId, iCh, iSide, iSel ),
	   nbClWalkBinX,fdTOTMin,fdTOTMax,nbClWalkBinY,-TSumMax,TSumMax);
	 }
       }
      }
     }
   }
   // MC reference 
   fhHitsPerTracks = new TH1I( "Clus_TofHitPerTrk", "Mean Number of TofHit per Mc Track; Nb TofHits/Nb MC Tracks []",
         2000, 0.0, 20.0 );
   if( kFALSE == fDigiBdfPar->ClustUseTrackId() )
      fhPtsPerHit = new TH1I( "Clus_TofPtsPerHit", "Distribution of the Number of MCPoints associated to each TofHit; Nb MCPoint []",
         20, 0.0, 20.0 );
   if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
   {
      fhTimeResSingHits = new TH1I( "Clus_TofTimeResClust", "Time resolution for TofHits containing Digis from a single MC Track; t(1st Mc Point) -tTofHit [ns]",
            10000, -25.0, 25.0 );
      fhTimeResSingHitsB = new TH2I( "Clus_TofTimeResClustB", "Time resolution for TofHits containing Digis from a single MC Track; (1st Mc Point) -tTofHit [ns]",
            5000, -25.0, 25.0,
            6, 0, 6);
      fhTimePtVsHits = new TH2I( "Clus_TofTimePtVsHit", "Time resolution for TofHits containing Digis from a single MC Track; t(1st Mc Point) -tTofHit [ps]",
            2000, 0.0, 50000.0,
            2000, 0.0, 50000.0 );
   }
      else
      {
         fhTimeResSingHits = new TH1I( "Clus_TofTimeResClust", "Time resolution for TofHits containing Digis from a single TofPoint; tMcPoint -tTofHit [ns]",
               10000, -25.0, 25.0 );
         fhTimeResSingHitsB = new TH2I( "Clus_TofTimeResClustB", "Time resolution for TofHits containing Digis from a single TofPoint; tMcPoint -tTofHit [ns]",
               5000, -25.0, 25.0,
               6, 0, 6);
         fhTimePtVsHits = new TH2I( "Clus_TofTimePtVsHit", "Time resolution for TofHits containing Digis from a single TofPoint; tMcPoint -tTofHit [ps]",
               2000, 0.0, 50000.0,
               2000, 0.0, 50000.0 );
      } // else of if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
   fhClusterSize = new TH1I( "Clus_ClusterSize", "Cluster Size distribution; Cluster Size [Strips]",
         100, 0.5, 100.5 );
   fhClusterSizeType = new TH2I( "Clus_ClusterSizeType", "Cluster Size distribution in each (SM type, Rpc) pair; Cluster Size [Strips]; 10*SM Type + Rpc Index []",
         100, 0.5, 100.5,
         40*fDigiBdfPar->GetNbSmTypes(), 0.0, 40*fDigiBdfPar->GetNbSmTypes() );
   if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
   {
      fhTrackMul = new TH1I( "Clus_TrackMul", "Number of MC tracks generating the cluster; MC Tracks multiplicity []",
            100, 0.5, 100.5 );
      fhClusterSizeMulti = new TH2I( "Clus_ClusterSizeMulti",
            "Cluster Size distribution as function of Number of MC tracks generating the cluster; Cluster Size [Strips]; MC tracks mul. []",
            100, 0.5, 100.5,
            100, 0.5, 100.5 );
      fhTrk1MulPos = new TH2D( "Clus_Trk1MulPos",
            "Position of Clusters with only 1 MC tracks generating the cluster; X [cm]; Y [cm]",
            1500, -750, 750,
            1000, -500, 500 );
      fhHiTrkMulPos = new TH2D( "Clus_HiTrkMulPos",
            "Position of Clusters with >1 MC tracks generating the cluster; X [cm]; Y [cm]",
            1500, -750, 750,
            1000, -500, 500 );
      fhAllTrkMulPos = new TH2D( "Clus_AllTrkMulPos",
            "Position of all clusters generating the cluster; X [cm]; Y [cm]",
            1500, -750, 750,
            1000, -500, 500 );
      fhMultiTrkProbPos = new TH2D( "Clus_MultiTrkProbPos",
            "Probability of having a cluster with multiple tracks as function of position; X [cm]; Y [cm]; Prob. [%]",
            1500, -750, 750,
            1000, -500, 500 );
   } // if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
   
   gDirectory->cd( oldir->GetPath() ); // <= To prevent histos from being sucked in by the param file of the TRootManager!

   return kTRUE;
}
Bool_t   CbmTofTestBeamClusterizer::FillHistos()
{
 fhClustBuildTime->Fill( fStop.GetSec() - fStart.GetSec()
                           + (fStop.GetNanoSec() - fStart.GetNanoSec())/1e9 );
 Int_t iNbTofHits  = fTofHitsColl->GetEntries();
 CbmTofHit  *pHit;

 if(0<iNbTofHits){
  Double_t    dCluMul=0.;
  Bool_t      BSel[iNSel];
  Double_t    dTTrig[iNSel];
  CbmTofHit  *pTrig[iNSel]; 
  if(0<iNSel){                  // check software triggers
    LOG(DEBUG) <<"CbmTofTestBeamClusterizer::FillHistos() for "<<iNSel<<" triggers, "
	       <<" Muls "<<fviClusterMul[0][0][0]
	       <<", "<<fviClusterMul[1][0][0]
      /*
	       <<", "<<fviClusterMul[2][0][0]
	       <<", "<<fviClusterMul[2][1][0]
	       <<", "<<fviClusterMul[2][2][0]
	       <<", "<<fviClusterMul[2][3][0]
      */
	       <<", "<<fviClusterMul[3][0][0]
	       <<", "<<fviClusterMul[4][0][0]
	       <<", "<<fviClusterMul[5][0][0]
	       <<", "<<fviClusterMul[5][1][0]
	       <<", "<<fviClusterMul[5][2][0]
	       <<", "<<fviClusterMul[6][0][0]
	       <<", Dut "<<fDutId<<", Sel "<<fSelId
               <<FairLogger::endl;

   // do reference first 
   dTRef=dDoubleMax;
   Int_t iBeamAddRefMul=0;
   for( Int_t iHitInd = 0; iHitInd < iNbTofHits; iHitInd++)
   {
      pHit = (CbmTofHit*) fTofHitsColl->At( iHitInd );
      if (NULL==pHit) continue;
      Int_t iDetId = (pHit->GetAddress() & DetMask);

      if( fiBeamRefType == CbmTofAddress::GetSmType( iDetId )){
       if(fiBeamRefSm   ==  CbmTofAddress::GetSmId( iDetId ))
       {
	  fTRefHits=1;
            if(pHit->GetTime() < dTRef)
	    {
	      dTRef = pHit->GetTime();
	    }
       }else{ //additional reference type multiplicity
	 iBeamAddRefMul++;
       }
      }
   }

   for (Int_t iSel=0; iSel<iNSel; iSel++){
       BSel[iSel]=kFALSE;
       pTrig[iSel]=NULL;
       Int_t    iNbRpc  = fDigiBdfPar->GetNbRpc( fDutId );

       switch(iSel) {
       case 0 :         //  Detector under Test (Dut) && Diamonds,BeamRef
	 if(   fviClusterMul[fDutId][0][0]>0.
	    && fviClusterMul[fiBeamRefType][fiBeamRefSm][fiBeamRefDet]>0   // request beam reference counter 
	    && iBeamAddRefMul>fiBeamAddRefMul) {                           // ask for confirmation by other beam counters 
	     LOG(DEBUG1)<<"CbmTofTestBeamClusterizer::FillHistos(): Found selector 0"<<FairLogger::endl;
	     dTTrig[iSel]=dDoubleMax;
	     for( Int_t iHitInd = 0; iHitInd < iNbTofHits; iHitInd++)
	     {
	      pHit = (CbmTofHit*) fTofHitsColl->At( iHitInd );
              if(NULL == pHit) continue;

	      Int_t iDetId = (pHit->GetAddress() & DetMask);

	      if( fDutId == CbmTofAddress::GetSmType( iDetId ))
	      {
                 if(pHit->GetTime() < dTTrig[iSel])
		 {
		  dTTrig[iSel] = pHit->GetTime();
		  pTrig[iSel]  = pHit;
		  BSel[iSel]=kTRUE;
		 }	 
	      }
	     }
	 } 
	 break; 
       case 1 :         // MRef & BRef 
	 if(fviClusterMul[fSelId][0][0]>0 
	    && fviClusterMul[fiBeamRefType][fiBeamRefSm][fiBeamRefDet]>0   // request beam reference counter 
	    && iBeamAddRefMul>fiBeamAddRefMul) {                           // ask for confirmation by other beam counters 
	     LOG(DEBUG1)<<"CbmTofTestBeamClusterizer::FillHistos(): Found selector 1"<<FairLogger::endl;
	     dTTrig[iSel]=dDoubleMax;
	     for( Int_t iHitInd = 0; iHitInd < iNbTofHits; iHitInd++)
	     {
	      pHit = (CbmTofHit*) fTofHitsColl->At( iHitInd );
	      if(NULL == pHit) continue;

	      Int_t iDetId = (pHit->GetAddress() & DetMask);
	      if( fSelId == CbmTofAddress::GetSmType( iDetId ))
	      {
		if(0 == CbmTofAddress::GetSmId( iDetId )) 
		{
		  if(pHit->GetTime() < dTTrig[iSel])
		  {
		    dTTrig[iSel] = pHit->GetTime();
		    pTrig[iSel]  = pHit;
		    BSel[iSel]=kTRUE;
		  }
		}
	      }
	     }
	 }
	 break;

       default :
       LOG(INFO)<<"CbmTofTestBeamClusterizer::FillHistos: selection not implemented "<<iSel<<FairLogger::endl;
	 ;
       }  // switch end 
       if(fTRefMode>10){
        dTTrig[iSel]=dTRef;
       }
    } // iSel - loop end 

    if (fSel2Id > 0 ) {  // confirm selector by independent match 
      for (Int_t iSel=0; iSel<iNSel; iSel++){
	if (BSel[iSel]){
	  BSel[iSel]=kFALSE;

	  for( Int_t iHitInd = 0; iHitInd < iNbTofHits; iHitInd++)
	  {
	     pHit = (CbmTofHit*) fTofHitsColl->At( iHitInd );
             if(NULL == pHit) continue;

	     Int_t iDetId = (pHit->GetAddress() & DetMask);

	     if( fSel2Id == CbmTofAddress::GetSmType( iDetId ))
	     {
	       if (TMath::Sqrt(TMath::Power(Zref/pHit->GetZ()*pHit->GetX()-Zref/pTrig[iSel]->GetZ()*pTrig[iSel]->GetX(),2.)
			      +TMath::Power(Zref/pHit->GetZ()*pHit->GetY()-Zref/pTrig[iSel]->GetZ()*pTrig[iSel]->GetY(),2.))<fdCaldXdYMax)
		 {
		   BSel[iSel]=kTRUE;
		   break;
		 }
	     }
	  }
	} // BSel condition end 
      } // iSel lopp end 
    } // Sel2Id condition end
    /*
    LOG(INFO)<<"CbmTofTestBeamClusterizer: dtTrig "<<fTRefMode<<" - "
	     <<dTRef<<", "<<dTTrig[0]<<", "<<dTTrig[1]<<", "<<dTTrig[2]<<", "<<dTTrig[3]
	     <<FairLogger::endl;
    */
    for (Int_t iSel=0; iSel<iNSel; iSel++){
      if(BSel[iSel]){
	if (dTRef!=0. && fTRefHits>0) fhSeldT[iSel]->Fill(dTTrig[iSel]-dTRef);    
      }
    }
  } // 0<iNSel end 
 
   for( Int_t iHitInd = 0; iHitInd < iNbTofHits; iHitInd++)
   {
     pHit = (CbmTofHit*) fTofHitsColl->At( iHitInd );
     if(NULL == pHit) continue;

     if( kFALSE == fDigiBdfPar->ClustUseTrackId() ) fhPtsPerHit->Fill(pHit->GetFlag());
     Int_t iDetId = (pHit->GetAddress() & DetMask);

     for(Int_t iDetIndx=0; iDetIndx<fMbsMappingPar->GetNbMappedDet(); iDetIndx++){
       Int_t iUniqueId = fMbsMappingPar->GetMappedDetUId( iDetIndx );

       LOG(DEBUG1)<<"CbmTofTestBeamClusterizer::FillHistos: Inspect Hit  "
                  << Form(" %d %08x %08x %d %08x ", iHitInd, pHit->GetAddress(), DetMask, iDetIndx, iUniqueId)
		  <<FairLogger::endl;
       if(iDetId == iUniqueId){    // detector index found
	 Int_t iSmType = CbmTofAddress::GetSmType( iUniqueId );
	 Int_t iSm     = CbmTofAddress::GetSmId( iUniqueId );
	 Int_t iRpc    = CbmTofAddress::GetRpcId( iUniqueId );
         Int_t iNbRpc  = fDigiBdfPar->GetNbRpc( iSmType );
         if(-1<fviClusterMul[iSmType][iSm][iRpc]){           // fill only once per detector 
	   fhRpcCluMul[iDetIndx]->Fill(fviClusterMul[iSmType][iSm][iRpc]);
	   for (Int_t iSel=0; iSel<iNSel; iSel++) if(BSel[iSel]){
	       fhTRpcCluMul[iDetIndx][iSel]->Fill(fviClusterMul[iSmType][iSm][iRpc]);
	   }	 
	   //	   fviClusterMul[iSmType][iSm][iRpc]=-1;  // invalidate
	 }

	 if(fviClusterMul[iSmType][iSm][iRpc] > fiCluMulMax) break; // skip this event 

         Int_t iChId = pHit->GetAddress();
         fChannelInfo = fDigiPar->GetCell( iChId );
	 Int_t iCh = CbmTofAddress::GetChannelId(iChId);
	 if(NULL == fChannelInfo){
	   LOG(ERROR) << "CbmTofTestBeamClusterizer::FillHistos: Invalid Channel Pointer for ChId "
		      << Form(" 0x%08x ",iChId)<<", Ch "<<iCh
		      <<FairLogger::endl;
	   continue;
	 }
	 TGeoNode *fNode=        // prepare global->local trafo
	 gGeoManager->FindNode(fChannelInfo->GetX(),fChannelInfo->GetY(),fChannelInfo->GetZ());

         LOG(DEBUG1)<<"CbmTofTestBeamClusterizer::FillHistos: Hit  "
		    <<Form(" 0x%08x %d %f %f %f %f %f %d",iChId,iCh,
			   pHit->GetX(),pHit->GetY(),pHit->GetTime(),fChannelInfo->GetX(),fChannelInfo->GetY(), pHit->GetRefId() )
		    <<FairLogger::endl;

	 Double_t hitpos[3];
	 hitpos[0]=pHit->GetX();
	 hitpos[1]=pHit->GetY();
	 hitpos[2]=pHit->GetZ();
         Double_t hitpos_local[3];
	 TGeoNode* cNode= gGeoManager->GetCurrentNode();
	 gGeoManager->MasterToLocal(hitpos, hitpos_local);
	 LOG(DEBUG1)<<
	    Form(" MasterToLocal for node 0x%p: (%6.1f,%6.1f,%6.1f) ->(%6.1f,%6.1f,%6.1f)", 
		 cNode, hitpos[0], hitpos[1], hitpos[2], 
		 hitpos_local[0], hitpos_local[1], hitpos_local[2])
		   <<FairLogger::endl;

         fhRpcCluPosition[iDetIndx]->Fill((Double_t)iCh,hitpos_local[1]); //pHit->GetY()-fChannelInfo->GetY());
	 fhSmCluPosition[iSmType]->Fill((Double_t)(iSm*iNbRpc+iRpc),hitpos_local[1]);

	 for (Int_t iSel=0; iSel<iNSel; iSel++) if(BSel[iSel]){
	     fhTRpcCluPosition[iDetIndx][iSel]->Fill((Double_t)iCh,hitpos_local[1]);  //pHit->GetY()-fChannelInfo->GetY());
	     fhTSmCluPosition[iSmType][iSel]->Fill((Double_t)(iSm*iNbRpc+iRpc),hitpos_local[1]);
	 }

	 if(TMath::Abs(hitpos_local[1])>fChannelInfo->GetSizey()*fPosYMaxScal) continue;

	 LOG(DEBUG1)<<"CbmTofTestBeamClusterizer::FillHistos: TofDigiMatchColl entries:"
		    <<fTofDigiMatchColl->GetEntries()
		    <<FairLogger::endl;

	 if(pHit->GetRefId()>fTofDigiMatchColl->GetEntries()){
	   LOG(ERROR)<<"CbmTofTestBeamClusterizer::FillHistos: Inconsistent DigiMatches for Hit "
		     <<iHitInd<<" Ref: "<<pHit->GetRefId()
		     <<", TClonesArraySize: "<<fTofDigiMatchColl->GetEntries()
		     <<FairLogger::endl;
	 }

         CbmMatch* digiMatch=(CbmMatch *)fTofDigiMatchColl->At(pHit->GetRefId());
         LOG(DEBUG1)<<"CbmTofTestBeamClusterizer::FillHistos: got matches: "
		    <<digiMatch->GetNofLinks()<<":";
	 fhRpcCluSize[iDetIndx]->Fill((Double_t)iCh,digiMatch->GetNofLinks()/2.);

	 for (Int_t iSel=0; iSel<iNSel; iSel++) if(BSel[iSel]){
	   fhTRpcCluSize[iDetIndx][iSel]->Fill((Double_t)iCh,digiMatch->GetNofLinks()/2.);
	 }

	 Double_t TotSum=0.;
         for (Int_t iLink=0; iLink<digiMatch->GetNofLinks(); iLink++){  // loop over digis
           CbmLink L0 = digiMatch->GetLink(iLink);   //vDigish.at(ivDigInd);
	   Int_t iDigInd0=L0.GetIndex(); 
           if (iDigInd0 < fTofDigisColl->GetEntries()){
            CbmTofDigiExp *pDig0 = (CbmTofDigiExp*) (fTofDigisColl->At(iDigInd0));
            TotSum += pDig0->GetTot();
	   }
	 }


	 Double_t dMeanTimeSquared=0.;
	 Double_t dNstrips=0.;
	 
	 Double_t dDelTof=0.;
	 Double_t dTcor=0.;
	 Double_t dTTcor=0.;
 		
         for (Int_t iLink=0; iLink<digiMatch->GetNofLinks(); iLink+=2){  // loop over digis
           CbmLink L0 = digiMatch->GetLink(iLink);   //vDigish.at(ivDigInd);
	   Int_t iDigInd0=L0.GetIndex(); 
           Int_t iDigInd1=(digiMatch->GetLink(iLink+1)).GetIndex(); //vDigish.at(ivDigInd+1);
           LOG(DEBUG1)<<" " << iDigInd0<<", "<<iDigInd1<<FairLogger::endl;

           if (iDigInd0 < fTofDigisColl->GetEntries() && iDigInd1 < fTofDigisColl->GetEntries()){
            CbmTofDigiExp *pDig0 = (CbmTofDigiExp*) (fTofDigisColl->At(iDigInd0));
            CbmTofDigiExp *pDig1 = (CbmTofDigiExp*) (fTofDigisColl->At(iDigInd1));
            if((Int_t)pDig0->GetType()!=iSmType){
		LOG(ERROR)<<Form(" Wrong Digi SmType for Tofhit %d in iDetIndx %d, Ch %d with %3.0f strips at Indx %d, %d",
				 iHitInd,iDetIndx,iCh,dNstrips,iDigInd0,iDigInd1)
			  << FairLogger::endl; 
	    }
	    LOG(DEBUG2)<<" Digi 0 "<<iDigInd0<<": Ch "<<pDig0->GetChannel()<<", Side "<<pDig0->GetSide()
		     <<" Digi 1 "<<iDigInd1<<": Ch "<<pDig1->GetChannel()<<", Side "<<pDig1->GetSide()<<FairLogger::endl;

	    fhRpcCluTot[iDetIndx]->Fill((Double_t)iCh*2+pDig0->GetSide(),pDig0->GetTot());
	    fhRpcCluTot[iDetIndx]->Fill((Double_t)iCh*2+pDig1->GetSide(),pDig1->GetTot());

            if (digiMatch->GetNofLinks()>2 && digiMatch->GetNofLinks()<8 ) // FIXME: hardwired limits on CluSize
	    {
	      dNstrips+=1.;
              dMeanTimeSquared += TMath::Power(0.5*(pDig0->GetTime()+pDig1->GetTime())-pHit->GetTime(),2);
	      Int_t iCh0=pDig0->GetChannel();
	      Int_t iCh1=pDig1->GetChannel();
	      Int_t iS0=pDig0->GetSide();
	      Int_t iS1=pDig1->GetSide();
	      if(iCh0 != iCh1 || iS0==iS1){
    	       LOG(ERROR)<<Form(" MT2 for Tofhit %d in iDetIndx %d, Ch %d from %3.0f strips: ",iHitInd,iDetIndx,iCh,dNstrips)
		         <<Form(" Dig0: Ch %d, Side %d, T: %6.1f ",iCh0,iS0,pDig0->GetTime())
	 	         <<Form(" Dig1: Ch %d, Side %d, T: %6.1f ",iCh1,iS1,pDig1->GetTime())
		         << FairLogger::endl; 
	       continue;
	      }
 
	      if(0>iCh0 ||  fDigiBdfPar->GetNbChan( iSmType, iRpc )<=iCh0) {
		LOG(ERROR)<<Form(" Wrong Digi for Tofhit %d in iDetIndx %d, Ch %d at Indx %d, %d from %3.0f strips:  %d, %d, %d, %d",
				 iHitInd,iDetIndx,iCh,iDigInd0,iDigInd1,dNstrips,iCh0,iCh1,iS0,iS1)
			  << FairLogger::endl; 
		continue;
	      }


	      fhRpcCluAvWalk[iDetIndx]->Fill(0.5*(pDig0->GetTot()+pDig1->GetTot()),
					0.5*(pDig0->GetTime()+pDig1->GetTime())-pHit->GetTime());
	      fhRpcCluAvLnWalk[iDetIndx]->Fill(TMath::Log(0.5*(pDig0->GetTot()+pDig1->GetTot())),
					0.5*(pDig0->GetTime()+pDig1->GetTime())-pHit->GetTime());

	      Double_t dTotWeigth=(pDig0->GetTot()+pDig1->GetTot())/TotSum;
	      Double_t dCorWeigth=1.-dTotWeigth;

	      fhRpcCluDelTOff[iDetIndx]->Fill(pDig0->GetChannel(),
					      dCorWeigth*(0.5*(pDig0->GetTime()+pDig1->GetTime())-pHit->GetTime()));

	      Double_t dDelPos=0.5*(pDig0->GetTime()-pDig1->GetTime())*fDigiBdfPar->GetSigVel(iSmType,iRpc);
	      if(0==pDig0->GetSide()) dDelPos *= -1.;
	      fhRpcCluDelPos[iDetIndx]->Fill(pDig0->GetChannel(),dCorWeigth*(dDelPos-hitpos_local[1]));

	      fhRpcCluWalk[iDetIndx][iCh0][iS0]->Fill(pDig0->GetTot(),
	      pDig0->GetTime()-(pHit->GetTime()-(1.-2.*pDig0->GetSide())*hitpos_local[1]/fDigiBdfPar->GetSigVel(iSmType,iRpc)));
	      
	      fhRpcCluWalk[iDetIndx][iCh1][iS1]->Fill(pDig1->GetTot(),
	      pDig1->GetTime()-(pHit->GetTime()-(1.-2.*pDig1->GetSide())*hitpos_local[1]/fDigiBdfPar->GetSigVel(iSmType,iRpc)));

	    }

	    for (Int_t iSel=0; iSel<iNSel; iSel++) if(BSel[iSel]){
		fhTRpcCluTot[iDetIndx][iSel]->Fill((Double_t)iCh*2+pDig0->GetSide(),pDig0->GetTot());
		fhTRpcCluTot[iDetIndx][iSel]->Fill((Double_t)iCh*2+pDig1->GetSide(),pDig1->GetTot());
	    }

	    for (Int_t iSel=0; iSel<iNSel; iSel++) if(BSel[iSel]){  
	     if (NULL == pHit  || NULL == pTrig[iSel]) {
	       LOG(INFO)<<"CbmTofTestBeamClusterizer::FillHistos: invalid pHit, iSel "
			 <<iSel<<", iDetIndx "<< iDetIndx
			 <<FairLogger::endl;
	       break;
	     }

	     if(iLink==0) {      // Fill histo only once (for 1. digi entry)
	       fhTRpcCludXdY[iDetIndx][iSel]->Fill(
			 Zref/pHit->GetZ()*pHit->GetX()-Zref/pTrig[iSel]->GetZ()*pTrig[iSel]->GetX(),
			 Zref/pHit->GetZ()*pHit->GetY()-Zref/pTrig[iSel]->GetZ()*pTrig[iSel]->GetY());
	     }

	     if(iSmType==5       // to get entries in diamond histos  
             || TMath::Sqrt(TMath::Power(Zref/pHit->GetZ()*pHit->GetX()-Zref/pTrig[iSel]->GetZ()*pTrig[iSel]->GetX(),2.)
	       +TMath::Power(Zref/pHit->GetZ()*pHit->GetY()-Zref/pTrig[iSel]->GetZ()*pTrig[iSel]->GetY(),2.))<fdCaldXdYMax)
	     {
	      if (dTRef !=0.  && TMath::Abs(dTRef-dTTrig[iSel])<DelTofMax) {  // correct times for DelTof - velocity spread 

		if(iLink==0){   // do calculations only once (at 1. digi entry)
		 Int_t iBx = (dTRef-dTTrig[iSel]+DelTofMax)/2./DelTofMax*nbClDelTofBinX;
		 if(iBx<0) iBx=0;
		 if(iBx>nbClDelTofBinX-1) iBx=nbClDelTofBinX-1;
		 dDelTof=fvCPDelTof[iSmType][iSm*iNbRpc+iRpc][iBx][iSel];
		 LOG(DEBUG1)<<Form(" DelTof for SmT %d, Sm %d, R %d, T %d, dTRef %f, iBx %d -> DelT %f",
			       iSmType, iSm, iRpc, iSel, dTRef-dTTrig[iSel], iBx, dDelTof)
			    <<FairLogger::endl;
		 dTcor=pHit->GetTime()-dDelTof-dTTrig[iSel];  // corrected CbmTofHit time
		 if(fCalSel == iSel) dTTcor=dDelTof;
	       }

	       Double_t dDt=0.5*(pDig0->GetTime()+pDig1->GetTime())-dDelTof-dTTrig[iSel];
	       Double_t dAvTot=0.5*(pDig0->GetTot()+pDig1->GetTot());
	       /*
	       fhTRpcCluWalk[iDetIndx][iSel][pDig0->GetChannel()][pDig0->GetSide()]->Fill(dAvTot,dDt);
	       fhTRpcCluWalk[iDetIndx][iSel][pDig1->GetChannel()][pDig1->GetSide()]->Fill(dAvTot,dDt);
	       */
	       
	       fhTRpcCluWalk[iDetIndx][iSel][pDig0->GetChannel()][pDig0->GetSide()]->Fill(pDig0->GetTot(),pDig0->GetTime()
		+((1.-2.*pDig0->GetSide())*hitpos_local[1]/fDigiBdfPar->GetSigVel(iSmType,iRpc))-dDelTof-dTTrig[iSel]);
	       fhTRpcCluWalk[iDetIndx][iSel][pDig1->GetChannel()][pDig1->GetSide()]->Fill(pDig1->GetTot(),pDig1->GetTime()
		+((1.-2.*pDig1->GetSide())*hitpos_local[1]/fDigiBdfPar->GetSigVel(iSmType,iRpc))-dDelTof-dTTrig[iSel]);
	       
	       if(iLink==0){          // Fill histo only once (for 1. digi entry)
		 fhTRpcCluAvWalk[iDetIndx][iSel]->Fill(dAvTot,dTcor);
		 fhTRpcCluDelTof[iDetIndx][iSel]->Fill(dTRef-dTTrig[iSel],dTcor);
		 fhTSmCluTOff[iSmType][iSel]->Fill((Double_t)(iSm*iNbRpc+iRpc),dTcor);
		 fhTSmCluTRun[iSmType][iSel]->Fill(fdEvent,dTcor);
		 if( iDetId != (pTrig[iSel]->GetAddress() & DetMask) ){ // transform matched hit-pair back into detector frame
		   hitpos[0]=pHit->GetX()-pHit->GetZ()/pTrig[iSel]->GetZ()*pTrig[iSel]->GetX() + fChannelInfo->GetX();
		   hitpos[1]=pHit->GetY()-pHit->GetZ()/pTrig[iSel]->GetZ()*pTrig[iSel]->GetY() + fChannelInfo->GetY();
                   hitpos[2]=pHit->GetZ();
		   gGeoManager->MasterToLocal(hitpos, hitpos_local); //  transform into local frame
	           fhRpcCluDelMatPos[iDetIndx]->Fill(pDig0->GetChannel(),hitpos_local[1]);
		   fhRpcCluDelMatTOff[iDetIndx]->Fill(pDig0->GetChannel(),dTcor);
		   /*
		   if(iSmType == 4 || iSmType ==3)
		   LOG(INFO)<<" Match for T "<<iSmType
		            <<" M "<< iSm
			    <<" R "<<iRpc
			    <<" dTcor "<< dTcor
			    <<" pos: "<<hitpos[0]<<", "<<hitpos[1]<<", "<<hitpos[2]
			    <<" loc: "<<hitpos_local[0]<<", "<<hitpos_local[1]<<", "<<hitpos_local[2]
                            <<FairLogger::endl;
		   */
		 }
	       }    // iLink==0 condition end 
	      }
	     } // Match condition end
	    }  // closing of trigger loop
	   } 
	   else {
	     LOG(ERROR)<<"CbmTofTestBeamClusterizer::FillHistos: invalid digi index "<<iDetIndx
		       <<" digi0,1"<<iDigInd0<<", "<<iDigInd1<<" - max:"
		       << fTofDigisColl->GetEntries() 
	       //                       << " in event " << XXX 
		       << FairLogger::endl;  
           }
	 } // digi loop end;
	 if (2<dNstrips){
	   //	   Double_t dVar=dMeanTimeSquared/dNstrips - TMath::Power(pHit->GetTime(),2);
	   Double_t dVar=dMeanTimeSquared/(dNstrips-1);
	   if(dVar<0.) dVar=0.;
	   Double_t dTrms=TMath::Sqrt(dVar);
	   LOG(DEBUG)<<Form(" Trms for Tofhit %d in iDetIndx %d, Ch %d from %3.0f strips: %6.1f ps",
			     iHitInd,iDetIndx,iCh,dNstrips,dTrms)
		    << FairLogger::endl;  
	   fhRpcCluTrms[iDetIndx]->Fill((Double_t)iCh,dTrms);

	 }

	 LOG(DEBUG1)<<" Fill Time of iDetIndx "<<iDetIndx<<" for |y| <"
                  <<fhRpcCluPosition[iDetIndx]->GetYaxis()->GetXmax()
		  <<FairLogger::endl;
	 if(TMath::Abs(hitpos_local[1])< (fhRpcCluPosition[iDetIndx]->GetYaxis()->GetXmax())){
	   if (dTRef !=0. && fTRefHits==1){ 
	     fhRpcCluTOff[iDetIndx]->Fill((Double_t)iCh,pHit->GetTime()-dTTcor-dTRef);
	     fhSmCluTOff[iSmType]->Fill((Double_t)(iSm*iNbRpc+iRpc),pHit->GetTime()-dTTcor-dTRef);
	     /*
	     if(iUniqueId == fTRefMode && (pHit->GetTime()-dTRef) !=0.){
	     LOG(WARNING)<<Form(" Inconsistent dTRef %f for detetor 0x%08x with %d hits ",dTRef,iUniqueId,fTRefHits)
			 <<FairLogger::endl;
	     }
	     */
	     for (Int_t iSel=0; iSel<iNSel; iSel++) if(BSel[iSel]){
	       LOG(DEBUG1)<<"CbmTofTestBeamClusterizer::FillHistos: TRpcCluTOff "<< iDetIndx <<", "<< iSel
		        <<FairLogger::endl;
	       fhTRpcCluTOff[iDetIndx][iSel]->Fill((Double_t)iCh,pHit->GetTime()-dTTcor-dTTrig[iSel]);
	     }
	   }
	 }
	 break;  // iDetId found       
       }
     }
   }

   for( Int_t iSmType = 0; iSmType < fDigiBdfPar->GetNbSmTypes(); iSmType++ ){
      for( Int_t iRpc = 0; iRpc < fDigiBdfPar->GetNbRpc( iSmType); iRpc++ )
      {
         LOG(DEBUG1)<<"CbmTofTestBeamClusterizer::FillHistos:  "
                    <<Form(" %3d %3d %3lu ",iSmType,iRpc,fviClusterSize[iSmType][iRpc].size())
                    <<FairLogger::endl;
    
	 for( Int_t iCluster = 0; iCluster < fviClusterSize[iSmType][iRpc].size(); iCluster++ )
         {
            LOG(DEBUG1)<<"CbmTofTestBeamClusterizer::FillHistos:  "
                       <<Form(" %3d %3d %3d ",iSmType,iRpc,iCluster)
                       <<FairLogger::endl;

            fhClusterSize->Fill( fviClusterSize[iSmType][iRpc][iCluster]);
            fhClusterSizeType->Fill( fviClusterSize[iSmType][iRpc][iCluster], 40*iSmType + iRpc );
            if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
            {
               fhTrackMul->Fill( fviTrkMul[iSmType][iRpc][iCluster] );
               fhClusterSizeMulti->Fill( fviClusterSize[iSmType][iRpc][iCluster],
                                         fviTrkMul[iSmType][iRpc][iCluster] );
               if( 1 == fviTrkMul[iSmType][iRpc][iCluster] )
                  fhTrk1MulPos->Fill( fvdX[iSmType][iRpc][iCluster], fvdY[iSmType][iRpc][iCluster] );
               if( 1 < fviTrkMul[iSmType][iRpc][iCluster] )
                  fhHiTrkMulPos->Fill(fvdX[iSmType][iRpc][iCluster], fvdY[iSmType][iRpc][iCluster] );
               fhAllTrkMulPos->Fill(fvdX[iSmType][iRpc][iCluster], fvdY[iSmType][iRpc][iCluster] );
            } // if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
            if(kFALSE) //  1 == fviTrkMul[iSmType][iRpc][iCluster] )
            {
               fhClustSizeDifX->Fill( fviClusterSize[iSmType][iRpc][iCluster], fvdDifX[iSmType][iRpc][iCluster]);
               fhClustSizeDifY->Fill( fviClusterSize[iSmType][iRpc][iCluster], fvdDifY[iSmType][iRpc][iCluster]);
               if( 1 == fviClusterSize[iSmType][iRpc][iCluster] )
               {
                  fhChDifDifX->Fill( fvdDifCh[iSmType][iRpc][iCluster], fvdDifX[iSmType][iRpc][iCluster]);
                  fhChDifDifY->Fill( fvdDifCh[iSmType][iRpc][iCluster], fvdDifY[iSmType][iRpc][iCluster]);
               }
            }
         } // for( Int_t iCluster = 0; iCluster < fviClusterSize[iSmType][iRpc].size(); iCluster++ )
         fviClusterSize[iSmType][iRpc].clear();
         fviTrkMul[iSmType][iRpc].clear();
         fvdX[iSmType][iRpc].clear();
         fvdY[iSmType][iRpc].clear();
         fvdDifX[iSmType][iRpc].clear();
         fvdDifY[iSmType][iRpc].clear();
         fvdDifCh[iSmType][iRpc].clear();
      } // for( Int_t iRpc = 0; iRpc < fDigiBdfPar->GetNbRpc( iSmType); iRpc++ )
   }
   fhNbSameSide->Fill(fiNbSameSide);
  }

  return kTRUE;
}

Bool_t   CbmTofTestBeamClusterizer::WriteHistos()
{
   TDirectory * oldir = gDirectory;
   TFile *fHist;
   fHist = new TFile(fOutHstFileName,"RECREATE");

   fHist->cd();
   fhClustBuildTime->Write();

   for (Int_t iS=0; iS< fDigiBdfPar->GetNbSmTypes(); iS++){
     if (NULL == fhSmCluPosition[iS]) continue; 
     fhSmCluPosition[iS]->Write();
     fhSmCluTOff[iS]->Write();
     for (Int_t iSel=0; iSel<iNSel; iSel++){  // Loop over selectors 
       fhTSmCluPosition[iS][iSel]->Write();
       fhTSmCluTOff[iS][iSel]->Write();
       fhTSmCluTRun[iS][iSel]->Write();
     }
   }

   for(Int_t iDetIndx=0; iDetIndx< fMbsMappingPar->GetNbMappedDet(); iDetIndx++){
     fhRpcCluMul[iDetIndx]->Write();
     fhRpcCluPosition[iDetIndx]->Write();
     fhRpcCluDelPos[iDetIndx]->Write();
     fhRpcCluTOff[iDetIndx]->Write();
     fhRpcCluDelTOff[iDetIndx]->Write();
     fhRpcCluTrms[iDetIndx]->Write();
     fhRpcCluTot[iDetIndx]->Write();
     fhRpcCluAvWalk[iDetIndx]->Write();
     fhRpcCluAvLnWalk[iDetIndx]->Write();

     for (Int_t iSel=0; iSel<iNSel; iSel++){   // Save trigger selected histos 
       fhTRpcCluMul[iDetIndx][iSel]->Write();
       fhTRpcCluPosition[iDetIndx][iSel]->Write();
       fhTRpcCluTOff[iDetIndx][iSel]->Write();
       fhTRpcCluTot[iDetIndx][iSel]->Write();
       fhTRpcCluAvWalk[iDetIndx][iSel]->Write();
     }

     Int_t iUniqueId = fMbsMappingPar->GetMappedDetUId( iDetIndx );
     Int_t iSmType   = CbmTofAddress::GetSmType( iUniqueId );
     Int_t iSm       = CbmTofAddress::GetSmId( iUniqueId );
     Int_t iRpc      = CbmTofAddress::GetRpcId( iUniqueId );
     Int_t iNent =0;
     if(fCalSel > -1) 
      iNent = fhTRpcCluAvWalk[iDetIndx][fCalSel]->GetEntries();
     else
      iNent = fhRpcCluAvWalk[iDetIndx]->GetEntries();

     if(0==iNent){
       LOG(INFO)<<"CbmTofTestBeamClusterizer::WriteHistos: No entries in Walk histos for " 
		 <<"Smtype"<<iSmType<<", Sm "<<iSm<<", Rpc "<<iRpc 
		 <<FairLogger::endl;
       // continue;
     }
     //     if(-1<fCalSmType && fcalType != iSmType) continue;

     TProfile *htempPos_pfx  = NULL;  
     TProfile *htempTOff_pfx = NULL;
     TH1      *htempTOff_px  = NULL;
     TProfile *hAvPos_pfx    = NULL;  
     TProfile *hAvTOff_pfx   = NULL;
//     TH2      *htempTOff     = NULL; // -> Comment to remove warning because set but never used
     TH2      *htempTot      = NULL;
     TProfile *htempTot_pfx  = NULL; 
     TH1      *htempTot_Mean = NULL; 
     TH1      *htempTot_Off  = NULL; 

     if(-1<fCalSel){
       htempPos_pfx  = fhTRpcCluPosition[iDetIndx][fCalSel]->ProfileX("_pfx",1,fhTRpcCluPosition[iDetIndx][fCalSel]->GetNbinsY());
//       htempTOff     = fhTRpcCluTOff[iDetIndx][fCalSel]; // -> Comment to remove warning because set but never used
       htempTOff_pfx = fhTRpcCluTOff[iDetIndx][fCalSel]->ProfileX("_pfx",1,fhTRpcCluTOff[iDetIndx][fCalSel]->GetNbinsY());
       htempTOff_px  = fhTRpcCluTOff[iDetIndx][fCalSel]->ProjectionX("_px",1,fhTRpcCluTOff[iDetIndx][fCalSel]->GetNbinsY());
       htempTot      = fhTRpcCluTot[iDetIndx][fCalSel]; 
       htempTot_pfx  = fhTRpcCluTot[iDetIndx][fCalSel]->ProfileX("_pfx",1,fhTRpcCluTot[iDetIndx][fCalSel]->GetNbinsY());
       hAvPos_pfx    = fhTSmCluPosition[iSmType][fCalSel]->ProfileX("_pfx",1,fhTSmCluPosition[iSmType][fCalSel]->GetNbinsY());
       hAvTOff_pfx   = fhTSmCluTOff[iSmType][fCalSel]->ProfileX("_pfx",1,fhTSmCluTOff[iSmType][fCalSel]->GetNbinsY(),"s");
     }else  // all triggers
     {
       htempTot      = fhRpcCluTot[iDetIndx];
       htempTot_pfx  = fhRpcCluTot[iDetIndx]->ProfileX("_pfx",1,fhRpcCluTot[iDetIndx]->GetNbinsY());
       hAvPos_pfx    = fhSmCluPosition[iSmType]->ProfileX("_pfx",1,fhSmCluPosition[iSmType]->GetNbinsY());
       hAvTOff_pfx   = fhSmCluTOff[iSmType]->ProfileX("_pfx",1,fhSmCluTOff[iSmType]->GetNbinsY());
       if(-1==fCalSel){    // take corrections from untriggered distributions 
         htempPos_pfx  = fhRpcCluPosition[iDetIndx]->ProfileX("_pfx",1,fhRpcCluPosition[iDetIndx]->GetNbinsY());
//         htempTOff     = fhRpcCluTOff[iDetIndx]; // -> Comment to remove warning because set but never used
         htempTOff_pfx = fhRpcCluTOff[iDetIndx]->ProfileX("_pfx",1,fhRpcCluTOff[iDetIndx]->GetNbinsY(),"s");
         htempTOff_px  = fhRpcCluTOff[iDetIndx]->ProjectionX("_px",1,fhRpcCluTOff[iDetIndx]->GetNbinsY());
       }else
	 { if(-2==fCalSel){ //take corrections from Cluster deviations 
	   htempPos_pfx  = fhRpcCluDelPos[iDetIndx]->ProfileX("_pfx",1,fhRpcCluDelPos[iDetIndx]->GetNbinsY());
//           htempTOff     = fhRpcCluDelTOff[iDetIndx]; // -> Comment to remove warning because set but never used
           htempTOff_pfx = fhRpcCluDelTOff[iDetIndx]->ProfileX("_pfx",1,fhRpcCluDelTOff[iDetIndx]->GetNbinsY());
           htempTOff_px  = fhRpcCluDelTOff[iDetIndx]->ProjectionX("_px",1,fhRpcCluDelTOff[iDetIndx]->GetNbinsY());
         }else
	 {
	   if(-3==fCalSel){  // take corrections from deviations to matched trigger hit
	     htempPos_pfx  = fhRpcCluDelMatPos[iDetIndx]->ProfileX("_pfx",1,fhRpcCluDelMatPos[iDetIndx]->GetNbinsY());
//	     htempTOff     = fhRpcCluDelMatTOff[iDetIndx]; // -> Comment to remove warning because set but never used
	     htempTOff_pfx = fhRpcCluDelMatTOff[iDetIndx]->ProfileX("_pfx",1,fhRpcCluDelMatTOff[iDetIndx]->GetNbinsY());
	     htempTOff_px  = fhRpcCluDelMatTOff[iDetIndx]->ProjectionX("_px",1,fhRpcCluDelMatTOff[iDetIndx]->GetNbinsY());
	   }
	 }
       }
     }

     if(NULL == htempPos_pfx) {
       LOG(INFO)<<"CbmTofTestBeamClusterizer::WriteHistos: Projections not available, continue " 
		<< FairLogger::endl;
       continue;
     }

     htempTot_Mean=htempTot_pfx->ProjectionX("_Mean");
     htempTot_Off =htempTot_pfx->ProjectionX("_Off");

     htempPos_pfx->SetName( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Pos_pfx",iSmType,iSm,iRpc) );
     htempTOff_pfx->SetName( Form("cl_CorSmT%01d_sm%03d_rpc%03d_TOff_pfx",iSmType,iSm,iRpc) );
     htempTot_pfx->SetName( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Tot_pfx",iSmType,iSm,iRpc) );
     htempTot_Mean->SetName( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Tot_Mean",iSmType,iSm,iRpc) );
     htempTot_Off->SetName( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Tot_Off",iSmType,iSm,iRpc) );
     hAvPos_pfx->SetName( Form("cl_CorSmT%01d_Pos_pfx",iSmType) );
     hAvTOff_pfx->SetName( Form("cl_CorSmT%01d_TOff_pfx",iSmType) );

     switch(fCalMode%10){
     case 0 : {                // Initialize
       htempTot_Off->Reset();   // prepare TotOffset histo
       TH1 *hbins[200];
       Int_t nbins=htempTot->GetNbinsX();
       for (int i=0;i<nbins;i++) {
	 hbins[i] = htempTot->ProjectionY(Form("bin%d",i+1),i+1,i+1);
	 Double_t Ymax=hbins[i]->GetMaximum();
	 Int_t iBmax=hbins[i]->GetMaximumBin();
	 TAxis *xaxis = hbins[i]->GetXaxis();
	 Double_t Xmax=xaxis->GetBinCenter(iBmax);
	 Double_t XOff=Xmax-fTotPreRange;
	 XOff = (Double_t)(Int_t)XOff;
	 if(XOff<0) XOff=0;    
	 htempTot_Off->SetBinContent(i+1,XOff);
	 Double_t Xmean=htempTot_Mean->GetBinContent(i+1);
	 if(Xmean<XOff) {
	   LOG(WARNING)<<"Inconsistent  Tot numbers for "
		       <<Form("SmT%01d_sm%03d_rpc%03d bin%d: mean %f, Off %f",iSmType,iSm,iRpc,i,Xmean,XOff)
		       <<FairLogger::endl; 
	 }
	 htempTot_Mean->SetBinContent(i+1,(Xmean-XOff));
	 if ( htempTot_Mean->GetBinContent(i+1) != (Xmean-XOff))
	   LOG(WARNING)<<"Tot numbers not stored properly for "
		       <<Form("SmT%01d_sm%03d_rpc%03d bin%d: mean %f, target %f",iSmType,iSm,iRpc,i,htempTot_Mean->GetBinContent(i+1),Xmean-XOff)
		       <<FairLogger::endl; 
       }
       htempPos_pfx->Write();
       htempTOff_pfx->Write();
       htempTot_pfx->Write();
       htempTot_Mean->Write();
       htempTot_Off->Write();
     }
       break;
     case 1 :                  //save offsets, update walks 
     {
        Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType);
	Int_t iNbCh = fDigiBdfPar->GetNbChan( iSmType, iRpc );
        LOG(INFO)<<"CbmTofTestBeamClusterizer::WriteHistos: restore Offsets and Gains and save Walk for "
		 <<"Smtype"<<iSmType<<", Sm "<<iSm<<", Rpc "<<iRpc 
		 <<FairLogger::endl;
	htempPos_pfx->Reset();    //reset to restore means of original histos
	htempTOff_pfx->Reset();
	htempTot_Mean->Reset();
	htempTot_Off->Reset();
	for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) 
	{
	 Double_t YMean=fDigiBdfPar->GetSigVel(iSmType,iRpc)*0.5
	                   *(fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]-fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]);
	 Double_t TMean=0.5*(fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]+fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]);
	 htempPos_pfx->Fill(iCh,YMean);
         if( ((TProfile *)htempPos_pfx)->GetBinContent(iCh+1)!=YMean) {
	   LOG(ERROR)<<"CbmTofTestBeamClusterizer::WriteHistos: restore unsuccessful! ch "<<iCh
		     <<" got "<< htempPos_pfx->GetBinContent(iCh)<<","<<htempPos_pfx->GetBinContent(iCh+1)
		     <<","<<htempPos_pfx->GetBinContent(iCh+2)
	             <<", expected "<<YMean
		     <<FairLogger::endl;
	 }
	 htempTOff_pfx->Fill(iCh,TMean); 

	 for(Int_t iSide=0; iSide<2; iSide++){
	   htempTot_Mean->SetBinContent(iCh*2+1+iSide,
                          fdTTotMean/fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide] );  //nh +1 empirical(?)
	   htempTot_Off->SetBinContent(iCh*2+1+iSide,fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][iSide]);
	 }
	}

        LOG(DEBUG1)<<" Offset, gain restoring done ... "<<FairLogger::endl;
        htempPos_pfx->Write();
        htempTOff_pfx->Write();
        htempTot_pfx->Write();
        htempTot_Mean->Write();
        htempTot_Off->Write();

        for(Int_t iSel=0; iSel<iNSel; iSel++){
	 // Store DelTof corrections 
	 TDirectory * curdir = gDirectory;
	 gROOT->cd(); //
	 TH1D *hCorDelTof =(TH1D*) gDirectory->FindObjectAny( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));
	 gDirectory->cd( curdir->GetPath() );
	 if (NULL!=hCorDelTof) {
	   TH1D *hCorDelTofout=(TH1D*)hCorDelTof->Clone(Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));
	  hCorDelTofout->Write();
	 }else {
         LOG(INFO)<<" No CorDelTof histo "
		  <<Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel)<<FairLogger::endl;
	 }
	}

        if(   (fCalSmType < 0 && TMath::Abs(fCalSmType) != iSmType)
           || fCalSmType == iSmType)  // select detectors for determination of walk correction
        for( Int_t iCh=0; iCh<fMbsMappingPar->GetSmTypeNbCh(iSmType); iCh++){
          TH2 *h2tmp0;
          TH2 *h2tmp1;
          if(-1<fCalSel){
	    h2tmp0=fhTRpcCluWalk[iDetIndx][fCalSel][iCh][0];
	    h2tmp1=fhTRpcCluWalk[iDetIndx][fCalSel][iCh][1];
	  }else{ // take correction from deviation within clusters 
	    h2tmp0=fhRpcCluWalk[iDetIndx][iCh][0];
	    h2tmp1=fhRpcCluWalk[iDetIndx][iCh][1];
	  }
          if(NULL == h2tmp0){
	    LOG(INFO)<<Form("CbmTofTestBeamClusterizer::WriteHistos: Walk histo not available for SmT %d, Sm %d, Rpc %d, Ch %d",iSmType,iSm,iRpc,iCh)
 		    <<FairLogger::endl;
	    continue;
	  }
          Int_t iNEntries=h2tmp0->GetEntries();
	  if(iCh==0)  // condition to print message only once
	  LOG(INFO)<<Form(" Update Walk correction for SmT %d, Sm %d, Rpc %d, Ch %d, Sel%d: Entries %d",
			  iSmType,iSm,iRpc,iCh,fCalSel,iNEntries)
 		    <<FairLogger::endl;

	 h2tmp0->Write();
	 h2tmp1->Write();
         if(-1<iNEntries){  // always done 
	  TProfile *htmp0 = h2tmp0->ProfileX("_pfx",1,h2tmp0->GetNbinsY());
          TProfile *htmp1 = h2tmp1->ProfileX("_pfx",1,h2tmp1->GetNbinsY());
          TH1D *h1tmp0    = h2tmp0->ProjectionX("_px",1,h2tmp0->GetNbinsY());
          TH1D *h1tmp1    = h2tmp1->ProjectionX("_px",1,h2tmp1->GetNbinsY());
          TH1D *h1ytmp0   = h2tmp0->ProjectionY("_py",1,nbClWalkBinX);  // preserve means
          TH1D *h1ytmp1   = h2tmp1->ProjectionY("_py",1,nbClWalkBinX);
	  Double_t dWMean0=h1ytmp0->GetMean();
	  Double_t dWMean1=h1ytmp1->GetMean();
	  Double_t dWMean=0.5*(dWMean0+dWMean1);
	  Int_t iWalkUpd=1;                        // Walk update mode flag
          if(5==iSmType || 8==iSmType) iWalkUpd=0; // keep both sides consistent for diamonds and pads 
	  for(Int_t iWx=0; iWx<nbClWalkBinX; iWx++){
           if(iSmType==2)
	     LOG(INFO)<<"Update Walk iSm = "<<iSm<<"("<<iNbRpc<<")," << iWx << ": "
		      <<fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]<<" + "
		      <<((TProfile *)htmp0)->GetBinContent(iWx+1)
		      <<FairLogger::endl;
	   switch(iWalkUpd){
	   case 0 :
	     if(h1tmp0->GetBinContent(iWx+1)>WalkNHmin && h1tmp1->GetBinContent(iWx+1)>WalkNHmin ){
	     // preserve y - position (difference) on average 
	       Double_t dWcor=(((TProfile *)htmp0)->GetBinContent(iWx+1) + ((TProfile *)htmp1)->GetBinContent(iWx+1))*0.5;
	       fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]+=dWcor-dWMean;
	       fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iWx]+=dWcor-dWMean;
	     }
	     break;
	   case 1 :
	     if(h1tmp0->GetBinContent(iWx+1)>WalkNHmin)
	       fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]+=((TProfile *)htmp0)->GetBinContent(iWx+1)-dWMean0;
	     if(h1tmp1->GetBinContent(iWx+1)>WalkNHmin)
	       fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iWx]+=((TProfile *)htmp1)->GetBinContent(iWx+1)-dWMean1;
	     break;
	   default :
	     ;
	   }
	  }
          h1tmp0->Reset();
          h1tmp1->Reset();
	  for(Int_t iWx=0; iWx<nbClWalkBinX; iWx++){
	   h1tmp0->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]);
	   h1tmp1->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iWx]);
           if( h1tmp0->GetBinContent(iWx+1)!=fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]) {
	   LOG(ERROR)<<"CbmTofTestBeamClusterizer::WriteHistos: restore unsuccessful! iWx "<<iWx
		     <<" got "<< h1tmp0->GetBinContent(iWx+1)
	             <<", expected "<<fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]
		     <<FairLogger::endl;	 }
	  }
	  h1tmp0->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S0_Walk_px", iSmType, iSm, iRpc, iCh ));
	  h1tmp0->Smooth(iNWalkSmooth);
          htmp0->Write();
          h1tmp0->Write();
	  h1tmp1->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S1_Walk_px", iSmType, iSm, iRpc, iCh ));
	  h1tmp1->Smooth(iNWalkSmooth);
          htmp1->Write();
          h1tmp1->Write();
	 }
        }else{  // preserve whatever is there for fCalSmType !
	 for( Int_t iCh = 0; iCh < fMbsMappingPar->GetSmTypeNbCh(iSmType); iCh++ ) // restore old values 
	 {
          TProfile *htmp0 = fhRpcCluWalk[iDetIndx][iCh][0]->ProfileX("_pfx",1,nbClWalkBinY);
          TProfile *htmp1 = fhRpcCluWalk[iDetIndx][iCh][1]->ProfileX("_pfx",1,nbClWalkBinY);
          TH1D *h1tmp0    = fhRpcCluWalk[iDetIndx][iCh][0]->ProjectionX("_px",1,nbClWalkBinY);
          TH1D *h1tmp1    = fhRpcCluWalk[iDetIndx][iCh][1]->ProjectionX("_px",1,nbClWalkBinY);
	  for(Int_t iWx=0; iWx<nbClWalkBinX; iWx++){
	   h1tmp0->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]);
	   h1tmp1->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iWx]);
           if( h1tmp0->GetBinContent(iWx+1)!=fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]) {
	   LOG(ERROR)<<"CbmTofTestBeamClusterizer::WriteHistos: restore unsuccessful! iWx "<<iWx
		     <<" got "<< h1tmp0->GetBinContent(iWx+1)
	             <<", expected "<<fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]
		     <<FairLogger::endl;	 }
	  }
	  h1tmp0->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S0_Walk_px", iSmType, iSm, iRpc, iCh ));
          htmp0->Write();
          h1tmp0->Write();
	  h1tmp1->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S1_Walk_px", iSmType, iSm, iRpc, iCh ));
          htmp1->Write();
          h1tmp1->Write();
	 }
	}     
     }
     break;

     case 2 :         //update time offsets from positions and times with Sm averages, save walks and DELTOF 
     {    
        Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType);
	Int_t iNbCh = fDigiBdfPar->GetNbChan( iSmType, iRpc );
        if((fCalSmType < 0) || (fCalSmType != iSmType)){   // select detectors for updating offsets
         LOG(INFO)<<"CbmTofTestBeamClusterizer::WriteHistos: (case 2) update Offsets and keep Gains, Walk and DELTOF for "
		  <<"Smtype"<<iSmType<<", Sm "<<iSm<<", Rpc "<<iRpc 
		  <<FairLogger::endl;
	 Int_t iB=iSm*iNbRpc+iRpc;
	 Double_t YMean=((TProfile *)hAvPos_pfx)->GetBinContent(iB+1);  //nh +1 empirical(?)
	 Double_t TMean=((TProfile *)hAvTOff_pfx)->GetBinContent(iB+1);
	 Double_t TWidth=((TProfile *)hAvTOff_pfx)->GetBinError(iB+1);
         Double_t dTYOff=YMean/fDigiBdfPar->GetSigVel(iSmType,iRpc) ;


	 if (fiBeamRefType == iSmType && fiBeamRefSm == iSm && fiBeamRefDet == iRpc) TMean=0.; // don't shift reference counter
         
	 LOG(INFO)<<Form("<ICor> Correct %d %d %d by TMean=%8.2f, TYOff=%8.2f, TWidth=%8.2f, ",iSmType,iSm,iRpc,TMean,dTYOff,TWidth)
		  <<FairLogger::endl;
	
		  
	 for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) // update Offset and Gain 
	 {
	  fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0] += -dTYOff + TMean ;
	  fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1] += +dTYOff + TMean ;

	  LOG(INFO)<<"CbmTofTestBeamClusterizer::FillCalHist:" 
		   <<" SmT "<< iSmType<<" Sm "<<iSm<<" Rpc "<<iRpc<<" Ch "<<iCh
		   <<": YMean "<<YMean<<", TMean "<< TMean
		   <<" -> " << Form(" %f, %f, %f, %f ",fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0],
				    fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1],
				    fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][0],
				    fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][1])
		   <<FairLogger::endl;
	 } // for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) 
	}
	htempPos_pfx->Reset();    //reset to store new values 
	htempTOff_pfx->Reset();
	htempTot_Mean->Reset();
	htempTot_Off->Reset();
	for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) // store new values 
	{
	  Double_t YMean=fDigiBdfPar->GetSigVel(iSmType,iRpc)*0.5
	  *(fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]-fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]);
	  Double_t TMean=0.5*(fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]+fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]);
	  htempPos_pfx->Fill(iCh,YMean);
	  if( ((TProfile *)htempPos_pfx)->GetBinContent(iCh+1)!=YMean) {
	   LOG(ERROR)<<"CbmTofTestBeamClusterizer::WriteHistos: restore unsuccessful! ch "<<iCh
		     <<" got "<< htempPos_pfx->GetBinContent(iCh)<<","<<htempPos_pfx->GetBinContent(iCh+1)
		     <<","<<htempPos_pfx->GetBinContent(iCh+2)
	             <<", expected "<<YMean
		     <<FairLogger::endl;
	  }
	  htempTOff_pfx->Fill(iCh,TMean); 

	  for(Int_t iSide=0; iSide<2; iSide++){
	   htempTot_Mean->SetBinContent(iCh*2+1+iSide,
                          fdTTotMean/fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide] );  //nh +1 empirical(?)
	   htempTot_Off->SetBinContent(iCh*2+1+iSide,fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][iSide]);
	  }
        } // for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) 

	LOG(DEBUG1)<<" Updating done ... write to file "<<FairLogger::endl;
	htempPos_pfx->Write();
	htempTOff_pfx->Write();
	htempTot_pfx->Write();
	htempTot_Mean->Write();
	htempTot_Off->Write();

	// store old DELTOF histos
	LOG(DEBUG)<<" Copy old DelTof histos from "<< gDirectory->GetName()<<" to file "<<FairLogger::endl;

	for(Int_t iSel=0; iSel<iNSel; iSel++){
	 // Store DelTof corrections 
          TDirectory * curdir = gDirectory;
	  gROOT->cd(); //
	  TH1D *hCorDelTof =(TH1D*) gDirectory->FindObjectAny( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));
	  gDirectory->cd( curdir->GetPath() );
	  if (NULL!=hCorDelTof) {
	     TH1D *hCorDelTofout=(TH1D*)hCorDelTof->Clone(Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));
	     hCorDelTofout->Write();
	  }else {
	     LOG(DEBUG)<<" No CorDelTof histo "
		  <<Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel)<<FairLogger::endl;
          }
	}

        // store walk histos
	for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) // store new values 
	{
          TProfile *htmp0 = fhRpcCluWalk[iDetIndx][iCh][0]->ProfileX("_pfx",1,nbClWalkBinY);
          TProfile *htmp1 = fhRpcCluWalk[iDetIndx][iCh][1]->ProfileX("_pfx",1,nbClWalkBinY);
          TH1D *h1tmp0    = fhRpcCluWalk[iDetIndx][iCh][0]->ProjectionX("_px",1,nbClWalkBinY);
          TH1D *h1tmp1    = fhRpcCluWalk[iDetIndx][iCh][1]->ProjectionX("_px",1,nbClWalkBinY);
	  for(Int_t iWx=0; iWx<nbClWalkBinX; iWx++){
	   h1tmp0->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]);
	   h1tmp1->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iWx]);
           if( h1tmp0->GetBinContent(iWx+1)!=fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]) {
	   LOG(ERROR)<<"CbmTofTestBeamClusterizer::WriteHistos: restore unsuccessful! iWx "<<iWx
		     <<" got "<< h1tmp0->GetBinContent(iWx+1)
	             <<", expected "<<fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]
		     <<FairLogger::endl;	 }
	  }
	  h1tmp0->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S0_Walk_px", iSmType, iSm, iRpc, iCh ));
          htmp0->Write();
          h1tmp0->Write();
	  h1tmp1->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S1_Walk_px", iSmType, iSm, iRpc, iCh ));
          htmp1->Write();
          h1tmp1->Write();
	}
     }
     break;
     case 3 :    //update offsets, gains, save walks and DELTOF 
     {     
        Int_t iNbRpc = fDigiBdfPar->GetNbRpc(  iSmType);
	Int_t iNbCh  = fDigiBdfPar->GetNbChan( iSmType, iRpc );
        if((fCalSmType < 0) || (fCalSmType != iSmType) ){     // select detectors for updating offsets
         LOG(INFO)<<"CbmTofTestBeamClusterizer::WriteHistos (calMode==3): update Offsets and Gains, keep Walk and DelTof for "
		  <<"Smtype"<<iSmType<<", Sm "<<iSm<<", Rpc "<<iRpc<<" with " <<  iNbCh << " channels "
	 	  <<" using selector "<<fCalSel
		  <<FairLogger::endl;
	 /*
	 Double_t dTRefMean=0.;
	 if (5 == iSmType && fTRefMode%10 == iSm){   // reference counter
	   dTRefMean=htempTOff->GetMean(2);
	 }
	 */
	 for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) // update Offset and Gain 
	 {
	  Double_t YMean=((TProfile *)htempPos_pfx)->GetBinContent(iCh+1);  //nh +1 empirical(?)
	  Double_t TMean=((TProfile *)htempTOff_pfx)->GetBinContent(iCh+1);
          Double_t dTYOff=YMean/fDigiBdfPar->GetSigVel(iSmType,iRpc) ;

	  if (fiBeamRefType == iSmType && fiBeamRefSm == iSm && fiBeamRefDet == iRpc) {
	    TMean-=((TProfile *)hAvTOff_pfx)->GetBinContent(iSm*iNbRpc+iRpc+1); // don't shift reference counter
	  }

          if(htempTOff_px->GetBinContent(iCh+1)>WalkNHmin){
 	   fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0] += -dTYOff + TMean;
	   fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1] += +dTYOff + TMean;
	  }
	  /*
 	  Double_t TotMean=((TProfile *)htempTot_pfx)->GetBinContent(iCh+1);  //nh +1 empirical(!)
	  if(1<TotMean){
	    fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][0] *= fdTTotMean / TotMean;
	    fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][1] *= fdTTotMean / TotMean;
	  }
	  */
	  for(Int_t iSide=0; iSide<2; iSide++){
	    Int_t ib=iCh*2+1+iSide;
	    TH1 * hbin=htempTot->ProjectionY(Form("bin%d",ib),ib,ib);
	    if(100>hbin->GetEntries()) continue;  //request min number of entries
	    Double_t Ymax=hbin->GetMaximum();
	    Int_t iBmax=hbin->GetMaximumBin();
	    TAxis *xaxis = hbin->GetXaxis();
	    Double_t Xmax=xaxis->GetBinCenter(iBmax)/fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide];
	    Double_t XOff=Xmax-fTotPreRange;
	    if(0){//TMath::Abs(XOff - fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][iSide])>100){
	      LOG(WARNING)<<"XOff changed for "
			  <<Form("SmT%01d_sm%03d_rpc%03d_Side%d: XOff %f, old %f",iSmType,iSm,iRpc,iSide,XOff,fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][iSide])
		          <<FairLogger::endl; 
	    }
	    //	    Double_t TotMean=htempTot_Mean->GetBinContent(ib);
	    Double_t TotMean=hbin->GetMean();
	    if(15==iSmType){
	      LOG(WARNING)<<"Gain for "
			  <<Form("SmT%01d_sm%03d_rpc%03d_Side%d: TotMean %f, prof %f, gain %f, modg %f ",
				 iSmType,iSm,iRpc,iSide,TotMean,htempTot_Mean->GetBinContent(ib),
				 fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide],fdTTotMean / TotMean)
		          <<FairLogger::endl; 	    }
	    if(1<TotMean){
	      fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide] *= fdTTotMean / TotMean;
	    }
	  }
          if(5==iSmType && 
	    fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0] != fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]) {// diamond 
	    LOG(WARNING)<<"CbmTofTestBeamClusterizer::FillCalHist:" 
			<<" SmT "<< iSmType<<" Sm "<<iSm<<" Rpc "<<iRpc<<" Ch "<<iCh
			<<": YMean "<<YMean<<", TMean "<< TMean
			<<" -> " 
                        <<Form(" %f %f %f %f ",fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0],
			                       fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1],
			                       fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][0],
			                       fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][1])
			<<FairLogger::endl;
	    Double_t dTOff=0.5*(fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]+fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]);
	    Double_t dGain=0.5*(fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][0]+fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][1]);        
	    fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]   =dTOff;
	    fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]   =dTOff;
	    fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][0]=dGain;
	    fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][1]=dGain;
	  }   // diamond warning end 
	 }    // for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) 
	}     // iSmType selection condition  

	htempPos_pfx->Reset();    //reset to store new values 
	htempTOff_pfx->Reset();
	htempTot_Mean->Reset();
	htempTot_Off->Reset();
	for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) // store new values 
	{
	 Double_t YMean=fDigiBdfPar->GetSigVel(iSmType,iRpc)*0.5
	  *(fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]-fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]);
	 Double_t TMean=0.5*(fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]+fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]);
	 htempPos_pfx->Fill(iCh,YMean);
         if( ((TProfile *)htempPos_pfx)->GetBinContent(iCh+1)!=YMean) {
	   LOG(ERROR)<<"CbmTofTestBeamClusterizer::WriteHistos: restore unsuccessful! ch "<<iCh
		     <<" got "<< htempPos_pfx->GetBinContent(iCh)<<","<<htempPos_pfx->GetBinContent(iCh+1)
		     <<","<<htempPos_pfx->GetBinContent(iCh+2)
	             <<", expected "<<YMean
		     <<FairLogger::endl;
	 }
	 htempTOff_pfx->Fill(iCh,TMean); 

	  for(Int_t iSide=0; iSide<2; iSide++){
	   htempTot_Mean->SetBinContent(iCh*2+1+iSide,
                          fdTTotMean/fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide] );
	   htempTot_Off->SetBinContent(iCh*2+1+iSide,fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][iSide]);
	  }
	  //	 htempTot_pfx->Fill(iCh,fdTTotMean/fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][1]);
	} // for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) 


        LOG(DEBUG1)<<" Updating done ... write to file "<<FairLogger::endl;
        htempPos_pfx->Write();
        htempTOff_pfx->Write();
        htempTot_pfx->Write();
        htempTot_Mean->Write();
        htempTot_Off->Write();

	// store old DELTOF histos
        LOG(DEBUG)<<" Copy old DelTof histos from "<< gDirectory->GetName()<<" to file "<<FairLogger::endl;

        for(Int_t iSel=0; iSel<iNSel; iSel++){
	 // Store DelTof corrections 
	 TDirectory * curdir = gDirectory;
	 gROOT->cd(); //
	 TH1D *hCorDelTof =(TH1D*) gDirectory->FindObjectAny( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));
	 gDirectory->cd( curdir->GetPath() );
	 if (NULL!=hCorDelTof) {
	   TH1D *hCorDelTofout=(TH1D*)hCorDelTof->Clone(Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));
	  hCorDelTofout->Write();
	 }else {
         LOG(DEBUG)<<" No CorDelTof histo "
		  <<Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel)<<FairLogger::endl;
	 }
	}

        LOG(DEBUG)<<" Store old walk histos to file "<<FairLogger::endl;
	// store walk histos
	for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) // restore old values 
	{
          TProfile *htmp0 = fhRpcCluWalk[iDetIndx][iCh][0]->ProfileX("_pfx",1,nbClWalkBinY);
          TProfile *htmp1 = fhRpcCluWalk[iDetIndx][iCh][1]->ProfileX("_pfx",1,nbClWalkBinY);
          TH1D *h1tmp0    = fhRpcCluWalk[iDetIndx][iCh][0]->ProjectionX("_px",1,nbClWalkBinY);
          TH1D *h1tmp1    = fhRpcCluWalk[iDetIndx][iCh][1]->ProjectionX("_px",1,nbClWalkBinY);
	  for(Int_t iWx=0; iWx<nbClWalkBinX; iWx++){
	   h1tmp0->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]);
	   h1tmp1->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iWx]);
           if( h1tmp0->GetBinContent(iWx+1)!=fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]) {
	   LOG(ERROR)<<"CbmTofTestBeamClusterizer::WriteHistos: restore unsuccessful! iWx "<<iWx
		     <<" got "<< h1tmp0->GetBinContent(iWx+1)
	             <<", expected "<<fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]
		     <<FairLogger::endl;	 }
	  }
	  h1tmp0->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S0_Walk_px", iSmType, iSm, iRpc, iCh ));
          htmp0->Write();
          h1tmp0->Write();
	  h1tmp1->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S1_Walk_px", iSmType, iSm, iRpc, iCh ));
          htmp1->Write();
          h1tmp1->Write();
	}
     }
     break;
     case 4 :   //update DelTof save offsets, gains and walks 
     {    
        Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType);
	Int_t iNbCh = fDigiBdfPar->GetNbChan( iSmType, iRpc );
        LOG(INFO)<<"CbmTofTestBeamClusterizer::WriteHistos: restore Offsets, Gains and Walk, save DelTof for "
		 <<"Smtype"<<iSmType<<", Sm "<<iSm<<", Rpc "<<iRpc 
		 <<FairLogger::endl;
	htempPos_pfx->Reset();    //reset to restore mean of original histos
	htempTOff_pfx->Reset();
	htempTot_Mean->Reset();
	htempTot_Off->Reset();
	for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) 
	{
	 Double_t YMean=fDigiBdfPar->GetSigVel(iSmType,iRpc)*0.5
	                   *(fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]-fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]);
	 Double_t TMean=0.5*(fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]+fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]);
	 htempPos_pfx->Fill(iCh,YMean);
         if( ((TProfile *)htempPos_pfx)->GetBinContent(iCh+1)!=YMean) {
	   LOG(ERROR)<<"CbmTofTestBeamClusterizer::WriteHistos: restore unsuccessful! ch "<<iCh
		     <<" got "<< htempPos_pfx->GetBinContent(iCh)<<","<<htempPos_pfx->GetBinContent(iCh+1)
		     <<","<<htempPos_pfx->GetBinContent(iCh+2)
	             <<", expected "<<YMean
		     <<FairLogger::endl;
	 }
	 htempTOff_pfx->Fill(iCh,TMean); 

	 for(Int_t iSide=0; iSide<2; iSide++){
	   htempTot_Mean->SetBinContent(iCh*2+1+iSide,
                          fdTTotMean/fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide] );  //nh +1 empirical(?)
	   htempTot_Off->SetBinContent(iCh*2+1+iSide,fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][iSide]);
	 }
	}

        LOG(DEBUG1)<<" Restoring of Offsets and Gains done ... "<<FairLogger::endl;
        htempPos_pfx->Write();
        htempTOff_pfx->Write();
        htempTot_pfx->Write();
        htempTot_Mean->Write();
        htempTot_Off->Write();

	// restore walk histos
	for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) // restore old values 
	{
          TProfile *htmp0 = fhRpcCluWalk[iDetIndx][iCh][0]->ProfileX("_pfx",1,nbClWalkBinY);
          TProfile *htmp1 = fhRpcCluWalk[iDetIndx][iCh][1]->ProfileX("_pfx",1,nbClWalkBinY);
          TH1D *h1tmp0    = fhRpcCluWalk[iDetIndx][iCh][0]->ProjectionX("_px",1,nbClWalkBinY);
          TH1D *h1tmp1    = fhRpcCluWalk[iDetIndx][iCh][1]->ProjectionX("_px",1,nbClWalkBinY);
	  for(Int_t iWx=0; iWx<nbClWalkBinX; iWx++){
	   h1tmp0->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]);
	   h1tmp1->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iWx]);
           if( h1tmp0->GetBinContent(iWx+1)!=fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]) {
	   LOG(ERROR)<<"CbmTofTestBeamClusterizer::WriteHistos: restore unsuccessful! iWx "<<iWx
		     <<" got "<< h1tmp0->GetBinContent(iWx+1)
	             <<", expected "<<fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]
		     <<FairLogger::endl;	 }
	  }
	  h1tmp0->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S0_Walk_px", iSmType, iSm, iRpc, iCh ));
          htmp0->Write();
          h1tmp0->Write();
	  h1tmp1->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S1_Walk_px", iSmType, iSm, iRpc, iCh ));
          htmp1->Write();
          h1tmp1->Write();
	}

	// generate/update DelTof corrections 
        if((fCalSmType < 0 && fCalSmType != iSmType) || (fCalSmType == iSmType))  // select detectors for determination of DelTof correction
	{
	  // if( fiBeamRefType == iSmType ) continue;  // no DelTof correction for Diamonds

	  for(Int_t iSel=0; iSel<iNSel; iSel++){
	  TH2 *h2tmp=fhTRpcCluDelTof[iDetIndx][iSel];
          if(NULL == h2tmp){
	   LOG(INFO)<<Form("CbmTofTestBeamClusterizer::WriteHistos:  histo not available for SmT %d, Sm %d, Rpc %d",
			  iSmType,iSm,iRpc)
 		    <<FairLogger::endl;
	    break;
	  }
          Int_t iNEntries=h2tmp->GetEntries();
	  LOG(INFO)<<Form(" Update DelTof correction for SmT %d, Sm %d, Rpc %d, Sel%d: Entries %d",
			  iSmType,iSm,iRpc,iSel,iNEntries)
 	 	  <<FairLogger::endl;
	  h2tmp->Write();
          TProfile *htmp = h2tmp->ProfileX("_pfx",1,h2tmp->GetNbinsY());
          TH1D *h1tmp    = h2tmp->ProjectionX("_px",1,h2tmp->GetNbinsY());
          TH1D *h1ytmp   = h2tmp->ProjectionY("_py",1,h2tmp->GetNbinsX());
	  Double_t dDelMean=0.;//h1ytmp->GetMean();// inspect normalization, interferes with mode 3 for diamonds, nh, 10.01.2015 (?) 
	  for(Int_t iBx=0; iBx<nbClDelTofBinX; iBx++){
	    fvCPDelTof[iSmType][iSm*iNbRpc+iRpc][iBx][iSel]+=((TProfile *)htmp)->GetBinContent(iBx+1)-dDelMean;
	    h1tmp->SetBinContent(iBx+1,fvCPDelTof[iSmType][iSm*iNbRpc+iRpc][iBx][iSel]);
	  }
	  h1tmp->SetName(Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof", iSmType, iSm, iRpc,iSel));
	  h1tmp->Write();
	 }
	}else{  // copy existing histograms  
         for(Int_t iSel=0; iSel<iNSel; iSel++){
	  // Store DelTof corrections 
	  TDirectory * curdir = gDirectory;
	  gROOT->cd(); //
	  TH1D *hCorDelTof =(TH1D*) gDirectory->FindObjectAny( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));
	  gDirectory->cd( curdir->GetPath() );
	  if (NULL!=hCorDelTof) {
	   TH1D *hCorDelTofout=(TH1D*)hCorDelTof->Clone(Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));
           LOG(INFO)<<" Save existing CorDelTof histo "
		  <<Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel)<<FairLogger::endl;
	   hCorDelTofout->Write();
	  }else {
          LOG(DEBUG)<<" No CorDelTof histo "
	 	   <<Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel)<<FairLogger::endl;
	  }
	 }
	}
     }
     break;
     default: 
         LOG(DEBUG)<<"CbmTofTestBeamClusterizer::WriteHistos: update mode "
		   <<fCalMode<<" not yet implemented"
		   <<FairLogger::endl;
     }
   }

   fhDigSpacDifClust->Write();
   fhDigTimeDifClust->Write();
   fhDigDistClust->Write();

   fhClustSizeDifX->Write();
   fhClustSizeDifY->Write();

   fhChDifDifX->Write();
   fhChDifDifY->Write();

   fhNbSameSide->Write();
   fhNbDigiPerChan->Write();


   fhHitsPerTracks->Write();
   if( kFALSE == fDigiBdfPar->ClustUseTrackId() )
      fhPtsPerHit->Write();
   fhTimeResSingHits->Write();
   fhTimeResSingHitsB->Write();
   fhTimePtVsHits->Write();
   fhClusterSize->Write();
   fhClusterSizeType->Write();
   if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
   {
      fhTrackMul->Write();
      fhClusterSizeMulti->Write();
      fhTrk1MulPos->Write();
      fhHiTrkMulPos->Write();
      fhAllTrkMulPos->Write();
      fhMultiTrkProbPos->Divide( fhHiTrkMulPos, fhAllTrkMulPos);
      fhMultiTrkProbPos->Scale( 100.0 );
      fhMultiTrkProbPos->Write();
   } // if( kTRUE == fDigiBdfPar->ClustUseTrackId() )


   gDirectory->cd( oldir->GetPath() );

   fHist->Close();

   return kTRUE;
}
Bool_t   CbmTofTestBeamClusterizer::DeleteHistos()
{
   delete fhClustBuildTime;
   delete fhHitsPerTracks;
   delete fhPtsPerHit;
   delete fhTimeResSingHits;
   delete fhTimeResSingHitsB;
   delete fhTimePtVsHits;
   delete fhClusterSize;
   delete fhClusterSizeType;

   if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
   {
      delete fhTrackMul;
      delete fhClusterSizeMulti;
      delete fhTrk1MulPos;
      delete fhHiTrkMulPos;
      delete fhAllTrkMulPos;
      delete fhMultiTrkProbPos;
   }
   delete fhDigSpacDifClust;
   delete fhDigTimeDifClust;
   delete fhDigDistClust;

   delete fhClustSizeDifX;
   delete fhClustSizeDifY;

   delete fhChDifDifX;
   delete fhChDifDifY;

   delete fhNbSameSide;
   delete fhNbDigiPerChan;

   return kTRUE;
}
/************************************************************************************/
Bool_t   CbmTofTestBeamClusterizer::BuildClusters()
{
   /*
    * FIXME: maybe use the 2D distance (X/Y) as criteria instead of the distance long channel
    * direction
    */
  if(NULL == fTofDigisColl) {
    LOG(INFO) <<" No Digis defined ! Check! " << FairLogger::endl;
    return kFALSE;
  }
  fiNevtBuild++;
  LOG(DEBUG)<<"CbmTofTestBeamClusterizer::BuildClusters from "
	    <<fTofDigisColl->GetEntries()<<" digis in event "<<fiNevtBuild<<FairLogger::endl;

   dTRef=0.;     // reset reference time offset 
   fTRefHits=0.;

   Int_t iNbTofDigi  = fTofDigisColl->GetEntries();
   if( kTRUE )
   {
      for( Int_t iDigInd = 0; iDigInd < iNbTofDigi; iDigInd++ )
      {
       CbmTofDigiExp *pDigi = (CbmTofDigiExp*) fTofDigisColl->At( iDigInd );
       LOG(DEBUG1)<<iDigInd<<" "<<pDigi<<Form(" Address : 0x%08x ",pDigi->GetAddress())
		 <<" SmT " << pDigi->GetType()
		 <<" Sm " << pDigi->GetSm()
		 <<" Rpc "<< pDigi->GetRpc()
		 <<" Ch " << pDigi->GetChannel()
		 <<" S " << pDigi->GetSide()
		 <<" : " << pDigi->ToString()
	 //	 <<" Time "<<pDigi->GetTime()
	 //	 <<" Tot " <<pDigi->GetTot()
		 <<FairLogger::endl;

       Int_t iDetIndx= fMbsMappingPar->GetMappedDetInd( pDigi->GetAddress() );
       if (fMbsMappingPar->GetNbMappedDet()-1<iDetIndx || iDetIndx<0){
	 LOG(DEBUG)<<Form(" Wrong DetIndx %d >< %d,0 ",iDetIndx,fMbsMappingPar->GetNbMappedDet())
		   <<FairLogger::endl;
	 break;
       }
               
       Double_t dTDifMin=dDoubleMax;
       CbmTofDigiExp *pDigi2Min=NULL;
       //       for (Int_t iDigI2 =iDigInd+1; iDigI2<iNbTofDigi;iDigI2++){
       for (Int_t iDigI2 =0; iDigI2<iNbTofDigi;iDigI2++){
	 CbmTofDigiExp *pDigi2 = (CbmTofDigiExp*) fTofDigisColl->At( iDigI2 );
         if( iDetIndx == fMbsMappingPar->GetMappedDetInd( pDigi2->GetAddress() )){
	   if(0.==pDigi->GetSide() && 1.==pDigi2->GetSide()){
	     fhRpcDigiCor[iDetIndx]->Fill( pDigi->GetChannel(),  pDigi2->GetChannel() );
	   }
	   else 
	   {
	     if (1.==pDigi->GetSide() && 0.==pDigi2->GetSide()){
		 fhRpcDigiCor[iDetIndx]->Fill( pDigi2->GetChannel(),  pDigi->GetChannel() );
	     }
	   }
	   if(   pDigi->GetChannel() == pDigi2->GetChannel()
              && pDigi->GetSide()    != pDigi2->GetSide()){
            Double_t dTDif=TMath::Abs(pDigi->GetTime()-pDigi2->GetTime());
	    if(dTDif<dTDifMin){
	     dTDifMin=dTDif;
	     pDigi2Min=pDigi2;
	    }
	   }
	 }
       }
       
       if( pDigi2Min !=NULL ){
	 CbmTofDetectorInfo xDetInfo(kTOF, pDigi->GetType(), pDigi->GetSm(), pDigi->GetRpc(), 0, pDigi->GetChannel());
         Int_t iChId = fTofId->SetDetectorInfo( xDetInfo );
         fChannelInfo = fDigiPar->GetCell( iChId );
	 if(NULL == fChannelInfo){
	   LOG(WARNING)<<Form("BuildClusters: invalid ChannelInfo for 0x%08x",iChId)<<FairLogger::endl;
	   continue;
	 }
         if(  fDigiBdfPar->GetSigVel(pDigi->GetType(),pDigi->GetRpc()) * dTDifMin * 0.5 
	    < fPosYMaxScal*fChannelInfo->GetSizey()) {
	 //check consistency
	   if(8==pDigi->GetType() || 5==pDigi->GetType()){
	     if(pDigi->GetTime() != pDigi2Min->GetTime()){
	       if(fiMsgCnt-- >0){
		   LOG(WARNING)<<" BuildClusters: Inconsistent duplicated digis in event " << fiNevtBuild
			       <<", Ind: "<<iDigInd<< FairLogger::endl;
                   LOG(WARNING)<<"   "<<pDigi->ToString()<< FairLogger::endl;
                   LOG(WARNING)<<"   "<<pDigi2Min->ToString()<< FairLogger::endl;
	       }
	       pDigi2Min->SetTot(pDigi->GetTot());
	       pDigi2Min->SetTime(pDigi->GetTime());
	     }
	 }

	 // average ToTs! temporary fix, FIXME
	 /*
	 Double_t dAvTot=0.5*(pDigi->GetTot()+pDigi2Min->GetTot());
         pDigi->SetTot(dAvTot);
         pDigi2Min->SetTot(dAvTot);
	 LOG(DEBUG)<<" BuildClusters: TDif "<<dTDifMin<<", Average Tot "<<dAvTot<<FairLogger::endl;
	 LOG(DEBUG)<<"      "<<pDigi->ToString()<< FairLogger::endl;
         LOG(DEBUG)<<"      "<<pDigi2Min->ToString()<< FairLogger::endl;
	 */
       }
      }
   }
   }

   // First Time order the Digis array
   // fTofDigisColl->Sort();

   // Then loop over the digis array and store the Digis in separate vectors for
   // each RPC modules
   if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
   {
      CbmTofDigiExp *pDigi;
      for( Int_t iDigInd = 0; iDigInd < iNbTofDigi; iDigInd++ )
      {
         pDigi = (CbmTofDigiExp*) fTofDigisColl->At( iDigInd );
         LOG(DEBUG1)<<"CbmTofTestBeamClusterizer::BuildClusters: "
	           <<iDigInd<<" "<<pDigi<<" "
		   <<pDigi->GetType()<<" "
		   <<pDigi->GetSm()<<" "
		   <<pDigi->GetRpc()<<" "
		   <<pDigi->GetChannel()<<" "
		   <<pDigi->GetTime()<<" "
		   <<pDigi->GetTot()
		   <<FairLogger::endl;
         if(    fDigiBdfPar->GetNbSmTypes() > pDigi->GetType()  // prevent crash due to misconfiguration 
	     &&	fDigiBdfPar->GetNbSm(  pDigi->GetType()) > pDigi->GetSm()
	     && fDigiBdfPar->GetNbRpc( pDigi->GetType()) > pDigi->GetRpc()
	     && fDigiBdfPar->GetNbChan(pDigi->GetType(),0) >pDigi->GetChannel() 
		)
	 {
         fStorDigiExp[pDigi->GetType()]
                     [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
                     [pDigi->GetChannel()].push_back(pDigi);
         fStorDigiInd[pDigi->GetType()]
                     [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
                     [pDigi->GetChannel()].push_back(iDigInd);
	 LOG(DEBUG1)<<FairLogger::endl<<" CluCal-Init: "<<pDigi->ToString()<<FairLogger::endl;
	 // apply calibration vectors 
	 pDigi->SetTime(pDigi->GetTime()- // calibrate Digi Time 
			fvCPTOff[pDigi->GetType()]
		       [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
		       [pDigi->GetChannel()]
		       [pDigi->GetSide()]);
	 LOG(DEBUG1)<<" CluCal-TOff: "<<pDigi->ToString()<<FairLogger::endl;

	 Double_t dTot = pDigi->GetTot()-  // subtract Offset 
		       fvCPTotOff[pDigi->GetType()]
		       [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
		       [pDigi->GetChannel()]
	               [pDigi->GetSide()];
	 if (dTot<1.)  dTot=1;
	 pDigi->SetTot(dTot *  // calibrate Digi ToT 
		       fvCPTotGain[pDigi->GetType()]
		       [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
		       [pDigi->GetChannel()]
		       [pDigi->GetSide()]);

	 // walk correction 
         Double_t dTotBinSize = (fdTOTMax-fdTOTMin)/ nbClWalkBinX;
         Int_t iWx = (Int_t)((pDigi->GetTot()-fdTOTMin)/dTotBinSize);
         if (0>iWx) iWx=0;
         if (iWx>nbClWalkBinX) iWx=nbClWalkBinX-1;	
	 Double_t dDTot = (pDigi->GetTot()-fdTOTMin)/dTotBinSize-(Double_t)iWx-0.5;
	 Double_t dWT  = fvCPWalk[pDigi->GetType()]
		         [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
		         [pDigi->GetChannel()]
		         [pDigi->GetSide()]
                         [iWx];
         if(dDTot > 0) {    // linear interpolation to next bin
	   dWT += dDTot * (fvCPWalk[pDigi->GetType()]
		          [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
		          [pDigi->GetChannel()]
		          [pDigi->GetSide()]
                          [iWx+1]
	                 -fvCPWalk[pDigi->GetType()]
		          [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
		          [pDigi->GetChannel()]
		          [pDigi->GetSide()]
			  [iWx]); //memory leak???
	 }else  // dDTot < 0,  linear interpolation to next bin
	 {
	   dWT -= dDTot * (fvCPWalk[pDigi->GetType()]
		          [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
		          [pDigi->GetChannel()]
		          [pDigi->GetSide()]
                          [iWx-1]
	                 -fvCPWalk[pDigi->GetType()]
		          [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
		          [pDigi->GetChannel()]
		          [pDigi->GetSide()]
			  [iWx]); //memory leak???
	 }
	     
         pDigi->SetTime(pDigi->GetTime() - dWT); // calibrate Digi Time 
	 LOG(DEBUG1)<<" CluCal-Walk: "<<pDigi->ToString()<<FairLogger::endl;

	 if(0) {//pDigi->GetType()==7 && pDigi->GetSm()==0){
	  LOG(INFO)<<"CbmTofTestBeamClusterizer::BuildClusters: CalDigi "
	            <<iDigInd<<",  T "
		    <<pDigi->GetType()<<", Sm "
		    <<pDigi->GetSm()<<", R "
		    <<pDigi->GetRpc()<<", Ch "
		    <<pDigi->GetChannel()<<", S "
		    <<pDigi->GetSide()<<", T "
		    <<pDigi->GetTime()<<", Tot "
		    <<pDigi->GetTot()
		    <<", TotGain "<<
	               fvCPTotGain[pDigi->GetType()]
		       [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
		       [pDigi->GetChannel()]
		       [pDigi->GetSide()]
		    <<", TotOff "<<
	               fvCPTotOff[pDigi->GetType()]
		       [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
		       [pDigi->GetChannel()]
		       [pDigi->GetSide()]
	            <<", Walk "<<iWx<<": "<<
		        fvCPWalk[pDigi->GetType()]
		        [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
		        [pDigi->GetChannel()]
		        [pDigi->GetSide()]
                        [iWx]
		   <<FairLogger::endl;

	  LOG(INFO)<<" dDTot "<<dDTot
		   <<" BinSize: "<<dTotBinSize
		   <<", CPWalk "<<fvCPWalk[pDigi->GetType()]
		        [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
		        [pDigi->GetChannel()]
		        [pDigi->GetSide()]
	                [iWx-1]
		  <<", "<<fvCPWalk[pDigi->GetType()]
		        [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
		        [pDigi->GetChannel()]
		        [pDigi->GetSide()]
                        [iWx]
		  <<", "<<fvCPWalk[pDigi->GetType()]
		        [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
		        [pDigi->GetChannel()]
		        [pDigi->GetSide()]
                        [iWx+1]
		  <<" -> dWT = "<< dWT
		  <<FairLogger::endl;
	  }
 	 } else 
	   {
           LOG(DEBUG)<<"CbmTofTestBeamClusterizer::BuildClusters: Skip Digi "
		     <<" Type "<<pDigi->GetType()<<" "<< fDigiBdfPar->GetNbSmTypes()
		     <<" Sm "  <<pDigi->GetSm()<<" " << fDigiBdfPar->GetNbSm(pDigi->GetType())
		     <<" Rpc " <<pDigi->GetRpc()<<" "<< fDigiBdfPar->GetNbRpc(pDigi->GetType())
		     <<" Ch "  <<pDigi->GetChannel()<<" "<<fDigiBdfPar->GetNbChan(pDigi->GetType(),0)
		     <<FairLogger::endl;
	   }
      } // for( Int_t iDigInd = 0; iDigInd < nTofDigi; iDigInd++ )
   } // if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
      else
      {
         CbmTofDigi *pDigi;
         for( Int_t iDigInd = 0; iDigInd < iNbTofDigi; iDigInd++ )
         {
            pDigi = (CbmTofDigi*) fTofDigisColl->At( iDigInd );
            fStorDigi[pDigi->GetType()]
                     [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
                     [pDigi->GetChannel()].push_back(pDigi);
         fStorDigiInd[pDigi->GetType()]
                     [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
                     [pDigi->GetChannel()].push_back(iDigInd);
         } // for( Int_t iDigInd = 0; iDigInd < nTofDigi; iDigInd++ )
      } // else of if( kTRUE == fDigiBdfPar->UseExpandedDigi() )

   // Then build clusters inside each RPC module
   // Assume only 0 or 1 Digi per channel/side in each event
   // Use simplest method possible, scan direction independent:
   // a) Loop over channels in the RPC starting from 0
   //   * If strips
   //     i) Loop over Digis to check if both ends of the channel have a Digi
   //    ii) Reconstruct a mean channel time and a mean position
   //     + If a Hit is currently filled & the mean position (space, time) is less than XXX from last channel position
   //   iii) Add the mean channel time and the mean position to the ones of the hit
   //     + else
   //   iii) Use nb of strips in cluster to cal. the hit mean time and pos (charge/tot weighting)
   //    iv) Save the hit
   //     v) Start a new hit with current channel
   //   * else (pads)
   //     i) Loop over Digis to find if this channel fired
   //    ii) FIXME: either scan all other channels to check for matching Digis or have more than 1 hit open
   Int_t iNbSmTypes = fDigiBdfPar->GetNbSmTypes();
   // Hit variables
   Double_t dWeightedTime = 0.0;
   Double_t dWeightedPosX = 0.0;
   Double_t dWeightedPosY = 0.0;
   Double_t dWeightedPosZ = 0.0;
   Double_t dWeightsSum   = 0.0;
   std::vector< CbmTofPoint* > vPtsRef;
   std::vector< Int_t > vDigiIndRef;
   Int_t    iNbChanInHit  = 0;
   // Last Channel Temp variables
   Int_t    iLastChan = -1;
//   Double_t dLastPosX = 0.0; // -> Comment to remove warning because set but never used
   Double_t dLastPosY = 0.0;
   Double_t dLastTime = 0.0;
   // Channel Temp variables
   Double_t dPosX = 0.0;
   Double_t dPosY = 0.0;
   Double_t dPosZ = 0.0;
   Double_t dTime = 0.0;
   Double_t dTimeDif = 0.0;
   Double_t dTotS = 0.0;
   fiNbSameSide = 0;
   if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
   {
      for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
      {
         Int_t iNbSm  = fDigiBdfPar->GetNbSm(  iSmType);
         Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType);
         for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
            for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ )
            {
               Int_t iNbCh = fDigiBdfPar->GetNbChan( iSmType, iRpc );
               Int_t iChType = fDigiBdfPar->GetChanType( iSmType, iRpc );
	       LOG(DEBUG2)<<"CbmTofTestBeamClusterizer::BuildClusters: RPC - Loop  "
			 << Form(" %3d %3d %3d %3d ",iSmType,iSm,iRpc,iChType)
                         <<FairLogger::endl;
               fviClusterMul[iSmType][iSm][iRpc]=0; 
               Int_t  iChId = 0;
               if( 0 == iChType )
               {
                  // Don't spread clusters over RPCs!!!
                  dWeightedTime = 0.0;
                  dWeightedPosX = 0.0;
                  dWeightedPosY = 0.0;
                  dWeightedPosZ = 0.0;
                  dWeightsSum   = 0.0;
                  iNbChanInHit  = 0;
                  vPtsRef.clear();
                  // For safety reinitialize everything
                  iLastChan = -1;
//                  dLastPosX = 0.0; // -> Comment to remove warning because set but never used
                  dLastPosY = 0.0;
                  dLastTime = 0.0;
                  LOG(DEBUG2)<<"CbmTofTestBeamClusterizer::BuildClusters: ChanOrient "
                             << Form(" %3d %3d %3d %3d %3d ",iSmType,iSm,iRpc,fDigiBdfPar->GetChanOrient( iSmType, iRpc ),iNbCh)
                             <<FairLogger::endl;

                  if( 1 == fDigiBdfPar->GetChanOrient( iSmType, iRpc ) )
                  {
                     // Horizontal strips => X comes from left right time difference
                  } // if( 1 == fDigiBdfPar->GetChanOrient( iSmType, iRpc ) )
                     else
                     {
                        // Vertical strips => Y comes from bottom top time difference
                        for( Int_t iCh = 0; iCh < iNbCh; iCh++ )
                        {
			  LOG(DEBUG3)<<"CbmTofTestBeamClusterizer::BuildClusters: VDigisize "
			     << Form(" T %3d Sm %3d R %3d Ch %3d Size %3lu ",
                        iSmType,iSm,iRpc,iCh,fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].size())
                             <<FairLogger::endl;

                           if( 0 < fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].size() )
                              fhNbDigiPerChan->Fill( fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].size() );
                           while( 1 < fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].size() )
                           {

                              while( (fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][0])->GetSide() ==
                                      (fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][1])->GetSide() )
                              {
                                 // Not one Digi of each end!
                                 fiNbSameSide++;
                                 LOG(DEBUG) << "CbmTofTestBeamClusterizer::BuildClusters: SameSide Hits! Times: "
					      <<   (fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][0])->GetTime()
					      << ", "<<(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][1])->GetTime()
					      <<", DeltaT " <<(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][1])->GetTime() - 
                                                              (fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][0])->GetTime()
				              <<FairLogger::endl;
                                 LOG(DEBUG2)<<" SameSide Erase fStor entries(d) "<<iSmType<<", SR "<<iSm*iNbRpc+iRpc<<", Ch"<<iCh
					       <<FairLogger::endl;
				 fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
				 fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
				 if(2 > fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].size()) break;
				 continue;  
                              }

			      LOG(DEBUG2) << "CbmTofTestBeamClusterizer::BuildClusters: digis processing for " 
					  << Form(" SmT %3d Sm %3d Rpc %3d Ch %3d # %3lu ",iSmType,iSm,iRpc,iCh,
						  fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].size())
		  	                  <<FairLogger::endl;
                              if(2 > fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].size()) break;
                              Int_t iLastChId = iChId; // Save Last hit channel

                              // 2 Digis = both sides present
                              CbmTofDetectorInfo xDetInfo(kTOF, iSmType, iSm, iRpc, 0, iCh);
                              iChId = fTofId->SetDetectorInfo( xDetInfo );
			      Int_t iUCellId=CbmTofAddress::GetUniqueAddress(iSm,iRpc,iCh,0,iSmType);
			      LOG(DEBUG1)<<"CbmTofTestBeamClusterizer::BuildClusters:" 
					 << Form(" T %3d Sm %3d R %3d Ch %3d size %3lu ",
						 iSmType,iSm,iRpc,iCh,fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].size())
					 << Form(" ChId: 0x%08x 0x%08x ",iChId,iUCellId)
					 <<FairLogger::endl;
                              fChannelInfo = fDigiPar->GetCell( iChId );
                              if(NULL == fChannelInfo){
			      LOG(ERROR)<<"CbmTofTestBeamClusterizer::BuildClusters: no geometry info! "
					<< Form(" %3d %3d %3d %3d 0x%08x 0x%08x ",iSmType, iSm, iRpc, iCh, iChId,iUCellId)
					<<FairLogger::endl;
			      break;
			      }

                              CbmTofDigiExp * xDigiA = fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][0];
                              CbmTofDigiExp * xDigiB = fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][1];

			      LOG(DEBUG2) << "    "<<xDigiA->ToString()<<FairLogger::endl;
			      LOG(DEBUG2) << "    "<<xDigiB->ToString()<<FairLogger::endl;

                              // The "Strip" time is the mean time between each end
                              dTime    =0.5 * ( xDigiA->GetTime() + xDigiB->GetTime() ) ; 
                              dTimeDif =      ( xDigiA->GetTime() - xDigiB->GetTime() ) ; 
			      if(5==iSmType && dTimeDif !=0.) { 
				// FIXME -> Overflow treatment in calib/tdc/TMbsCalibTdcTof.cxx
				LOG(DEBUG)<<"CbmTofTestBeamClusterizer::BuildClusters: Diamond hit in "
					  << iSm <<" with inconsistent digits " 
					  <<  xDigiA->GetTime() << ", " << xDigiB->GetTime()
				          << " -> "<<dTimeDif
				    <<FairLogger::endl;
				LOG(DEBUG) << "    "<<xDigiA->ToString()<<FairLogger::endl;
				LOG(DEBUG) << "    "<<xDigiB->ToString()<<FairLogger::endl;
			      }

                              // Weight is the total charge => sum of both ends ToT
                              dTotS = xDigiA->GetTot() + xDigiB->GetTot();

			      TGeoNode *fNode=        // prepare local->global trafo
			      gGeoManager->FindNode(fChannelInfo->GetX(),fChannelInfo->GetY(),fChannelInfo->GetZ());
			      LOG(DEBUG2)<<Form(" Node at (%6.1f,%6.1f,%6.1f) : 0x%p",
					       fChannelInfo->GetX(),fChannelInfo->GetY(),fChannelInfo->GetZ(),fNode)
					<<FairLogger::endl;
			      //	  fNode->Print();			 
			      // switch to local coordinates, (0,0,0) is in the center of counter  ?
			      dPosX=((Double_t)(-iNbCh/2 + iCh)+0.5)*fChannelInfo->GetSizex();
			      dPosY=0.;
			      dPosZ=0.;

                              if( 1 == xDigiA->GetSide() )
                                 // 0 is the top side, 1 is the bottom side
				dPosY += fDigiBdfPar->GetSigVel(iSmType,iRpc) * dTimeDif * 0.5;
                              else			      
                                 // 0 is the bottom side, 1 is the top side
				dPosY += -fDigiBdfPar->GetSigVel(iSmType,iRpc) * dTimeDif * 0.5;

			      LOG(DEBUG1)
				   <<"CbmTofTestBeamClusterizer::BuildClusters: NbChanInHit  "
				   << Form(" %3d %3d %3d %3d %3d 0x%p %1.0f Time %f PosX %f PosY %f Svel %f ",
					   iNbChanInHit,iSmType,iRpc,iCh,iLastChan,xDigiA,xDigiA->GetSide(),
					   dTime,dPosX,dPosY,fDigiBdfPar->GetSigVel(iSmType,iRpc))
				//  << Form( ", Offs %f, %f ",fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0],
				//                            fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1])
				   <<FairLogger::endl;

                              // Now check if a hit/cluster is already started
                              if( 0 < iNbChanInHit) 
                              {
                                 if( iLastChan == iCh - 1 )
                                 {
                                    fhDigTimeDifClust->Fill( dTime - dLastTime );
                                    fhDigSpacDifClust->Fill( dPosY - dLastPosY );
                                    fhDigDistClust->Fill( dPosY - dLastPosY,
                                                          dTime - dLastTime );
                                 } 
				 // if( iLastChan == iCh - 1 )
                                 // a cluster is already started => check distance in space/time
                                 // For simplicity, just check along strip direction for now
                                 // and break cluster when a not fired strip is found
                                 if( TMath::Abs( dTime - dLastTime) < fdMaxTimeDist &&
                                      iLastChan == iCh - 1 &&
                                      TMath::Abs( dPosY - dLastPosY) < fdMaxSpaceDist  )
                                 {
                                    // Add to cluster/hit
                                    dWeightedTime += dTime*dTotS;
                                    dWeightedPosX += dPosX*dTotS;
                                    dWeightedPosY += dPosY*dTotS;
                                    dWeightedPosZ += dPosZ*dTotS;
                                    dWeightsSum   += dTotS;
                                    iNbChanInHit  += 1;
                                    // if one of the side digi comes from a CbmTofPoint not already found
                                    // in this cluster, save its pointer
                                    // Bool_t bFoundA = kFALSE;
                                    // Bool_t bFoundB = kFALSE;
                                    if(NULL != fTofPointsColl){ // MC - FIXME 
                                    if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
                                       for( Int_t iPtRef = 0; iPtRef < vPtsRef.size(); iPtRef++)
                                       {
					//if( ((CbmTofPoint*)(vPtsRef[iPtRef]))->GetTrackID() == ((CbmTofPoint*)(xDigiA->GetLinks()))->GetTrackID() )
                                        //bFoundA = kTRUE;
					//if( ((CbmTofPoint*)(vPtsRef[iPtRef]))->GetTrackID() == ((CbmTofPoint*)(xDigiB->GetLinks()))->GetTrackID() )
                                        //bFoundB = kTRUE;
                                       } // for( Int
                                       else for( Int_t iPtRef = 0; iPtRef < vPtsRef.size(); iPtRef++)
                                       {
					 //if( vPtsRef[iPtRef] == (CbmTofPoint*)xDigiA->GetLinks() )
					 //bFoundA = kTRUE;
					 //if( vPtsRef[iPtRef] == (CbmTofPoint*)xDigiB->GetLinks() )
					 //bFoundB = kTRUE;
                                       } // for( Int_t iPtRef = 0; iPtRef < vPtsRef.size(); iPtRef++)

                                    // CbmTofPoint pointer for 1st digi not found => save it
                                    //if( kFALSE == bFoundA )
				      //                                       vPtsRef.push_back( (CbmTofPoint*)(xDigiA->GetLinks()) );
                                    // CbmTofPoint pointer for 2nd digi not found => save it
				      //                                    if( kFALSE == bFoundB )
				      //                                       vPtsRef.push_back( (CbmTofPoint*)(xDigiB->GetLinks()) );
				    } // MC end 
				    vDigiIndRef.push_back( (Int_t )(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh][0]));
				    vDigiIndRef.push_back( (Int_t )(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh][1]));

                                    LOG(DEBUG1)<<" Add Digi and erase fStor entries(a): NbChanInHit "<< iNbChanInHit<<", "
					     <<iSmType<<", SR "<<iSm*iNbRpc+iRpc<<", Ch"<<iCh
					     <<FairLogger::endl;
		
				    fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
				    fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
				    fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
				    fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
 
                                } // if current Digis compatible with last fired chan
                                 else
                                 {
                                    // Save Hit
                                    dWeightedTime /= dWeightsSum;
                                    dWeightedPosX /= dWeightsSum;
                                    dWeightedPosY /= dWeightsSum;
                                    dWeightedPosZ /= dWeightsSum;
				    //  TVector3 hitPosLocal(dWeightedPosX, dWeightedPosY, dWeightedPosZ);
				    //TVector3 hitPos;
                                    Double_t hitpos_local[3];
				    hitpos_local[0] = dWeightedPosX;
				    hitpos_local[1] = dWeightedPosY;
				    hitpos_local[2] = dWeightedPosZ;

				    Double_t hitpos[3];
				    TGeoNode*	 cNode   = gGeoManager->GetCurrentNode();
				    TGeoHMatrix* cMatrix = gGeoManager->GetCurrentMatrix();
				    //cNode->Print();
				    //cMatrix->Print();

				    gGeoManager->LocalToMaster(hitpos_local, hitpos);
				    LOG(DEBUG1)<<
				    Form(" LocalToMaster for node 0x%p: (%6.1f,%6.1f,%6.1f) ->(%6.1f,%6.1f,%6.1f)", 
					 cNode, hitpos_local[0], hitpos_local[1], hitpos_local[2], 
					 hitpos[0], hitpos[1], hitpos[2])
					      <<FairLogger::endl;

                                    TVector3 hitPos(hitpos[0],hitpos[1],hitpos[2]);

                                    // Simple errors, not properly done at all for now
                                    // Right way of doing it should take into account the weight distribution
                                    // and real system time resolution
				    TVector3 hitPosErr(0.5,0.5,0.5);  // including positioning uncertainty
				    /*
                                    TVector3 hitPosErr( fChannelInfo->GetSizex()/TMath::Sqrt(12.0),   // Single strips approximation
                                       0.5, // Use generic value 
				       1.);

				    */				       // fDigiBdfPar->GetFeeTimeRes() * fDigiBdfPar->GetSigVel(iSmType,iRpc), // Use the electronics resolution
                                       //fDigiBdfPar->GetNbGaps( iSmType, iRpc)*
				       //fDigiBdfPar->GetGapSize( iSmType, iRpc)/ //10.0 / // Change gap size in cm
                                       //TMath::Sqrt(12.0) ); // Use full RPC thickness as "Channel" Z size

				    // Int_t iDetId = vPtsRef[0]->GetDetectorID();// detID = pt->GetDetectorID() <= from TofPoint
			            // calc mean ch from dPosX=((Double_t)(-iNbCh/2 + iCh)+0.5)*fChannelInfo->GetSizex();

			            Int_t iChm=floor(dWeightedPosX/fChannelInfo->GetSizex())+iNbCh/2;
                                    Int_t iDetId = CbmTofAddress::GetUniqueAddress(iSm,iRpc,iChm,0,iSmType);
                                    Int_t iRefId = 0; // Index of the correspondng TofPoint
                                    if(NULL != fTofPointsColl) {
				      iRefId = fTofPointsColl->IndexOf( vPtsRef[0] );
				    }
				    LOG(DEBUG)<<"CbmTofTestBeamClusterizer::BuildClusters: Save Hit  "
					       << Form(" %3d %3d 0x%08x %3d %3d %3d %f %f",
						       fiNbHits,iNbChanInHit,iDetId,iCh,iLastChan,iRefId,
						       dWeightedTime,dWeightedPosY)
				               <<", DigiSize: "<<vDigiIndRef.size()
					       <<", DigiInds: ";

                                    fviClusterMul[iSmType][iSm][iRpc]++; 

				    for (Int_t i=0; i<vDigiIndRef.size();i++){
				      LOG(DEBUG)<<" "<<vDigiIndRef.at(i);
				    }
				    LOG(DEBUG)  <<FairLogger::endl;
				    if(	vDigiIndRef.size() < 2 ){
				      LOG(WARNING)<<"CbmTofTestBeamClusterizer::BuildClusters: Digi refs for Hit "
					  	  << fiNbHits<<":	vDigiIndRef.size()"
				                  <<FairLogger::endl;
				    }			    
                                    if(fiNbHits>0){
                                      CbmTofHit *pHitL = (CbmTofHit*) fTofHitsColl->At(fiNbHits-1);
                                      if(iDetId == pHitL->GetAddress() && dWeightedTime==pHitL->GetTime()){
				      LOG(DEBUG)<<"CbmTofTestBeamClusterizer::BuildClusters: Store Hit twice? "
						  <<" fiNbHits "<<fiNbHits<<", "<<Form("0x%08x",iDetId)
				                  <<FairLogger::endl;

				       for (Int_t i=0; i<vDigiIndRef.size();i++){
				 	 CbmTofDigiExp *pDigiC = (CbmTofDigiExp*) fTofDigisColl->At(vDigiIndRef.at(i));
					 LOG(DEBUG)<<" Digi  "<<pDigiC->ToString()<<FairLogger::endl;
				       }
				       CbmMatch* digiMatchL=(CbmMatch *)fTofDigiMatchColl->At(pHitL->GetRefId());
				       for (Int_t i=0; i<digiMatchL->GetNofLinks();i++){
					 CbmLink L0 = digiMatchL->GetLink(i);  
					 Int_t iDigIndL=L0.GetIndex();
					 CbmTofDigiExp *pDigiC = (CbmTofDigiExp*) fTofDigisColl->At(iDigIndL);
					 LOG(DEBUG)<<" DigiL "<<pDigiC->ToString()<<FairLogger::endl;
				       }
				      }
				    }
                                    new((*fTofHitsColl)[fiNbHits]) CbmTofHit( iDetId,
							hitPos, hitPosErr,  //local detector coordinates
							fiNbHits,  // this number is used as reference!!
                                                        dWeightedTime,
							vPtsRef.size(), // flag  = number of TofPoints generating the cluster
							0) ; //channel
				      //		vDigiIndRef);

				    new((*fTofDigiMatchColl)[fiNbHits]) CbmMatch();
				    CbmMatch* digiMatch = (CbmMatch *)fTofDigiMatchColl->At(fiNbHits);
				    for (Int_t i=0; i<vDigiIndRef.size();i++){
				      Double_t dTot = ((CbmTofDigiExp*) (fTofDigisColl->At(vDigiIndRef.at(i))))->GetTot();
				      digiMatch->AddLink(CbmLink(dTot,vDigiIndRef.at(i)));
				    }
				  				    
                                    fiNbHits++;
                                    // For Histogramming
                                    fviClusterSize[iSmType][iRpc].push_back(iNbChanInHit);
                                    fviTrkMul[iSmType][iRpc].push_back( vPtsRef.size() );
                                    fvdX[iSmType][iRpc].push_back(dWeightedPosX);
                                    fvdY[iSmType][iRpc].push_back(dWeightedPosY);
				    /*  no TofPoint available for data!  
                                    fvdDifX[iSmType][iRpc].push_back( vPtsRef[0]->GetX() - dWeightedPosX);
                                    fvdDifY[iSmType][iRpc].push_back( vPtsRef[0]->GetY() - dWeightedPosY);
                                    fvdDifCh[iSmType][iRpc].push_back( fGeoHandler->GetCell( vPtsRef[0]->GetDetectorID() ) -1 -iLastChan );
				    */
                                    vPtsRef.clear();
                                    vDigiIndRef.clear();

                                    // Start a new hit
                                    dWeightedTime = dTime*dTotS;
                                    dWeightedPosX = dPosX*dTotS;
                                    dWeightedPosY = dPosY*dTotS;
                                    dWeightedPosZ = dPosZ*dTotS;
                                    dWeightsSum   = dTotS;
                                    iNbChanInHit  = 1;
                                    // Save pointer on CbmTofPoint
				    // vPtsRef.push_back( (CbmTofPoint*)(xDigiA->GetLinks()) );
                                    // Save next digi address
                                    LOG(DEBUG2)<<" Next fStor Digi "<<iSmType<<", SR "<<iSm*iNbRpc+iRpc<<", Ch"<<iCh
					       <<", Dig0 "<<(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh][0])
					       <<", Dig1 "<<(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh][1])
					       <<FairLogger::endl;

				    vDigiIndRef.push_back( (Int_t )(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh][0]));
				    vDigiIndRef.push_back( (Int_t )(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh][1]));
                                    LOG(DEBUG2)<<" Erase fStor entries(b) "<<iSmType<<", SR "<<iSm*iNbRpc+iRpc<<", Ch"<<iCh
					       <<FairLogger::endl;
				    fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
				    fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
				    fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
				    fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].begin());

                                    if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
                                    {
				      //                                       if( ((CbmTofPoint*)(xDigiA->GetLinks()))->GetTrackID() !=
				      //                       ((CbmTofPoint*)(xDigiB->GetLinks()))->GetTrackID() )
                                          // if other side come from a different Track,
                                          // also save the pointer on CbmTofPoint
				      //  vPtsRef.push_back( (CbmTofPoint*)(xDigiB->GetLinks()) );
                                    } // if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
				    //else if( xDigiA->GetLinks() != xDigiB->GetLinks() )
                                          // if other side come from a different TofPoint,
                                          // also save the pointer on CbmTofPoint
				    //    vPtsRef.push_back( (CbmTofPoint*)(xDigiB->GetLinks()) );
                                 } // else of if current Digis compatible with last fired chan
                              } // if( 0 < iNbChanInHit)
                                 else
                                 {
 		                    LOG(DEBUG1)<<"CbmTofTestBeamClusterizer::BuildClusters: "
					       <<Form("1.Hit on channel %d, time: %f",iCh,dTime) 
					       <<FairLogger::endl;

                                    // first fired strip in this RPC
                                    dWeightedTime = dTime*dTotS;
                                    dWeightedPosX = dPosX*dTotS;
                                    dWeightedPosY = dPosY*dTotS;
                                    dWeightedPosZ = dPosZ*dTotS;
                                    dWeightsSum   = dTotS;
                                    iNbChanInHit  = 1;
                                    // Save pointer on CbmTofPoint
                                    //if(NULL != fTofPointsColl)
				    //                                    vPtsRef.push_back( (CbmTofPoint*)(xDigiA->GetLinks()) );
				    vDigiIndRef.push_back( (Int_t )(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh][0]));
				    vDigiIndRef.push_back( (Int_t )(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh][1]));

                                    LOG(DEBUG2)<<" Erase fStor entries(c) "<<iSmType<<", SR "<<iSm*iNbRpc+iRpc<<", Ch"<<iCh
					       <<FairLogger::endl;
				    fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
				    fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
				    fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
				    fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].begin());

                                    if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
                                    {
				      // if( ((CbmTofPoint*)(xDigiA->GetLinks()))->GetTrackID() !=
				      //     ((CbmTofPoint*)(xDigiB->GetLinks()))->GetTrackID() )
                                      // if other side come from a different Track,
                                      // also save the pointer on CbmTofPoint
				      // vPtsRef.push_back( (CbmTofPoint*)(xDigiB->GetLinks()) );
                                    } // if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
				      // else if( xDigiA->GetLinks() != xDigiB->GetLinks() )
                                      // if other side come from a different TofPoint,
                                      // also save the pointer on CbmTofPoint
				    //   vPtsRef.push_back( (CbmTofPoint*)(xDigiB->GetLinks()) );
                                 } // else of if( 0 < iNbChanInHit)
                              iLastChan = iCh;
//                              dLastPosX = dPosX; // -> Comment to remove warning because set but never used
                              dLastPosY = dPosY;
                              dLastTime = dTime;
                           } // if( 2 == fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].size() )
                           fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].clear();
                           fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].clear();
                        } // for( Int_t iCh = 0; iCh < iNbCh; iCh++ )
			LOG(DEBUG2)<<"CbmTofTestBeamClusterizer::BuildClusters: finished V-RPC"
				   << Form(" %3d %3d %3d %d",iSmType,iSm,iRpc,fTofHitsColl->GetEntries())
				   <<FairLogger::endl;
                     } // else of if( 1 == fDigiBdfPar->GetChanOrient( iSmType, iRpc ) )
               } // if( 0 == iChType)
                  else
                  {
                     LOG(ERROR)<<"CbmTofTestBeamClusterizer::BuildClusters => Cluster building "
                           <<"from digis to hits not implemented for pads, Sm type "
                           <<iSmType<<" Rpc "<<iRpc<<FairLogger::endl;
                     return kFALSE;
                  } // else of if( 0 == iChType)

               // Now check if another hit/cluster is started
               // and save it if it's the case
               if( 0 < iNbChanInHit)
               {
		   LOG(DEBUG1)<<"CbmTofTestBeamClusterizer::BuildClusters: Process cluster " 
			      <<iNbChanInHit<<FairLogger::endl;

                  // Check orientation to properly assign errors
                  if( 1 == fDigiBdfPar->GetChanOrient( iSmType, iRpc ) )
                  {
		     LOG(DEBUG1)<<"CbmTofTestBeamClusterizer::BuildClusters: H-Hit " <<FairLogger::endl;
                  } // if( 1 == fDigiBdfPar->GetChanOrient( iSmType, iRpc ) )
                  else
                  {
		     LOG(DEBUG2)<<"CbmTofTestBeamClusterizer::BuildClusters: V-Hit " <<FairLogger::endl;
                     // Save Hit
                     dWeightedTime /= dWeightsSum;
                     dWeightedPosX /= dWeightsSum;
                     dWeightedPosY /= dWeightsSum;
                     dWeightedPosZ /= dWeightsSum;
		     //TVector3 hitPos(dWeightedPosX, dWeightedPosY, dWeightedPosZ);

                     Double_t hitpos_local[3];
		     hitpos_local[0] = dWeightedPosX;
		     hitpos_local[1] = dWeightedPosY;
		     hitpos_local[2] = dWeightedPosZ;

		     Double_t hitpos[3];
		     TGeoNode*	cNode= gGeoManager->GetCurrentNode();
		     TGeoHMatrix* cMatrix = gGeoManager->GetCurrentMatrix();
		     //cNode->Print();
		     //cMatrix->Print();

		     gGeoManager->LocalToMaster(hitpos_local, hitpos);
		     LOG(DEBUG1)<<
		     Form(" LocalToMaster for V-node 0x%p: (%6.1f,%6.1f,%6.1f) ->(%6.1f,%6.1f,%6.1f)", 
			 cNode, hitpos_local[0], hitpos_local[1], hitpos_local[2], 
			 hitpos[0], hitpos[1], hitpos[2])
			     <<FairLogger::endl;

                     TVector3 hitPos(hitpos[0],hitpos[1],hitpos[2]);
                     // TestBeam errors, not properly done at all for now
                     // Right way of doing it should take into account the weight distribution
                     // and real system time resolution
 		     TVector3 hitPosErr(0.5,0.5,0.5);  // including positioning uncertainty
		     /*
                     TVector3 hitPosErr( fChannelInfo->GetSizex()/TMath::Sqrt(12.0),   // Single strips approximation
                                       0.5, // Use generic value 
				       1.);
		     */
		     //                fDigiBdfPar->GetFeeTimeRes() * fDigiBdfPar->GetSigVel(iSmType,iRpc), // Use the electronics resolution
		     //                fDigiBdfPar->GetNbGaps( iSmType, iRpc)*
		     //                fDigiBdfPar->GetGapSize( iSmType, iRpc)/10.0 / // Change gap size in cm
		     //                TMath::Sqrt(12.0) ); // Use full RPC thickness as "Channel" Z size
//                     cout<<"a "<<vPtsRef.size()<<endl;
//                     cout<<"b "<<vPtsRef[0]<<endl;
//                     cout<<"c "<<vPtsRef[0]->GetDetectorID()<<endl;
//                     Int_t iDetId = vPtsRef[0]->GetDetectorID();// detID = pt->GetDetectorID() <= from TofPoint
//                     Int_t iDetId = iChId;
 	             Int_t iChm=floor(dWeightedPosX/fChannelInfo->GetSizex())+iNbCh/2;
                     Int_t iDetId = CbmTofAddress::GetUniqueAddress(iSm,iRpc,iChm,0,iSmType);
                     Int_t iRefId = 0; // Index of the correspondng TofPoint
                     if(NULL != fTofPointsColl) iRefId = fTofPointsColl->IndexOf( vPtsRef[0] );
		     LOG(DEBUG)<<"CbmTofTestBeamClusterizer::BuildClusters: Save V-Hit  "
		     << Form(" %3d %3d 0x%08x %3d 0x%08x", // %3d %3d 
			     fiNbHits,iNbChanInHit,iDetId,iLastChan,iRefId) //vPtsRef.size(),vPtsRef[0])
		       //   dWeightedTime,dWeightedPosY)
				<<", DigiSize: "<<vDigiIndRef.size();
		     LOG(DEBUG)<<", DigiInds: ";
                     for (Int_t i=0; i<vDigiIndRef.size();i++){
		       LOG(DEBUG)<<" "<<vDigiIndRef.at(i);
		     }
		     LOG(DEBUG)  <<FairLogger::endl;
                     
		     fviClusterMul[iSmType][iSm][iRpc]++; 

		     if( vDigiIndRef.size() < 2 ){
		      LOG(WARNING)<<"CbmTofTestBeamClusterizer::BuildClusters: Digi refs for Hit "
				  << fiNbHits<<":	vDigiIndRef.size()"
		                  <<FairLogger::endl;
		     }	
                     if(fiNbHits>0){
                       CbmTofHit *pHitL = (CbmTofHit*) fTofHitsColl->At(fiNbHits-1);
                       if(iDetId == pHitL->GetAddress() && dWeightedTime==pHitL->GetTime())
  		        LOG(DEBUG)<<"CbmTofTestBeamClusterizer::BuildClusters: Store Hit twice? "
				    <<" fiNbHits "<<fiNbHits<<", "<<Form("0x%08x",iDetId)
				    <<FairLogger::endl;
		     }
                     new((*fTofHitsColl)[fiNbHits]) CbmTofHit( iDetId,
                                         hitPos, hitPosErr,
					 fiNbHits,     // -> iRefId
                                         dWeightedTime,
					 vPtsRef.size(),
					 0);

		     //		                         vDigiIndRef);
		     new((*fTofDigiMatchColl)[fiNbHits]) CbmMatch();
		     CbmMatch* digiMatch = (CbmMatch *)fTofDigiMatchColl->At(fiNbHits);
                     for (Int_t i=0; i<vDigiIndRef.size();i++){
		       Double_t dTot = ((CbmTofDigiExp*) (fTofDigisColl->At(vDigiIndRef.at(i))))->GetTot();
		       digiMatch->AddLink(CbmLink(dTot,vDigiIndRef.at(i)));
		     }

                     fiNbHits++;
                     // For Histogramming
                     fviClusterSize[iSmType][iRpc].push_back(iNbChanInHit);
                     fviTrkMul[iSmType][iRpc].push_back( vPtsRef.size() );
                     fvdX[iSmType][iRpc].push_back(dWeightedPosX);
                     fvdY[iSmType][iRpc].push_back(dWeightedPosY);
		     /*
                     fvdDifX[iSmType][iRpc].push_back( vPtsRef[0]->GetX() - dWeightedPosX);
                     fvdDifY[iSmType][iRpc].push_back( vPtsRef[0]->GetY() - dWeightedPosY);
                     fvdDifCh[iSmType][iRpc].push_back( fGeoHandler->GetCell( vPtsRef[0]->GetDetectorID() ) -1 -iLastChan );
		     */
                     vPtsRef.clear();
                     vDigiIndRef.clear();
                  } // else of if( 1 == fDigiBdfPar->GetChanOrient( iSmType, iRpc ) )
               } // if( 0 < iNbChanInHit)
	       LOG(DEBUG2)<<" Fini-A "<<Form(" %3d %3d %3d ",iSmType, iSm, iRpc)<<FairLogger::endl;
            } // for each sm/rpc pair
	       LOG(DEBUG2)<<" Fini-B "<<Form(" %3d ",iSmType)<<FairLogger::endl;
      } // for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
   } // if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
   else
   {
     LOG(ERROR)<<" Compressed Digis not implemented ... "<<FairLogger::endl;
   }
   return kTRUE;
}

Bool_t   CbmTofTestBeamClusterizer::MergeClusters()
{
  // Merge clusters from neigbouring Rpc within a (Super)Module
  if(NULL == fTofHitsColl) {
    LOG(INFO) <<" No Hits defined ! Check! " << FairLogger::endl;
    return kFALSE;
  }
  // inspect hits
  for( Int_t iHitInd = 0; iHitInd < fTofHitsColl->GetEntries(); iHitInd++)
  {
     CbmTofHit *pHit = (CbmTofHit*) fTofHitsColl->At( iHitInd );
     if(NULL == pHit) continue;

     Int_t iDetId = (pHit->GetAddress() & DetMask);
     Int_t iSmType = CbmTofAddress::GetSmType( iDetId );
     Int_t iNbRpc  = fDigiBdfPar->GetNbRpc( iSmType);
     if(iNbRpc>1){  // check for possible mergers
       Int_t iSm     = CbmTofAddress::GetSmId( iDetId );
       Int_t iRpc    = CbmTofAddress::GetRpcId( iDetId );
       Int_t iChId = pHit->GetAddress();
       fChannelInfo = fDigiPar->GetCell( iChId );
       Int_t iCh = CbmTofAddress::GetChannelId(iChId);
       LOG(DEBUG)<<"MergeClusters: Check for mergers in "
		<<Form(" SmT %d, Sm %d, Rpc %d, Ch %d - hit %d",iSmType,iSm,iRpc,iCh,iHitInd)
		<<FairLogger::endl;
       for( Int_t iHitInd2 = iHitInd+1; iHitInd2 < fTofHitsColl->GetEntries(); iHitInd2++)
       {
	  CbmTofHit *pHit2 = (CbmTofHit*) fTofHitsColl->At( iHitInd2 );
	  if(NULL == pHit2) continue;
	  Int_t iDetId2    = (pHit2->GetAddress() & DetMask);
	  Int_t iSmType2   = CbmTofAddress::GetSmType( iDetId2 );
	  if(iSmType2 == iSmType) {
	    Int_t iSm2     = CbmTofAddress::GetSmId( iDetId2 );
	    if(iSm2 == iSm){
	      Int_t iRpc2  = CbmTofAddress::GetRpcId( iDetId2 );
	      if(TMath::Abs(iRpc-iRpc2)==1){  // Found neighbour 
		Int_t iChId2 = pHit2->GetAddress();
		CbmTofCell  *fChannelInfo2 = fDigiPar->GetCell( iChId2 );
		Int_t iCh2 = CbmTofAddress::GetChannelId(iChId2);
		Double_t xPos=pHit->GetX();
		Double_t yPos=pHit->GetY();
		Double_t tof=pHit->GetTime();
		Double_t xPos2=pHit2->GetX();
		Double_t yPos2=pHit2->GetY();
		Double_t tof2=pHit2->GetTime();		
		LOG(DEBUG)<<"MergeClusters: Found hit in neighbour "
		<<Form(" SmT %d, Sm %d, Rpc %d, Ch %d - hit %d",iSmType2,iSm2,iRpc2,iCh2,iHitInd2)
		<<Form(" DX %6.1f, DY %6.1f, DT %6.1f",xPos-xPos2,yPos-yPos2,tof-tof2)
		<<FairLogger::endl;

		CbmMatch* digiMatch=(CbmMatch *)fTofDigiMatchColl->At(pHit->GetRefId());

		if(      TMath::Abs(xPos-xPos2)<fdCaldXdYMax*2.
		      && TMath::Abs(yPos-yPos2)<fdCaldXdYMax*2.
		      && TMath::Abs(tof-tof2)<fMaxTimeDist ){

		  Double_t dTot=0;
		  for (Int_t iLink=0; iLink<digiMatch->GetNofLinks(); iLink+=2){  // loop over digis
		    CbmLink L0 = digiMatch->GetLink(iLink);  
		    Int_t iDigInd0=L0.GetIndex(); 
		    Int_t iDigInd1=(digiMatch->GetLink(iLink+1)).GetIndex(); 
		    if (iDigInd0 < fTofDigisColl->GetEntries() && iDigInd1 < fTofDigisColl->GetEntries()){
		      CbmTofDigiExp *pDig0 = (CbmTofDigiExp*) (fTofDigisColl->At(iDigInd0));
		      CbmTofDigiExp *pDig1 = (CbmTofDigiExp*) (fTofDigisColl->At(iDigInd1));
		      dTot += pDig0->GetTot(); 
		      dTot += pDig1->GetTot();
		    } 
		  }

		  CbmMatch* digiMatch2=(CbmMatch *)fTofDigiMatchColl->At(pHit2->GetRefId());
		  Double_t dTot2=0;
		  for (Int_t iLink=0; iLink<digiMatch2->GetNofLinks(); iLink+=2){  // loop over digis
		    CbmLink L0 = digiMatch2->GetLink(iLink);  
		    Int_t iDigInd0=L0.GetIndex(); 
		    Int_t iDigInd1=(digiMatch2->GetLink(iLink+1)).GetIndex(); 
		    if (iDigInd0 < fTofDigisColl->GetEntries() && iDigInd1 < fTofDigisColl->GetEntries()){
		      CbmTofDigiExp *pDig0 = (CbmTofDigiExp*) (fTofDigisColl->At(iDigInd0));
		      CbmTofDigiExp *pDig1 = (CbmTofDigiExp*) (fTofDigisColl->At(iDigInd1));
		      dTot2 += pDig0->GetTot(); 
		      dTot2 += pDig1->GetTot();
		      digiMatch->AddLink(CbmLink(pDig0->GetTot(),iDigInd0));
		      digiMatch->AddLink(CbmLink(pDig1->GetTot(),iDigInd1));
		    } 
		  }
		  LOG(DEBUG)<<"MergeClusters: Found merger in neighbour "
			    <<Form(" SmT %d, Sm %d, Rpc %d, Ch %d - hit %d(%d)",
				   iSmType2,iSm2,iRpc2,iCh2,iHitInd2,fTofHitsColl->GetEntries())
			    <<Form(" DX %6.1f, DY %6.1f, DT %6.1f",xPos-xPos2,yPos-yPos2,tof-tof2)
			    <<Form(" Tots %6.1f - %6.1f", dTot, dTot2) 
			    <<FairLogger::endl;
		  Double_t dTotSum=dTot+dTot2;
		  Double_t dxPosM = (xPos*dTot + xPos2*dTot2)/dTotSum;
		  Double_t dyPosM = (yPos*dTot + yPos2*dTot2)/dTotSum;
		  Double_t dtofM  = (tof*dTot  + tof2*dTot2) /dTotSum;
		  pHit->SetX(dxPosM);
		  pHit->SetY(dyPosM);
		  pHit->SetTime(dtofM);

		  // remove merged hit at iHitInd2 and update digiMatch
		  if( pHit2->GetRefId() != iHitInd2 )
		    LOG(ERROR)<<"CbmTofTestBeamClusterizer::MergeClusters: Inconsistent Links! Check!"
			      <<FairLogger::endl;

		  fTofDigiMatchColl->RemoveAt( pHit2->GetRefId() );
		  fTofDigiMatchColl->Compress();
		  fTofHitsColl->RemoveAt( iHitInd2 );
		  fTofHitsColl->Compress();
		  LOG(DEBUG)<<"MergeClusters: Compress TClonesArrays to "
			    <<fTofHitsColl->GetEntries()<<", "
			    <<fTofDigiMatchColl->GetEntries()
			    <<FairLogger::endl;
		  for(Int_t i=iHitInd2; i<fTofHitsColl->GetEntries(); i++){ // update RefLinks
 	            CbmTofHit *pHiti = (CbmTofHit*) fTofHitsColl->At( i );
		    pHiti->SetRefId(i);
		  }
		  //check merged hit (cluster)
		  //pHit->Print();
		}
	      }
	    }
	  }
       }
     }
  }
  return kTRUE;
}
