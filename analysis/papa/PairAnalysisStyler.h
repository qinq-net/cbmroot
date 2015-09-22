#ifndef PAIRANALYSISSTYLER_H
#define PAIRANALYSISSTYLER_H
///////////////////////////////////////////////////////////////////////////////////////////
//                                                                                       //
// PairAnalysis stylers                                                                    //
//                                                                                       //
//                                                                                       //
// Authors:                                                                              //
//   Julian Book <Julian.Book@cern.ch>                                                   //
//                                                                                       //
///////////////////////////////////////////////////////////////////////////////////////////


#include <TStyle.h>
#include <TAttMarker.h>
#include <Rtypes.h>
#include <TLegend.h>

namespace PairAnalysisStyler
{
  // predefined style for signal extraction
  enum Eidx   { kRaw=100, kBgrd, kSig, kFit, kNidx };
  static Int_t    fCol[kNidx]={kBlack,kBlue,kBlack,kRed};
  static Int_t    fMrk[kNidx]={kFullCircle,kOpenCircle,kOpenCircle,kDot};
  static Double_t fSze[kNidx]={1.,1.,1.,1.};
  static Int_t    fLne[kNidx]={kSolid,kSolid,kSolid,kSolid};
  static Double_t fWdt[kNidx]={2.,2.,2.,2.};
  static Int_t    fFll[kNidx]={0,0,0,0}; //kFEmpty

  enum Estyle { kNMaxMarker=8, kNMaxLine=4, kNMaxColor=9 };
  static Int_t Marker[]= {kFullCircle,
				kFullDiamond,
				kFullSquare,
				kFullCross,
				kFullStar,
				kMultiply,
				kPlus,
				kStar }; // kNMaxMarker

  static Int_t Line[]= {kSolid,
			      kDashed,
			      kDotted,
			      //			      9,
			      kDashDotted }; // kNMaxLine

  static Int_t Color[]= {kRed-4,
			       kBlue-4,
			       kGray+1,
			       kGreen+1,
			       kAzure+1,
			       kOrange+1,
			       kSpring+4,
			       kViolet+1,
			       kCyan+2 }; // kNMaxColor

  extern TStyle *fUserDielStyle;   // user defined style
  extern void SetStyle(TStyle *userStyle);

  extern void SetStyle(Eidx idx, Int_t col=kBlack, Int_t marker=kOpenCircle, Double_t size=1.5, Int_t line=kSolid, Double_t width=2., Int_t fill=kFEmpty);

  void LoadStyle();
  void Style(TObject *obj, Int_t idx=0);
  void SetForceLineStyle(Int_t line=kSolid);
  

  extern UInt_t fLegAlign;           // legend alignement (11,12,21,22)
  void SetLegendAlign(UInt_t align);
  void SetLegendCoordinates(TLegend *leg);
  
}

#endif
