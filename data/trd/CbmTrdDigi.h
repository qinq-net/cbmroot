#ifndef CBMTRDDIGI_H
#define CBMTRDDIGI_H

#include "CbmDigi.h"
#include "CbmDefs.h"
#include <string>

class CbmTrdDigi : public CbmDigi
{
public:
  enum CbmTrdAsicType{
    kSPADIC = 0
    ,kFASP
    ,kNTypes 
  };
  enum CbmTrdTriggerType{
    kSelf = 0
    ,k1
    ,k2
    ,kNeighbor 
  };
  enum CbmTrdDigiDef{
    kType = 0   //< define ASIC type producing the digi
    ,kFlag2     //< 
    ,kFlag3     //< in case of FASP simulations define pileup
    ,kFlag4     //< in case of FASP simulations define masked
    ,kNflags
  };
  /**
   * \brief Default constructor.
   */
  CbmTrdDigi();
  /**
   * \brief Constructor for the FASP type.
   * \param[in] address  Unique channel address in the module.
   * \param[in] chargeT Charge for tilt pad parring.
   * \param[in] chargeR Charge for rectangle pad parring.
   * \param[in] time   Absolute time [ASIC clocks].
   */
  CbmTrdDigi(Int_t address, Float_t chargeT, Float_t chargeR, ULong64_t time);
  /**
   * \brief Constructor for the SPADIC type.
   * \param[in] address  Unique channel address in the module.
   * \param[in] charge Charge.
   * \param[in] time   Absolute time [ASIC clocks].
   * \param[in] triggerType SPADIC trigger type see CbmTrdTriggerType.
   * \param[in] errClass SPADIC signal error parametrization based on message type.
   */
  CbmTrdDigi(Int_t address, Float_t charge, ULong64_t time, Int_t triggerType, Int_t errClass/*nrSamples*/);
  
  /** \brief Charge addition in case of pile-up (FASP simulation only)
   * \param[in] sd previous digi absorbed by current
   * \param[in] f scaling factor
   */
  void      AddCharge(CbmTrdDigi *sd, Double_t f);
  /** \brief Charge addition in case of pile-up (SPADIC simulation only)
   * \param[in] c extra charge to be added
   * \param[in] f scaling factor
   */
  void      AddCharge(Double_t c, Double_t f=1);
  /** \brief DAQ clock accessor for each ASIC*/
  static Float_t Clk(CbmTrdAsicType ty)     { return (ty==kNTypes?0:fgClk[ty]);}
  /** \brief Inherited from CbmDigi.*/
  Int_t     GetAddress() const              { return GetAddressChannel();}
  /** \brief Getter read-out id.
   * \return index of row-column read-out unit
   */
  Int_t     GetAddressChannel() const       { return fAddress&0xfff;}
  /** \brief Getter module id.
   * \return index of module in the experimental set-up. Should be dropped if data get organized module-wise
   */
  Int_t     GetAddressModule() const        { return (fAddress&0xff000)>>12;}
  /** \brief Charge getter for SPADIC*/
  Double_t  GetCharge()  const;
  /** \brief Charge getter for FASP
   * \param[in] tilt on returns contain the charge measured on tilted pads
   * \return charge measured on rectangular coupled pads
   */
  Double_t  GetCharge(Double_t &tilt)  const;
  /** \brief Charge error parametrisation. SPADIC specific see GetErrorClass()*/
  Double_t  GetChargeError()  const;
  /** \brief Inherited from CbmDigi.*/
  Int_t     GetSystemId() const             { return kTrd;} 
  /** \brief Getter for physical time [ns]. Accounts for clock representation of each ASIC*/
  Double_t  GetTime() const                 { return fTime*fgClk[GetType()];}
  /** \brief Getter for global DAQ time [clk]. Differes for each ASIC*/
  ULong64_t GetTimeDAQ() const              { return fTime;}
  /** \brief Channel trigger type. SPADIC specific see CbmTrdTriggerType*/
  Int_t     GetTriggerType() const          { return fAddress>>30;}
  /** \brief Channel status. SPADIC specific see LUT*/
  Int_t     GetErrorClass() const           { return (fAddress>>24)&0x3f;}

