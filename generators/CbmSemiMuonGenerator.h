// -------------------------------------------------------------------------
// -----                 CbmSemiMuononGenerator header file
             
// -----          Created Partha Pratim Bhaduri           -----
// -------------------------------------------------------------------------

/** CbmSemiMuonGenerator.h
 
 * Can be used for dielectron as well
 The CbmSemiMuonGenerator reads the open charm decay output file (ROOT format)
 and inserts the tracks into the FairStack via the FairPrimaryGenerator.
 Derived from FairGenerator.
**/


#ifndef FAIR_SEMIMUONGENERATOR_H
#define FAIR_SEMIMUONGENERATOR_H


#include "FairGenerator.h"

class TClonesArray;
class TFile;
class TTree;
class TBranch;
class TVirtualMCStack;
class FairPrimaryGenerator;



class CbmSemiMuonGenerator : public FairGenerator
{

  public:

    /** Default constructor (should not be used) **/
    CbmSemiMuonGenerator();


    /** Standard constructor
     ** @param fileName The input (PLUTO) file name
     **/
    CbmSemiMuonGenerator(const Char_t* fileName);


    /** Destructor **/
    virtual ~CbmSemiMuonGenerator();


    /** Reads on event from the input file and pushes the tracks onto
     ** the stack. Abstract method in base class.
     ** @param primGen  pointer to the FairPrimaryGenerator
     **/
    virtual Bool_t ReadEvent(FairPrimaryGenerator* primGen);



  private:

    Int_t iEvent;      //! Event number
    const Char_t* fFileName;   //! Input file name
    TFile* fInputFile;        //! Pointer to input file
    TTree* fInputTree;        //! Pointer to input tree
    //  TClonesArray* fParticles;  //! Particle array from PLUTO

    TBranch        *b_Run;   //!
    TBranch        *b_Event;   //!
    TBranch        *b_trackNo;   //!
    TBranch        *b_Px;   //!
    TBranch        *b_Py;   //!
    TBranch        *b_Pz;   //!
    TBranch        *b_En;   //!

    Int_t           Run;
    Int_t           Event;
    Int_t           trackNo;
    Double_t        Px[2];   //[trackNo]
    Double_t        Py[2];   //[trackNo]
    Double_t        Pz[2];   //[trackNo]
    Double_t        En[2];   //[trackNo]


    /** Private method CloseInput. Just for convenience. Closes the
     ** input file properly. Called from destructor and from ReadEvent. **/
    void CloseInput();

    CbmSemiMuonGenerator(const CbmSemiMuonGenerator&);
    CbmSemiMuonGenerator& operator=(const CbmSemiMuonGenerator&);

    ClassDef(CbmSemiMuonGenerator,1);

};

#endif
