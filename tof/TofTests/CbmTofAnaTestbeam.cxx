// ------------------------------------------------------------------
// -----                     CbmTofAnaTestbeam                        -----
// -----              Created 12/04/2014 by nh                 -----
// ------------------------------------------------------------------

#include "CbmTofAnaTestbeam.h"

// TOF Classes and includes
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
#include "CbmTofTracklet.h"

#include "CbmTofTestBeamClusterizer.h"

#include "TTrbHeader.h"
#include "TMbsMappingTofPar.h"

// CBMroot classes and includes

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
#include "TH3.h"
#include "TProfile.h"
#include "TString.h"
#include "TFile.h"
#include "TMath.h"
#include "TRandom.h"
#include "TROOT.h"
#include "TDirectory.h"
#include "TGeoManager.h"

const Int_t DetMask = 4194303;
const Double_t DTDMAX=6000.;   // diamond inspection range in ps  

Double_t dTDia;
Double_t dDTD4Min=1.E8;

//___________________________________________________________________
//
// CbmTofAnaTestbeam
//
// Task for analysis of Testbeam data
//
// ------------------------------------------------------------------
CbmTofAnaTestbeam::CbmTofAnaTestbeam()
  : FairTask("HadronAnalysis"),
    fEvents(0),
    fGeoHandler(new CbmTofGeoHandler()),
    fTofId(NULL),
    fChannelInfo(NULL),
    fChannelInfoRef(NULL),
    fChannelInfoDut(NULL),
    fChannelInfoSel2(NULL),
    fMbsMappingPar(NULL),
    iNbSmTot(0),
    fvTypeSmOffs(),
    iNbRpcTot(0),
    fvSmRpcOffs(),
    iNbChTot(0),
    fvRpcChOffs(),
    fDigiPar(NULL),
    fDigiBdfPar(NULL),
    fTofDigisColl(NULL),
    fTofHitsColl(NULL),
    fTofDigiMatchColl(NULL),
    fTofTrackColl(NULL),
    fTrbHeader(NULL),
    fdDXMean(0.),
    fdDYMean(0.),
    fdDTMean(0.),
    fdDXWidth(0.),
    fdDYWidth(0.),
    fdDTWidth(0.),
    fhTriggerPattern(NULL),
    fhTriggerType(NULL),
    fhTimeInSpill(NULL),
    fhTIS_all(NULL),
    fhTIS_sel(NULL),
    fhTIS_sel1(NULL),
    fhTIS_sel2(NULL),
    fhDT2(NULL),
    fhXX2(NULL),
    fhYY2(NULL),
    fhNMatch04(NULL),
    fhXX04(NULL),
    fhYY04(NULL),
    fhXY04(NULL),
    fhYX04(NULL),
    fhTT04(NULL),
    fhChi04(NULL),
    fhChiSel24(NULL),
    fhDXSel24(NULL),
    fhDYSel24(NULL),
    fhDTSel24(NULL),
    fhDXDY04(NULL),
    fhDXDT04(NULL),
    fhDYDT04(NULL),
    fhTofSel24(NULL),
    fhNMatch04sel(NULL),
    fhChi04best(NULL),
    fhDigiMul0best(NULL),
    fhDigiMul4best(NULL),
    fhDXDY04best(NULL),
    fhDXDT04best(NULL),
    fhDYDT04best(NULL),
    fhChiDT04best(NULL),
    fhDT24DT04best(NULL),
    fhDTD4DT04best(NULL),
    fhX0DT04best(NULL), 
    fhY0DT04best(NULL),
    fhNMatchD4sel(NULL),
    fhChi04D4best(NULL),
    fhTofD4best(NULL),
    fhVelD4best(NULL),
    fhDigiMul0D4best(NULL),
    fhDigiMul4D4best(NULL),
    fhCluMul04D4best(NULL),
    fhDXDY04D4best(NULL),
    fhDXDT04D4best(NULL),
    fhDYDT04D4best(NULL),
    fhDistDT04D4best(NULL),
    fhTexpDT04D4best(NULL),
    fhCluSize0DT04D4best(NULL),
    fhCluSize4DT04D4best(NULL),
    fhTot0DT04D4best(NULL),
    fhTot4DT04D4best(NULL),
    fhChiDT04D4best(NULL),
    fhDT24DT04D4best(NULL),
    fhDTD4DT04D4best(NULL),
    fhX0DT04D4best(NULL),
    fhY0DT04D4best(NULL),
    fhTISDT04D4best(NULL),
    fhDTMul4D4best(NULL),
    fhDTX4D4best(NULL),
    fhDTY4D4best(NULL),
    fhDXX4D4best(NULL),
    fhDXY4D4best(NULL),
    fhDYX4D4best(NULL),
    fhDYY4D4best(NULL),
    fhDTMul0D4best(NULL),
    fhDTX0D4best(NULL),
    fhDTY0D4best(NULL),
    fhDXX0D4best(NULL),
    fhDXY0D4best(NULL),
    fhDYX0D4best(NULL),
    fhDYY0D4best(NULL),
    fhChi04D4sbest(NULL),
    fhTofD4sbest(NULL),
    fhVelD4sbest(NULL),
    fhDigiMul0D4sbest(NULL),
    fhDigiMul4D4sbest(NULL),
    fhCluMul04D4sbest(NULL),
    fhDXDY04D4sbest(NULL),
    fhDXDT04D4sbest(NULL),
    fhDYDT04D4sbest(NULL),
    fhDistDT04D4sbest(NULL),
    fhTexpDT04D4sbest(NULL),
    fhCluSize0DT04D4sbest(NULL),
    fhCluSize4DT04D4sbest(NULL),
    fhTot0DT04D4sbest(NULL),
    fhTot4DT04D4sbest(NULL),
    fhChiDT04D4sbest(NULL),
    fhDT24DT04D4sbest(NULL),
    fhDTD4DT04D4sbest(NULL),
    fhX0DT04D4sbest(NULL),
    fhY0DT04D4sbest(NULL),
    fhDTMul4D4sbest(NULL),
    fhDTX4D4sbest(NULL),
    fhDTY4D4sbest(NULL),
    fhDXX4D4sbest(NULL),
    fhDXY4D4sbest(NULL),
    fhDYX4D4sbest(NULL),
    fhDYY4D4sbest(NULL),
    fhDTMul0D4sbest(NULL),
    fhDTX0D4sbest(NULL),
    fhDTY0D4sbest(NULL),
    fhDXX0D4sbest(NULL),
    fhDXY0D4sbest(NULL),
    fhDYX0D4sbest(NULL),
    fhDYY0D4sbest(NULL),
    fhNMatch24(NULL),
    fhNMatch24sel(NULL),
    fhDT24sel(NULL),
    fhChi24(NULL),
    fhXY24(NULL),
    fhDXDY24(NULL),
    fhDXDT24(NULL),
    fhDYDT24(NULL),
    fhXY0D4best(NULL),
    fhXY4D4best(NULL),
    fhXX04D4best(NULL),
    fhYY04D4best(NULL),
    fhXYSel2D4best(NULL),
    fhXY0D4sel(NULL),
    fhXY4D4sel(NULL),
    fhXYSel2D4sel(NULL),
    fhDTD4sel(NULL),
    fhTofD4sel(NULL),
    fhBRefMul(NULL),
    fhDTD4(NULL),
    fhXYPos(),
    fhDT04DX0_1(NULL),
    fhDT04DY0_1(NULL),
    fhDT04DT0_1(NULL),
    fhDT04DX4_1(NULL),
    fhDT04DY4_1(NULL),
    fhDT04DT4_1(NULL),
    fhDT04DX0_2(NULL),
    fhDT04DY0_2(NULL),
    fhDT04DT0_2(NULL),
    fhDT04DX4_2(NULL),
    fhDT04DY4_2(NULL),
    fhDT04DT4_2(NULL),
    fhDutPullX(NULL),
    fhDutPullY(NULL),
    fhDutPullZ(NULL),
    fhDutPullT(NULL),
    fhDutPullTB(NULL),
    fhDutChiFound(NULL),
    fhDutChiMissed(NULL),
    fhDutChiMatch(NULL),
    fhDutXY_Found(NULL),     
    fhDutXY_Missed(NULL), 
    fhDutXYDT(NULL),
    fStart(),
    fStop(),
    fCalParFileName(""),
    fCalParFile(NULL),
    fhDTD4DT04D4Off(NULL),
    fhDTX4D4Off(NULL),
    fhDTY4D4Off(NULL),
    fhDTTexpD4Off(NULL),
    fdMulDMax(0.),
    fdDTDia(0.),
    fdDTD4MAX(0.),
    fdMul0Max(0.),
    fdMul4Max(0.),
    fdCh4Sel(0.),
    fdDCh4Sel(0.),
    fdPosY4Sel(0.),
    fdPosY4SelOff(0.),
    fdChS2Sel(0.),
    fdDChS2Sel(0.),
    fdPosYS2Sel(0.),
    fdPosYS2SelOff(0.),
    fdSel2TOff(0.),
    fdHitDistMin(0.),
    fdTOffD4(0.),
    fdTShift(0.),
    fdChi2Lim(0.),
    fiCorMode(0),
    fiDutAddr(0),
    fiMrpcRefAddr(0),
    fiMrpcSel2Addr(0),
    fiMrpcSel3Addr(0),
    fiBeamRefAddr(0),
    fiDut(0),
    fiDutSm(0),
    fiDutRpc(0),
    fiMrpcRef(0),
    fiMrpcRefSm(0),
    fiMrpcRefRpc(0),
    fiMrpcSel2(0),
    fiMrpcSel2Sm(0),
    fiMrpcSel2Rpc(0),
    fiMrpcSel3(0),
    fiMrpcSel3Sm(0),
    fiMrpcSel3Rpc(0),
    fiPlaSelect(0),
    fiBeamRefSmType(0),
    fiBeamRefSmId(0),
    fiDutNch(0),
    fiReqTrg(-1),
    fSIGLIM(3.),
    fSIGT(100.),
    fSIGX(1.),
    fSIGY(1.),
    fFindTracks(NULL)
{
  cout << "CbmTofTests: Task started " << endl;
}
// ------------------------------------------------------------------

// ------------------------------------------------------------------
CbmTofAnaTestbeam::CbmTofAnaTestbeam(const char* name, Int_t verbose)
  : FairTask(name, verbose),
    fEvents(0),
    fGeoHandler(new CbmTofGeoHandler()),
    fTofId(NULL),
    fChannelInfo(NULL),
    fChannelInfoRef(NULL),
    fChannelInfoDut(NULL),
    fChannelInfoSel2(NULL),
    fMbsMappingPar(NULL),
    iNbSmTot(0),
    fvTypeSmOffs(),
    iNbRpcTot(0),
    fvSmRpcOffs(),
    iNbChTot(0),
    fvRpcChOffs(),
    fDigiPar(NULL),
    fDigiBdfPar(NULL),
    fTofDigisColl(NULL),
    fTofHitsColl(NULL),
    fTofDigiMatchColl(NULL),
    fTofTrackColl(NULL),
    fTrbHeader(NULL),
    fdDXMean(0.),
    fdDYMean(0.),
    fdDTMean(0.),
    fdDXWidth(0.),
    fdDYWidth(0.),
    fdDTWidth(0.),
    fhTriggerPattern(NULL),
    fhTriggerType(NULL),
    fhTimeInSpill(NULL),
    fhTIS_all(NULL),
    fhTIS_sel(NULL),
    fhTIS_sel1(NULL),
    fhTIS_sel2(NULL),
    fhDT2(NULL),
    fhXX2(NULL),
    fhYY2(NULL),
    fhNMatch04(NULL),
    fhXX04(NULL),
    fhYY04(NULL),
    fhXY04(NULL),
    fhYX04(NULL),
    fhTT04(NULL),
    fhChi04(NULL),
    fhChiSel24(NULL),
    fhDXSel24(NULL),
    fhDYSel24(NULL),
    fhDTSel24(NULL),
    fhDXDY04(NULL),
    fhDXDT04(NULL),
    fhDYDT04(NULL),
    fhTofSel24(NULL),
    fhNMatch04sel(NULL),
    fhChi04best(NULL),
    fhDigiMul0best(NULL),
    fhDigiMul4best(NULL),
    fhDXDY04best(NULL),
    fhDXDT04best(NULL),
    fhDYDT04best(NULL),
    fhChiDT04best(NULL),
    fhDT24DT04best(NULL),
    fhDTD4DT04best(NULL),
    fhX0DT04best(NULL),
    fhY0DT04best(NULL),
    fhNMatchD4sel(NULL),
    fhChi04D4best(NULL),
    fhTofD4best(NULL),
    fhVelD4best(NULL),
    fhDigiMul0D4best(NULL),
    fhDigiMul4D4best(NULL),
    fhCluMul04D4best(NULL),
    fhDXDY04D4best(NULL),
    fhDXDT04D4best(NULL),
    fhDYDT04D4best(NULL),
    fhDistDT04D4best(NULL),
    fhTexpDT04D4best(NULL),
    fhCluSize0DT04D4best(NULL),
    fhCluSize4DT04D4best(NULL),
    fhTot0DT04D4best(NULL),
    fhTot4DT04D4best(NULL),
    fhChiDT04D4best(NULL),
    fhDT24DT04D4best(NULL),
    fhDTD4DT04D4best(NULL),
    fhX0DT04D4best(NULL),
    fhY0DT04D4best(NULL),
    fhTISDT04D4best(NULL),
    fhDTMul4D4best(NULL),
    fhDTX4D4best(NULL),
    fhDTY4D4best(NULL),
    fhDXX4D4best(NULL),
    fhDXY4D4best(NULL),
    fhDYX4D4best(NULL),
    fhDYY4D4best(NULL),
    fhDTMul0D4best(NULL),
    fhDTX0D4best(NULL),
    fhDTY0D4best(NULL),
    fhDXX0D4best(NULL),
    fhDXY0D4best(NULL),
    fhDYX0D4best(NULL),
    fhDYY0D4best(NULL),
    fhChi04D4sbest(NULL),
    fhTofD4sbest(NULL),
    fhVelD4sbest(NULL),
    fhDigiMul0D4sbest(NULL),
    fhDigiMul4D4sbest(NULL),
    fhCluMul04D4sbest(NULL),
    fhDXDY04D4sbest(NULL),
    fhDXDT04D4sbest(NULL),
    fhDYDT04D4sbest(NULL),
    fhDistDT04D4sbest(NULL),
    fhTexpDT04D4sbest(NULL),
    fhCluSize0DT04D4sbest(NULL),
    fhCluSize4DT04D4sbest(NULL),
    fhTot0DT04D4sbest(NULL),
    fhTot4DT04D4sbest(NULL),
    fhChiDT04D4sbest(NULL),
    fhDT24DT04D4sbest(NULL),
    fhDTD4DT04D4sbest(NULL),
    fhX0DT04D4sbest(NULL),
    fhY0DT04D4sbest(NULL),
    fhDTMul4D4sbest(NULL),
    fhDTX4D4sbest(NULL),
    fhDTY4D4sbest(NULL),
    fhDXX4D4sbest(NULL),
    fhDXY4D4sbest(NULL),
    fhDYX4D4sbest(NULL),
    fhDYY4D4sbest(NULL),
    fhDTMul0D4sbest(NULL),
    fhDTX0D4sbest(NULL),
    fhDTY0D4sbest(NULL),
    fhDXX0D4sbest(NULL),
    fhDXY0D4sbest(NULL),
    fhDYX0D4sbest(NULL),
    fhDYY0D4sbest(NULL),
    fhNMatch24(NULL),
    fhNMatch24sel(NULL),
    fhDT24sel(NULL),
    fhChi24(NULL),
    fhXY24(NULL),
    fhDXDY24(NULL),
    fhDXDT24(NULL),
    fhDYDT24(NULL),
    fhXY0D4best(NULL),
    fhXY4D4best(NULL),
    fhXX04D4best(NULL),
    fhYY04D4best(NULL),
    fhXYSel2D4best(NULL),
    fhXY0D4sel(NULL),
    fhXY4D4sel(NULL),
    fhXYSel2D4sel(NULL),
    fhDTD4sel(NULL),
    fhTofD4sel(NULL),
    fhBRefMul(NULL),
    fhDTD4(NULL),
    fhXYPos(),
    fhDT04DX0_1(NULL),
    fhDT04DY0_1(NULL),
    fhDT04DT0_1(NULL),
    fhDT04DX4_1(NULL),
    fhDT04DY4_1(NULL),
    fhDT04DT4_1(NULL),
    fhDT04DX0_2(NULL),
    fhDT04DY0_2(NULL),
    fhDT04DT0_2(NULL),
    fhDT04DX4_2(NULL),
    fhDT04DY4_2(NULL),
    fhDT04DT4_2(NULL),
    fhDutPullX(NULL),
    fhDutPullY(NULL),
    fhDutPullZ(NULL),
    fhDutPullT(NULL),
    fhDutPullTB(NULL), 
    fhDutChiFound(NULL),
    fhDutChiMissed(NULL),
    fhDutChiMatch(NULL),
    fhDutXY_Found(NULL),     
    fhDutXY_Missed(NULL), 
    fhDutXYDT(NULL),
    fStart(),
    fStop(),
    fCalParFileName(""),
    fCalParFile(NULL),
    fhDTD4DT04D4Off(NULL),
    fhDTX4D4Off(NULL),
    fhDTY4D4Off(NULL),
    fhDTTexpD4Off(NULL),
    fdMulDMax(0.),
    fdDTDia(0.),
    fdDTD4MAX(0.),
    fdMul0Max(0.),
    fdMul4Max(0.),
    fdCh4Sel(0.),
    fdDCh4Sel(0.),
    fdPosY4Sel(0.),
    fdPosY4SelOff(0.),
    fdChS2Sel(0.),
    fdDChS2Sel(0.),
    fdPosYS2Sel(0.),
    fdPosYS2SelOff(0.),
    fdSel2TOff(0.),
    fdHitDistMin(0.),
    fdTOffD4(0.),
    fdTShift(0.),
    fdChi2Lim(0.),
    fiCorMode(0),
    fiDutAddr(0),
    fiMrpcRefAddr(0),
    fiMrpcSel2Addr(0),
    fiMrpcSel3Addr(0),
    fiBeamRefAddr(0),
    fiDut(0),
    fiDutSm(0),
    fiDutRpc(0),
    fiMrpcRef(0),
    fiMrpcRefSm(0),
    fiMrpcRefRpc(0),
    fiMrpcSel2(0),
    fiMrpcSel2Sm(0),
    fiMrpcSel2Rpc(0),
    fiMrpcSel3(0),
    fiMrpcSel3Sm(0),
    fiMrpcSel3Rpc(0),
    fiPlaSelect(0),
    fiBeamRefSmType(0),
    fiBeamRefSmId(0),
    fiDutNch(0),
    fiReqTrg(-1),
    fSIGLIM(3.),
    fSIGT(100.),
    fSIGX(1.),
    fSIGY(1.),
    fFindTracks(NULL)
{
}
// ------------------------------------------------------------------

