#include "CbmStsDigitizeQa.h"
#include "CbmStsDigitize.h"
#include "CbmHistManager.h"
#include "FairRootManager.h"
#include "FairLogger.h"
#include "CbmMatch.h"
#include "CbmStsDigi.h"
#include "CbmStsAddress.h"
#include "CbmStsModule.h"
#include "CbmStsElement.h"
#include "CbmStsSetup.h"
#include "CbmMCDataManager.h"
#include "CbmMCBuffer.h"
#include "CbmSimulationReport.h"
#include "CbmStsDigitizeQaReport.h"
#include "TGeoPhysicalNode.h"
#include "TGeoMatrix.h"

#include "TClonesArray.h"
#include "TH1.h"
#include "TF1.h"
#include "TH1D.h"
#include "TH2.h"
#include "TProfile.h"
#include "TProfile2D.h"

using std::cout;
using std::vector;
using std::set;
using std::map;
using std::pair;

CbmStsDigitizeQa::CbmStsDigitizeQa(CbmStsDigitize * digitizer):
    fDigitizer(digitizer)
    , fHM(NULL)
    , fOutputDir(" ")
    , fStsDigis(NULL)
    , fStsPoints(NULL)
    , fSetup(NULL)
    , fNofStation(8)
    , fMaxScale(0)
    , fOutFile(NULL)
    , fnOfDigisChip()
{
}

CbmStsDigitizeQa::~CbmStsDigitizeQa(){
    if ( fHM ) delete fHM;
}

InitStatus CbmStsDigitizeQa::Init(){
    fSetup = CbmStsSetup::Instance();
    fNofStation = fSetup -> GetNofElements(kStsStation);
    fHM = new CbmHistManager();
    fnOfDigisChip.resize(fSetup -> GetNofElements(kStsStation));
    for (Int_t iStation = 0; iStation < fSetup -> GetNofElements(kStsStation); iStation ++){
	CbmStsElement * stat = fSetup -> GetDaughter(iStation);
	fnOfDigisChip[iStation].resize(stat -> GetNofDaughters());
	for (Int_t iLad = 0; iLad < stat -> GetNofDaughters(); iLad++) {
	    CbmStsElement* ladd = stat -> GetDaughter(iLad);
	    fnOfDigisChip[iStation][iLad].resize(ladd -> GetNofDaughters());
	    for (Int_t iHla = 0; iHla < ladd -> GetNofDaughters(); iHla++) {
		CbmStsElement* hlad = ladd -> GetDaughter(iHla);
		fnOfDigisChip[iStation][iLad][iHla].resize(hlad -> GetNofDaughters());
		for (Int_t iMod = 0; iMod < hlad -> GetNofDaughters(); iMod++) {
		    CbmStsModule* modu = static_cast<CbmStsModule*>(hlad -> GetDaughter(iMod));
		    Int_t nOfChips = Int_t(modu -> GetNofChannels() / 128.);
		    fnOfDigisChip[iStation][iLad][iHla][iMod].resize(nOfChips);
		    for (Int_t iChip = 0; iChip < nOfChips; iChip++){
			fnOfDigisChip[iStation][iLad][iHla][iMod][iChip] = 0;
		    }
		}
	    }
	}
    }

    ReadDataBranches();
    CreateHistograms();
    return kSUCCESS;
}

void CbmStsDigitizeQa::Exec(Option_t* opt){
    ProcessDigisAndPoints(fStsDigis, fStsPoints);
    fHM -> H1("h_EventNo_DigitizeQa") -> Fill(0.5);
}

