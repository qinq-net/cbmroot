// -------------------------------------------------------------------------
// -----                   CbmTrdFindTracks source file                -----
// -----                  Created 25/04/15  by N. Herrmann             -----
// -----                  initially following  CbmTrdFindTracks        -----
// -------------------------------------------------------------------------

#include "CbmTofFindTracks.h"

#include "CbmTofTracklet.h"
#include "CbmTofTrackFinder.h"
#include "CbmTofTrackFinderNN.h"
#include "CbmTofTrackFitter.h"
#include "CbmTofHit.h"
#include "CbmTofGeoHandler.h" // in tof/TofTools
#include "CbmTofDetectorId_v12b.h" // in cbmdata/tof
#include "CbmTofDetectorId_v14a.h" // in cbmdata/tof
#include "CbmTofCell.h"       // in tof/TofData
#include "CbmTofDigiPar.h"    // in tof/TofParam
#include "CbmTofDigiBdfPar.h" // in tof/TofParam
#include "CbmTofAddress.h"    // in cbmdata/tof
#include "CbmMatch.h"

#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairLogger.h"

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

#include <vector>
#include <iostream>

using std::cout;
using std::endl;
using std::vector;

const Int_t DetMask = 4194303; // check for consistency with geometry

ClassImp(CbmTofFindTracks);

CbmTofFindTracks *CbmTofFindTracks::fInstance = 0;