// ------------------------------------------------------------------
CbmTofAnaTestbeam::~CbmTofAnaTestbeam()
{
    // Destructor
}
// ------------------------------------------------------------------
/************************************************************************************/
// FairTasks inherited functions
InitStatus CbmTofAnaTestbeam::Init()
{
   if( kFALSE == RegisterInputs() )
      return kFATAL;

   //   fTofId = new ( CbmTofDetectorId )CbmTofDetectorId_v14a();
   if( kFALSE == InitParameters() )
      return kFATAL;

   if( kFALSE == LoadGeometry() )
      return kFATAL;

   if( kFALSE == LoadCalParameter() )
      return kFATAL;

   if( kFALSE == CreateHistos() )
      return kFATAL;

  fFindTracks = CbmTofFindTracks::Instance();
  if (NULL == fFindTracks) 
  LOG(WARNING) << Form("CbmTofAnaTestbeam::Init : no FindTracks instance found")
	     << FairLogger::endl;

   return kSUCCESS;
}

void CbmTofAnaTestbeam::SetParContainers()
{
   LOG(INFO)<<" CbmTofAnaTestbeam => Get the digi parameters for tof"<<FairLogger::endl;
   return;
   // Get Base Container
   FairRunAna* ana = FairRunAna::Instance();
   FairRuntimeDb* rtdb=ana->GetRuntimeDb();

   fDigiPar = (CbmTofDigiPar*) (rtdb->getContainer("CbmTofDigiPar"));

   fDigiBdfPar = (CbmTofDigiBdfPar*) (rtdb->getContainer("CbmTofDigiBdfPar"));
}

Bool_t   CbmTofAnaTestbeam::LoadGeometry()
{
   LOG(INFO)<<"CbmTofAnaTestbeam::LoadGeometry starting for  "
            <<fDigiPar->GetNrOfModules() << " geometrically known detector cells, looking for type " << fiDut
            <<FairLogger::endl;
   fiDutNch=0;
   Double_t dDutXmin=1.E300;
   Double_t dDutXmax=-1.E300;
   fChannelInfoDut=NULL;
   for (Int_t icell = 0; icell < fDigiPar->GetNrOfModules(); ++icell) {

     Int_t cellId = fDigiPar->GetCellId(icell); // cellId is assigned in CbmTofCreateDigiPar
     fChannelInfo = fDigiPar->GetCell(cellId);
     /*
     Int_t smodule  = fGeoHandler->GetSMType(cellId);   // FIXME  - wrong inline functions!!!
     Int_t smtype   = fGeoHandler->GetSModule(cellId);  // FIXME
     Int_t module  = fGeoHandler->GetCounter(cellId);
     */
     Int_t smtype   = CbmTofAddress::GetSmType( cellId );
     Int_t smodule  = CbmTofAddress::GetSmId( cellId );
     Int_t module   = CbmTofAddress::GetRpcId( cellId );

     LOG(DEBUG1) <<Form(" Id 0x%08x ",cellId) 
		<< "  got cell " << smtype << ", " << smodule << ", " << module 
		<< ", x-size "<< fChannelInfo->GetSizex() 
		<< ", y-size "<< fChannelInfo->GetSizey()
		<< FairLogger::endl;
     if( smtype == fiDut && smodule == fiDutSm && module == fiDutRpc){
       fiDutNch++;
       if(fChannelInfo->GetX() > dDutXmax) dDutXmax=fChannelInfo->GetX();
       if(fChannelInfo->GetX() < dDutXmin) dDutXmin=fChannelInfo->GetX();
       if(fiDutNch == 1) fChannelInfoDut = fChannelInfo; 
       LOG(DEBUG) <<Form(" DutId 0x%08x ",cellId) 
		  << "  got cell " << smtype << ", " << smodule << ", " << module 
		  << ", x-size "<< fChannelInfo->GetSizex() 
		  << ", y-size "<< fChannelInfo->GetSizey()
		  <<", # "<<fiDutNch<<", Xmin "<<dDutXmin<<", Xmax "<<dDutXmax
		  << FairLogger::endl;
     }
   }
   if (fiDutNch > 0){
     Double_t dDutDx = (dDutXmax - dDutXmin)/(fiDutNch-1);
     LOG(INFO)<<"CbmTofAnaTestbeam::LoadGeometry Dut = "
	      << fiDut << " with " << fiDutNch << " channels in x- direction from "
	      << dDutXmin << " to " << dDutXmax 
	      <<", dx = "<< dDutDx
	      <<FairLogger::endl;
   return kTRUE;
   } else {
     LOG(ERROR) <<"CbmTofAnaTestbeam::LoadGeometry Dut inconsistent "<<fiDut<<", "<<fiDutNch
		<<FairLogger::endl;
     return kFALSE;
   }
}

void CbmTofAnaTestbeam::Exec(Option_t * /*option*/)
{
   // Task execution

   LOG(DEBUG)<<" CbmTofAnaTestbeam => New event"<<FairLogger::endl;

   fStart.Set();
   FillHistos();
   fStop.Set();


   if( 0 == ( fEvents%100000 ) && 0 < fEvents )
   {
      cout << "-I- CbmTofAnaTestbeam::Exec : "
           << "event " << fEvents << " processed." << endl;
   }
   fEvents += 1;
}

void CbmTofAnaTestbeam::Finish()
{
   // Normalisations
   cout << "CbmTofAnaTestbeam::Finish up with " << fEvents << " analyzed events " << endl;

   WriteHistos();
   // Prevent them from being sucked in by the CbmHadronAnalysis WriteHistograms method
   DeleteHistos();
}