void CbmStsDigitizeQa::Finish(){
    ProcessAngles();
    Int_t nofEvents = fHM -> H1("h_EventNo_DigitizeQa") -> GetEntries();
    TString fileName = fOutputDir + "/digiRateChip";
    fileName += nofEvents;
    fileName += ".dat";
    TString rmFile = "rm " + fileName;
    gSystem -> Exec(rmFile);
    fOutFile.open(Form("%s",fileName.Data()), std::ofstream::app);
    for (Int_t iStation = 0; iStation < fSetup -> GetNofElements(kStsStation); iStation ++){
	CbmStsElement * stat = fSetup -> GetDaughter(iStation);
	for (Int_t iLad = 0; iLad < stat -> GetNofDaughters(); iLad++) {
	    CbmStsElement* ladd = stat -> GetDaughter(iLad);
	    for (Int_t iHla = 0; iHla < ladd -> GetNofDaughters(); iHla++) {
		CbmStsElement* hlad = ladd -> GetDaughter(iHla);
		for (Int_t iMod = 0; iMod < hlad -> GetNofDaughters(); iMod++) {
		    CbmStsModule* modu = static_cast<CbmStsModule*>(hlad -> GetDaughter(iMod));
		    if(modu -> GetNofChannels() != 2048) cout << "nofChannels = " << modu -> GetNofChannels() << endl;
		    Int_t nOfChips = Int_t(modu -> GetNofChannels() / 128.);
		    for (Int_t iChip = 0; iChip < nOfChips; iChip++){
			fOutFile << iStation << "\t" << iLad << "\t" << iHla << "\t" << iMod << "\t" << iChip << "\t" << fnOfDigisChip[iStation][iLad][iHla][iMod][iChip] << endl;
		}
		}
	    }
	}
    }
    gDirectory -> mkdir("STSDigitizeQA");
    gDirectory -> cd("STSDigitizeQA");
    fHM -> WriteToFile();
    CbmSimulationReport* report = new CbmStsDigitizeQaReport(fSetup, fDigitizer);
    report -> Create(fHM, fOutputDir);
    delete report;

    /*  Double_t matchedHits = 100. * (Double_t) fHM->H1("hno_NofObjects_MatchedHits_Station_" + type)->Integral() /
	(Double_t) fHM->H1("hno_NofObjects_Hits_Station_" + type)->Integral();
	Double_t efficiency = 100 * (Double_t) fHM->H1("hno_NofObjects_MatchedHits_Station_" + type)->Integral() /
	(Double_t) fHM->H1("hno_NofObjects_Points_Station_" + type)->Integral();
	Double_t ghost = 100 * ((Double_t) fHM->H1("hno_NofObjects_Hits_Station_" + type)->Integral() -
	(Double_t) fHM->H1("hno_NofObjects_MatchedHits_Station_" + type)->Integral()) /
	(Double_t) fHM->H1("hno_NofObjects_Points_Station_" + type)->Integral();

	std::cout<<" -I- CbmStsTimeBasedQa: Hits: "<<fHM->H1("hno_NofObjects_Hits_Station_" + type)->Integral()
	<<"\n -I- CbmStsTimeBasedQa: MatchedHits: "<<fHM->H1("hno_NofObjects_MatchedHits_Station_" + type)->Integral()
	<<"\n -I- CbmStsTimeBasedQa: MatchedHits: "<<matchedHits<<" %"
	<<"\n -I- CbmStsTimeBasedQa: Efficiency : "<<efficiency<<" %"
	<<"\n -I- CbmStsTimeBasedQa: Ghost      : "<<ghost<<" %";*/
}

void CbmStsDigitizeQa::ReadDataBranches(){
    FairRootManager* ioman = FairRootManager::Instance();
    if ( NULL == ioman )
	LOG(FATAL) << GetName() << ": No FairRootManager!" << FairLogger::endl;

    fStsPoints = (TClonesArray*)ioman -> GetObject("StsPoint");
    if ( NULL == fStsPoints )
	LOG(ERROR) << GetName() << ": No StsPoint array!" << FairLogger::endl;

    fStsDigis = (TClonesArray*) ioman -> GetObject("StsDigi");
    if ( NULL == fStsDigis )
	LOG(ERROR) << GetName() << ": No StsDigi array!" << FairLogger::endl;
}

