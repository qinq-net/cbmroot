/** @file CbmStsStation_new.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 04.03.2015
 **/

#include "TGeoMatrix.h"
#include "CbmStsElement.h"
#include "CbmStsSensor.h"
#include "CbmStsStation_new.h"


// -----   Default constructor   -------------------------------------------
CbmStsStation_new::CbmStsStation_new()
	: CbmStsElement(), fZ(0.) {
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmStsStation_new::CbmStsStation_new(const char* name, const char* title,
		                                 TGeoPhysicalNode* node)
	: CbmStsElement(name, title, kStsStation, node),
	  fZ(0.) {
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStsStation_new::~CbmStsStation_new() {
}
// -------------------------------------------------------------------------



// -----   Initialise station parameters   ---------------------------------
void CbmStsStation_new::Init() {
	InitPosition();
}
// -------------------------------------------------------------------------



// -----   Initialise the station z position   -----------------------------
void CbmStsStation_new::InitPosition() {

	Int_t nSensors = 0;         // sensor counter
	Double_t zMin  =  999999.;  // sensor z minimum
	Double_t zMax  = -999999.;  // sensor z maximum

	// --- Loop over ladders
  for (Int_t iLad = 0; iLad < GetNofDaughters(); iLad++) {
  	CbmStsElement* ladd = GetDaughter(iLad);

  	// --- Loop over half-ladders
  	for (Int_t iHla = 0; iHla < ladd->GetNofDaughters(); iHla++) {
  		CbmStsElement* hlad = ladd->GetDaughter(iHla);

  		// --- Loop over modules
  		for (Int_t iMod = 0; iMod < hlad->GetNofDaughters(); iMod++) {
  			CbmStsElement* modu = hlad->GetDaughter(iMod);

  			// --- Loop over sensors
  			for (Int_t iSen = 0; iSen < modu->GetNofDaughters(); iSen++) {
  				CbmStsSensor* sensor =
  						dynamic_cast<CbmStsSensor*>(modu->GetDaughter(iSen));

  				// Get sensor z position
  		    TGeoPhysicalNode* sensorNode = sensor->GetPnode();
  		    // --- Transform entry coordinates into local C.S.
  		    Double_t local[3] = {0., 0., 0.};  // sensor centre, local c.s.
   		    Double_t global[3];                // sensor centre, global c.s.
    		  sensorNode->GetMatrix()->LocalToMaster(local, global);
    		  nSensors++;

    		  // Update zMin and zMax
    		  if ( ! nSensors ) {  // first sensor
    		  	zMin = global[2];
    		  	zMax = global[2];
    		  }
    		  else {
    		  	zMin = TMath::Min(zMin, global[2]);
    		  	zMax = TMath::Max(zMax, global[2]);
    		  }

  			} // # sensors
  		} // # modules
  	} // # half-ladders
  } // # ladders

  fZ = 0.5 * (zMin + zMax);
  LOG(DEBUG) << GetName() << ": " << nSensors << " sensors from z = " << zMin
  		      << " cm to z = " << zMax << "cm" << FairLogger::endl;

}
// -------------------------------------------------------------------------


ClassImp(CbmStsStation_new)

