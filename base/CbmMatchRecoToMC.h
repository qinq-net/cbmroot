/**
 * \file CbmMatchRecoToMC.h
 * \brief FairTask for matching RECO data to MC.
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2013
 */

#ifndef CBMMATCHRECOTOMC_H_
#define CBMMATCHRECOTOMC_H_

#include "FairTask.h"
#include <vector>
#include <utility>

class TClonesArray;
class CbmMCDataArray;
class CbmRichHit;

class CbmMatchRecoToMC : public FairTask
{
public:
   /**
    * \brief Constructor.
    */
   CbmMatchRecoToMC();

   /**
    * \brief Destructor.
    */
   virtual ~CbmMatchRecoToMC();

   /**
     * \brief Derived from FairTask.
     */
    virtual InitStatus Init();

    /**
     * \brief Derived from FairTask.
     */
    virtual void Exec(
       Option_t* opt);

    /**
     * \brief Derived from FairTask.
     */
    virtual void Finish();

    void SetIncludeMvdHitsInStsTrack(Bool_t includeMvdHitsInStsTrack);

private:
    /**
     * \brief Read and create data branches.
     */
    void ReadAndCreateDataBranches();

    void MatchClusters(
          const TClonesArray* digiMatches,
          const TClonesArray* clusters,
          TClonesArray* clusterMatches);


    /** @brief Match STS clusters, using digi match objects
     ** @param digis   TClonesArray with CbnStsDigi
     ** @param clusters  TClonesArray with CbmStsCluster
     ** @param clusterMatches  TClonesArray with cluster matches (to be filled)
     **
     ** The cluster match is constructed by adding up the digi matches from all
     ** digis belonging to the cluster. In contrast to the method MatchClusters,
     ** the digi match is retrieved from the digi object itself, not from a
     ** separate TClonesArray.
     **/
    void MatchStsClusters(
          const TClonesArray* digis,
          const TClonesArray* clusters,
          TClonesArray* clusterMatches);

    void MatchHits(
          const TClonesArray* matches,
          const TClonesArray* hits,
          TClonesArray* hitMatches);

    /** @brief Match STS hits, using cluster match objects
     ** @param clusterMatches   TClonesArray with cluster matches
     ** @param hits             TClonesArray with CbmStsHit
     ** @param hitMatches       TClonesArray with hit matches (to be filled)
     **
     ** Since a StsHit is constructed from two StsClusters (from front and back
     ** side of a sensor), its match object must also be constructed from the two
     ** match objects corresponding to the clusters. This makes it different from
     ** the method MatchHits, which just copies the cluster match object to the
     ** hit match object.
     */
    void MatchHitsSts(
          const TClonesArray* clusterMmatches,
          const TClonesArray* hits,
          TClonesArray* hitMatches);

    void MatchHitsMvd(
          const TClonesArray* matches,
          const TClonesArray* hits,
          TClonesArray* hitMatches);

    void MatchHitsTof(
          const TClonesArray* DigiPntMatches,
          const TClonesArray* digis,
          const TClonesArray* HitDigiMatches,
          const TClonesArray* hits,
          TClonesArray* hitMatches);

    void MatchHitsTofDigiExp(
          const TClonesArray* DigiPntMatches,
          const TClonesArray* digis,
          const TClonesArray* HitDigiMatches,
          const TClonesArray* hits,
          TClonesArray* hitMatches);

    void MatchHitsToPoints(
          CbmMCDataArray* points,
          const TClonesArray* hits,
          TClonesArray* hitMatches);

    void MatchTracks(
          const TClonesArray* hitMatches,
          CbmMCDataArray* points,
          const TClonesArray* tracks,
          TClonesArray* trackMatches);

    //Special case for STS: now evbased compatible
    void MatchStsTracks(
    	  const TClonesArray* mvdHitMatches,
          const TClonesArray* stsHitMatches,
    	  CbmMCDataArray* mvdPoints,
          CbmMCDataArray* stsPoints,
          const TClonesArray* tracks,
          TClonesArray* trackMatches);
    
    void MatchRichRings(
          const TClonesArray* richRings,
          const TClonesArray* richHits,
          const TClonesArray* richDigis,
          CbmMCDataArray* richMcPoints,
          CbmMCDataArray* mcTracks,
          TClonesArray* ringMatches);

public:
    /**
     * \brief Return McTrack Ids for RICH hit
     * C++11 efficient way to return vector
     */
    static std::vector<std::pair<Int_t, Int_t> > GetMcTrackMotherIdsForRichHit(
            const CbmRichHit* hit,
            const TClonesArray* richDigis,
            CbmMCDataArray* richPoints,
            CbmMCDataArray* mcTracks);
    
