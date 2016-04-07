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
#include <TColor.h>
#include <Rtypes.h>
#include <TLegend.h>
#include <TH1.h>

namespace PairAnalysisStyler
{
  // predefined style for signal extraction
  enum Eidx   { kRaw=100, kBgrd, kSig, kFit, kCocktail, kNidx };
  static Int_t    fCol[kNidx-kRaw]={kBlack,kTeal-8,kBlack,kTeal-7,kTeal-7};
  static Int_t    fMrk[kNidx-kRaw]={kFullCircle,kOpenCircle,kOpenCircle,kDot,kDot};
  static Double_t fSze[kNidx-kRaw]={1.,1.,1.,1.,1.};
  static Int_t    fLne[kNidx-kRaw]={kSolid,kSolid,kSolid,kSolid,kSolid};
  static Double_t fWdt[kNidx-kRaw]={2.,2.,2.,2.,2.};
  static Int_t    fFll[kNidx-kRaw]={0,0,0,0,0}; //kFEmpty

  enum Estyle { kNMaxMarker=13, kNMaxLine=4, kNMaxColor=17 };
  static Int_t Marker[]= {kFullCircle,
			  kFullDiamond,
			  kFullSquare,
			  kFullCross,
			  kFullStar,
			  kMultiply,
			  kPlus,
			  kStar,
			  kOpenCircle,
			  kOpenDiamond,
			  kOpenSquare,
			  kOpenCross,
			  kOpenStar}; // kNMaxMarker

  static Int_t Line[]= {kSolid,
			kDashed,
			kDotted,
			//			      9,
			kDashDotted }; // kNMaxLine

  static Int_t Color[]= {kRed-4
			 ,kBlue-4
			 ,kBlack
			 ,kGreen+1
			 ,kAzure+1
			 ,kOrange+2
			 ,kSpring+4
			 ,kViolet+1
			 ,kOrange
			 ,kGray+1
			 ,kRed+2
			 ,kCyan+1
			 ,kGreen+3
			 ,kBlue+1
			 ,kMagenta+1
			 ,kOrange-6
			 ,kCyan-2
  }; // kNMaxColor

  static TStyle *fUserDielStyle = NULL;   // user defined style
  void SetStyle(TStyle *userStyle);

  void SetStyle(Eidx idx, Int_t col=kBlack, Int_t marker=kOpenCircle, Double_t size=1.5, Int_t line=kSolid, Double_t width=2., Int_t fill=kFEmpty);

  void LoadStyle();
  void Style(TObject *obj, Int_t idx=0);
  void SetForceLineStyle(Int_t line=kSolid);
  void SetForceColor(Int_t color=kBlack);

  enum Epalette { kDefault=0, kGoodBad };
  void SetPalette(Epalette colors=kDefault, Bool_t reverse=kFALSE);

  static UInt_t fLegAlign = 22;           // legend alignement (11,12,21,22)
  void SetLegendAlign(UInt_t align);
  void SetLegendAttributes(TLegend *leg, Bool_t fill=kFALSE);

  TH1* GetFirstHistogram();

} // Styling (static)

#endif
