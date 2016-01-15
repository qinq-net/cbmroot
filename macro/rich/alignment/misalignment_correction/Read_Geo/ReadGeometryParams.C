#include <TGeoManager.h>
#include <TGDMLParse.h>
#include <TMath.h>

#include <map>
#include <string>

/**
 * Use the macro only compiled - this means you have to put plus sign after the macro file name!
 * It reads only GDML file.
 * For example:
 * root -l "ReadGeometryParams.C+(\"../../../../../geometry/rich/alignment/v15_rot/Rich_dec_2015_misalign.gdml\")"
 */

/**
 * Recursive function to fill one std::map with the list of pairs [name] - [node]
 */
void FillMap(TGeoNode* curNode, std::map<std::string, TGeoNode*>& map);

/**
 * Print the filled map into the std::out
 */
void PrintMap(std::map<std::string, TGeoNode*>& map);

/**
 * Reconstruct the hierarchy of the nodes from the bottom to top.
 */
void PrintHierarchy(TString leafNameFilter, std::map<std::string, TGeoNode*>& map, Double_t out_tr[], const Double_t shift[]);

/**
 * User analysis here
 */
void Analyse(std::map<std::string, TGeoNode*>& map);

/**
 * main
 */
void ReadGeometryParams(TString filename)
{
   printf("\n");

   // Import geometry
   TGeoManager *gdml = new TGeoManager("gdml", "FAIRGeom");
   TGDMLParse parser;
   TGeoVolume* gdmlTop = parser.GDMLReadFile(filename);
   gGeoManager->SetTopVolume(gdmlTop);
   gGeoManager->CloseGeometry();

   // Declare the map of nodes
   std::map<std::string, TGeoNode*> mapOfNodes;

   // Get the top node of the geometry
   TGeoNode* topNode = gGeoManager->GetTopNode();

   // Starting from the top node go through the whole setup and fill the map of nodes
   FillMap(topNode, mapOfNodes);

   // Uncomment if you want
//   PrintMap(mapOfNodes);

   // User code there
   Analyse(mapOfNodes);

   // You may export geometry to some other file if you need
/*
   gGeoManager->Export("output/output.gdml");
   gGeoManager->Export("output/output.root");
*/
}

void FillMap(TGeoNode* curNode, std::map<std::string, TGeoNode*>& map)
{
   Int_t nDaughters = curNode->GetNdaughters();
   for (Int_t i=0; i<nDaughters; i++) {
      TGeoNode* curDaughter = curNode->GetDaughter(i);
      std::string cppStrDaughterName(curDaughter->GetName()); // Just to have std::string type and not char* or TString
      // Per se fill the map
      map.insert(std::pair<std::string, TGeoNode*>(cppStrDaughterName, curDaughter));
      // Recursive call for the current node
      FillMap(curDaughter, map);
   }
}

void PrintMap(std::map<std::string, TGeoNode*>& map)
{
   std::map<std::string, TGeoNode*>::const_iterator mapOfNodes_citer;
   for (mapOfNodes_citer = map.begin(); mapOfNodes_citer != map.end(); ++mapOfNodes_citer) {
      TGeoNode* nod = mapOfNodes_citer->second;
      std::cout << mapOfNodes_citer->first << "\t\t" << std::hex << nod << std::dec << ": " << nod->GetMotherVolume()->GetName() << std::endl;
   }
}

/**
 * Assume we know the structure
 * rich1 <-- richContainer <-- rich_gas <-- mirror_full_half <-- mirror_tile_type*
 * Unfortunately reading the geometry from the bottom to the top node is impossble.
 * So we state that we have only one volume and one node named
 * rich1, richContainer, rich_gas.
 * One volume mirror_full_half which is instantiated twice in rich_gas => two nodes.
 * We don't care about the second mirror_full_half node (the one which has y<0) -
 * this is the lower symmetrical half of the mirror, we study ony the upper one.
 * We also know that by default, without any misalignments:
 * richContainer is not rotated in rich1
 * rich_gas is not rotated in richContainer
 * mirror_full_half is not rotated in rich_gas (yes!)
 * mirror_tile_type? are rotated in mirror_full_half only around Z which gives us array in horizontal direction.
 * As for vertical direction we have basically 4 types of tiles. Tiles type 5 and 6 are modified tiles type 4
 * with the hole in the middle to allow the beampipe go through.
 *
 * Now imagine we don't know the full volume hierarchy, we just know that the mirror tile
 * has "mirror_tile" substring in the name.
 * So we try to reconstruct the path from the mirror_tile node to the top.
 * Serious constrain is that there should be not more than one instance of each volume in the geometry.
 * Still possible to filter, say, second instance which is below ZX plane (has Y coord < 0).
 */
