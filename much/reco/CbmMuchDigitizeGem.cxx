/** CbmMuchDigitizeGem.cxx
 *@author Vikas Singhal <vikas@vecc.gov.in>
 *@since 01.10.16
 *@version 2.0
 *@author Evgeny Kryshen <e.kryshen@gsi.de>
 *@since 01.05.11
 *@version 2.0
 *@author Mikhail Ryzhinskiy <m.ryzhinskiy@gsi.de>
 *@since 19.03.07
 *@version 1.0
 **
 ** CBM task class for digitizing MUCH for both Event by event and Time based mode.
 ** Task level RECO
 ** Produces objects of type CbmMuchDigi out of CbmMuchPoint.
 **/

// Includes from MUCH
#include "CbmMuchDigitizeGem.h"
#include "CbmMuchPoint.h"
#include "CbmMuchSector.h"
#include "CbmMuchStation.h"
#include "CbmMuchModuleGem.h"
#include "CbmMuchModuleGemRadial.h"
#include "CbmMuchModuleGemRectangular.h"
#include "CbmMuchPad.h"
#include "CbmMuchPadRadial.h"
#include "CbmMuchPadRectangular.h"
#include "CbmMuchSectorRadial.h"
#include "CbmMuchSectorRectangular.h"
#include "CbmMuchDigi.h"
#include "CbmMuchReadoutBuffer.h"

// Includes from base
#include "FairRootManager.h"
#include "FairEventHeader.h"
#include "FairMCEventHeader.h"
#include "FairMCPoint.h"
#include "FairRunAna.h"
#include "FairRunSim.h"

// Includes from Cbm
#include "CbmMCTrack.h"
#include "CbmMCEpoch.h"
//#include "CbmMCBuffer.h"
#include "CbmDaqBuffer.h"


// Includes from ROOT
#include "TObjArray.h"
#include "TDatabasePDG.h"
#include "TFile.h"
#include "TRandom.h"
#include "TChain.h"

#include <cassert>
#include <vector>
using std::map;

// -------------------------------------------------------------------------
CbmMuchDigitizeGem::CbmMuchDigitizeGem(const char* digiFileName) 
  : FairTask("MuchDigitizeGem",1),
    fgDeltaResponse(),
    fAlgorithm(1),
    fGeoScheme(CbmMuchGeoScheme::Instance()),
    fDigiFile(digiFileName),
    fPoints(NULL),
    fMCTracks(NULL),
    fDigis(NULL),
    fDigiMatches(NULL),
    fNFailed(0),
    fNOutside(0),
    fNMulti(0),
    fNADCChannels(256),
    fQMax(500000),
    fQThreshold(10000),
    fMeanNoise(1500),
    fSpotRadius(0.05),
    fMeanGasGain(1e4),
    fDTime(3),
    fDeadPadsFrac(0),
    fTimer(),
    fDaq(0),
    fMcChain(NULL),
    fDeadTime(400),
    fDriftVelocity(100),
    fPeakingTime(20),
    fRemainderTime(40),
    fTimeBinWidth(1),
    fNTimeBins(200),
    fNdigis(0),
    fTOT(0),
    fTotalDriftTime(0.4/fDriftVelocity*10000), // 40 ns
    fSigma(),
    fMPV(),
    fIsLight(1) // fIsLight = 1 (default) Store Light CbmMuchDigiMatch in output branch, fIsLight = 0 Create Heavy CbmMuchDigiMatch with fSignalShape info.  
{
  fSigma[0] = new TF1("sigma_e","pol6",-5,10);
  fSigma[0]->SetParameters(sigma_e);

  fSigma[1] = new TF1("sigma_mu","pol6",-5,10);
  fSigma[1]->SetParameters(sigma_mu);

  fSigma[2] = new TF1("sigma_p","pol6",-5,10);
  fSigma[2]->SetParameters(sigma_p);

  fMPV[0]   = new TF1("mpv_e","pol6",-5,10);
  fMPV[0]->SetParameters(mpv_e);

  fMPV[1]   = new TF1("mpv_mu","pol6",-5,10);
  fMPV[1]->SetParameters(mpv_mu);

  fMPV[2]   = new TF1("mpv_p","pol6",-5,10);
  fMPV[2]->SetParameters(mpv_p);
}
// -------------------------------------------------------------------------

