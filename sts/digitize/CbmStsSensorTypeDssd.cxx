/** @file CbmStsSensorTypeDssd.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 02.05.2013
 **/

#include "CbmStsSensorTypeDssd.h"

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
#include "CbmMatch.h"


using std::fixed;
using std::setprecision;
using std::stringstream;



// -----   Constructor   ---------------------------------------------------
CbmStsSensorTypeDssd::CbmStsSensorTypeDssd()
    : CbmStsSensorType(), 
      fDx(-1.), fDy(-1.), fDz(-1.),
      fNofStrips(), fStereo(), fIsSet(kFALSE), fOld(kFALSE),
      fPitch(), fTanStereo(), fCosStereo(), fStripShift(),
      fStripCharge(),
      fPhysics(NULL),
      fHitFinderModel(1)
{
	fPhysics = CbmStsPhysics::Instance();
}
// -------------------------------------------------------------------------



// -----   Cross talk   ----------------------------------------------------
void CbmStsSensorTypeDssd::CrossTalk(Double_t ctcoeff) {

	for (Int_t side = 0; side < 2; side++) {  // front and back side
		Int_t nStrips = fNofStrips[side];

		// First strip
		Double_t qLeft    = 0.;
		Double_t qCurrent = fStripCharge[side][0];
		fStripCharge[side][0] =
				(1. - ctcoeff ) * qCurrent + ctcoeff * fStripCharge[side][1];

		// Strips 1 to n-2
		for (Int_t strip = 1; strip < nStrips - 1; strip++) {
			qLeft    = qCurrent;
			qCurrent = fStripCharge[side][strip];
			fStripCharge[side][strip] =
					ctcoeff * ( qLeft + fStripCharge[side][strip+1] ) +
					( 1. - 2. * ctcoeff ) * qCurrent;
		} //# strips

		// Last strip
		qLeft = qCurrent;
		qCurrent = fStripCharge[side][nStrips-1];
		fStripCharge[side][nStrips-1] =
				ctcoeff * qLeft + ( 1. - ctcoeff ) * qCurrent;

	} //# front and back side

}
// -------------------------------------------------------------------------



