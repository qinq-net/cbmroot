/**
 * \file CbmTrdParManager.h
 * \brief Assign pad layout to TRD Modules.
 * \author Florian Uhlig <f.uhlig@gsi.de>
 * \date 06/06/2008
 *
 * Updated 20/05/2013 by Andrey Lebedev <andrey.lebedev@gsi.de>
 *
 * Create digitisation parameter map.
 * Read pad geometry from CbmTrdPads.h,
 * assign pad layout to sectors in TRD modules.
 */

#ifndef CBMTRDPARMANAGER_H
#define CBMTRDPARMANAGER_H

#include "FairTask.h"

#include "TArrayD.h"

#include <map>

class CbmTrdParSetAsic;
class CbmTrdParSetDigi;
class CbmTrdParSetGas;
class CbmTrdParSetGain;
class CbmTrdGeoHandler;

/**
 * \class CbmTrdParManager
 * \brief Manipulate calibration parameters for the TRD detectors
 */
class CbmTrdParManager : public FairTask
{
public:

   /**
    * \brief Default constructor.
    * \param[in] fasp switch between ASICs. Default SPADIC (fasp == kFALSE)
    * \sa SetFASP(Bool_t) 
    **/
   CbmTrdParManager(Bool_t fasp=kFALSE);

   /**
    * \brief Destructor.
    **/
   virtual ~CbmTrdParManager();

   /**
    * \breif Inherited from FairTask.
    **/
   virtual InitStatus Init();

   /**
    * \brief Inherited from FairTask.
    **/
   virtual void SetParContainers();

   /**
    * \brief Change the default ASIC to FASP
    **/
   virtual void SetFASP(Bool_t set=kTRUE)  {fFASP = set;}

   /**
    * \brief Inherited from FairTask.
    **/
   virtual void Exec(Option_t* option);

   /**
    * \breif Inherited from FairTask.
    **/
   virtual void Finish();

private:

   void CreateModuleParameters(
         const TString& path, Bool_t tripad=kFALSE);

   Int_t fMaxSectors; // Maximum number of sectors for all modules
   Bool_t fFASP;      ///< Switch to FASP ASIC

//    // Map of Unique TRD Module Id to corresponding CbmTrdModuleSim
//    std::map<Int_t, CbmTrdModuleSim*> fModuleMap;

   CbmTrdParSetAsic* fAsicPar;  ///< The set of ASIC characterization parameters
   CbmTrdParSetDigi* fDigiPar;  ///< The set of read-out description parameters
   CbmTrdParSetGas*  fGasPar;   ///< The set of gas description parameters
   CbmTrdParSetGain* fGainPar;  ///< The set of gain conversion parameters

   CbmTrdGeoHandler* fGeoHandler;

   CbmTrdParManager(const CbmTrdParManager&);
   CbmTrdParManager& operator=(const CbmTrdParManager&);

   ClassDef(CbmTrdParManager, 1)
};
#endif //CBMTRDPARMANAGER_H
