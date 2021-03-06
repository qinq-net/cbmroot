// ------------------------------------------------------------------------------
// -----                    CbmMvdDigitizeL header file                     -----
// -----                    Created by C.Dritsa (2009)                      -----
// -----                    Maintained by M.Deveaux (m.deveaux(att)gsi.de   -----
// ------------------------------------------------------------------------------


/** CbmMvdDigitizeL header file
 ** Read "ReadmeMvdDigitizer.pdf" for instructions
 **
 **/


#ifndef CBMMVDDIGITIZEL_H
#define CBMMVDDIGITIZEL_H 1

//#include "omp.h"
#include "FairTask.h"
#include "CbmMvdPoint.h"
#include "CbmMvdDigi.h"
#include "CbmMvdDigiMatch.h"
#include "MyG4UniversalFluctuationForSi.h"

#include "TRandom3.h"
#include "TStopwatch.h"
#include "TString.h"
#include "TMath.h"
#include "TH1.h"
#include "TH2.h"
#include "TRefArray.h"
#include "TObjArray.h"
#include <vector>
#include <list>
#include <map>
#include <utility>
#include "CbmMvdPixelCharge.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"



class TClonesArray;
class TRandom3;
class CbmMvdGeoPar;
class CbmMvdPileupManager;
class CbmMvdStation;


class CbmMvdDigitizeL : public FairTask
{
 
 public:

  /** Default constructor **/  
  CbmMvdDigitizeL();


  /** Standard constructor 
  *@param name  Task name
  *@param mode  0 = MAPS, 1 = Ideal
  **/
  CbmMvdDigitizeL(const char* name, 
		    Int_t mode = 0, Int_t iVerbose = 1);


  /** Destructor **/
  virtual ~CbmMvdDigitizeL();


  /** Task execution **/
  virtual void Exec(Option_t* opt);


  /** Accessors **/
  TString  GetBranchName()    const { return fBranchName; };


  /** Added by CDritsa **/

  void ProduceIonisationPoints(CbmMvdPoint* point, CbmMvdStation* station);
  //void ProduceSignalPoints();
  void ProducePixelCharge(CbmMvdPoint* point, CbmMvdStation* station);
  void TransformXYtoPixelIndex(Double_t x, Double_t y,Int_t & ix, Int_t & iy);
  void TransformPixelIndexToXY(Int_t ix, Int_t iy, Double_t & x, Double_t & y );
//  void PositionWithinCell(Double_t x, Double_t y,  Int_t & ix, Int_t & iy, Double_t & xCell, Double_t & yCell);
 // void AddChargeToPixel(Int_t channelX, Int_t channelY, Int_t charge, CbmMvdPoint* point);
  Int_t BuildEvent();
//  Double_t GetDetectorGeometry(CbmMvdPoint* point);

  /** Modifiers **/
  void SetEpiThickness(Double_t epiTh)              { fEpiTh = epiTh;                     }
  void SetSegmentLength(Double_t segmentLength)     { fSegmentLength = segmentLength;     }
  void SetDiffusionCoef(Double_t diffCoeff)         { fDiffusionCoefficient = diffCoeff;  }
  void SetElectronsPerKeV(Double_t electronsPerKeV) { fElectronsPerKeV = electronsPerKeV; }
  void SetWidthOfCluster(Double_t widthOfCluster)   { fWidthOfCluster = widthOfCluster;   }
  void SetPixelSizeX(Double_t pixelSizeX)           { fPixelSizeX = pixelSizeX;           }
  void SetPixelSizeY(Double_t pixelSizeY)           { fPixelSizeY = pixelSizeY;           }
  void SetCutOnDeltaRays(Double_t cutOnDeltaRays)   { fCutOnDeltaRays = cutOnDeltaRays;   }
  void SetChargeThreshold(Double_t chargeThreshold) { fChargeThreshold = chargeThreshold; }
  void SetPixelSize(Double_t pixelSize);
//  void SetMvdGeometry(Int_t detId);
  void SetPileUp(Int_t pileUp)           { fNPileup         = pileUp;      }
  void SetDeltaEvents(Int_t deltaEvents) { fNDeltaElect     = deltaEvents; }
  void SetBgFileName(TString fileName)   { fBgFileName      = fileName;    }
  void SetDeltaName(TString fileName)    { fDeltaFileName   = fileName;    }
  void SetBgBufferSize(Int_t nBuffer)    { fBgBufferSize    = nBuffer;     }
  void SetDeltaBufferSize(Int_t nBuffer) { fDeltaBufferSize = nBuffer;     }

  void ShowDebugHistograms() {fShowDebugHistos = kTRUE;}



//protected:
public:

    // ----------   Protected data members  ------------------------------------

    Double_t fEpiTh;
    Double_t fSegmentLength;

    Double_t fDiffusionCoefficient;
    Double_t fElectronsPerKeV;
    Double_t fWidthOfCluster;
    Double_t fPixelSizeX;
    Double_t fPixelSizeY;
    Double_t fCutOnDeltaRays;
    Double_t fChargeThreshold;
    Double_t fFanoSilicium;
   
