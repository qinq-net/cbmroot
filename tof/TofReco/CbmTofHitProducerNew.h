// --------------------------------------------------------------------------
// -----             Header for the CbmTofHitProducer                  ------
// -----           Created by E. Cordier 14/09/05                      ------
// -----           Modified by D. Gonzalez-Diaz 07/09/06               ------
// -----           Modified by D. Gonzalez-Diaz 02/02/07               ------
// -----           Modified by nh 23/10/12                             ------
// -----           modified pal 28/09 to 15/10/2015                    ------
// --------------------------------------------------------------------------

#ifndef CBMTOFHITPRODUCERNEW_H
#define CBMTOFHITPRODUCERNEW_H

#include "FairTask.h"

// C++ Classes and includes
#include <vector>

class CbmTofGeoHandler;
class CbmTofDigiPar;
class CbmTofCell;

class TVector3;
class TClonesArray;
class TString;
class TH1;
class TH2;


class CbmTofHitProducerNew : public FairTask {

 public:

  CbmTofHitProducerNew();

  CbmTofHitProducerNew(const char *name, Int_t verbose = 1);

  virtual ~CbmTofHitProducerNew();

  virtual InitStatus Init();
  virtual InitStatus ReInit();
  virtual void SetParContainers();
  virtual void Exec(Option_t * option);
  virtual void Finish();

  void AddHit(Int_t detID, TVector3 &posHit, TVector3 &posHitErr,
	      Int_t ref, Double_t tHit, Int_t flag, Int_t iChannel);

  void SetSigmaT(Double_t sigma);
  void SetSigmaEl(Double_t sigma);
  void SetSigmaXY(Double_t sigma);
  void SetSigmaY(Double_t sigma);
  void SetSigmaZ(Double_t sigma);
  void SetParFileName(const std::string& fnam) { fParFileName = fnam;}
  void SetInitFromAscii(Bool_t ascii) {fParInitFromAscii=ascii;}
  void SetHistosFileName( TString inName ){ fsHistosFileName = inName;}

  Double_t GetSigmaT();
  Double_t GetSigmaEl();
  Double_t GetSigmaXY();
  Double_t GetSigmaY();
  Double_t GetSigmaZ();

private:

  Int_t fVerbose;

  TString fParFileName;   // name of the file name with geometry parameters
  void InitParametersFromContainer();

  TClonesArray *fTofPoints;     // TOF MC points
  TClonesArray *fMCTracks;      // MC tracks
  TClonesArray *fHitCollection; // TOFhits
  TClonesArray* fTofHitMatches; // CbmMatch array for Hit -> MC

  //Temporary substitute of the parameter container. FIXME 

  static const Int_t maxSMtyp=   8;
  static const Int_t maxnSM=   255;
  static const Int_t maxnMod=   64;
  static const Int_t maxnCell= 255;
/*
  Float_t X [maxSMtyp][maxnSM][maxnMod][maxnCell];  //X-Position in cm
  Float_t Dx[maxSMtyp][maxnSM][maxnMod][maxnCell];  //Resolution in position in cm
  Float_t Y [maxSMtyp][maxnSM][maxnMod][maxnCell];  //Y-Position in cm
  Float_t Dy[maxSMtyp][maxnSM][maxnMod][maxnCell];  //Resolution in position in cm
  Float_t Z [maxSMtyp][maxnSM][maxnMod][maxnCell];  //Z-Position in cm
  Int_t   Ch[maxSMtyp][maxnSM][maxnMod][maxnCell];  //Channel number
*/
  std::vector< std::vector< std::vector< std::vector< Float_t > > > > X ;  //X-Position in cm
  std::vector< std::vector< std::vector< std::vector< Float_t > > > > Dx;  //Resolution in position in cm
  std::vector< std::vector< std::vector< std::vector< Float_t > > > > Y ;  //Y-Position in cm
  std::vector< std::vector< std::vector< std::vector< Float_t > > > > Dy;  //Resolution in position in cm
  std::vector< std::vector< std::vector< std::vector< Float_t > > > > Z ;  //Z-Position in cm
  std::vector< std::vector< std::vector< std::vector< Int_t   > > > > Ch;  //Channel number

  Int_t ActSMtypMax;
  Int_t ActnSMMax[maxSMtyp];
  Int_t ActnModMax[maxSMtyp];
  Int_t ActnCellMax[maxSMtyp];

  Int_t ActSMtypMin;
  Int_t ActnSMMin[maxSMtyp];
  Int_t ActnModMin[maxSMtyp];
  Int_t ActnCellMin[maxSMtyp];

  //End of temporary data members

  //Members to store the hit during loop over all the points. FIXME
/*
  Float_t tl[maxSMtyp][maxnSM][maxnMod][maxnCell];
  Float_t tr[maxSMtyp][maxnSM][maxnMod][maxnCell];
  Int_t trackID_left[maxSMtyp][maxnSM][maxnMod][maxnCell];
  Int_t trackID_right[maxSMtyp][maxnSM][maxnMod][maxnCell];
  Int_t point_left [maxSMtyp][maxnSM][maxnMod][maxnCell];
  Int_t point_right[maxSMtyp][maxnSM][maxnMod][maxnCell];
*/
  std::vector< std::vector< std::vector< std::vector< Float_t > > > > tl;
  std::vector< std::vector< std::vector< std::vector< Float_t > > > > tr;
  std::vector< std::vector< std::vector< std::vector< Int_t   > > > > trackID_left;
  std::vector< std::vector< std::vector< std::vector< Int_t   > > > > trackID_right;
  std::vector< std::vector< std::vector< std::vector< Int_t   > > > > point_left ;
  std::vector< std::vector< std::vector< std::vector< Int_t   > > > > point_right;


