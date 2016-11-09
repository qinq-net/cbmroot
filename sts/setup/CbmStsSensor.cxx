/** @file CbmStsSensor.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 03.05.2013
 **/


// Include class header
#include "CbmStsSensor.h"

// Includes from c++
#include <cassert>

// Includes from ROOT
#include "TClonesArray.h"
#include "TGeoBBox.h"
#include "TGeoMatrix.h"
#include "TMath.h"

// Includes from FairRoot
#include "FairField.h"
#include "FairRunAna.h"

// Includes from CbmRoot
#include "CbmEvent.h"
#include "CbmLink.h"
#include "CbmStsHit.h"
#include "CbmStsPoint.h"

// Includes from STS
#include "setup/CbmStsModule.h"
#include "setup/CbmStsSensorConditions.h"
#include "setup/CbmStsSensorPoint.h"
#include "setup/CbmStsSetup.h"
#include "digitize/CbmStsSensorTypeDssd.h"

using std::vector;


// -----   Constructor   ---------------------------------------------------
CbmStsSensor::CbmStsSensor() : CbmStsElement(),
                               fType(NULL),
                               fConditions(),
                               fCurrentLink(NULL),
                               fHits(NULL),
                               fEvent(NULL)
{
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmStsSensor::CbmStsSensor(const char* name, const char* title,
                           TGeoPhysicalNode* node) :
                           CbmStsElement(name, title, kStsSensor, node),
                           fType(NULL),
                           fConditions(),
                           fCurrentLink(NULL),
                           fHits(NULL),
						   fEvent(NULL)
{
}
// -------------------------------------------------------------------------



// -----   Create a new hit   ----------------------------------------------
void CbmStsSensor::CreateHit(Double_t xLocal, Double_t yLocal, Double_t varX,
		                     Double_t varY, Double_t varXY,
		                     CbmStsCluster* clusterF, CbmStsCluster* clusterB) {

  // ---  Check clusters and output array
	if ( ! fHits ) {
		LOG(FATAL) << GetName() << ": Hit output array not set!"
				       << FairLogger::endl;
		return;
	}
	if ( ! clusterF ) {
		LOG(FATAL) << GetName() << ": Invalid pointer to front cluster!"
				       << FairLogger::endl;
	}
	if ( ! clusterB ) {
		LOG(FATAL) << GetName() << ": Invalid pointer to back cluster!"
				       << FairLogger::endl;
	}

	// --- If a TGeoNode is attached, transform into global coordinate system
	Double_t local[3] = { xLocal, yLocal, 0.};
	Double_t global[3];
	if ( fNode ) fNode->GetMatrix()->LocalToMaster(local, global);
	else {
		global[0] = local[0];
		global[1] = local[1];
		global[2] = local[2];
	}

	// We assume here that the local-to-global transformations is only translation
	// plus maybe rotation upside down or front-side back. In that case, the
	// global covariance matrix is the same as the local one.
	Double_t error[3] = { TMath::Sqrt(varX), TMath::Sqrt(varY), 0.};


	// --- Calculate hit time (average of cluster times)
	Double_t hitTime = 0.5 * ( clusterF->GetTime() + clusterB->GetTime());
	Double_t etF = clusterF->GetTimeError();
	Double_t etB = clusterB->GetTimeError();
	Double_t hitTimeError = TMath::Sqrt( (etF*etF + etB*etB) / 2. );

	// --- Create hit
	Int_t index = fHits->GetEntriesFast();
	new ( (*fHits)[index] )
			CbmStsHit(GetAddress(),          // address
					      global,                // coordinates
					      error,                 // coordinate error
					      varXY,                 // covariance xy
					      clusterF->GetIndex(),  // front cluster index
					      clusterB->GetIndex(),  // back cluster index
					      hitTime,               // hit time
					      hitTimeError);         // hit time error
	if ( fEvent) fEvent->AddData(Cbm::kStsHit, index);

	LOG(DEBUG2) << GetName() << ": Creating hit at (" << global[0] << ", "
			        << global[1] << ", " << global[2] << ")" << FairLogger::endl;
	return;
}
// -------------------------------------------------------------------------



// -----   Find hits in sensor   -------------------------------------------
Int_t CbmStsSensor::FindHits(vector<CbmStsCluster*>& clusters,
		                         TClonesArray* hitArray, CbmEvent* event,
		                         Double_t dTime) {
	fHits = hitArray;
	fEvent = event;
	Int_t nHits = fType->FindHits(clusters, this, dTime);
	LOG(DEBUG2) << GetName() << ": Clusters " << clusters.size()
			        << ", hits " << nHits << FairLogger::endl;
	return nHits;
}
// -------------------------------------------------------------------------



// -----  Get the mother module   ------------------------------------------
CbmStsModule* CbmStsSensor::GetModule() const {
	 return dynamic_cast<CbmStsModule*> ( GetMother() );
}
// -------------------------------------------------------------------------



// -----   Process a CbmStsPoint  ------------------------------------------
Int_t CbmStsSensor::ProcessPoint(const CbmStsPoint* point,
		                             Double_t eventTime, CbmLink* link) {

	// Check whether type is assigned
	if ( ! fType ) {
		LOG(FATAL) << GetName() << ": No sensor type assigned!"
				       << FairLogger::endl;
		return -1;
	}

	// --- Set current link
	fCurrentLink = link;

  // --- Debug output of transformation matrix
  if ( FairLogger::GetLogger()->IsLogNeeded(DEBUG4) ) {
  	LOG(DEBUG4) << GetName() << ": Transformation matrix is:"
  			        << FairLogger::endl;
  	fNode->GetMatrix()->Print();
  }

  // --- Transform start coordinates into local C.S.
  Double_t global[3];
  Double_t local[3];
  global[0] = point->GetXIn();
  global[1] = point->GetYIn();
  global[2] = point->GetZIn();
  fNode->GetMatrix()->MasterToLocal(global, local);
  Double_t x1 = local[0];
  Double_t y1 = local[1];
  Double_t z1 = local[2];

  // --- Transform stop coordinates into local C.S.
  global[0] = point->GetXOut();
  global[1] = point->GetYOut();
  global[2] = point->GetZOut();
  fNode->GetMatrix()->MasterToLocal(global, local);
  Double_t x2 = local[0];
  Double_t y2 = local[1];
  Double_t z2 = local[2];

  // --- Average track direction in local c.s.
  Double_t tXav = 0.;
  Double_t tYav = 0.;
  Int_t    tZav = 0;
  if ( z2 - z1 != 0. ) {
  	tXav = ( x2 - x1 ) / (z2 - z1);
  	tYav = ( y2 - y1 ) / (z2 - z1);
  	tZav = 1;
  }

  // --- Normally, the entry and exit coordinates are slightly outside of
  // --- the active node,which is a feature of the transport engine.
  // --- We correct here for this, in case a track was entering or
  // --- exiting the sensor (not for tracks newly created or stopped
  // --- in the sensor volume).
  // --- We here consider only the case of tracks leaving through the front
  // --- or back plane. The rare case of tracks leaving through the sensor
  // --- sides is caught by the digitisation procedure.
  Double_t dZ = dynamic_cast<TGeoBBox*>(fNode->GetShape())->GetDZ();

  // --- Correct start coordinates in case of entry step
  if ( point->IsEntry() ) {

 		// Get track direction in local c.s.
 		global[0] = point->GetPx();
 		global[1] = point->GetPy();
 		global[2] = point->GetPz();
 		Double_t* rot;
 		rot = fNode->GetMatrix()->GetRotationMatrix();
 		TGeoHMatrix rotMat;
 		rotMat.SetRotation(rot);
 		rotMat.MasterToLocal(global,local);
 		if ( local[2] != 0.) {;  // should always be; else no correction
 			Double_t	tX = local[0] / local[2]; // px/pz
 			Double_t	tY = local[1] / local[2]; // py/pz

 			// New start coordinates
 			Double_t xNew = 0.;
 			Double_t yNew = 0.;
 			Double_t zNew = 0.;
 			if ( z1 > 0. ) zNew = dZ - 1.e-4; // front plane, safety margin 1 mum
 			else           zNew = 1.e-4 - dZ; // back plane, safety margin 1 mum
 			xNew = x1 + tX * (zNew - z1);
 			yNew = y1 + tY * (zNew - z1);

 			x1 = xNew;
 			y1 = yNew;
 			z1 = zNew;
 		} //? pz != 0.

  }  //? track has entered

  // --- Correct stop coordinates in case of being outside the sensor
  if ( TMath::Abs(z2) > dZ ) {

  	// Get track direction in local c.s.
  	global[0] = point->GetPxOut();
 		global[1] = point->GetPyOut();
 		global[2] = point->GetPzOut();
 		Double_t* rot;
  	rot = fNode->GetMatrix()->GetRotationMatrix();
 		TGeoHMatrix rotMat;
 		rotMat.SetRotation(rot);
 		rotMat.MasterToLocal(global,local);
 		Double_t tX = 0.;
 		Double_t tY = 0.;
 		// Use momentum components for track direction, if available
 		if ( local[2] != 0. ) {
 			tX = local[0] / local[2]; // px/pz
 			tY = local[1] / local[2]; // py/pz
 		}
 		// Sometimes, a track is stopped outside the sensor volume.
 		// Then we take the average track direction as best approximation.
 		// Note that there may be cases where entry and exit coordinates are
 		// the same. In this case, tXav = tYav = 0; there will be no correction
 		// of the coordinates.
 		else {
 		  tX = tXav;  // (x2-x1)/(z2-z1) or 0 if z2 = z1
 		  tY = tYav;  // (y2-y1)/(z2-z1) or 0 if z2 = z1
 		}

 		// New coordinates
 		Double_t xNew = 0.;
 		Double_t yNew = 0.;
 		Double_t zNew = 0.;
 		if ( z2 > 0. ) zNew = dZ - 1.e-4; // front plane, safety margin 1 mum
 		else           zNew = 1.e-4 - dZ; // back plane, safety margin 1 mum
 		xNew = x2 + tX * (zNew - z2);
 		yNew = y2 + tY * (zNew - z2);

 		x2 = xNew;
 		y2 = yNew;
 		z2 = zNew;

  } //? track step outside sensor


  // --- Momentum magnitude
  Double_t px = 0.5 * ( point->GetPx() + point->GetPxOut() );
  Double_t py = 0.5 * ( point->GetPy() + point->GetPyOut() );
  Double_t pz = 0.5 * ( point->GetPz() + point->GetPzOut() );
  Double_t p = TMath::Sqrt( px*px + py*py + pz*pz );

  // --- Get magnetic field
  global[0] = 0.5 * ( point->GetXIn() + point->GetXOut() );
  global[1] = 0.5 * ( point->GetYIn() + point->GetYOut() );
  global[2] = 0.5 * ( point->GetZIn() + point->GetZOut() );
  Double_t bField[3] = { 0., 0., 0.};
  if ( FairRun::Instance() -> GetField())
  	FairRun::Instance()->GetField()->Field(global, bField);

  // --- Absolute time of StsPoint
  Double_t pTime = eventTime + point->GetTime();

  // --- Create SensorPoint
  // Note: there is a conversion from kG to T in the field values.
  CbmStsSensorPoint* sPoint = new CbmStsSensorPoint(x1, y1, z1, x2, y2, z2, p,
                                                    point->GetEnergyLoss(),
                                                    pTime,
                                                    bField[0] / 10.,
                                                    bField[1] / 10.,
                                                    bField[2] / 10.,
                                                    point->GetPid());
  LOG(DEBUG2) << GetName() << ": Local point coordinates are (" << x1
  		        << ", " << y1 << "), (" << x2 << ", " << y2 << ")"
  		        << FairLogger::endl;
  LOG(DEBUG2) << GetName() << ": Sensor type is " << fType->GetName()
  		        << " " << fType->GetTitle() << FairLogger::endl;
  LOG(DEBUG2) << point->IsEntry() << " " << point->IsExit() << FairLogger::endl;

  // --- Call ProcessPoint method from sensor type
  Int_t result = fType->ProcessPoint(sPoint, this);
  delete sPoint;

  return result;
}
// -------------------------------------------------------------------------


ClassImp(CbmStsSensor)