// -----   Diffusion   -----------------------------------------------------
void CbmStsSensorTypeDssd::Diffusion(Double_t x, Double_t y,
		                                 Double_t sigma, Int_t side,
		                                 Double_t& fracL, Double_t& fracC,
		                                 Double_t& fracR) {

	// Check side qualifier
	assert( side == 0 || side == 1);

	// x coordinate at the readout edge (y = fDy/2 )
	// This x is counted from the left edge.
	Double_t xRo = x + fDx / 2. - ( fDy / 2. - y ) * fTanStereo[side];

	// Centre strip number (w/o cross connection; may be negative or large than
	// the number of strips)
	Int_t iStrip = TMath::FloorNint(xRo / fPitch[side]);

	// Strip boundaries at the readout edge (y = fDy/2)
	Double_t xLeftRo  = Double_t(iStrip) * fPitch[side];
	Double_t xRightRo = xLeftRo + fPitch[side];

	// Distance from strip boundaries across the strip
	Double_t dLeft  = ( xRo - xLeftRo )  * fCosStereo[side];
	Double_t dRight = ( xRightRo - xRo ) * fCosStereo[side];

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



// -----   Find hits   -----------------------------------------------------
Int_t CbmStsSensorTypeDssd::FindHits(vector<CbmStsCluster*>& clusters,
		                                 CbmStsSensor* sensor, Double_t dTime) {

	Int_t nHits = 0;
	Int_t nClusters = clusters.size();

	Int_t nClustersF = 0;
	Int_t nClustersB = 0;
	Double_t xF = -1.;   // Cluster position on sensor edge
	Double_t xB = -1.;   // Cluster position on sensor edge

	// --- Sort clusters into front and back side
	vector<Int_t> frontClusters;
	vector<Int_t> backClusters;
	Double_t xCluster = -1.;  // cluster position on read-out edge
	Int_t side  = -1;         // front or back side
	for (Int_t iCluster = 0; iCluster < nClusters; iCluster++) {
		CbmStsCluster* cluster = clusters[iCluster];
		side = GetSide( cluster->GetCentre() );

		if ( side == 0) {
			frontClusters.push_back(iCluster);
			nClustersF++;
		}
		else if ( side == 1 ) {
			backClusters.push_back(iCluster);
			nClustersB++;
		}
		else
			LOG(FATAL) << GetName() << ": Illegal side qualifier "
			           << side << FairLogger::endl;
	}  // Loop over clusters in module
	LOG(DEBUG3) << GetName() << ": " << nClusters << " clusters (front "
			        << frontClusters.size() << ", back " << backClusters.size()
			        << ") " << FairLogger::endl;

	// --- Loop over front and back side clusters
	Double_t xClusterF = -1.;  // Front cluster position at r/o edge
	Double_t xClusterB = -1.;  // Back cluster position at r/o edge
	for (Int_t iClusterF = 0; iClusterF < nClustersF; iClusterF++) {
		CbmStsCluster* clusterF = clusters[frontClusters[iClusterF]];
		for (Int_t iClusterB = 0; iClusterB < nClustersB;	iClusterB++) {
			CbmStsCluster* clusterB = clusters[backClusters[iClusterB]];

			// --- For time-based hit finding ---
			if ( dTime > 0. && fabs(clusterF->GetTime() - clusterB->GetTime()) > dTime ) continue;
			// ----------------------
			
			
			// --- Calculate intersection points
			Int_t nOfHits = IntersectClusters(clusterF, clusterB, sensor);
			LOG(DEBUG4) << GetName() << ": Cluster front " << iClusterF
					        << ", cluster back " << iClusterB
					        << ", intersections " << nOfHits << FairLogger::endl;
			nHits += nOfHits;

		}  // back side clusters

	}  // front side clusters

  LOG(DEBUG3) << GetName() << ": Clusters " << nClusters << " ( "
  		        << nClustersF << " / " << nClustersB << " ), hits: "
  		        << nHits << FairLogger::endl;
	return nHits;
}
// -------------------------------------------------------------------------



// -----   Get cluster position at read-out edge   -------------------------
// Same as GetStrip, but with float instead of channel number
void CbmStsSensorTypeDssd::GetClusterPosition(Double_t centre,
		                                          CbmStsSensor* sensor,
                                              Double_t& xCluster,
                                              Int_t& side) {

	// Take integer channel
	Int_t iChannel = Int_t(centre);
	Double_t xDif = centre - Double_t(iChannel);

	// Calculate corresponding strip on sensor
	Int_t iStrip = -1;
	GetStrip(iChannel, sensor->GetIndex(), iStrip, side);

	// Re-add difference to integer channel. Convert channel to
	// coordinate
	xCluster = (Double_t(iStrip) + xDif + 0.5 ) * fPitch[side];

	// Correct for Lorentz-Shift
	// Simplification: The correction uses only the y component of the
	// magnetic field. The shift is calculated using the mid-plane of the
	// sensor, which is not correct for tracks not traversing the entire
	// sensor thickness (i.e., are created or stopped somewhere in the sensor).
	// However, this is the best one can do in reconstruction.
	//Double_t mobility = (side == 0 ? 0.1650 : 0.0310 );  // in m^2/(Vs)
	//Double_t tanLorentz = mobility * sensor->GetConditions().GetBy();
	//xCluster -= tanLorentz * fDz / 2.;
	if ( CbmStsDigitize::UseLorentzShift() ) xCluster -= sensor->GetConditions().GetMeanLorentzShift(side);

	LOG(DEBUG4) << GetName() << ": Cluster centre " << centre
			        << ", sensor index " << sensor->GetIndex() << ", side "
			        << side << ", cluster position " << xCluster
			        << FairLogger::endl;
	return;
}
// -------------------------------------------------------------------------



// -----   Get channel number in module   ----------------------------------
Int_t CbmStsSensorTypeDssd::GetModuleChannel(Int_t strip, Int_t side,
                                             Int_t sensorId) const {

  // --- Check side
  if ( side < 0 || side > 1 ) {
    LOG(ERROR) << "Illegal side qualifier " << side << FairLogger::endl;
    return -1;
  }

  // --- Account for offset due to stereo angle
  Int_t channel = strip - sensorId * fStripShift[side];

  // --- Account for horizontal cross-connection of strips
  while ( channel < 0 ) channel += fNofStrips[side];
  while ( channel >= fNofStrips[side] ) channel -= fNofStrips[side];

  // --- Account for front or back side
  if ( side ) channel += fNofStrips[0];

  return channel;
}
// -------------------------------------------------------------------------



// -----  Get the strip pitch    -------------------------------------------
Double_t CbmStsSensorTypeDssd::GetPitch(Int_t iSide) const {
	if ( iSide < 0 || iSide > 1 ) {
		LOG(FATAL) << GetName() << ": illegal side identifier!"
				       << FairLogger::endl;
	}
	return fPitch[iSide];
}
// -------------------------------------------------------------------------



// -----  Get the stereo angle    ------------------------------------------
Double_t CbmStsSensorTypeDssd::GetStereoAngle(Int_t iSide) const {
	if ( iSide < 0 || iSide > 1 ) {
		LOG(FATAL) << GetName() << ": illegal side identifier!"
				       << FairLogger::endl;
	}
	return fStereo[iSide];
}
// -------------------------------------------------------------------------



// -----   Get strip number from coordinates   -----------------------------
Int_t CbmStsSensorTypeDssd::GetStripNumber(Double_t x, Double_t y,
		                                       Int_t side) const {

	// Cave: This implementation assumes that the centre of the sensor volume
	// is also the centre of the active area, i.e. that the inactive borders
	// (guard ring) are symmetric both and x and y (not necessarily the same
	// in y and y).

	// Check side
	if ( side < 0 || side > 1) {
		LOG(ERROR) << "Illegal side qualifier " << side << FairLogger::endl;
		return -1;
	}

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

  // Stereo angle and strip pitch
  Double_t tanphi = fTanStereo[side];
  Double_t pitch  = fPitch[side];
  Int_t nStrips   = fNofStrips[side];

  // Calculate distance from lower left corner of the active area.
  // Note: the coordinates are given w.r.t. the centre of the volume.
  Double_t xdist = x + 0.5 * fDx;
  Double_t ydist = y + 0.5 * fDy;

  // Project coordinates to readout (top) edge
  Double_t xro = xdist - ( fDy - ydist ) * tanphi;

  // Calculate corresponding strip number
  Int_t iStrip = TMath::FloorNint( xro / pitch );

  // Account for horizontal cross-connection of strips
  // not extending to the top edge
  while ( iStrip < 0 )        iStrip += nStrips;
  while ( iStrip >= nStrips ) iStrip -= nStrips;

  return iStrip;
}
// -------------------------------------------------------------------------



// -----   Get strip and side from channel number   ------------------------
void CbmStsSensorTypeDssd::GetStrip(Int_t channel, Int_t sensorId,
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

  // --- Offset due to stereo angle
  stripNr += sensorId * fStripShift[side];

  // --- Horizontal cross-connection
  while ( stripNr < 0 ) stripNr += fNofStrips[side];
  while ( stripNr >= fNofStrips[side] ) stripNr -= fNofStrips[side];

  strip = stripNr;

 return;
}
// -------------------------------------------------------------------------



// -----   Intersection of two lines along the strips   --------------------
Bool_t CbmStsSensorTypeDssd::Intersect(Double_t xF, Double_t xB,
		                                   Double_t& x, Double_t& y) {

	// In the coordinate system with origin at the bottom left corner,
	// a line along the strips with coordinate x0 at the top edge is
	// given by the function y(x) = Dy - ( x - x0 ) / tan(phi), if
	// the stereo angle phi does not vanish. Two lines yF(x), yB(x) with top
	// edge coordinates xF, xB intersect at
	// x = ( tan(phiB)*xF - tan(phiF)*xB ) / (tan(phiB) - tan(phiF)
	// y = Dy + ( xB - xF ) / ( tan(phiB) - tan(phiF) )
	// For the case that one of the stereo angles vanish (vertical strips),
	// the calculation of the intersection is straightforward.

	// --- First check whether stereo angles are different. Else there is
	// --- no intersection.
	if ( TMath::Abs(fStereo[0]-fStereo[1]) < 0.5 ) {
		x = -1000.;
		y = -1000.;
		return kFALSE;
	}

	// --- Now treat vertical front strips
	if ( TMath::Abs(fStereo[0]) < 0.001 ) {
		x = xF;
		y = fDy - ( xF - xB ) / fTanStereo[1];
		return IsInside(x-fDx/2., y-fDy/2.);
	}

	// --- Maybe the back side has vertical strips?
	if ( TMath::Abs(fStereo[1]) < 0.001 ) {
		x = xB;
		y = fDy - ( xB - xF ) / fTanStereo[0];
		return IsInside(x-fDx/2., y-fDy/2.);
	}

	// --- OK, both sides have stereo angle
	x = ( fTanStereo[1] * xF - fTanStereo[0] * xB ) /
			( fTanStereo[1] - fTanStereo[0]);
	y = fDy + ( xB - xF ) / ( fTanStereo[1] - fTanStereo[0]);


	// --- Check for being in active area.
	return IsInside(x-fDx/2., y-fDy/2.);

}
// -------------------------------------------------------------------------



// -----  Intersect two clusters (front / back)   --------------------------
Int_t CbmStsSensorTypeDssd::IntersectClusters(CbmStsCluster* clusterF,
		                                          CbmStsCluster* clusterB,
		                                          CbmStsSensor* sensor) {

	// --- Check pointer validity
	if ( ! clusterF ) {
		LOG(FATAL) << GetName() << ": invalid front cluster pointer!"
				        << FairLogger::endl;
		return 0;
	}
	if ( ! clusterB ) {
		LOG(FATAL) << GetName() << ": invalid back cluster pointer!"
				        << FairLogger::endl;
		return 0;
	}
	if ( ! sensor ) {
		LOG(FATAL) << GetName() << ": invalid sensor pointer!"
				        << FairLogger::endl;
		return 0;
	}
	//Ideal hit finder
	if (fHitFinderModel == 0){
		LOG(DEBUG3) << GetName() << ": ideal model of Hit Finder" << FairLogger::endl;
	    const CbmMatch * clusterFMatch, *clusterBMatch;
	    clusterFMatch = static_cast<const CbmMatch*>(clusterF -> GetMatch());
	    if (!clusterFMatch){
		LOG(DEBUG4) << GetName() << ": front cluster exists" << FairLogger::endl;
		if ((clusterFMatch -> GetNofLinks()) > 1) {
		    LOG(DEBUG4) << GetName() << ": front cluster has more than 1 CbmLink" << FairLogger::endl;
		    return 0;
		}
	    }
	    clusterBMatch = static_cast<const CbmMatch*> (clusterB -> GetMatch());
	    if (!clusterBMatch){
		LOG(DEBUG4) << GetName() << ": back cluster exists" << FairLogger::endl;

		if ((clusterBMatch -> GetNofLinks()) > 1){
		    LOG(DEBUG4) << GetName() << ": back cluster has more than 1 CbmLink" << FairLogger::endl;
		    return 0;
		}
	    }
	    if (clusterBMatch -> GetLink(0).GetIndex() != clusterFMatch -> GetLink(0).GetIndex()){
		LOG(DEBUG4) << GetName() << ": back and front clusters have different index of CbmLink" << FairLogger::endl;
		return 0;
	    }
	}

	// --- Calculate cluster centre position on readout edge
	Int_t side  = -1;
	Double_t xF = -1.;
	Double_t xB = -1.;
	GetClusterPosition(clusterF->GetCentre(), sensor, xF, side);
	if ( side != 0 )
		LOG(FATAL) << GetName() << ": Inconsistent side qualifier " << side
		           << " for front side cluster! " << FairLogger::endl;
	GetClusterPosition(clusterB->GetCentre(), sensor, xB, side);
	if ( side != 1 )
		LOG(FATAL) << GetName() << ": Inconsistent side qualifier " << side
		           << " for back side cluster! " << FairLogger::endl;

	// --- Should be inside active area
	if ( ! ( xF >= 0. || xF <= fDx) ) return 0;
	if ( ! ( xB >= 0. || xB <= fDx) ) return 0;

	// --- Hit counter
	Int_t nHits = 0;

	// --- Calculate number of line segments due to horizontal
	// --- cross-connection. If x(y=0) does not fall on the bottom edge,
	// --- the strip is connected to the one corresponding to the line
	// --- with top edge coordinate xF' = xF +/- Dx. For odd combinations
	// --- of stereo angle and sensor dimensions, this could even happen
	// --- multiple times. For each of these lines, the intersection with
	// --- the line on the other side is calculated. If inside the active area,
	// --- a hit is created.
	Int_t nF = Int_t( (xF + fDy * fTanStereo[0]) / fDx );
	Int_t nB = Int_t( (xB + fDy * fTanStereo[1]) / fDx );

	// --- If n is positive, all lines from 0 to n must be considered,
	// --- if it is negative (phi negative), all lines from n to 0.
	Int_t nF1 = TMath::Min(0, nF);
	Int_t nF2 = TMath::Max(0, nF);
	Int_t nB1 = TMath::Min(0, nB);
	Int_t nB2 = TMath::Max(0, nB);

	// --- Double loop over possible lines
	Double_t xC = -1.;   // x coordinate of intersection point
	Double_t yC = -1.;   // y coordinate of intersection point
	for (Int_t iF = nF1; iF <= nF2; iF++) {
		Double_t xFi = xF - Double_t(iF) * fDx;
		for (Int_t iB = nB1; iB <= nB2; iB++) {
		    Double_t xBi = xB - Double_t(iB) * fDx;
		    
		    // --- Intersect the two lines
		    Bool_t found = Intersect(xFi, xBi, xC, yC);
		    LOG(DEBUG4) << GetName() << ": Trying " << xFi << ", " << xBi
					        << ", intersection ( " << xC << ", " << yC
					        << " ) " << ( found ? "TRUE" : "FALSE" )
					        << FairLogger::endl;
			if ( found ) {

				// --- Transform into sensor system with origin at sensor centre
				xC -= 0.5 * fDx;
				yC -= 0.5 * fDy;
				// --- Send hit information to sensor
				sensor->CreateHit(xC, yC, clusterF, clusterB);
				nHits++;

			}  //? Intersection of lines
		}  // lines on back side
	}  // lines on front side

	return nHits;
}
// -------------------------------------------------------------------------



// -----   Check whether a point is inside the active area   ---------------
Bool_t CbmStsSensorTypeDssd::IsInside(Double_t x, Double_t y) {
	if ( x < -fDx/2. ) return kFALSE;
	if ( x >  fDx/2. ) return kFALSE;
	if ( y < -fDy/2. ) return kFALSE;
	if ( y >  fDy/2. ) return kFALSE;
	return kTRUE;
}
// -------------------------------------------------------------------------



// -----   Lorentz shift   -------------------------------------------------
Double_t CbmStsSensorTypeDssd::LorentzShift(Double_t z, Int_t chargeType,
		                                        const CbmStsSensor* sensor)
	                                          const {

	// --- Magnetic field (y component) in sensor centre
	Double_t bY = sensor->GetConditions().GetBy();

	return LorentzShift(z, chargeType, sensor, bY);
}
// -------------------------------------------------------------------------



// -----   Lorentz shift   -------------------------------------------------
Double_t CbmStsSensorTypeDssd::LorentzShift(Double_t z, Int_t chargeType,
		                                        const CbmStsSensor* sensor,
		                                        Double_t bY) const {

	// --- Drift distance to readout plane
	// Electrons drift to the front side (z = d/2), holes to the back side (z = -d/2)
	Double_t driftZ = 0.;
	if      ( chargeType == 0 ) driftZ = fDz / 2. - z;  // electrons
	else if ( chargeType == 1 ) driftZ = fDz / 2. + z;  // holes
	else {
		LOG(ERROR) << GetName() << ": illegal charge type " << chargeType
				       << FairLogger::endl;
		return 0.;
	}

	// --- Hall mobility
	Double_t vBias = sensor->GetConditions().GetVbias();
	Double_t vFd   = sensor->GetConditions().GetVfd();
	Double_t eField = CbmStsPhysics::ElectricField(vBias, vFd, fDz, z + fDz/2.);
	Double_t eFieldMax = CbmStsPhysics::ElectricField(vBias, vFd, fDz, fDz);
	Double_t eFieldMin = CbmStsPhysics::ElectricField(vBias, vFd, fDz, 0.);
	 
	Double_t muHall;
	if (chargeType == 0) muHall = sensor->GetConditions().HallMobility((eField + eFieldMax)/2., chargeType);
	if (chargeType == 1) muHall = sensor->GetConditions().HallMobility((eField + eFieldMin)/2., chargeType);

	// --- The direction of the shift is the same for electrons and holes.
	// --- Holes drift in negative z direction, the field is in
	// --- positive y direction, thus the Lorentz force v x B acts in positive
	// --- x direction. Electrons drift in the opposite (positive z) direction,
	// --- but the have also the opposite charge sign, so the Lorentz force
	// --- on them is also in the positive x direction.
	Double_t shift = muHall * bY * driftZ * 1.e-4;
	LOG(DEBUG4) << GetName() << ": Drift " << driftZ
			        << " cm, mobility " << muHall
			        << " cm**2/(Vs), field " << bY
			        << " T, shift " << shift << " cm" << FairLogger::endl;
	// The factor 1.e-4 is because bZ is in T = Vs/m**2, but muHall is in
	// cm**2/(Vs) and z in cm.

	return shift;
}
// -------------------------------------------------------------------------



// -----   Print parameters   ----------------------------------------------
void CbmStsSensorTypeDssd::Print(Option_t* /*opt*/) const {

  LOG(INFO) << "Properties of sensor type " << GetName() << ": "
            << FairLogger::endl
            << "\t  Dimensions: (" << fixed << setprecision(4)
            << fDx << ", " << fDy << ", " << fDz << ") cm"
            << FairLogger::endl
            << "\t  Front side: pitch = "
            << setprecision(0) << fPitch[0]*1.e4 << " mum, "
            << fNofStrips[0] << " strips, stereo angle "
            << setprecision(1) << fStereo[0] << " degrees"
            << FairLogger::endl
            << "\t  Back side:  pitch = "
            << setprecision(0) << fPitch[1]*1.e4 << " mum, "
            << fNofStrips[1] << " strips, stereo angle "
            << setprecision(1) << fStereo[1] << " degrees"
            << FairLogger::endl;

}
// -------------------------------------------------------------------------



// -----   Print charge status   -------------------------------------------
void CbmStsSensorTypeDssd::PrintChargeStatus() const {
	LOG(INFO) << GetName() << ": Charge status: \n";
	for (Int_t side = 0; side < 2; side++) {
		for (Int_t strip = 0; strip < fNofStrips[side]; strip++) {
			if ( fStripCharge[side][strip] > 0. )
				LOG(INFO) << "          " << (side ? "Back  " : "Front ") << "strip "
				     << strip << "  charge " << fStripCharge[side][strip] << "\n";
		} //# strips
	} //# front and back side
	LOG(INFO) << "          Total: front side "
			 << (fStripCharge[0]).GetSum() << ", back side "
			 << (fStripCharge[1]).GetSum() << FairLogger::endl;
}
// -------------------------------------------------------------------------



// -----   Process an MC Point  --------------------------------------------
Int_t CbmStsSensorTypeDssd::ProcessPoint(CbmStsSensorPoint* point,
                                         const CbmStsSensor* sensor) {

	// --- If desired, use the old ProcessPoint method
	if ( fOld ) {
	  Int_t nSignalsOld = ProcessPointOld(point, sensor);
	  return nSignalsOld;
	}

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

  // --- Number of created charge signals (coded front/back side)
  Int_t nSignals = 0;

  // --- Reset the strip charge arrays
  fStripCharge[0].Reset();   // front side
  fStripCharge[1].Reset();   // back side

  // --- Produce charge and propagate it to the readout strips
  ProduceCharge(point, sensor);

  // --- Cross talk
  if ( CbmStsDigitize::UseCrossTalk() ) {
    if ( FairLogger::GetLogger()->IsLogNeeded(DEBUG4) ) {
    	LOG(DEBUG4) << GetName() << ": Status before cross talk"
    			        << FairLogger::endl;
    	PrintChargeStatus();
    }
  	Double_t ctcoeff =  sensor -> GetConditions().GetCrossTalk();
  	LOG(DEBUG4) << GetName() << ": Cross-talk coefficient is "
  			        << ctcoeff << FairLogger::endl;
  	CrossTalk(ctcoeff);
  }

  // --- Debug
  if ( FairLogger::GetLogger()->IsLogNeeded(DEBUG3) )
  	PrintChargeStatus();

  // --- Stop here if no module is connected (e.g. for test purposes)
  if ( ! sensor->GetModule() ) {
  	/*
  	LOG(WARNING) << GetName() << ": No module connected to sensor!"
  			         << FairLogger::endl;
  			         */
  	return 0;
  }

  // --- Register charges in strips to the module
  Int_t nCharges[2] = { 0, 0 };
  for (Int_t side = 0; side < 2; side ++) {  // front and back side

  	for (Int_t strip = 0; strip < fNofStrips[side]; strip++) {
  		if ( fStripCharge[side][strip] > 0. ) {
  			RegisterCharge(sensor, side, strip, fStripCharge[side][strip],
  					           point->GetTime());
  			nCharges[side]++;
  		} //? charge in strip
  	} //# strips

  } //# front and back side

  // Code number of signals
  nSignals = 1000 * nCharges[0] + nCharges[1];

  return nSignals;
}
// -------------------------------------------------------------------------



// -----   Process an MC Point (old)   -------------------------------------
Int_t CbmStsSensorTypeDssd::ProcessPointOld(CbmStsSensorPoint* point,
                                            const CbmStsSensor* sensor) {

	// TODO: This implementation can be removed after validation of the new one.

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



// -----   Produce charge and propagate it to the readout strips   ---------
void CbmStsSensorTypeDssd::ProduceCharge(CbmStsSensorPoint* point,
		                                     const CbmStsSensor* sensor) {

	// Total charge created in the sensor: is calculated from the energy loss
	Double_t chargeTotal = point->GetELoss()
			                 / CbmStsPhysics::PairCreationEnergy();  // in e


	// For ideal energy loss, just have all charge in the mid-point of the
	// trajectory
	if ( CbmStsDigitize::GetElossModel() == 0 ) {
	  Double_t xP = 0.5 * ( point->GetX1() + point->GetX2() );
	  Double_t yP = 0.5 * ( point->GetY1() + point->GetY2() );
	  Double_t zP = 0.5 * ( point->GetZ1() + point->GetZ2() );
	  PropagateCharge(xP, yP, zP, chargeTotal, point->GetBy(),
	  		            0, sensor); // front side (n)
	  PropagateCharge(xP, yP, zP, chargeTotal, point->GetBy(),
	  		            1, sensor); // back side (p)
	  return;
	}

	// Kinetic energy
	Double_t mass = CbmStsPhysics::ParticleMass(point->GetPid());
	Double_t eKin = TMath::Sqrt( point->GetP() * point->GetP() + mass * mass )
	              - mass;

	// Length of trajectory inside sensor and its projections
	Double_t trajLx = point->GetX2() - point->GetX1();
	Double_t trajLy = point->GetY2() - point->GetY1();
	Double_t trajLz = point->GetZ2() - point->GetZ1();
	Double_t trajLength = TMath::Sqrt( trajLx*trajLx +
			                               trajLy*trajLy +
			                               trajLz*trajLz );

	// The trajectory is sub-divided into equidistant steps, with a step size
	// close to 3 micrometer.
	Double_t stepSizeTarget = 3.e-4;   // targeted step size is 3 micrometer
  Int_t nSteps = TMath::Nint( trajLength / stepSizeTarget );
  if ( nSteps == 0 ) nSteps = 1;     // assure at least one step
	Double_t stepSize  = trajLength / nSteps;
	Double_t stepSizeX = trajLx / nSteps;
	Double_t stepSizeY = trajLy / nSteps;
	Double_t stepSizeZ = trajLz / nSteps;

	// Average charge per step, used for uniform distribution
	Double_t chargePerStep = chargeTotal / nSteps;
	LOG(DEBUG3) << GetName() << ": Trajectory length " << trajLength
			        << " cm, steps " << nSteps << ", step size " << stepSize * 1.e4
			        << " mu, charge per step " << chargePerStep << FairLogger::endl;

	// Stopping power, needed for energy loss fluctuations
	Double_t dedx = 0.;
	if ( CbmStsDigitize::GetElossModel() == 2 )
		dedx = fPhysics->StoppingPower(eKin, point->GetPid());

	// Stepping over the trajectory
	Double_t chargeSum = 0.;
	Double_t xStep = point->GetX1() - 0.5 * stepSizeX;
	Double_t yStep = point->GetY1() - 0.5 * stepSizeY;
	Double_t zStep = point->GetZ1() - 0.5 * stepSizeZ;
	for (Int_t iStep = 0; iStep < nSteps; iStep++ ) {
		xStep += stepSizeX;
		yStep += stepSizeY;
		zStep += stepSizeZ;

		// Charge for this step
		Double_t chargeInStep = chargePerStep;  // uniform energy loss
		if ( CbmStsDigitize::GetElossModel() == 2 ) // energy loss fluctuations
			chargeInStep = fPhysics->EnergyLoss(stepSize, mass, eKin, dedx)
			               / CbmStsPhysics::PairCreationEnergy();
		chargeSum += chargeInStep;

		// Propagate charge to strips
		PropagateCharge(xStep, yStep, zStep, chargeInStep, point->GetBy(),
				            0, sensor);  // front
		PropagateCharge(xStep, yStep, zStep, chargeInStep, point->GetBy(),
				            1, sensor);  // back

	} //# steps of the trajectory

	// For fluctuations: normalise to the total charge from GEANT.
	// Since the number of steps is finite (about 100), the average
	// charge per step does not coincide with the expectation value.
	// In order to be consistent with the transport, the charges are
	// re-normalised.
	if ( CbmStsDigitize::GetElossModel() == 2) {
		for (Int_t side = 0; side < 2; side++) {  // front and back side
			for (Int_t strip = 0; strip < fNofStrips[side]; strip++)
				fStripCharge[side][strip] *= ( chargeTotal / chargeSum );
		} //# front and back side
	} //? E loss fluctuations

}
// -------------------------------------------------------------------------



// -----   Produce charge on the strips   ----------------------------------
Int_t CbmStsSensorTypeDssd::ProduceCharge(CbmStsSensorPoint* point,
                                          Int_t side,
                                          const CbmStsSensor* sensor)
                                          const {

	// TODO: This implementation can be removed after validation of the new one.
  // It projects a uniform charge distribution on the strips.

  // --- Protect against being called without parameters being set
  if ( ! fIsSet ) LOG(FATAL) << "Parameters of sensor " << fName
                             << " are not set!" << FairLogger::endl;

  // This implementation assumes a straight trajectory in the sensor
  // and a constant charge distribution along it.

  // Check for side qualifier
  if ( side < 0 || side > 1 )  {
    LOG(ERROR) << "Illegal side qualifier!" << FairLogger::endl;
    return -1;
  }

  // Total produced charge
  Double_t qtot = point->GetELoss() / CbmStsPhysics::PairCreationEnergy();

  // Stereo angle and strip pitch
  Double_t tanphi = fTanStereo[side];
  Double_t pitch  = fPitch[side];
  Int_t nStrips   = fNofStrips[side];

  // Debug output
  LOG(DEBUG3) << GetName() << ": Side " << side << ", dx = " << fDx
  		        << " cm, dy = " << fDy << " cm, stereo " << fStereo[side]
  		        << " degrees, strips " << fNofStrips[side] << ", pitch "
  		        << pitch << " mum" << FairLogger::endl;

  // Project point coordinates (in / out) along strips to readout (top) edge
  // Keep in mind that the SensorPoint gives coordinates with
  // respect to the centre of the active area.
  Double_t x1 = point->GetX1() + 0.5 * fDx
                - ( 0.5 * fDy - point->GetY1() ) * tanphi;
  Double_t x2 = point->GetX2() + 0.5 * fDx
                - ( 0.5 * fDy - point->GetY2() ) * tanphi;
  LOG(DEBUG3) << GetName() << ": R/O x coordinates are " << x1 << " "
  		        << x2 << FairLogger::endl;


  // Calculate corresponding strip numbers
  // This can be negative or larger than the number of channels if the
  // strip does not extend to the top edge.
  Int_t i1 = TMath::FloorNint( x1 / pitch );
  Int_t i2 = TMath::FloorNint( x2 / pitch );


  // --- More than one strip: sort strips
  if ( i1 > i2 ) {
    Int_t tempI = i1;
    i1 = i2;
    i2 = tempI;
    Double_t tempX = x1;
    x1 = x2;
    x2 = tempX;
  }
  LOG(DEBUG3) << GetName() << ": R/O strips are " << i1 << " to "
  		        << i2 << FairLogger::endl;

  // --- Loop over fired strips
  Int_t nSignals = 0;
  for (Int_t iStrip = i1; iStrip <= i2; iStrip++) {

    // --- Calculate charge in strip
    Double_t y1 = TMath::Max(x1, Double_t(iStrip) * pitch);  // start in strip
    Double_t y2 = TMath::Min(x2, Double_t(iStrip+1) * pitch); // stop in strip
    Double_t charge = (y2 - y1) * qtot / ( x2 - x1 );

    // --- Register charge to module
    RegisterCharge(sensor, side, iStrip, charge, point->GetTime());
    nSignals++;

  } // Loop over fired strips

  return nSignals;
}
// -------------------------------------------------------------------------



// -----   Propagate charge to the readout strips   ------------------------
void CbmStsSensorTypeDssd::PropagateCharge(Double_t x, Double_t y,
		                                       Double_t z, Double_t charge,
		                                       Double_t bY, Int_t side,
		                                       const CbmStsSensor* sensor) {

	// Check side qualifier
	assert( side == 0 || side == 1);

	Double_t xCharge = x;
	Double_t yCharge = y;
	Double_t zCharge = z;

	// Debug
	LOG(DEBUG4) << GetName() << ": Propagating charge " << charge
			        << " from (" << x << ", " << y << ", " << z
			        << ") on side " << side << " of sensor " << sensor->GetName()
			        << FairLogger::endl;

	// Lorentz shift on the drift to the readout plane
	if ( CbmStsDigitize::UseLorentzShift() ) {
		xCharge += LorentzShift(z, side, sensor, bY);
    LOG(DEBUG4) << GetName() << ": After Lorentz shift: (" << xCharge << ", "
		   	        << yCharge << ", " << zCharge << ") cm" << FairLogger::endl;
	}

	// Stop is the charge after Lorentz shift is not in the active area.
	// Diffusion into the active area is not treated.
	if ( ! IsInside(xCharge, yCharge) ) {
		LOG(DEBUG4) << GetName() << ": Charge outside active area"
				        << FairLogger::endl;
		return;
	}

	// No diffusion: all charge is in one strip
	if ( ! CbmStsDigitize::UseDiffusion() ) {
		Int_t iStrip = GetStripNumber(xCharge, yCharge, side);
		fStripCharge[side][iStrip] += charge;
		LOG(DEBUG4) << GetName() << ": Adding charge " << charge << " to strip "
				        << iStrip << FairLogger::endl;
	} //? Do not use diffusion

	// Diffusion: charge is distributed over centre strip and neighbours
	else {
		// Calculate diffusion width
		Double_t diffusionWidth =
			CbmStsPhysics::DiffusionWidth(z + fDz / 2.,// distance from back side
																		fDz,
																		sensor->GetConditions().GetVbias(),
																		sensor->GetConditions().GetVfd(),
																		sensor->GetConditions().GetTemperature(),
																		side);
		assert (diffusionWidth >= 0.);
		LOG(DEBUG4) << GetName() << ": Diffusion width = " << diffusionWidth
				        << " cm" << FairLogger::endl;
		// Calculate charge fractions in strips
		Double_t fracL = 0.;  // fraction of charge in left neighbour
		Double_t fracC = 1.;  // fraction of charge in centre strip
		Double_t fracR = 0.;  // fraction of charge in right neighbour
		Diffusion(xCharge, yCharge, diffusionWidth, side, fracL, fracC, fracR);
		// Calculate strip numbers
		// Note: In this implementation, charge can diffuse out of the sensitive
		// area only for vertical strips. In case of stereo angle (cross-connection
		// of strips), all charge is assigned to some strip, so the edge effects
		// are not treated optimally.
		Int_t iStripC  = GetStripNumber(xCharge, yCharge, side);  // centre strip
		Int_t iStripL  = 0;                                     // left neighbour
		Int_t iStripR  = 0;                                    // right neighbour
		if ( fTanStereo[side] < 0.0001 )  {   // vertical strips, no cross connection
			iStripL = iStripC - 1;  // might be = -1
			iStripR = iStripC + 1;  // might be = nOfStrips
		}
		else {   // stereo angle, cross connection
			iStripL = ( iStripC == 0 ? fNofStrips[side] - 1 : iStripC - 1);
			iStripR = ( iStripC == fNofStrips[side] - 1 ? 0 : iStripC + 1);
		}
		// Collect charge on the readout strips
		if ( fracC > 0. ) {
			fStripCharge[side][iStripC] += charge * fracC;    // centre strip
			LOG(DEBUG4) << GetName() << ": Adding charge " << charge * fracC
				          << " to strip " << iStripC << FairLogger::endl;
		}
		if ( fracL > 0. && iStripL >= 0 ) {
			fStripCharge[side][iStripL] += charge * fracL;  // right neighbour
			LOG(DEBUG4) << GetName() << ": Adding charge " << charge * fracL
					        << " to strip " << iStripL << FairLogger::endl;
		}
		if ( fracR > 0. && iStripR < fNofStrips[side] ) {
			fStripCharge[side][iStripR] += charge * fracR;  // left neighbour
		  LOG(DEBUG4) << GetName() << ": Adding charge " << charge * fracR
				          << " to strip " << iStripR << FairLogger::endl;
		}
	} //? Use diffusion

}
// -------------------------------------------------------------------------



// -----   Register charge   -----------------------------------------------
void CbmStsSensorTypeDssd::RegisterCharge(const CbmStsSensor* sensor,
                                          Int_t side, Int_t strip,
                                          Double_t charge,
                                          Double_t time) const {

  // --- Protect against invalid sensor pointer
	assert ( sensor );

	// --- Check existence of module
	if ( ! sensor->GetModule() ) {
  	LOG(ERROR) << GetName() << ": No module connected to sensor "
  			       << sensor->GetName() << ", side " << side << ", strip "
  			       << strip << ", time " << time << ", charge " << charge
  			       << FairLogger::endl;
		return;
	}

  // --- Determine module channel for given sensor strip
  Int_t channel = GetModuleChannel(strip, side, sensor->GetSensorId() );

  // --- Debug output
  LOG(DEBUG4) << fName << ": Registering charge: side " << side
              << ", strip " << strip << ", time " << time
              << ", charge " << charge
              << " to channel " << channel
              << " of module " << sensor->GetModule()->GetName()
              << FairLogger::endl;

  // --- Get the MC link information
  Int_t index = -1;
  Int_t entry = -1;
  Int_t file  = -1;
  if ( sensor->GetCurrentLink() ) {
  	index = sensor->GetCurrentLink()->GetIndex();
  	entry = sensor->GetCurrentLink()->GetEntry();
  	file  = sensor->GetCurrentLink()->GetFile();
  }

  // --- Send signal to module
  sensor->GetModule()->AddSignal(channel, time, charge, index, entry, file);

}
// -------------------------------------------------------------------------



// -----   Self test   -----------------------------------------------------
Bool_t CbmStsSensorTypeDssd::SelfTest() {

  for (Int_t sensorId = 0; sensorId < 3; sensorId++ ) {
    for (Int_t side = 0; side < 2; side ++ ) {
      for (Int_t strip = 0; strip < fNofStrips[side]; strip++ ) {
        Int_t channel = GetModuleChannel(strip, side, sensorId);
        Int_t testStrip, testSide;
        GetStrip(channel, sensorId, testStrip, testSide);
        if ( testStrip != strip || testSide != side ) {
          LOG(ERROR) << fName << "Self test failed! Sensor " << sensorId
                     << " side " << side << " strip " << strip
                     << " gives channel " << channel << " gives strip "
                     << testStrip << " side " << testSide
                     << FairLogger::endl;
          return kFALSE;
        }
      } // strip loop
    } // side loop
  } // sensor loop

  //LOG(DEBUG2) << fName << ": self test passed" << FairLogger::endl;
  return kTRUE;
}
// -------------------------------------------------------------------------



// -----   Set the parameters   --------------------------------------------
void CbmStsSensorTypeDssd::SetParameters(Double_t dx, Double_t dy,
                                         Double_t dz, Int_t nStripsF,
                                         Int_t nStripsB, Double_t stereoF,
                                         Double_t stereoB) {

  // --- Check stereo angles
  if ( TMath::Abs(stereoF) > 85. )
    LOG(FATAL) << "Stereo angle for front side " << stereoF
               << "outside allowed limits! "
               << "Must be between -85 and 85 degrees."
               << FairLogger::endl;
  if ( TMath::Abs(stereoB) > 85. )
    LOG(FATAL) << "Stereo angle for back side " << stereoB
               << "outside allowed limits! "
               << "Must be between -85 and 85 degrees."
               << FairLogger::endl;

  // --- Set members
  fDx           = dx;
  fDy           = dy;
  fDz           = dz;
  fNofStrips[0] = nStripsF;
  fNofStrips[1] = nStripsB;
  fStereo[0]    = stereoF;
  fStereo[1]    = stereoB;

  // --- Calculate parameters for front and back
  for (Int_t side = 0; side < 2; side++) {
    fPitch[side] = fDx / Double_t(fNofStrips[side]);
    fTanStereo[side] = TMath::Tan( fStereo[side] * TMath::DegToRad() );
    fCosStereo[side] = TMath::Cos( fStereo[side] * TMath::DegToRad() );
    fStripShift[side] = TMath::Nint(fDy * fTanStereo[side] / fPitch[side]);
  }
  fStripCharge[0].Set(fNofStrips[0]);
  fStripCharge[1].Set(fNofStrips[1]);

  // --- Flag parameters to be set if test is ok
  fIsSet = SelfTest();

}
// -------------------------------------------------------------------------



// ----- Set the strip pitch   ---------------------------------------------
void CbmStsSensorTypeDssd::SetStripPitch(Double_t pitch) {

	// Assert that the parameters were set before
	if ( ! fIsSet ) {
		LOG(ERROR) << GetName() << ": SetStripPitch is called without "
				       << "parameters being set before! Will skip statement."
				       << FairLogger::endl;
		return;
	}

	// Re-calculate number of strips on front and back side
	// If the extension in x is not a multiple of the pitch, the number
	// of strips is determined by rounding to the next integer; i.e.,
	// fractional strips smaller than half the pitch are neglected.
	Int_t nStrips = Int_t( round(fDx / pitch) );

	// Re-set the parameters
	SetParameters(fDx, fDy, fDz, nStrips, nStrips, fStereo[0], fStereo[1]);

}
// -------------------------------------------------------------------------



// -----   String output   -------------------------------------------------
string CbmStsSensorTypeDssd::ToString() const
{
   stringstream ss;
   ss << fName << ": ";
   if ( ! fIsSet ) {
  	 ss << "parameters are not set";
  	 return ss.str();
   }
   ss << "Dimension (" << fDx << ", " << fDy << ", " << fDz << ") cm, ";
   ss << "# strips " << fNofStrips[0] << "/" << fNofStrips[1] << ", ";
   ss << "pitch " << fPitch[0] << "/" << fPitch[1] << " cm, ";
   ss << "stereo " << fStereo[0] << "/" << fStereo[1] << " degrees";
   return ss.str();
}
// -------------------------------------------------------------------------



ClassImp(CbmStsSensorTypeDssd)
