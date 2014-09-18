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
 : TObject(),
   fDigiArray(),
   fNeighbourDown(-1),
   fNeighbourUp(-1),
   fDigisInThisObject(0),
   fTotalDigisInCluster(-1)
{
  for(Int_t i=0;i<fMaxDigisInObject; i++){fDigiArray[i]=-1;}
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmMvdCluster::CbmMvdCluster(Int_t* digiList, Short_t digisInThisObject, Short_t totalDigisInCluster, Int_t neighbourDown) 
 : TObject(),
   fDigiArray(),
   fNeighbourDown(neighbourDown),
   fNeighbourUp(-1),
   fDigisInThisObject(digisInThisObject),
   fTotalDigisInCluster(totalDigisInCluster)
{
    for(Int_t i=0;i<fMaxDigisInObject; i++){fDigiArray[i]=-1;}
    for(Int_t i=0;i<digisInThisObject; i++){fDigiArray[i]=digiList[i];}   
}
// -------------------------------------------------------------------------

Int_t CbmMvdCluster::GetDigiIndex(UInt_t index){
  
 
  if (index<fDigisInThisObject){ return fDigiArray[index];}
  else {cout << "-E- "<< GetName() <<"::GetDigiIndex(UInt_t entry) - You called for an invalid entry. Returning -1."<< endl;
	return -1;
  }
  

}

// -----   Destructor   ----------------------------------------------------
CbmMvdCluster::~CbmMvdCluster() {}
// -------------------------------------------------------------------------



ClassImp(CbmMvdCluster)
