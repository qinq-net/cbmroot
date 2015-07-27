// -------------------------------------------------------------------------
// -----                      FairMCTracks header file                 -----
// -----                Created 10/12/07  by M. Al-Turany              -----
// -------------------------------------------------------------------------


/** from FairMCTracks
 * @author M. Al-Turany
 * @since 10.12.07
 *   MVD event display object
 **
 **/

#ifndef CBMEVDISTRACKS_H
#define CBMEVDISTRACKS_H

#include "FairTask.h"                   // for FairTask, InitStatus

#include "Rtypes.h"                     // for Double_t, etc
#include "TEveTrackPropagator.h"        // IWYU pragma: keep needed by cint
#include "TString.h"                    // for TString

class FairEventManager;
class TClonesArray;
class TEveTrackList;
class TObjArray;
class TParticle;

class CbmEvDisTracks : public FairTask
{

  public:

    /** Default constructor **/
    CbmEvDisTracks();


    /** Standard constructor
    *@param name        Name of task
    *@param iVerbose    Verbosity level
    **/
    CbmEvDisTracks(const char* name, Int_t iVerbose = 1);

    /** Destructor **/
    virtual ~CbmEvDisTracks();

    /** Set verbosity level. For this task and all of the subtasks. **/
    void SetVerbose(Int_t iVerbose) {fVerbose = iVerbose;}
    /** Executed task **/
    virtual void Exec(Option_t* option);
    virtual InitStatus Init();
    virtual void SetParContainers();

    /** Action after each event**/
    virtual void Finish();
    void Reset();
    TEveTrackList* GetTrGroup(Int_t ihmul);

  protected:


    TClonesArray*  fTrackList;  //!
    TEveTrackPropagator* fTrPr;
    FairEventManager* fEventManager;  //!
    TObjArray* fEveTrList;
    TString fEvent; //!
    TEveTrackList* fTrList;  //!
    //TEveElementList *fTrackCont;

    Double_t MinEnergyLimit;
    Double_t MaxEnergyLimit;
    Double_t PEnergy;

  private:
    CbmEvDisTracks(const CbmEvDisTracks&);
    CbmEvDisTracks& operator=(const CbmEvDisTracks&);

    ClassDef(CbmEvDisTracks,1);

};


#endif
