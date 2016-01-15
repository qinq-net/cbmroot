// Dump()
// gGeoManager->GetListOfNodes()->Print()
#include "../../../../rich/CbmRichHitProducer.h"

void getTransfoMatrix()
{
	fGP = CbmRichHitProducer::InitGeometry();
	fGP.Print();

	TGeoVolume* topVol = gGeoManager->GetTopVolume();

	TGeoNode* node1 = topVol->GetNode("rich1_1");
	TGeoVolume* v1 = node1->GetVolume();
	TGeoNode* node2 = v1->GetNode("RICH_gas_221");
	TGeoVolume* v2 = node2->GetVolume();

	TGeoNode* node3 = v2->GetNode("RICH_mirror_half_total_208");
	TGeoMatrix* matrixMirrorHalf = node3->GetMatrix();
	matrixMirrorHalf->Print();
	TGeoVolume* v3 = node3->GetVolume();

	TGeoNode* node4 = v3->GetNode("RICH_mirror_and_support_belt_strip3_126");
	TGeoVolume* v4 = node4->GetVolume();

//	v4->GetNodes()->Print();

	TGeoNode* node5 = v4->GetNode("RICH_mirror_2_52");

//	TGeoVolume* v5 = node5->GetVolume();
//	v5->Dump();

	TGeoMatrix* matrixMirrorTile = node5->GetMatrix();
	matrixMirrorTile->Print();
/*	Double_t tr1 = matrixMirror1->GetTranslation()[0];
	Double_t tr2 = matrixMirror1->GetTranslation()[1];
	Double_t tr3 = matrixMirror1->GetTranslation()[2];*/

	Double_t rot1 = matrixMirrorHalf->GetRotationMatrix()[0];
	Double_t rot2 = matrixMirrorHalf->GetRotationMatrix()[1];
	Double_t rot3 = matrixMirrorHalf->GetRotationMatrix()[2];
	Double_t rot4 = matrixMirrorHalf->GetRotationMatrix()[3];
	Double_t rot5 = matrixMirrorHalf->GetRotationMatrix()[4];
	Double_t rot6 = matrixMirrorHalf->GetRotationMatrix()[5];
	Double_t rot7 = matrixMirrorHalf->GetRotationMatrix()[6];
	Double_t rot8 = matrixMirrorHalf->GetRotationMatrix()[7];
	Double_t rot9 = matrixMirrorHalf->GetRotationMatrix()[8];

//	std::cout << "Translation: \n" << tr1 << "\t" << tr2 << "\t" << tr3 << std::endl;
	std::cout << "Rotation: \n" << rot1 << "\t" << rot2 << "\t" << rot3 << "\n" << rot4 << "\t" << rot5 << "\t" << rot6 << "\n" << rot7 << "\t" << rot8 << "\t" << rot9 << std::endl;

}
