// -------------------------------------------------------------------------
// -----                     CbmMvdStationPar source file              -----
// -----                  Created 28/10/14  by P.Sitzmann              -----
// -------------------------------------------------------------------------

#include "CbmMvdStationPar.h"

#include <iostream>


using std::cout;
using std::endl;



// -----   Default constructor   -------------------------------------------
CbmMvdStationPar::CbmMvdStationPar() 
  : TNamed(),
    fStationCount(0),
    fZPositions(), 
    fThicknesses(), 
    fHeights(), 
    fWidths(), 
    fXResolutions(), 
    fYResolutions(), 
    fRadiationLength(), 
    fBeamHeights(), 
    fBeamWidths()
{
}
// -------------------------------------------------------------------------

// -----   Standart constructor   -------------------------------------------
CbmMvdStationPar::CbmMvdStationPar(Int_t count) 
  : TNamed(),
    fStationCount(count),
    fZPositions(), 
    fThicknesses(), 
    fHeights(), 
    fWidths(), 
    fXResolutions(), 
    fYResolutions(), 
    fRadiationLength(), 
    fBeamHeights(), 
    fBeamWidths()
{
for (Int_t i = 0; i <= count; i++)
	{ fZPositions[count]=0.; 
    	  fThicknesses[count]=0.; 
          fHeights[count]=0.;
          fWidths[count]=0.;
          fXResolutions[count]=0.;
          fYResolutions[count]=0.;
          fRadiationLength[count]=0.; 
          fBeamHeights[count]=0.; 
          fBeamWidths[count]=0.; 
	}
}
// -------------------------------------------------------------------------


// -----   Destructor   ----------------------------------------------------
CbmMvdStationPar::~CbmMvdStationPar() {
}
// -------------------------------------------------------------------------

// -----   Public method Print   -------------------------------------------
void CbmMvdStationPar::Print(Option_t* opt) const {
  cout << " --- " << GetName() 
       << endl;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Double_t CbmMvdStationPar::GetZPosition(Int_t stationNumber)
{
if(stationNumber <= fStationCount)
	{
	Double_t value;
	value = fZPositions[stationNumber];
	return value;
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
	Double_t value;
	value = fThicknesses[stationNumber];
	return value;
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
	Double_t value;
	value = fHeights[stationNumber];
	return value;
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
	Double_t value;
	value = fWidths[stationNumber];
	return value;
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
	Double_t value;
	value = fXResolutions[stationNumber];
	return value;
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
	Double_t value;
	value = fYResolutions[stationNumber];
	return value;
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
	Double_t value;
	value = fRadiationLength[stationNumber];
	return value;
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
	Double_t value;
	value = fBeamHeights[stationNumber];
	return value;
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
	Double_t value;
	value = fBeamWidths[stationNumber];
	return value;
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
	fZPositions[stationNumber] = (fZPositions[stationNumber] + z)/2;
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
fThicknesses[stationNumber] = thickness;

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
