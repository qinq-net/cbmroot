///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                                                                       //
// Authors:
//   Julian Book   <Julian.Book@cern.ch>
/*

  This class provides a common TStyle for all PAPa plots including
  usefull functions to automatically set (predefined) styles:
   SetStyle(), SetForceLineStyle(), SetPalette(), SetLegendAttributes()
  place legends SetLegendAlign().

  The PAPa style can be replaced by the method call SetStyle(TStyle *userStyle).

*/
//                                                                       //
///////////////////////////////////////////////////////////////////////////

#include <TROOT.h>
#include <TError.h>
#include <TStyle.h>
#include <TGaxis.h>
#include <TAttMarker.h>
#include <TAttLine.h>
#include <TAttFill.h>
#include <TColor.h>
#include <TCollection.h>
#include <TH1.h>
#include <TPad.h>
#include <TLegend.h>
#include <TLegendEntry.h>
#include <TLatex.h>

#include "PairAnalysisStyler.h"

// const Int_t PairAnalysisStyler::Marker[] = {kOpenCircle,
// 					   kOpenDiamond,
// 					   kOpenSquare,
// 					   kOpenCross,
// 					   kOpenStar,
// 					   kMultiply,
// 					   kPlus,
// 					   kStar };

TStyle *PairAnalysisStyler::fUserDielStyle=0x0;   // user defined style
void PairAnalysisStyler::SetStyle(TStyle *userStyle) {
  //
  // set and load user defined style
  //
  fUserDielStyle=userStyle;
  LoadStyle();
}