  //End of storing data members

  Double_t fSigmaT;       //Resolution in time in ns 
  Double_t fSigmaEl;      //Electronics time   in ns 
  Double_t fSigmaXY;      //Resolution in x/y  in cm
  Double_t fSigmaY;       //Resolution in y    in cm
  Double_t fSigmaZ;       //Resolution in z    in cm
  
  TString fVersion;       //
  Int_t  fNHits;          //Index of the CbmTofHit TClonesArray

  CbmTofGeoHandler* fGeoHandler;
  CbmTofDigiPar  *fDigiPar;
  CbmTofCell     *fCellInfo;

  Bool_t fParInitFromAscii;
    
   // Hit Quality for Hits coming from a single MC Point
   TH1 * fhSinglePointHitDeltaX;
   TH1 * fhSinglePointHitDeltaY;
   TH1 * fhSinglePointHitDeltaZ;
   TH1 * fhSinglePointHitDeltaR;
   TH1 * fhSinglePointHitDeltaT;
   TH1 * fhSinglePointHitPullX;
   TH1 * fhSinglePointHitPullY;
   TH1 * fhSinglePointHitPullZ;
   TH1 * fhSinglePointHitPullR;
   // Hit Quality for Hits coming from different MC Point on strip ends
      // To MC Point on left side
   TH1 * fhDiffPointHitLeftDeltaX;
   TH1 * fhDiffPointHitLeftDeltaY;
   TH1 * fhDiffPointHitLeftDeltaZ;
   TH1 * fhDiffPointHitLeftDeltaR;
   TH1 * fhDiffPointHitLeftDeltaT;
   TH1 * fhDiffPointHitLeftPullX;
   TH1 * fhDiffPointHitLeftPullY;
   TH1 * fhDiffPointHitLeftPullZ;
   TH1 * fhDiffPointHitLeftPullR;
      // To MC Point on right side
   TH1 * fhDiffPointHitRightDeltaX;
   TH1 * fhDiffPointHitRightDeltaY;
   TH1 * fhDiffPointHitRightDeltaZ;
   TH1 * fhDiffPointHitRightDeltaR;
   TH1 * fhDiffPointHitRightDeltaT;
   TH1 * fhDiffPointHitRightPullX;
   TH1 * fhDiffPointHitRightPullY;
   TH1 * fhDiffPointHitRightPullZ;
   TH1 * fhDiffPointHitRightPullR;
   
   // Hit Quality for Hits coming from multiple a single MC Track but multiple points
   TH1 * fhSingleTrackHitLeftDeltaX;
   TH1 * fhSingleTrackHitLeftDeltaY;
   TH1 * fhSingleTrackHitLeftDeltaZ;
   TH1 * fhSingleTrackHitLeftDeltaR;
   TH1 * fhSingleTrackHitLeftDeltaT;
   TH1 * fhSingleTrackHitLeftPullX;
   TH1 * fhSingleTrackHitLeftPullY;
   TH1 * fhSingleTrackHitLeftPullZ;
   TH1 * fhSingleTrackHitLeftPullR;
   TH1 * fhSingleTrackHitRightDeltaX;
   TH1 * fhSingleTrackHitRightDeltaY;
   TH1 * fhSingleTrackHitRightDeltaZ;
   TH1 * fhSingleTrackHitRightDeltaR;
   TH1 * fhSingleTrackHitRightDeltaT;
   TH1 * fhSingleTrackHitRightPullX;
   TH1 * fhSingleTrackHitRightPullY;
   TH1 * fhSingleTrackHitRightPullZ;
   TH1 * fhSingleTrackHitRightPullR;
   // Hit Quality for Hits coming from different MC Track on strip ends
      // To MC Point from Track on left side
   TH1 * fhDiffTrackHitLeftDeltaX;
   TH1 * fhDiffTrackHitLeftDeltaY;
   TH1 * fhDiffTrackHitLeftDeltaZ;
   TH1 * fhDiffTrackHitLeftDeltaR;
   TH1 * fhDiffTrackHitLeftDeltaT;
   TH1 * fhDiffTrackHitLeftPullX;
   TH1 * fhDiffTrackHitLeftPullY;
   TH1 * fhDiffTrackHitLeftPullZ;
   TH1 * fhDiffTrackHitLeftPullR;
      // To MC Point from Track on right side
   TH1 * fhDiffTrackHitRightDeltaX;
   TH1 * fhDiffTrackHitRightDeltaY;
   TH1 * fhDiffTrackHitRightDeltaZ;
   TH1 * fhDiffTrackHitRightDeltaR;
   TH1 * fhDiffTrackHitRightDeltaT;
   TH1 * fhDiffTrackHitRightPullX;
   TH1 * fhDiffTrackHitRightPullY;
   TH1 * fhDiffTrackHitRightPullZ;
   TH1 * fhDiffTrackHitRightPullR;
   
      // DEBUG histograms for efficiency losses
   TH2 * fhNbPrimTrkHits;
   TH2 * fhNbAllTrkHits;
   TH2 * fhNbPrimTrkTofHits;
   TH2 * fhNbAllTrkTofHits;
   
   TString fsHistosFileName;
   void CreateHistos();
   void WriteHistos();
   void DeleteHistos();

  CbmTofHitProducerNew(const CbmTofHitProducerNew&);
  CbmTofHitProducerNew& operator=(const CbmTofHitProducerNew&);
  
  ClassDef(CbmTofHitProducerNew,1) //CbmTofHitProducer

};
#endif 