void CbmStsDigitizeQa::CreateHistograms(){
    CreateNofObjectsHistograms();
    CreateDigiHistograms();
    fHM -> Create1<TH1F>("h_EventNo_DigitizeQa", "h_EventNo_DigitizeQa", 1, 0, 1.);
}

void CbmStsDigitizeQa::CreateNofObjectsHistograms(){
    Int_t nofBins = 100;
    Double_t minX = -0.5;
    Double_t maxX = 49999.5;
    string name = "h_NofObjects_";
    fHM -> Create1<TH1F>(name + "Points", name + "Points;Objects per event;Entries", nofBins, minX, maxX);
    fHM -> Create1<TH1F>(name + "Digis",  name + "Digis;Objects per event;Entries",  nofBins, minX, maxX);

    nofBins = 8;
    minX = -0.5;
    maxX = 7.5;
    fHM -> Create1<TH1F>(name + "Points_Station", name + "Points_Station;Station number;Objects per event", nofBins, minX, maxX);
    fHM -> Create1<TH1F>(name + "Digis_Station",  name + "Digis_Station;Station number;Oblects per enent",   nofBins, minX, maxX);
}

void CbmStsDigitizeQa::CreateDigiHistograms(){
    Int_t nofBins = 25;
    Double_t minX = 0.5;
    Double_t maxX = minX + nofBins;
    fHM -> Create1<TH1F>("h_PointsInDigi", "PointsInDigi;Number of Points;Entries", nofBins, minX, maxX);
    fHM -> Create1<TH1F>("h_PointsInDigiLog", "PointsInDigi;Number of Points;Entries", nofBins, minX, maxX);
    fHM -> Create1<TH1F>("h_DigisByPoint", "DigisByPoint;Number of Digis;Entries" , nofBins, minX, maxX);
    fHM -> Create1<TH1F>("h_DigisByPointLog", "DigisByPoint;Number of Digis;Entries" , nofBins, minX, maxX);
    Double_t tempD; Int_t tempI, nAdc; 
    fDigitizer -> GetParameters(tempD, tempD, nAdc, tempD, tempD, tempD);
    nofBins = nAdc;
    fHM -> Create1<TH1F>("h_DigiCharge", "DigiCharge;Digi Charge, ADC;Entries", nofBins, 0., Double_t(nofBins));
    for (Int_t stationId = 0; stationId < fNofStation; stationId++){
	fHM -> Create2<TH2F>(Form("h_DigisPerChip_Station%i",stationId), 
		Form("Digis per Chip, Station %i;x, cm;y, cm", stationId), 400, -50, 50, 200, -50, 50);
	fHM -> Create2<TH2F>(Form("h_PointsMap_Station%i",stationId),    
		Form("Points Map, Station %i;x, cm;y, cm", stationId),     100, -50, 50, 100, -50, 50);
	fHM -> Create2<TH2F>(Form("h_MeanAngleMap_Station%i",stationId), 
		Form("Mean Angle Map, Station %i;x, cm;y, cm", stationId), 50, -50, 50, 50, -50, 50);
	fHM -> Create2<TH2F>(Form("h_RMSAngleMap_Station%i",stationId),  
		Form("RMS Angle Map, Station %i;x, cm;y, cm", stationId),  50, -50, 50, 50, -50, 50);
    }
    Double_t local[3] = {0.,0.,0.};
    Double_t global[3];
    for (Int_t moduId = 0; moduId < fSetup -> GetNofModules(); moduId++){
	CbmStsModule * modu  = static_cast<CbmStsModule*>(fSetup -> GetModule(moduId));
	TGeoPhysicalNode * node = modu -> CbmStsElement::GetDaughter(0) -> CbmStsElement::GetPnode();
	if ( node ){ 
	    TGeoMatrix * matrix = node -> GetMatrix();
	    matrix -> LocalToMaster(local, global);
	}
	fHM -> Create1<TH1F>(Form("h_ParticleAngles_%s", modu -> GetName()), 
		Form("Particle Angles (%.0f cm, %.0f cm);Angle, deg;Entries", global[0], global[1]), 90, 0., 90.);
    }
}