//_____________________________________________________________________________
void PairAnalysisStyler::LoadStyle() {
  //
  // load style
  //

  //  return;
  // load style defined by the user
  if(fUserDielStyle) { fUserDielStyle->cd(); gROOT->ForceStyle(); }
  else {

    Int_t font      = 42;
    Int_t bgrdcolor = 0; // set to -1 for transparaent, will causes warnings

    TStyle *defaultSty = new TStyle("default","default");
    defaultSty->SetPaperSize(TStyle::kA4);
    defaultSty->SetTextFont(font);

    // For the canvas:
    defaultSty->SetCanvasBorderMode(0);
    defaultSty->SetCanvasColor(bgrdcolor); //transparent
    defaultSty->SetCanvasDefH(600); //Height of canvas
    defaultSty->SetCanvasDefW(600); //Width of canvas
    defaultSty->SetCanvasDefX(0);   //POsition on screen
    defaultSty->SetCanvasDefY(0);

    // For the Pad:
    defaultSty->SetPadBorderMode(0);
    // defaultSty->SetPadBorderSize(Width_t size = 1);
    defaultSty->SetPadColor(bgrdcolor);        //transparent
    defaultSty->SetPadGridX(false);
    defaultSty->SetPadGridY(false);
    defaultSty->SetGridColor(0);
    defaultSty->SetGridStyle(3);
    defaultSty->SetGridWidth(1);

    // For the frame:
    defaultSty->SetFrameBorderMode(0);
    defaultSty->SetFrameBorderSize(1);
    defaultSty->SetFrameFillColor(bgrdcolor);  //transparent
    defaultSty->SetFrameFillStyle(0);
    defaultSty->SetFrameLineColor(1);
    defaultSty->SetFrameLineStyle(1);
    defaultSty->SetFrameLineWidth(1);

    // Margins:
    defaultSty->SetPadTopMargin(0.075);
    defaultSty->SetPadBottomMargin(0.18);
    defaultSty->SetPadLeftMargin(0.18); //0.13
    defaultSty->SetPadRightMargin(0.075);

    // For the histo:
    defaultSty->SetHistFillColor(bgrdcolor);    //transparent
    // defaultSty->SetHistFillStyle(0);
    defaultSty->SetHatchesSpacing(2.);   // spacing between hatches, def.=1.
    defaultSty->SetHatchesLineWidth(2); // hatches line width.
    defaultSty->SetHistLineColor(1);
    defaultSty->SetHistLineStyle(0);
    defaultSty->SetHistLineWidth(2);
    // defaultSty->SetLegoInnerR(Float_t rad = 0.5);
    defaultSty->SetHistMinimumZero();
    //    defaultSty->SetEndErrorSize(2);
    defaultSty->SetErrorX(0.);
    defaultSty->SetLineColor(1);
    defaultSty->SetMarkerColor(1);
    defaultSty->SetMarkerStyle(kFullCircle); // Kind of dot used for points
    defaultSty->SetMarkerSize(1.5);

    
    //For the fit/function:
    defaultSty->SetOptFit(1);
    defaultSty->SetFitFormat("5.4g");
    defaultSty->SetFuncColor(2);
    defaultSty->SetFuncStyle(1);
    defaultSty->SetFuncWidth(1);
    
    //For the date:
    defaultSty->SetOptDate(22); // bottom right only date
    // defaultSty->SetDateX(Float_t x = 0.01);
    // defaultSty->SetDateY(Float_t y = 0.01);
    defaultSty->GetAttDate()->SetTextColor(kGray);
    defaultSty->GetAttDate()->SetTextFont(font);
    defaultSty->GetAttDate()->SetTextSize(0.02);

    // For the statistics box:
    defaultSty->SetOptFile(0);
    defaultSty->SetOptStat(0); // To display the mean and RMS:   SetOptStat("mr");
    //  defaultSty->SetOptStat("n"); // only name
    defaultSty->SetStatColor(kWhite);
    defaultSty->SetStatFont(font);
    defaultSty->SetStatFontSize(0.02);
    defaultSty->SetStatTextColor(kGray);
    defaultSty->SetStatFormat("6.4g");
    defaultSty->SetStatBorderSize(1);
    defaultSty->SetStatH(0.1);
    defaultSty->SetStatW(0.5);
    defaultSty->SetStatStyle(kFEmpty);
    defaultSty->SetStatX(0.01);
    defaultSty->SetStatY(0.01);
    
    // For the Global title:
    defaultSty->SetOptTitle(0);
    defaultSty->SetTitleFont(font,"T");
    defaultSty->SetTitleSize(0.05,"T");
    defaultSty->SetTitleColor(bgrdcolor,"T");
    defaultSty->SetTitleTextColor(1);
    defaultSty->SetTitleFillColor(bgrdcolor);  //transparent
    defaultSty->SetTitleFontSize(0.06);
    // defaultSty->SetTitleH(0); // Set the height of the title box
    defaultSty->SetTitleW(0.5); // Set the width of the title box
    defaultSty->SetTitleX(0.25); // Set the position of the title box
    // defaultSty->SetTitleY(0.985); // Set the position of the title box
    defaultSty->SetTitleStyle(kFEmpty);
    // defaultSty->SetTitleBorderSize(2);
    defaultSty->SetPaintTextFormat(".3f"); // format if you plot with TEXT option

    // for Paves and boxes
    //    defaultSty->SetFillColor(bgrdcolor); // this makes palettes unicolored
    defaultSty->SetFillStyle(kFEmpty);
    defaultSty->SetLineColor(0);
    defaultSty->SetLineWidth(0);
    defaultSty->SetLineStyle(1);

    // For the axis titles:
    defaultSty->SetTitleColor(1, "XYZ");
    defaultSty->SetTitleFont(font, "XYZ");
    defaultSty->SetTitleSize(0.06, "XYZ");             //
    // defaultSty->SetTitleXSize(Float_t size = 0.02); // Another way to set the size?
    // defaultSty->SetTitleYSize(Float_t size = 0.02);
    defaultSty->SetTitleXOffset(1.0); //  defaultSty->SetTitleXOffset(1.08);
    defaultSty->SetTitleYOffset(1.25); //  defaultSty->SetTitleYOffset(1.5);
    //defaultSty->SetTitleOffset(1.0, "XYZ"); // Another way to set the Offset

    // For the axis labels:
    defaultSty->SetLabelColor(1, "XYZ");
    defaultSty->SetLabelFont(font, "XYZ");
    defaultSty->SetLabelOffset(0.007, "XYZ");
    defaultSty->SetLabelSize(0.04, "XYZ"); //  defaultSty->SetLabelSize(0.06, "XYZ");
    defaultSty->SetLabelSize(0.03, "Z"); //  defaultSty->SetLabelSize(0.06, "XYZ");

    // For the axis:
    defaultSty->SetHistMinimumZero();
    defaultSty->SetAxisColor(1, "XYZ");
    //    defaultSty->SetStripDecimals(kTRUE);
    //    defaultSty->SetTickLength(0.03, "XYZ");
    defaultSty->SetNdivisions(810, "XYZ");  //  defaultSty->SetNdivisions(505, "XYZ");
    defaultSty->SetPadTickX(1);  // To get tick marks on the opposite side of the frame
    defaultSty->SetPadTickY(1);

    // Change for log plots:
    defaultSty->SetOptLogx(0);
    defaultSty->SetOptLogy(0);
    defaultSty->SetOptLogz(0);

    // For the colored palette
    //    defaultSty->SetPalette(1,0);
    const Int_t NRGBs = 5;
    const Int_t NCont = 255;
    // beautiful colors blue to red
    Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51};
    Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00};
    // Double_t green[NRGBs] = { 0.20, 0.81, 1.00, 0.20, 0.00};
    //Double_t blue[NRGBs]  = { 0.0, 0.00, 0.0, 0.00, 0.00};
    Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00};
    Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00};
    TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    defaultSty->SetNumberContours(NCont);


    // For the legends:
    defaultSty->SetLegendFillColor(bgrdcolor);
    //defaultSty->SetLegendFillColor(kWhite);
    defaultSty->SetLegendFont(font);
    //    defaultSty->SetLegendTextSize(0.02); //for root>v6??

    //                                       Additions -- transparent style
    defaultSty->SetFillStyle(4000);
    defaultSty->SetStatStyle(0);
    defaultSty->SetTitleStyle(0);
    defaultSty->SetCanvasBorderSize(0);
    defaultSty->SetFrameBorderSize(0);
    defaultSty->SetLegendBorderSize(0);
    defaultSty->SetStatBorderSize(0);
    defaultSty->SetTitleBorderSize(0);

    // set current style to default
    //  gROOT->SetStyle(sty);
    defaultSty->cd();
    gROOT->ForceStyle();
    fUserDielStyle=defaultSty;

    // axis
    //    TGaxis::SetMaxDigits(4);
    TGaxis::SetMaxDigits(3);
    TGaxis::SetExponentOffset(-1.*0.06,0.,"y");
  }

}

