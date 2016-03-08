// -------------------------------------------------------------------------
// -----                     CbmMvdStationPar source file              -----
// -----                  Created 28/10/14  by P.Sitzmann              -----
// -------------------------------------------------------------------------

#include "CbmMvdStationPar.h"

#include <iostream>
#include <math.h>

using std::cout;
using std::endl;



// -----   Default constructor   -------------------------------------------
CbmMvdStationPar::CbmMvdStationPar() 
  : TNamed(),
    fZPositions(), 
    fThicknesses(), 
    fHeights(), 
    fWidths(), 
    fXResolutions(), 
    fYResolutions(), 
    fRadiationLength(), 
    fBeamHeights(), 
    fBeamWidths(),
    fStationCount()
{
}
// -------------------------------------------------------------------------

// -----   Destructor   ----------------------------------------------------
CbmMvdStationPar::~CbmMvdStationPar() {
}
// -------------------------------------------------------------------------

// -----   Public method Print   -------------------------------------------
void CbmMvdStationPar::Print(Option_t* /*opt*/) const {

cout 	<< endl << "--I--" << GetName() << "--I--" << endl
	<< endl << "Initialized ParameterFile with " << fZPositions.size() << " Stations" << endl
	<< endl << "-------------------------------------------------------------------------" << endl;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Bool_t CbmMvdStationPar::Init()
{
for (Int_t i = 0; i < fStationCount; i++)
	{ fZPositions[i]=0.; 
    	  fThicknesses[i]=0.; 
          fHeights[i]=0.;
          fWidths[i]=0.;
          fXResolutions[i]=0.;
          fYResolutions[i]=0.;
          fRadiationLength[i]=0.; 
          fBeamHeights[i]=0.; 
          fBeamWidths[i]=0.; 
	}
	return 1;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Double_t CbmMvdStationPar::GetZPosition(Int_t stationNumber)
{
if(stationNumber <= fStationCount)
	{
	return fZPositions[stationNumber];
	}
else
	{
	cout <<  "Station number out of Range " << endl;
	}
return 0.;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Double_t CbmMvdStationPar::GetThickness(Int_t stationNumber)
{
if(stationNumber <= fStationCount)
	{
	return fThicknesses[stationNumber];
	}
else
	{
	cout <<  "Station number out of Range " << endl;
	}
return 0.;
}

// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Double_t CbmMvdStationPar::GetHeight(Int_t stationNumber)
{
if(stationNumber <= fStationCount)
	{
	return fHeights[stationNumber];
	}
else
	{
	cout <<  "Station number out of Range " << endl;
	}
return 0.;
}

// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Double_t CbmMvdStationPar::GetWidth(Int_t stationNumber)
{
if(stationNumber <= fStationCount)
	{
	return fWidths[stationNumber];
	
	}
else
	{
	cout <<  "Station number out of Range " << endl;
	}
return 0.;
}

// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Double_t CbmMvdStationPar::GetXRes(Int_t stationNumber)
{
if(stationNumber <= fStationCount)
	{
	return fXResolutions[stationNumber]; 
	}
else
	{
	cout <<  "Station number out of Range " << endl;
	}
return 0.;
}

// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Double_t CbmMvdStationPar::GetYRes(Int_t stationNumber)
{
if(stationNumber <= fStationCount)
	{
	return fYResolutions[stationNumber];
	}
else
	{
	cout <<  "Station number out of Range " << endl;
	}
return 0.;
} 

// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Double_t CbmMvdStationPar::GetRadLength(Int_t stationNumber)
{
if(stationNumber <= fStationCount)
	{
	return fRadiationLength[stationNumber];
	}
else
	{
	cout <<  "Station number out of Range " << endl;
	}
return 0.;
}

// -------------------------------------------------------------------------

// -------------------------------------------------------------------------  
Double_t CbmMvdStationPar::GetBeamHeight(Int_t stationNumber)
{
if(stationNumber <= fStationCount)
	{
	return fBeamHeights[stationNumber];
	}
else
	{
	cout <<  "Station number out of Range " << endl;
	}
return 0.;
} 

// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Double_t CbmMvdStationPar::GetBeamWidth(Int_t stationNumber)
{
if(stationNumber <= fStationCount)
	{
	return fBeamWidths[stationNumber];
	}
else
	{
	cout <<  "Station number out of Range " << endl;
	}
return 0.;
}

// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
void CbmMvdStationPar::SetZPosition(Int_t stationNumber, Double_t z)
{
if(fZPositions[stationNumber]!=0)
	{
	fZPositions[stationNumber] = Int_t(((fZPositions[stationNumber] + z)/2)+0.5);
	}
else
	{
	fZPositions[stationNumber] = z;
	}

}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMvdStationPar::SetThickness(Int_t stationNumber, Double_t thickness)
{
Float_t Sensorthickness = fabs(thickness-fZPositions[stationNumber]);
if(Sensorthickness > fThicknesses[stationNumber])
	fThicknesses[stationNumber]=Sensorthickness;
}  
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMvdStationPar::SetHeight(Int_t stationNumber, Double_t height)
{
if(fHeights[stationNumber] < height)
	{
	fHeights[stationNumber] = height;
	}

}  
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMvdStationPar::SetWidth(Int_t stationNumber, Double_t width)
{
if(fWidths[stationNumber] < width)
	{
	fWidths[stationNumber] = width;
	}
}  
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMvdStationPar::SetXRes(Int_t stationNumber, Double_t xres)
{
	fXResolutions[stationNumber] = xres;
}  
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMvdStationPar::SetYRes(Int_t stationNumber, Double_t yres)
{
fYResolutions[stationNumber] = yres;
}  
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMvdStationPar::SetRadLength(Int_t stationNumber, Double_t length)
{
fRadiationLength[stationNumber] = length;
}  
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMvdStationPar::SetBeamHeight(Int_t stationNumber, Double_t beamheight)
{
if(fBeamHeights[stationNumber] != 0)
	{
	if(fBeamHeights[stationNumber] > beamheight)
		{
		fBeamHeights[stationNumber] = beamheight;
		}
	}
else
	{
	fBeamHeights[stationNumber] = beamheight;
	}
} 
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMvdStationPar::SetBeamWidth(Int_t stationNumber, Double_t beamwidth)
{
if(fBeamWidths[stationNumber] != 0)
	{
	if(fBeamWidths[stationNumber] > beamwidth)
		{
		fBeamWidths[stationNumber] = beamwidth;
		}
	}
else
	{
	fBeamWidths[stationNumber] = beamwidth;
	}
}  
// -------------------------------------------------------------------------



ClassImp(CbmMvdStationPar)