// -----   Destructor   ----------------------------------------------------
CbmMuchDigitizeGem::~CbmMuchDigitizeGem() {
  if (fDigis) {
    fDigis->Delete();
    delete fDigis;
  }
  if (fDigiMatches) {
    fDigiMatches->Delete();
    delete fDigiMatches;
  }
  if (fSigma) {
//    fSigma->Delete();
    delete fSigma[0];
    delete fSigma[1];
    delete fSigma[2];
    //delete[] fSigma;
  }
  if (fMPV) {
//    fMPV->Delete();
    delete fMPV[0];
    delete fMPV[1];
    delete fMPV[2];
    //delete[] fMPV;
  }
}
// -------------------------------------------------------------------------

// -----   Private method Init   -------------------------------------------
InitStatus CbmMuchDigitizeGem::Init() {
  FairRootManager* ioman = FairRootManager::Instance();
  if (!ioman)
    Fatal("Init", "No FairRootManager");

  // Initialize GeoScheme
  TFile* oldfile=gFile;
  TFile* file=new TFile(fDigiFile);
  TObjArray* stations = (TObjArray*) file->Get("stations");
  file->Close();
  file->Delete();
  gFile=oldfile;
  fGeoScheme->Init(stations);
  // Determine drift volume width
  Double_t driftVolumeWidth = 0.4; // cm - default
  for (Int_t i=0;i<fGeoScheme->GetNStations();i++){
    CbmMuchStation* station = fGeoScheme->GetStation(i);
    if (station->GetNLayers()<=0) continue;
    CbmMuchLayerSide* side = station->GetLayer(0)->GetSide(0);
    if (side->GetNModules()<=0) continue;
    CbmMuchModule* module = side->GetModule(0);
    if (module->GetDetectorType()!=1 && module->GetDetectorType()!=3) continue;
    driftVolumeWidth = module->GetSize().Z();
    break;
  }
  fTotalDriftTime = driftVolumeWidth/fDriftVelocity*10000; // [ns];
  //Reading MC point as Event by event for time based digi generation also.  
  // Get input array of MuchPoints
  fPoints = (TClonesArray*) ioman->GetObject("MuchPoint");
  // Get input array of MC tracks
  fMCTracks = (TClonesArray*) ioman->GetObject("MCTrack");
  //For event by event mode output will be stored via ioman 
  if(!fDaq){
    // Register output array MuchDigi
    fDigis = new TClonesArray("CbmMuchDigi", 1000);
    ioman->Register("MuchDigi", "Digital response in MUCH", fDigis, kTRUE);
    // Register output array MuchDigiMatches
    fDigiMatches = new TClonesArray("CbmMuchDigiMatch", 1000);
    ioman->Register("MuchDigiMatch", "Digi Match in MUCH", fDigiMatches, kTRUE);
  }
  else{
	FairTask* daq     = FairRun::Instance()->GetTask("Daq");
        if ( daq ) {
        LOG(INFO) << GetName() << ": Using stream mode."
                              << FairLogger::endl;
        //fMode = 0;
  	}  //? stream mode

	if (!CbmDaqBuffer::Instance() )  {
		fLogger->Fatal(MESSAGE_ORIGIN, "No CbmDaqBuffer present for building TimeSlice!");
		return kFATAL;
	} 
    //    fMCTracks = (TClonesArray*) ioman->GetObject("MCTrack");
  }


  //fgDeltaResponse is used in the CbmMuchSignal for analysing the Signal Shape,
  //it is generated once in the digitizer and then be used by each CbmMuchSignal.
  //For reducing the time therefore generated once in the CbmMuchDigitize Gem and
  //not generated in the CbmMuchSignal
  // Set response on delta function
  Int_t nShapeTimeBins=Int_t(gkResponsePeriod/gkResponseBin);
  fgDeltaResponse.Set(nShapeTimeBins);
  for (Int_t i=0;i<fgDeltaResponse.GetSize();i++){
    Double_t time = i*gkResponseBin;
    if (time<=fPeakingTime) fgDeltaResponse[i]=time/fPeakingTime;
    else fgDeltaResponse[i] = exp(-(time-fPeakingTime)/fRemainderTime); 
  }

  return kSUCCESS;
}
// -------------------------------------------------------------------------


