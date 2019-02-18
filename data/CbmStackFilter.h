/** @file CbmStackFilter.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 12.02.2019
 **/

#ifndef CBMSTACKFILTER_H
#define CBMSTACKFILTER_H 1

#include <map>
#include <vector>
#include "Rtypes.h"
#include "CbmDefs.h"

class TClonesArray;
class TParticle;



/** @class CbmStackFilter
 ** @brief Class for filtering the stack before writing
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 12.02.2019
 ** @version 1.0
 **
 ** During transport simulation, the stack is filled with
 ** primary particles (defined by the generator) and secondary
 ** particles generated by the transport engine. For each
 ** particle, a TParticle object is created. After the event
 ** is finished, the particle objects are converted into
 ** CbmMCTrack objects and written to the respective branch
 ** in the output tree.
 **
 ** Not all of the tracks created during the transport are of
 ** interest for analysis or further processing - e.g., secondary
 ** tracks not registered in any detector. The CbmStackFilter
 ** allows to define selection criteria for the persistent
 ** storage of MCTracks in the output - such as number of MCPoints
 ** in some detector or kinetic energy.
 **
 ** The default behaviour is:
 ** - Particles with at least one point in any detector are stored.
 ** - Primary particles are stored.
 ** - All mother particles of particles to be stored are also stored.
 ** This default behaviour can be modified by the appropriate methods.
 **
 **/

#include <map>

class CbmStackFilter
{

  public:

    typedef std::map<std::pair<Int_t, Int_t>, Int_t> PointMap;


    /** @brief Constructor **/
    CbmStackFilter();


    /** @brief Destructor **/
    virtual ~CbmStackFilter();


    /** @brief Check the stack particles for fulfilling the storage criteria
     ** @param particles TClonesArray of TParticles
     ** @param points    Map holding the number of points in each detector
     ** @value Storage decision for each index in the TClonesArray.
     **
     ** The nPoints map gives access to the number of points of a
     ** particle in a detector. The key is pair<index, detector>,
     ** with the index of the particle in its array detector
     ** the detector identifier (EcbmModuleId).
     ** The return vector must have the same size as the TClonesArray.
     **/
    const std::vector<Bool_t>& Select(const TClonesArray& particles,
                                      const PointMap& points);


    /** @brief Set the minimum kinetic energy
     ** @param minimum  Minimum kinetic energy [GeV]
     **
     ** Tracks kinetic energy less than the minimum will not be stored,
     ** irrespective of their points in the detectors.
     **
     ** The default value is 0., meaning no energy cut.
     **/
    void SetMinEkin(Double_t minimum) {
      fMinEkin = minimum;
    }


    /** @brief Set the minimum number of total MCPoints
     ** @param minimum  Minimum number of points in detector
     **
     ** Tracks with a number of points in all detectors not less than
     ** the minimum will be stored.
     **
     ** The default value is 1, meaning that tracks with at least one
     ** point in any detector are stored.
     **/
    void SetMinNofPoints(UInt_t minimum) {
      fMinNofPointsGlobal = minimum;
    }


    /** @brief Set the minimum number of MCPoints for a given detector
     ** @param detector Detector ID
     ** @param minimum  Minimum number of points in detector
     **
     ** Tracks with a number of points in the detector not less than
     ** the minimum will be stored. In case this cut is defined for
     ** several detector systems, the cuts are connected by logical AND,
     ** i.e., all cuts have to be satisfied.
     **
     ** By default, no detector-specific cut is active.
     **/
    void SetMinNofPoints(ECbmModuleId detector, UInt_t minimum) {
      fMinNofPoints[detector] = minimum;
    }


    /** @brief Set the storage of all mothers of selected tracks
     ** @param choice  If kTRUE, all mothers will be stored.
     **
     ** If activated, all mother tracks of selected tracks will be stored,
     ** irrespective of any other selection criteria. This holds recursively,
     ** i.e. also for grandmothers and grand-grand-mothers etc.
     **
     ** By default, storage of mother tracks is activated.
     **/
    void SetStoreAllMothers(Bool_t choice) {
      fStoreAllMothers = choice;
    }


    /** @brief Set the storage of primary tracks
     ** @param choice  If kTRUE, all primaries will be stored.
     **
     ** If activated, all primary tracks will be stored, irrespective of
     ** any other selection criteria.
     **
     ** By default, storage of all primary tracks is activated.
     **/
    void SetStoreAllPrimaries(Bool_t choice) {
      fStoreAllPrimaries = choice;
    }


  private:

    Bool_t fStoreAllPrimaries;   /// Flag for storage of primaries
    Bool_t fStoreAllMothers;     /// Flag for storage of mothers
    UInt_t fMinNofPointsGlobal;  /// Cut on global number of points
    std::map<ECbmModuleId, UInt_t> fMinNofPoints; /// Cuts on local number of points
    Double_t fMinEkin;           /// Cut on kinetic energy
    std::vector<Bool_t> fStore;  /// Vector with storage decision

    ClassDef(CbmStackFilter,1);

};

#endif /* CBMSTACKFILTER_H */