void PairAnalysisStyler::Style(TObject *obj, Int_t idx) {
  //
  // style object according to index
  //
  Int_t id=0;
  if(!obj) return;

  // axis attributes
  if (obj->InheritsFrom(TH1::Class()) && gPad) {
    // reset axis names in order to allow repositioning of exponents 
    dynamic_cast<TH1*>(obj)->GetXaxis()->SetName("xaxis");
    dynamic_cast<TH1*>(obj)->GetYaxis()->SetName("yaxis");
    // logarithmic labels
    if(gPad->GetLogx()) {
      dynamic_cast<TH1*>(obj)->GetXaxis()->SetMoreLogLabels(kTRUE);
      dynamic_cast<TH1*>(obj)->GetXaxis()->SetNoExponent(kTRUE);
    }
    if(gPad->GetLogy()) {
      dynamic_cast<TH1*>(obj)->GetYaxis()->SetMoreLogLabels(kTRUE);
      dynamic_cast<TH1*>(obj)->GetYaxis()->SetNoExponent(kTRUE);
    }
    if(gPad->GetLogz()) {
      dynamic_cast<TH1*>(obj)->GetZaxis()->SetMoreLogLabels(kTRUE);
      dynamic_cast<TH1*>(obj)->GetZaxis()->SetNoExponent(kTRUE);
    }
  }

  // marker attributes
  if (obj->InheritsFrom(TAttMarker::Class())) {
    if(idx>=100) { // predefined styles
      dynamic_cast<TAttMarker*>(obj)->SetMarkerSize( fSze[idx-100]);
      dynamic_cast<TAttMarker*>(obj)->SetMarkerStyle(fMrk[idx-100]);
      dynamic_cast<TAttMarker*>(obj)->SetMarkerColor(fCol[idx-100]);
    }
    else {
      dynamic_cast<TAttMarker*>(obj)->SetMarkerSize(1.5);
      dynamic_cast<TAttMarker*>(obj)->SetMarkerStyle(Marker[idx%kNMaxMarker]);
      dynamic_cast<TAttMarker*>(obj)->SetMarkerColor(Color[idx%kNMaxColor]);
      if(idx>=kNMaxColor && idx<kNMaxColor*2) {
	idx=idx%kNMaxColor;
	dynamic_cast<TAttMarker*>(obj)->SetMarkerColor(TColor::GetColorDark(Color[idx]));
      }
      else if(idx>=kNMaxColor*2) {
	idx=idx%(2*kNMaxColor);
	dynamic_cast<TAttMarker*>(obj)->SetMarkerColor(TColor::GetColorBright(Color[idx]));
      }
    }
  }

  // line attributes
  if (obj->InheritsFrom(TAttLine::Class())) {
    //    printf("index %d for %s \n",idx,obj->GetName());
    if(idx>=100) { // predefined styles
      dynamic_cast<TAttLine*>(obj)->SetLineWidth(fWdt[idx-100]);
      dynamic_cast<TAttLine*>(obj)->SetLineStyle(fLne[idx-100]);
      dynamic_cast<TAttLine*>(obj)->SetLineColor(fCol[idx-100]);
    }
    else {
      dynamic_cast<TAttLine*>(obj)->SetLineWidth(2);
      dynamic_cast<TAttLine*>(obj)->SetLineStyle(Line[idx%kNMaxLine]);
      dynamic_cast<TAttLine*>(obj)->SetLineColor(Color[idx%kNMaxColor]);
      if(idx>=kNMaxColor && idx<kNMaxColor*2) {
	idx=idx%kNMaxColor;
	dynamic_cast<TAttLine*>(obj)->SetLineColor(TColor::GetColorDark(Color[idx]));
      }
      else if(idx>=kNMaxColor*2) {
	idx=idx%(2*kNMaxColor);
	dynamic_cast<TAttLine*>(obj)->SetLineColor(TColor::GetColorBright(Color[idx]));
      }
    }
  }

  // fill attributes
  // if (obj->InheritsFrom(TAttFill::Class()))
  //   dynamic_cast<TAttFill*>(obj)->SetFillStyle();
  //   dynamic_cast<TAttFill*>(obj)->SetFillColor();

}

