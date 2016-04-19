/**
 * \file CbmTrdCluster.h
 * \author Florian Uhlig <f.uhlig@gsi.de>
 * \brief Data Container for TRD clusters.
 **/

#ifndef CBMTRDCLUSTER_H
#define CBMTRDCLUSTER_H 

#include "CbmCluster.h"

/**
 * \class CbmTrdCluster
 * \author Florian Uhlig <f.uhlig@gsi.de>
 * \brief Data Container for TRD clusters.
 */
class CbmTrdCluster : public CbmCluster
{
public:

  /**
   * \brief Default constructor.
   */
  CbmTrdCluster();
  CbmTrdCluster(const std::vector<Int_t>& indices, Int_t address);
  /**
   * \brief Destructor.
   */
  virtual ~CbmTrdCluster();


  /** Setters **/
  void SetNCols(UShort_t ncols)  { fNCols=ncols; }
  void SetNRows(UShort_t nrows)  { fNRows=nrows; }

  /** Accessors **/
  UShort_t GetNCols() const { return fNCols; }
  UShort_t GetNRows() const { return fNRows; }

 protected:
  UShort_t fNCols; // number of columns with charge above threshold
  UShort_t fNRows; // number of rows with charge above threshold

  ClassDef(CbmTrdCluster, 3)
};
#endif
