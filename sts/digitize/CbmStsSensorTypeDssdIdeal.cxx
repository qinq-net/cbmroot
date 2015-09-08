/** @file CbmStsSensorTypeDssdIdeal.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 20.05.2014
 **/

#include <iomanip>

#include "TMath.h"

#include "FairLogger.h"

#include "digitize/CbmStsSensorTypeDssdIdeal.h"
#include "setup/CbmStsSensorPoint.h"
#include "CbmStsSensor.h"


using namespace std;


// --- Energy for creation of an electron-hole pair in silicon [GeV]  ------
const double kPairEnergy = 3.57142e-9;



// -----   Constructor   ---------------------------------------------------
CbmStsSensorTypeDssdIdeal::CbmStsSensorTypeDssdIdeal()
    : CbmStsSensorTypeDssd()
{
}
// -------------------------------------------------------------------------



// -----   Process an MC Point  --------------------------------------------
Int_t CbmStsSensorTypeDssdIdeal::ProcessPoint(CbmStsSensorPoint* point,
                                         const CbmStsSensor* sensor) {

  // --- Catch if parameters are not set
  if ( ! fIsSet ) {
    LOG(FATAL) << fName << ": parameters are not set!"
               << FairLogger::endl;
    return -1;
  }

  // --- Debug
  LOG(DEBUG3) << ToString() << FairLogger::endl;
  LOG(DEBUG3) << GetName() << ": Processing point " << point->ToString()
  		        << FairLogger::endl;

  // --- Check for being in sensitive area
  // --- Note: No charge is produced if either entry or exit point
  // --- (or both) are outside the active area. This is not an exact
  // --- description since the track may enter the sensitive area
  // --- if not perpendicular to the sensor plane. The simplification
  // --- was chosen to avoid complexity. :-)
  if ( TMath::Abs(point->GetX1()) > fDx/2. ||
	   TMath::Abs(point->GetY1()) > fDy/2. ||
	   TMath::Abs(point->GetX2()) > fDx/2. ||
	   TMath::Abs(point->GetY2()) > fDy/2. ) {
  	LOG(DEBUG4) << GetName() << ": not in sensitive area" << FairLogger::endl;
  	return 0;
  }

  // --- Number of created charge signals (coded front/back side)
  Int_t nSignals = 0;

  // --- Produce charge on front and back side
  nSignals += 1000 * ProduceCharge(point, 0, sensor); // front
  nSignals +=        ProduceCharge(point, 1, sensor); // back


  return nSignals;
}
// -------------------------------------------------------------------------


// -----   Produce charge on the strips   ----------------------------------
Int_t CbmStsSensorTypeDssdIdeal::ProduceCharge(CbmStsSensorPoint* point,
																					 		 Int_t side,
                                               const CbmStsSensor* sensor)
																						 	 {

  // --- Protect against being called without parameters being set
  if ( ! fIsSet ) LOG(FATAL) << "Parameters of sensor " << fName
                             << " are not set!" << FairLogger::endl;

  // This implementation projects all charge on one strip on front and
  // back side, which correspond to the horizontal projections of
  // the mid-point of the trajectory in the sensor.

  // Check side
  if ( side < 0 || side > 1) {
    LOG(ERROR) << "Illegal side qualifier " << side << FairLogger::endl;
    return -1;
 }

  // Calculate trajectory mid-point
  Double_t xP = 0.5 * ( point->GetX1() + point->GetX2() );
  Double_t yP = 0.5 * ( point->GetY1() + point->GetY2() );

  // Check for being in sensitive area
  if ( TMath::Abs(xP) > fDx/2. || TMath::Abs(yP) > fDy/2. )
  	return 0;

  // Get strip number
	Int_t iStrip = GetStripNumber(xP, yP, side);
	if ( iStrip < 0 || iStrip >= fNofStrips[side] ) {
		LOG(ERROR) << GetName() << ": Illegal strip number " << iStrip
				       << FairLogger::endl;
		return -1;
	}

  // Total produced charge
  Double_t qtot = point->GetELoss() / kPairEnergy;

  // Register charge to module, if inside active area.
  if ( iStrip >= 0 )
  	RegisterCharge(sensor, side, iStrip, qtot, point->GetTime() );

  return 1;
}
// -------------------------------------------------------------------------


ClassImp(CbmStsSensorTypeDssdIdeal)
