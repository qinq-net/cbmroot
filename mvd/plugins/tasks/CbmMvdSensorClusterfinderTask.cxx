// -------------------------------------------------------------------------
// -----                  CbmMvdSensorClusterfinderTask source file    -----
// -----                  Created 03.12.2014 by P. Sitzmann            -----
// -------------------------------------------------------------------------

#include "CbmMvdSensorClusterfinderTask.h"
#include "TClonesArray.h"
#include "TObjArray.h"
#include "FairLogger.h"



// -----   Default constructor   -------------------------------------------
CbmMvdSensorClusterfinderTask::CbmMvdSensorClusterfinderTask() 
  : CbmMvdSensorTask(),
    fAdcDynamic(200),
    fAdcOffset(0),
    fAdcBits(1),
    fAdcSteps(-1),
    fAdcStepSize(-1.),
    fDigis(NULL),
    fPixelChargeHistos(NULL),
    fTotalChargeInNpixelsArray(NULL),
    fResolutionHistoX(NULL),
    fResolutionHistoY(NULL),
    fResolutionHistoCleanX(NULL),
    fResolutionHistoCleanY(NULL),
    fResolutionHistoMergedX(NULL),
    fResolutionHistoMergedY(NULL),
    fBadHitHisto(NULL),
  fGausArray(NULL),
  fGausArrayIt(-1),
  fGausArrayLimit(5000),  
  fDigiMap(),
  fDigiMapIt(),
  h(NULL),
  h3(NULL),
  h1(NULL),
  h2(NULL),
  Qseed(NULL),
  fFullClusterHisto(NULL),
  c1(NULL),
  fNEvent(0),
  fMode(0),
  fCounter(0),
  fSigmaNoise(15.),
  fSeedThreshold(1.),
  fNeighThreshold(1.),
  fShowDebugHistos(kFALSE),
  fUseMCInfo(kFALSE),
  fLayerRadius(0.),
  fLayerRadiusInner(0.),
  fLayerPosZ(0.),
  fHitPosX(0.),
  fHitPosY(0.),
  fHitPosZ(0.),
  fHitPosErrX(0.0005),
  fHitPosErrY(0.0005),
  fHitPosErrZ(0.0),
  fBranchName("MvdHit"),
  fAddNoise(kFALSE),
  inputSet(kFALSE),
