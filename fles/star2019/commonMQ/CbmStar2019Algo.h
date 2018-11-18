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
#include <utility>

class TList;
class TNamed;

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
         fvpAllHistoPointers(),
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

      /// For monitor algos
      void AddHistoToVector( TNamed * pointer, std::string sFolder = "" )
         { fvpAllHistoPointers.push_back( std::pair< TNamed *, std::string >( pointer, sFolder ) ); }
      std::vector< std::pair< TNamed *, std::string > > GetHistoVector() { return fvpAllHistoPointers; }

      /// For unpacker algos
      void ClearVector() {fDigiVect.clear();}
      std::vector<T> GetVector() {return fDigiVect;}

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
      Double_t              fdTsFullSizeInNs;   //! Total size of the core MS in a TS, [nanoseconds]

      /// For monitor algos
      /// => Pointers should be filled with TH1*, TH2*, TProfile*, ...
      /// ==> To check if object N is of type T, use "T ObjectPointer = dynamic_cast<T>( fvpAllHistoPointers[N].first );" and check for nullptr
      /// ==> To get back the original class name use "fvpAllHistoPointers[N].first->ClassName()" which returns a const char * (e.g. "TH1I")
      /// ===> Usage example with feeding a THttpServer:
      /// ===> #include "TH2.h"
      /// ===> std::string sClassName = vHistos[ uHisto ].first.ClassName();
      /// ===> if( !strncmp( sClassName, "TH1", 3 ) )
      /// ===>    server->Register( vHistos[ uHisto ].second.data(), dynamic_cast< TH1 * >(vHistos[ uHisto ].first) );
      /// ===> else if( !strncmp( sClassName, "TH2", 3 ) )
      /// ===>    server->Register( vHistos[ uHisto ].second.data(), dynamic_cast< TH2 * >(vHistos[ uHisto ].first) );
      std::vector< std::pair< TNamed *, std::string > > fvpAllHistoPointers; //! Vector of pointers to histograms + optional folder name

      /// For unpacker algos
      std::vector<T> fDigiVect;

      /// For any algo
      std::map<std::string, std::string> fParameterMap; //! Map of parameter name and type

      Bool_t CheckParamterValidity(std::string parameterName, std::string parameterType) {;}
   private:


};

#endif
