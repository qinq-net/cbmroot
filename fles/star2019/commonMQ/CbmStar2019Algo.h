// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                         CbmStar2019Algo                           -----
// -----               Created 08.11.2018 by P.-A. Loizeau                 -----
// -----                Based on CbmStar2019Algo by F.Uhlig                  -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CbmStar2019Algo_H
#define CbmStar2019Algo_H

#include "Timeslice.hpp"

#include "Rtypes.h"

#include <boost/any.hpp>

#include <vector>
#include <string>
#include <map>

class TList;

template<typename T>
bool is_this_type(const boost::any& varValue)
{
    if (auto q = boost::any_cast<T>(&varValue))
        return true;
    else
        return false;
}


template<typename T>
class CbmStar2019Algo
{
   public:

      CbmStar2019Algo() :
         fParCList( nullptr ),
         fvMsComponentsList(),
         fuNbCoreMsPerTs( 0 ),
         fuNbOverMsPerTs( 0 ),
         fuNbMsLoop( 0 ),
         fbIgnoreOverlapMs( kFALSE ),
         fdMsSizeInNs(-1.0),
         fdTsCoreSizeInNs(-1.0),
         fDigiVect(),
         fParameterMap()
         {};
      virtual ~CbmStar2019Algo() = default;
      CbmStar2019Algo(const CbmStar2019Algo&) = delete;
      CbmStar2019Algo& operator=(const CbmStar2019Algo&) = delete;

      virtual Bool_t Init() = 0;
      virtual void Reset() = 0;
      virtual void Finish() = 0;

      virtual Bool_t ProcessTs( const fles::Timeslice& ts ) = 0;
      virtual Bool_t ProcessTs( const fles::Timeslice& ts, size_t component ) = 0;
      virtual Bool_t ProcessMs( const fles::Timeslice& ts, size_t uMsCompIdx, size_t uMsIdx ) = 0;

      virtual Bool_t InitContainers() = 0;
      virtual Bool_t ReInitContainers() = 0;
      virtual TList* GetParList() = 0;
      virtual void SetParameter(std::string param) {;}
      virtual std::string GetParameter(std::string param) { return std::string{""}; }

      /// For unpacker algos
      void ClearVector() {fDigiVect.clear();}
      std::vector<T> GetVector() {return fDigiVect;}
      virtual void SetTShiftRef(Double_t);

   protected:
      /// Parameter management
      TList* fParCList;

      /// Parameters related to FLES containers
      std::vector< size_t > fvMsComponentsList; //!
      size_t                fuNbCoreMsPerTs;    //!
      size_t                fuNbOverMsPerTs;    //!
      size_t                fuNbMsLoop;         //!
      Bool_t                fbIgnoreOverlapMs;  //! /** Ignore Overlap Ms: all fuOverlapMsNb MS at the end of timeslice **/
      Double_t              fdMsSizeInNs;       //! Size of a single MS, [nanoseconds]
      Double_t              fdTsCoreSizeInNs;   //! Total size of the core MS in a TS, [nanoseconds]

      /// For unpacker algos
      std::vector<T> fDigiVect;

      /// For any algo
      std::map<std::string, std::string> fParameterMap; //! Map of parameter name and type

      Bool_t CheckParamterValidity(std::string parameterName, std::string parameterType) {;}
   private:


};

#endif
