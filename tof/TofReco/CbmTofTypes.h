/**
 * \file CbmTOFTypes.h
 * \author A.Lebedev <andrey.lebedev@gsi.de>
 * \date 2008
 * \brief Typedefs for data structures used in littrack.
 */

#ifndef CBMTOFTYPES_H_
#define CBMTOFTYPES_H_

#include <vector>
using std::vector;
using std::pair;
class CbmTofHit;
class CbmTofTrack;

typedef vector<CbmTofHit*> HitPtrVector;
typedef vector<CbmTofHit*>::iterator HitPtrIterator;
typedef pair<HitPtrIterator, HitPtrIterator> HitPtrIteratorPair;
typedef vector<CbmTofHit*>::const_iterator HitPtrConstIterator;

typedef vector<CbmTofTrack*>::iterator TrackPtrIterator;
typedef vector<CbmTofTrack*> TrackPtrVector;

#endif /* CBMLITTYPES_H_ */
