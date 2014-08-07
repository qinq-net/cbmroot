// ----------------------------------------------------------------------------
// -----                    CbmMvdHitfinder header file                    -----
// -----                   Created by C. Dritsa (2009)                    -----
// -----                   Maintained by M.Deveaux (m.deveaux(att)gsi.de) -----
// ----------------------------------------------------------------------------

#ifndef CBMMVDHITFINDER_H
#define CBMMVDHITFINDER_H 1

#include "FairTask.h"
#include "CbmMvdDetector.h"
#include "TGeoManager.h"

using namespace std;


class CbmMvdHitfinder : public FairTask
{
 
 public:

  /** Default constructor **/  
  CbmMvdHitfinder();


  /** Standard constructor 
  *@param name  Task name
  *@param mode  0 = MAPS, 1 = Ideal
  **/
  CbmMvdHitfinder(const char* name, 
		    Int_t mode = 0, Int_t iVerbose = 1);


  /** Destructor **/
  virtual ~CbmMvdHitfinder();

  virtual void Exec(Option_t* opt);

  

protected:
 
 


private:

 CbmMvdDetector* fDetector;
 
 TClonesArray* fInputDigis;
 TClonesArray* fHits;

 UInt_t fHitfinderPluginNr;

// -----   Private methods   ---------------------------------------------
 /** Intialisation **/
  virtual InitStatus Init();


  /** Reinitialisation **/
  virtual InitStatus ReInit();


  /** Virtual method Finish **/
  virtual void Finish();


  /** Register the output arrays to the IOManager **/
  void Register();


  /** Clear the hit arrays **/
  void Reset();	 

  void GetMvdGeometry();

  /** Print digitisation parameters **/
  void PrintParameters();

ClassDef(CbmMvdHitfinder,1);    
};
    
    
#endif   		     