// -----   Default constructor   -------------------------------------------
CbmTofFindTracks::CbmTofFindTracks()  : FairTask(),
    fFinder(NULL),
    fFitter(NULL),
    fTofHitArray(NULL),
    fTrackArray(NULL),
    fMinNofHits(-1),
    fNofTracks(-1),
    fNTofStations(-1),
    fStationType(),
    fhTrklMul(NULL),
    fhTrklChi2(NULL),
    fhAllHitsStation(NULL),
    fhAllHitsSmTypes(NULL),
    fhUsedHitsStation(NULL),
    fhTrklMulNhits(NULL),
    fhTrklMul34(NULL),
    fhTrklMul3D(NULL),
    fhTrklHMul(NULL),
    fhTrklZ0xHMul(NULL),
    fhTrklZ0yHMul(NULL),
    fhTrklTxHMul(NULL),
    fhTrklTyHMul(NULL),
    fhTrklTtHMul(NULL),
    fhTrklVelHMul(NULL),
    fhTrklT0HMul(NULL),
    fhTrklT0Mul(NULL),
    fhTrklDT0SmMis(NULL),
    fhTrklDT0StMis2(NULL),
    fhTrklXY0_3(NULL),
    fhTrklXY0_4(NULL),
    fhTrklXY0_5(NULL),
    vhPullX(),
    vhPullY(),
    vhPullZ(),
    vhPullT(),
    vhPullTB(),
    vhXY_AllStations(),
    vhXY_MissedStation(),
    fhVTXNorm(NULL),
    fhVTX_XY0(NULL),
    fhVTX_DT0_Norm(NULL),
    fOutHstFileName(""),
    fCalParFileName(""),
    fCalParFile(NULL),
    fhPullT_Smt(NULL),
    fhPullT_Smt_Off(NULL),
    fhTOff_Smt(NULL),
    fhTOff_Smt_Off(NULL),
    fhDeltaTt_Smt(NULL),
    fiCorMode(0),
    fTtTarg(30.),
    fVTXNorm(0.),
    fVTX_T(0.),
    fVTX_X(0.),
    fVTX_Y(0.),
    fVTX_Z(0.),
    fDigiPar(NULL),
    fDigiBdfPar(NULL)
{
  if( !fInstance ) fInstance = this;
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmTofFindTracks::CbmTofFindTracks(const char* name,
				   const char* title,
				   CbmTofTrackFinder* finder)
  : FairTask(name),
    fFinder(finder),
    fFitter(NULL),
    fTofHitArray(NULL),
    fTrackArray(NULL),
    fMinNofHits(-1),
    fNofTracks(-1),
    fNTofStations(-1),
    fStationType(),
    fhTrklMul(NULL),
    fhTrklChi2(NULL),
    fhAllHitsStation(NULL),
    fhAllHitsSmTypes(NULL),
    fhUsedHitsStation(NULL),
    fhTrklMulNhits(NULL),
    fhTrklMul34(NULL),
    fhTrklMul3D(NULL),
    fhTrklHMul(NULL),
    fhTrklZ0xHMul(NULL),
    fhTrklZ0yHMul(NULL),
    fhTrklTxHMul(NULL),
    fhTrklTyHMul(NULL),
    fhTrklTtHMul(NULL),
    fhTrklVelHMul(NULL),
    fhTrklT0HMul(NULL),
    fhTrklT0Mul(NULL),
    fhTrklDT0SmMis(NULL),
    fhTrklDT0StMis2(NULL),
    fhTrklXY0_3(NULL),
    fhTrklXY0_4(NULL),
    fhTrklXY0_5(NULL),
    vhPullX(),
    vhPullY(),
    vhPullZ(),
    vhPullT(),
    vhPullTB(),
    vhXY_AllStations(),
    vhXY_MissedStation(),
    fhVTXNorm(NULL),
    fhVTX_XY0(NULL),
    fhVTX_DT0_Norm(NULL),
    fOutHstFileName(""),
    fCalParFileName(""),
    fCalParFile(NULL),
    fhPullT_Smt(NULL),
    fhPullT_Smt_Off(NULL),
    fhTOff_Smt(NULL),
    fhTOff_Smt_Off(NULL),
    fhDeltaTt_Smt(NULL),
    fiCorMode(0),
    fTtTarg(30.),
    fVTXNorm(0.),
    fVTX_T(0.),
    fVTX_X(0.),
    fVTX_Y(0.),
    fVTX_Z(0.),
    fDigiPar(NULL),
    fDigiBdfPar(NULL)
{
    if ( !fInstance ) fInstance = this;
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmTofFindTracks::~CbmTofFindTracks()
{
  if( fInstance==this ) fInstance = 0;
  fTrackArray->Delete();
}
// -------------------------------------------------------------------------



// -----   Public method Init (abstract in base class)  --------------------
InitStatus CbmTofFindTracks::Init()
{

  // Check for Track finder
  if (! fFinder) {
    cout << "-W- CbmTofFindTracks::Init: No track finder selected!" << endl;
    return kERROR;
  }

  // Check for Track fitter
  if (! fFitter) {
    cout << "-W- CbmTofFindTracks::Init: No track fitter selected!" << endl;
    return kERROR;
  }
  cout << Form("-D- CbmTofFindTracks::Init: track fitter at 0x%p", fFitter) << endl;

  // Get and check FairRootManager
  FairRootManager* ioman = FairRootManager::Instance();
  if (! ioman) {
    cout << "-E- CbmTofFindTracks::Init: "
	 << "RootManager not instantised!" << endl;
    return kFATAL;
  }

  // Get TOF hit Array
  fTofHitArray
    = (TClonesArray*) ioman->GetObject("TofHit");
  if ( ! fTofHitArray) {
    cout << "-W- CbmTofFindTracks::Init: No TofHit array!"
	 << endl;
    return kERROR;
  }

  // Create and register TofTrack array
  fTrackArray = new TClonesArray("CbmTofTracklet",100);
  ioman->Register("TofTracks", "TOF", fTrackArray, kFALSE); // kTRUE); //FIXME 
    cout << "-I- CbmTofFindTracks::Init:TofTrack array registered"
	 << endl;

  // Call the Init method of the track finder
  fFinder->Init();

  if (fOutHstFileName==""){
     fOutHstFileName="./FindTofTracks.hst.root";
   }
  LOG(INFO)<<"CbmTofFindTracks::Init: Hst Output filename = "<<fOutHstFileName
           <<FairLogger::endl;

  // default parameters
  if (fMinNofHits < 1) fMinNofHits=1;

  LoadCalParameter();

  CreateHistograms();

  return kSUCCESS;

}
// -------------------------------------------------------------------------
/************************************************************************************/
Bool_t   CbmTofFindTracks::LoadCalParameter()
{
    if(fCalParFileName.IsNull()) return kTRUE;

    fCalParFile = new TFile(fCalParFileName,"");
    if(NULL == fCalParFile) {
      LOG(ERROR) << "CbmTofFindTracks::LoadCalParameter: "
		 << "file " << fCalParFileName << " does not exist!" << FairLogger::endl;
      return kTRUE;
    }

    TProfile *fhtmp=(TProfile *) gDirectory->FindObjectAny( Form("hPullT_Smt_pfx_px"));
    if (NULL == fhtmp) {
      LOG(INFO)<<" Histo " << Form("hPullT_Smt_pfx_px") << " not found. "
             <<FairLogger::endl;
    }

    gROOT->cd();
    fhPullT_Smt_Off = NULL;
    if(NULL != fhtmp) {
      fhPullT_Smt_Off = (TH1D *)fhtmp->Clone();
      fhPullT_Smt_Off->SetName("hPullT_Smt_Off");
    }

    fCalParFile->Close();

    LOG(INFO)<<"CbmTofFindTracks::LoadCalParameter: fhPullT_Smt_Off at "<<fhPullT_Smt_Off<<FairLogger::endl;
    return kTRUE;
}


// -----  SetParContainers -------------------------------------------------
void CbmTofFindTracks::SetParContainers()
{
    FairRunAna* ana = FairRunAna::Instance();
    FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  //  rtdb->getContainer("CbmGeoPassivePar");
 //   rtdb->getContainer("CbmGeoStsPar");
 //   rtdb->getContainer("CbmGeoTofPar");
    rtdb->getContainer("FairBaseParSet");
    //    rtdb->getContainer("CbmGeoPassivePar");
    // rtdb->getContainer("CbmGeoStsPar");
    // rtdb->getContainer("CbmGeoRichPar");
    rtdb->getContainer("CbmGeoTofPar");
    // rtdb->getContainer("CbmFieldPar");
    fDigiPar = (CbmTofDigiPar*) (rtdb->getContainer("CbmTofDigiPar"));

    LOG(INFO)<<"  CbmTofFindTracks::SetParContainers found " 
            << fDigiPar->GetNrOfModules() << " cells " <<FairLogger::endl;
    fDigiBdfPar = (CbmTofDigiBdfPar*)
              (rtdb->getContainer("CbmTofDigiBdfPar"));
}
// -------------------------------------------------------------------------

Bool_t CbmTofFindTracks::WriteHistos()
{
   if ( fiCorMode < 0 ) return kTRUE;

   LOG(INFO)<<"CbmTofFindTracks::WriteHistos: ./tofFindTracks.hst.root, mode = "
	    << fiCorMode
	    << FairLogger::endl;
 
   // Write histogramms to the file
   TDirectory * oldir = gDirectory;
   TFile *fHist = new TFile("./tofFindTracks.hst.root","RECREATE");
   fHist->cd();

   switch(fiCorMode){
   case 0 : 
     {
     TProfile *htmp=fhPullT_Smt->ProfileX();
     TH1D *htmp1D=htmp->ProjectionX();
     TProfile *hTOff=fhTOff_Smt->ProfileX();
     TH1D *hTOff1D=hTOff->ProjectionX();

     if(fhPullT_Smt_Off != NULL){
       Double_t nx=htmp1D->GetNbinsX();
       for (Int_t ix=1; ix<nx; ix++){
	 Double_t dVal  = fhPullT_Smt_Off->GetBinContent(ix+1);
	          dVal -= htmp1D->GetBinContent(ix+1);
	          dVal -= hTOff1D->GetBinContent(ix+1);

	 LOG(INFO)<<"Update hPullT_Smt_Off "<<ix<<": "
		  << fhPullT_Smt_Off->GetBinContent(ix+1) <<" + "
		  << htmp1D->GetBinContent(ix+1)<<" + "
		  << hTOff1D->GetBinContent(ix+1) << " -> " << dVal << FairLogger::endl;
	 htmp1D->SetBinContent(ix+1,dVal);
       }
     }else
     {
       LOG(WARNING)<<"CbmTofFindTracks::WriteHistos: fhPullT_Smt_Off not found "
		   << FairLogger::endl;
     }
     htmp1D->Write();
     }
     break;
   case 1 : 
     {
     TProfile *htmp=fhPullT_Smt->ProfileX();
     TH1D *htmp1D=htmp->ProjectionX();
     TProfile *hTOff=fhTOff_Smt->ProfileX();
     TH1D *hTOff1D=hTOff->ProjectionX();

     if(fhPullT_Smt_Off != NULL){
       Double_t nx=htmp1D->GetNbinsX();
       for (Int_t ix=1; ix<nx; ix++){
	 Double_t dVal  = fhPullT_Smt_Off->GetBinContent(ix+1);
	          dVal -= htmp1D->GetBinContent(ix+1);

	 LOG(INFO)<<"Update hPullT_Smt_Off "<<ix<<": "
		  << fhPullT_Smt_Off->GetBinContent(ix+1) <<" + "
		  << htmp1D->GetBinContent(ix+1)<<" + "
		  << hTOff1D->GetBinContent(ix+1) << " -> " << dVal << FairLogger::endl;
	 htmp1D->SetBinContent(ix+1,dVal);
       }
     }else
     {
       LOG(WARNING)<<"CbmTofFindTracks::WriteHistos: fhPullT_Smt_Off not found "
		   << FairLogger::endl;
     }
     htmp1D->Write();
     }
     break;
   case 2 : 
     {
     TProfile *htmp=fhPullT_Smt->ProfileX();
     TH1D *htmp1D=htmp->ProjectionX();
     TProfile *hTOff=fhTOff_Smt->ProfileX();
     TH1D *hTOff1D=hTOff->ProjectionX();

     if(fhPullT_Smt_Off != NULL){
       Double_t nx=htmp1D->GetNbinsX();
       for (Int_t ix=1; ix<nx; ix++){
	 Double_t dVal  = fhPullT_Smt_Off->GetBinContent(ix+1);
	          dVal -= hTOff1D->GetBinContent(ix+1);

	 LOG(INFO)<<"Update hPullT_Smt_Off "<<ix<<": "
		  << fhPullT_Smt_Off->GetBinContent(ix+1) <<" + "
		  << htmp1D->GetBinContent(ix+1)<<" + "
		  << hTOff1D->GetBinContent(ix+1) << " -> " << dVal << FairLogger::endl;
	 htmp1D->SetBinContent(ix+1,dVal);
       }
     }else
     {
       LOG(WARNING)<<"CbmTofFindTracks::WriteHistos: fhPullT_Smt_Off not found "
		   << FairLogger::endl;
     }
     htmp1D->Write();
     }
     break;

   default: 
     ;
   }
   return kTRUE;
}


// -----   Public method Exec   --------------------------------------------
void CbmTofFindTracks::Exec(Option_t* opt)
{
  // recalibrate hits  
  for (Int_t iHit=0; iHit<fTofHitArray->GetEntries(); iHit++){
    CbmTofHit* pHit = (CbmTofHit*) fTofHitArray->At(iHit);
    Int_t iDetId = (pHit->GetAddress() & DetMask);
    Int_t iSmType=CbmTofAddress::GetSmType( iDetId );
    if(fhPullT_Smt_Off != NULL &&  fTypeStation[iSmType] >-1) {
      Double_t dTcor=(Double_t)fhPullT_Smt_Off->GetBinContent( iSmType + 1 );
      pHit->SetTime(pHit->GetTime()+dTcor);
    }
  }

  //fTrackArray->Clear("C+C");
  fTrackArray->Delete();
  fNofTracks = fFinder->DoFind(fTofHitArray, fTrackArray);

  LOG(DEBUG) << Form("CbmTofFindTracks::Exec found %d Tracklets",fTrackArray->GetEntriesFast())
	     <<FairLogger::endl;

  FindVertex();
  /*
  for (Int_t iTrack=0; iTrack<fTrackArray->GetEntriesFast(); iTrack++) {
    CbmTofTracklet* track = (CbmTofTracklet*) fTrackArray->At(iTrack);
    for (Int_t iTOF=0; iTOF < track->GetNofHits(); iTOF++){
      Int_t TOFindex = track->GetHitIndex(iTOF);
      CbmTofHit* tofHit = (CbmTofHit*) fTofHitArray->At(TOFindex);
    }
  }
  */

  FillHistograms();
}
// -------------------------------------------------------------------------


// -----   Public method Finish   ------------------------------------------
void CbmTofFindTracks::Finish()
{

  WriteHistos();

}
// -------------------------------------------------------------------------

void CbmTofFindTracks::CreateHistograms(){

   TDirectory * oldir = gDirectory; // <= To prevent histos from being sucked in by the param file of the TRootManager!
   gROOT->cd(); // <= To prevent histos from being sucked in by the param file of the TRootManager !

   // define histos here

  LOG(INFO) << Form(" CbmTofFindTracks::CreateHistograms for %d stations ",fNTofStations)
	    <<FairLogger::endl;

  fhTrklMul =  new TH1F(  Form("hTrklMul"),
			  Form("Tracklet Multiplicity; MulTracklet"),
			  100, 0, 100);    

  fhAllHitsStation =  new TH1F(  Form("hAllHitsStation"),
			  Form("Reconstructed Hits; Station #"),
			  fNTofStations, 0, fNTofStations);  
  fhAllHitsSmTypes =  new TH1F(  Form("hAllHitsSmTypes"),
			  Form("Reconstructed Hits; SmType #"),
			  10, 0, 10);   

  fhUsedHitsStation =  new TH1F(  Form("hUsedHitsStation"),
			  Form("Used (HMul>2) / Reconstructed Hits; Station #"),
			  fNTofStations, 0, fNTofStations);    

  fhTrklChi2 =  new TH1F(  Form("hTrklChi2"),
			  Form("Tracklet Chi;  #chi"),
			  100, 0, ((CbmTofTrackFinderNN *)fFinder)->GetSIGLIM());  
  
  fhTrklMulNhits =  new TH2F(  Form("hTrklMulNhits"),
			       Form("Tracklet Multiplicity; NHits; NTracklet"),
			       100, 0, 100, 20, 0, 20);

  fhTrklMul34 =  new TH2F(  Form("hTrklMul34"),
			       Form("Tracklet Multiplicity; TMul3; TMul4"),
			       10, 0, 10, 10, 0, 10);
  fhTrklMul3D =  new TH3F(  Form("hTrklMul3D"),
			    Form("Tracklet Multiplicities; TMul3; TMul4; TMul5"),
			    10, 0, 10, 10, 0, 10, 10, 0, 10);

  fhTrklHMul =  new TH2F(  Form("hTrklHMul"),
			   Form("Tracklet Hit - Multiplicity; HMul_{Tracklet}; Mul_{HMul}"),
			   fNTofStations-1, 2, fNTofStations+1, 20, 0, 20);  
  fhTrklZ0xHMul =  new TH2F(  Form("hTrklZ0xHMul"),
			   Form("Tracklet Z0x vs. Hit - Multiplicity; HMul_{Tracklet}; Z0x"),
			   fNTofStations-1, 2, fNTofStations+1, 100, -500, 500);
  fhTrklZ0yHMul =  new TH2F(  Form("hTrklZ0yHMul"),
			   Form("Tracklet Z0y vs. Hit - Multiplicity; HMul_{Tracklet}; Z0y"),
			   fNTofStations-1, 2, fNTofStations+1, 100, -300, 300);

  fhTrklTxHMul =  new TH2F(  Form("hTrklTxHMul"),
			   Form("Tracklet Tx vs. Hit - Multiplicity; HMul_{Tracklet}; Tx"),
			   fNTofStations-1, 2, fNTofStations+1, 100, -0.5, 0.5);

  fhTrklTyHMul =  new TH2F(  Form("hTrklTyHMul"),
			   Form("Tracklet Ty vs. Hit - Multiplicity; HMul_{Tracklet}; Ty"),
			   fNTofStations-1, 2, fNTofStations+1, 100, -0.5, 0.5);
  Double_t TTMAX=200.;
  fhTrklTtHMul =  new TH2F(  Form("hTrklTtHMul"),
			   Form("Tracklet Tt vs. Hit - Multiplicity; HMul_{Tracklet}; Tt"),
			   fNTofStations-1, 2, fNTofStations+1, 100, -TTMAX, TTMAX);
  fhTrklVelHMul =  new TH2F(  Form("hTrklVelHMul"),
			   Form("Tracklet Vel vs. Hit - Multiplicity; HMul_{Tracklet}; v (cm/ns)"),
			   fNTofStations-1, 2, fNTofStations+1, 100, -50., 50.);
  fhTrklT0HMul =  new TH2F(  Form("hTrklT0HMul"),
			   Form("Tracklet T0 vs. Hit - Multiplicity; HMul_{Tracklet}; T0"),
			   fNTofStations-1, 2, fNTofStations+1, 100, -500., 500.);

  fhTrklT0Mul =  new TH2F( Form("hTrklT0Mul"),
			   Form("Tracklet #DeltaT0 vs. Trkl - Multiplicity; Mul_{Tracklet}; #Delta(T0)"),
			   10, 0, 10, 100, -2000., 2000.);  
  fhTrklDT0SmMis  =  new TH2F( Form("hTrklDT0SmMis"),
			   Form("Tracklet DeltaT0 vs. Trkl - ID; SmType_{missed}; #Delta(T0)"),
			   10, 0, 10, 100, -2000., 2000.);
  fhTrklDT0StMis2 =  new TH2F( Form("hTrklDT0SmMis2"),
			   Form("Tracklet DeltaT0 vs. Station - ID; St2_{missed}; #Delta(T0)"),
			   50, 0, 50, 100, -2000., 2000.);

  Double_t X0MAX=50.;
  fhTrklXY0_3 =  new TH2F( Form("hTrklXY0_3"),
			   Form("Tracklet XY at z=0 for hmul=3 ; x (xm); y (cm)"),
			   100, -X0MAX, X0MAX, 100, -X0MAX, X0MAX);
  fhTrklXY0_4 =  new TH2F( Form("hTrklXY0_4"),
			   Form("Tracklet XY at z=0 for hmul=4 ; x (xm); y (cm)"),
			   100, -X0MAX, X0MAX, 100, -X0MAX, X0MAX);
  fhTrklXY0_5 =  new TH2F( Form("hTrklXY0_5"),
			   Form("Tracklet XY at z=0 for hmul=5 ; x (xm); y (cm)"),
			   100, -X0MAX, X0MAX, 100, -X0MAX, X0MAX);
  Double_t DTMAX=1000.;
  Double_t nSmt=10;
  fhPullT_Smt = new TH2F( Form("hPullT_Smt"),
			   Form("Tracklet PullT vs Sm ; Sm ; #DeltaT (ps)"),
			   nSmt, 0, nSmt, 100, -DTMAX, DTMAX);
  Double_t DT0MAX=25000.;
  fhTOff_Smt = new TH2F( Form("hTOff_Smt"),
			   Form("Tracklet TOff vs Sm ; Sm ; TOff (ps)"),
			   nSmt, 0, nSmt, 100, -DT0MAX, DT0MAX);

  Double_t DTTMAX=20.;
  fhDeltaTt_Smt = new TH2F( Form("hDeltaTt_Smt"),
			   Form("Tracklet DeltaTt vs Sm ; Sm ; #DeltaTt (ps/cm)"),
			   nSmt, 0, nSmt, 100, -DTTMAX, DTTMAX);

  vhPullX.resize( fNTofStations );
  vhPullY.resize( fNTofStations );
  vhPullZ.resize( fNTofStations );
  vhPullT.resize( fNTofStations );
  vhPullTB.resize(fNTofStations );
  vhXY_AllStations.resize(fNTofStations );
  vhXY_MissedStation.resize(fNTofStations );

  for (Int_t iSt=0; iSt<fNTofStations; iSt++){
    vhPullX[iSt]=new TH1F(  Form("hPullX_Station_%d",iSt),
			    Form("hPullX_Station_%d;  #DeltaX",iSt),
			    100, -10., 10.);  
    vhPullY[iSt]=new TH1F(  Form("hPullY_Station_%d",iSt),
			    Form("hPullY_Station_%d;  #DeltaY",iSt),
			    100, -10., 10.);  
    vhPullZ[iSt]=new TH1F(  Form("hPullZ_Station_%d",iSt),
			    Form("hPullZ_Station_%d;  #DeltaZ",iSt),
			    100, -200., 200.);  
    vhPullT[iSt]=new TH1F(  Form("hPullT_Station_%d",iSt),
			    Form("hPullT_Station_%d;  #DeltaT",iSt),
			    100, -500., 500.); 
    vhPullTB[iSt]=new TH1F( Form("hPullTB_Station_%d",iSt),
			    Form("hPullTB_Station_%d;  #DeltaT",iSt),
			    100, -500., 500.); 
    Double_t XSIZ=20.; 
    Int_t Nbins=40.;
    vhXY_AllStations[iSt] = new TH2F( Form("hXY_AllStations_%d",iSt),
			    Form("hXY_AllStations_%d;  x(cm); y (cm)",iSt),
			    Nbins, -XSIZ, XSIZ, Nbins, -XSIZ, XSIZ); 
    vhXY_MissedStation[iSt] = new TH2F( Form("hXY_MissedStation_%d",iSt),
			    Form("hXY_MissedStation_%d;  x(cm); y (cm)",iSt),
			    Nbins, -XSIZ, XSIZ, Nbins, -XSIZ, XSIZ); 
  }
  // vertex histrograms
  Double_t NNORM=40.;
  fhVTXNorm =  new TH1F(  Form("hVTXNorm"),
			  Form("Vertex Normalisation; #_{TrackletHits}"),
			  NNORM, 0, NNORM);  
  fhVTX_XY0 =  new TH2F( Form("hVTX_XY0"),
			 Form("Vertex XY at z=0  ; x (xm); y (cm)"),
			 100, -X0MAX, X0MAX, 100, -X0MAX, X0MAX);
  fhVTX_DT0_Norm =  new TH2F( Form("hVTX_DT0_Norm"),
			 Form("Vertex #DeltaT at z=0  ; #_{TrackletHits}; #DeltaT (ps)"),
			 NNORM, 0, NNORM, 100, -2000., 2000.);

  gDirectory->cd( oldir->GetPath() ); // <= To prevent histos from being sucked in by the param file of the TRootManager!

}

void CbmTofFindTracks::FindVertex(){
  fVTX_T=0.; //reset
  fVTX_X=0.;
  fVTX_Y=0.;
  fVTX_Z=0.;
  fVTXNorm=0.;

  for (Int_t iTrk=0; iTrk<fTrackArray->GetEntries();iTrk++) {
    CbmTofTracklet *pTrk = (CbmTofTracklet*)fTrackArray->At(iTrk);
    if(NULL == pTrk) continue;
    Double_t w=pTrk->GetNofHits();
    LOG(DEBUG1)<<Form("CbmTofFindTracks::FindVertex: N %3.0f, w %3.0f, min %d",fVTXNorm,w,fMinNofHits)
	      <<FairLogger::endl;

    if( w > (Double_t)fMinNofHits ){   // for further analysis request minimum number of hits
      fVTXNorm  += w;
      fVTX_T += w*pTrk->GetFitT(0.);
      fVTX_X += w*pTrk->GetFitX(0.);
      fVTX_Y += w*pTrk->GetFitY(0.);
    }
  }
  if(fVTXNorm > 0.) {
    fVTX_T /= fVTXNorm;
    fVTX_X /= fVTXNorm;
    fVTX_Y /= fVTXNorm;
    fVTX_Z /= fVTXNorm;
  }
  LOG(DEBUG)<<Form("CbmTofFindTracks::FindVertex: N %3.0f, T %6.2f, X=%6.2f, Y=%6.2f Z=%6.2f ",
		   fVTXNorm,fVTX_T,fVTX_X,fVTX_Y,fVTX_Z)
	    <<FairLogger::endl;

}

void CbmTofFindTracks::FillHistograms(){
  std::vector<Int_t> HMul;
  HMul.resize(fNTofStations+1);
  
  fhTrklMul->Fill(fTrackArray->GetEntries());

  Int_t iTMul=0;
  for (Int_t iTrk=0; iTrk<fTrackArray->GetEntries();iTrk++) {
    CbmTofTracklet *pTrk = (CbmTofTracklet*)fTrackArray->At(iTrk);
    if(NULL == pTrk) continue;
    HMul[pTrk->GetNofHits()]++;

    if( pTrk->GetNofHits() > fMinNofHits ){   // for further analysis request at least 3 matched hits
      iTMul++;
      fhTrklChi2->Fill(pTrk->GetChiSq());

      CbmTofTrackletParam *tPar = pTrk->GetTrackParameter();
      LOG(DEBUG)<<Form("CbmTofFindTracks::FillHistograms Trk %d: Lz=%6.2f Z0x=%6.2f Z0y=%6.2f ",
		       iTrk,tPar->GetLz(),pTrk->GetZ0x(),pTrk->GetZ0y())
	       <<tPar->ToString()
 	       <<FairLogger::endl;

      fhTrklZ0xHMul->Fill(pTrk->GetNofHits(),pTrk->GetFitX(0.));
      fhTrklZ0yHMul->Fill(pTrk->GetNofHits(),pTrk->GetFitY(0.));
      fhTrklTxHMul->Fill(pTrk->GetNofHits(),tPar->GetTx());
      fhTrklTyHMul->Fill(pTrk->GetNofHits(),tPar->GetTy());
      Double_t dTt=pTrk->GetTt();
      fhTrklTtHMul->Fill(pTrk->GetNofHits(),dTt);
      if (dTt != 0.) fhTrklVelHMul->Fill(pTrk->GetNofHits(),1000./dTt);

      for (Int_t iSt=0; iSt<fNTofStations; iSt++){
	Int_t iH  = pTrk->GetStationHitIndex(fStationType[iSt]); // Station Hit index
	if(iH<0) continue;                                       // Station not part of tracklet
	Int_t iH0 = pTrk->GetFirstInd(fStationType[iSt]);        // Closest station to target
	fhUsedHitsStation->Fill(iSt); 
	CbmTofHit* pHit  = (CbmTofHit*)fTofHitArray->At(iH);	
	CbmTofHit* pHit0 = (CbmTofHit*)fTofHitArray->At(iH0);
	
        if (0 == fStationType[iSt]) pHit->SetTime(pTrk->GetT0()); 
	/*
	cout << " -D- CbmTofFindTracks::FillHistograms: "<< iSt <<", "
	     <<fStationType[iSt]<<", "<< iH <<", "<< iH0 <<", "<<pHit->ToString() << endl; 
	*/
	Double_t dDZ = pHit->GetZ() - tPar->GetZ();    // z- Distance to reference point 
	Double_t dDX = pHit->GetX() - pTrk->GetFitX(pHit->GetZ());    // - tPar->GetX() - tPar->GetTx()*dDZ;
	Double_t dDY = pHit->GetY() - pTrk->GetFitY(pHit->GetZ());    // - tPar->GetTy()*dDZ;
	Double_t dDT = pHit->GetTime() - pTrk->GetFitT(pHit->GetR()); //pTrk->GetTdif(fStationType[iSt]);
	Double_t dDTB= pTrk->GetTdif(fStationType[iSt], pHit);  // ignore pHit in calc of reference

	Double_t dZZ = pHit->GetZ() - tPar->GetZy(pHit->GetY());
	LOG(DEBUG)<<Form("  St %d Ty %d Hit %d - DX %6.2f, DY %6.2f, DZ %6.2f, DT %6.2f, %6.2f, ZZ %6.2f ",
			 iSt, fStationType[iSt], iH, dDX, dDY, dDZ, dDT, dDTB, dZZ)
		 <<pHit->ToString()
		 <<FairLogger::endl; 

	vhPullX[iSt]->Fill(dDX);
	vhPullY[iSt]->Fill(dDY);
	vhPullZ[iSt]->Fill(dZZ);
	vhPullT[iSt]->Fill(dDT);
	vhPullTB[iSt]->Fill(dDTB);

	fhPullT_Smt->Fill((Double_t)fStationType[iSt],dDTB);
	Double_t dDeltaTt=dTt - fTtTarg;
	fhDeltaTt_Smt->Fill((Double_t)fStationType[iSt], dDeltaTt );
	Double_t dTOff = dDeltaTt*pHit->GetR(); 
	fhTOff_Smt->Fill((Double_t)fStationType[iSt], dTOff );
      }

      switch(pTrk->GetNofHits()){
      case 3:
        fhTrklXY0_3->Fill(pTrk->GetFitX(0.),pTrk->GetFitY(0.));
	break;
      case 4:
        fhTrklXY0_4->Fill(pTrk->GetFitX(0.),pTrk->GetFitY(0.));
	break;
      case 5:
        fhTrklXY0_5->Fill(pTrk->GetFitX(0.),pTrk->GetFitY(0.));
	break;
      default:
	;
      }
      // extrapolation of tracklet to vertex @ z=0
      FairTrackParam paramExtr;
      //      fFitter->Extrapolate(pTrk->GetParamFirst(),0.,&paramExtr);
    } // condition on NofHits>2 end 

   if( pTrk->GetNofHits() == fNTofStations ){   // all possible hits are there
     for (Int_t iSt=0; iSt<fNTofStations; iSt++){
       Int_t iH  = pTrk->GetStationHitIndex(fStationType[iSt]); // Station Hit index
       if(iH<0) {
	 LOG(ERROR)<<" Inconsistent Tracklet "<<FairLogger::endl;
	 continue;                                       // Station not part of tracklet
       }
       CbmTofHit* pHit  = (CbmTofHit*)fTofHitArray->At(iH);	
       Int_t iChId = pHit->GetAddress();
       Double_t hitpos[3]={3*0.};
       Double_t hitpos_local[3]={3*0.};
       CbmTofCell* fChannelInfo = fDigiPar->GetCell( iChId );
       if(NULL == fChannelInfo){
	 //faked hit, take init values 
       }else{
	 TGeoNode *fNode=        // prepare global->local trafo
	   gGeoManager->FindNode(fChannelInfo->GetX(),fChannelInfo->GetY(),fChannelInfo->GetZ());
	 hitpos[0]=pHit->GetX();
	 hitpos[1]=pHit->GetY();
	 hitpos[2]=pHit->GetZ();
	 TGeoNode* cNode= gGeoManager->GetCurrentNode();
	 gGeoManager->MasterToLocal(hitpos, hitpos_local);
       }      
       vhXY_AllStations[iSt]->Fill(hitpos_local[0],hitpos_local[1]);  
     }
   }else{
     if( pTrk->GetNofHits() == fNTofStations-1 ) { // one hit missing
       for (Int_t iSt=0; iSt<fNTofStations; iSt++){
	 Int_t iH  = pTrk->GetStationHitIndex(fStationType[iSt]); // Station Hit index
	 if(iH<0) { // find geo element for the missing Station iSt
	   Int_t iSmType = fStationType[iSt];
	   if (iSmType < 1) continue;
	   Int_t iChId   = CbmTofAddress::GetUniqueAddress(0,0,0,0,iSmType);
	   CbmTofCell* fChannelInfo = fDigiPar->GetCell( iChId );
	   TGeoNode *fNode=        // prepare global->local trafo
	   gGeoManager->FindNode(fChannelInfo->GetX(),fChannelInfo->GetY(),fChannelInfo->GetZ());
	   Double_t zPos=fChannelInfo->GetZ();
	   Double_t hitpos[3];
	   hitpos[0]=pTrk->GetFitX(zPos);
	   hitpos[1]=pTrk->GetFitY(zPos);
	   hitpos[2]=zPos;
	   Double_t hitpos_local[3];
	   TGeoNode* cNode= gGeoManager->GetCurrentNode();
	   gGeoManager->MasterToLocal(hitpos, hitpos_local);
	   vhXY_MissedStation[iSt]->Fill(hitpos_local[0],hitpos_local[1]);
	   // correlation analysis
	   for (Int_t iTrk1=iTrk+1; iTrk1<fTrackArray->GetEntries();iTrk1++) {
	     CbmTofTracklet *pTrk1 = (CbmTofTracklet*)fTrackArray->At(iTrk1);
	     if(NULL == pTrk1 || pTrk == pTrk1) continue;
	     if( pTrk1->GetNofHits() == fNTofStations ){   // all possible hits are there
	       fhTrklDT0SmMis->Fill(iSmType,pTrk->GetFitT(0.)-pTrk1->GetFitT(0.));
	     }else{
	     if( pTrk1->GetNofHits() == fNTofStations-1 ){   // one hit missing
	       for (Int_t iSt1=0; iSt1<fNTofStations; iSt1++){
		 Int_t iH1  = pTrk1->GetStationHitIndex(fStationType[iSt1]); // Station Hit index
		 if(iH1<0) { // find geo element for the missing Station iSt
		   Int_t iSmType1 = fStationType[iSt1];
		   if (iSmType1 < 1) continue;
		   fhTrklDT0StMis2->Fill(Double_t(iSt*10+iSt1),pTrk->GetFitT(0.)-pTrk1->GetFitT(0.));
		 }
	       }
	     }
	     }
	   }        
	 }
       }
     } 
   }
  }  // loop over tracklets end 
  fhTrklMul34->Fill(HMul[3],HMul[4]);
  if(HMul.size()>5)  fhTrklMul3D->Fill(HMul[3],HMul[4],HMul[5]);
  fhTrklMulNhits->Fill(fTofHitArray->GetEntries(),iTMul);

  if(iTMul > 1){
    LOG(DEBUG)<<Form("CbmTofFindTracks::FillHistograms NTrkl %d(%d) ", iTMul,fTrackArray->GetEntries())
	   <<FairLogger::endl;
    for (Int_t iTrk=0; iTrk<fTrackArray->GetEntries();iTrk++) {
      CbmTofTracklet *pTrk = (CbmTofTracklet*)fTrackArray->At(iTrk);
      if(NULL == pTrk) continue;
      if(pTrk->GetNofHits() > fMinNofHits){ // for further analysis request min # of matched hits
	for (Int_t iTrk1=iTrk+1; iTrk1<fTrackArray->GetEntries();iTrk1++) {
	  CbmTofTracklet *pTrk1 = (CbmTofTracklet*)fTrackArray->At(iTrk1);
	  if(NULL == pTrk1) continue;
	  if(pTrk1->GetNofHits() > fMinNofHits){ // for further analysis request min # of  matched hits
	    //cout << " -D- iT "<<iTrk<<", iT1 "<<iTrk1<<endl;
	    fhTrklT0Mul->Fill(iTMul,pTrk->GetFitT(0.)-pTrk1->GetFitT(0.));
	  }
	}
      }
    }
  }

  for (Int_t iHMul=2; iHMul<HMul.size(); iHMul++){
    LOG(DEBUG) << Form("CbmTofFindTracks::FillHistograms() HMul %d, #%d",iHMul,HMul[iHMul])
	    <<FairLogger::endl;
    if(HMul[iHMul]>0) {
      fhTrklHMul->Fill(iHMul,HMul[iHMul]);
    }
  }

  for (Int_t iHit=0; iHit<fTofHitArray->GetEntries(); iHit++){ // loop over Hits 
    CbmTofHit* pHit = (CbmTofHit*) fTofHitArray->At( iHit );	
    Int_t iSmType = CbmTofAddress::GetSmType( pHit->GetAddress() & DetMask );
    fhAllHitsSmTypes->Fill(iSmType); 
    //cout << " -D- " << iSmType <<", " << fTypeStation[iSmType] << endl; 
    if(fTypeStation[iSmType]>-1) fhAllHitsStation->Fill(fTypeStation[iSmType]); 
  }
  // vertex stuff
  fhVTXNorm->Fill(fVTXNorm);
  if(fVTXNorm>0.){
    fhVTX_XY0->Fill(fVTX_X,fVTX_Y);
    for (Int_t iTrk=0; iTrk<fTrackArray->GetEntries();iTrk++) {
      CbmTofTracklet *pTrk = (CbmTofTracklet*)fTrackArray->At(iTrk);
      if(NULL == pTrk) continue;
      if( Double_t w=pTrk->GetNofHits() > (Double_t)fMinNofHits ){
	if(fVTXNorm > w){
	  Double_t DeltaT0=pTrk->GetFitT(0.)-(fVTXNorm*fVTX_T - w*pTrk->GetFitT(0.))/(fVTXNorm-w);  
	  fhVTX_DT0_Norm->Fill(fVTXNorm,DeltaT0);
	}
      }
    }
  }
  if (0 == fStationType[0]){ // Generated Pseudo TofHit at origin
    fTofHitArray->RemoveAt(fTofHitArray->GetEntries()-1);               // remove added hit
  }
}


