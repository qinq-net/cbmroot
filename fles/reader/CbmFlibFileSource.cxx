// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                          CbmFlibFileSource                        -----
// -----                    Created 01.11.2013 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmFlibFileSource.h"

#include "FairLogger.h"

#include "TimesliceInputArchive.hpp"

#include <iostream>

CbmFlibFileSource::CbmFlibFileSource()
  : TObject(),
    fFileName(""),
    fSource(NULL)
{
}


CbmFlibFileSource::CbmFlibFileSource(const CbmFlibFileSource& source)
  : TObject(source),
    fFileName(""),
    fSource(NULL)
{
}


CbmFlibFileSource::~CbmFlibFileSource()
{
}

Bool_t CbmFlibFileSource::Init()
{
  if ( 0 == fFileName.Length() ) {
    LOG(FATAL) << "No input file defined." << FairLogger::endl;
  } else {
    // Open the input file
    fSource = new fles::TimesliceInputArchive(fFileName.Data());
  }
}

Int_t CbmFlibFileSource::ReadEvent()
{
}

void CbmFlibFileSource::Close()
{
}

void CbmFlibFileSource::Reset()
{
}


ClassImp(CbmFlibFileSource)