void Analyse(std::map<std::string, TGeoNode*>& map)
{
   const Double_t* tr; // 3 components - pointers to some memory which is provided by ROOT
   const Double_t* rot; // 9 components - pointers to some memory which is provided by ROOT

   // Coordinates from CATIA. This are the coordinates of the middle of the tile from the sphere center
   // for different types of the tile.
   const Double_t type0_shift[3] = {0., 0., 0.};
   const Double_t type1_shift[3] = {299.6985489, 0., 13.4454382};
   const Double_t type2_shift[3] = {298.5714116, 0., -29.2423010};
   const Double_t type3_shift[3] = {291.3948410, 0., -71.3375544};
   const Double_t type4_shift[3] = {278.3142433, 0., -111.9874189};

   // Just to use PI in code instead of writing TMath::Pi()
   Double_t PI = TMath::Pi();
   // Useful basic constants
   Double_t RadToDeg = 180./PI;
   Double_t DegToRad = PI/180.;

   // Starting point

   Double_t tr_tile_type1[3] = {0., 0., 0.};
   Double_t tr_tile_type2[3] = {0., 0., 0.};
   Double_t tr_tile_type3[3] = {0., 0., 0.};
   Double_t tr_tile_type4[3] = {0., 0., 0.};

   cout << endl;
   PrintHierarchy(TString("mirror_tile_type3_inter_379"), map, tr_tile_type1, type0_shift);
   PrintHierarchy(TString("mirror_tile_type3_inter_380"), map, tr_tile_type1, type1_shift);
   PrintHierarchy(TString("mirror_tile_type3_inter_381"), map, tr_tile_type1, type1_shift);
   PrintHierarchy(TString("mirror_tile_type3_inter_382"), map, tr_tile_type1, type1_shift);
   PrintHierarchy(TString("mirror_tile_type3_inter_383"), map, tr_tile_type1, type1_shift);
   PrintHierarchy(TString("mirror_tile_type3_inter_384"), map, tr_tile_type1, type1_shift);
   PrintHierarchy(TString("mirror_tile_type3_inter_385"), map, tr_tile_type1, type1_shift);
   PrintHierarchy(TString("mirror_tile_type3_inter_386"), map, tr_tile_type1, type1_shift);
   PrintHierarchy(TString("mirror_tile_type3_inter_387"), map, tr_tile_type1, type1_shift);
   PrintHierarchy(TString("mirror_tile_type3_inter_388"), map, tr_tile_type1, type1_shift);


/*
   PrintHierarchy(TString("mirror_tile_type2"), map, tr_tile_type2);
   PrintHierarchy(TString("mirror_tile_type3"), map, tr_tile_type3);
   PrintHierarchy(TString("mirror_tile_type4"), map, tr_tile_type4);
*/
}