ftempPixelMap()
{


}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmMvdSensorClusterfinderTask::~CbmMvdSensorClusterfinderTask() 
{
;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmMvdSensorClusterfinderTask::CbmMvdSensorClusterfinderTask(const char* name, Int_t iMode,
			       Int_t iVerbose)
  : CbmMvdSensorTask(),
    fAdcDynamic(200),
    fAdcOffset(0),
    fAdcBits(1),
    fAdcSteps(-1),
    fAdcStepSize(-1.),
    fDigis(NULL),
    fPixelChargeHistos(NULL),
    fTotalChargeInNpixelsArray(NULL),
    fResolutionHistoX(NULL),
    fResolutionHistoY(NULL),
    fResolutionHistoCleanX(NULL),
    fResolutionHistoCleanY(NULL),
    fResolutionHistoMergedX(NULL),
    fResolutionHistoMergedY(NULL),
    fBadHitHisto(NULL),
  fGausArray(NULL),
  fGausArrayIt(-1),
  fGausArrayLimit(5000),  
  fDigiMap(),
  fDigiMapIt(),
  h(NULL),
  h3(NULL),
  h1(NULL),
  h2(NULL),
  Qseed(NULL),
  fFullClusterHisto(NULL),
  c1(NULL),
  fNEvent(0),
  fMode(iMode),
  fCounter(0),
  fSigmaNoise(15.),
  fSeedThreshold(1.),
  fNeighThreshold(1.),
  fShowDebugHistos(kFALSE),
  fUseMCInfo(kFALSE),
  fLayerRadius(0.),
  fLayerRadiusInner(0.),
  fLayerPosZ(0.),
  fHitPosX(0.),
  fHitPosY(0.),
  fHitPosZ(0.),
  fHitPosErrX(0.0005),
  fHitPosErrY(0.0005),
  fHitPosErrZ(0.0),
  fBranchName("MvdHit"),
  fAddNoise(kFALSE),
  inputSet(kFALSE),
ftempPixelMap()
{    
   
}
// -------------------------------------------------------------------------




// -----    Virtual private method Init   ----------------------------------
void CbmMvdSensorClusterfinderTask::Init(CbmMvdSensor* mysensor) {


  fSensor = mysensor;
 //cout << "-Start- " << GetName() << ": Initialisation of sensor " << fSensor->GetName() << endl;
   fInputBuffer = new TClonesArray("CbmMvdDigi",10000); 
   fOutputBuffer= new TClonesArray("CbmMvdCluster", 10000);
   
 
    //Add charge collection histograms
    fPixelChargeHistos=new TObjArray();

     fTotalChargeInNpixelsArray = new TObjArray();
 Int_t adcMax = fAdcOffset + fAdcDynamic;
    fAdcSteps= (Int_t)TMath::Power(2,fAdcBits);
    fAdcStepSize  = fAdcDynamic/fAdcSteps;




initialized = kTRUE;

   //cout << "-Finished- " << GetName() << ": Initialisation of sensor " << fSensor->GetName() << endl;
}
// -------------------------------------------------------------------------

// -----   Virtual public method Reinit   ----------------------------------
Bool_t CbmMvdSensorClusterfinderTask::ReInit() {
    cout << "-I- " <<"CbmMvdSensorClusterfinderTask::ReInt---------------"<<endl;
    return kTRUE;
}
// -------------------------------------------------------------------------

// -----   Virtual public method ExecChain   --------------
void CbmMvdSensorClusterfinderTask::ExecChain() {
  
  Exec();

 
}
// -------------------------------------------------------------------------

// -----   Public method Exec   --------------
void CbmMvdSensorClusterfinderTask::Exec() {   
if(fInputBuffer->GetEntriesFast() > 0)
  {
fOutputBuffer->Clear();
inputSet = kFALSE;
vector<Int_t>* clusterArray=new vector<Int_t>;
        
CbmMvdDigi* digi;
    
Int_t iDigi=0;
digi = (CbmMvdDigi*) fInputBuffer->At(iDigi);
Int_t number = 0;
    
    if(!digi){
	cout << "-E- : CbmMvdSensorFindHitTask - Fatal: No Digits found in this event."<< endl;
    }

Int_t nDigis = fInputBuffer->GetEntriesFast();
Double_t pixelSizeX = digi->GetPixelSizeX();
Double_t pixelSizeY = digi->GetPixelSizeY();
nDigis = fInputBuffer->GetEntriesFast();
TArrayS* pixelUsed  = new TArrayS(nDigis);

    for ( iDigi=0; iDigi<nDigis; iDigi++) {
	pixelUsed->AddAt(0,iDigi);
    }

fDigiMap.clear();
Int_t refId;
	for(Int_t k=0;k<nDigis;k++){

	    digi = (CbmMvdDigi*) fInputBuffer->At(k);
	    refId = digi->GetRefId();
	    if ( refId < 0)
		{
		LOG(FATAL) << "RefID of this digi is -1 this should not happend "<< FairLogger::endl;
		}
	    //apply fNeighThreshold
	   
	    if(GetAdcCharge(digi->GetCharge()) < fNeighThreshold ) continue;

	    pair<Int_t, Int_t> a (digi->GetPixelX(),digi->GetPixelY());
	    //cout << endl << "registerde pixel x:" << digi->GetPixelX() << " y:" << digi->GetPixelY() << endl;
	    fDigiMap[a]=k;
	};


	if( gDebug>0 ){cout << "\n-I- " << GetName() << ": VolumeId " << fSensor->GetVolumeId() << endl;}

	for ( iDigi=0; iDigi<nDigis; iDigi++) {

	    if( gDebug>0 && iDigi%10000==0 ){ cout << "-I- " << GetName() << " Digi:" << iDigi << endl; };

	    digi = (CbmMvdDigi*) fInputBuffer->At(iDigi);
            //cout << endl << "working with pixel x:" << digi->GetPixelX() << " y:" << digi->GetPixelY() << endl;
	 

	    /*
	     ---------------------------------------------------------
	     check if digi is above threshold (define seed pixel)
	     then check for neighbours.
	     Once the cluster is created (seed and neighbours)
	     calculate the position of the hit
	     using center of gravity (CoG) method.
	     ---------------------------------------------------------
	     */

	    if( gDebug>0 ){
		cout << "-I- " << "CbmMvdSensorFindHitTask: Checking for seed pixels..." << endl;
	    }

	    if( ( GetAdcCharge(digi->GetCharge())>=fSeedThreshold ) && ( pixelUsed->At(iDigi)==kFALSE ) ){
		clusterArray->clear();
		clusterArray->push_back(iDigi);

		pixelUsed->AddAt(1,iDigi);

		pair<Int_t, Int_t> a(digi->GetPixelX(), digi->GetPixelY());
    		fDigiMapIt = fDigiMap.find(a);
    		fDigiMap.erase(fDigiMapIt);

		for ( Int_t iCluster=0; iCluster<clusterArray->size(); iCluster++ ){

		    if( gDebug>0 ){
			cout << "-I- " << " CbmMvdSensorClusterfinderTask: Calling method CheckForNeighbours()..." << endl;
		    }

		    CheckForNeighbours(clusterArray, iCluster, pixelUsed);
		     //cout << endl << "checked for neighbours, create cluster" << endl;   

		}

			Int_t i=0;
			Int_t pixelCharge;
			Int_t pixelX;
			Int_t pixelY;
			pair<Int_t, Int_t> pixelCoords;
    			Int_t clusterSize=clusterArray->size();
			Int_t nClusters = fOutputBuffer->GetEntriesFast();
			//cout << endl << "new cluster: " << nClusters << endl;
			CbmMvdCluster* clusterNew=new ((*fOutputBuffer)[nClusters]) CbmMvdCluster();
	 		clusterNew->SetDetectorId(fSensor->GetDetectorID());
			clusterNew->SetStationNr(fSensor->GetStationNr());
   			for(i=0;i<clusterSize;i++) 
			{
	      		CbmMvdDigi* digiInCluster = (CbmMvdDigi*) fInputBuffer->At(clusterArray->at(i));
			clusterNew->AddDigi(digiInCluster->GetRefId());
			pixelCoords = std::make_pair(digiInCluster->GetPixelX(),digiInCluster->GetPixelY());
			pixelCharge = digiInCluster->GetCharge();
			ftempPixelMap[pixelCoords] = pixelCharge;
			}
			clusterNew->SetPixelMap(ftempPixelMap);
			ftempPixelMap.clear();
	  			
    				
	    }// if AdcCharge>threshold
		else {//cout << endl << "pixel is with " <<  digi->GetCharge() << " under Threshold or used" << endl;
			}
	}// loop on digis


delete pixelUsed;
clusterArray->clear();
delete clusterArray;
fInputBuffer->Clear();

fDigiMap.clear();
}
else {//cout << endl << "No input found." << endl;
     }
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMvdSensorClusterfinderTask::CheckForNeighbours(vector<Int_t>* clusterArray, Int_t clusterDigi, TArrayS* pixelUsed)
{
    CbmMvdDigi* seed = (CbmMvdDigi*)fInputBuffer->At(clusterArray->at(clusterDigi));
    //cout << endl << "pixel nr. " << clusterDigi << " is seed" << endl ;
   
    CbmMvdDigi* digiOfInterest;
   
    	
    // Remove Seed Pixel from list of non-used pixels
    Int_t channelX=seed->GetPixelX();
    Int_t channelY=seed->GetPixelY();
    pair<Int_t, Int_t> a(channelX, channelY);
    
    // Find first neighbour
    
    a=std::make_pair(channelX+1, channelY);
    fDigiMapIt=fDigiMap.find(a);

    if (!(fDigiMapIt == fDigiMap.end()))
    	{ 
	  Int_t i=fDigiMap[a];
	  //cout << endl << "pixel nr. " << i << " is used" << endl ;	  	  
	  // Only digis depassing fNeighThreshold are in the map, no cut required
	  clusterArray->push_back(i);
	  	  
	  pixelUsed->AddAt(1,i); // block pixel for the seed pixel scanner
	  fDigiMap.erase(fDigiMapIt); // block pixel for the neighbour pixel scanner
	 } 
	  
    a = std::make_pair(channelX-1, channelY);
    fDigiMapIt = fDigiMap.find(a);

    if (!(fDigiMapIt == fDigiMap.end()))
    {
	Int_t i=fDigiMap[a];
	//cout << endl << "pixel nr. " << i << " is used" << endl ;
	// Only digits depassing fNeighThreshold are in the map, no cut required
	clusterArray->push_back(i);
	pixelUsed->AddAt(1,i); // block pixel for the seed pixel scanner
	fDigiMap.erase(fDigiMapIt); // block pixel for the neighbour pixel scanner
    }

    a = std::make_pair(channelX, channelY-1);
    fDigiMapIt=fDigiMap.find(a);
    if (!(fDigiMapIt == fDigiMap.end()))
    {
	Int_t i=fDigiMap[a];
	// Only digits depassing fNeighThreshold are in the map, no cut required
       //cout << endl << "pixel nr. " << i << " is used" << endl ;
	clusterArray->push_back(i);
	pixelUsed->AddAt(1,i); // block pixel for the seed pixel scanner
	fDigiMap.erase(fDigiMapIt); // block pixel for the neighbour pixel scanner
    }

    a = std::make_pair (channelX, channelY+1);
    fDigiMapIt=fDigiMap.find(a);

    if (!(fDigiMapIt == fDigiMap.end()))
    {
	Int_t i=fDigiMap[a];
        //cout << endl << "pixel nr. " << i << " is used" << endl ;
	// Only digis depassing fNeighThreshold are in the map, no cut required
	clusterArray->push_back(i);
	pixelUsed->AddAt(1,i); // block pixel for the seed pixel scanner
	fDigiMap.erase(fDigiMapIt); // block pixel for the neighbour pixel scanner
    }




}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Int_t CbmMvdSensorClusterfinderTask::GetAdcCharge(Float_t charge)
{

    Int_t adcCharge;

    if(charge<fAdcOffset){return 0;};

    adcCharge = int( (charge-fAdcOffset)/fAdcStepSize );
    if ( adcCharge>fAdcSteps-1 ) {adcCharge = fAdcSteps-1;}

    return adcCharge;

}
// -------------------------------------------------------------------------
ClassImp(CbmMvdSensorClusterfinderTask)
