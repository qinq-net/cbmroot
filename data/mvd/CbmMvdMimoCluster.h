/*
  File: CbmMvdMimoCluster.h

*/

#ifndef CbmMvdMimoCluster_H
#define CbmMvdMimoCluster_H 1

#include "TObject.h"
#include <vector>
//#include "FairTimeStamp.h"

class CbmMvdMimoCluster : public TObject
{
  public:
    
    /** Default constructor**/
    CbmMvdMimoCluster();

    /** Constructor with all variables **/
    CbmMvdMimoCluster();
  
  /** destructor**/
   ~CbmMvdMimoCluster();


  /** accesors**/
  Int_t GetHeader()		
  
   /** modifier **/
  //header and framecounter are 32-bits value
  void SetHeader(Int_t header)	
  
  private:
  typedef struct fCluster
    {
      Int_t fFrameCounter;
      Int_t fNrOfCluster;
      Int_t fNrOfPixel;
      Int_t fRowAddrOfFirstPixel;
      Int_t fCulAddrOfFirstPixel;
    } cluster;
    
    std::vector< cluster > fClustVector;  
    
       
ClassDef(CbmMvdMimoCluster,1);
};
#endif