void PrintHierarchy(TString leafNameFilter, std::map<std::string, TGeoNode*>& map, Double_t out_tr[], const Double_t shift[])
{
   Double_t tmpVector1[3];
   tmpVector1[0] = shift[0]; tmpVector1[1] = shift[1]; tmpVector1[2] = shift[2];
   Double_t tmpVector2[3] = {0., 0., 0.};

   TString curMotherName = leafNameFilter;

   // Declarations
   std::map<std::string, TGeoNode*>::const_iterator mapOfNodes_citer;
   Bool_t notDone = kTRUE;
   const Double_t* tr; // 3 components - pointers to some memory which is provided by ROOT

   // for loop inside a do-while loop.
   // Outer loop is needed to go from the leaf up the tree
   // Inner loop is needed to find the mother node for each current node
   // By the way we get full matrix of the tile in the coord system of the top volume (which is rich1)
   // Also taking as input a special vector from CATIA we get the center of the tile

   do {
      notDone = kTRUE;
      for (mapOfNodes_citer = map.begin(); mapOfNodes_citer != map.end(); ++mapOfNodes_citer) {
         // Get current node
         TGeoNode* nod = mapOfNodes_citer->second;
         // Get the name of the current node
         TString curNodeName(nod->GetName());

         if (curNodeName.Contains(curMotherName)) {
            // Extract the matrix of the interesting node
            TGeoMatrix* matr = nod->GetMatrix();
            // Extract translation from the matrix
            tr = matr->GetTranslation();

            //FIXME >= comparison of double - dangerous
//            if (tr[1] >= 0.) // only above XZ plane
            {
               curMotherName = TString(nod->GetMotherVolume()->GetName());
               printf ("%s inside %s\n", nod->GetName(), curMotherName.Data());
//               printf ("Translation: %08f\t%08f\t%08f\n", tr[0], tr[1], tr[2]);
//               matr->Print();

               // Add
               out_tr[0] += tr[0];
               out_tr[1] += tr[1];
               out_tr[2] += tr[2];

               nod->LocalToMaster(tmpVector1, tmpVector2);
               tmpVector1[0] = tmpVector2[0]; tmpVector1[1] = tmpVector2[1]; tmpVector1[2] = tmpVector2[2];

               notDone = kFALSE;
               break;
            }
         }
      }
   } while (!notDone);

   printf("Total translation: %08f\t%08f\t%08f\n", out_tr[0], out_tr[1], out_tr[2]);
   printf("\n");
//   printf ("Center of the tile: %08f\t%08f\t%08f\n", tmpVector1[0]*10., tmpVector1[1]*10., tmpVector1[2]*10.);
//   printf("\n");

}

/*

   // Loop over the map filled in FillMap beforehand
   for (mapOfNodes_citer = map.begin(); mapOfNodes_citer != map.end(); ++mapOfNodes_citer) {

      // Get current node
      TGeoNode* nod = mapOfNodes_citer->second;

      // Get the name of the current node
      TString curNodeName(nod->GetName());

//      std::cout << curNodeName << std::endl;

      // If the name contains smth which is interesting for you do your stuff there
      if (curNodeName.Contains("tile_type1"))
      {
         std::cout << curNodeName << std::endl;

         // Extract the medium of the interesting node
//         TGeoMedium* med = nod->GetMedium();

         // Extract the matrix of the interesting node
         TGeoMatrix* matr = nod->GetMatrix();
         // Extract translation from the matrix
         tr = matr->GetTranslation();

         printf ("Translation: %08f\t%08f\t%08f\n", tr[0], tr[1], tr[2]);



//         matr->Print();


         // Extract the rotation from the matrix
//         rot = matr->GetRotationMatrix();

//         printf ("Rotation:    %08f\t%08f\t%08f\n", rot[0], rot[1], rot[2]);
//         printf ("             %08f\t%08f\t%08f\n", rot[3], rot[4], rot[5]);
//         printf ("             %08f\t%08f\t%08f\n", rot[6], rot[7], rot[8]);

         // Here comes the complicated part.

//         Double_t beta = TMath::ASin(-rot[2])*RadToDeg;          // around Y
//         Double_t alpha1 = TMath::ASin(rot[1]/TMath::Cos(TMath::ASin(-rot[2])))*RadToDeg; // around Z
//         printf ("%08f\n", alpha);
//         Double_t gamma1 = TMath::ASin(rot[5]/TMath::Cos(TMath::ASin(-rot[2])))*RadToDeg; // around X
//         Double_t gamma2 = TMath::ACos(rot[8]/TMath::Cos(TMath::ASin(-rot[2])))*RadToDeg; // around X
//         printf ("%s:\t\t%08f\t\t%08f\t%08f\n", curNodeName.Data(), beta, gamma1, gamma2);

      }

   }

*/