// -----   Public method Exec   --------------------------------------------
void CbmMuchDigitizeGem::Exec(Option_t* opt) {
  
	// get current event to revert back at the end of exec
	//Int_t currentEvent = FairRootManager::Instance()->GetInTree()->GetBranch("MCTrack")->GetReadEntry();
	//LOG(DEBUG) << GetName() << ": Event Number is "<< currentEvent << FairLogger::endl;
	fTimer.Start();
  	fNdigis = 0;
  	Int_t nPoints=0;
	
	//Storing digi->Time < EventStartTime
	ReadAndRegister();
	//	cout<< "Exec Started and came out from first ReadAndRegister"<< endl;
	// --- Loop over all MuchPoints and execute the ExecPoint method for both Event by event and Time Based Digitization
     
  	for (Int_t iPoint=0; iPoint<fPoints->GetEntriesFast(); iPoint++) {
		const CbmMuchPoint* point = (const CbmMuchPoint*) fPoints->At(iPoint);
		LOG(DEBUG) << GetName() << ": Processing MCPoint " << iPoint << FairLogger::endl;
    		if(!point){
			LOG(DEBUG) << GetName() << ":Not Processing MCPoint " << iPoint <<"as MCPoint is NULL."<< point << FairLogger::endl;
			return;
		}
  		nPoints++;
		ExecPoint(point, iPoint);
	}  // MuchPoint loop
	// Add remaining digis
	
	//For event by event mode read all the data from CbmMuchReadoutBuffer and register in the Output.
	//Read the Signal, convert into Digi And Register into output
  	ReadAndRegister();
 	fTimer.Stop();
  	gLogger->Info(MESSAGE_ORIGIN,"MuchDigitizeGem: %5.2f s, %i points, %i digis",fTimer.RealTime(),nPoints,fNdigis);

  	// revert branch to "current event"
  	//FairRootManager::Instance()->GetInTree()->GetBranch("MCTrack")->GetEntry(currentEvent);
}


// -------------------------------------------------------------------------
	//Read all the Signal from CbmMuchReadoutBuffer, convert the analog signal into the digital response  and register Output according to event by event mode and Time based mode.
void CbmMuchDigitizeGem::ReadAndRegister(){
	std::vector<CbmMuchSignal*> SignalList;
	
	Double_t eventTime = -1.;
	if(fDaq){
		eventTime = FairRun::Instance()->GetEventHeader()->GetEventTime();
  	}
	
	Int_t ReadOutSignal = CbmMuchReadoutBuffer::Instance()->ReadOutData(eventTime,SignalList);
	LOG(DEBUG) << GetName() << ": Number of digi's read out from Buffer "<< ReadOutSignal << " and SignalList contain " << SignalList.size() << " entries."<<FairLogger::endl;

    	for (std::vector<CbmMuchSignal*>::iterator LoopOver= SignalList.begin(); LoopOver != SignalList.end(); LoopOver++)
	{
		CbmMuchDigi* digi = ConvertSignalToDigi(*LoopOver);
		//assert(digi);
		if (!digi){
			LOG(DEBUG)<< GetName() << ": Digi not created as signal is below threshold."<<FairLogger::endl;
		}
		else {
			LOG(DEBUG)<< GetName() << ": New digi: sector = "<< CbmMuchAddress::GetSectorIndex(digi->GetAddress()) <<" channel= " << CbmMuchAddress::GetChannelIndex(digi->GetAddress())<< FairLogger::endl;
			if(fDaq)
				//Buffer these digi's in CbmDaqBuffer for Timeslice building.
				CbmDaqBuffer::Instance()->InsertData(digi);
			else{
  				new ((*fDigis)[fDigis->GetEntriesFast()]) CbmMuchDigi(digi);
				//Above syntax is equivalent as below:-
				//Int_t nDigis = fDigis->GetEntriesFast();
				//CbmMuchDigi* digi = new ((*fDigis)[nDigis]) CbmMuchDigi(digi)
  				new ((*fDigiMatches)[fDigiMatches->GetEntriesFast()]) CbmMuchDigiMatch((CbmMuchDigiMatch*)digi->GetMatch());
				
				// Match object will be deleted in the digi destructor.	
				delete digi;
			}
			fNdigis++;
		}
	}
}


//Convert Signal into the Digi with appropriate methods.