void PairAnalysisStyler::SetForceLineStyle(Int_t line)
{
  //
  // force a certain line style
  //
  for(Int_t i=0; i<kNMaxLine; i++) {
    Line[i] = line;
  }
}


void PairAnalysisStyler::SetStyle(Eidx idx, Int_t col, Int_t marker, Double_t size, Int_t line, Double_t width, Int_t fill)
{
  //
  // change predefined style according to index
  //
  fCol[idx]=col;
  fMrk[idx]=marker;
  fSze[idx]=size;
  fLne[idx]=line;
  fWdt[idx]=width;
  fFll[idx]=fill;
}


UInt_t PairAnalysisStyler::fLegAlign=22; //top-right
void PairAnalysisStyler::SetLegendAlign(UInt_t align) { fLegAlign=align; }
void PairAnalysisStyler::SetLegendAttributes(TLegend *leg, Bool_t fill)
{
  //
  // set/update legend cooordinates according to alignement (stored in uniqueID)
  //
  // if(leg->GetUniqueID()==0) leg->SetUniqueID(fLegAlign);
  // UInt_t fLegAlign = leg->GetUniqueID();

   // printf("CURRENT leg coordinates: x1 %f x2 %f \t",leg->GetX1(),leg->GetX2());
   // printf(" y1 %f y2 %f \n",leg->GetY1(),leg->GetY2());

  Double_t maxhgth   = 0.0;
  Double_t symblwdth = 0.065; //ndc
  Double_t txtsze    = 0.04; //0.025; //gStyle->GetLegendTextSize());
  Double_t charwdth  = 0.01;//0.01; // own defintion
  Double_t entrysep  = 1.25;//1.25;  //entry seperation
  leg->SetTextSize(txtsze); // this should switch off the autosize

  // calculate get legend width
  Double_t maxwdth=0.0;
  TList *llist = leg->GetListOfPrimitives();
  Int_t nent = llist->GetEntries();

  if(nent>5) txtsze=0.025;
  for(Int_t il=0; il<nent; il++) {
    TLegendEntry *lent = static_cast<TLegendEntry*>(llist->At(il));
    TString lst(lent->GetLabel());
    //    lent->SetTextSize(20);
    /*    lst.ReplaceAll("#it","");
    lst.ReplaceAll("{","");
    lst.ReplaceAll("}","");
    lst.ReplaceAll("^","");
    lst.ReplaceAll("_","");
    lst.ReplaceAll("#LT","#");
    lst.ReplaceAll("#GT","#");
    lst.ReplaceAll("#phi","#");
    lst.ReplaceAll("#rho","#");
    lst.ReplaceAll("#omega","#");
    lst.ReplaceAll("#eta","#");
    lst.ReplaceAll("#psi","#");
    lst.ReplaceAll("#pi","#");
    lst.ReplaceAll("#alpha","#");
    lst.ReplaceAll("#gamma","#");
    lst.ReplaceAll("#rightarrow","#");
    */
    //    lst.ReplaceAll(" ","");
    TLatex entrytex( 0., 0., lst.Data());
    entrytex.SetNDC(kTRUE);
    entrytex.SetTextSize(txtsze);
    entrytex.SetTextFont(gStyle->GetLegendFont());
    //entrytex.SetTextFont(lent->GetTextFont());
    Double_t wdth = entrytex.GetXsize();
    Double_t hgth = entrytex.GetYsize();
    //wdth = lst.Length()*charwdth;
    if(maxwdth<wdth) maxwdth=wdth;
    // Printf("wdth %f and height %f for '%s' \t max width: %f by textsize %f for  nchars %d --> own wdth %f \n",
    // 	   wdth,entrytex.GetYsize(),entrytex.GetTitle(),maxwdth, entrytex.GetTextSize(),lst.Length(),lst.Length()*charwdth);
    maxhgth+=hgth;
  }

  maxhgth = nent*txtsze*entrysep;
  //  Bool_t addhdr = !(((TString)leg->GetHeader()).IsNull());
  //  maxhgth = (maxhgth/nent)*entrysep*(nent+1); // add legend header height

  // set legend coordinates
  if(fLegAlign==12 || fLegAlign==22) { //top
    leg->SetY2(1.-gPad->GetTopMargin()-gStyle->GetTickLength("X"));
    leg->SetY1(leg->GetY2()-maxhgth);
  }
  else { // bottom
    leg->SetY1(0.+gPad->GetBottomMargin()+gStyle->GetTickLength("X"));
    leg->SetY2(leg->GetY1()+maxhgth);
  }
  if(fLegAlign==22 || fLegAlign==21) {  //right
    leg->SetX2(1.-gPad->GetRightMargin()-gStyle->GetTickLength("Y")*1.0); //x2.0 ticklength
    leg->SetX1(leg->GetX2()-maxwdth*1.0 - symblwdth);
  }
  else if(fLegAlign==12 || fLegAlign==11) { //left
    leg->SetX1(0.+gPad->GetLeftMargin()+gStyle->GetTickLength("Y"));
    leg->SetX2(leg->GetX1()+maxwdth*1.0 + symblwdth);
  }

  // fix margin such that lines allways have the same length
  leg->SetMargin(symblwdth / (leg->GetX2()-leg->GetX1()));
  leg->SetEntrySeparation(entrysep-1.);

  // styling
  if(fill) leg->SetFillStyle(1001); // solid
  else     leg->SetFillStyle(kFEmpty); // empty
  // leg->SetFillColorAlpha(gStyle->GetLegendFillColor(), 0.8);
  //  leg->SetFillColorAlpha(gStyle->GetLegendFillColor(), 0.8);

   // printf("FINAL leg coordinates: x1 %f x2 %f \t",leg->GetX1(),leg->GetX2());
   // printf(" y1 %f y2 %f \n",leg->GetY1(),leg->GetY2());

}

