/********************************************************************************
 *    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             * 
 *         GNU Lesser General Public Licence version 3 (LGPL) version 3,        *  
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/
// -------------------------------------------------------------------------
// -----                      FairMCTracks header file                 -----
// -----                Created 10/12/07  by M. Al-Turany              -----
// -------------------------------------------------------------------------


/** FairMCTracks
 * @author M. Al-Turany
 * @since 10.12.07
 *   MVD event display object
 **
 **/

#ifndef CBMRECOTRACKS_H
#define CBMRECOTRACKS_H

#include "FairTask.h"                   // for FairTask, InitStatus

#include "Rtypes.h"                     // for Double_t, etc
#include "TEveTrack.h"                  // for TEveTrackList, TEveTrack
#include "TEveTrackPropagator.h"        // IWYU pragma: keep needed by cint
#include "TString.h"                    // for TString
#include "TClonesArray.h"
#include "CbmTrack.h"
#include "CbmPixelHit.h"

class FairEventManager;
class TEveTrackList;
class TObjArray;
class TParticle;

class CbmRecoTracks : public FairTask
{

  public:

    /** Default constructor **/
    CbmRecoTracks();


    /** Standard constructor
    *@param name        Name of task
    *@param iVerbose    Verbosity level
    **/
    CbmRecoTracks(const char* name, Int_t iVerbose = 1);

    /** Destructor **/
    virtual ~CbmRecoTracks();

    /** Set verbosity level. For this task and all of the subtasks. **/
    void SetVerbose(Int_t iVerbose) {fVerbose = iVerbose;}
    /** Executed task **/
    virtual void Exec(Option_t* option);
    virtual InitStatus Init();
    virtual void SetParContainers();

    /** Action after each event**/
    virtual void Finish();
    void Reset();
    TEveTrackList* GetTrGroup(TParticle* P);

  protected:
      void HandlePixelHit(TEveTrack* eveTrack, Int_t& n, const CbmPixelHit* hit, TEveVector* pMom);
      void HandleTrack(TEveTrack* eveTrack, Int_t& n, TClonesArray* fHits, const CbmTrack* recoTrack, bool setMom);

    TClonesArray* fGlobalTracks;
    TClonesArray* fStsHits;
    TClonesArray* fStsTracks;
    TClonesArray* fRichRings;
    TClonesArray* fMuchPixelHits;
    TClonesArray* fMuchTracks;
    TClonesArray* fTrdHits;
    TClonesArray* fTrdTracks;
    TClonesArray* fTofHits;
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
    CbmRecoTracks(const CbmRecoTracks&);
    CbmRecoTracks& operator=(const CbmRecoTracks&);

    ClassDef(CbmRecoTracks,1);

};


#endif
