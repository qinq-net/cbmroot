// -------------------------------------------------------------------------
// -----             CbmCosmicGenerator header file                    -----
// -----              Created 05/09/18  by F.Uhlig                     -----
// -------------------------------------------------------------------------

/**  CbmCosmicGenerator.h
 *@author F.Uhlig <f.uhlig@gsi.de>
 *
 * The CbmCosmicGenerator generates one particle from a plane 
 * with a given position in y and variable sizes in x and z
 * 
 * The phi and theta angles are according to the distribution
 * of cosmic particles.
**/

#ifndef CBMCOSMICGENERATOR_H
#define CBMCOSMICGENERATOR_H

#include "FairGenerator.h"              // for FairGenerator

#include "Rtypes.h"                     // for Double32_t, Bool_t, kTRUE, etc

class FairPrimaryGenerator;

class CbmCosmicGenerator : public FairGenerator
{
  public:

    /** Default constructor. **/
    CbmCosmicGenerator();

    /** Constructor with PDG-ID, multiplicity
     **@param pdgid Particle type (PDG encoding)
     **@param mult  Multiplicity (default is 1)
     **/
    CbmCosmicGenerator(Int_t pdgid, Int_t mult=1);

    /** Copy constructor. **/
    CbmCosmicGenerator(const CbmCosmicGenerator&)=delete;

    /** Assignment operator **/
    CbmCosmicGenerator& operator=(const CbmCosmicGenerator&)=delete;

    /** Destructor **/
    virtual ~CbmCosmicGenerator() {};

    /** Modifiers **/
    void SetPDGType      (Int_t pdg)  {fPDGType = pdg;  };

    void SetMultiplicity (Int_t mult) {fMult    = mult; };

    void SetPlaneXYZ (Double32_t x1=0, Double32_t x2=0, Double32_t y=0, Double32_t z1=0, Double32_t z2=0) 
    {
      fX1=x1;
      fY=y;
      fX2=x2;
      fZ1=z1;
      fZ2=z2;
    }

    /** Initializer **/
    Bool_t Init();

    /** Creates an event with given type and multiplicity.
     **@param primGen  pointer to the FairPrimaryGenerator
     **/
    virtual Bool_t ReadEvent(FairPrimaryGenerator* primGen);

    /** Clone this object (used in MT mode only) */
//    virtual FairGenerator* CloneGenerator() const;

  protected:

  private:
    Int_t      fPDGType;             // Particle type (PDG encoding)
    Int_t      fMult;                // Multiplicity

    Double32_t fX1, fX2, fY, fZ1, fZ2;   // Plane vertex coords 

    ClassDef(CbmCosmicGenerator,1);

};


#endif