    Double_t fEsum;
    Double_t fSegmentDepth;
    Double_t fCurrentTotalCharge;
    Double_t fCurrentParticleMass;
    Double_t fCurrentParticleMomentum;
    Int_t    fCurrentParticlePdg;

    TH1F* fRandomGeneratorTestHisto;
    TH2F* fPosXY;
    TH1F* fpZ;
    TH1F* fPosXinIOut;
    TH1F* fAngle;
    TH1F* fSegResolutionHistoX;
    TH1F* fSegResolutionHistoY;
    TH1F* fSegResolutionHistoZ;
    TH1F* fTotalChargeHisto;
    TH1F* fTotalSegmentChargeHisto;


    Double_t fLorentzY0;
    Double_t fLorentzXc; 
    Double_t fLorentzW;
    Double_t fLorentzA;
    Double_t fLorentzNorm;
  
    Double_t fLandauMPV;
    Double_t fLandauSigma;
    Double_t fLandauGain;
    TRandom3* fLandauRandom;

    Double_t fPixelSize;
    Double_t fPar0;
    Double_t fPar1;
    Double_t fPar2;

    Bool_t fShowDebugHistos;
    TH1F* fResolutionHistoX;
    TH1F* fResolutionHistoY;

    Int_t fNumberOfSegments;
    Int_t fCurrentLayer;
    Int_t fEvent;
    Int_t fVolumeId;

    MyG4UniversalFluctuationForSi * fFluctuate;

    TClonesArray* fDigis;
    TClonesArray* fDigiMatch;
    TClonesArray* fMCTracks;
    TClonesArray* fPixelCharge;
    
    std::vector<CbmMvdPixelCharge*> fPixelChargeShort;

    TObjArray* fPixelScanAccelerator;
    map<pair<Int_t, Int_t>, CbmMvdPixelCharge*> fChargeMap;
    map<pair<Int_t, Int_t>, CbmMvdPixelCharge*>::iterator fChargeMapIt;
    


private:

  /** Hit producer mode (0 = MAPS, 1 = Ideal) **/
  Int_t fMode;


  /** MAPS properties **/
  Double_t fSigmaX, fSigmaY;   // MAPS resolution in cm
  Double_t fEfficiency;        // MAPS detection efficiency
  Double_t fMergeDist;         // Merging distance
  Double_t fFakeRate;          // Fake hit rate
  Int_t    fNPileup;           // Number of pile-up background events
  Int_t    fNDeltaElect;       // Number of delta electron events
  Int_t    fDeltaBufferSize;
  Int_t    fBgBufferSize;


  /** Map from station number to MvdStation **/
  std::map<Int_t, CbmMvdStation*> fStationMap;                  //!   



  /** IO arrays **/
  TString	fBranchName;     // Name of input branch (STSPoint)
  TString       fBgFileName;     // Name of background (pileup) file 
  TString       fDeltaFileName;  // Name of the file containing delta electrons
  TClonesArray* fInputPoints;     // Array of MCPoints (input)
  TRefArray* fPoints;            // Array of all MCPoints (including background files)


  /** Random generator and Stopwatch **/
  TRandom3   fRandGen; 
  TStopwatch fTimer;
  

  /** Pileup manager **/
  CbmMvdPileupManager* fPileupManager;
  CbmMvdPileupManager* fDeltaManager;


  /** Parameters **/
  CbmMvdGeoPar*  fGeoPar;


  /** Counters **/ 
  Int_t    fNEvents;
  Double_t fNPoints;
  Double_t fNReal;
  Double_t fNBg;
  Double_t fNFake;
  Double_t fNLost;
  Double_t fNMerged;
  Double_t fTime;
  
  // -----   Private methods   ---------------------------------------------

  struct SignalPoint {
      double x;
      double y;
      double z;
      double sigmaX;
      double sigmaY;
      double charge;
      double eloss;

  };


  typedef std::vector<SignalPoint> SignalPointVec;
  
  SignalPointVec fSignalPoints;//!


  /** Set parameter containers **/
  virtual void SetParContainers();
  

  /** Intialisation **/
  virtual InitStatus Init();


  /** Reinitialisation **/
  virtual InitStatus ReInit();


  /** Virtual method Finish **/
  virtual void Finish();


  /** Register the output arrays to the IOManager **/
  void Register();


  /** Clear the hit arrays **/
  void Reset();	 


  /** Print digitisation parameters **/
  void PrintParameters();


 
  /** Get MVD geometry parameters from database 
   **@value Number of MVD stations
   **/
  Int_t GetMvdGeometry();
 

  TH1F* h_trackLength;
  TH1F* h_numSegments;
  TH2F* h_LengthVsAngle;
  TH2F* h_LengthVsEloss;
  TH2F* h_ElossVsMomIn;

  CbmMvdDigitizeL(const CbmMvdDigitizeL&);
  CbmMvdDigitizeL& operator=(const CbmMvdDigitizeL&);

  ClassDef(CbmMvdDigitizeL,1);
    
};
    
    
#endif   		     
