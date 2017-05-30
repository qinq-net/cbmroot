/**
 * \file CbmLitMCTrackCreator.h
 * \brief Creates CbmLitMCTrack objects.
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2011
 **/
#ifndef CBMLITMCTRACKCREATOR_H_
#define CBMLITMCTRACKCREATOR_H_

#include "CbmLitMCTrack.h"
#include "CbmStsAddress.h"
#include "CbmDefs.h"
#include <map>
#include <iostream>
#include <assert.h>

class TClonesArray;
class FairMCPoint;
class CbmMvdPoint;
class CbmStsPoint;
class CbmTrdPoint;
class CbmMuchPoint;
class CbmGeoStsPar;
class CbmStsDigiPar;
class CbmRichRingFitterEllipseTau;
class CbmMCDataArray;

/**
 * \class CbmLitMCTrackCreator
 * \brief Creates CbmLitMCTrack objects.
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2011
 **/
class CbmLitMCTrackCreator
{
private:
    /**
     * \brief Constructor.
     */
    CbmLitMCTrackCreator();
    
public:
    /**
     * \brief Destructor.
     */
    virtual ~CbmLitMCTrackCreator();
    
    /**
     * \brief Singleton instance.
     */
    static CbmLitMCTrackCreator* Instance();
    
    /**
     * \brief Creates array of CbmLitMCTracks for current event.
     * Has to be executed in Init() function of the task.
     */
    void CreateMC();
    
    /**
     * \brief Creates array of CbmLitMCTracks for current event.
     * Has to be executed in Exec() function of the task.
     */
    void CreateReco();
    
    /**
     * \brief Check whether a track exists in the array.
     * \param[in] mcId MC index of track.
     * \return true if track exists in array.
     */
    bool TrackExists(
                     int mcEventId,
                     int mcId) const {
        return (fLitMCTracks.count(std::make_pair(mcEventId, mcId)) > 0) ? true : false;
    }
    
    /**
     * \brief Return CbmLitMCTrack by its index.
     * \param[in] mcId MC index of track.
     * \return MC track.
     */
    const CbmLitMCTrack& GetTrack(
                                  int mcEventId,
                                  int mcId) const {
        assert(TrackExists(mcEventId, mcId));
        return fLitMCTracks.find(std::make_pair(mcEventId, mcId))->second;
    }
    
    /**
     * \brief Return number of tracks.
     * \return Number of tracks.
     */
    Int_t GetNofTracks() const {
        return fLitMCTracks.size();
    }
    
private:
    
    /**
     * \brief Read data branches.
     */
    void ReadDataBranches();
    
    /**
     * \brief Add MC points from a certain detector.
     * \param[in] detId Detector identificator.
     * \param[in] array Array of FairMCPoint.
     */
    void AddPoints(
                   ECbmModuleId detId,
                   CbmMCDataArray* array);
    
    /**
     * \brief Calculate and set number of RICH hits for MC track.
     */
    void AddRichHits();
    
    /**
     * \brief Fit Rich MC points using ellipse fitter and fill ellipse parameters.
     */
    void AddRingParameters();
    
    /**
     * \brief Convert FairMCPoint to CbmLitMCPoint.
     * \param[in] fairPoint Pointer to FairMCPoint to be converted.
     * \param[out] litPoint Pointer to output CbmLitMCPoint.
     * \param[in] refId Reference index of the MC point.
     */
    void FairMCPointToLitMCPoint(
                                 const FairMCPoint* fairPoint,
                                 CbmLitMCPoint* litPoint,
                                 int eventId,
                                 int refId,
                                 int stationId);
    
    /**
     * \brief Fill maps for MC points to station id.
     */
    void FillStationMaps();
    
    
    void FairMCPointCoordinatesAndMomentumToLitMCPoint(
                                                       const FairMCPoint* fairPoint,
                                                       CbmLitMCPoint* litPoint);
    
    void MvdPointCoordinatesAndMomentumToLitMCPoint(
                                                    const CbmMvdPoint* mvdPoint,
                                                    CbmLitMCPoint* litPoint);
    
    void StsPointCoordinatesAndMomentumToLitMCPoint(
                                                    const CbmStsPoint* stsPoint,
                                                    CbmLitMCPoint* litPoint);
    
    void TrdPointCoordinatesAndMomentumToLitMCPoint(
                                                    const CbmTrdPoint* trdPoint,
                                                    CbmLitMCPoint* litPoint);
    
    void MuchPointCoordinatesAndMomentumToLitMCPoint(
                                                     const CbmMuchPoint* muchPoint,
                                                     CbmLitMCPoint* litPoint);
    
    CbmMCDataArray* fMCTracks; // CbmMCTrack array
    CbmMCDataArray* fMvdPoints; // CbmMvdPoint array
    CbmMCDataArray* fStsPoints; // CbmStsPoint array
    CbmMCDataArray* fTrdPoints; // CbmTrdPoint array
    CbmMCDataArray* fMuchPoints; // CbmMuchPoint array
    CbmMCDataArray* fTofPoints; // CbmTofPoint array
    CbmMCDataArray* fRichPoints; // CbmRichPoint array
    TClonesArray* fRichHits; // CbmRichHit array
    TClonesArray* fRichDigis; //CbmRichdigi array
    
    // Stores created CbmLitMCTrack objects.
    // std::map<MC track index, CbmLitMCTrack object>.
    std::map<std::pair<int, int>, CbmLitMCTrack> fLitMCTracks;
    
    // Map <MC point index, station index>
    std::map<std::pair<int, int>, int>fMvdStationsMap; // for MVD
    std::map<std::pair<int, int>, int>fStsStationsMap; // for STS
    std::map<std::pair<int, int>, int>fTrdStationsMap; // for TRD
    std::map<std::pair<int, int>, int>fMuchStationsMap; // for MUCH
    
    CbmRichRingFitterEllipseTau* fTauFit; // Ellipse fitter algorithm
    
    CbmLitMCTrackCreator(const CbmLitMCTrackCreator&);
    CbmLitMCTrackCreator operator=(const CbmLitMCTrackCreator&);
};

#endif /* CBMLITMCTRACKCREATOR_H_ */