/************************************************************************************/
// Functions common for all clusters approximations
Bool_t   CbmTofAnaTestbeam::RegisterInputs()
{
   FairRootManager *fManager = FairRootManager::Instance();

   fTofDigisColl   = (TClonesArray *) fManager->GetObject("CbmTofDigiExp");

   if( NULL == fTofDigisColl)
      fTofDigisColl = (TClonesArray *) fManager->GetObject("CbmTofDigi");

   if( NULL == fTofDigisColl)
      fTofDigisColl = (TClonesArray *) fManager->GetObject("TofDigi");

   if( NULL == fTofDigisColl)
   {
      LOG(ERROR)<<"CbmTofAnaTestbeam::RegisterInputs => Could not get the TofDigi TClonesArray!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fTofDigisColl)

   fTofHitsColl   = (TClonesArray *) fManager->GetObject("TofHit");
   if( NULL == fTofHitsColl)
   {
      LOG(ERROR)<<"CbmTofAnaTestbeam::RegisterInputs => Could not get the TofHit TClonesArray!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fTofHitsColl)

   fTofDigiMatchColl= (TClonesArray *) fManager->GetObject("TofDigiMatch");
   if( NULL == fTofDigiMatchColl)
   {
      LOG(ERROR)<<"CbmTofAnaTestbeam::RegisterInputs => Could not get the Match TClonesArray!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fTofDigiMatchColl)

   fTofTrackColl   = (TClonesArray *) fManager->GetObject("TofTracks");
   if( NULL == fTofTrackColl)
   {
      LOG(INFO)<<"CbmTofAnaTestbeam::RegisterInputs => Could not get the TofTracklet TClonesArray!!!"<<FairLogger::endl;
     //      return kFALSE;
   } // if( NULL == fTofHitsColl)

   fTrbHeader = (TTrbHeader *)  fManager->GetObject("TofTrbHeader");
   if( NULL == fTrbHeader)
   {
      LOG(INFO)<<"CbmTofAnaTestbeam::RegisterInputs => Could not get the TofTrbHeader Object"<<FairLogger::endl;
   }
   return kTRUE;
}
/************************************************************************************/
Bool_t   CbmTofAnaTestbeam::InitParameters()
{

   // Initialize the TOF GeoHandler
   Bool_t isSimulation=kFALSE;
   LOG(INFO)<<"CbmTofAnaTestbeam::InitParameters - Geometry, Mapping, ... "
             <<FairLogger::endl;
   // Get Base Container
   FairRun* ana = FairRun::Instance();
   FairRuntimeDb* rtdb=ana->GetRuntimeDb();   

   Int_t iGeoVersion = fGeoHandler->Init(isSimulation);
   if( k14a > iGeoVersion )
   {

   }

   fDigiPar = (CbmTofDigiPar*) (rtdb->getContainer("CbmTofDigiPar"));
   if( 0 == fDigiPar )
   {
      LOG(ERROR)<<"CbmTofAnaTestbeam::InitParameters => Could not obtain the CbmTofDigiPar "<<FairLogger::endl;
      return kFALSE; 
   }

   fDigiBdfPar = (CbmTofDigiBdfPar*) (rtdb->getContainer("CbmTofDigiBdfPar"));
   if( 0 == fDigiBdfPar )
   {
      LOG(ERROR)<<"CbmTofAnaTestbeam::InitParameters => Could not obtain the CbmTofDigiBdfPar "<<FairLogger::endl;
      return kFALSE; 
   }

   // Mapping parameter
   fMbsMappingPar = (TMbsMappingTofPar*) (rtdb->getContainer("TMbsMappingTofPar"));
   if( 0 == fMbsMappingPar )
   {
      LOG(ERROR)<<"CbmTofAnaTestBeam::InitParameters => Could not obtain the TMbsMappingTofPar "<<FairLogger::endl;
      return kFALSE; 
   }

   rtdb->initContainers(  ana->GetRunId() );

   LOG(INFO)<<"CbmTofAnaTestbeam::InitParameter: currently " 
            << fDigiPar->GetNrOfModules() << " digi cells " <<FairLogger::endl;

   // set defaults for backward compatibility to sep14
   if(0 == fiBeamRefSmType) {
   LOG(INFO)<<"CbmTofAnaTestbeam::InitParameter: set beam ref to default (sep14) "
	    <<FairLogger::endl;
     fiBeamRefSmType=5;
     fiBeamRefSmId = 1;
   }
   if( 0 == fdDTD4MAX) fdDTD4MAX=DTDMAX;   

   if ( 0. == fdChi2Lim ) fdChi2Lim = 10.;
   return kTRUE;
}
/************************************************************************************/
Bool_t   CbmTofAnaTestbeam::LoadCalParameter()
{
    if(fCalParFileName.IsNull()) return kTRUE;

    fCalParFile = new TFile(fCalParFileName,"");
    if(NULL == fCalParFile) {
      LOG(ERROR) << "CbmTofAnaTestBeam::LoadCalParameter: "
		 << "file " << fCalParFileName << " does not exist!" << FairLogger::endl;
      return kTRUE;
    }

    TProfile *fhtmp=(TProfile *) gDirectory->FindObjectAny( Form("hDTD4DT04D4best_pfx_px"));
    if (NULL == fhtmp) {
      LOG(INFO)<<" Histo " << Form("hDTD4DT04D4best_pfx_px") << " not found. "
             <<FairLogger::endl;
    }

    TProfile *fhtmpx=(TProfile *) gDirectory->FindObjectAny( Form("hDTX4D4best_pfx_px"));
    if (NULL == fhtmpx) {
      LOG(INFO)<<" Histo " << Form("hDTX4D4best_pfx_px") << " not found. "
             <<FairLogger::endl;
    }

    TProfile *fhtmpy=(TProfile *) gDirectory->FindObjectAny( Form("hDTY4D4best_pfx_px"));
    if (NULL == fhtmpy) {
      LOG(INFO)<<" Histo " << Form("hDTY4D4best_pfx_px") << " not found. "
             <<FairLogger::endl;
    }

    TProfile *fhtmpt=(TProfile *) gDirectory->FindObjectAny( Form("hTexpDT04D4best_pfx_px"));
    if (NULL == fhtmpt) {
      LOG(INFO)<<" Histo " << Form("hTexpDT04D4best_pfx_px") << " not found. "
             <<FairLogger::endl;
    }

    gROOT->cd();
    if(NULL != fhtmp)  fhDTD4DT04D4Off=(TH1D *)fhtmp->Clone();
    if(NULL != fhtmpx) fhDTX4D4Off=(TH1D *)fhtmpx->Clone();
    if(NULL != fhtmpy) fhDTY4D4Off=(TH1D *)fhtmpy->Clone();
    if(NULL != fhtmpt) fhDTTexpD4Off=(TH1D *)fhtmpt->Clone();

    fCalParFile->Close();
    //    fhDTD4DT04D4Off->Draw();
    
    if (fdDCh4Sel==0.)  fdDCh4Sel=1000.;  // open default window
    if (fdPosY4Sel==0.) fdPosY4Sel=10.;   // open default window

    return kTRUE;
}
/************************************************************************************/
// ------------------------------------------------------------------
Bool_t CbmTofAnaTestbeam::CreateHistos()
{
  // Create histogramms
   Double_t XDMAX=10.;
   Double_t YDMAX=80.;
   Double_t TDMAX=1000000.;
   Double_t DTMAX=1000.;
   Double_t DXMAX=10.;
   Double_t DYMAX=10.;
/*   Double_t XMAX=100.;*/
/*   Double_t YMAX=100.;*/

   TDirectory * oldir = gDirectory; // <= To prevent histos from being sucked in by the param file of the TRootManager!
   gROOT->cd(); // <= To prevent histos from being sucked in by the param file of the TRootManager !

   // define histos here
   Double_t TISmax = 1.E9;
   Double_t TISnbins = 50.;
   fhTriggerPattern = new TH1I("tof_trb_trigger_pattern", "CTS trigger pattern", 16, 0, 16);
   fhTriggerType = new TH1I("tof_trb_trigger_types", "CTS trigger types", 16, 0, 16);
   fhTimeInSpill = new TH1I("tof_trb_time_in_spill", "Time in Spill", TISnbins, 0, TISmax);
   fhTIS_all  = new TH1F("TIS_all", "Time in Spill (all)", TISnbins, 0, TISmax);
   fhTIS_sel  = new TH1F("TIS_sel", "Time in Spill (sel)", TISnbins, 0, TISmax);
   fhTIS_sel1 = new TH1F("TIS_sel1", "Time in Spill (sel1)", TISnbins, 0, TISmax);
   fhTIS_sel2 = new TH1F("TIS_sel2", "Time in Spill (sel2)", TISnbins, 0, TISmax);

   fhBRefMul =  new TH1F( Form("hBRefMul"),Form("Multiplicity in Beam Reference counter ; Mul ()"),
			  50, 0., 50.); 
   fhDTD4    =  new TH1F( Form("hDTD4"),Form("reference time ; #Delta tD4 (ps)"),
			  100, -100000., 100000.); 

   Int_t iNbDet=fMbsMappingPar->GetNbMappedDet();
   fhXYPos.resize( iNbDet  );
   for(Int_t iDet=0; iDet<iNbDet; iDet++){
     fhXYPos[iDet] =  new TH2F( Form("hXY_SmT%d",iDet),
				Form("XY Position correlation of Det# %d; X[cm]; Y [cm]",iDet),
			       100, -YDMAX, YDMAX, 100, -YDMAX, YDMAX);       
   }

   
    fhXX2 =  new TH2F( 
          Form("hXX2"),
          Form("Plastic position correlation; XPla0; XPla1"),
          100, -XDMAX, XDMAX, 100, -XDMAX, XDMAX);
    fhYY2 =  new TH2F( 
          Form("hYY2"),
          Form("Plastic position correlation; YPla0; YPla1"),
          100, -YDMAX, YDMAX, 100, -YDMAX, YDMAX);
    fhDT2 =  new TH1F( 
          Form("hDT2"),
          Form("Plastic time difference; TPla0 - TPla1 (ps)"),
          100, -1000., 1000.);
 
   for (Int_t iDet=0; iDet<2; iDet++)
   {
     fhXX02[iDet] =  new TH2F( Form("hXX02_%d",iDet),Form("X Position correlation; X0; X2"),
			       100, -YDMAX, YDMAX, 100, -YDMAX, YDMAX);      
     fhYY02[iDet] =  new TH2F( Form("hYY02_%d",iDet),Form("Y Position correlation; Y0; Y2"),
			       100, -YDMAX, YDMAX, 100, -YDMAX, YDMAX); 
   }
   

     fhXX04 =  new TH2F( Form("hXX04"),Form("X Position correlation; X0 [cm]; X4 [cm]"),
			       500, -YDMAX, YDMAX, 500, -YDMAX, YDMAX);      
     fhYY04 =  new TH2F( Form("hYY04"),Form("Y Position correlation; Y0 [cm]; Y4 [cm]"),
			       500, -YDMAX, YDMAX, 500, -YDMAX, YDMAX); 
     fhXY04 =  new TH2F( Form("hXY04"),Form("X Position correlation; X0 [cm]; Y4 [cm]"),
			       500, -YDMAX, YDMAX, 500, -YDMAX, YDMAX);      
     fhYX04 =  new TH2F( Form("hYX04"),Form("Y Position correlation; Y0 [cm]; X4 [cm]"),
			       500, -YDMAX, YDMAX, 500, -YDMAX, YDMAX); 
     fhTT04 =  new TH2F( Form("hTT04"),Form("Time  correlation; T0 [ps]; T0 - T4 [ps]"),
			       100, -TDMAX, TDMAX, 100, -1000., 1000.); 
     fhDXDY04 = new TH2F( Form("hDXDY04"),Form("position correlation; #Delta x [cm]; #DeltaY [cm]"),
			       100, -DXMAX, DXMAX, 100, -DYMAX, DYMAX); 
     fhDXDT04 = new TH2F( Form("hDXDT04"),Form("Time - position correlation; #Delta x [cm]; #DeltaT [ps]"),
			       100, -DXMAX, DXMAX, 100, -DTMAX, DTMAX); 
     fhDYDT04 = new TH2F( Form("hDYDT04"),Form("Time - position correlation; #Delta y [cm]; #DeltaT [ps]"),
			       100, -DYMAX, DYMAX, 100, -DTMAX, DTMAX); 
     fhChi04 =  new TH1F( Form("hChi04"),Form("Matching Chi2; #chi; Nhits"),
			  100, 0., fdChi2Lim); 
     fhChiSel24 =  new TH1F( Form("hChiSel24"),Form("Matching Chi2S24; #chi; Nhits"),
			  100, 0., fdChi2Lim); 
     fhDXSel24 =  new TH1F( Form("hDXSel24"),Form("Matching Sel24; #Delta x [cm]; Nhits"),
			  100, -10., 10.); 
     fhDYSel24 =  new TH1F( Form("hDYSel24"),Form("Matching Sel24; #Delta y [cm]; Nhits"),
			  100, -10., 10.); 
     fhDTSel24 =  new TH1F( Form("hDTSel24"),Form("Matching Sel24; #Delta t_{cor} [ps]; Nhits"),
			  100, -1000., 1000.); 
     fhTofSel24 =  new TH1F( Form("hTofSel24"),Form("Matching Sel24; #Delta t [ps]; Nhits"),
			  100, -5000., 5000.); 
     Int_t iNbinXY=68;
     fhXY0D4best = new TH2F( Form("hXY0D4best"),Form("local position 0;  x [cm]; y [cm]"), iNbinXY, -17., 17., iNbinXY, -17., 17.);
     fhXY4D4best = new TH2F( Form("hXY4D4best"),Form("local position 4;  x [cm]; y [cm]"), iNbinXY, -17., 17., iNbinXY, -17., 17.);
     fhXX04D4best = new TH2F( Form("hXX04D4best"),Form("local x position 0-4; x0 [cm]; x4 [cm]"), iNbinXY, -17., 17., iNbinXY, -17., 17.);
     fhYY04D4best = new TH2F( Form("hYY04D4best"),Form("local y position 0-4; y0 [cm]; y4 [cm]"), iNbinXY, -17., 17., iNbinXY, -17., 17.);
     fhXYSel2D4best = new TH2F( Form("hXYSel2D4best"),Form("local position Sel2;  x [cm]; y [cm]"), iNbinXY, -17., 17., iNbinXY, -17., 17.);
     fhXY0D4sel  = new TH2F( Form("hXY0D4sel"),Form("predicted position 0;  x [cm]; y [cm]"), iNbinXY, -17., 17., iNbinXY, -17., 17.);
     fhXY4D4sel  = new TH2F( Form("hXY4D4sel"),Form("selected position 4;  x [cm]; y [cm]"), iNbinXY, -17., 17., iNbinXY, -17., 17.);
     fhXYSel2D4sel  = new TH2F( Form("hXYSel2D4sel"),Form("selected position Sel2;  x [cm]; y [cm]"), iNbinXY, -17., 17., iNbinXY, -17., 17.);

     fhDXDY04best = new TH2F( Form("hDXDY04best"),Form("position correlation; #Delta x [cm]; #DeltaY [cm]"),
			       100, -DXMAX, DXMAX, 100, -DYMAX, DYMAX); 
     fhDXDT04best = new TH2F( Form("hDXDT04best"),Form("time - position correlation; #Delta x [cm]; #DeltaT [ps]"),
			       100, -DXMAX, DXMAX, 100, -DTMAX, DTMAX); 
     fhDYDT04best = new TH2F( Form("hDYDT04best"),Form("time - position correlation; #Delta y [cm]; #DeltaT [ps]"),
			       100, -DYMAX, DYMAX, 100, -DTMAX, DTMAX); 

     fhX0DT04best = new TH2F( Form("hX0DT04best"),Form("time - position correlation; #Delta x [cm]; #DeltaT [ps]"),
			       100, -50., 50., 100, -DTMAX, DTMAX); 
     fhY0DT04best = new TH2F( Form("hY0DT04best"),Form("time - position correlation; #Delta y [cm]; #DeltaT [ps]"),
			       100, -50., 50., 100, -DTMAX, DTMAX); 

     fhChi04best  =  new TH1F( Form("hChi04best"),Form("matching chi2; #chi; Nhits"),
			  100, 0., fdChi2Lim);      
     fhDigiMul0best =  new TH1F( Form("hDigiMul0best"),Form("Number of digis in cluster; N_{digi}; "),
			  20, 0., 20.); 
     fhDigiMul4best =  new TH1F( Form("hDigiMul4best"),Form("Number of digis in cluster; N_{digi}; "),
			  20, 0., 20.); 

     fhChiDT04best = new TH2F( Form("hChiDT04best"),Form("Time - Chi correlation; #chi; #DeltaT [ps]"),
			       100, 0., 100., 100, -DTMAX, DTMAX);

     fhDXDY04D4best = new TH2F( Form("hDXDY04D4best"),Form("position correlation; #Delta x [cm]; #DeltaY [cm]"),
			       100, -DXMAX, DXMAX, 100, -DYMAX, DYMAX); 
     fhDXDT04D4best = new TH2F( Form("hDXDT04D4best"),Form("time - position correlation; #Delta x [cm]; #DeltaT [ps]"),
			       100, -DXMAX, DXMAX, 100, -DTMAX, DTMAX); 
     fhDYDT04D4best = new TH2F( Form("hDYDT04D4best"),Form("time - position correlation; #Delta y [cm]; #DeltaT [ps]"),
			       100, -DYMAX, DYMAX, 100, -DTMAX, DTMAX); 
     fhDistDT04D4best = new TH2F( Form("hDistDT04D4best"),Form("time - distance correlation; Dist [cm]; #DeltaT [ps]"),
			       100, fdHitDistMin, fdHitDistMin + 5., 100, -DTMAX, DTMAX); 
     fhTexpDT04D4best = new TH2F( Form("hTexpDT04D4best"),Form("measured - expected time - correlation; Texp [ps]; #DeltaT [ps]"),
			       100, 0., 6000., 100, -DTMAX, DTMAX); 
     fhCluSize0DT04D4best = new TH2F( Form("hCluSize0DT04D4best"),
	 			     Form("time - CluSize correlation; N_{strips} ; #DeltaT [ps]"),
				     16, 0.5, 16.5, 100, -DTMAX, DTMAX); 
     fhCluSize4DT04D4best = new TH2F( Form("hCluSize4DT04D4best"),
	 			     Form("time - CluSize correlation; N_{strips} ; #DeltaT [ps]"),
				     16, 0.5, 16.5, 100, -DTMAX, DTMAX);
     fhTot0DT04D4best = new TH2F( Form("hTot0DT04D4best"),
	 			     Form("time - Tot correlation; TOT0 ; #DeltaT [ps]"),
				     100, 0., 10000., 100, -DTMAX, DTMAX);  
     fhTot4DT04D4best = new TH2F( Form("hTot4DT04D4best"),
	 			     Form("time - Tot correlation; TOT4 ; #DeltaT [ps]"),
				     100, 0., 10000., 100, -DTMAX, DTMAX);  

     fhX0DT04D4best = new TH2F( Form("hX0DT04D4best"),Form("time - position correlation; #Delta x [cm]; #DeltaT [ps]"),
			       100, -50., 50., 100, -DTMAX, DTMAX); 
     fhY0DT04D4best = new TH2F( Form("hY0DT04D4best"),Form("time - position correlation; #Delta y [cm]; #DeltaT [ps]"),
			       100, -50., 50., 100, -DTMAX, DTMAX); 

     fhTISDT04D4best = new TH2F( Form("hTISDT04D4best"),Form("time - TIS; time in spill  [ns]; #DeltaT [ps]"),
			       TISnbins, 0., TISmax, 100, -DTMAX, DTMAX); 

     fhChi04D4best  =  new TH1F( Form("hChi04D4best"),Form("matching chi2; #chi; Nhits"),
			  100, 0., fdChi2Lim); 
     fhTofD4best  =  new TH1F( Form("hTofD4best"),Form("tof D4; t [ps]; Counts"),
			  100, 0., 50000.); 
     fhVelD4best  =  new TH1F( Form("hVelD4best"),Form("vel D4; v [cm/ns]; Counts"),
			  100, 0., 50.); 

     fhDigiMul0D4best =  new TH1F( Form("hDigiMul0D4best"),Form("Number of digis in cluster; N_{digi}; "),
			  20, 0., 20.); 
     fhDigiMul4D4best =  new TH1F( Form("hDigiMul4D4best"),Form("Number of digis in cluster; N_{digi}; "),
			  20, 0., 20.); 
     fhCluMul04D4best =  new TH2F( Form("hCluMul04D4best"),Form("cluster multiplicity ; Mul0; Mul4"),
				   20, 0., 20., 20, 0., 20.); 


     fhChiDT04D4best = new TH2F( Form("hChiDT04D4best"),Form("Time - position correlation; #chi; #DeltaT [ps]"),
			       100, 0., 100., 100, -DTMAX, DTMAX);

     fhDTD4DT04D4best = new TH2F( Form("hDTD4DT04D4best"),
			    Form("Time - velocity correlation; #DeltaTD4 [ps]; #DeltaT04 [ps]"),
			    100, -DTMAX*6., DTMAX*6., 100, -DTMAX*2., DTMAX*2.); 

/*     Double_t dXMAX=30.;*/
/*     Double_t dYMAX=20.;*/
     fhDTMul4D4best = new TH2F( Form("hDTMul4D4best"),
			    Form("Time - Multiplicity correlation; Mul4 ; #DeltaT04 [ps]"),
			    20, 0., 20., 100, -DTMAX, DTMAX);
     fhDTX4D4best = new TH2F( Form("hDTX4D4best"),
			    Form("Time - position correlation; X4 [cm]; #DeltaT04 [ps]"),
			    50, -17., 17., 100, -DTMAX, DTMAX); 
     fhDTY4D4best = new TH2F( Form("hDTY4D4best"),
			    Form("Time - position correlation; Y4 [cm]; #DeltaT04 [ps]"),
			    50, -15., 15., 100, -DTMAX, DTMAX); 
     fhDXX4D4best = new TH2F( Form("hDXX4D4best"),
			    Form("DX - position correlation; X4 [cm]; #DeltaX04 [cm]"),
			    50, -17., 17., 50, -10., 10.); 
     fhDXY4D4best = new TH2F( Form("hDXY4D4best"),
			    Form("DX - position correlation; Y4 [cm]; #DeltaX04 [cm]"),
			    50, -15., 15., 50, -10., 10.); 
     fhDYX4D4best = new TH2F( Form("hDYX4D4best"),
			    Form("DY - position correlation; X4 [cm]; #DeltaY04 [cm]"),
			    50, -17., 17., 50, -10., 10.); 
     fhDYY4D4best = new TH2F( Form("hDYY4D4best"),
			    Form("DY - position correlation; Y4 [cm]; #DeltaY04 [cm]"),
			    50, -15., 15., 50, -10., 10.); 

     fhDTMul0D4best = new TH2F( Form("hDTMul0D4best"),
			    Form("Time - Multiplicity correlation; Mul0 ; #DeltaT04 [ps]"),
			    20, 0., 20., 100, -DTMAX, DTMAX);
     fhDTX0D4best = new TH2F( Form("hDTX0D4best"),
			    Form("Time - position correlation; X0 [cm]; #DeltaT04 [ps]"),
			    50, -17., 17., 100, -DTMAX, DTMAX); 
     fhDTY0D4best = new TH2F( Form("hDTY0D4best"),
			    Form("Time - position correlation; Y0 [cm]; #DeltaT04 [ps]"),
			    50, -15., 15., 100, -DTMAX, DTMAX); 
     fhDXX0D4best = new TH2F( Form("hDXX0D4best"),
			    Form("DX - position correlation; X0 [cm]; #DeltaX04 [cm]"),
			    50, -17., 17., 50, -15., 15.); 
     fhDXY0D4best = new TH2F( Form("hDXY0D4best"),
			    Form("DX - position correlation; Y0 [cm]; #DeltaX04 [cm]"),
			    50, -15., 15., 50, -15., 15.); 
     fhDYX0D4best = new TH2F( Form("hDYX0D4best"),
			    Form("DY - position correlation; X0 [cm]; #DeltaY04 [cm]"),
			    50, -17., 17., 50, -15., 15.); 
     fhDYY0D4best = new TH2F( Form("hDYY0D4best"),
			    Form("DY - position correlation; Y0 [cm]; #DeltaY04 [cm]"),
			    50, -15., 15., 50, -15., 15.); 


     fhX0DT04D4sbest = new TH2F( Form("hX0DT04D4sbest"),Form("time - position correlation; #Delta x [cm]; #DeltaT [ps]"),
			       100, -50., 50., 100, -DTMAX, DTMAX); 
     fhY0DT04D4sbest = new TH2F( Form("hY0DT04D4sbest"),Form("time - position correlation; #Delta y [cm]; #DeltaT [ps]"),
			       100, -50., 50., 100, -DTMAX, DTMAX); 

     fhDXDY04D4sbest = new TH2F( Form("hDXDY04D4sbest"),Form("position correlation; #Delta x [cm]; #DeltaY [cm]"),
			       100, -DXMAX, DXMAX, 100, -DYMAX, DYMAX); 
     fhDXDT04D4sbest = new TH2F( Form("hDXDT04D4sbest"),Form("time - position correlation; #Delta x [cm]; #DeltaT [ps]"),
			       100, -DXMAX, DXMAX, 100, -DTMAX, DTMAX); 
     fhDYDT04D4sbest = new TH2F( Form("hDYDT04D4sbest"),Form("time - position correlation; #Delta y [cm]; #DeltaT [ps]"),
			       100, -DYMAX, DYMAX, 100, -DTMAX, DTMAX); 
     fhDistDT04D4sbest = new TH2F( Form("hDistDT04D4sbest"),Form("time - distance correlation; Dist [cm]; #DeltaT [ps]"),
			       100, fdHitDistMin, fdHitDistMin + 5., 100, -DTMAX, DTMAX); 
     fhTexpDT04D4sbest = new TH2F( Form("hTexpDT04D4sbest"),Form("measured - expected time - correlation; Texp [ps]; #DeltaT [ps]"),
			       100, 0., 6000., 100, -DTMAX, DTMAX); 
     fhCluSize0DT04D4sbest = new TH2F( Form("hCluSize0DT04D4sbest"),
	 			     Form("time - CluSize correlation; N_{strips} ; #DeltaT [ps]"),
				     16, 0.5, 16.5, 100, -DTMAX, DTMAX); 
     fhCluSize4DT04D4sbest = new TH2F( Form("hCluSize4DT04D4sbest"),
	 			     Form("time - CluSize correlation; N_{strips} ; #DeltaT [ps]"),
				     16, 0.5, 16.5, 100, -DTMAX, DTMAX);
     fhTot0DT04D4sbest = new TH2F( Form("hTot0DT04D4sbest"),
	 			     Form("time - Tot correlation; TOT0 ; #DeltaT [ps]"),
				     100, 0., 10000., 100, -DTMAX, DTMAX);  
     fhTot4DT04D4sbest = new TH2F( Form("hTot4DT04D4sbest"),
	 			     Form("time - Tot correlation; TOT4 ; #DeltaT [ps]"),
				     100, 0., 10000., 100, -DTMAX, DTMAX); 

     fhChi04D4sbest  =  new TH1F( Form("hChi04D4sbest"),Form("matching chi2; #chi; Nhits"),
			  100, 0., fdChi2Lim);
     fhTofD4sbest  =  new TH1F( Form("hTofD4sbest"),Form("tof D4; t [ps]; Counts"),
			  100, 0., 50000.); 
     fhVelD4sbest  =  new TH1F( Form("hVelD4sbest"),Form("vel D4; v [cm/ns]; Counts"),
			  100, 0., 50.); 

     fhDigiMul0D4sbest =  new TH1F( Form("hDigiMul0D4sbest"),Form("Number of digis in cluster; N_{digi}; "),
			  20, 0., 20.); 
     fhDigiMul4D4sbest =  new TH1F( Form("hDigiMul4D4sbest"),Form("Number of digis in cluster; N_{digi}; "),
			  20, 0., 20.); 
     fhCluMul04D4sbest =  new TH2F( Form("hCluMul04D4sbest"),Form("cluster multiplicity ; Mul0; Mul4"),
				   10, 0., 10., 10, 0., 10.); 

     fhChiDT04D4sbest = new TH2F( Form("hChiDT04D4sbest"),Form("Time - position correlation; #chi; #DeltaT [ps]"),
			       100, 0., 100., 100, -DTMAX, DTMAX);
 
     fhDTD4DT04D4sbest = new TH2F( Form("hDTD4DT04D4sbest"),
			    Form("Time - velocity correlation; #DeltaTD4 [ps]; #DeltaT04 [ps]"),
			    100, -DTMAX*6., DTMAX*6., 100, -DTMAX, DTMAX); 

     fhDTMul4D4sbest = new TH2F( Form("hDTMul4D4sbest"),
			    Form("Time - Multiplicity correlation; Mul4 ; #DeltaT04 [ps]"),
			    10, 0., 10., 100, -DTMAX, DTMAX);
     fhDTX4D4sbest = new TH2F( Form("hDTX4D4sbest"),
			    Form("Time - position correlation; X4 [cm]; #DeltaT04 [ps]"),
			    50, -15., 15., 100, -DTMAX, DTMAX); 
     fhDTY4D4sbest = new TH2F( Form("hDTY4D4sbest"),
			    Form("Time - position correlation; Y4 [cm]; #DeltaT04 [ps]"),
			    50, -15., 15., 100, -DTMAX, DTMAX); 
     fhDXX4D4sbest = new TH2F( Form("hDXX4D4sbest"),
			    Form("DX - position correlation; X4 [cm]; #DeltaX04 [cm]"),
			    50, -15., 15., 50, -10., 10.); 
     fhDXY4D4sbest = new TH2F( Form("hDXY4D4sbest"),
			    Form("DX - position correlation; Y4 [cm]; #DeltaX04 [cm]"),
			    50, -15., 15., 50, -10., 10.); 
     fhDYX4D4sbest = new TH2F( Form("hDYX4D4sbest"),
			    Form("DY - position correlation; X4 [cm]; #DeltaY04 [cm]"),
			    50, -15., 15., 50, -10., 10.); 
     fhDYY4D4sbest = new TH2F( Form("hDYY4D4sbest"),
			    Form("DY - position correlation; Y4 [cm]; #DeltaY04 [cm]"),
			    50, -15., 15., 50, -10., 10.); 

     fhDTMul0D4sbest = new TH2F( Form("hDTMul0D4sbest"),
			    Form("Time - Multiplicity correlation; Mul0 ; #DeltaT04 [ps]"),
			    10, 0., 10., 100, -DTMAX, DTMAX);
     fhDTX0D4sbest = new TH2F( Form("hDTX0D4sbest"),
			    Form("Time - position correlation; X0 [cm]; #DeltaT04 [ps]"),
			    50, -17., 17., 100, -DTMAX, DTMAX); 
     fhDTY0D4sbest = new TH2F( Form("hDTY0D4sbest"),
			    Form("Time - position correlation; Y0 [cm]; #DeltaT04 [ps]"),
			    50, -15., 15., 100, -DTMAX, DTMAX); 
     fhDXX0D4sbest = new TH2F( Form("hDXX0D4sbest"),
			    Form("DX - position correlation; X0 [cm]; #DeltaX04 [cm]"),
			    50, -17., 17., 50, -15., 15.); 
     fhDXY0D4sbest = new TH2F( Form("hDXY0D4sbest"),
			    Form("DX - position correlation; Y0 [cm]; #DeltaX04 [cm]"),
			    50, -15., 15., 50, -15., 15.); 
     fhDYX0D4sbest = new TH2F( Form("hDYX0D4sbest"),
			    Form("DY - position correlation; X0 [cm]; #DeltaY04 [cm]"),
			    50, -17., 17., 50, -15., 15.); 
     fhDYY0D4sbest = new TH2F( Form("hDYY0D4sbest"),
			    Form("DY - position correlation; Y0 [cm]; #DeltaY04 [cm]"),
			    50, -15., 15., 50, -15., 15.); 

     fhNMatch04 =  new TH1F( Form("hNMatch04"),Form("Number of Matched Hit pairs 0-4; NMatched"),
			  50, 0., 50.); 

     fhNMatch04sel =  new TH1F( Form("hNMatch04sel"),Form("Number of Matched Hit pairs 0-4; NMatched"),
			  50, 0., 50.); 

     fhNMatchD4sel =  new TH1F( Form("hNMatchD4sel"),Form("Number of Matched Hit pairs 0-4; NMatched"),
			  50, 0., 50.); 

     fhDTD4sel =  new TH1F( Form("hDTD4sel"),Form("Time difference BRef - MrpcRef; #DeltaTD4 (ps)"),
			  100, -DTDMAX, DTDMAX); 

     fhTofD4sel =  new TH1F( Form("hTofD4sel"),Form("Time difference BRef - MrpcRef; #DeltaTD4 (ps)"),
			  100, -DTDMAX, DTDMAX); 

     fhDT04DX0_1 = new TH2F( Form("hDT04DX0_1"),
			    Form("Time - position correlation; #DeltaX0 [cm]; #DeltaT04 [ps]"),
			    50, -25., 25., 100, -DTMAX, DTMAX); 
     fhDT04DY0_1 = new TH2F( Form("hDT04DY0_1"),
			    Form("Time - position correlation; #DeltaY0 [cm]; #DeltaT04 [ps]"),
			    50, -25., 25., 100, -DTMAX, DTMAX); 
     fhDT04DT0_1 = new TH2F( Form("hDT04DT0_1"),
			    Form("Time - time correlation; #DeltaT0 [ps]; #DeltaT04 [ps]"),
			    50, -5000., 5000., 100, -DTMAX, DTMAX); 

     fhDT04DX4_1 = new TH2F( Form("hDT04DX4_1"),
			    Form("Time - position correlation; #DeltaX4 [cm]; #DeltaT04 [ps]"),
			    50, -10., 10., 100, -DTMAX, DTMAX); 
     fhDT04DY4_1 = new TH2F( Form("hDT04DY4_1"),
			    Form("Time - position correlation; #DeltaY4 [cm]; #DeltaT04 [ps]"),
			    50, -15., 15., 100, -DTMAX, DTMAX); 
     fhDT04DT4_1 = new TH2F( Form("hDT04DT4_1"),
			    Form("Time - time correlation; #DeltaT4 [ps]; #DeltaT04 [ps]"),
			    50, -5000., 5000., 100, -DTMAX, DTMAX); 

     fhDT04DX0_2 = new TH2F( Form("hDT04DX0_2"),
			    Form("Time - position correlation; #DeltaX0 [cm]; #DeltaT04 [ps]"),
			    50, -25., 25., 100, -DTMAX, DTMAX); 
     fhDT04DY0_2 = new TH2F( Form("hDT04DY0_2"),
			    Form("Time - position correlation; #DeltaY0 [cm]; #DeltaT04 [ps]"),
			    50, -25., 25., 100, -DTMAX, DTMAX); 
     fhDT04DT0_2 = new TH2F( Form("hDT04DT0_2"),
			    Form("Time - time correlation; #DeltaT0 [ps]; #DeltaT04 [ps]"),
			    50, -5000., 5000., 100, -DTMAX, DTMAX); 

     fhDT04DX4_2 = new TH2F( Form("hDT04DX4_2"),
			    Form("Time - position correlation; #DeltaX4 [cm]; #DeltaT04 [ps]"),
			    50, -10., 10., 100, -DTMAX, DTMAX); 
     fhDT04DY4_2 = new TH2F( Form("hDT04DY4_2"),
			    Form("Time - position correlation; #DeltaY4 [cm]; #DeltaT04 [ps]"),
			    50, -15., 15., 100, -DTMAX, DTMAX); 
     fhDT04DT4_2 = new TH2F( Form("hDT04DT4_2"),
			    Form("Time - time correlation; #DeltaT4 [ps]; #DeltaT04 [ps]"),
			    50, -5000., 5000., 100, -DTMAX, DTMAX); 

  // Dut histos

    fhDutPullX=new TH1F(  Form("hDutPullX_Sm_%d",fiDut),
			    Form("hDutPullX_Sm_%d;  #DeltaX",fiDut),
			    100, -10., 10.);  
    fhDutPullY=new TH1F(  Form("hDutPullY_Sm_%d",fiDut),
			    Form("hDutPullY_Sm_%d;  #DeltaY",fiDut),
			    100, -10., 10.);  
    fhDutPullZ=new TH1F(  Form("hDutPullZ_Sm_%d",fiDut),
			    Form("hDutPullZ_Sm_%d;  #DeltaZ",fiDut),
			    100, -200., 200.);  
    fhDutPullT=new TH1F(  Form("hDutPullT_Sm_%d",fiDut),
			    Form("hDutPullT_Sm_%d;  #DeltaT",fiDut),
			    100, -500., 500.); 
    fhDutPullTB=new TH1F( Form("hDutPullTB_Sm_%d",fiDut),
			    Form("hDutPullTB_Sm_%d;  #DeltaT",fiDut),
			    150, -750., 750.); 

    fhDutChiFound=new TH1F(  Form("hDutChi_Found_Sm_%d",fiDut),
			    Form("hDutChi_Found_Sm_%d;  #chi",fiDut),
			    50, 0., 10.);  
    fhDutChiMissed=new TH1F(  Form("hDutChi_Missed_Sm_%d",fiDut),
			    Form("hDutChi_Missed_Sm_%d;  #chi",fiDut),
			    50, 0., 10.);  

    fhDutChiMatch=new TH1F(  Form("hDutChi_Match_Sm_%d",fiDut),
			    Form("hDutChi_Match_Sm_%d;  #chi",fiDut),
			    50, 0., 10.);  
    Double_t XSIZ=20.; 
    Double_t DTSIZ=500;
    Int_t Nbins=40.;
    fhDutXY_Found = new TH2F( Form("hDutXY_Found_%d",fiDut),
			    Form("hDutXY_Found_%d;  x(cm); y (cm)",fiDut),
			    Nbins, -XSIZ, XSIZ, Nbins, -XSIZ, XSIZ); 
    fhDutXY_Missed = new TH2F( Form("hDutXY_Missed_%d",fiDut),
			    Form("hDutXY_Missed_%d;  x(cm); y (cm)",fiDut),
			    Nbins, -XSIZ, XSIZ, Nbins, -XSIZ, XSIZ); 

    fhDutXYDT      = new TH3F( Form("hDutXYDT_%d",fiDut), 
			    Form("hDutXYDT_%d;  x(cm); y (cm); #Deltat (ps)",fiDut),
			    Nbins, -XSIZ, XSIZ, Nbins, -XSIZ, XSIZ, Nbins, -DTSIZ, DTSIZ); 

   gDirectory->cd( oldir->GetPath() ); // <= To prevent histos from being sucked in by the param file of the TRootManager!

   return kTRUE;
}

// ------------------------------------------------------------------
Bool_t CbmTofAnaTestbeam::FillHistos()
{
   // Constants, TODO => put as parameter !!!

   // Declare variables outside the loop
   CbmTofHit   *pHit;
   CbmTofHit   *pHit1;
   CbmTofHit   *pHit2;
   CbmTofHit   *pHit3;
   CbmTofHit   *pHit4;
   CbmTofHit   *pHitRef =NULL; // May be used uninitialized later, to check!
   CbmTofHit   *pHitSel2=NULL;
   CbmTofHit   *pDia;
   CbmTofCell  *fChannelInfo1;
   CbmTofCell  *fChannelInfo2;
   CbmTofCell  *fChannelInfo3;
   CbmTofCell  *fChannelInfo4;

   // Trb System 
   if (NULL != fTrbHeader) { 
     if(fiReqTrg>-1) if(!fTrbHeader->TriggerFired( fiReqTrg )) return kFALSE;
     UInt_t uTriggerPattern=fTrbHeader->GetTriggerPattern();
     for(UInt_t uChannel = 0; uChannel < 16; uChannel++)
     {
       if( uTriggerPattern & (0x1 << uChannel) )
       {
         fhTriggerPattern->Fill( uChannel );
       }
     }
     fhTriggerType->Fill(fTrbHeader->GetTriggerType());
     fhTimeInSpill->Fill(fTrbHeader->GetTimeInSpill());
   }

/*   Int_t iNbTofDigis;*/
   Int_t iNbTofHits, iNbTofTracks;

   //   iNbTofDigis   = fTofDigisColl->GetEntriesFast();
   iNbTofHits    = fTofHitsColl->GetEntriesFast();

   /*
   LOG(INFO)<<Form("CbmTofAnaTestbeam::FillHistos for %d digis and %d Tof hits",iNbTofDigis,iNbTofHits)
	    <<FairLogger::endl;
   */
   // Digis info
   /*
   Double_t dTotalDataSize = 0;
   if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
   {
      CbmTofDigiExp *pDigi;
      for( Int_t iDigInd = 0; iDigInd < iNbTofDigis; iDigInd++ )
      {
         pDigi = (CbmTofDigiExp*) fTofDigisColl->At( iDigInd );

         Int_t iSmType = pDigi->GetType();
         Int_t iSm     = pDigi->GetSm();
         Int_t iRpc    = pDigi->GetRpc();
         Int_t iCh     = pDigi->GetChannel();

      } // for( Int_t iDigInd = 0; iDigInd < iNbTofDigis; iDigInd++ )
   } // if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
   */
   // Hits info
   Int_t iNbMatchedHits = 0;
   Int_t iNbMaxMatch=100;
   Double_t Zref=300.;
/*   Double_t Chi2MatchMin=1.E8;*/
   Double_t Chi2List[iNbMaxMatch];
   CbmTofHit *pChi2Hit1[iNbMaxMatch];
   CbmTofHit *pChi2Hit2[iNbMaxMatch];
   Chi2List[0]=1.E8;
   pChi2Hit1[0]=NULL;
   pChi2Hit2[0]=NULL;
   pDia=NULL;
   Int_t iNSel=1;
   Bool_t  BSel[iNSel];
   for(Int_t iSel=0;iSel<iNSel;iSel++){BSel[iSel]=kFALSE;}
   Double_t dDTD4=0.;
   dTDia=1.E300;
   dDTD4Min=1.E300;
   Double_t dMul0=0.;
   Double_t dMul4=0.;
   Double_t dMulD=0.;
   Double_t dM4Max=1; // modify
   if (fdMul4Max>0) dM4Max=fdMul4Max;
   Double_t dM0Max=100; // modify
   if (fdMul0Max>0) dM0Max=fdMul0Max;
   Double_t dMDMax=1; 
   if (fdMulDMax>0) dMDMax=fdMulDMax;
   Double_t hitpos1[3], hitpos2[3], hitpos3[3], hitpos4[3];
   Double_t hitpos1_local[3], hitpos2_local[3], hitpos3_local[3], hitpos4_local[3];

   // find diamond reference

   for( Int_t iHitInd = 0; iHitInd < iNbTofHits; iHitInd++)
   {
      pHit = (CbmTofHit*) fTofHitsColl->At( iHitInd );
      if(NULL == pHit) continue;
      Int_t iDetId = (pHit->GetAddress() & DetMask);
      Int_t iChId = pHit->GetAddress();
      fChannelInfo = fDigiPar->GetCell( iChId );
      Int_t iSmType=CbmTofAddress::GetSmType( iDetId );
      
      if(NULL == fChannelInfo){
        LOG(DEBUG) << "CbmTofAnaTestbeam::FillHistos: NULL Channel Pointer for ChId "
		   << Form(" 0x%08x ",iChId)
		   <<FairLogger::endl;
	continue;
      }
      if(iSmType == fiBeamRefSmType){ // diamond hit (or other reference counter)
        if( fiBeamRefSmId == CbmTofAddress::GetSmId( iDetId )) {
	  dMulD++;
	  if (0) {  //fdTShift != 0.) {  //nhmod
	    Double_t dTime=pHit->GetTime()+fdTShift;
	    pHit->SetTime(dTime);  // shift beam reference times
	    if(pHit->GetTime() != dTime) 
	      LOG(ERROR)<<" Hit time not updated "<<pHit->GetTime()<<" -> "<<dTime<<FairLogger::endl;
	  }
	  if ( pHit->GetTime() < dTDia) {
	    dTDia = pHit->GetTime();
	    pDia  = pHit;
	  }
	}
      }
   } // reaction reference loop end;

   fhBRefMul->Fill(dMulD);
   LOG(DEBUG)<<Form("CbmTofAnaTestbeam::FillHistos: Diamond mul %f, time: %6.2e",dMulD,dTDia)
	     <<Form(", inspect Dut 0x%08x, Ref 0x%08x, Sel2  0x%08x, Sel3  0x%08x ",fiDutAddr,
		    fiMrpcRefAddr,fiMrpcSel2Addr,fiMrpcSel3Addr)
	     <<FairLogger::endl;

   // process counter hits, fill Chi2List, check selectors 

   for( Int_t iHitInd = 0; iHitInd < iNbTofHits; iHitInd++)
   {
      pHit = (CbmTofHit*) fTofHitsColl->At( iHitInd );
      if(NULL == pHit) continue;
      Int_t iDetId = (pHit->GetAddress() & DetMask);
      Int_t iChId = pHit->GetAddress();
      fChannelInfo = fDigiPar->GetCell( iChId );
      Int_t iSmType = CbmTofAddress::GetSmType( iDetId );
      Int_t iDetInd = fMbsMappingPar->GetMappedDetInd( pHit->GetAddress()); 
      LOG(DEBUG)<<Form("CbmTofAnaTestbeam::FillHistos: process %d.(%d) Tof hit 0x%08x, Ind %d, x = %6.1f, y = %6.1f, z=%6.1f, t=%10.1f",
		       iHitInd, iNbTofHits, iChId, iDetInd,
		       pHit->GetX(), pHit->GetY(), pHit->GetZ(), pHit->GetTime())
	    <<FairLogger::endl;
      
      if(NULL == fChannelInfo){
        LOG(DEBUG) << "CbmTofAnaTestbeam::FillHistos: NULL Channel Pointer for ChId "
		   << Form(" 0x%08x ",iChId)
		   <<FairLogger::endl;
	continue;
      }

      if(static_cast<UInt_t>(iDetInd)<fhXYPos.size()) fhXYPos[iDetInd]->Fill(pHit->GetX(),pHit->GetY());

      LOG(DEBUG2)  <<"CbmTofAnaTestbeam::FillHistos: process iDetId  "
		   <<Form(" 0x%08x ",iDetId)
		   <<Form(", Muls %4.0f, %4.0f, %4.0f",dMulD, dMul0, dMul4)
		   <<FairLogger::endl;

      if(fiDutAddr == iDetId) {
	dMul0++;
	Double_t xPos1=Zref/pHit->GetZ()*pHit->GetX();
	Double_t yPos1=Zref/pHit->GetZ()*pHit->GetY();
        Double_t tof1 =pHit->GetTime();

	for( Int_t iHitInd2 = 0; iHitInd2 < iNbTofHits; iHitInd2++)
	{
          if(iHitInd2 != iHitInd){
	   pHit2 = (CbmTofHit*) fTofHitsColl->At( iHitInd2 );
	   if (NULL== pHit2) continue;

	   Int_t iDetId2 = (pHit2->GetAddress() & DetMask);
	   Int_t iChId2  = pHit2->GetAddress();
           fChannelInfo2 = fDigiPar->GetCell( iChId2 );

	   if (   fiMrpcRefAddr  == iDetId2
	     /* 
	            fiMrpcRef  == CbmTofAddress::GetSmType( iDetId2 )
	       && fiMrpcRefSm  == CbmTofAddress::GetSmId( iDetId2 )
	       && fiMrpcRefRpc == CbmTofAddress::GetRpcId( iDetId2 )
	     */
               ){  // MrpcRef
             Double_t xPos2=Zref/pHit2->GetZ()*pHit2->GetX();
	     Double_t yPos2=Zref/pHit2->GetZ()*pHit2->GetY();
             Double_t tof2 =pHit2->GetTime();
	     Double_t dTcor=0.;
	     if(fhDTD4DT04D4Off != NULL) 
	        dTcor=(Double_t)fhDTD4DT04D4Off->GetBinContent(fhDTD4DT04D4Off->FindBin(dTDia-tof2));

             Double_t Chi2Match =TMath::Power((xPos1-xPos2-fdDXMean)/fdDXWidth,2.)
	                        +TMath::Power((yPos1-yPos2-fdDYMean)/fdDYWidth,2.)
	                        +TMath::Power((tof1-tof2-dTcor-fdDTMean)/fdDTWidth,2.);
             if (Chi2Match > 1.E8) continue;
	     Chi2Match /= 3;

	     LOG(DEBUG1)<<" Chi2 "<<Form(" %f %f %f %f %f %f ",fdDXMean,fdDXWidth,fdDYMean,fdDYWidth,fdDTMean,fdDTWidth)
		        <<Form(" -> %f ", Chi2Match)
		        <<FairLogger::endl;
	     LOG(DEBUG1)<<" Chi2 "<<Form(" %f %f %f %f %f %f ",xPos1,xPos2,yPos1,yPos2,tof1,tof2)
		        <<Form(" -> %f ", Chi2Match)
		        <<FairLogger::endl;

	     iNbMatchedHits++;
	     if(iNbMatchedHits==iNbMaxMatch) iNbMatchedHits=iNbMaxMatch-1; //prevent array overflow
 	     LOG(DEBUG)
	       <<Form("CbmTofAnaTestbeam::FillHistos: match %d (%f):  %2d.-%2d. Tof hit 0x%08x of SmT %1d, 0x%08x with 0x%08x",
		 iNbMatchedHits, Chi2Match, iHitInd, iHitInd2, iDetId2, CbmTofAddress::GetSmType( iDetId2 ), iChId2, iChId)
	       <<FairLogger::endl;
	     /*
	     fhXX04->Fill(xPos1,xPos2);
	     fhYY04->Fill(yPos1,yPos2);
	     fhXY04->Fill(xPos1,yPos2);
	     fhYX04->Fill(yPos1,xPos2);
	     fhTT04->Fill(tof1,tof2);
	     */
	     fhDXDY04->Fill(xPos1-xPos2,yPos1-yPos2);
	     fhDXDT04->Fill(xPos1-xPos2,tof1-tof2);
	     fhDYDT04->Fill(yPos1-yPos2,tof1-tof2);
             fhChi04->Fill(Chi2Match);

	     for(Int_t iM=0; iM<iNbMatchedHits; iM++)
	     {
              if(Chi2Match < Chi2List[iM]){

	       LOG(DEBUG)<<Form(" fill Chi2 %3d (%3d), %12.1f, %12.1f: HitInd %3d, %3d, Ids 0x%08x, 0x%08x",  
				iM, iNbMatchedHits, Chi2Match, Chi2List[iM], iHitInd,iHitInd2,iChId,iChId2)
			 <<FairLogger::endl;

               for(Int_t iMM=iNbMatchedHits; iMM>=iM; iMM--){
		 Chi2List[iMM]= Chi2List[iMM-1];
		 pChi2Hit1[iMM]= pChi2Hit1[iMM-1];
		 pChi2Hit2[iMM]= pChi2Hit2[iMM-1];
	       }

 	       Chi2List[iM]=Chi2Match;
	       pChi2Hit1[iM]=pHit;
	       pChi2Hit2[iM]=pHit2;
	       Chi2List[iNbMatchedHits]=1.E8;
               if(iM>0){
		 if(Chi2Match == Chi2List[iM-1]){
		   LOG(DEBUG)<<Form("same Chi2?: M %d, Mul0 %3.0f, HitInd %d, %d, Ids 0x%p, 0x%p - 0x%p, 0x%p",  
				   iM,dMul0,iHitInd,iHitInd2,pHit,pHit2,pChi2Hit1[iM-1],pChi2Hit2[iM-1])
			    <<FairLogger::endl;
		   LOG(DEBUG) << pHit->ToString()           <<FairLogger::endl;
		   LOG(DEBUG) << pChi2Hit1[iM-1]->ToString()<<FairLogger::endl;
		   LOG(DEBUG) << pHit2->ToString()          <<FairLogger::endl;
		 }
	       }
	       break;
	      }
	     }
	   } //fiMrpcRef condition end
	   if (2==CbmTofAddress::GetSmType( iDetId2 )){  // Plastic 
	     Int_t iSm=CbmTofAddress::GetSmId( iDetId2 );
             if(iSm<2){
	      fhXX02[iSm]->Fill(Zref/pHit->GetZ()*pHit->GetX(),Zref/pHit2->GetZ()*pHit2->GetX());
	      fhYY02[iSm]->Fill(Zref/pHit->GetZ()*pHit->GetY(),Zref/pHit2->GetZ()*pHit2->GetY());
	     }
	   }
	  } //iHit2 != iHit condition end
	} // iHit2 loop end 
      } // fiDut condition end 
      /*
      if(fiMrpcRef == iSmType) {
	Int_t iSm  = CbmTofAddress::GetSmId( iDetId );
	if(iSm != fiMrpcRefSm) continue; // skip module
	Int_t iRpc = CbmTofAddress::GetRpcId( iDetId );
	if(iRpc != fiMrpcRefRpc) continue; // skip Rpc 
      */
      if(fiMrpcRefAddr == iDetId) {
	dMul4++;
      }

      if(fiBeamRefAddr == iDetId) {
	if( pHit != pDia ) continue;
	if( fdDTDia>0. )
	{
	  Double_t dDDia=0.;
	  for( Int_t iHitInd1 = 0; iHitInd1 < iNbTofHits; iHitInd1++) 
	  if( iHitInd1!=iHitInd)
	  {
	    pHit1 = (CbmTofHit*) fTofHitsColl->At( iHitInd1 );
	    if(pHit1==NULL) continue;
	    Int_t iDetId1 = (pHit1->GetAddress() & DetMask);
	    Int_t iChId1 = pHit1->GetAddress();
            fChannelInfo1 = fDigiPar->GetCell( iChId1 );
	    pHit1 = (CbmTofHit*) fTofHitsColl->At( iHitInd1 );
	    if(  fiBeamRefSmType == CbmTofAddress::GetSmType( iDetId1 ) 
	      && fiBeamRefSmId != CbmTofAddress::GetSmId  ( iDetId1 )
	      && TMath::Abs( pHit1->GetTime()-dTDia)<fdDTDia){   // second diamond fired
	      dDDia= pHit1->GetTime()-dTDia;
	    }
	  }
 	  LOG(DEBUG)<<Form("CbmTofAnaTestbeam:FillHisto: DDia %f",dDDia)<<FairLogger::endl; 
	  if (dDDia==0.) continue;
	} // 2 diamond condition end 

	for( Int_t iHitInd2 = 0; iHitInd2 < iNbTofHits; iHitInd2++) 
	  if(iHitInd2!=iHitInd)
	  {
	   pHit2 = (CbmTofHit*) fTofHitsColl->At( iHitInd2 );
	   if(pHit2==NULL) continue;

	   Int_t iDetId2 = (pHit2->GetAddress() & DetMask);
	   Int_t iChId2 = pHit2->GetAddress();
	   fChannelInfo2 = fDigiPar->GetCell( iChId2 );
	   if(NULL == fChannelInfo2){
	     LOG(DEBUG) << "CbmTofAnaTestbeam::FillHistos: Invalid Channel Pointer for ChId2 "
			<< Form(" 0x%08x ",iChId2)
			<<FairLogger::endl;
	     continue;
	   }
	   LOG(DEBUG2)<<Form("CbmTofAnaTestbeam:FillHisto: TDia %f, THit %f",dTDia,pHit2->GetTime())
		      <<FairLogger::endl;

	   if(  fiMrpcRefAddr  == iDetId2  //CbmTofAddress::GetSmType( iDetId2 )
	     //&& fiMrpcRefSm  == CbmTofAddress::GetSmId( iDetId2 )
	     //&& fiMrpcRefRpc == CbmTofAddress::GetRpcId( iDetId2 )
	       ){   // Reference RPC hit
	     dDTD4=pHit2->GetTime()-dTDia + fdTShift;
	     fhDTD4->Fill(dDTD4);

	     LOG(DEBUG1)<<Form("CbmTofAnaTestbeam:FillHisto: dDTD4 %6.2f, min: %6.2e",dDTD4,dDTD4Min)
			<<FairLogger::endl;

	     if( TMath::Abs(dDTD4)<fdDTD4MAX &&        // single selection scheme, selector 0
	         TMath::Abs(CbmTofAddress::GetChannelId( iChId2 ) - fdCh4Sel) < fdDCh4Sel     )
	       {
		 /*TGeoNode *fNode=*/        // prepare global->local trafo
		   gGeoManager->FindNode(fChannelInfo2->GetX(),fChannelInfo2->GetY(),fChannelInfo2->GetZ());
		 Double_t hitpos[3],  hitpos_local[3];
		 hitpos[0]=pHit2->GetX();
		 hitpos[1]=pHit2->GetY();
		 hitpos[2]=pHit2->GetZ();
		 /*TGeoNode* cNode=*/ gGeoManager->GetCurrentNode();
		 gGeoManager->MasterToLocal(hitpos, hitpos_local);
	         if( TMath::Abs(hitpos_local[1]-fdPosY4SelOff)<fdPosY4Sel*fChannelInfo2->GetSizey()
		   &&TMath::Abs(dDTD4)<TMath::Abs(dDTD4Min)){

		   dDTD4Min=dDTD4;
		   pHitRef=pHit2;
		   fChannelInfoRef=fChannelInfo2;
		   if( fiMrpcSel2 < 1 ) {  // assume to be initialized to 0 
		     BSel[0]=kTRUE;
		   } else { // request presence of coincident fiMrpcSel2 hit!
		     Double_t xPos2=Zref/pHit2->GetZ()*pHit2->GetX();
		     Double_t yPos2=Zref/pHit2->GetZ()*pHit2->GetY();
		     Double_t tof2 =pHit2->GetTime();
		     Double_t dTcor=0.;
		     
		     //	 if(fhDTD4DT04D4Off != NULL) 
		     //  dTcor=(Double_t)fhDTD4DT04D4Off->GetBinContent(fhDTD4DT04D4Off->FindBin(dTDia-tof2));
		     
		     Double_t Chi2Max = fdChi2Lim;
		     pHitSel2 = NULL;

		     for( Int_t iHitInd3 = 0; iHitInd3 < iNbTofHits; iHitInd3++) 
		     if(iHitInd3 != iHitInd && iHitInd3 != iHitInd2)
		     {
		       pHit3 = (CbmTofHit*) fTofHitsColl->At( iHitInd3 );
		       if(pHit3==NULL) continue;
		       Int_t iDetId3 = (pHit3->GetAddress() & DetMask);
		       Int_t iChId3  = pHit3->GetAddress();
		       fChannelInfo3 = fDigiPar->GetCell( iChId3 );
		       if(NULL == fChannelInfo3){
			 LOG(DEBUG) << "CbmTofAnaTestbeam::FillHistos: Invalid Channel Pointer for ChId3 "
				    << Form(" 0x%08x ",iChId3)
				    <<FairLogger::endl;
			 continue;
		       }
		       if( fiMrpcSel2Addr == iDetId3 ) { //CbmTofAddress::GetSmType( iDetId3 )){   // Sel2 RPC hit
			 if(TMath::Abs(CbmTofAddress::GetChannelId( iChId3 ) - fdChS2Sel) < fdDChS2Sel) {
			   /*TGeoNode *fNode3= */       // prepare global->local trafo
			     gGeoManager->FindNode(fChannelInfo3->GetX(),fChannelInfo3->GetY(),fChannelInfo3->GetZ());
			   hitpos3[0]=pHit3->GetX();
			   hitpos3[1]=pHit3->GetY();
			   hitpos3[2]=pHit3->GetZ();
			   /*TGeoNode* cNode3=*/ gGeoManager->GetCurrentNode();
			   gGeoManager->MasterToLocal(hitpos3, hitpos3_local);
			   if( TMath::Abs(hitpos3_local[1]-fdPosYS2SelOff)<fdPosYS2Sel*fChannelInfo3->GetSizey() ){
			     Double_t xPos3=Zref/pHit3->GetZ()*pHit3->GetX();
			     Double_t yPos3=Zref/pHit3->GetZ()*pHit3->GetY();
			     Double_t tof3 =pHit3->GetTime();	
	       
			     Double_t Chi2Match =TMath::Power((xPos3-xPos2)/fdDXWidth,2.)
	                        +TMath::Power((yPos3-yPos2)/fdDYWidth,2.)
	                        +TMath::Power((tof3-tof2-dTcor-fdSel2TOff)/fdDTWidth,2.);

			     Chi2Match /= 3;
			     
			     if (Chi2Match < Chi2Max) {

			       fhChiSel24->Fill(Chi2Match);
			       fhDXSel24->Fill(xPos3-xPos2);
			       fhDYSel24->Fill(yPos3-yPos2);
			       fhDTSel24->Fill(tof3-tof2-dTcor-fdSel2TOff);
			       fhTofSel24->Fill(tof3-tof2-fdSel2TOff);

			       Chi2Max = Chi2Match;
			       BSel[0] = kTRUE;
			       pHitSel2= pHit3;
			       fChannelInfoSel2=fChannelInfo3;
			     }
			   }
			 }
		       }
		     } // loop over third hit 
		   }		   
		 }		  
	       }
	    }
	   }	//LOG(INFO)<<" TDia="<<dTDia<<FairLogger::endl;
      }  // diamond condition end 
   } // for( Int_t iHitInd = 0; iHitInd < iNbTofHits; iHitInd++)

   if(dMul4>dM4Max || dMulD>dMDMax || dMul0>dM0Max)
   {
       BSel[0]=kFALSE;
       LOG(DEBUG) << Form("<D> Muls %4.0f, %4.0f, %4.0f, Matches %d",dMulD, dMul0, dMul4, iNbMatchedHits) 
		  << FairLogger::endl;
   }
   //  normalisation distributions 
   fhNMatch04->Fill(iNbMatchedHits);
   if(fTrbHeader != NULL) fhTIS_all->Fill(fTrbHeader->GetTimeInSpill());

   LOG(DEBUG)<<Form(" Matches: %d with first chi2s = %12.1f, %12.1f, %12.1f, %12.1f",iNbMatchedHits,
		    Chi2List[0],Chi2List[1],Chi2List[2],Chi2List[3])
	     <<Form(", Muls %4.0f, %4.0f, %4.0f",dMulD, dMul0, dMul4)
 	     <<FairLogger::endl;
   // selector 0 distributions 
   if(BSel[0]){
    fhNMatchD4sel->Fill(iNbMatchedHits);  // use as normalisation
    if(fTrbHeader != NULL) fhTIS_sel->Fill(fTrbHeader->GetTimeInSpill());
    fhTofD4sel->Fill(pHitRef->GetTime()-pDia->GetTime());           //  general normalisation
    fhDTD4sel->Fill(dDTD4Min);           //  general normalisation
    
    if(fChannelInfoDut != NULL){
     // Project into Dut reference frame
     /*TGeoNode *fNodeDut=*/        // prepare global->local trafo
     gGeoManager->FindNode(fChannelInfoDut->GetX(),fChannelInfoDut->GetY(),fChannelInfoDut->GetZ());

     hitpos1[0]=fChannelInfoDut->GetZ()/pHitRef->GetZ()*pHitRef->GetX();
     hitpos1[1]=fChannelInfoDut->GetZ()/pHitRef->GetZ()*pHitRef->GetY();
     hitpos1[2]=fChannelInfoDut->GetZ();

     /*TGeoNode* cNodeDut=*/ gGeoManager->GetCurrentNode();
     gGeoManager->MasterToLocal(hitpos1, hitpos1_local);
     //hitpos1_local[0] -= fiDutNch/2 * fChannelInfoDut->GetSizex();
     fhXY0D4sel->Fill(hitpos1_local[0],hitpos1_local[1]);

    }

    // Monitor selected Reference Hit position
    /*TGeoNode *fNodeRef=*/        // prepare global->local trafo
    gGeoManager->FindNode(fChannelInfoRef->GetX(),fChannelInfoRef->GetY(),fChannelInfoRef->GetZ());

    hitpos2[0]=pHitRef->GetX();
    hitpos2[1]=pHitRef->GetY();
    hitpos2[2]=pHitRef->GetZ();
    /*TGeoNode* cNodeRef=*/ gGeoManager->GetCurrentNode();
    gGeoManager->MasterToLocal(hitpos2, hitpos2_local);
    fhXY4D4sel->Fill(hitpos2_local[0],hitpos2_local[1]);

    if(NULL != pHitSel2){  
      /*Int_t iDetId3 = (pHitSel2->GetAddress() & DetMask);*/
      Int_t iChId3  = pHitSel2->GetAddress();
      fChannelInfo3 = fDigiPar->GetCell( iChId3 );
      /*TGeoNode *fNode3=*/        // prepare global->local trafo
	gGeoManager->FindNode(fChannelInfo3->GetX(),fChannelInfo3->GetY(),fChannelInfo3->GetZ());
      hitpos3[0]=pHitSel2->GetX();
      hitpos3[1]=pHitSel2->GetY();
      hitpos3[2]=pHitSel2->GetZ();
      /*TGeoNode* cNode3=*/ gGeoManager->GetCurrentNode();
      gGeoManager->MasterToLocal(hitpos3, hitpos3_local);
      fhXYSel2D4sel->Fill(hitpos3_local[0],hitpos3_local[1]);

      if(fTrbHeader != NULL) fhTIS_sel2->Fill(fTrbHeader->GetTimeInSpill());
    }

    if(iNbMatchedHits>0){
     // best match
     LOG(DEBUG)<<Form(" best match D4 (%d): 0x%p, 0x%p in ch 0x%08x, 0x%08x: %12.1f",
		      iNbMatchedHits,pChi2Hit1[0],pChi2Hit2[0],
		      pChi2Hit1[0]->GetAddress(),pChi2Hit2[0]->GetAddress(), Chi2List[0])
	      <<FairLogger::endl;

     if(NULL != pHitSel2){ 
       fhXYSel2D4best->Fill(hitpos3_local[0],hitpos3_local[1]);
     }

     pHit1=pChi2Hit1[0];
     pHit2=pChi2Hit2[0];
     Int_t iM0=0;
     if (pHit2 != pHitRef ) {
       LOG(DEBUG)<<" selector hit does not match reference hit for best match, chi2best "
		<<  Chi2List[0] 
		<< FairLogger::endl;
       for (iM0=1; iM0<iNbMatchedHits; iM0++){
	 if (pHitRef == pChi2Hit2[iM0]){
	   LOG(DEBUG)<<" found reference hit for best match, chi2new "
		<<  Chi2List[iM0] 
		<< FairLogger::endl;
	   pHit1=pChi2Hit1[iM0];
	   pHit2=pChi2Hit2[iM0];
	   break;
	 }
       }
       if (iM0 == iNbMatchedHits) {
	 LOG(WARNING)<<" no valid match found "<< FairLogger::endl;
	 return 0;
       }
     }

     if ( Chi2List[iM0] > fdChi2Lim) return 0;

     Int_t iDetId1 = (pHit1->GetAddress() & DetMask);
     Int_t iChId1 = pHit1->GetAddress();
     fChannelInfo1 = fDigiPar->GetCell( iChId1 );

     Int_t iDetId2 = (pHit2->GetAddress() & DetMask);
     Int_t iChId2 = pHit2->GetAddress();
     fChannelInfo2 = fDigiPar->GetCell( iChId2 );

     Double_t xPos1=Zref/pHit1->GetZ()*pHit1->GetX();
     Double_t yPos1=Zref/pHit1->GetZ()*pHit1->GetY();
     Double_t tof1=pHit1->GetTime();

     Double_t xPos2=Zref/pHit2->GetZ()*pHit2->GetX();
     Double_t yPos2=Zref/pHit2->GetZ()*pHit2->GetY();
     Double_t tof2=pHit2->GetTime();

     if(fTrbHeader != NULL) fhTIS_sel1->Fill(fTrbHeader->GetTimeInSpill());
     fhXX04->Fill(xPos1,xPos2);
     fhYY04->Fill(yPos1,yPos2);
     fhXY04->Fill(xPos1,yPos2);
     fhYX04->Fill(yPos1,xPos2);
     fhTT04->Fill(tof1,tof1-tof2);

     Double_t dDist=TMath::Sqrt(
				TMath::Power(pHit1->GetX()-pHit2->GetX(),2)
			       +TMath::Power(pHit1->GetY()-pHit2->GetY(),2)
			       +TMath::Power(pHit1->GetZ()-pHit2->GetZ(),2)
				);

     fhChi04D4best->Fill(Chi2List[iM0]);

     CbmMatch* digiMatch1=(CbmMatch *)fTofDigiMatchColl->At(pHit1->GetRefId());
     fhDigiMul0D4best->Fill(digiMatch1->GetNofLinks()/2.);
     CbmMatch* digiMatch2=(CbmMatch *)fTofDigiMatchColl->At(pHit2->GetRefId());
     fhDigiMul4D4best->Fill(digiMatch2->GetNofLinks()/2.);

     fhCluMul04D4best->Fill(dMul0,dMul4);

     // check for dependence in counter reference frame 
     /*TGeoNode *fNode=*/        // prepare global->local trafo
     gGeoManager->FindNode(fChannelInfo2->GetX(),fChannelInfo2->GetY(),fChannelInfo2->GetZ());

     hitpos2[0]=pHit2->GetX();
     hitpos2[1]=pHit2->GetY();
     hitpos2[2]=pHit2->GetZ();

/*     TGeoNode* cNode=*/ gGeoManager->GetCurrentNode(); // -> Comment to remove warning because set but never used
     // if(0) cNode->Print();
     gGeoManager->MasterToLocal(hitpos2, hitpos2_local);

     Double_t dTofD4  = fdTOffD4 + dDTD4Min;
     Double_t dInvVel = dTofD4/pHitRef->GetR(); // in ps/cm
     Double_t dDTexp  = dDist*dInvVel;

     Double_t dTcor=0.;
     if(fhDTD4DT04D4Off != NULL) dTcor=(Double_t)fhDTD4DT04D4Off->GetBinContent(fhDTD4DT04D4Off->FindBin(-dDTD4Min));
     if(fhDTX4D4Off     != NULL) dTcor+=(Double_t)fhDTX4D4Off->GetBinContent(fhDTX4D4Off->FindBin(hitpos2_local[0]));
     if(fhDTY4D4Off     != NULL) dTcor+=(Double_t)fhDTY4D4Off->GetBinContent(fhDTY4D4Off->FindBin(hitpos2_local[1]));
     if(fhDTTexpD4Off   != NULL) dTcor+=(Double_t)fhDTTexpD4Off->GetBinContent(fhDTTexpD4Off->FindBin(dDTexp));

     //     LOG(INFO) << "dTcor for "<<-dDTD4<<" from "<<fhDTD4DT04D4Off<<": "<<dTcor<<FairLogger::endl;

     fhTofD4best->Fill(dTofD4);
     if(dInvVel>0.) fhVelD4best->Fill(1000./dInvVel);

     fhChiDT04D4best->Fill(Chi2List[iM0],tof1-tof2-dTcor);

     fhDTD4DT04D4best->Fill(-dDTD4Min,tof1-tof2-dTcor);
     fhDTMul4D4best->Fill(dMul4,tof1-tof2-dTcor);

     fhXY4D4best->Fill(hitpos2_local[0],hitpos2_local[1]);
     fhDTX4D4best->Fill(hitpos2_local[0],tof1-tof2-dTcor);
     fhDTY4D4best->Fill(hitpos2_local[1],tof1-tof2-dTcor);
     fhDXX4D4best->Fill(hitpos2_local[0],xPos1-xPos2);
     fhDXY4D4best->Fill(hitpos2_local[1],xPos1-xPos2);
     fhDYX4D4best->Fill(hitpos2_local[0],yPos1-yPos2);
     fhDYY4D4best->Fill(hitpos2_local[1],yPos1-yPos2);

     fhCluSize4DT04D4best->Fill(digiMatch2->GetNofLinks()/2.,tof1-tof2-dTcor);

     Double_t dTot = 0.;
     for (Int_t iLink=0; iLink<digiMatch2->GetNofLinks(); iLink++){  // loop over digis
         CbmLink L0 = digiMatch2->GetLink(iLink);  
	 Int_t iDigInd0=L0.GetIndex();
         if (iDigInd0 < fTofDigisColl->GetEntries()){
            CbmTofDigiExp *pDig0 = (CbmTofDigiExp*) (fTofDigisColl->At(iDigInd0));
	    dTot += pDig0->GetTot();
	    LOG(DEBUG)<<Form(" dTot of hit 0x%08x: digind %d add %f -> sum %f",iDetId2,iDigInd0,pDig0->GetTot(),dTot)
		     <<FairLogger::endl;
	 }
     } 
     dTot /= digiMatch2->GetNofLinks();  // average time over threshold
     fhTot4DT04D4best->Fill(dTot,tof1-tof2-dTcor);

     CbmMatch* digiMatch0=(CbmMatch *)fTofDigiMatchColl->At(pHit1->GetRefId());
     fhCluSize0DT04D4best->Fill(digiMatch0->GetNofLinks()/2.,tof1-tof2-dTcor);

     dTot = 0.;
     for (Int_t iLink=0; iLink<digiMatch0->GetNofLinks(); iLink++){  // loop over digis
         CbmLink L0 = digiMatch0->GetLink(iLink);  
	 Int_t iDigInd0=L0.GetIndex();
         if (iDigInd0 < fTofDigisColl->GetEntries()){
            CbmTofDigiExp *pDig0 = (CbmTofDigiExp*) (fTofDigisColl->At(iDigInd0));
	    dTot += pDig0->GetTot();
	    LOG(DEBUG1)<<Form(" dTot of hit 0x%08x: digind %d add %f -> sum %f",iDetId1,iDigInd0,pDig0->GetTot(),dTot)
		      <<FairLogger::endl;
	 }
     } 
     dTot /= digiMatch0->GetNofLinks();  // average time over threshold
     fhTot0DT04D4best->Fill(dTot,tof1-tof2-dTcor);

     fhDTMul0D4best->Fill(dMul0,tof1-tof2-dTcor);

     // check for dependence in counter reference frame
     /*TGeoNode *fNode1=*/        // prepare global->local trafo
     gGeoManager->FindNode(fChannelInfo1->GetX(),fChannelInfo1->GetY(),fChannelInfo1->GetZ());

     hitpos1[0]=pHit1->GetX();
     hitpos1[1]=pHit1->GetY();
     hitpos1[2]=pHit1->GetZ();

     /*TGeoNode* cNode1=*/ gGeoManager->GetCurrentNode();
     gGeoManager->MasterToLocal(hitpos1, hitpos1_local);

     fhXY0D4best->Fill(hitpos1_local[0],hitpos1_local[1]);
     fhXX04D4best->Fill(hitpos1_local[0],hitpos2_local[0]);
     fhYY04D4best->Fill(hitpos1_local[1],hitpos2_local[1]);

     fhDTX0D4best->Fill(hitpos1_local[0],tof1-tof2-dTcor);
     fhDTY0D4best->Fill(hitpos1_local[1],tof1-tof2-dTcor);
     fhDXX0D4best->Fill(hitpos1_local[0],xPos1-xPos2);
     fhDXY0D4best->Fill(hitpos1_local[1],xPos1-xPos2);
     fhDYX0D4best->Fill(hitpos1_local[0],yPos1-yPos2);
     fhDYY0D4best->Fill(hitpos1_local[1],yPos1-yPos2);

     fhDXDY04D4best->Fill(xPos1-xPos2,yPos1-yPos2);
     fhDXDT04D4best->Fill(xPos1-xPos2,tof1-tof2-dTcor);
     fhDYDT04D4best->Fill(yPos1-yPos2,tof1-tof2-dTcor);
     fhDistDT04D4best->Fill(dDist,tof1-tof2-dTcor);
     fhTexpDT04D4best->Fill(dDTexp,tof1-tof2-dTcor);
     fhX0DT04D4best->Fill(hitpos1_local[0],tof1-tof2-dTcor);
     fhY0DT04D4best->Fill(hitpos1_local[1],tof1-tof2-dTcor);
     if(fTrbHeader != NULL) fhTISDT04D4best->Fill(fTrbHeader->GetTimeInSpill(),tof1-tof2-dTcor);

     if(iNbMatchedHits>1){
       LOG(DEBUG)<<Form(" Matches>1: %d with first chi2s = %12.1f, %12.1f, %12.1f, %12.1f",iNbMatchedHits,
		    Chi2List[0],Chi2List[1],Chi2List[2],Chi2List[3])
		 <<FairLogger::endl;

       for(Int_t iM=0; iM<iNbMatchedHits; iM++){
	 LOG(DEBUG)<<Form(" Match: %d (%d) with ids = 0x%08x, 0x%08x - 0x%08x, 0x%08x",
			  iM,iNbMatchedHits,pChi2Hit1[iM]->GetAddress(),pHit1->GetAddress(),
			  pChi2Hit2[iM]->GetAddress(),pHit2->GetAddress()
		   )
 	     <<FairLogger::endl;
	 if(pChi2Hit1[iM] != pHit1 && pChi2Hit2[iM] != pHit2){

	 LOG(DEBUG)<<Form(" second best match D4 at %d (%d): chi2 %f ",iM,iNbMatchedHits,Chi2List[iM])
		    <<FairLogger::endl;

         if (Chi2List[iM] > 1.E3) break; // FIXME hardwired limit !

	 pHit3=pChi2Hit1[iM];
	 pHit4=pChi2Hit2[iM];

	 /*Int_t iDetId3 = (pHit1->GetAddress() & DetMask);*/
	 Int_t iChId3 = pHit1->GetAddress();
	 fChannelInfo3 = fDigiPar->GetCell( iChId3 );

	 /*Int_t iDetId4 = (pHit4->GetAddress() & DetMask);*/
	 Int_t iChId4 = pHit4->GetAddress();
	 fChannelInfo4 = fDigiPar->GetCell( iChId4 );

	 // check for dependence in counter reference frame
	 /*TGeoNode *fNode4= */       // prepare global->local trafo
	   gGeoManager->FindNode(fChannelInfo4->GetX(),fChannelInfo4->GetY(),fChannelInfo4->GetZ());

	 hitpos4[0]=pChi2Hit2[iM]->GetX();
	 hitpos4[1]=pChi2Hit2[iM]->GetY();
	 hitpos4[2]=pChi2Hit2[iM]->GetZ();

/*	 cNode=*/ gGeoManager->GetCurrentNode(); // -> Comment to remove warning because set but never used
	 gGeoManager->MasterToLocal(hitpos4, hitpos4_local);

	 if(TMath::Abs(hitpos4_local[1])>fdPosY4Sel*fChannelInfo4->GetSizey()) continue;

	 fhChi04D4sbest->Fill(Chi2List[iM]);

	 Double_t xPos3=Zref/pChi2Hit1[iM]->GetZ()*pChi2Hit1[iM]->GetX();
	 Double_t yPos3=Zref/pChi2Hit1[iM]->GetZ()*pChi2Hit1[iM]->GetY();
	 Double_t tof3=pChi2Hit1[iM]->GetTime();

	 Double_t xPos4=Zref/pChi2Hit2[iM]->GetZ()*pChi2Hit2[iM]->GetX();
	 Double_t yPos4=Zref/pChi2Hit2[iM]->GetZ()*pChi2Hit2[iM]->GetY();
	 Double_t tof4=pChi2Hit2[iM]->GetTime();

	 Double_t dDist34=TMath::Sqrt(
				TMath::Power(pHit3->GetX()-pHit4->GetX(),2)
			       +TMath::Power(pHit3->GetY()-pHit4->GetY(),2)
			       +TMath::Power(pHit3->GetZ()-pHit4->GetZ(),2)
				);

	 CbmMatch* digiMatch3=(CbmMatch *)fTofDigiMatchColl->At(pHit3->GetRefId());
	 fhDigiMul0D4sbest->Fill(digiMatch3->GetNofLinks()/2.);
	 CbmMatch* digiMatch4=(CbmMatch *)fTofDigiMatchColl->At(pHit4->GetRefId());
	 fhDigiMul4D4sbest->Fill(digiMatch4->GetNofLinks()/2.);

	 fhCluMul04D4sbest->Fill(dMul0,dMul4);

	 Double_t dTofD44  = fdTOffD4 + pHit4->GetTime()-dTDia;
	 Double_t dInvVel4 = dTofD44/pHitRef->GetR(); // in ps/cm
	 Double_t dDTexp4  = dDist34*dInvVel;
	 Double_t dTcor4=0.;
	 if(fhDTD4DT04D4Off != NULL) dTcor4=(Double_t)fhDTD4DT04D4Off->GetBinContent(fhDTD4DT04D4Off->FindBin(dTDia-pHit4->GetTime()));
	 if(fhDTX4D4Off     != NULL) dTcor4+=(Double_t)fhDTX4D4Off->GetBinContent(fhDTX4D4Off->FindBin(hitpos4_local[0]));
	 if(fhDTY4D4Off     != NULL) dTcor4+=(Double_t)fhDTY4D4Off->GetBinContent(fhDTY4D4Off->FindBin(hitpos4_local[1]));
	 if(fhDTTexpD4Off   != NULL) dTcor4+=(Double_t)fhDTTexpD4Off->GetBinContent(fhDTTexpD4Off->FindBin(dDTexp4));

	 fhTofD4sbest->Fill(dTofD44);
	 if(dInvVel>0.) fhVelD4sbest->Fill(1000./dInvVel4);

	 fhChiDT04D4sbest->Fill(Chi2List[iM],tof3-tof4-dTcor4);

	 fhDTD4DT04D4sbest->Fill(dTDia-pHit4->GetTime(),tof3-tof4-dTcor4);
	 fhDTMul4D4sbest->Fill(dMul4,tof3-tof4-dTcor4);

	 fhDTX4D4sbest->Fill(hitpos4_local[0],tof3-tof4-dTcor4);
	 fhDTY4D4sbest->Fill(hitpos4_local[1],tof3-tof4-dTcor4);
	 fhDXX4D4sbest->Fill(hitpos4_local[0],xPos3-xPos4);
	 fhDXY4D4sbest->Fill(hitpos4_local[1],xPos3-xPos4);
	 fhDYX4D4sbest->Fill(hitpos4_local[0],yPos3-yPos4);
	 fhDYY4D4sbest->Fill(hitpos4_local[1],yPos3-yPos4);

	 fhCluSize4DT04D4sbest->Fill(digiMatch4->GetNofLinks()/2.,tof3-tof4-dTcor4);

	 Double_t dTot4 = 0.;
	 for (Int_t iLink=0; iLink<digiMatch4->GetNofLinks(); iLink++){  // loop over digis
	   CbmLink L0 = digiMatch4->GetLink(iLink);  
	   Int_t iDigInd0=L0.GetIndex();
	   if (iDigInd0 < fTofDigisColl->GetEntries()){
            CbmTofDigiExp *pDig0 = (CbmTofDigiExp*) (fTofDigisColl->At(iDigInd0));
	    dTot4 += pDig0->GetTot();
	    LOG(DEBUG)<<Form(" dTot4 of hit 0x%08x: digind %d add %f -> sum %f",iDetId2,iDigInd0,pDig0->GetTot(),dTot4)
		     <<FairLogger::endl;
	   }
	 } 
	 dTot4 /= digiMatch4->GetNofLinks();  // average time over threshold
	 fhTot4DT04D4sbest->Fill(dTot4,tof3-tof4-dTcor4);

	 fhCluSize0DT04D4sbest->Fill(digiMatch3->GetNofLinks()/2.,tof3-tof4-dTcor4);

	 Double_t dTot3 = 0.;
	 for (Int_t iLink=0; iLink<digiMatch3->GetNofLinks(); iLink++){  // loop over digis
	   CbmLink L0 = digiMatch3->GetLink(iLink);  
	   Int_t iDigInd0=L0.GetIndex();
	   if (iDigInd0 < fTofDigisColl->GetEntries()){
	     CbmTofDigiExp *pDig0 = (CbmTofDigiExp*) (fTofDigisColl->At(iDigInd0));
	     dTot3 += pDig0->GetTot();
	     LOG(DEBUG)<<Form(" dTot of hit 0x%08x: digind %d add %f -> sum %f",iDetId1,iDigInd0,pDig0->GetTot(),dTot3)
		       <<FairLogger::endl;
	   }
	 } 
	 dTot3 /= digiMatch3->GetNofLinks();  // average time over threshold
	 fhTot0DT04D4sbest->Fill(dTot3,tof3-tof4-dTcor4);

	 fhDTMul0D4sbest->Fill(dMul0,tof3-tof4-dTcor4);

	 // check for dependence in counter reference frame
	 /*TGeoNode *fNode3=*/        // prepare global->local trafo
	   gGeoManager->FindNode(fChannelInfo3->GetX(),fChannelInfo3->GetY(),fChannelInfo3->GetZ());

	 hitpos3[0]=pChi2Hit1[iM]->GetX();
	 hitpos3[1]=pChi2Hit1[iM]->GetY();
	 hitpos3[2]=pChi2Hit1[iM]->GetZ();

	 /*TGeoNode* cNode3=*/ gGeoManager->GetCurrentNode();
	 gGeoManager->MasterToLocal(hitpos3, hitpos3_local);

	 fhDTX0D4sbest->Fill(hitpos3_local[0],tof3-tof4-dTcor4);
	 fhDTY0D4sbest->Fill(hitpos3_local[1],tof4-tof4-dTcor4);
	 fhDXX0D4sbest->Fill(hitpos3_local[0],xPos3-xPos4);
	 fhDXY0D4sbest->Fill(hitpos3_local[1],xPos3-xPos4);
	 fhDYX0D4sbest->Fill(hitpos3_local[0],yPos3-yPos4);
	 fhDYY0D4sbest->Fill(hitpos3_local[1],yPos3-yPos4);

	 fhDXDY04D4sbest->Fill(xPos3-xPos4,yPos3-yPos4);
	 fhDXDT04D4sbest->Fill(xPos3-xPos4,tof3-tof4-dTcor4);
	 fhDYDT04D4sbest->Fill(yPos3-yPos4,tof3-tof4-dTcor4);
	 fhDistDT04D4sbest->Fill(dDist34,tof3-tof4-dTcor4);
	 fhTexpDT04D4sbest->Fill(dDTexp4,tof3-tof4-dTcor4);
	 fhX0DT04D4sbest->Fill(hitpos3_local[0],tof3-tof4-dTcor4);
	 fhY0DT04D4sbest->Fill(hitpos3_local[1],tof3-tof4-dTcor4);

	 fhDT04DX0_2->Fill(hitpos1_local[0]-hitpos3_local[0],tof3-tof4-dTcor4);
	 fhDT04DY0_2->Fill(hitpos1_local[1]-hitpos3_local[1],tof3-tof4-dTcor4);
	 fhDT04DT0_2->Fill(tof1-tof3,tof3-tof4-dTcor4);

	 fhDT04DX4_2->Fill(hitpos2_local[0]-hitpos4_local[0],tof3-tof4-dTcor4);
	 fhDT04DY4_2->Fill(hitpos2_local[1]-hitpos4_local[1],tof3-tof4-dTcor4);
	 fhDT04DT4_2->Fill(tof2-tof4,tof3-tof4-dTcor4);

	 fhDT04DX0_1->Fill(hitpos1_local[0]-hitpos3_local[0],tof1-tof2-dTcor);
	 fhDT04DY0_1->Fill(hitpos1_local[1]-hitpos3_local[1],tof1-tof2-dTcor);
	 fhDT04DT0_1->Fill(tof1-tof3,tof1-tof2-dTcor);

	 fhDT04DX4_1->Fill(hitpos2_local[0]-hitpos4_local[0],tof1-tof2-dTcor);
	 fhDT04DY4_1->Fill(hitpos2_local[1]-hitpos4_local[1],tof1-tof2-dTcor);
	 fhDT04DT4_1->Fill(tof2-tof4,tof1-tof2-dTcor);

	 break;
	 }
       }
     }
    } // end of if(iNbMatchedHits>0)
   }

 // Tracklet based analysis


   if(NULL!=fTofTrackColl){
     iNbTofTracks  = fTofTrackColl->GetEntries();
     Int_t NStations = fFindTracks->GetNStations();
     LOG(DEBUG)<<Form("Tracklet analysis of %d tracklets from %d stations",iNbTofTracks,NStations)<< FairLogger::endl;

     if(iNbTofTracks>0){   // Tracklet Analysis
                           // prepare Dut Hit List

       Int_t iChIdDut   = CbmTofAddress::GetUniqueAddress(fiDutSm,fiDutRpc,0,0,fiDut);
       fChannelInfo = fDigiPar->GetCell( iChIdDut );
       /*TGeoNode *fNode=*/        // prepare global->local trafo
	 gGeoManager->FindNode(fChannelInfo->GetX(),fChannelInfo->GetY(),fChannelInfo->GetZ());
       /*TGeoNode* cNode=*/ gGeoManager->GetCurrentNode();
       Double_t  dDutzPos=fChannelInfo->GetZ();
       Double_t  hitpos[3], hitpos_local[3];

       vector <CbmTofHit *> vDutHit;
       for( Int_t iHitInd = 0; iHitInd < iNbTofHits; iHitInd++)
       {
	 pHit = (CbmTofHit*) fTofHitsColl->At( iHitInd );
	 if(NULL == pHit) continue;
	 Int_t iDetId = (pHit->GetAddress() & DetMask);
	 Int_t iSmType=CbmTofAddress::GetSmType( iDetId );
	 Int_t iSm    =CbmTofAddress::GetSmId( iDetId );
	 Int_t iRpc   =CbmTofAddress::GetRpcId(iDetId );

	 if(iSmType == fiDut && iSm==fiDutSm && iRpc==fiDutRpc) {
	   vDutHit.push_back(pHit);
	 }
       }
	
       LOG(DEBUG)<<Form("Tracklet analysis of %d tracks and %d Dut Hits ",iNbTofTracks,(int)vDutHit.size())<< FairLogger::endl;

       std::vector<std::map <Double_t, Int_t> > vTrkMap; //contains the tracks for a given hit
       std::vector<std::map <Double_t, Int_t> > vHitMap; //contains the hits for a given track

       vTrkMap.resize(vDutHit.size());
       vHitMap.resize(iNbTofTracks);

       for (Int_t iTrk=0; iTrk<iNbTofTracks;iTrk++) { // loop over all Tracklets
	 CbmTofTracklet *pTrk = (CbmTofTracklet*)fTofTrackColl->At(iTrk);
	 if(NULL == pTrk) continue;
	 if (pTrk->GetNofHits() < NStations) continue;  

	 // Calculate positions and time in Dut plane
	 Double_t dXex=pTrk->GetFitX(dDutzPos);
	 Double_t dYex=pTrk->GetFitY(dDutzPos);
	 Double_t dR=TMath::Sqrt(dXex*dXex + dYex*dYex + dDutzPos*dDutzPos);
	 Double_t dTex=pTrk->GetFitT(dR);

	 for (UInt_t i=0; i<vDutHit.size();i++){ // loop over Dut Hits
	  Double_t dChi = TMath::Sqrt(TMath::Power(TMath::Abs(dTex-vDutHit[i]->GetTime())/fSIGT,2)
				     +TMath::Power(TMath::Abs(dXex-vDutHit[i]->GetX())/fSIGX,2)
	   			     +TMath::Power(TMath::Abs(dYex-vDutHit[i]->GetY())/fSIGY,2))/3;
	  LOG(DEBUG1)<<Form(" Inspect track %d, hit %d Chi %6.2f,%6.2f, T  %6.2f,%6.2f ",
			    iTrk,i,dChi,fSIGLIM,dTex,vDutHit[i]->GetTime())
		    << FairLogger::endl;

	  if(dChi < fSIGLIM) {       // acceptable match
            if(vHitMap[iTrk].size()>0) {
	      Int_t iCnt=0;
	      for ( std::map<Double_t,Int_t>::iterator it=vHitMap[iTrk].begin(); it!=vHitMap[iTrk].end(); it++){
		iCnt++;
		// LOG(DEBUG)<<Form(" HitMap[%d]: cnt %d, check %d, %6.2f > %6.2f ?",iTrk,iCnt, it->second,it->first,dChi)
		// << FairLogger::endl;
                if(it->first > dChi) {
		  vHitMap[iTrk].insert(--it,std::pair<Double_t, Int_t>(dChi,i));
		  //LOG(DEBUG)<<Form(" HitMap[%d]: ins at %d:  %d, %6.2f ",iTrk,iCnt,it->second,it->first)<< FairLogger::endl;
		  break;
		}
	      }
	    }
	    else{
	      vHitMap[iTrk].insert(std::pair<Double_t, Int_t>(dChi,i));
	      //LOG(DEBUG)<<Form(" HitMap[%d]:ins  %d, %6.2f ",iTrk,i,dChi)<< FairLogger::endl;
	    }

            if(vTrkMap[i].size()>0) {
	      for ( std::map<Double_t,Int_t>::iterator it=vTrkMap[i].begin(); it!=vTrkMap[i].end(); it++){
               if(it->first > dChi) {
		  vTrkMap[i].insert(--it,std::pair<Double_t, Int_t>(dChi,iTrk));
		  break;
		}	
	      }
	    }
	    else{
	      vTrkMap[i].insert(std::pair<Double_t, Int_t>(dChi,iTrk));
	    }

	  }  // end of Chi condition 
	  if(vTrkMap[i].size()>0)
	  LOG(DEBUG1)<<Form(" TrkMap[%d]: best %d, %6.4f ",i,vTrkMap[i].begin()->second,vTrkMap[i].begin()->first)
		    <<FairLogger::endl;
	 }
	 if(vHitMap[iTrk].size()>0)
	 LOG(DEBUG)<<Form(" HitMap[%d]: best %d, %6.4f ",iTrk,vHitMap[iTrk].begin()->second,vHitMap[iTrk].begin()->first)
		   <<FairLogger::endl;
       } // tracklet loop end 
       
       // inspect assignment results
       Int_t iCheck = 1;
       while(iCheck-- > 0)
       for(Int_t iHit=0; static_cast<UInt_t>(iHit)<vDutHit.size(); iHit++) {
	 if(vTrkMap[iHit].size()>0){ 
	   Int_t iTrk=vTrkMap[iHit].begin()->second;     // hit was assigned best to track iTrk
	   if(vHitMap[iTrk].begin()->second == iHit) {   // unique/consistent assignment
	     LOG(DEBUG)<<Form(" Hit %d -> HitMap[%d]: uni %d, %6.4f ",iHit,iTrk,vHitMap[iTrk].begin()->second,vHitMap[iTrk].begin()->first)
		       <<FairLogger::endl;
	     // remove all other assignments of this hit and this track
	     for ( std::map<Double_t,Int_t>::iterator it=vTrkMap[iHit].begin()++; it != vTrkMap[iHit].end(); it++){
	       Int_t iTrk1=it->second;
	       if(iTrk != iTrk1)
	       for ( std::map<Double_t,Int_t>::iterator it1=vHitMap[iTrk1].begin()++; it1 != vHitMap[iTrk1].end(); it1++){
		 if(it1->second == iHit) {
		   vHitMap[iTrk1].erase(it1);
  	           LOG(DEBUG1)<<Form("    Erase hit %d from  HitMap[%d]",iHit,iTrk1)<<FairLogger::endl;
		 }
	       }
	     }
	     for ( std::map<Double_t,Int_t>::iterator it=vHitMap[iTrk].begin()++; it != vHitMap[iTrk].end(); it++){
	       Int_t iHit1=it->second;
	       if(iHit != iHit1)
	       for ( std::map<Double_t,Int_t>::iterator it1=vTrkMap[iHit1].begin()++; it1 != vTrkMap[iHit1].end(); it1++){
		 if(it1->second == iTrk) {
		   vTrkMap[iHit1].erase(it1);
  	           LOG(DEBUG1)<<Form("    Erase trk %d from  TrkMap[%d]",iHit1,iTrk)<<FairLogger::endl;
		 }
	       }
	     }
	   }else{      // mismatch, other track fits even better
	     LOG(DEBUG)<<Form(" Hit %d -> HitMap[%d]: mis %d, %6.4f < %6.4f ",iHit,iTrk,
			      vHitMap[iTrk].begin()->second,vHitMap[iTrk].begin()->first,vTrkMap[iHit].begin()->first)
		       <<FairLogger::endl;
	   }
	   LOG(DEBUG)<<Form(" Hit %d -> TrkMap.size:  %d  ",iHit,(int)vTrkMap[iHit].size())
		     <<FairLogger::endl;

	 }
       }


       // fill tracklet histos 
       for (Int_t iTrk=0; iTrk<iNbTofTracks;iTrk++) { // loop over all Tracklets
	 CbmTofTracklet *pTrk = (CbmTofTracklet*)fTofTrackColl->At(iTrk);
	 if(NULL == pTrk) continue;
	 if (pTrk->GetNofHits() < NStations) continue;  

	 // Calculate positions and time in Dut plane
/*	 Double_t dXex=pTrk->GetFitX(dDutzPos);*/
/*	 Double_t dYex=pTrk->GetFitY(dDutzPos);*/
/*	 Double_t dTex=pTrk->GetFitT(dDutzPos);*/

	 hitpos[0]=pTrk->GetFitX(dDutzPos);
	 hitpos[1]=pTrk->GetFitY(dDutzPos);
	 hitpos[2]=dDutzPos;
	 gGeoManager->MasterToLocal(hitpos, hitpos_local);
	 if(vHitMap[iTrk].size()>0){  // matched hit found 
	   LOG(DEBUG)<<Form(" Fill/process Trk %d, HMul %d,  with DutHitMap.size:  %d,  best hit %d, %6.2f ",
			    iTrk,pTrk->GetNofHits(),(int)vHitMap[iTrk].size(),vHitMap[iTrk].begin()->second,vHitMap[iTrk].begin()->first)
		     <<FairLogger::endl;

	   pHit = vDutHit[vHitMap[iTrk].begin()->second];
	   Double_t dDX = pHit->GetX() - pTrk->GetFitX(pHit->GetZ());    // - tPar->GetX() - tPar->GetTx()*dDZ;
	   Double_t dDY = pHit->GetY() - pTrk->GetFitY(pHit->GetZ());    // - tPar->GetTy()*dDZ;
	   Double_t dDT = pHit->GetTime() - pTrk->GetFitT(pHit->GetR()); //pTrk->GetTdif(fStationType[iSt]);
	   Double_t dDTB= pTrk->GetTdif(fiDut, pHit);                    // ignore pHit in calc of reference
	   fhDutPullX->Fill(dDX);
	   fhDutPullY->Fill(dDY);
	   fhDutPullT->Fill(dDT);
	   fhDutPullTB->Fill(dDTB);
	   fhDutChiFound->Fill(pTrk->GetChiSq());
	   fhDutChiMatch->Fill(vHitMap[iTrk].begin()->first);
	   fhDutXY_Found->Fill(hitpos_local[0],hitpos_local[1]);  
	   fhDutXYDT->Fill(hitpos_local[0],hitpos_local[1],dDTB);
  
	 }else{                       // no match for this track
	   fhDutChiMissed->Fill(pTrk->GetChiSq());
	   fhDutXY_Missed->Fill(hitpos_local[0],hitpos_local[1]);
	 }                 
       }

     } // #tracklets > 0 end
   }  // TclonesArray existing end
   return kTRUE;  
}  //FillHistos
// ------------------------------------------------------------------

Bool_t CbmTofAnaTestbeam::WriteHistos()
{

   LOG(INFO)<<"CbmTofAnaTestbeam::WriteHistos: ./tofAnaTestBeam.hst.root, mode = "
	    << fiCorMode
	    << FairLogger::endl;

   // Write histogramms to the file
   TDirectory * oldir = gDirectory;
   TFile *fHist = new TFile("./tofAnaTestBeam.hst.root","RECREATE");
   fHist->cd();

   switch(fiCorMode){
   case 1 : {
     TProfile *htmp=fhDTD4DT04D4best->ProfileX();
     TH1D *htmp1D=htmp->ProjectionX();

     //     htmp1D->Draw();

     if(fhDTD4DT04D4Off != NULL){
       // fhDTD4DT04D4Off->Draw("same");
       // fhDTD4DT04D4Off->Write();
       //LOG(INFO)<<"Update hDTD4DT04D4best"<<FairLogger::endl;
       Double_t nx=htmp1D->GetNbinsX();
       for (Int_t ix=0; ix<nx; ix++){
	 Double_t dVal=htmp1D->GetBinContent(ix) + fhDTD4DT04D4Off->GetBinContent(ix);
	 // Double_t dVal=fhDTD4DT04D4Off->GetBinContent(ix);
	 LOG(DEBUG1)<<"Update hDTD4DT04D4best "<<ix<<": "<<htmp1D->GetBinContent(ix)<<" + "
		  << fhDTD4DT04D4Off->GetBinContent(ix) << " -> " << dVal << FairLogger::endl;
	 htmp1D->SetBinContent(ix,dVal);
       }
     }else
     {
       LOG(WARNING)<<"CbmTofAnaTestbeam::WriteHistos: fhDTD4DT04D4Off not found "
		   << FairLogger::endl;
     }
   //   fhDTD4DT04D4best->Write();
     htmp1D->Write();
     if(fhDTX4D4Off != NULL)    fhDTX4D4Off->Write();
     if(fhDTY4D4Off != NULL)    fhDTY4D4Off->Write();
     if(fhDTTexpD4Off != NULL)  fhDTTexpD4Off->Write();
     }
     break;
   case 2 :{
     TProfile *htmpx=fhDTX4D4best->ProfileX();
     TH1D *htmpx1D=htmpx->ProjectionX();
     if(fhDTX4D4Off != NULL){
       Double_t nx=htmpx1D->GetNbinsX();
       for (Int_t ix=0; ix<nx; ix++){
	 Double_t dVal=htmpx1D->GetBinContent(ix) + fhDTX4D4Off->GetBinContent(ix);
	 LOG(DEBUG1)<<"Update hDTX4D4best "<<ix<<": "<<htmpx1D->GetBinContent(ix)<<" + "
		    << fhDTX4D4Off->GetBinContent(ix) << " -> " << dVal << FairLogger::endl;
	 htmpx1D->SetBinContent(ix,dVal);
       }
     }else
     {
       LOG(WARNING)<<"CbmTofAnaTestbeam::WriteHistos: fhDTX4D4Off not found "
		   << FairLogger::endl;
     }
     htmpx1D->Write();
     if(fhDTD4DT04D4Off != NULL) fhDTD4DT04D4Off->Write();
     if(fhDTY4D4Off != NULL)     fhDTY4D4Off->Write();
     if(fhDTTexpD4Off != NULL)   fhDTTexpD4Off->Write();
     }
     break;
   case 3 :{
     TProfile *htmpx=fhDTY4D4best->ProfileX();
     TH1D *htmpx1D=htmpx->ProjectionX();
     if(fhDTY4D4Off != NULL){
       Double_t nx=htmpx1D->GetNbinsX();
       for (Int_t ix=0; ix<nx; ix++){
	 Double_t dVal=htmpx1D->GetBinContent(ix) + fhDTY4D4Off->GetBinContent(ix);
	 LOG(DEBUG1)<<"Update hDTY4D4best "<<ix<<": "<<htmpx1D->GetBinContent(ix)<<" + "
		    << fhDTY4D4Off->GetBinContent(ix) << " -> " << dVal << FairLogger::endl;
	 htmpx1D->SetBinContent(ix,dVal);
       }
     }else
     {
       LOG(WARNING)<<"CbmTofAnaTestbeam::WriteHistos: fhDTY4D4Off not found "
		   << FairLogger::endl;
     }
     htmpx1D->Write();
     if(fhDTD4DT04D4Off != NULL) fhDTD4DT04D4Off->Write();
     if(fhDTX4D4Off != NULL)     fhDTX4D4Off->Write();
     if(fhDTTexpD4Off != NULL)   fhDTTexpD4Off->Write();
     }
     break;

   case 4 :{
     TProfile *htmpx=fhTexpDT04D4best->ProfileX();
     TH1D *htmpx1D=htmpx->ProjectionX();
     if(fhDTTexpD4Off != NULL){
       Double_t nx=htmpx1D->GetNbinsX();
       for (Int_t ix=0; ix<nx; ix++){
	 Double_t dVal=htmpx1D->GetBinContent(ix) + fhDTTexpD4Off->GetBinContent(ix);
	 LOG(DEBUG1)<<"Update hDTTexpD4best "<<ix<<": "<<htmpx1D->GetBinContent(ix)<<" + "
		    << fhDTTexpD4Off->GetBinContent(ix) << " -> " << dVal << FairLogger::endl;
	 htmpx1D->SetBinContent(ix,dVal);
       }
     }else
     {
       LOG(WARNING)<<"CbmTofAnaTestbeam::WriteHistos: fhDTTexpD4Off not found "
		   << FairLogger::endl;
     }
     htmpx1D->Write();
     if(fhDTD4DT04D4Off != NULL) fhDTD4DT04D4Off->Write();
     if(fhDTX4D4Off != NULL)     fhDTX4D4Off->Write();
     if(fhDTY4D4Off != NULL)     fhDTY4D4Off->Write();
     }
     break;

   default:
     ;
   }

   //   fHist->Write();
   if(0) {
   fhXX2->Write();
   fhYY2->Write();
   for (Int_t iDet=0; iDet<2; iDet++)
   {
     fhXX02[iDet]->Write();
     fhYY02[iDet]->Write(); 
   }
   fhXX04->Write();
   fhYY04->Write();
   fhXY04->Write();
   fhYX04->Write();
   }
   gDirectory->cd( oldir->GetPath() );

   fHist->Close();
   return kTRUE;
}

Bool_t   CbmTofAnaTestbeam::DeleteHistos()
{
   // Test class performance

   // Mapping

   return kTRUE;
}

ClassImp(CbmTofAnaTestbeam);
