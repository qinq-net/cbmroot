/**
 * \file CbmLitPtrTypes.h
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2008
 * \brief Typedefs for algorithm interfaces.
 */

#ifndef CBMLITPTRTYPES_H_
#define CBMLITPTRTYPES_H_

#include "interface/CbmLitTrackExtrapolator.h"
#include "interface/CbmLitTrackPropagator.h"
#include "interface/CbmLitTrackFitter.h"
#include "interface/CbmLitTrackSelection.h"
#include "interface/CbmLitTrackUpdate.h"
#include "interface/CbmLitGeoNavigator.h"
#include "interface/CbmLitMaterialEffects.h"
#include "interface/CbmLitTrackFinder.h"
#include "interface/CbmLitHitToTrackMerger.h"

#include <boost/shared_ptr.hpp>
typedef boost::shared_ptr<CbmLitTrackExtrapolator> TrackExtrapolatorPtr;
typedef boost::shared_ptr<CbmLitTrackPropagator> TrackPropagatorPtr;
typedef boost::shared_ptr<CbmLitTrackFitter> TrackFitterPtr;
typedef boost::shared_ptr<CbmLitTrackSelection> TrackSelectionPtr;
typedef boost::shared_ptr<CbmLitTrackUpdate> TrackUpdatePtr;
typedef boost::shared_ptr<CbmLitGeoNavigator> GeoNavigatorPtr;
typedef boost::shared_ptr<CbmLitMaterialEffects> MaterialEffectsPtr;
typedef boost::shared_ptr<CbmLitTrackFinder> TrackFinderPtr;
typedef boost::shared_ptr<CbmLitHitToTrackMerger> HitToTrackMergerPtr;

#endif
