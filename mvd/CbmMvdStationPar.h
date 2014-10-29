// -------------------------------------------------------------------------
// -----                      CbmMvdStationPar header file             -----
// -----                  Created 28/10/14 by P.Sitzmann               -----
// -------------------------------------------------------------------------


/** CbmMvdStationPar.h
 *@author P.Sitzmann <p.sitzmann@gsi.de>
 **
 ** Parameter class for the CbmMvdDetector class.
 ** It holds the parameters necessary for tracking.
 **/


#ifndef CBMMVDSTATIONPAR_H
#define CBMMVDSTATIONPAR_H 1



#include <vector>
#include <map>
#include "TNamed.h"

using std::map;

class CbmMvdStationPar : public TNamed
{

 public:

  /** Default constructor **/
  CbmMvdStationPar();

  CbmMvdStationPar(Int_t stationCount);
  
  /** Destructor **/
  virtual ~CbmMvdStationPar();


  /** Accessors **/
  Int_t    GetStationCount() const { return fStationCount; };
  Double_t GetZPosition(Int_t stationNumber);
  Double_t GetThickness(Int_t stationNumber);
  Double_t GetHeight(Int_t stationNumber);
  Double_t GetWidth(Int_t stationNumber);
  Double_t GetXRes(Int_t stationNumber);
  Double_t GetYRes(Int_t stationNumber);
  Double_t GetRadLength(Int_t stationNumber);
  Double_t GetBeamHeight(Int_t stationNumber);
  Double_t GetBeamWidth(Int_t stationNumber);


  /** Data interface */
  void SetStationCount(Int_t count) { fStationCount = count; };
  void SetZPosition(Int_t stationNumber, Double_t z);
  void SetThickness(Int_t stationNumber, Double_t thickness);
  void SetHeight(Int_t stationNumber, Double_t height);
  void SetWidth(Int_t stationNumber, Double_t width);
  void SetXRes(Int_t stationNumber, Double_t xres);
  void SetYRes(Int_t stationNumber, Double_t yres);
  void SetRadLength(Int_t stationNumber, Double_t length);
  void SetBeamHeight(Int_t stationNumber, Double_t beamheight);
  void SetBeamWidth(Int_t stationNumber, Double_t beamwidth);


  /** Output to screen **/
  virtual void Print(Option_t* opt="") const;



 protected:

  Int_t                   fStationCount;        // Number of Stations
  map<Int_t , Double_t >  fZPositions;          // map of the z positions of all Stations 
  map<Int_t , Double_t >  fThicknesses;         //
  map<Int_t , Double_t >  fHeights; 
  map<Int_t , Double_t >  fWidths;  
  map<Int_t , Double_t >  fXResolutions;
  map<Int_t , Double_t >  fYResolutions;
  map<Int_t , Double_t >  fRadiationLength;
  map<Int_t , Double_t >  fBeamHeights;
  map<Int_t , Double_t >  fBeamWidths;


  ClassDef(CbmMvdStationPar,1);

};



#endif