void CbmStsDigitizeQa::ProcessDigisAndPoints(const TClonesArray* digis, const TClonesArray * points){
    if ( NULL != digis && fHM -> Exists("h_NofObjects_Digis") ) fHM -> H1("h_NofObjects_Digis") -> Fill(digis -> GetEntriesFast());
    std::set<Double_t> pointIndexes;
    std::map<Double_t, Int_t> stations;
    std::map<Double_t, Int_t> digisByPoint;
    std::map<Double_t, Int_t>::iterator map_it;
    pointIndexes.clear();
    Double_t local[3] = {0.,0.,0.};
    Double_t global[3];
    for(Int_t iDigi = 0; iDigi < digis -> GetEntriesFast(); iDigi++) {
	const CbmStsDigi* stsDigi = static_cast<const CbmStsDigi*>(digis -> At(iDigi));
	const CbmMatch* digiMatch = static_cast<const CbmMatch*>(stsDigi -> GetMatch());
	Int_t stationId = CbmStsAddress::GetElementId(stsDigi -> GetAddress(), kStsStation);
	Int_t iLad = CbmStsAddress::GetElementId(stsDigi -> GetAddress(), kStsLadder);
	Int_t iHla = CbmStsAddress::GetElementId(stsDigi -> GetAddress(), kStsHalfLadder);
	Int_t iMod = CbmStsAddress::GetElementId(stsDigi -> GetAddress(), kStsModule);
	CbmStsModule * modu = static_cast<CbmStsModule*>(fSetup -> GetElement(stsDigi -> GetAddress(), kStsModule));
	Int_t nOfChannelsM = modu -> GetNofChannels();
	TGeoPhysicalNode * node = modu -> CbmStsElement::GetDaughter(0) -> CbmStsElement::GetPnode();
	if ( node ){ 
	    TGeoMatrix * matrix = node -> GetMatrix();
	    matrix -> LocalToMaster(local, global);
	}

	map<Int_t, pair<CbmStsDigi*, Int_t> >* digiMap = modu -> GetDigiMap();
	map<Int_t, pair<CbmStsDigi*, Int_t> >::iterator digiIt;

	for (digiIt = digiMap -> begin(); digiIt != digiMap -> end(); digiIt++) {
	    Int_t channel = digiIt -> first;  
	    Int_t iChip = channel / 128 ;
	    fnOfDigisChip[stationId][iLad][iHla][iMod][iChip]++;
	    fHM -> H2(Form("h_DigisPerChip_Station%i",stationId)) -> Fill(global[0] + 50. / 400. * ((iChip-8.) * 2. - 1.), global[1]);
	}

	for(Int_t iLink = 0; iLink < digiMatch -> GetNofLinks(); iLink++) {
	    const CbmLink link = digiMatch -> GetLink(iLink);
	    Double_t index = (1000 * link.GetIndex()) + (link.GetFile()) + (0.0001 * link.GetEntry());
	    pointIndexes.insert(index);
	    stations.insert(std::pair<Double_t, Int_t>(index, stationId));
	    Int_t channel = CbmStsAddress::GetElementId(stsDigi -> GetAddress(), kStsChannel);

	    Int_t side = channel < Int_t(nOfChannelsM / 2.) ? 0 : 1;
	    map_it = digisByPoint.find(index + (side * 0.00001));
	    if ( map_it != digisByPoint.end() ) {
		map_it -> second++;
	    } else {
		digisByPoint.insert(std::pair<Double_t, Int_t>(index + (side * 0.00001), 1));
	    }
	}
	fHM -> H1("h_NofObjects_Digis_Station") -> Fill(stationId);
	fHM -> H1("h_PointsInDigi") -> Fill(digiMatch -> GetNofLinks());
	fHM -> H1("h_PointsInDigiLog") -> Fill(digiMatch -> GetNofLinks());
	fHM -> H1("h_DigiCharge") -> Fill(stsDigi -> GetCharge());
    }
    fHM -> H1("h_NofObjects_Points") -> Fill(pointIndexes.size());
    std::set<Double_t>::iterator set_it;
    for(set_it = pointIndexes.begin(); set_it != pointIndexes.end(); ++set_it) {
	fHM -> H1("h_NofObjects_Points_Station") -> Fill(stations[*set_it]);
	fHM -> H1("h_DigisByPoint") -> Fill(digisByPoint[*set_it]);
	fHM -> H1("h_DigisByPoint") -> Fill(digisByPoint[*set_it + 0.00001]);
	fHM -> H1("h_DigisByPointLog") -> Fill(digisByPoint[*set_it]);
	fHM -> H1("h_DigisByPointLog") -> Fill(digisByPoint[*set_it + 0.00001]);
    }
    if ( pointIndexes.size() > fMaxScale ) fMaxScale = pointIndexes.size();

    Double_t pointX, pointY, pointZ;
    Double_t pointPX, pointPZ;
    for (Int_t iPoint = 0; iPoint < points -> GetEntriesFast(); iPoint ++){
	const FairMCPoint * stsPoint = static_cast<const FairMCPoint*>(points -> At(iPoint));
	CbmStsModule * modu = static_cast<CbmStsModule*>(fSetup -> GetElement(stsPoint -> GetDetectorID(), kStsModule));
	TGeoPhysicalNode * node = modu -> CbmStsElement::GetDaughter(0) -> CbmStsElement::GetPnode();
	if ( node ){ 
	    TGeoMatrix * matrix = node -> GetMatrix();
	    matrix -> LocalToMaster(local, global);
	}
	pointX = stsPoint -> GetX();
	pointY = stsPoint -> GetY();
	pointZ = stsPoint -> GetZ();
	pointPX = stsPoint -> GetPx();
	pointPZ = stsPoint -> GetPz();
	Int_t stationId = fSetup -> GetElement(stsPoint -> GetDetectorID(), kStsStation) -> GetIndex();
	fHM -> H2(Form("h_PointsMap_Station%i", stationId)) -> Fill(pointX, pointY);
	fHM -> H1(Form("h_ParticleAngles_%s", modu -> GetName())) -> Fill(TMath::Abs(TMath::ATan(pointPX / pointPZ)) * 180. / 3.1416);	
    }
}


