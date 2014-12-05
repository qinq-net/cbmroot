// -------------------------------------------------------------------------
// -----                CbmMvdCluster source file                  -----

// -------------------------------------------------------------------------

#include "CbmMvdCluster.h"
#include "TVector3.h"

#include <iostream>

using std::cout;
using std::endl;


// -----   Default constructor   -------------------------------------------
CbmMvdCluster::CbmMvdCluster() 
 : CbmCluster(),
   fDigiArray(),
   fNeighbourDown(-1),
   fNeighbourUp(-1),
   fDigisInThisObject(0),
   fTotalDigisInCluster(-1),
   fPixelMap(),
   fStation(-1)
{
  for(Int_t i=0;i<fMaxDigisInObject; i++){fDigiArray[i]=-1;}
}
// -------------------------------------------------------------------------

// -----   Standard constructor   ------------------------------------------
CbmMvdCluster::CbmMvdCluster(Int_t* digiList, Short_t digisInThisObject, Short_t totalDigisInCluster, Int_t neighbourDown) 
 : CbmCluster(),
   fDigiArray(),
   fNeighbourDown(neighbourDown),
   fNeighbourUp(-1),
   fDigisInThisObject(digisInThisObject),
   fTotalDigisInCluster(totalDigisInCluster),
   fStation(-1)
{
    for(Int_t i=0;i<fMaxDigisInObject; i++){fDigiArray[i]=-1;}
    for(Int_t i=0;i<digisInThisObject; i++){fDigiArray[i]=digiList[i];}   
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmMvdCluster::CbmMvdCluster(const CbmMvdCluster& rhs)
  :CbmCluster(),
   fDigiArray(),
   fNeighbourDown(-1),
   fNeighbourUp(-1),
   fDigisInThisObject(0),
   fTotalDigisInCluster(-1),
   fPixelMap(),
   fStation(-1)  
{ 
	fPixelMap = rhs.fPixelMap;
        fNeighbourDown = rhs.fNeighbourDown;
        fNeighbourUp = rhs.fNeighbourUp;
        fDigisInThisObject = rhs.fDigisInThisObject;
        fTotalDigisInCluster = rhs.fTotalDigisInCluster;
        fStation = rhs.fStation;
        fRefId = rhs.fRefId;
        fDetectorId = rhs.fDetectorId;
}
// -------------------------------------------------------------------------

// -----   Destructor   ----------------------------------------------------
CbmMvdCluster::~CbmMvdCluster() {}
// -------------------------------------------------------------------------



ClassImp(CbmMvdCluster)
