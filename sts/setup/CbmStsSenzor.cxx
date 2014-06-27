/** @file CbmStsSenzor.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 03.05.2013
 **/


// Include class header
#include "CbmStsSenzor.h"

// Includes from ROOT
#include "TClonesArray.h"
#include "TGeoMatrix.h"

// Includes from CbmRoot
#include "CbmStsHit.h"
#include "CbmStsPoint.h"

// Includes from STS
#include "setup/CbmStsModule.h"
#include "setup/CbmStsSensorPoint.h"
#include "setup/CbmStsSetup.h"



// -----   Constructor   ---------------------------------------------------
CbmStsSenzor::CbmStsSenzor() : CbmStsElement(), fType(NULL), fHits(NULL)
{
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmStsSenzor::CbmStsSenzor(const char* name, const char* title,
                           TGeoPhysicalNode* node) :
                           CbmStsElement(name, title, kStsSensor, node),
                           fType(NULL), fHits(NULL)
{
}
// -------------------------------------------------------------------------



// -----   Create a new hit   ----------------------------------------------
void CbmStsSenzor::CreateHit(Double_t xLocal, Double_t yLocal,
		                         CbmStsCluster* clusterF,
		                         CbmStsCluster* clusterB) {

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

	// --- Transform into global coordinate system
	Double_t local[3] = { xLocal, yLocal, 0.};
	Double_t global[3];
	Double_t error[3] = { 0., 0., 0. };
	fNode->GetMatrix()->LocalToMaster(local, global);

	// --- Calculate hit time (average of cluster times)
	Double_t hitTime = 0.5 * ( clusterF->GetTime() + clusterB->GetTime());

	// --- Create hit
	Int_t nHits = fHits->GetEntriesFast();
	new ( (*fHits)[nHits] ) CbmStsHit(GetAddress(),   // address
			                              global,         // coordinates
			                              error,          // coord. error
			                              0.,             // covariance xy
			                              0,              // front cluster index
			                              0,              // back cluster index
			                              0,              // front digi index
			                              0,              // back digi index
			                              0,              // sectorNr
			                              hitTime);       // hit time

	LOG(DEBUG2) << GetName() << ": Creating hit at (" << global[0] << ", "
			        << global[1] << ", " << global[2] << ")" << FairLogger::endl;
	return;
}
// -------------------------------------------------------------------------



// -----   Find hits in sensor   -------------------------------------------
Int_t CbmStsSenzor::FindHits(vector<CbmStsCluster*>& clusters,
		                         TClonesArray* hitArray) {
	fHits = hitArray;
	Int_t nHits = fType->FindHits(clusters, this);
	LOG(DEBUG2) << GetName() << ": Clusters " << clusters.size()
			        << ", hits " << nHits << FairLogger::endl;
	return nHits;
}
// -------------------------------------------------------------------------



// -----  Get the mother module   ------------------------------------------
CbmStsModule* CbmStsSenzor::GetModule() const {
	 return dynamic_cast<CbmStsModule*> ( GetMother() );
}
// -------------------------------------------------------------------------



// -----   Process a CbmStsPoint  ------------------------------------------
Int_t CbmStsSenzor::ProcessPoint(CbmStsPoint* point) const {

	// Check whether type is assigned
	if ( ! fType ) {
		LOG(FATAL) << GetName() << ": No sensor type assigned!"
				       << FairLogger::endl;
		return -1;
	}

  Double_t global[3];
  Double_t local[3];

  // --- Debug output of transformation matrix
  if ( FairLogger::GetLogger()->IsLogNeeded(DEBUG4) ) {
  	LOG(DEBUG4) << GetName() << ": Transformation matrix is:"
  			        << FairLogger::endl;
  	fNode->GetMatrix()->Print();
  }

  // --- Transform entry coordinates into local C.S.
  global[0] = point->GetXIn();
  global[1] = point->GetYIn();
  global[2] = point->GetZIn();
  fNode->GetMatrix()->MasterToLocal(global, local);
  Double_t x1 = local[0];
  Double_t y1 = local[1];

  // --- Transform exit coordinates into local C.S.
  global[0] = point->GetXOut();
  global[1] = point->GetYOut();
  global[2] = point->GetZOut();
  fNode->GetMatrix()->MasterToLocal(global, local);
  Double_t x2 = local[0];
  Double_t y2 = local[1];

  // Create SensorPoint
  CbmStsSensorPoint* sPoint = new CbmStsSensorPoint(x1, y1, x2, y2,
                                                    point->GetEnergyLoss(),
                                                    point->GetTime());
  LOG(DEBUG2) << GetName() << ": Local point coordinates are (" << x1
  		        << ", " << y1 << "), (" << x2 << ", " << y2 << ")"
  		        << FairLogger::endl;
  LOG(DEBUG2) << GetName() << ": Sensor type is " << fType->GetName()
  		        << " " << fType->GetTitle() << FairLogger::endl;

  // --- Call ProcessPoint method from sensor type

  return fType->ProcessPoint(sPoint, this);
}
// -------------------------------------------------------------------------


ClassImp(CbmStsSenzor)
