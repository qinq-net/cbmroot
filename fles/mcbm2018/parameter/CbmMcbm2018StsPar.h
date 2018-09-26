// -------------------------------------------------------------------------
// -----            CbmMcbm2018StsPar header file                      -----
// -----            Created 25/09/18  by P.-A. Loizeau                 -----
// -------------------------------------------------------------------------

#ifndef CBMMCBM2018STSPAR_H
#define CBMMCBM2018STSPAR_H

#include "FairParGenericSet.h"

#include "TArrayI.h"

class FairParIo;
class FairParamList;


class CbmMcbm2018StsPar : public FairParGenericSet
{

 public:

   /** Standard constructor **/
   CbmMcbm2018StsPar(const char* name    = "CbmMcbm2018StsPar",
                     const char* title   = "Sts parameters",
                     const char* context = "Default");


   /** Destructor **/
   virtual ~CbmMcbm2018StsPar();

   /** Reset all parameters **/
   virtual void clear();

   void putParams(FairParamList*);
   Bool_t getParams(FairParamList*);

   static constexpr UInt_t GetNbCrobsPerDpb()  { return kuNbCrobsPerDpb; }
   static constexpr UInt_t GetNbElinkPerCrob() { return kuNbElinksPerCrob; }
   static constexpr UInt_t GetNbFebsPerCrob()  { return kuNbFebsPerCrob; }
   static constexpr UInt_t GetNbAsicsPerFeb()  { return kuNbAsicsPerFeb; }
   static constexpr UInt_t GetNbChanPerAsic()  { return kuNbChanPerAsic; }

   UInt_t ElinkIdxToAsicIdx( Bool_t bFebType, UInt_t uElink )
         { return kTRUE == bFebType ? ElinkIdxToAsicIdxFebB( uElink ) :
                                      ElinkIdxToAsicIdxFebA( uElink );
         }
   UInt_t ElinkIdxToAsicIdxFebA( UInt_t uElink );
   UInt_t ElinkIdxToAsicIdxFebB( UInt_t uElink );

   UInt_t GetNbOfModules()    { return fuNbModules; }
   UInt_t GetModuleType( UInt_t uModuleIdx );
   UInt_t GetModuleAddress( UInt_t uModuleIdx );

   UInt_t GetNrOfDpbs()       { return fuNrOfDpbs; }
   UInt_t GetDpbId( UInt_t uDpbIdx );

   Bool_t IsCrobActive( UInt_t uDpbIdx, UInt_t uCrobIdx );
   Int_t GetFebModuleIdx( UInt_t uDpbIdx, UInt_t uCrobIdx, UInt_t uFebIdx );
   Int_t GetFebModuleSide( UInt_t uDpbIdx, UInt_t uCrobIdx, UInt_t uFebIdx );

 private:

   /// Constants
   static const UInt_t  kuNbCrobsPerDpb   =   2; // Number of CROBs possible per DPB
   static const UInt_t  kuNbElinksPerCrob =  42; // Number of elinks in each CROB
   static const UInt_t  kuNbFebsPerCrob   =   5; // Number of FEBs  connected to each DPB
   static const UInt_t  kuNbAsicsPerFeb   =   8; // Number of ASICs connected in each FEB
   static const UInt_t  kuNbChanPerAsic   = 128; // Number of channels in each ASIC
//   static constexpr UInt_t  kuCrobMapElinkFebA[ kuNbElinksPerCrob ] = {
   const UInt_t  kuCrobMapElinkFebA[ kuNbElinksPerCrob ] = {
            0x0026, 0x0024, 0x0022, 0x0027, 0x0025, 0x001F,
            0x001D, 0x001B, 0x0023, 0x0020, 0xFFFF, 0xFFFF,
            0x0021, 0x0019, 0x0017, 0x0015, 0x001E, 0x001C,
            0x0018, 0x0009, 0x0016, 0x0014, 0x000B, 0x0012,
            0x0010, 0x0011, 0x001A, 0x0013, 0x000E, 0x000A,
            0x0008, 0x000F, 0x000D, 0x0005, 0x0003, 0x0001,
            0x000C, 0x0002, 0x0007, 0x0004, 0x0000, 0x0006
         }; //! Map from eLink index to ASIC index within CROB ( 0 to kuNbFebsPerCrob * kuNbAsicPerFeb )
//   static constexpr UInt_t  kuCrobMapElinkFebB[ kuNbElinksPerCrob ] = {
   const UInt_t  kuCrobMapElinkFebB[ kuNbElinksPerCrob ] = {
            0x0027, 0x0025, 0x0023, 0x0026, 0x0024, 0x001E,
            0x001C, 0x001A, 0x0022, 0x0021, 0xFFFF, 0xFFFF,
            0x0020, 0x0018, 0x0016, 0x0014, 0x001F, 0x001D,
            0x0019, 0x0008, 0x0017, 0x0015, 0x000A, 0x0013,
            0x0011, 0x0010, 0x001B, 0x0012, 0x000F, 0x000B,
            0x0009, 0x000E, 0x000C, 0x0004, 0x0002, 0x0000,
            0x000D, 0x0003, 0x0006, 0x0005, 0x0001, 0x0007

         }; //! Map from eLink index to ASIC index within CROB ( 0 to kuNbFebsPerCrob * kuNbAsicPerFeb )

   /// Variables
   UInt_t  fuNbModules;      // Total number of STS modules in the setup
   TArrayI fiModuleType;     // Type of each module: 0 for connectors on the right, 1 for connectors on the left
   TArrayI fiModAddress;     // STS address for the first strip of each module

   UInt_t  fuNrOfDpbs;       // Total number of STS DPBs in system
   TArrayI fiDbpIdArray;     // Array to hold the unique IDs (equipment ID) for all STS DPBs
   TArrayI fiCrobActiveFlag; // Array to hold the active flag for all CROBs, [ NbDpb * kuNbCrobPerDpb ]
   TArrayI fiFebModuleIdx;   // Index of the STS module for each FEB, [ NbDpb * kuNbCrobPerDpb * kuNbFebsPerCrob ], -1 if inactive
   TArrayI fiFebModuleSide;  // STS module side for each FEB, [ NbDpb * kuNbCrobPerDpb * kuNbFebsPerCrob ], 0 = P, 1 = N, -1 if inactive

  ClassDef(CbmMcbm2018StsPar,1);
};
#endif // CBMMCBM2018STSPAR_H
