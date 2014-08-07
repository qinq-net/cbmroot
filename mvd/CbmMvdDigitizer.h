// ----------------------------------------------------------------------------
// -----                    CbmMvdDigitizer header file                    -----
// -----                   Created by C. Dritsa (2009)                    -----
// -----                   Maintained by M.Deveaux (m.deveaux(att)gsi.de) -----
// ----------------------------------------------------------------------------


/** CbmMvdDigitize header file
 ** Read "ReadmeMvdDigitizer.pdf" for instructions
 **
 ** Obsolete version with limited physics model.
 ** The use of CbmMvdDigitizeL is recommended
 **/


#ifndef CBMMVDDIGITIZER_H
#define CBMMVDDIGITIZER_H 1

#include "FairTask.h"
#include "CbmMvdDetector.h"
#include "TGeoManager.h"

using namespace std;


class CbmMvdDigitizer : public FairTask
{
 
 public:

  /** Default constructor **/  
  CbmMvdDigitizer();


  /** Standard constructor 
  *@param name  Task name
  *@param mode  0 = MAPS, 1 = Ideal
  **/
  CbmMvdDigitizer(const char* name, 
		    Int_t mode = 0, Int_t iVerbose = 1);


  /** Destructor **/
  virtual ~CbmMvdDigitizer();

  virtual void Exec(Option_t* opt);



protected:
 
 


private:

 CbmMvdDetector* fDetector;
 
 TClonesArray* fInputPoints;
 TClonesArray* fDigis;
 TClonesArray* fDigiMatch;

 UInt_t fDigiPluginNr;

// -----   Private methods   ---------------------------------------------
 /** Intialisation **/
  virtual InitStatus Init();


  /** Reinitialisation **/
  virtual InitStatus ReInit();


  /** Virtual method Finish **/
  virtual void Finish();


  /** Register the output arrays to the IOManager **/
  void Register();

  void GetMvdGeometry();


  /** Clear the hit arrays **/
  void Reset();	 


  /** Print digitisation parameters **/
  void PrintParameters();

ClassDef(CbmMvdDigitizer,1);    
};
    
    
#endif   		     
