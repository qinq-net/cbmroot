/**
 * \file CbmLitPtrTypes.h
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2008
 * \brief Typedefs for algorithm interfaces.
 */

#ifndef CBMLITPTRTYPES_H_
#define CBMLITPTRTYPES_H_

#include "../littrack/std/interface/CbmLitTrackExtrapolator.h"
#include "../littrack/std/interface/CbmLitTrackPropagator.h"
#include "../littrack/std/interface/CbmLitTrackFitter.h"
#include "../littrack/std/interface/CbmLitTrackSelection.h"
#include "../littrack/std/interface/CbmLitTrackUpdate.h"
#include "../littrack/std/interface/CbmLitGeoNavigator.h"
#include "../littrack/std/interface/CbmLitMaterialEffects.h"
#include "../littrack/std/interface/CbmLitTrackFinder.h"
#include "../littrack/std/interface/CbmLitHitToTrackMerger.h"

#ifndef __CINT__
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
#else
template <class T> class shared_ptr;
typedef shared_ptr<CbmLitTrackExtrapolator> TrackExtrapolatorPtr;
typedef shared_ptr<CbmLitTrackPropagator> TrackPropagatorPtr;
typedef shared_ptr<CbmLitTrackFitter> TrackFitterPtr;
typedef shared_ptr<CbmLitTrackSelection> TrackSelectionPtr;
typedef shared_ptr<CbmLitTrackUpdate> TrackUpdatePtr;
typedef shared_ptr<CbmLitGeoNavigator> GeoNavigatorPtr;
typedef shared_ptr<CbmLitMaterialEffects> MaterialEffectsPtr;
typedef shared_ptr<CbmLitTrackFinder> TrackFinderPtr;
typedef shared_ptr<CbmLitHitToTrackMerger> HitToTrackMergerPtr;
#endif

#endif
