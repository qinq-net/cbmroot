#ifndef CBMTRDHITPRODUCERCLUSTER_H
#define CBMTRDHITPRODUCERCLUSTER_H 

#include "FairTask.h"

class CbmTrdDigiPar;
class CbmTrdModule;
class CbmTrdHit;
class TClonesArray;
class TF1;
class TGraphErrors;

class CbmTrdHitProducerCluster : public FairTask
{
public:

   CbmTrdHitProducerCluster();

   virtual ~CbmTrdHitProducerCluster();

   virtual InitStatus Init();

   virtual void SetParContainers();

   virtual void Exec(Option_t * option);
   void SetTriangularPads(Bool_t triangles);
private:
   Double_t CalcDisplacement(Double_t Q_left, Double_t Q_center, Double_t Q_right, Double_t padWidth);
   Double_t CalcY(Double_t padWidth, Double_t padHeight, Double_t rect_Dis, Double_t para_Dis, Int_t digiRow);
   void TriangularPadReconstruction(Int_t clusterId);
   /**
   * \brief Reconstruct clusters for the triangular pad geometry
   * \param[in] clusterId cluster position in fClusters array
   * \sa triangularProcessRowCluster() triangularFinalizeModule() 
   * \author A.Bercuci <abercuci@niham.nipne.ro>
   **/
   void TriangularPadReconstructionAB(Int_t clusterId);
   /**
   * \brief Reconstruct hits in one row of the cluster
   * \param[in,out] digiMap signal/time position in cluster row
   * \param[in] cid cluster id from out of which the hit will be produced
   * \sa TriangularGetX() TriangularGetY()
   * \author A.Bercuci <abercuci@niham.nipne.ro>
   **/
   void TriangularProcessRowCluster(std::map<UInt_t, std::pair<Double_t, UChar_t>> &digiMap, Int_t cid);
   /**
   * \brief Reconstruct hits in one row of the cluster
   * \sa TriangularMergeHits()
   * \author A.Bercuci <abercuci@niham.nipne.ro>
   **/
   void TriangularFinalizeModule();
   /**
   * \brief Merge pad row cross clusters
   * \param[in] h0 first hit to be merged
   * \param[in] h1 second hit to be merged
   * \author A.Bercuci <abercuci@niham.nipne.ro>
   **/
   void TriangularMergeHits(CbmTrdHit *h0, CbmTrdHit *h1);
   /**
   * \brief Computes the position of hit along wires for 1 pad row
   * \param[in] n size of the signal array
   * \param[in] sgn signal array
   * \param[in] max maximum position
   * \param[in] an optional anode position (iteration x position reconstruction)
   * \return the offset in pad-width units along wires
   * \author A.Bercuci <abercuci@niham.nipne.ro>
   **/
   Double_t TriangularGetX(Int_t n, Double_t *sgn, Int_t max, Int_t an=-5);
   /**
   * \brief Computes the anode wire for the 1 pad-row hit
   * \param[in] n size of the signal array
   * \param[in] sgn signal array
   * \param[in,out] x at call time position of hit along wires, at output PRF across wires
   * \param[in] dt time profile of hit in 25ns clock units wrt prompt signal
   * \return the position across wires in pad coordinates
   * \author A.Bercuci <abercuci@niham.nipne.ro>
   **/
   Double_t TriangularGetY(Int_t n, Double_t *sgn, Double_t &x, Char_t *dt=NULL) const;
   /**
   * \brief Computes the energy deposit per hit
   * \param[in] n size of the signal array
   * \param[in] sgn signal array
   * \return the dEdx
   * \author A.Bercuci <abercuci@niham.nipne.ro>
   **/
   Double_t TriangularGetEdep(Int_t n, Double_t *sgn) const;
   void CenterOfGravity(
         Int_t clusterId);

   TClonesArray* fDigis; /** Input array of CbmTrdDigi **/
   TClonesArray* fClusters; /** Input array of CbmTrdCluster **/
   TClonesArray* fHits; /** Output array of CbmTrdHit **/

   CbmTrdDigiPar* fDigiPar;
   /// TRD module definition for the current detector beeing processed
   CbmTrdModule*  fModule;        //!
   /// stores raw-wise hits in one TRD module for further merging  
   std::vector<CbmTrdHit*> *fModuleHits; //!
   /// signal distribution / hit 
   TGraphErrors*  fTriangularSignal;  //!
   /// PRF model used for triangular pad  
   TF1*           fTriangularPRF;  //!

   Bool_t fTrianglePads;

   Int_t fRecoTriangular;

   CbmTrdHitProducerCluster(const CbmTrdHitProducerCluster&);
   CbmTrdHitProducerCluster& operator=(const CbmTrdHitProducerCluster&);

   ClassDef(CbmTrdHitProducerCluster,3);
};
#endif