CbmMuchDigi* CbmMuchDigitizeGem::ConvertSignalToDigi(CbmMuchSignal* signal){
		Int_t TimeStamp = signal->GetTimeStamp(fQThreshold);
		if (TimeStamp < 0) return (NULL);//entire signal is below threshold, no digi generation.

		CbmMuchDigi *digi = new CbmMuchDigi();
		digi->SetAddress(signal->GetAddress());
		//Charge in number of electrons, need to be converted in ADC value
	
		digi->SetAdc((signal->GetCharge())*fNADCChannels/fQMax);//Charge should be computed as per Electronics Response.
		digi->SetTime(TimeStamp);
		digi->SetMatch(signal->GetMatch());

	//	digi->SetPileUp();
	//	digi->SetDiffEvent();
		return(digi);
}



// -------------------------------------------------------------------------
void CbmMuchDigitizeGem::Finish(){
  //if (fDaq) Exec("");
  //Store all the remaining digi's in the buffer into the CbmDaqBuffer

   if (fDaq)	ReadAndRegister();
             
}
// -------------------------------------------------------------------------


// ------- Private method ExecAdvanced -------------------------------------
Bool_t CbmMuchDigitizeGem::ExecPoint(const CbmMuchPoint* point, Int_t iPoint) {
  
  TVector3 v1,v2,dv;
  point->PositionIn(v1);
  point->PositionOut(v2);
  dv = v2-v1;

  Bool_t Status;
  Int_t detectorId = point->GetDetectorID();
  CbmMuchModule* module = fGeoScheme->GetModuleByDetId(detectorId);
  
  if (fAlgorithm==0){    // Simple digitization
    TVector3 v = 0.5*(v1+v2);
    CbmMuchPad* pad = 0;
    if (module->GetDetectorType()==1){
      CbmMuchModuleGemRectangular* module1 = (CbmMuchModuleGemRectangular*) module;
      pad = module1->GetPad(v[0],v[1]);
      if (pad) printf("x0=%f,y0=%f\n",pad->GetX(),pad->GetY());
    } else if (module->GetDetectorType()==3){
      CbmMuchModuleGemRadial* module3 = (CbmMuchModuleGemRadial*) module;
      pad = module3->GetPad(v[0],v[1]);
    }
    if (!pad) return kFALSE;
    AddCharge(pad,fQMax,iPoint,point->GetTime(),0);
    return kTRUE;
  }
  
  // Start of advanced digitization
  Int_t nElectrons = Int_t(GetNPrimaryElectronsPerCm(point)*dv.Mag());
  if (nElectrons<0) return kFALSE;
  
  Double_t time = point->GetTime();
  
  if (module->GetDetectorType()==1) {
    CbmMuchModuleGemRectangular* module1 = (CbmMuchModuleGemRectangular*) module;
    map<CbmMuchSector*,Int_t> firedSectors;
    for (Int_t i=0;i<nElectrons;i++) {
      Double_t aL = gRandom->Rndm();
      Double_t driftTime = (1-aL)*fTotalDriftTime;
      TVector3 ve = v1 + dv*aL;
      UInt_t ne   = GasGain();
      Double_t x  = ve.X();
      Double_t y  = ve.Y();
      Double_t x1 = x-fSpotRadius;
      Double_t x2 = x+fSpotRadius;
      Double_t y1 = y-fSpotRadius;
      Double_t y2 = y+fSpotRadius;
      Double_t s  = 4*fSpotRadius*fSpotRadius;
      firedSectors[module1->GetSector(x1,y1)]=0;
      firedSectors[module1->GetSector(x1,y2)]=0;
      firedSectors[module1->GetSector(x2,y1)]=0;
      firedSectors[module1->GetSector(x2,y2)]=0;
      for (map<CbmMuchSector*, Int_t>::iterator it = firedSectors.begin(); it!= firedSectors.end(); it++) {
        CbmMuchSector* sector  = (*it).first;
        if (!sector) continue;
        for (Int_t iPad=0;iPad<sector->GetNChannels();iPad++){
          CbmMuchPad* pad = sector->GetPadByChannelIndex(iPad);
          Double_t xp0 = pad->GetX();
          Double_t xpd = pad->GetDx()/2.;
          Double_t xp1 = xp0-xpd;
          Double_t xp2 = xp0+xpd;
          if (x1>xp2 || x2<xp1) continue;
          Double_t yp0 = pad->GetY();
          Double_t ypd = pad->GetDy()/2.;
          Double_t yp1 = yp0-ypd;
          Double_t yp2 = yp0+ypd;
          if (y1>yp2 || y2<yp1) continue;
          Double_t lx = x1>xp1 ? (x2<xp2 ? x2-x1 : xp2-x1) : x2-xp1;
          Double_t ly = y1>yp1 ? (y2<yp2 ? y2-y1 : yp2-y1) : y2-yp1;
          AddCharge(pad,UInt_t(ne*lx*ly/s),iPoint,time,driftTime);
        }
      } // loop fired sectors
      firedSectors.clear();
    }
  }
  
  if (module->GetDetectorType()==3) {
    fAddressCharge.clear();
    CbmMuchModuleGemRadial* module3 = (CbmMuchModuleGemRadial*) module;
    if(!module3){
	LOG(DEBUG) << GetName() << ": Not Processing MCPoint " << iPoint <<" because it is not on any GEM module."<< FairLogger::endl;
	return 1;
	}
    CbmMuchSectorRadial* sFirst = (CbmMuchSectorRadial*) module3->GetSectorByIndex(0);  //First sector
    if(!sFirst){
	LOG(DEBUG) << GetName() << ": Not Processing MCPoint " << iPoint <<" because it is on the module " << module3 <<"  but not the first sector. "<< sFirst << FairLogger::endl;
	return 1;
	}
    CbmMuchSectorRadial* sLast  = (CbmMuchSectorRadial*) module3->GetSectorByIndex(module3->GetNSectors()-1); //Last sector

    if(!sLast){
	LOG(DEBUG) << GetName() << ": Not Processing MCPoint " << iPoint <<" because it is not the last sector of module."<< module3 << FairLogger::endl;
	return 1;
	}
    Double_t rMin = sFirst->GetR1(); //Mimimum radius of the Sector
    Double_t rMax = sLast->GetR2();  //Maximum radius of the Sector
    //Calculating drifttime once for one track or one MCPoint, not for all the Primary Electrons generated during DriftGap.
    Double_t aL   = gRandom->Rndm();
    Double_t driftTime = -1;
    while(driftTime < 0) driftTime = (1-aL)*fTotalDriftTime + gRandom->Gaus(0, fDTime); //Finding drifttime with random factor of Detector Time Resolution
    for (Int_t i=0;i<nElectrons;i++) { //Looping over all the primary electrons
      //Double_t aL   = gRandom->Rndm();
      //Double_t driftTime = -1;
      //while(driftTime < 0) driftTime = (1-aL)*fTotalDriftTime + gRandom->Gaus(0, fDTime); //Finding drifttime with random factor of Detector Time Resolution
      TVector3 ve   = v1 + dv*aL;
      UInt_t ne     = GasGain(); //Number of secondary electrons
      Double_t r    = ve.Perp(); //
      Double_t phi  = ve.Phi();
      Double_t r1   = r-fSpotRadius; 
      Double_t r2   = r+fSpotRadius;
      Double_t phi1 = phi-fSpotRadius/r;
      Double_t phi2 = phi+fSpotRadius/r;
      if (r1<rMin && r2>rMin) {//Adding charge to the pad which is on Lower Boundary 
        Status = AddCharge(sFirst,UInt_t(ne*(r2-rMin)/(r2-r1)),iPoint,time,driftTime,phi1,phi2);
	if(!Status)LOG(DEBUG) << GetName() << ": Processing MCPoint " << iPoint <<" in which Primary Electron : "<<i<< " not contributed charge. "<< FairLogger::endl;
        continue;
      }  
      if (r1<rMax && r2>rMax) {//Adding charge to the pad which is on Upper Boundary
        Status = AddCharge(sLast,UInt_t(ne*(rMax-r1)/(r2-r1)),iPoint,time,driftTime,phi1,phi2);
	if(!Status)LOG(DEBUG) << GetName() << ": Processing MCPoint " << iPoint <<" in which Primary Electron : "<<i<< " not contributed charge. "<< FairLogger::endl;
	  continue;
      }

      CbmMuchSectorRadial* s1 = module3->GetSectorByRadius(r1);
      CbmMuchSectorRadial* s2 = module3->GetSectorByRadius(r2);
      if (s1==s2) {Status = AddCharge(s1,ne,iPoint,time,driftTime,phi1,phi2); 
			if(!Status)LOG(DEBUG) << GetName() << ": Processing MCPoint " << iPoint <<" in which Primary Electron : "<<i<< " not contributed charge. "<< FairLogger::endl;
		}
      else {//Adding praportionate charge to both the pad 
	Status = AddCharge(s1,UInt_t(ne*(s1->GetR2()-r1)/(r2-r1)),iPoint,time,driftTime,phi1,phi2);
	if(!Status)LOG(DEBUG) << GetName() << ": Processing MCPoint " << iPoint <<" in which Primary Electron : "<<i<< " not contributed charge. "<< FairLogger::endl;
        Status = AddCharge(s2,UInt_t(ne*(r2-s2->GetR1())/(r2-r1)),iPoint,time,driftTime,phi1,phi2);
	if(!Status)LOG(DEBUG) << GetName() << ": Processing MCPoint " << iPoint <<" in which Primary Electron : "<<i<< " not contributed charge. "<< FairLogger::endl;
      }
    }
    //Generate CbmMuchSignal for each entry of fAddressCharge and store in the CbmMuchReadoutBuffer
      if(!BufferSignals(iPoint,time,driftTime)) 
	LOG(DEBUG) << GetName() << ": Processing MCPoint " << iPoint <<" nothing is buffered. "<< FairLogger::endl;
    fAddressCharge.clear();
    LOG(DEBUG) << GetName() << ": fAddressCharge size is " << fAddressCharge.size() <<" Cleared fAddressCharge. "<< FairLogger::endl;
  }

  return kTRUE;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Int_t CbmMuchDigitizeGem::GasGain() {
  Double_t gasGain = -fMeanGasGain * TMath::Log(1 - gRandom->Rndm());
  if (gasGain < 0.) gasGain = 1e6;
  return (Int_t) gasGain;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Double_t CbmMuchDigitizeGem::Sigma_n_e(Double_t Tkin, Double_t mass) {
  Double_t logT;
  if (mass < 0.1) {
    logT = log(Tkin * 0.511 / mass);
    if (logT > 9.21034)    logT = 9.21034;
    if (logT < min_logT_e) logT = min_logT_e;
    return fSigma[0]->Eval(logT);
  } else if (mass >= 0.1 && mass < 0.2) {
    logT = log(Tkin * 105.658 / mass);
    if (logT > 9.21034)    logT = 9.21034;
    if (logT < min_logT_mu) logT = min_logT_mu;
    return fSigma[1]->Eval(logT);
  } else {
    logT = log(Tkin * 938.272 / mass);
    if (logT > 9.21034)    logT = 9.21034;
    if (logT < min_logT_p) logT = min_logT_p;
    return fSigma[2]->Eval(logT);
  }
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Double_t CbmMuchDigitizeGem::MPV_n_e(Double_t Tkin, Double_t mass) {
  Double_t logT;
  if (mass < 0.1) {
    logT = log(Tkin * 0.511 / mass);
    if (logT > 9.21034)    logT = 9.21034;
    if (logT < min_logT_e) logT = min_logT_e;
    return fMPV[0]->Eval(logT);
  } else if (mass >= 0.1 && mass < 0.2) {
    logT = log(Tkin * 105.658 / mass);
    if (logT > 9.21034)    logT = 9.21034;
    if (logT < min_logT_mu) logT = min_logT_mu;
    return fMPV[1]->Eval(logT);
  } else {
    logT = log(Tkin * 938.272 / mass);
    if (logT > 9.21034)    logT = 9.21034;
    if (logT < min_logT_p) logT = min_logT_p;
    return fMPV[2]->Eval(logT);
  }
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
Double_t CbmMuchDigitizeGem::GetNPrimaryElectronsPerCm(const CbmMuchPoint* point){
  Int_t trackId = point->GetTrackID();
  Int_t eventId = point->GetEventID();
  if (trackId < 0) return -1;


  /* Commented out on request of A. Senger from 22.01.2014
  if (fDaq && eventId!=FairRootManager::Instance()->GetInTree()->GetBranch("MCTrack")->GetReadEntry())
      FairRootManager::Instance()->GetInTree()->GetBranch("MCTrack")->GetEntry(eventId);
  */
  CbmMCTrack* mcTrack = (CbmMCTrack*) fMCTracks->At(trackId);
  
  if (!mcTrack) return -1;
  Int_t pdgCode = mcTrack->GetPdgCode();

  TParticlePDG *particle = TDatabasePDG::Instance()->GetParticle(pdgCode);
  // Assign proton hypothesis for unknown particles
  if (!particle) particle = TDatabasePDG::Instance()->GetParticle(2212);
  if (TMath::Abs(particle->Charge()) < 0.1) return -1;

  Double_t m = particle->Mass();
  TLorentzVector p;
  p.SetXYZM(point->GetPx(),point->GetPy(),point->GetPz(),m);
  Double_t Tkin = p.E()-m; // kinetic energy of the particle
  Double_t sigma = CbmMuchDigitizeGem::Sigma_n_e(Tkin,m); // sigma for Landau distribution
  Double_t mpv   = CbmMuchDigitizeGem::MPV_n_e(Tkin,m);   // most probable value for Landau distr.
  Double_t n = gRandom->Landau(mpv, sigma);
  while (n > 5e4) n = gRandom->Landau(mpv, sigma); // restrict Landau tail to increase performance
  return m<0.1 ? n/l_e : n/l_not_e;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Bool_t CbmMuchDigitizeGem::AddCharge(CbmMuchSectorRadial* s,UInt_t ne, Int_t iPoint, Double_t time, Double_t driftTime, 
    Double_t phi1, Double_t phi2){
  CbmMuchPadRadial* pad1 = s->GetPadByPhi(phi1);
  if(!pad1) return kFALSE;
  //assert(pad1); has to check if any pad address is NULL
  CbmMuchPadRadial* pad2 = s->GetPadByPhi(phi2);
  if(!pad2) return kFALSE;
  //assert(pad2); has to check if any pad address is NULL
  if (pad1==pad2){
	 UInt_t address = pad1->GetAddress();
         //Finding that if for the same address if already charge stored then add the charge.
         std::map<UInt_t,UInt_t>::iterator it=fAddressCharge.find(address);
         if(it!=fAddressCharge.end()) it->second=it->second+ne;
         else fAddressCharge.insert(std::pair<UInt_t,UInt_t>(address,ne));
        //    AddChargePerMC(pad1,ne,iPoint,time,driftTime);
  }
  else {
    Double_t phi = pad1 ? pad1->GetPhi2() : pad2 ? pad2->GetPhi1() : 0;
    UInt_t pad1_ne = UInt_t(ne*(phi-phi1)/(phi2-phi1));

	UInt_t address = pad1->GetAddress();
        //Finding that if for the same address if already charge stored then add the charge.
        std::map<UInt_t,UInt_t>::iterator it=fAddressCharge.find(address);
        if(it!=fAddressCharge.end()) it->second=it->second+pad1_ne;
        else fAddressCharge.insert (std::pair<UInt_t,UInt_t>(address,pad1_ne));
        //    AddChargePerMC(pad1,pad1_ne   ,iPoint,time,driftTime);
 

	// Getting some segmentation fault a
	address = pad2->GetAddress();
        //Finding that if for the same address if already charge stored then add the charge.
        it=fAddressCharge.find(address);
        if(it!=fAddressCharge.end()) it->second=it->second+ne-pad1_ne;
        else fAddressCharge.insert (std::pair<UInt_t,UInt_t>(address,ne-pad1_ne));
	// AddChargePerMC(pad2,ne-pad1_ne,iPoint,time,driftTime);
  }
  return kTRUE;
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
//Will remove this AddCharge, only used for simple and Rectangular Geometry.
void CbmMuchDigitizeGem::AddCharge(CbmMuchPad* pad, UInt_t charge, Int_t iPoint, Double_t time, Double_t driftTime){
  	if (!pad) return;

	Double_t eventTime = 0.;
        if (fDaq) eventTime = FairRun::Instance()->GetEventHeader()->GetEventTime();
        //LOG(DEBUG) << GetName() << ": Readout time is " << readoutTime << " ns"<< FairLogger::endl;
	UInt_t  AbsTime = eventTime + time + driftTime;

        //Creating a new Signal, it will be deleted by CbmReadoutBuffer()
	CbmMuchSignal* signal = new CbmMuchSignal(pad->GetAddress());
        signal->SetTimeStart(AbsTime);
        signal->SetTimeStop(AbsTime+fDeadTime);
        signal->MakeSignalShape(charge,fgDeltaResponse);
        signal->AddNoise(fMeanNoise);
        UInt_t address = pad->GetAddress();
        Int_t    inputNr   = 0;
        Int_t    eventNr   = 0;
        GetEventInfo(inputNr, eventNr, eventTime);
        LOG(DEBUG) << GetName() << ": Processing event " << eventNr
            << " from input " << inputNr << " at t = " << eventTime
            << " ns with " << fPoints->GetEntriesFast() << " MuchPoints "
                               << FairLogger::endl;
        //match->AddCharge(iPoint,charge,time+driftTime,fgDeltaResponse,time,eventNr,inputNr);
        CbmLink link(charge,iPoint,eventNr,inputNr);
        //std::cout<<"Before AddLink"<< endl;
        (signal->GetMatch())->AddLink(link);
        //std::cout<<"After AddLink"<< endl;
        //Adding all these temporary signal into the CbmMuchReadoutBuffer
        CbmMuchReadoutBuffer::Instance()->Fill(address, signal);
        LOG(DEBUG4)<<" Registered the CbmMuchSignal into the CbmMuchReadoutBuffer "<<FairLogger::endl;

}//end of AddCharge 



//----------------------------------------------------------
Bool_t CbmMuchDigitizeGem::BufferSignals(Int_t iPoint,Double_t time, Double_t driftTime){

	if(!fAddressCharge.size()) { LOG(DEBUG) << "Buffering MC Point " << iPoint
            << " but fAddressCharge size is " << fAddressCharge.size() << "so nothing to Buffer for this MCPoint." << FairLogger::endl;
		return kFALSE;
	}
	//fetching event time and event info
	
	Double_t eventTime = 0.;
        if (fDaq) eventTime = FairRun::Instance()->GetEventHeader()->GetEventTime();
        // LOG(DEBUG) << GetName() << ": Readout time is " << readoutTime << " ns"<< FairLogger::endl;
        UInt_t  AbsTime = eventTime + time + driftTime;
        Int_t    inputNr   = 0;
        Int_t    eventNr   = 0;
        GetEventInfo(inputNr, eventNr, eventTime);
        LOG(DEBUG) << GetName() << ": Processing event " << eventNr
            << " from input " << inputNr << " at t = " << eventTime
            << " ns with " << fPoints->GetEntriesFast() << " MuchPoints "
	    << " and Number of pad hit is "<< fAddressCharge.size()<<"." 
                               << FairLogger::endl;
        //Loop on the fAddressCharge to store all the Signals into the CbmReadoutBuffer()
	//Generate one by one CbmMuchSignal from the fAddressCharge and store them into the CbmMuchReadoutBuffer.
	for(auto it=fAddressCharge.begin();it!=fAddressCharge.end();++it){
		UInt_t address = it->first;
	        //Creating a new Signal, it will be deleted by CbmReadoutBuffer()
		CbmMuchSignal* signal = new CbmMuchSignal(address);
        	signal->SetTimeStart(AbsTime);
        	signal->SetTimeStop(AbsTime+fDeadTime);
        	signal->MakeSignalShape(it->second,fgDeltaResponse);
        	signal->AddNoise(fMeanNoise);
		CbmLink link(signal->GetMaxCharge(),iPoint,eventNr,inputNr);
		(signal->GetMatch())->AddLink(link);
		//Adding all these temporary signal into the CbmMuchReadoutBuffer
		CbmMuchReadoutBuffer::Instance()->Fill(address, signal);
		LOG(DEBUG)<<" Registered the CbmMuchSignal into the CbmMuchReadoutBuffer "<<FairLogger::endl;
	}
	
	LOG(DEBUG) << GetName() << ": For MC Point " << iPoint
            << " buffered " << fAddressCharge.size() << " CbmMuchSignal into the CbmReadoutBuffer." << FairLogger::endl;
	return kTRUE;
}//end of BufferSignals
// -------------------------------------------------------------------------


// -----   Get event information   -----------------------------------------
void CbmMuchDigitizeGem::GetEventInfo(Int_t& inputNr, Int_t& eventNr,
                                              Double_t& eventTime) {

	// --- In a FairRunAna, take the information from FairEventHeader
	if ( FairRunAna::Instance() ) {
                FairEventHeader* event = FairRunAna::Instance()->GetEventHeader();
                assert ( event );
          inputNr   = event->GetInputFileId();
          eventNr   = event->GetMCEntryNumber();
          eventTime = event->GetEventTime();
        }
 	// --- In a FairRunSim, the input number and event time are always zero;
	 // --- only the event number is retrieved.
     else {
                if ( ! FairRunSim::Instance() )
                        LOG(FATAL) << GetName() << ": neither SIM nor ANA run."
                                               << FairLogger::endl;
                FairMCEventHeader* event = FairRunSim::Instance()->GetMCEventHeader();
                assert ( event );
                inputNr   = 0;
                eventNr   = event->GetEventID();
                eventTime = 0.;
        }

}
	// -------------------------------------------------------------------------

ClassImp(CbmMuchDigitizeGem)






