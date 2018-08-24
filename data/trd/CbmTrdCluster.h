/**
 * \file CbmTrdCluster.h
 * \author Florian Uhlig <f.uhlig@gsi.de>
 * \brief Data Container for TRD clusters.
 **/

#ifndef CBMTRDCLUSTER_H
#define CBMTRDCLUSTER_H 

#include "CbmCluster.h"
using namespace std;

/**
 * \class CbmTrdCluster
 * \author Florian Uhlig <f.uhlig@gsi.de>
 * \brief Data Container for TRD clusters.
 */
class CbmTrdCluster : public CbmCluster
{
public:
  enum CbmTrdClusterDef{
    kTriang = BIT(15)     ///< set type of pads on which the cluster is reconstructed
  };
  /**
   * \brief Default constructor.
   */
  CbmTrdCluster();
  CbmTrdCluster(const CbmTrdCluster &ref);
  CbmTrdCluster(const std::vector<Int_t>& indices, Int_t address);
  /**
   * \brief Constructor starting from first digit.
   * \param[in] address global module address 
   * \param[in] idx global digi index in the TClonesArray
   * \param[in] ch RO channel address within the module 
   * \param[in] r module row for the RO channel 
   * \param[in] time relative buffer DAQ time 
   */
  CbmTrdCluster(Int_t address, Int_t idx, Int_t ch, Int_t r, Int_t time);
  /**
   * \brief Destructor.
   */
  virtual ~CbmTrdCluster();

  /** \brief Append digi to cluster
   * \param[in] idx index of digi in TClonesArray
   * \param[in] channel RO channel for digi
   * \param[in] terminator last digi on the cluster (e.g. trigger neighbour)
   */
  void      AddDigi(Int_t idx, Int_t channel=-1, Bool_t terminator=kFALSE);
  /** \brief reset cluster data*/
  void      Clear();
  /** Accessors **/
  UShort_t  GetNCols() const          { return fNCols; }
  UShort_t  GetNRows() const          { return fNRows; }
  UShort_t  GetEndCh() const          { return fStartCh+fNCols-1; }
  UShort_t  GetStartCh() const        { return fStartCh; }
  UShort_t  GetStartTime() const      { return fStartTime; }
  Bool_t    HasTrianglePads() const   { return TestBit(kTriang); }

  /** \brief Query on RO channel list 
   * \param[in] channel RO channel for digi
   * \return -1 before range; 0 in range; 1 after range; -2 cluster empty of digits
   */
  Int_t     IsChannelInRange(Int_t ch) const;
  /** \brief Merge current cluster with info from second 
   * \param[in] second cluster to be added
   * \return success or fail
   */
  Bool_t    Merge(CbmTrdCluster *second);
  /** \brief Initialize basic parameters of the cluster
   * \param[in] address global module address
   * \param[in] row cluster row in the module
   * \param[in] time cluster time in time buffer
   */
  void      ReInit(Int_t address, Int_t row, Int_t time);
  /** Setters **/
  void      SetNCols(UShort_t ncols)  { fNCols=ncols; }
  void      SetNRows(UShort_t nrows)  { fNRows=nrows; }
  void      SetTrianglePads(Bool_t set=kTRUE)  { SetBit(kTriang, set); }
  
  /** \brief Extended functionality*/
  virtual   string ToString() const;
  
 protected:
  UChar_t  fNCols;      // number of columns with charge above threshold
  UChar_t  fNRows;      // number of rows with charge above threshold
  UShort_t fStartCh;    //! channel address of first channel
  UShort_t fStartTime;  //! start time of cluster in clk units wrt buffer start 
  
  ClassDef(CbmTrdCluster, 4)// cluster of digi for the TRD detector 
};
#endif
