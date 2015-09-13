// -------------------------------------------------------------------------
// -----                   CbmTofFindTracks header file                -----
// -----                  Created 25/04/15  by N. Herrmann              -----
// -----                 according to the CbmTrdFindTracks             -----
// -------------------------------------------------------------------------

/** CbmTofFindTracks
 **
 ** Task class for track finding in the TOF.
 ** Input:  TClonesArray of CbmTofHit
 ** Output: TClonesArray of CbmTofTrack
 **
 ** Uses as track finding algorithm classes derived from CbmTofTrackFinder.
 **/


#ifndef CBMTOFFINDTRACKS
#define CBMTOFFINDTRACKS 1

#include "FairTask.h"
//#include "CbmTofTypes.h"
#include <vector>

class CbmTofTrackFinder;
class CbmTofTrackFitter;
class TClonesArray;
class TH1;
class TH2;
class TH3;
class CbmTofDigiPar;
class CbmTofDigiBdfPar;

class CbmTofFindTracks : public FairTask
{
  friend class CbmTofTrackFinderNN;

 public:

  /** Default constructor **/
  CbmTofFindTracks();


  /** Standard constructor 
   **
   *@param name   Name of class
   *@param title  Task title
   *@param finder Pointer to STS track finder concrete class
   **/
  CbmTofFindTracks(const char* name, const char* title = "FairTask", 
		   CbmTofTrackFinder* finder = NULL);

  /** Destructor **/
  virtual ~CbmTofFindTracks();
  
  inline static CbmTofFindTracks *Instance(){ return fInstance; }
  

  /** Initialisation at beginning of each event **/
  virtual InitStatus Init();

  /** Task execution **/
  virtual void Exec(Option_t* opt);

  /** Finish at the end of each event **/
  virtual void Finish();

  /** SetParContainers **/
  virtual void SetParContainers();

  virtual void CreateHistograms();

  virtual void FindVertex();

  virtual void FillHistograms();

  /** Accessors **/
  CbmTofTrackFinder* GetFinder() { return fFinder; };
  CbmTofTrackFitter* GetFitter() { return fFitter; };
  Int_t GetNofTracks()           { return fNofTracks; };
  Int_t GetNofStations()         { return fNTofStations; };

  /** Set concrete track finder **/
  void UseFinder(CbmTofTrackFinder* finder) { fFinder = finder; };
  void UseFitter(CbmTofTrackFitter* fitter) { fFitter = fitter; };

  inline void SetMinNofHits (Int_t i)         { fMinNofHits = i;};
  inline void SetNStations (Int_t i)          { fNTofStations = i;};
  inline void SetStations  (Int_t ival) { 
    fStationType.resize(fNTofStations);
    for (Int_t i=0; i<10; i++)  fTypeStation[i]=-1; // initialize
    for (Int_t i=0; i<fNTofStations; i++){
     Int_t iSm = ival%10;
     Int_t iSt = fNTofStations-1-i;
     fStationType[iSt] = iSm;
     fTypeStation[iSm] = iSt;       
     ival = (ival-iSm)/10;
    }
  };

  inline Int_t GetMinNofHits() const   { return fMinNofHits;}
  inline Int_t GetNStations() const    { return fNTofStations;}
  inline Int_t GetStationType(Int_t i) { return fStationType[i]; }
  inline Int_t GetTypeStation(Int_t i) { return fTypeStation[i]; }
  inline Int_t GetCorMode   () const   { return fiCorMode;}

  inline void SetCorMode   (Int_t ival){ fiCorMode = ival;}
  inline void SetCalParFileName(TString CalParFileName) { fCalParFileName = CalParFileName; }
  inline void SetTtTarg(Double_t val){ fTtTarg=val; }

 private:

  static CbmTofFindTracks *fInstance;
  CbmTofTrackFinder* fFinder;    // Pointer to TrackFinder concrete class
  CbmTofTrackFitter* fFitter;    // Pointer to TrackFitter concrete class
  TClonesArray* fTofHitArray;    // Input array of TOF hits
  TClonesArray* fTrackArray;     // Output array of CbmTofTracks 

  Int_t fMinNofHits;             // minimal number of Tof Hits for filling histos 
  Int_t fNofTracks;              // Number of tracks created
  Int_t fNTofStations;           // Number of Tof Stations 
  std::vector<Int_t> fStationType; // Station SM type 

  CbmTofFindTracks(const CbmTofFindTracks&);
  CbmTofFindTracks& operator=(const CbmTofFindTracks&);

  // Control histograms 
  TH1* fhTrklMul;
  TH1* fhTrklChi2;
  TH1* fhAllHitsStation;
  TH1* fhAllHitsSmTypes;
  TH1* fhUsedHitsStation;

  TH2* fhTrklMulNhits;
  TH2* fhTrklMul34;
  TH3* fhTrklMul3D;
  TH2* fhTrklHMul;
  TH2* fhTrklZ0xHMul;
  TH2* fhTrklZ0yHMul;
  TH2* fhTrklTxHMul;
  TH2* fhTrklTyHMul;
  TH2* fhTrklTtHMul;
  TH2* fhTrklVelHMul;
  TH2* fhTrklT0HMul;
  TH2* fhTrklT0Mul;
  TH2* fhTrklDT0SmMis;
  TH2* fhTrklDT0StMis2;
  TH2* fhTrklXY0_3;
  TH2* fhTrklXY0_4;
  TH2* fhTrklXY0_5;

  std::vector<TH1 *> vhPullX;
  std::vector<TH1 *> vhPullY;
  std::vector<TH1 *> vhPullZ;
  std::vector<TH1 *> vhPullT;
  std::vector<TH1 *> vhPullTB;
  std::vector<TH2 *> vhXY_AllStations;      // for efficiency estimation
  std::vector<TH2 *> vhXY_MissedStation;    // for efficiency estimation

  TH1* fhVTXNorm;
  TH2* fhVTX_XY0;
  TH2* fhVTX_DT0_Norm;

  Int_t fTypeStation[10];  // FIXME fixed array size
  TString  fOutHstFileName;      // name of the histogram output file name with Calibration Parameters

  Bool_t   LoadCalParameter();
  Bool_t   WriteHistos();
  TString       fCalParFileName;      // name of the file name with Calibration Parameters
  TFile*        fCalParFile;          // pointer to Calibration Parameter file 
  TH2*          fhPullT_Smt;           // Time calibration histo
  TH1*          fhPullT_Smt_Off;       // Time calibration histo
  TH2*          fhTOff_Smt;              // Time calibration histo
  TH1*          fhTOff_Smt_Off;          // Time calibration histo
  TH2*          fhDeltaTt_Smt;           // Time calibration histo
  Int_t         fiCorMode;
  Double_t      fTtTarg;                // expected average slope in ps/cm 
  Double_t      fVTXNorm;               // Number of Hits contributing to Vertex determination 
  Double_t      fVTX_T;                 // measured event wise t0 
  Double_t      fVTX_X;                 // measured event wise vertex x  
  Double_t      fVTX_Y;                 // measured event wise vertex y
  Double_t      fVTX_Z;                 // measured event wise vertex z

  // ToF geometry variables

  CbmTofDigiPar         * fDigiPar;
  CbmTofDigiBdfPar      * fDigiBdfPar;

  ClassDef(CbmTofFindTracks,1);

};

#endif