void CbmStsDigitizeQa::ProcessAngles(){
    Double_t local[3] = {0.,0.,0.};
    Double_t global[3];
    for (Int_t iStation = 0; iStation < fNofStation; iStation++){
	CbmStsElement * stat = fSetup -> GetDaughter(iStation);
	for (Int_t iLad = 0; iLad < stat -> GetNofDaughters(); iLad++) {
	    CbmStsElement* ladd = stat -> GetDaughter(iLad);
	    for (Int_t iHla = 0; iHla < ladd -> GetNofDaughters(); iHla++) {
		CbmStsElement* hlad = ladd -> GetDaughter(iHla);
		for (Int_t iMod = 0; iMod < hlad -> GetNofDaughters(); iMod++) {
		    CbmStsElement* modu = hlad -> GetDaughter(iMod);
		    Double_t mean = fHM -> H1(Form("h_ParticleAngles_%s",modu -> GetName())) -> GetMean();
		    Double_t rms = fHM -> H1(Form("h_ParticleAngles_%s",modu -> GetName())) -> GetRMS();
		    TGeoPhysicalNode * node = modu -> CbmStsElement::GetDaughter(0) -> CbmStsElement::GetPnode();
		    if ( node ){ 
			TGeoMatrix * matrix = node -> GetMatrix();
			matrix -> LocalToMaster(local, global);
		    }
		    fHM -> H2(Form("h_MeanAngleMap_Station%i", iStation)) -> Fill(global[0],global[1],mean);
		    fHM -> H2(Form("h_RMSAngleMap_Station%i",  iStation)) -> Fill(global[0],global[1],rms);
		}
	    }
	}
    }
}
ClassImp(CbmStsDigitizeQa);