  /** \brief Query digi mask (FASP only)*/
  Bool_t    IsMasked() const {return (IsFlagged(kType)==kFASP)&&IsFlagged(kFlag4);}
  /** \brief Query digi pile-up (FASP only)*/
  Bool_t    IsPileUp() const {return (IsFlagged(kType)==kFASP)&&IsFlagged(kFlag3);}
  /** \brief Query flag status (generic)*/
  Bool_t    IsFlagged(const Int_t iflag)  const;
  
  void      SetAddressChannel(const Int_t a){ fAddress|=(a&0xfff);}
  void      SetAddressModule(const Int_t a) { fAddress|=((a&0xff)<<12);}
  void      SetAsic(CbmTrdAsicType ty=kSPADIC);
  /** \brief Charge setter for SPADIC ASIC
   * \param[in] c charge on read-out pad
   */
  void      SetCharge(Float_t c);
  /** \brief Charge setter for FASP ASIC
   * \param[in] cT charge on tilt paired
   * \param[in] cR charge on rectangular paired
   */
  void      SetCharge(Float_t cT, Float_t cR);
  /** \brief Generic flag status setter*/
  void      SetFlag(const Int_t iflag, Bool_t set=kTRUE);
  /** \brief Set digi mask (FASP only)*/
  void      SetMasked(Bool_t set=kTRUE)     { if(IsFlagged(kType)==kFASP) SetFlag(kFlag4, set); }
  /** \brief Set digi pile-up (FASP only)*/
  void      SetPileUp(Bool_t set=kTRUE)     { if(IsFlagged(kType)==kFASP) SetFlag(kFlag3, set); }
  /** \brief Set global digi time (ns)*/
  void      SetTime(Double_t t)             { fTime=ULong64_t(t/fgClk[GetType()]);}
  /** \brief Set global digi time (clk)*/
  void      SetTimeDAQ(ULong64_t t)         { fTime=t;}
  /** \brief Set digi trigger type (SPADIC only)*/
  void      SetTriggerType(const Int_t ttype);
  /** \brief Set digi error class (SPADIC only)*/
  void      SetErrorClass(const Int_t n)    { if(IsFlagged(kType)==kSPADIC) fAddress|=((n&0x3f)<<24);}
  /** \brief String representation of a TRD digi. Account for digi type and specific information.*/
  std::string ToString() const;
  
  // TEMPORARY ADDED WITHOUT PROVIDING FUNCTIONALITY ! TODO should be removed 
  // keep backward compatible compilation of CbmTrdSPADIC
  void SetStopType(Int_t /*stopType*/)  {;}
  Int_t GetStopType() {return 0;}
  void SetPulseShape(Float_t pulse[45]) {;}
  // keep backward compatible compilation of fles/reader/tasks [/tools]
  Double_t GetChargeTR()  {return 0.;}
  void SetInfoType(Int_t /*infoType*/)  {;}
  void SetAddress(const Int_t a) { SetAddressModule(a);}
  CbmTrdDigi(Int_t /*address*/, Double_t /*fullTime*/, Int_t /*triggerType*/, Int_t /*infoType*/, Int_t /*stopType*/, Int_t /*nrSamples*/, Float_t* /*samples*/) {;}
  Int_t GetNrSamples() { return 0;}
  Float_t* GetSamples() {return NULL;}
  
protected:
  CbmTrdAsicType     GetType() const;
  
  UInt_t    fAddress;     //< pad address and extra information
  UInt_t    fCharge;      //< measured charge. For SPADIC is Int_t(charge*1eN) where N is the precission while 
                          //< for FASP it contains the R and T charges each on 16bits.
  ULong64_t fTime;        //< global time of the digi in ASIC clock
  
  static Float_t  fgClk[kNTypes]; //< clock length in ns for acquisition
  static Float_t  fgPrecission[kNTypes];   //< no. of digits stored for ASIC
  
  ClassDef(CbmTrdDigi, 1);   // Production ready TRD digit
};

#endif
