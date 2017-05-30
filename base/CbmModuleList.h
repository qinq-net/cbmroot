/** @file CbmModuleList.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 13.06.2013
 **/


#ifndef CBMMODULELIST_H
#define CBMMODULELIST_H 1


#include <map>

#include "Rtypes.h"
#include "TString.h"
#include "CbmAddress.h"
#include "CbmDefs.h"




/** @class CbmModuleList
 ** @brief Manages module Ids and names
 ** @author V.Friese <v.friese@gsi.de>
 ** @version 1.0
 **
 ** CbmModuleList is a tool to assess module names from their identifier
 ** and vice versa through static methods.
 ** Modules can be (detector) systems or passive ones.
 **/
class CbmModuleList
{

  public:

    /**  Constructor   **/
    CbmModuleList() { };


    /** Destructor **/
    virtual ~CbmModuleList() { };


    /** Get module name from module Id
     ** @param moduleId  Unique module identifier (SystemId or kMagnet/kTarget/kPipe)
     ** @return Name of module
     **/
    static TString GetModuleName(Int_t moduleId);


    /** Get module name in capitals from module Id
     ** @param moduleId  Unique module identifier (SystemId or kMagnet/kTarget/kPipe)
     ** @return Name of module (in capitals)
     **/
    static TString GetModuleNameCaps(Int_t moduleId);


   /** Get module Id from module name
     ** @param moduleName Name of module (case insensitive)
     ** @return Unique module Id
     */
    static Int_t GetModuleId(const char* moduleName);



  private:

    /** Map of module identifier to module name **/
    static std::map<Int_t, TString> fModules;

    /** Initialisation of module map **/
    static std::map<Int_t, TString> DefineModules();

};

#endif /* CBMMODULELIST_H */
