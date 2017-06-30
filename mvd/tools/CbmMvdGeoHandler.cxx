/**
 * \file CbmMvdGeoHandler.cxx
 * \author Philipp Sitzmann <p.sitzmann@gsi.de>
 * \brief addapted from TrdGeoHandler by Florian Uhlig <f.uhlig@gsi.de>
 */

#include "CbmMvdGeoHandler.h"


#include "FairLogger.h"

#include "TGeoVolume.h"
#include "TGeoBBox.h"
#include "TGeoNode.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TVirtualMC.h"
#include "TString.h"

#include <string>
#include <cstdlib>
#include <iostream>
#include <iomanip>   
using std::cout;
using std::endl;
using std::string;
using std::atoi;


//--------------------------------------------------------------------------
CbmMvdGeoHandler::CbmMvdGeoHandler() 
  : TObject(),
  fDetector(NULL),
  fStationPar(NULL),
  fStationMap(),
  fIsSimulation(kFALSE),
  fGeoPathHash(),   //!
  fCurrentVolume(), //!
  fVolumeShape(),   //!
  fGlobal(),     //! Global center of volume
  fGlobalMatrix(),  //!
  fLayerId(-1),       //!
  fModuleId(-1),     //!
  fModuleType(-1),    //!
  fStation(-1),       //! StationID
  fMother(""),
  fGeoTyp(),
  fVolId(),
  fStationNumber(-1),
  fWidth(0.),
  fHeight(0.),
  fRadLength(0.),
  fBeamwidth(0.),
  fBeamheight(0.),
  fThickness(0.),
  fXres(0.),
  fYres(0.),
  fStationName(""),
  fDetectorName(""),
  fSectorName(""),
  fQuadrantName(""),
  fSensorHolding(""),
  fSensorName(""),
  fnodeName("")
{
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
CbmMvdGeoHandler::~CbmMvdGeoHandler()
{

}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
void CbmMvdGeoHandler::Init(Bool_t isSimulation)
{
   fIsSimulation = isSimulation;
   GetPipe();
   GetGeometryTyp();
   if(!isSimulation)
	{
	    fStationPar = new CbmMvdStationPar();
            fDetector = CbmMvdDetector::Instance();
            fDetector->SetParameterFile(fStationPar);
	switch (fGeoTyp)
	{
	case scripted:
	case FourStation:
	case FourStationShift:
	fStationPar->SetNofStations(4);
	break;
	case ThreeStation:
	fStationPar->SetNofStations(3);
	break;
	case MiniCbm:
	fStationPar->SetNofStations(2);
	break;
	default:
	fStationPar->SetNofStations(0);
	}
	fStationPar->Init();
	}
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
Int_t CbmMvdGeoHandler::GetSensorAddress()
{
// In the simulation we can not rely on the TGeoManager information
// In the simulation we get the correct information only from gMC 

  return 1;
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
Int_t CbmMvdGeoHandler::GetSensorAddress(const TString& path)
{
  if (fGeoPathHash != path.Hash()) {
    NavigateTo(path);
  }
  return GetSensorAddress();
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
Double_t CbmMvdGeoHandler::GetSizeX(
      const TString& path)
{
   if (fGeoPathHash != path.Hash()) {
      NavigateTo(path);
   }
   return fVolumeShape->GetDX();
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
Double_t CbmMvdGeoHandler::GetSizeY(
      const TString& path)
{
   if (fGeoPathHash != path.Hash()) {
      NavigateTo(path);
   }
   return fVolumeShape->GetDY();
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
Double_t CbmMvdGeoHandler::GetSizeZ(
      const TString& path)
{
   if (fGeoPathHash != path.Hash()) {
      NavigateTo(path);
   }
   return fVolumeShape->GetDZ();
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
Double_t CbmMvdGeoHandler::GetZ(
      const TString& path)
{
   if (fGeoPathHash != path.Hash()) {
      NavigateTo(path);
   }
   return fGlobal[2];
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
Double_t CbmMvdGeoHandler::GetY(
      const TString& path)
{
   if (fGeoPathHash != path.Hash()) {
      NavigateTo(path);
   }
   return fGlobal[1];
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
Double_t CbmMvdGeoHandler::GetX(
      const TString& path)
{
   if (fGeoPathHash != path.Hash()) {
      NavigateTo(path);
   }
   return fGlobal[0];
}
//--------------------------------------------------------------------------


//--------------------------------------------------------------------------
Int_t CbmMvdGeoHandler::GetStation(
      const TString& path)
{
   if (fGeoPathHash != path.Hash()) {
      NavigateTo(path);
   }
   return fStation;
}
//--------------------------------------------------------------------------


//--------------------------------------------------------------------------
void CbmMvdGeoHandler::NavigateTo(
      const TString& path)
{
  //   cout << "path : " << path.Data() << endl;
   if (fIsSimulation) {
      LOG(FATAL) << "This method is not supported in simulation mode" << FairLogger::endl;
   } else {
      gGeoManager->cd(path.Data());
      fGeoPathHash = path.Hash();
      fCurrentVolume = gGeoManager->GetCurrentVolume();
      TString name = fCurrentVolume->GetName();
      LOG(DEBUG) << "this volume is " << name << FairLogger::endl;
      fVolumeShape = (TGeoBBox*)fCurrentVolume->GetShape();
      Double_t local[3] = {0., 0., 0.};  // Local center of volume
      gGeoManager->LocalToMaster(local, fGlobal);
      fGlobalMatrix = gGeoManager->GetCurrentMatrix();
	if(path.Contains("S0"))
	fStationNumber = 0;
	else if(path.Contains("S1"))
	fStationNumber = 1;
	else if(path.Contains("S2"))
	fStationNumber = 2;
	else if(path.Contains("S3"))
	fStationNumber = 3;
        else
	    LOG(FATAL) <<  "couldn't find Station in volume name, something seems fishy " << FairLogger::endl;

	LOG(DEBUG) << "I am on station: " << fStationNumber << FairLogger::endl;
	LOG(DEBUG) << "I am at X: " << fGlobal[0] << FairLogger::endl;
	LOG(DEBUG) << "I am at Y: " << fGlobal[1] << FairLogger::endl;
	LOG(DEBUG) << "I am at Z: " << fGlobal[2] << FairLogger::endl;

	local[0] = fVolumeShape->GetDX();
        local[1] = fVolumeShape->GetDY();
	Double_t fGlobalMax[3]; 
        gGeoManager->LocalToMaster(local, fGlobalMax);

	local[0] = -1*fVolumeShape->GetDX();
        local[1] = -1*fVolumeShape->GetDY();
	Double_t fGlobalMin[3]; 
        gGeoManager->LocalToMaster(local, fGlobalMin);

     	if(fGlobalMax[0] > fGlobalMin[0]){fWidth = fGlobalMax[0]; fBeamwidth = fGlobalMin[0];}else {fWidth = fGlobalMin[0]; fBeamwidth = fGlobalMax[0];}
  	if(fGlobalMax[1] > fGlobalMin[1]){fHeight = fGlobalMax[1]; fBeamheight = fGlobalMin[1];}else {fHeight = fGlobalMin[1]; fBeamheight = fGlobalMax[1];}

	// TODO: hard coded numbers, find other way only for Mvd_v14a / v14b / v15a
 	if(fStationNumber == 0) fRadLength = 0.24; 
	else if(fStationNumber == 1) fRadLength = 0.31;
	else if(fStationNumber == 2) fRadLength = 0.47; 
	else fRadLength = 0.49;  

  	fXres = 3.8; // TODO: pixelSizeX / sqrt{12}
  	fYres = 3.8; // TODO: pixelSizeY / sqrt{12}
   }
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
void CbmMvdGeoHandler::GetPipe()
{
TString pipeName = "pipe";
TString motherName;
Bool_t fail = kTRUE;

TObjArray* nodes = gGeoManager->GetTopNode()->GetNodes();

for (Int_t iNode = 0; iNode < nodes->GetEntriesFast(); iNode++)
    {
    TGeoNode* node = (TGeoNode*) nodes->At(iNode);
    if (TString(node->GetName()).Contains(pipeName, TString::ECaseCompare::kIgnoreCase))
       {
	   motherName = node->GetName();
	   fMother = Form("cave_1/%s/pipevac1_0", motherName.Data());
	   LOG(DEBUG) << "MvdGeoHandler found Mother: " << fMother << FairLogger::endl;
	   fail = kFALSE;
           break;
       }
    else continue;
    }
if(fail)
{
    LOG(DEBUG)<<"Check for MVD outside of pipe"<< FairLogger::endl;

    for (Int_t iNode = 0; iNode < nodes->GetEntriesFast(); iNode++)
    {
    TGeoNode* node = (TGeoNode*) nodes->At(iNode);
    if (TString(node->GetName()).Contains("mvd", TString::ECaseCompare::kIgnoreCase))
       {
	   motherName = node->GetName();
	   fMother = "cave_1";
	   LOG(DEBUG) << "MvdGeoHandler found Mother: " << fMother << FairLogger::endl;
	   LOG(WARNING) << "Mvd found outside of pipe, use this setup only in testing" << FairLogger::endl;
	   fail = kFALSE;
           break;
       }
    else continue;
    }
}
if(fail)
    LOG(FATAL)<<"MVD Geometry included, but pipe not found please check your setup" << FairLogger::endl;
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------

void CbmMvdGeoHandler::GetGeometryTyp()
{
if ( gGeoManager->CheckPath(fMother + "/Beamtimeosetupoobgnum_0"))
	{
	cout << endl << "Found Beamtimesetup" << endl;
	fGeoTyp = beamtest;
	}
else if (gGeoManager->CheckPath(fMother + "/MVDoMistraloquero012oStationo150umodigi_0"))
	{
	LOG(DEBUG) << "Found MVD with 3 Stations" << FairLogger::endl;
	fGeoTyp = ThreeStation;
	}
else if (gGeoManager->CheckPath(fMother + "/MVDo0123ohoFPCoextoHSoSo0123_0"))
	{
	LOG(DEBUG) << "Found MVD with 4 Stations" << FairLogger::endl;
	fGeoTyp = FourStation;
	fDetectorName = "/MVDo0123ohoFPCoextoHSoSo0123_0";
	}
else if (gGeoManager->CheckPath(fMother + "/MVDo1123ohoFPCoextoHSoSo1123_0"))
	{
	LOG(DEBUG) << "Found shifted MVD with 4 Stations" << FairLogger::endl;
	fGeoTyp = FourStationShift;
	fDetectorName = "/MVDo1123ohoFPCoextoHSoSo1123_0";
	}
else if (gGeoManager->CheckPath(fMother + "/MVDomCBM_0"))
	{
	LOG(DEBUG) << "Found mCBM MVD configuration" << FairLogger::endl;
	fDetectorName = "/MVDomCBM_0";
	fGeoTyp = MiniCbm;
	}
else if (gGeoManager->CheckPath(fMother + "/MVDomCBMorotated_0"))
        {
        LOG(DEBUG) << "Found mCBM MVD rotated configuration" << FairLogger::endl;
        fDetectorName = "/MVDomCBMorotated_0";
        fGeoTyp = MiniCbm;
	}
else if (gGeoManager->CheckPath(fMother + "/MVDscripted_0"))
        {
        LOG(DEBUG) << "Found scripted MVD configuration" << FairLogger::endl;
        fDetectorName = "/MVDscripted_0";
        fGeoTyp = scripted;
        }
else 
	{
	cout << endl << "Try standard Geometry" << endl;
	fGeoTyp = Default;
	}
 
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
Int_t CbmMvdGeoHandler::GetIDfromPath(TString path)
{
    // path is build as: cave_1/*pipe/MVDscripted_0/quadrant_S*_*/sensor_*/sensorActive
    Int_t id = 0;
    TString sensorName;

    TString quadrantName;
    TString stationName;

    gGeoManager->cd(path.Data());

    sensorName = gGeoManager->GetMother(1)->GetName();
    sensorName.Remove(0, 7);
    Int_t sensorNumber = sensorName.Atoi();

    quadrantName = gGeoManager->GetMother(2)->GetName();
    stationName = quadrantName(10);
    quadrantName.Remove(0, 12);
    Int_t quadNumber = quadrantName.Atoi();
    Int_t stationNumber = stationName.Atoi();

    id = 1000 * stationNumber + 100 * quadNumber + sensorNumber;

   // LOG(DEBUG) << "We are on Station: " << stationNumber << " in Quadrant: " << quadNumber << " and Sensor: " << sensorNumber << FairLogger::endl;
   // LOG(DEBUG) << "This ID is: " << id << FairLogger::endl;


    return id;
}
//--------------------------------------------------------------------------


//--------------------------------------------------------------------------
void CbmMvdGeoHandler::Fill()
{
if(fIsSimulation)
	FillStationMap();
else
	FillDetector();
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
void CbmMvdGeoHandler::FillDetector()
{
if(fGeoTyp == Default)
	LOG(ERROR) << "Using old Geometry files within the new Digitizer is not supported, "
		   << "please use CbmMvdDigitizeL if you want to use this Geometry" << FairLogger::endl;

if(fGeoTyp == FourStation || fGeoTyp == FourStationShift)
{

if(!fDetector)
	LOG(FATAL) <<  "GeometryHandler couldn't find a valid Detector"
		   << FairLogger::endl;

Int_t iStation = 0;
  	for(Int_t StatNr = 0; StatNr < 4; StatNr++)
	{
                   fStationNumber = StatNr;
	if(StatNr == 0 && fGeoTyp == FourStation)
		fStationName = "/MVDo0ohoFPCoHSoS_1";
	else
		fStationName = Form("/MVDo%iohoFPCoextoHSoS_1",StatNr);
		for(Int_t QuadNr = 0; QuadNr < 4; QuadNr++)
	    	{
		if(StatNr == 0  && fGeoTyp == 4)
	        fQuadrantName = Form("/St0Q%iohoFPC_1", QuadNr);
		else
		fQuadrantName = Form("/St%iQ%iohoFPCoext_1",StatNr, QuadNr);
	     		for(Int_t Layer = 0; Layer < 2; Layer++)
		 	 {
		  	
		     	  for(Int_t SensNr = 0; SensNr < 50; SensNr++)
			      {
			      fSensorHolding = Form("/MVD-S%i-Q%i-L%i-C%02i-P0oPartAss_1",  StatNr, QuadNr, Layer, SensNr);
			      fSensorName = Form("MVD-S%i-Q%i-L%i-C%02i-P0", StatNr, QuadNr, Layer, SensNr);
			      fVolId = gGeoManager->GetUID(fSensorName);
			      if(fVolId > -1)
			      for(Int_t SegmentNr = 0; SegmentNr < 50; SegmentNr++)
			         {
			          fSectorName = Form("/S%iQ%iS%i_1", StatNr, QuadNr, SegmentNr);
			         fnodeName = fMother + fDetectorName + fStationName + fQuadrantName + fSectorName + fSensorHolding + "/" + fSensorName + "_1";
			        LOG(DEBUG) << "looking for " << fnodeName << FairLogger::endl;
			        Bool_t nodeFound = gGeoManager->CheckPath(fnodeName.Data());
			               if(nodeFound)
			        	{
			        	fDetector->AddSensor(fSensorName, fSensorName, fnodeName, new CbmMvdMimosa26AHR, iStation, fVolId, 0.0, StatNr);
			        	iStation++;
			        	FillParameter();
			        	}
			        	}
				}
			}
		}
	}
}

if(fGeoTyp == scripted)
{
fSensorName = "sensorActive";
if(!fDetector)
	LOG(FATAL) <<  "GeometryHandler couldn't find a valid Detector"
		   << FairLogger::endl;

Int_t iStation = 0;
  	for(Int_t StatNr = 0; StatNr < 4; StatNr++)
	{
        fStationNumber = StatNr;
        fStationName = Form("/station_S%d_1",StatNr);
		for(Int_t QuadNr = 0; QuadNr < 4; QuadNr++)
	           {
	           fQuadrantName = Form("/quadrant_S%d_%d",StatNr, QuadNr);
	           for(Int_t SensNr = 0; SensNr < 50; SensNr++)
		      {
		      fSensorHolding = Form("/sensor_%d", SensNr);
                      fnodeName = fMother + fDetectorName + fStationName + fQuadrantName + fSensorHolding + "/" + fSensorName + "_1";
		      LOG(DEBUG) << "looking for " << fnodeName << FairLogger::endl;
                      Bool_t nodeFound = gGeoManager->CheckPath(fnodeName.Data());
		      if(nodeFound)
		        {
			gGeoManager->cd(fnodeName);
		        fVolId = GetIDfromPath(fnodeName);
		      	fDetector->AddSensor(fSensorName, fSensorName, fnodeName, new CbmMvdMimosa26AHR, iStation, fVolId, 0.0, StatNr);
		       	iStation++;
			FillParameter();
                         LOG(DEBUG) << "found " << fSensorHolding + "/" + fSensorName <<  " number: " << fVolId << "  and added to MVD Detector" << FairLogger::endl;
			}
                      else break;

		      }
		   }
	}
}





else if(fGeoTyp == MiniCbm)
{

if(!fDetector)
	LOG(FATAL) <<  "GeometryHandler couldn't find a valid mCBM Detector"
		   << FairLogger::endl;

Int_t iStation = 0;
  	for(Int_t StatNr = 0; StatNr < 2; StatNr++)
	{
                   fStationNumber = StatNr;
	fStationName = Form("/MVDomCBMoS%i_1",StatNr);
                     
	     		for(Int_t Layer = 0; Layer < 2; Layer++)
		 	 {
		     		 for(Int_t SensNr = 0; SensNr < 50; SensNr++)
				 {
				fQuadrantName = Form("/MVD-S%i-Q0-L%i-C%02i_1",StatNr, Layer, SensNr);
				fSensorHolding = Form("/MVD-S%i-Q0-L%i-C%02i-P0oPartAss_1",  StatNr, Layer, SensNr);
			    	fSensorName = Form("MVD-S%i-Q0-L%i-C%02i-P0", StatNr, Layer, SensNr);
			        fVolId = gGeoManager->GetUID(fSensorName);
			        if(fVolId > -1)
			          {
			        fnodeName = fMother + fDetectorName + fStationName + fQuadrantName + fSensorHolding + "/" + fSensorName + "_1";
			        Bool_t nodeFound = gGeoManager->CheckPath(fnodeName.Data());
			                if(nodeFound)
			        	{
				        fDetector->AddSensor(fSensorName, fSensorName, fnodeName, new CbmMvdMimosa26AHR, iStation, fVolId, 0.0, StatNr);
			        	iStation++;
			        	FillParameter();
			                }
			        }
				}
			}
		
	}
}
else
{
LOG(ERROR) << "Tried to load an unsupported MVD Geometry" << FairLogger::endl;
}
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
void CbmMvdGeoHandler::FillParameter()
{


if (fGeoPathHash != fnodeName.Hash()) {
      NavigateTo(fnodeName);
}
fStationPar->SetZPosition(fStationNumber, fGlobal[2]);
fStationPar->SetWidth(fStationNumber, fWidth);
fStationPar->SetHeight(fStationNumber, fHeight);
fStationPar->SetThickness(fStationNumber, fGlobal[2]);
fStationPar->SetXRes(fStationNumber, fXres);
fStationPar->SetYRes(fStationNumber, fYres);
fStationPar->SetRadLength(fStationNumber, fRadLength);
if(fBeamheight > 0)fStationPar->SetBeamHeight(fStationNumber, fBeamheight);
if(fBeamwidth > 0)fStationPar->SetBeamWidth(fStationNumber, fBeamwidth);
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
void CbmMvdGeoHandler::PrintGeoParameter()
{
fStationPar->Print();
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
void CbmMvdGeoHandler::FillStationMap()
{
if(fGeoTyp == FourStation || fGeoTyp == FourStationShift)
{
Int_t iStation = 0;
  	for(Int_t StatNr = 0; StatNr < 4; StatNr++)
      	{
	if(StatNr == 0 && fGeoTyp == FourStation)
		fStationName = "/MVDo0ohoFPCoHSoS_1";
	else
		fStationName = Form("/MVDo%iohoFPCoextoHSoS_1",StatNr);
		for(Int_t QuadNr = 0; QuadNr < 4; QuadNr++)
	    	{
		if(StatNr == 0  && fGeoTyp == 4)
	        fQuadrantName = Form("/St0Q%iohoFPC_1", QuadNr);
		else
		fQuadrantName = Form("/St%iQ%iohoFPCoext_1",StatNr, QuadNr);
	     		for(Int_t Layer = 0; Layer < 2; Layer++)
		 	 {
		  	
		     		 for(Int_t SensNr = 0; SensNr < 50; SensNr++)
			  	{
				fSensorHolding = Form("/MVD-S%i-Q%i-L%i-C%02i-P0oPartAss_1",  StatNr, QuadNr, Layer, SensNr);
			    	fSensorName = Form("MVD-S%i-Q%i-L%i-C%02i-P0", StatNr, QuadNr, Layer, SensNr);
			        fVolId = gGeoManager->GetUID(fSensorName);
			        if(fVolId > -1)
			        for(Int_t SegmentNr = 0; SegmentNr < 50; SegmentNr++)
			       {
			        fSectorName = Form("/S%iQ%iS%i_1", StatNr, QuadNr, SegmentNr);
			        fnodeName = fMother + fDetectorName + fStationName + fQuadrantName + fSectorName + fSensorHolding + "/" + fSensorName + "_1";
			        LOG(DEBUG) << "looking for " << fnodeName << FairLogger::endl;
			        Bool_t nodeFound = gGeoManager->CheckPath(fnodeName.Data());
			                if(nodeFound)
			        	{
			        	fStationMap[fVolId] = iStation;
			        	iStation++;
					
			       	}
			        	}
				}
			}
		}

}
}

else if(fGeoTyp == scripted)
{
fSensorName = "sensorActive";
Int_t iStation = 0;
  	for(Int_t StatNr = 0; StatNr < 4; StatNr++)
      	{
        fStationName = Form("/station_S%d_1",StatNr);
		for(Int_t QuadNr = 0; QuadNr < 4; QuadNr++)
	    	{
		fQuadrantName = Form("/quadrant_S%d_%d",StatNr, QuadNr);
	           for(Int_t SensNr = 0; SensNr < 50; SensNr++)
		      {
			  fSensorHolding = Form("/sensor_%d", SensNr);
                      fnodeName = fMother + fDetectorName + fStationName + fQuadrantName + fSensorHolding + "/" + fSensorName + "_1";
		      LOG(DEBUG) << "looking for " << fnodeName << FairLogger::endl;
                      Bool_t nodeFound = gGeoManager->CheckPath(fnodeName.Data());
		      if(nodeFound)
		        {
			gGeoManager->cd(fnodeName);
                        TGeoNode* node = gGeoManager->GetCurrentNode();
			fVolId = GetIDfromPath(fnodeName);
			LOG(DEBUG) << "found " << fnodeName << " number: " << iStation << " ID: " << fVolId << " and added to station map" << FairLogger::endl;
		          fStationMap[fVolId] = iStation;
			  iStation++;
			  LOG(DEBUG) << "Map now size: " << fStationMap.size() << FairLogger::endl;
		          }
                      else break;

		   }
		}
	}
}

else if(fGeoTyp == Default)
{
  Int_t iStation =  1;
  Int_t volId    = -1;
  do {
    TString volName = Form("mvdstation%02i", iStation);
    volId = gGeoManager->GetUID(volName);
    if (volId > -1 ) {
      fStationMap[volId] = iStation;
      LOG(INFO) << GetName() << "::ConstructAsciiGeometry: "
           << "Station No. " << iStation << ", volume ID " << volId
           << ", volume name " << volName << FairLogger::endl;
      iStation++;
    }
  } while ( volId > -1 );
}

else if(fGeoTyp == MiniCbm)
{
Int_t iStation = 0;
  	for(Int_t StatNr = 0; StatNr < 2; StatNr++)
      	{
	fStationName = Form("/MVDomCBMoS%i_1",StatNr);
                     
	     		for(Int_t Layer = 0; Layer < 2; Layer++)
		 	 {
		     		 for(Int_t SensNr = 0; SensNr < 50; SensNr++)
			  	{
                                fQuadrantName = Form("/MVD-S%i-Q0-L%i-C%02i_1",StatNr, Layer, SensNr);
				fSensorHolding = Form("/MVD-S%i-Q0-L%i-C%02i-P0oPartAss_1",  StatNr, Layer, SensNr);
			    	fSensorName = Form("MVD-S%i-Q0-L%i-C%02i-P0", StatNr, Layer, SensNr);
				//cout << endl << "try to find: " << fSensorName << endl;
			        fVolId = gGeoManager->GetUID(fSensorName);
			        if(fVolId > -1)
			        {
			       fnodeName = fMother + fDetectorName + fStationName + fQuadrantName + fSensorHolding + "/" + fSensorName + "_1";
				//cout << endl << "sensorfound check for node " << fnodeName << endl;
			        Bool_t nodeFound = gGeoManager->CheckPath(fnodeName.Data());
			                if(nodeFound)
			        	{
			        	//cout << endl << "node found " << fnodeName << endl;
			        	fStationMap[fVolId] = iStation;
			        	iStation++;

			                }
			        }
				}
			}
		
	}
}


else
	LOG(ERROR) << "You tried to use an unsuported Geometry" << FairLogger::endl;
}
//--------------------------------------------------------------------------

ClassImp(CbmMvdGeoHandler)
