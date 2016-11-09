/** @file CbmStsSensorTypeDssdOrtho.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 02.010.2015
 **/

#include "CbmStsSensorTypeDssdOrtho.h"

#include <cassert>
#include <iomanip>
#include <sstream>

#include "TArrayD.h"
#include "TMath.h"

#include "FairLogger.h"

#include "CbmStsDigitize.h"
#include "CbmStsModule.h"
#include "CbmStsPhysics.h"
#include "CbmStsSensor.h"
#include "CbmStsSensorPoint.h"


using std::fixed;
using std::setprecision;
using std::stringstream;



// -----   Constructor   ---------------------------------------------------
CbmStsSensorTypeDssdOrtho::CbmStsSensorTypeDssdOrtho()
    : CbmStsSensorTypeDssd()
{
	fStereo[0] =  0.;  // vertical strips on the front side
	fStereo[1] = 90.;  // horizontal strips on the back side
}
// -------------------------------------------------------------------------




// -----   Diffusion   -----------------------------------------------------
void CbmStsSensorTypeDssdOrtho::Diffusion(Double_t x, Double_t y,
		                                 Double_t sigma, Int_t side,
		                                 Double_t& fracL, Double_t& fracC,
		                                 Double_t& fracR) {

	// Check side qualifier
	assert( side == 0 || side == 1);

	// Coordinate at the readout edge, counted from the left / lower corner.
	Double_t xRo = 0.;
	if ( side == 0 )    // front side, vertical strips
		xRo = x + fDx / 2.;
	else                // back side, horizontal strips
		xRo = y + fDy / 2.;

	// Centre strip number
	Int_t iStrip = TMath::FloorNint(xRo / fPitch[side]);

	// Strip boundaries at the readout edge
	Double_t xLeftRo  = Double_t(iStrip) * fPitch[side];
	Double_t xRightRo = xLeftRo + fPitch[side];

	// Distance from strip boundaries across the strip
	Double_t dLeft  = ( xRo - xLeftRo );
	Double_t dRight = ( xRightRo - xRo );

	// Charge fractions
	// The value 0.707107 is 1/sqrt(2)
	fracL = 0.;
	if ( dLeft < 3. * sigma )
		fracL = 0.5 * ( 1. - TMath::Erf( 0.707107 * dLeft  / sigma) );
	fracR = 0.;
	if ( dRight < 3. * sigma )
		fracR = 0.5 * ( 1. - TMath::Erf( 0.707107 * dRight / sigma) );
	fracC = 1. - fracL - fracR;

	LOG(DEBUG4) << GetName() << ": Distances to next strip " << dLeft << " / "
			        << dRight << ", charge fractions " << fracL << " / " << fracC
			        << " / " << fracR << FairLogger::endl;
}
// -------------------------------------------------------------------------



// -----   Get channel number in module   ----------------------------------
Int_t CbmStsSensorTypeDssdOrtho::GetModuleChannel(Int_t strip, Int_t side,
                                             Int_t /*sensorId*/) const {

  // --- Check side
	assert ( side == 0 || side == 1 );

	// --- Prescription for connection
	Int_t channel = strip;
  if ( side ) channel += fNofStrips[0];

  return channel;
}
// -------------------------------------------------------------------------




// -----   Get strip number from coordinates   -----------------------------
Int_t CbmStsSensorTypeDssdOrtho::GetStripNumber(Double_t x, Double_t y,
		                                       Int_t side) const {

	// Cave: This implementation assumes that the centre of the sensor volume
	// is also the centre of the active area, i.e. that the inactive borders
	// (guard ring) are symmetric both and x and y (not necessarily the same
	// in y and y).

	// Check side
	assert ( side == 0 || side == 1);

	// Check whether in active area (should have been caught before)
	if ( TMath::Abs(x) > fDx / 2. ) {
		LOG(ERROR) << GetName() << ": Outside active area : x = "
				       << x << " cm"<< FairLogger::endl;
    return -1;
	}
	if ( TMath::Abs(y) > fDy / 2. ) {
		LOG(ERROR) << GetName() << ": Outside active area : y = "
			         << y << " cm"<< FairLogger::endl;
    return -1;
	}

  // Calculate distance from corner of readout edge
  // Note: the coordinates are given w.r.t. the centre of the volume.
  Double_t dist = 0.;
  if ( side == 0 ) dist = x + 0.5 * fDx;
  else             dist = y + 0.5 * fDy;

  // Calculate corresponding strip number
  Int_t iStrip = TMath::FloorNint( dist / fPitch[side] );

  return iStrip;
}
// -------------------------------------------------------------------------