void PairAnalysisStyler::SetPalette(Epalette colors, Bool_t reverse)
{
  //
  // use own definition of color palettes for zaxis
  //

  const Int_t NRGBs = 5;
  const Int_t NCont = 255;
  Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00};

  switch(colors) {
  case kDefault: {
    Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51};
    Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00};
    Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00};
    if(reverse)     TColor::CreateGradientColorTable(NRGBs, stops, blue, green, red, NCont);
    else            TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    break;
  }
  case kGoodBad: {
    Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.36};
    Double_t green[NRGBs] = { 0.20, 0.81, 1.00, 0.20, 0.00};
    Double_t blue[NRGBs]  = { 0.00, 0.00, 0.00, 0.00, 0.00};
    if(reverse)     TColor::CreateGradientColorTable(NRGBs, stops, green, red, blue, NCont);
    else            TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    break;
  }
  }


  gStyle->SetNumberContours(NCont);


}

TH1 * PairAnalysisStyler::GetFirstHistogram()
{
  //
  // get the first histogram drawn on the pad
  // this can be modified in the usual way
  //
  if(!gPad) { Error("GetFirstHistogram","No pad found, return NULL pointer!!"); return 0x0; }

  TIter nextObj(gPad->GetListOfPrimitives());
  TObject *obj;
  while ((obj = nextObj())) {
    if(obj->InheritsFrom(TH1::Class())) {
      return (static_cast<TH1*>(obj));
    }
  }
  return 0x0;

}