    static std::vector<Int_t> GetMcTrackMotherIdsForRichHit(
            const CbmRichHit* hit,
            const TClonesArray* richDigis,
            const TClonesArray* richPoints,
            const TClonesArray* mcTracks);
    
private:
    static Int_t fEventNumber;
    // If MVD hits has to be included in STS track
    Bool_t fIncludeMvdHitsInStsTrack;

    // Pointers to data arrays
    CbmMCDataArray* fMCTracks;  // Monte-Carlo tracks
    
    //TClonesArray* fMCTracksArray;  // Array of MCTracks
    
    // STS
    CbmMCDataArray* fStsPoints; // CbmStsPoint array
    TClonesArray* fStsDigis; // CbmStsDigi array
    TClonesArray* fStsClusters; // CbmStsCluster array
    TClonesArray* fStsHits; // CbmStsHit array
    TClonesArray* fStsTracks; // CbmStsTrack array
    TClonesArray* fStsDigiMatches; // Output CbmMatch array
    TClonesArray* fStsClusterMatches; // Output CbmMatch array
    TClonesArray* fStsHitMatches; // Output CbmMatch array
    TClonesArray* fStsTrackMatches; // Output CbmTrackMatchNew array
    
    //RICH
    TClonesArray* fRichDigis; // CbmRichDigi array
    TClonesArray* fRichHits; // CbmRichHit array
    TClonesArray* fRichRings; // CbmRichRing array
    CbmMCDataArray* fRichMcPoints; // CbmRichRing array
    TClonesArray* fRichTrackMatches; // Output CbmTrackMatchNew array
        

    // TRD
    CbmMCDataArray* fTrdPoints; // CbmTrdPoint array
    TClonesArray* fTrdDigis; // CbmTrdDigi array
    TClonesArray* fTrdClusters; // CbmTrdCluster array
    TClonesArray* fTrdHits; // CbmTrdHit array
    TClonesArray* fTrdTracks; // CbmTrdTrack array
    TClonesArray* fTrdDigiMatches; // Output CbmMatch array
    TClonesArray* fTrdClusterMatches; // Output CbmMatch array
    TClonesArray* fTrdHitMatches; // Output CbmMatch array
    TClonesArray* fTrdTrackMatches; // Output CbmTrackMatchNew array

    // MUCH
    CbmMCDataArray* fMuchPoints; // CbmMuchPoint array
    TClonesArray* fMuchPixelDigis; // CbmMuchDigi array
    TClonesArray* fMuchStrawDigis; // CbmMuchStrawDigi array
    TClonesArray* fMuchClusters; // CbmMuchCluster array
    TClonesArray* fMuchPixelHits; // CbmMuchPixelHit array
    TClonesArray* fMuchStrawHits; // CbmMuchStrawHit array
    TClonesArray* fMuchTracks; // CbmMuchTrack array
    TClonesArray* fMuchPixelDigiMatches; // Output CbmMatch array
    TClonesArray* fMuchStrawDigiMatches; // Output CbmMatch array
    TClonesArray* fMuchClusterMatches; // Output CbmMatch array
    TClonesArray* fMuchPixelHitMatches; // Output CbmMatch array
    TClonesArray* fMuchStrawHitMatches; // Output CbmMatch array
    TClonesArray* fMuchTrackMatches; // Output CbmMatch array

    // MVD
    CbmMCDataArray* fMvdPoints; // CbmMvdPoint array
    TClonesArray* fMvdHits; 		// CbmMvdHit array
    TClonesArray* fMvdDigiMatches;	// CbmMatch array
    TClonesArray* fMvdHitMatches; 	// Output CbmMatch array
    TClonesArray* fMvdCluster;       	// CbmMvdCluster array
    TClonesArray* fMvdClusterMatches;   // Output CbmMatch array

    // TOF
    CbmMCDataArray* fTofPoints; // CbmTofPoint array
    TClonesArray* fTofDigis; 	// CbmTofDigi or CbmTofDigiExp array
    TClonesArray* fTofHits; 		// CbmTofHit array
    Bool_t        fbDigiExpUsed; // Flag true if usage of CbmTofDigiExp detected
    TClonesArray* fTofDigiMatchesPoints;	// CbmMatch array for Digi -> MC
    TClonesArray* fTofDigiMatches;	// CbmMatch array for Hit -> Digi
    TClonesArray* fTofHitMatches; 	// Output CbmMatch array for Hit -> MC
    

    CbmMatchRecoToMC(const CbmMatchRecoToMC&);
    CbmMatchRecoToMC& operator=(const CbmMatchRecoToMC&);

    ClassDef(CbmMatchRecoToMC, 1);
};

#endif /* CBMMATCHRECOTOMC_H_ */