// -----   Get strip and side from channel number   ------------------------
void CbmStsSensorTypeDssdOrtho::GetStrip(Int_t channel, Int_t /*sensorId*/,
                                    Int_t& strip, Int_t& side) {

  Int_t stripNr = -1;
  Int_t sideNr  = -1;


  // --- Determine front or back side
  if ( channel < fNofStrips[0] ) {          // front side
    sideNr = 0;
    stripNr = channel;
  }
  else {
    sideNr = 1;
    stripNr = channel - fNofStrips[0];      // back side
  }

  side = sideNr;
  strip = stripNr;

 return;
}
// -------------------------------------------------------------------------




// -----  Intersect two clusters (front / back)   --------------------------
Int_t CbmStsSensorTypeDssdOrtho::IntersectClusters(CbmStsCluster* clusterF,
		                                          CbmStsCluster* clusterB,
		                                          CbmStsSensor* sensor) {

	// --- Check pointer validity
	assert( clusterF );
	assert( clusterB );
	assert( sensor );

	// --- Calculate cluster centre position on readout edge
	Int_t side  = -1;
	Double_t xF = -1.;
	Double_t yB = -1.;
	GetClusterPosition(clusterF->GetPosition(), sensor, xF, side);
	if ( side != 0 )
		LOG(FATAL) << GetName() << ": Inconsistent side qualifier " << side
		           << " for front side cluster! " << FairLogger::endl;
	Double_t varX = clusterF->GetPositionError() * clusterF->GetPositionError();
	GetClusterPosition(clusterB->GetPosition(), sensor, yB, side);
	Double_t varY = clusterB->GetPositionError() * clusterB->GetPositionError();
	Double_t varXY = 0.;
	if ( side != 1 )
		LOG(FATAL) << GetName() << ": Inconsistent side qualifier " << side
		           << " for back side cluster! " << FairLogger::endl;

	// --- Should be inside active area
	if ( ! ( xF >= 0. || xF <= fDx) ) return 0;
	if ( ! ( yB >= 0. || yB <= fDy) ) return 0;

	// --- Transform into sensor system with origin at sensor centre
	xF -= 0.5 * fDx;
	yB -= 0.5 * fDy;

	// --- Send hit information to sensor
	sensor->CreateHit(xF, yB, varX, varY, varXY, clusterF, clusterB);

	return 1;
}
// -------------------------------------------------------------------------



// -----   Set the parameters   --------------------------------------------
void CbmStsSensorTypeDssdOrtho::SetParameters(Double_t dx, Double_t dy,
                                         Double_t dz, Int_t nStripsF,
                                         Int_t nStripsB, Double_t /*stereoF*/,
                                         Double_t /*stereoB*/) {

  // --- Set members
  fDx           = dx;
  fDy           = dy;
  fDz           = dz;
  fNofStrips[0] = nStripsF;
  fNofStrips[1] = nStripsB;
  // The arguments stereoF and stereoB are neglected, since this type
  // has vertical strips on the front and horizontal strips on the back side.

  // --- Calculate parameters for front and back
  for (Int_t side = 0; side < 2; side++)
    fPitch[side] = fDx / Double_t(fNofStrips[side]);

  fStripCharge[0].Set(fNofStrips[0]);
  fStripCharge[1].Set(fNofStrips[1]);

  // --- Flag parameters to be set if test is ok
  fIsSet = SelfTest();

}
// -------------------------------------------------------------------------




ClassImp(CbmStsSensorTypeDssdOrtho)
