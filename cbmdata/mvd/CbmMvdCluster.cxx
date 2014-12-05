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
 : fDigiArray(),
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
 : 
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


// -----   Destructor   ----------------------------------------------------
CbmMvdCluster::~CbmMvdCluster() {}
// -------------------------------------------------------------------------



ClassImp(CbmMvdCluster)
