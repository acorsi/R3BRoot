// -------------------------------------------------------------------------
// -----                        R3BCalo source file                    -----
// -----                  Created 26/03/09  by D.Bertini               -----
// -----      Last modification 28/03/11 by H.Alvarez        -----
// -------------------------------------------------------------------------
#include "R3BCalo.h"

#include "R3BGeoCalo.h"
#include "R3BCaloPoint.h"
#include "R3BCaloCrystalHit.h"
#include "R3BGeoCaloPar.h"

#include "FairGeoInterface.h"
#include "FairGeoLoader.h"
#include "FairGeoNode.h"
#include "FairGeoRootBuilder.h"
#include "FairRootManager.h"
#include "FairStack.h"
#include "FairRuntimeDb.h"
#include "FairRun.h"
#include "FairVolume.h"

#include "TClonesArray.h"
#include "TGeoMCGeometry.h"
#include "TParticle.h"
#include "TVirtualMC.h"
#include "TObjArray.h"

// includes for modeling
#include "TGeoManager.h"
#include "TParticle.h"
#include "TVirtualMC.h"
#include "TGeoMatrix.h"
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoBBox.h"
#include "TGeoPgon.h"
#include "TGeoSphere.h"
#include "TGeoArb8.h"
#include "TGeoCone.h"
#include "TGeoTube.h"
#include "TGeoBoolNode.h"
#include "TGeoCompositeShape.h"
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;



// -----   Default constructor   -------------------------------------------
R3BCalo::R3BCalo() : R3BDetector("R3BCalo", kTRUE, kCALIFA)
{
  ResetParameters();
  fCaloCollection = new TClonesArray("R3BCaloPoint");
  fCaloCrystalHitCollection = new TClonesArray("R3BCaloCrystalHit");
  fPosIndex = 0;
  kGeoSaved = kFALSE;
  flGeoPar = new TList();
  flGeoPar->SetName( GetName());
  fVerboseLevel = 1;
  fNonUniformity = 0.;
  fGeometryVersion = 1;
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
R3BCalo::R3BCalo(const char* name, Bool_t active)
    : R3BDetector(name, active, kCALIFA)
{
  ResetParameters();
  fCaloCollection = new TClonesArray("R3BCaloPoint");
  fCaloCrystalHitCollection = new TClonesArray("R3BCaloCrystalHit");
  fPosIndex = 0;
  kGeoSaved = kFALSE;
  flGeoPar = new TList();
  flGeoPar->SetName( GetName());
  fVerboseLevel = 1;
  fNonUniformity = 0.;
  fGeometryVersion = 1;
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
R3BCalo::~R3BCalo()
{

  if ( flGeoPar ) delete flGeoPar;
  if (fCaloCollection) {
    fCaloCollection->Delete();
    delete fCaloCollection;
  }
  if (fCaloCrystalHitCollection) {
    fCaloCrystalHitCollection->Delete();
    delete fCaloCrystalHitCollection;
  }
}
// -------------------------------------------------------------------------
void R3BCalo::Initialize()
{
  FairDetector::Initialize();

  cout << endl;
  cout << "-I- R3BCalo: initialisation" << endl;
  cout << "-I- R3BCalo: Vol (McId) def." << endl;

  Char_t buffer[126];

  //Taken into different geometries during the initialization phase:
  // 0 - OLD CALIFA 5.0, including BARREL and ENDCAP:
  //   NOTE: THERE IS NO WARRANTY THAT THIS VERSION WORKS CORRECTLY IN R3BROOT
  //   Contains 30 different crystal types, repeated 64 times (64 copies)
  //
  // 1- CALIFA 7.05, only BARREL
  //   Each ring is made of 40 alveoli of 4 crystals each. There are 24 alveoli along the polar angle
  //   for a total of 40x24=960 alveoli or 3840 crystals. There are 12 different crystal shapes:
  //     @alveoliType=(1,1,2,2,2,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,6,6,6);
  //
  //     Volumes: Alveolus_[1,24] made of CrystalWithWrapping_[1,6][A,B] made of Crystal_[1,6][A,B]
  //
  // 2- CALIFA 7.07, only BARREL
  //   Each ring is made of 32 alveoli of 4 crystals each. There are 20 alveoli along the polar angle
  //   for a total of 32x20=640 alveoli or 2560 crystals. There are 16 different crystal shapes:
  //     @alveoliType=(1,1,2,2,2,3,3,4,4,4,5,5,6,6,6,7,7,7,8,8);
  //
  //     Volumes: Alveolus_[1,20] made of CrystalWithWrapping_[1,8][A,B] made of Crystal_[1,8][A,B]
  //
  // 3- CALIFA 7.09, only BARREL
  //   The first 16 rings are made of 32 alveoli of 4 crystals each. The last 3 rings are made of 32
  //   alveoli of 1 crystal each. There are 19 alveoli along the polar angle for a total of 32x19=608
  //   alveoli or 2144 crystals. There are 11 different crystal shapes:
  //     @alveoliType[19]={1,1,2,2,3,3,3,3,3,3,4,4,4,5,5,5,6,6,6};
  //
  //
  // 4- CALIFA 7.17, only ENDCAP (in CsI[Tl])
  //   Each ring is made of 32 alveoli of 8, 8 and 7 crystals each. There are 3 alveoli along the polar angle
  //   for a total of 32x3=96 alveoli or 736 crystals. There are 23 different crystal shapes:
  //     @alveoliType=(8,8,7);
  //
  //     Volumes: Alveolus_EC_[1,3] made of CrystalWithWrapping_[1,23] made of Crystal_[1,23]
  //
  // 5- CALIFA 7.07+7.17
  //   See above the two components
  //
  // 6- CALIFA 7.09+7.17
  //   See above the two components
  //
  // 10- CALIFA 8.11, only BARREL
	//   The first 15 rings are made of 32 alveoli of 4 crystals each. The last ring are made of 32
  //   alveoli of 1 crystal each. There are 16 alveoli along the polar angle for a total of 32x16=512
  //   alveoli and 32x15x4+32=1952 crystals. There are 11 (actually 5x2+1) different crystal shapes:
	//


  //HAPOL TODO -> Check the VolId() datamember to assign dinamically different crystal logical
  //  volumenes in different geometries. Setting now a fixed number, maybe not using all
  for (Int_t i=0; i<30; i++ ) {
    sprintf(buffer,"crystalLog%i",i+1);
    cout << "-I- R3BCalo: Crystal Nb   : " << i << " connected to (McId) ---> " <<  gMC->VolId(buffer)<< endl;
    fCrystalType[i] = gMC->VolId(buffer);
  }
  for (Int_t i=0; i<32; i++ ) {  //32 is the larger possible alveolus number (v7.05) in barrel
    sprintf(buffer,"Alveolus_%i",i+1);
    cout << "-I- R3BCalo: Alveolus_ Nb   : " << i+1 << " connected to (McId) ---> " <<  gMC->VolId(buffer)<< endl;
    fAlveolusType[i] = gMC->VolId(buffer);
  }
  for (Int_t i=0; i<3; i++ ) { //3 is the larger possible alveolus number (v7.17) in endcap
    sprintf(buffer,"Alveolus_EC_%i",i+1);
    cout << "-I- R3BCalo: Alveolus_EC Nb   : " << i+1 << " connected to (McId) ---> " <<  gMC->VolId(buffer)<< endl;
    fAlveolusECType[i] = gMC->VolId(buffer);
  }

  TGeoVolume *vol = gGeoManager->GetVolume("CalifaWorld");
  vol->SetVisibility(kFALSE);


}


void R3BCalo::SetSpecialPhysicsCuts()
{

  cout << endl;

  cout << "-I- R3BCalo: Adding customized Physics cut ... " << endl;
  cout << "-I- R3BCalo: Yet not implemented !... " << endl;

  cout << endl;

}




// -----   Public method ProcessHits  --------------------------------------
Bool_t R3BCalo::ProcessHits(FairVolume* vol)
{

  // Getting the Infos from Crystal Volumes
  Int_t cp1 = -1; Int_t volId1 = -1; Int_t cpAlv = -1; Int_t cpSupAlv = -1; Int_t volIdAlv = -1; Int_t volIdSupAlv = -1; Int_t cpCry = -1; Int_t volIdCry = -1;
  // Crystals Ids
  int crysNum;
  const char* bufferName = gMC->CurrentVolName();
  volId1 =  gMC->CurrentVolID(cp1);
  volIdCry =  gMC->CurrentVolOffID(1,cpCry);
  volIdAlv =  gMC->CurrentVolOffID(2,cpAlv);
  volIdSupAlv =  gMC->CurrentVolOffID(3,cpSupAlv); //needed for versions 8.# and later

  Int_t crystalType = 0;
  Int_t crystalCopy = 0;
  Int_t crystalId = 0;

  if (fGeometryVersion==0) {
    //The present scheme here done works nicely with 5.0
    // crystalType = crystal type (from 1 to 30)
    // crystalCopy = crystal copy (from 1 to 512 for crystal types from 1 to 6 (BARREL),
    //                 from 1 to 64 for crystal types from 7 to 30 (ENDCAP))
    // crystalId = (crystal type-1) *512 + crystal copy  (from 1 to 3072) for the BARREL
    // crystalId = 3072 + (crystal type-7) *64 + crystal copy  (from 3073 to 4608) for the ENDCAP
    //
    crystalType = GetCrystalType(volId1);
    //cout << " CHECKKKKKK ____ crystal Type "<< crystalType << endl;
    crystalCopy = cp1 + 1;
    //cout << " CHECKKKKKK ____ crystal Copy "<< crystalCopy << endl;
    if (crystalType<7) {//del 1 al 6 hay 512 de cada tipo; del 7 al 30 solo hay 64 de cada tipo
      crystalId = (crystalType-1)*512+crystalCopy;
    } else if (crystalType<31) {
      crystalId = 3072+(crystalType-7)*64+crystalCopy;
    } else cout << "-E- R3BCalo: Impossible crystalType for geometry 5.0" << endl;
    //cout << " CHECKKKKKK ____ crystal iD "<< crystalId << endl;
  } else if (fGeometryVersion==1)  {
    //The present scheme here done works nicely with 7.05
    // crystalType = alveolus type (from 1 to 24)   [Basically the alveolus number]
    // crystalCopy = (alveolus copy - 1) * 4 + crystals copy (from 1 to 160)  [Not exactly azimuthal]
    // crystalId = (alveolus type-1)*160 + (alvelous copy-1)*4 + (crystal copy)  (from 1 to 3840)
    //        crystalID is asingle identifier per crystal!
    crystalType = GetAlveolusType(volIdAlv);
    crystalCopy = cpAlv * 4 + cpCry;
    crystalId = (crystalType-1)*160 + cpAlv * 4 + cpCry;
    if (crystalType>24 || crystalType<1 || crystalCopy>160 || crystalCopy<1 || crystalId>3840 || crystalId<1)
      cout << "-E- R3BCalo: Wrong crystal number in geometryVersion 1. " << endl;
  } else if (fGeometryVersion==2)  {
    //The present scheme here done works nicely with 7.07
    // crystalType = alveolus type (from 1 to 20)   [Basically the alveolus number]
    // crystalCopy = (alveolus copy - 1) * 4 + crystals copy (from 1 to 128)  [Not exactly azimuthal]
    // crystalId = (alveolus type-1)*128 + (alvelous copy-1)*4 + (crystal copy)  (from 1 to 2560)
    crystalType = GetAlveolusType(volIdAlv);
    crystalCopy = cpAlv * 4 + cpCry;
    crystalId = (crystalType-1)*128 + cpAlv * 4 + cpCry;
    if (crystalType>20 || crystalType<1 || crystalCopy>128 || crystalCopy<1 || crystalId>2560 || crystalId<1)
      cout << "-E- R3BCalo: Wrong crystal number in geometryVersion 2. " << endl;
  } else if (fGeometryVersion==3) {
    //The present scheme here done works with 7.09
    if (GetAlveolusType(volIdAlv)<17) {
      crystalType = GetAlveolusType(volIdAlv);
      crystalCopy = cpAlv * 4 + cpCry;
      crystalId = (crystalType-1)*128 + cpAlv * 4 + cpCry;
    }
    // Crystaltypes  17-19 are large crystals which fill type 6 alveoli, as opposed to the smaller       crystals of which 4 fit in the other alveoli.
    else if (GetAlveolusType(volIdAlv)>16&&GetAlveolusType(volIdAlv)<20) {
      crystalType = GetAlveolusType(volIdAlv);
      crystalCopy = cpAlv + cpCry;
      crystalId = (crystalType-1)*32 + cpAlv + cpCry;
    }
    if (crystalType>19 || crystalType<1 || crystalCopy>128 || crystalCopy<1 || crystalId>2144 || crystalId<1)
      cout << "-E- R3BCalo: Wrong crystal number in geometryVersion 3. " << endl;
  } else if (fGeometryVersion==4) {
    //The present scheme here done works nicely with 7.17
    // crystalType = crystals type (from 1 to 23)
    // crystalCopy = alveolus copy (from 1 to 32)
    // crystalId = 3000 + (alvelous copy-1)*23 + (crystal copy-1)  (from 3000 to 3736)
    if ( GetAlveolusECType(volIdAlv) !=-1 ) {
      sscanf(bufferName,"%*8c %d",&crysNum);
      crystalType = crysNum;
      //crystalType = cpCry+1;
      crystalCopy = cpAlv+1;
      crystalId = 3000 + cpAlv*23 + (crystalType-1);
      if (crystalType>23 || crystalType<1 || crystalCopy>32 || crystalCopy<1 || crystalId<3000 || crystalId>3736)
        cout << "-E- R3BCalo: Wrong crystal number in geometryVersion 4. " << endl;
    } else cout << "-E- R3BCalo: Wrong alveolus volume in geometryVersion 4. " << endl;
  } else if (fGeometryVersion==5) {
    //The present scheme here done works nicely with 7.07+7.17
    //see the explanation for geometries 2 and 4
    if (GetAlveolusType(volIdAlv)!=-1) {
      crystalType = GetAlveolusType(volIdAlv);
      crystalCopy = cpAlv * 4 + cpCry;
      crystalId = (crystalType-1)*128 + cpAlv * 4 + cpCry;
      if (crystalType>20 || crystalType<1 || crystalCopy>128 || crystalCopy<1 || crystalId>2560 || crystalId<1)
        cout << "-E- R3BCalo: Wrong crystal number in geometryVersion 5 (barrel). " << endl;
    } else {
      sscanf(bufferName,"%*8c %d",&crysNum);
      crystalType = crysNum;
      //crystalType = cpCry+1;
      crystalCopy = cpAlv+1;
      crystalId = 3000 + cpAlv*23 + (crystalType-1);
      if (crystalType>23 || crystalType<1 || crystalCopy>32 || crystalCopy<1 || crystalId<3000 || crystalId>3736)
        cout << "-E- R3BCalo: Wrong crystal number in geometryVersion 5 (endcap). " << endl;
    }
  } else if (fGeometryVersion==6) {
    //The present scheme here done works with 7.09+7.17
    //see the explanation for geometries 3 and 4
    if (GetAlveolusType(volIdAlv)!=-1) {
      if (GetAlveolusType(volIdAlv)<17) {
        crystalType = GetAlveolusType(volIdAlv);
        crystalCopy = cpAlv * 4 + cpCry;
        crystalId = (crystalType-1)*128 + cpAlv * 4 + cpCry;
      }
      // Crystaltypes 17-19 are large crystals which fill type 6 alveoli, as opposed to the smaller crystals of which 4 fit in the other alveoli.
      else if (GetAlveolusType(volIdAlv)>16&&GetAlveolusType(volIdAlv)<20) {
        crystalType = GetAlveolusType(volIdAlv);
        crystalCopy = cpAlv + cpCry;
        crystalId = (crystalType-1)*32 + cpAlv + cpCry;
      }

      if (crystalType>19 || crystalType<1 || crystalCopy>128 || crystalCopy<1 || crystalId>2144 || crystalId<1)
        cout << "-E- R3BCalo: Wrong crystal number in geometryVersion 6 (Barrel). " << "\n" <<"crystalType = "<< crystalType << " crystalCopy = " << crystalCopy <<" crystalId = "<< crystalId <<endl;
    } else {
      sscanf(bufferName,"%*8c %d",&crysNum);
      crystalType = crysNum;
      //crystalType = cpCry+1;
      crystalCopy = cpAlv+1;
      crystalId = 3000 + cpAlv*23 + (crystalType-1);
      if (crystalType>23 || crystalType<1 || crystalCopy>32 || crystalCopy<1 || crystalId<3000 || crystalId>3736)
        cout << "-E- R3BCalo: Wrong crystal number in geometryVersion 6 (endcap). " << endl;
    }
  } else if (fGeometryVersion==10) {
    //The present scheme here done works with 8.11
    // crystalType = alveolus type (from 1 to 17)   [Basically the alveolus number]
    // crystalCopy = alveolus copy * 4 + crystals copy +1 (from 1 to 128)  [Not exactly azimuthal]
    // crystalId = 1 to 32 for the first 32 crystals (single crystal in each alveoli)
		//             32 + (alveolus type-2)*128 + (alvelous copy)*4 + (crystal copy) + 1        (from 1 to 1952)
		//
		//cout << "volIdSupAlv" << volIdSupAlv <<" "<<GetAlveolusType(volIdSupAlv)<<endl;
		if (GetAlveolusType(volIdSupAlv)==1) {
			crystalType = GetAlveolusType(volIdSupAlv); //note that there one level more (alveolusInner)
			crystalCopy = cpSupAlv+1;                   //only one crystal per alveoli in this ring, running from 1 to 32
			crystalId = cpSupAlv+1;                     //only one crystal per alveoli in this ring, running from 1 to 32
		}
		else if (GetAlveolusType(volIdSupAlv)>1 && GetAlveolusType(volIdSupAlv)<17) {
			crystalType = GetAlveolusType(volIdSupAlv);      //note that there one level more (alveolusInner)
			crystalCopy = cpSupAlv*4+cpCry+1;  			         //running from 0*4+0+1=1 to 31*4+3+1=128 
			crystalId = 32+(crystalType-2)*128+cpSupAlv*4+cpCry+1; //running from 32+0*128+0*4+0+1=1 to 32+14*128+31*4+3+1=1952
		}
		if (crystalType>16 || crystalType<1 || crystalCopy>128 || crystalCopy<1 || crystalId>1952 || crystalId<1)
			cout << "-E- R3BCalo: Wrong crystal number in geometryVersion 10. " << endl;
  } else cout << "-E- R3BCalo: Geometry version not available in R3BCalo::ProcessHits(). " << endl;
	
  if (fVerboseLevel>1)
    cout << "-I- R3BCalo: Processing Points in Alveolus Nb " << volIdAlv << ", copy Nb " << cpAlv
         << ", crystal copy Nb " << cpCry << " and unique crystal identifier " << crystalId << endl;

  if ( gMC->IsTrackEntering() ) {
    fELoss  = 0.;
    fTime   = gMC->TrackTime() * 1.0e09;
    fLength = gMC->TrackLength();
    gMC->TrackPosition(fPosIn);
    gMC->TrackMomentum(fMomIn);
  }

  // Sum energy loss for all steps in the active volume
  fELoss += gMC->Edep();

  // Set additional parameters at exit of active volume. Create R3BCaloPoint.
  if ( gMC->IsTrackExiting()    ||
       gMC->IsTrackStop()       ||
       gMC->IsTrackDisappeared()   ) {
    fTrackID  = gMC->GetStack()->GetCurrentTrackNumber();
    fVolumeID = vol->getMCid();
    gMC->TrackPosition(fPosOut);
    gMC->TrackMomentum(fMomOut);
    if (fELoss == 0. ) return kFALSE;

    if (gMC->IsTrackExiting()) {
      const Double_t* oldpos;
      const Double_t* olddirection;
      Double_t newpos[3];
      Double_t newdirection[3];
      Double_t safety;

      gGeoManager->FindNode(fPosOut.X(),fPosOut.Y(),fPosOut.Z());
      oldpos = gGeoManager->GetCurrentPoint();
      olddirection = gGeoManager->GetCurrentDirection();

//cout << "1st direction: " << olddirection[0] << "," << olddirection[1] << "," << olddirection[2] << endl;

      for (Int_t i=0; i<3; i++) {
        newdirection[i] = -1*olddirection[i];
      }

      gGeoManager->SetCurrentDirection(newdirection);
//TGeoNode *bla = gGeoManager->FindNextBoundary(2);
      safety = gGeoManager->GetSafeDistance();

      gGeoManager->SetCurrentDirection(-newdirection[0],-newdirection[1],-newdirection[2]);

      for (Int_t i=0; i<3; i++) {
        newpos[i] = oldpos[i] - (3*safety*olddirection[i]);
      }

      if ( fPosIn.Z() < 30. && newpos[2] > 30.02 ) {
        cerr << "2nd direction: " << olddirection[0] << "," << olddirection[1] << "," << olddirection[2]
             << " with safety = " << safety << endl;
        cerr << "oldpos = " << oldpos[0] << "," << oldpos[1] << "," << oldpos[2] << endl;
        cerr << "newpos = " << newpos[0] << "," << newpos[1] << "," << newpos[2] << endl;
      }

      fPosOut.SetX(newpos[0]);
      fPosOut.SetY(newpos[1]);
      fPosOut.SetZ(newpos[2]);
    }

    AddHit(fTrackID, fVolumeID, crystalType , crystalCopy , crystalId,
           TVector3(fPosIn.X(),   fPosIn.Y(),   fPosIn.Z()),
           TVector3(fPosOut.X(),  fPosOut.Y(),  fPosOut.Z()),
           TVector3(fMomIn.Px(),  fMomIn.Py(),  fMomIn.Pz()),
           TVector3(fMomOut.Px(), fMomOut.Py(), fMomOut.Pz()),
           fTime, fLength, fELoss);

    // Increment number of CaloPoints for this track
    FairStack* stack = (FairStack*) gMC->GetStack();
    stack->AddPoint(kCALIFA);

    //Adding a crystalHit support
    Int_t nCrystalHits = fCaloCrystalHitCollection->GetEntriesFast();
    Bool_t existHit = 0;

    if (nCrystalHits==0) AddCrystalHit(crystalType , crystalCopy , crystalId, NUSmearing(fELoss), fTime);
    else {
      for (Int_t i=0; i<nCrystalHits; i++) {
        if ( ((R3BCaloCrystalHit *)(fCaloCrystalHitCollection->At(i)))->GetCrystalId() == crystalId ) {
          ((R3BCaloCrystalHit *)(fCaloCrystalHitCollection->At(i)))->AddMoreEnergy(NUSmearing(fELoss));
          if ( ((R3BCaloCrystalHit *)(fCaloCrystalHitCollection->At(i)))->GetTime() > fTime ) {
            ((R3BCaloCrystalHit *)(fCaloCrystalHitCollection->At(i)))->SetTime(fTime);
          }
          existHit=1; //to avoid the creation of a new CrystalHit
          break;
        }
      }
      if (!existHit) AddCrystalHit(crystalType , crystalCopy , crystalId, NUSmearing(fELoss), fTime);
    }

    existHit=0;

    ResetParameters();
  }

  return kTRUE;
}
// ----------------------------------------------------------------------------
//void R3BCalo::SaveGeoParams(){
//
//  cout << " -I Save STS geo params " << endl;
//
//  TFolder *mf = (TFolder*) gDirectory->FindObjectAny("cbmroot");
//  cout << " mf: " << mf << endl;
//  TFolder *stsf = NULL;
//  if (mf ) stsf = (TFolder*) mf->FindObjectAny(GetName());
//  cout << " stsf: " << stsf << endl;
//  if (stsf) stsf->Add( flGeoPar0 ) ;
//  FairRootManager::Instance()->WriteFolder();
//  mf->Write("cbmroot",TObject::kWriteDelete);
//}


// -----   Public method EndOfEvent   -----------------------------------------
void R3BCalo::BeginEvent()
{

//  if (! kGeoSaved ) {
//      SaveGeoParams();
//  cout << "-I STS geometry parameters saved " << endl;
//  kGeoSaved = kTRUE;
//  }

}
// -----   Public method EndOfEvent   -----------------------------------------
void R3BCalo::EndOfEvent()
{

  /*
    Int_t nHits = fCaloCollection->GetEntriesFast();
    if(nHits>0) {
      //Int_t* detectorIdArray = new Int_t[nHits];
      Int_t* takeHitFromPointArray = new Int_t[nHits];
      Int_t* crystalTypesArray = new Int_t[nHits];
      Int_t* crystalCopyArray = new Int_t[nHits];
      Int_t* crystalIdArray = new Int_t[nHits];
      Double_t* crystalEnergyArray = new Double_t[nHits];
      Double_t* crystalTimeArray = new Double_t[nHits];
      Double_t* accumulatedEnergyArray = new Double_t[nHits];

      for(Int_t i=0;i<nHits;i++){
      //detectorIdArray[i] = ((R3BCaloPoint *)(fCaloCollection->At(i)))->GetDetectorID();
      crystalTypesArray[i] = ((R3BCaloPoint *)(fCaloCollection->At(i)))->GetCrystalType();
      crystalCopyArray[i] = ((R3BCaloPoint *)(fCaloCollection->At(i)))->GetCrystalCopy();
      crystalIdArray[i] = ((R3BCaloPoint *)(fCaloCollection->At(i)))->GetCrystalId();
      crystalEnergyArray[i] = ((R3BCaloPoint *)(fCaloCollection->At(i)))->GetEnergyLoss();
      crystalTimeArray[i] = ((R3BCaloPoint *)(fCaloCollection->At(i)))->GetTime();
      accumulatedEnergyArray[i] = ((R3BCaloPoint *)(fCaloCollection->At(i)))->GetEnergyLoss();
      takeHitFromPointArray[i] = 0;
      }


      if (fVerboseLevel>1)
        for(Int_t i=0;i<nHits;i++){
        cout << "  TEST Points " << crystalTypesArray[i] << "  "
           << crystalCopyArray[i] << "  " << crystalEnergyArray[i]<< "  " << crystalTimeArray[i] << endl;
        }

      takeHitFromPointArray[0] = 1;

      for (Int_t i=1;i<nHits;i++) {
        for(Int_t j=0;j<i;j++){
        if(crystalIdArray[i]==crystalIdArray[j]){
        accumulatedEnergyArray[j] += crystalEnergyArray[i];
        if(crystalTimeArray[i]<crystalTimeArray[j]) crystalTimeArray[j]=crystalTimeArray[i];
        }
        else takeHitFromPointArray[i] = 1;
      }
      }

      for (Int_t i=1;i<nHits;i++) {
      if(takeHitFromPointArray[i]==1)
        AddCrystalHit(crystalTypesArray[i], crystalCopyArray[i], crystalIdArray[i],
              accumulatedEnergyArray[i], crystalTimeArray[i]);
      }
    }
    */

  if (fVerboseLevel) Print();

  fCaloCollection->Clear();
  fCaloCrystalHitCollection->Clear();

  ResetParameters();
}
// ----------------------------------------------------------------------------



// -----   Public method Register   -------------------------------------------
void R3BCalo::Register()
{
  //FairRootManager::Instance()->Register("CrystalPoint", GetName(), fCaloCollection, kTRUE);
  FairRootManager::Instance()->Register("CrystalHit", GetName(), fCaloCrystalHitCollection, kTRUE);

}
// ----------------------------------------------------------------------------



// -----   Public method GetCollection   --------------------------------------
TClonesArray* R3BCalo::GetCollection(Int_t iColl) const
{
  //HAPOL TODO -- DO I NEED TO RETURN A fCaloCrystalHitColletion????
  //if (iColl == 0) return fCaloCollection;
  if (iColl == 0) return fCaloCrystalHitCollection;
  else return NULL;
}
// ----------------------------------------------------------------------------



// -----   Public method Print   ----------------------------------------------
void R3BCalo::Print() const
{
  Int_t nHits = fCaloCollection->GetEntriesFast();
  cout << "-I- R3BCalo: " << nHits << " points registered in this event."
       << endl;
  Int_t nCrystalHits = fCaloCrystalHitCollection->GetEntriesFast();
  cout << "-I- R3BCalo: " << nCrystalHits << " hits registered in this event."
       << endl;
}
// ----------------------------------------------------------------------------



// -----   Public method Reset   ----------------------------------------------
void R3BCalo::Reset()
{
  fCaloCollection->Clear();
  fCaloCrystalHitCollection->Clear();
  ResetParameters();
}
// ----------------------------------------------------------------------------



// -----   Public method CopyClones   -----------------------------------------
void R3BCalo::CopyClones(TClonesArray* cl1, TClonesArray* cl2, Int_t offset)
{
  Int_t nEntries = cl1->GetEntriesFast();
  cout << "-I- R3BCalo: " << nEntries << " entries to add." << endl;
  TClonesArray& clref = *cl2;
  R3BCaloPoint* oldpoint = NULL;
  for (Int_t i=0; i<nEntries; i++) {
    oldpoint = (R3BCaloPoint*) cl1->At(i);
    Int_t index = oldpoint->GetTrackID() + offset;
    oldpoint->SetTrackID(index);
    new (clref[fPosIndex]) R3BCaloPoint(*oldpoint);
    fPosIndex++;
  }
  cout << " -I- R3BCalo: " << cl2->GetEntriesFast() << " merged entries."
       << endl;
}

// -----   Private method AddHit   --------------------------------------------
R3BCaloPoint* R3BCalo::AddHit(Int_t trackID, Int_t detID, Int_t volid , Int_t copy, Int_t ident,
                              TVector3 posIn, TVector3 posOut, TVector3 momIn, TVector3 momOut,
                              Double_t time, Double_t length, Double_t eLoss)
{
  TClonesArray& clref = *fCaloCollection;
  Int_t size = clref.GetEntriesFast();
  if (fVerboseLevel>1)
    cout << "-I- R3BCalo: Adding Point at (" << posIn.X() << ", " << posIn.Y()
         << ", " << posIn.Z() << ") cm,  detector " << detID << ", track "
         << trackID << ", energy loss " << eLoss*1e06 << " keV" << endl;
  return new(clref[size]) R3BCaloPoint(trackID, detID, volid, copy , ident, posIn, posOut,
                                       momIn, momOut, time, length, eLoss);
}

// -----   Private method AddCrystalHit   --------------------------------------------
R3BCaloCrystalHit* R3BCalo::AddCrystalHit(Int_t type, Int_t copy, Int_t ident,
    Double_t energy, Double_t time)
{
  TClonesArray& clref = *fCaloCrystalHitCollection;
  Int_t size = clref.GetEntriesFast();
  if (fVerboseLevel>1)
    cout << "-I- R3BCalo: Adding Hit in detector type " << type << ", and copy " << copy
         << " with unique identifier " << ident << ", depositing " << energy*1e06 << " keV" << endl;
  return new(clref[size]) R3BCaloCrystalHit(type, copy, ident, energy, time);
}

// -----   Private method NUSmearing  --------------------------------------------
Double_t R3BCalo::NUSmearing(Double_t inputEnergy)
{
  // Very simple preliminary scheme where the NU is introduced as a flat random
  // distribution with limits fNonUniformity (%) of the energy value.
  //
  return gRandom->Uniform(inputEnergy-inputEnergy*fNonUniformity/100,inputEnergy+inputEnergy*fNonUniformity/100);
}



// -----  Public method SelectGeometryVersion  ----------------------------------
void R3BCalo::SelectGeometryVersion(Int_t version)
{
  fGeometryVersion=version;
}


// -----  Public method SetNonUniformity  ----------------------------------
void R3BCalo::SetNonUniformity(Double_t nonU)
{
  fNonUniformity = nonU;
  cout << "-I- R3BCalo::SetNonUniformity to " << fNonUniformity << " %." << endl;
}


// -----   Public method ConstructGeometry   ----------------------------------
void R3BCalo::ConstructGeometry()
{

  //Switch between different geometries of CALIFA calorimeter
  if (fGeometryVersion==0)  {
    cout << "-I- R3BCalo: Constructing old (v5) geometry translated from R3BSim ... " << endl;
    ConstructOldGeometry();
  } else if (fGeometryVersion>0 && fGeometryVersion<11 ) {
    cout << "-I- R3BCalo: Constructing CALIFA. Geometry version: " << fGeometryVersion << endl;
    ConstructUserDefinedGeometry();
  } else
    cout << "-E- R3BCalo: Selected a wrong geometry version ... " << endl;
}




// -----   Public method ConstructOldGeometry   ----------------------------------
void R3BCalo::ConstructOldGeometry()
{

  // out-of-file geometry definition
  Double_t h1, bl1, tl1, alpha1, h2, bl2, tl2, alpha2;
  Double_t rmin, rmax, rmin1, rmax1, rmin2, rmax2;
  Double_t aMat;
  Double_t thx, phx, thy, phy, thz, phz;
  Double_t phi1, phi2;
  Double_t z, density, w;
  Int_t nel, numed;


  /****************************************************************************/
// Material definition


  // Mixture: CsI
  TGeoMedium * pCsIMedium=NULL;
  if (gGeoManager->GetMedium("CsI") ) {
    pCsIMedium=gGeoManager->GetMedium("CsI");
  } else {
    nel     = 2;
    density = 4.510000;
    TGeoMixture*
    pCsIMaterial = new TGeoMixture("CsIn", nel,density);
    aMat = 132.905450;   z = 55.000000;   w = 0.511549;  // CS
    pCsIMaterial->DefineElement(0,aMat,z,w);
    aMat = 126.904470;   z = 53.000000;   w = 0.488451;  // I
    pCsIMaterial->DefineElement(1,aMat,z,w);
    numed = 801;
    pCsIMaterial->SetIndex(numed);
    Double_t par[8];
    par[0]  = 0.000000; // isvol
    par[1]  = 0.000000; // ifield
    par[2]  = 0.000000; // fieldm
    par[3]  = 0.000000; // tmaxfd
    par[4]  = 0.000000; // stemax
    par[5]  = 0.000000; // deemax
    par[6]  = 0.000100; // epsil
    par[7]  = 0.000000; // stmin
    pCsIMedium = new TGeoMedium("CsIn", numed,pCsIMaterial, par);
  }

  // Mixture: CarbonFibre
  TGeoMedium * pCarbonFibreMedium=NULL;
  if (gGeoManager->GetMedium("CarbonFibre") ) {
    pCarbonFibreMedium=gGeoManager->GetMedium("CarbonFibre");
  } else {
    nel     = 3;
    density = 1.690000;
    TGeoMixture*
    pCarbonFibreMaterial = new TGeoMixture("CarbonFibre", nel,density);
    aMat = 12.010700;   z = 6.000000;   w = 0.844907;  // C
    pCarbonFibreMaterial->DefineElement(0,aMat,z,w);
    aMat = 1.007940;   z = 1.000000;   w = 0.042543;  // H
    pCarbonFibreMaterial->DefineElement(1,aMat,z,w);
    aMat = 15.999400;   z = 8.000000;   w = 0.112550;  // O
    pCarbonFibreMaterial->DefineElement(2,aMat,z,w);
    // Medium: CarbonFibre
    numed   = 802;  // medium number
    pCarbonFibreMaterial->SetIndex(numed);
    Double_t par[8];
    par[0]  = 0.000000; // isvol
    par[1]  = 0.000000; // ifield
    par[2]  = 0.000000; // fieldm
    par[3]  = 0.000000; // tmaxfd
    par[4]  = 0.000000; // stemax
    par[5]  = 0.000000; // deemax
    par[6]  = 0.000100; // epsil
    par[7]  = 0.000000; // stmin
    pCarbonFibreMedium = new TGeoMedium("CarbonFibre", numed,pCarbonFibreMaterial,par);
  }

  // Mixture: Air
  TGeoMedium * pAirMedium=NULL;
  if (gGeoManager->GetMedium("Air") ) {
    pAirMedium=gGeoManager->GetMedium("Air");
  } else {
    nel     = 2;
    density = 0.001290;
    TGeoMixture*
    pAirMaterial = new TGeoMixture("Air", nel,density);
    aMat = 14.006740;   z = 7.000000;   w = 0.700000;  // N
    pAirMaterial->DefineElement(0,aMat,z,w);
    aMat = 15.999400;   z = 8.000000;   w = 0.300000;  // O
    pAirMaterial->DefineElement(1,aMat,z,w);
    pAirMaterial->SetIndex(1);
    // Medium: Air
    numed   = 1;  // medium number
    Double_t par[8];
    par[0]  = 0.000000; // isvol
    par[1]  = 0.000000; // ifield
    par[2]  = 0.000000; // fieldm
    par[3]  = 0.000000; // tmaxfd
    par[4]  = 0.000000; // stemax
    par[5]  = 0.000000; // deemax
    par[6]  = 0.000100; // epsil
    par[7]  = 0.000000; // stmin
    pAirMedium = new TGeoMedium("Air", numed,pAirMaterial, par);
  }


  /****************************************************************************/
//  Root geometry definition of the Barrel Calorimeter
//  <D.Bertini@gsi.de> 15.04.09
//              update:20.04.09
//#include "geoCal/geo2/CalBARRELv4_140706.list"
//#include "geoCal/geo5/CalECSv4b_050706.list"
  /****************************************************************************/
  //WORLD

  TGeoVolume *pAWorld  =  gGeoManager->GetTopVolume();

  // Defintion of the Mother Volume

  Double_t length = 300.;

  TGeoShape *pCBWorld = new TGeoBBox("Califa_box",
                                     length/2.0,
                                     length/2.0,
                                     length/2.0);

  TGeoVolume*
  pWorld  = new TGeoVolume("CalifaWorld",pCBWorld, pAirMedium);

  TGeoCombiTrans *t00 = new TGeoCombiTrans();
  TGeoCombiTrans *pGlobalc = GetGlobalPosition(t00);

  // add the sphere as Mother Volume
  pAWorld->AddNodeOverlap(pWorld, 0, pGlobalc);


  // TRAP1
  Double_t ddz, theta, phi;
  ddz     = 5.500000;
  theta  = 3.036676;
  phi    = 29.392735;
  h1     = 1.215700;
  bl1    = 2.280700;
  tl1    = 2.280600;
  alpha1 = 0.000000;
  h2     = 1.503900;
  bl2    = 2.792600;
  tl2    = 2.792600;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap1_2 = new TGeoTrap("testTrap1", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog1
  TGeoVolume*
  pcrystalLog1  = new TGeoVolume("crystalLog1",ptestTrap1_2, pCsIMedium);
  pcrystalLog1->SetVisLeaves(kTRUE);
  Double_t fac= TMath::RadToDeg();

  //CALO Parameters
  Double_t angles1[8] = {0.700254121793611,0.647890449052555,0.595528098570855,0.543171485859594,0.490808697595434,
                         0.438452005438718,0.386094598657623,0.333733755035807
                        };
  Double_t fullOrientedCrystalCentersX1[512] = {-70.466125,-123.148595,-174.645077,-224.459632,-272.112517,-317.144810,-359.122826,-397.642292,-432.332245,-462.858603,-488.927379,-510.287518,-526.733308,-538.106369,-544.297172,-545.246095,-540.944000,-531.432319,-516.802654,-497.195897,-472.800871,-443.852515,-410.629616,-373.452130,-332.678096,-288.700189,-241.941942,-192.853662,-141.908096,-89.595877,-36.420801,17.105027,70.466125,123.148595,174.645077,224.459632,272.112517,317.144810,359.122826,397.642292,432.332245,462.858603,488.927379,510.287518,526.733308,538.106369,544.297172,545.246095,540.944000,531.432319,516.802654,497.195897,472.800871,443.852515,410.629616,373.452130,332.678096,288.700189,241.941942,192.853662,141.908096,89.595877,36.420801,-17.105027,-70.4665,-122.853839,-174.058028,-223.585943,-270.960604,-315.725766,-357.450316,-395.732424,-430.203412,-460.531307,-486.424033,-507.632230,-523.951651,-535.225131,-541.344100,-542.249630,-537.933000,-528.435781,-513.849436,-494.314440,-470.018926,-441.196873,-408.125853,-371.124358,-330.548732,-286.789741,-240.268809,-191.433956,-140.755490,-88.721471,-35.833016,17.400530,70.466500,122.853839,174.058028,223.585943,270.960604,315.725766,357.450316,395.732424,430.203412,460.531307,486.424033,507.632230,523.951651,535.225131,541.344100,542.249630,537.933000,528.435781,513.849436,494.314440,470.018926,441.196873,408.125853,371.124358,330.548732,286.789741,240.268809,191.433956,140.755490,88.721471,35.833016,-17.400530,-70.4665,-122.572505,-173.498070,-222.752754,-269.862207,-314.372739,-355.855690,-393.911556,-428.173839,-458.312573,-484.037507,-505.100895,-521.299885,-532.478473,-538.529001,-539.393201,-535.062750,-525.579352,-511.034337,-491.567782,-467.367161,-438.665539,-405.739327,-368.905625,-328.519159,-284.968874,-238.674183,-190.080930,-139.657093,-87.888282,-35.273058,17.681864,70.466500,122.572505,173.498070,222.752754,269.862207,314.372739,355.855690,393.911556,428.173839,458.312573,484.037507,505.100895,521.299885,532.478473,538.529001,539.393201,535.062750,525.579352,511.034337,491.567782,467.367161,438.665539,405.739327,368.905625,328.519159,284.968874,238.674183,190.080930,139.657093,87.888282,35.273058,-17.681864,-70.4665,-122.305850,-172.967327,-221.963034,-268.821117,-313.090304,-354.344261,-392.185689,-426.250155,-456.209598,-481.775494,-502.701628,-518.786471,-529.875117,-535.860775,-536.685801,-532.342250,-522.871952,-508.366111,-488.964426,-464.853747,-436.266272,-403.477314,-366.802650,-326.595475,-283.243007,-237.162754,-188.798495,-138.616002,-87.098562,-34.742315,17.948520,70.466500,122.305850,172.967327,221.963034,268.821117,313.090304,354.344261,392.185689,426.250155,456.209598,481.775494,502.701628,518.786471,529.875117,535.860775,536.685801,532.342250,522.871952,508.366111,488.964426,464.853747,436.266272,403.477314,366.802650,326.595475,283.243007,237.162754,188.798495,138.616002,87.098562,34.742315,-17.948520,-70.4665,-122.054509,-172.467067,-221.218672,-267.839821,-311.881525,-352.919640,-390.558946,-424.436956,-454.227406,-479.643398,-500.440162,-516.417413,-527.421282,-533.345796,-534.133899,-529.778000,-520.320049,-505.851132,-486.510592,-462.484689,-434.004805,-401.345218,-364.820458,-324.782276,-281.616264,-235.738133,-187.589716,-137.634706,-86.354200,-34.242055,18.199860,70.466500,122.054509,172.467067,221.218672,267.839821,311.881525,352.919640,390.558946,424.436956,454.227406,479.643398,500.440162,516.417413,527.421282,533.345796,534.133899,529.778000,520.320049,505.851132,486.510592,462.484689,434.004805,401.345218,364.820458,324.782276,281.616264,235.738133,187.589716,137.634706,86.354200,34.242055,-18.199860,-70.4665,-121.819194,-171.998703,-220.521771,-266.921093,-310.749820,-351.585855,-389.035927,-422.739370,-452.371601,-477.647247,-498.322889,-514.199409,-525.123908,-530.991176,-531.744709,-527.377250,-517.930860,-503.496512,-484.213217,-460.266685,-431.887533,-399.349068,-362.964653,-323.084690,-280.093245,-234.404348,-186.458010,-136.715979,-85.657299,-33.773692,18.435175,70.466500,121.819194,171.998703,220.521771,266.921093,310.749820,351.585855,389.035927,422.739370,452.371601,477.647247,498.322889,514.199409,525.123908,530.991176,531.744709,527.377250,517.930860,503.496512,484.213217,460.266685,431.887533,399.349068,362.964653,323.084690,280.093245,234.404348,186.458010,136.715979,85.657299,33.773692,-18.435175,-70.4665,-121.600616,-171.563652,-219.874436,-266.067709,-309.698605,-350.346933,-387.621230,-421.162522,-450.647788,-475.793070,-496.356205,-512.139158,-522.989931,-528.804025,-529.525447,-525.147250,-515.711598,-501.309361,-482.079241,-458.206434,-429.920849,-397.494891,-361.240839,-321.507842,-278.678547,-233.165427,-185.406795,-135.862595,-85.009964,-33.338640,18.653753,70.466500,121.600616,171.563652,219.874436,266.067709,309.698605,350.346933,387.621230,421.162522,450.647788,475.793070,496.356205,512.139158,522.989931,528.804025,529.525447,525.147250,515.711598,501.309361,482.079241,458.206434,429.920849,397.494891,361.240839,321.507842,278.678547,233.165427,185.406795,135.862595,85.009964,33.338640,-18.653753,-70.4665,-121.399289,-171.162937,-219.278191,-265.281677,-308.730356,-349.205792,-386.318186,-419.710124,-449.060025,-474.085232,-494.544739,-510.241509,-521.024375,-526.789492,-527.481338,-523.093250,-513.667488,-499.294828,-480.113685,-456.308785,-428.109382,-395.787052,-359.653076,-320.055444,-277.375504,-232.024285,-184.438546,-135.076563,-84.413719,-32.937925,18.855080,70.466500,121.399289,171.162937,219.278191,265.281677,308.730356,349.205792,386.318186,419.710124,449.060025,474.085232,494.544739,510.241509,521.024375,526.789492,527.481338,523.093250,513.667488,499.294828,480.113685,456.308785,428.109382,395.787052,359.653076,320.055444,277.375504,232.024285,184.438546,135.076563,84.413719,32.937925,-18.855080};
  Double_t fullOrientedCrystalCentersY1[512] = {540.944,531.432319,516.802654,497.195897,472.800871,443.852515,410.629616,373.452130,332.678096,288.700189,241.941942,192.853662,141.908096,89.595877,36.420801,-17.105027,-70.466125,-123.148595,-174.645077,-224.459632,-272.112517,-317.144810,-359.122826,-397.642292,-432.332245,-462.858603,-488.927379,-510.287518,-526.733308,-538.106369,-544.297172,-545.246095,-540.944000,-531.432319,-516.802654,-497.195897,-472.800871,-443.852515,-410.629616,-373.452130,-332.678096,-288.700189,-241.941942,-192.853662,-141.908096,-89.595877,-36.420801,17.105027,70.466125,123.148595,174.645077,224.459632,272.112517,317.144810,359.122826,397.642292,432.332245,462.858603,488.927379,510.287518,526.733308,538.106369,544.297172,545.246095,537.933,528.435781,513.849436,494.314440,470.018926,441.196873,408.125853,371.124358,330.548732,286.789741,240.268809,191.433956,140.755490,88.721471,35.833016,-17.400530,-70.466500,-122.853839,-174.058028,-223.585943,-270.960604,-315.725766,-357.450316,-395.732424,-430.203412,-460.531307,-486.424033,-507.632230,-523.951651,-535.225131,-541.344100,-542.249630,-537.933000,-528.435781,-513.849436,-494.314440,-470.018926,-441.196873,-408.125853,-371.124358,-330.548732,-286.789741,-240.268809,-191.433956,-140.755490,-88.721471,-35.833016,17.400530,70.466500,122.853839,174.058028,223.585943,270.960604,315.725766,357.450316,395.732424,430.203412,460.531307,486.424033,507.632230,523.951651,535.225131,541.344100,542.249630,535.06275,525.579352,511.034337,491.567782,467.367161,438.665539,405.739327,368.905625,328.519159,284.968874,238.674183,190.080930,139.657093,87.888282,35.273058,-17.681864,-70.466500,-122.572505,-173.498070,-222.752754,-269.862207,-314.372739,-355.855690,-393.911556,-428.173839,-458.312573,-484.037507,-505.100895,-521.299885,-532.478473,-538.529001,-539.393201,-535.062750,-525.579352,-511.034337,-491.567782,-467.367161,-438.665539,-405.739327,-368.905625,-328.519159,-284.968874,-238.674183,-190.080930,-139.657093,-87.888282,-35.273058,17.681864,70.466500,122.572505,173.498070,222.752754,269.862207,314.372739,355.855690,393.911556,428.173839,458.312573,484.037507,505.100895,521.299885,532.478473,538.529001,539.393201,532.34225,522.871952,508.366111,488.964426,464.853747,436.266272,403.477314,366.802650,326.595475,283.243007,237.162754,188.798495,138.616002,87.098562,34.742315,-17.948520,-70.466500,-122.305850,-172.967327,-221.963034,-268.821117,-313.090304,-354.344261,-392.185689,-426.250155,-456.209598,-481.775494,-502.701628,-518.786471,-529.875117,-535.860775,-536.685801,-532.342250,-522.871952,-508.366111,-488.964426,-464.853747,-436.266272,-403.477314,-366.802650,-326.595475,-283.243007,-237.162754,-188.798495,-138.616002,-87.098562,-34.742315,17.948520,70.466500,122.305850,172.967327,221.963034,268.821117,313.090304,354.344261,392.185689,426.250155,456.209598,481.775494,502.701628,518.786471,529.875117,535.860775,536.685801,529.778,520.320049,505.851132,486.510592,462.484689,434.004805,401.345218,364.820458,324.782276,281.616264,235.738133,187.589716,137.634706,86.354200,34.242055,-18.199860,-70.466500,-122.054509,-172.467067,-221.218672,-267.839821,-311.881525,-352.919640,-390.558946,-424.436956,-454.227406,-479.643398,-500.440162,-516.417413,-527.421282,-533.345796,-534.133899,-529.778000,-520.320049,-505.851132,-486.510592,-462.484689,-434.004805,-401.345218,-364.820458,-324.782276,-281.616264,-235.738133,-187.589716,-137.634706,-86.354200,-34.242055,18.199860,70.466500,122.054509,172.467067,221.218672,267.839821,311.881525,352.919640,390.558946,424.436956,454.227406,479.643398,500.440162,516.417413,527.421282,533.345796,534.133899,527.37725,517.930860,503.496512,484.213217,460.266685,431.887533,399.349068,362.964653,323.084690,280.093245,234.404348,186.458010,136.715979,85.657299,33.773692,-18.435175,-70.466500,-121.819194,-171.998703,-220.521771,-266.921093,-310.749820,-351.585855,-389.035927,-422.739370,-452.371601,-477.647247,-498.322889,-514.199409,-525.123908,-530.991176,-531.744709,-527.377250,-517.930860,-503.496512,-484.213217,-460.266685,-431.887533,-399.349068,-362.964653,-323.084690,-280.093245,-234.404348,-186.458010,-136.715979,-85.657299,-33.773692,18.435175,70.466500,121.819194,171.998703,220.521771,266.921093,310.749820,351.585855,389.035927,422.739370,452.371601,477.647247,498.322889,514.199409,525.123908,530.991176,531.744709,525.14725,515.711598,501.309361,482.079241,458.206434,429.920849,397.494891,361.240839,321.507842,278.678547,233.165427,185.406795,135.862595,85.009964,33.338640,-18.653753,-70.466500,-121.600616,-171.563652,-219.874436,-266.067709,-309.698605,-350.346933,-387.621230,-421.162522,-450.647788,-475.793070,-496.356205,-512.139158,-522.989931,-528.804025,-529.525447,-525.147250,-515.711598,-501.309361,-482.079241,-458.206434,-429.920849,-397.494891,-361.240839,-321.507842,-278.678547,-233.165427,-185.406795,-135.862595,-85.009964,-33.338640,18.653753,70.466500,121.600616,171.563652,219.874436,266.067709,309.698605,350.346933,387.621230,421.162522,450.647788,475.793070,496.356205,512.139158,522.989931,528.804025,529.525447,523.09325,513.667488,499.294828,480.113685,456.308785,428.109382,395.787052,359.653076,320.055444,277.375504,232.024285,184.438546,135.076563,84.413719,32.937925,-18.855080,-70.466500,-121.399289,-171.162937,-219.278191,-265.281677,-308.730356,-349.205792,-386.318186,-419.710124,-449.060025,-474.085232,-494.544739,-510.241509,-521.024375,-526.789492,-527.481338,-523.093250,-513.667488,-499.294828,-480.113685,-456.308785,-428.109382,-395.787052,-359.653076,-320.055444,-277.375504,-232.024285,-184.438546,-135.076563,-84.413719,-32.937925,18.855080,70.466500,121.399289,171.162937,219.278191,265.281677,308.730356,349.205792,386.318186,419.710124,449.060025,474.085232,494.544739,510.241509,521.024375,526.789492,527.481338};
  Double_t fullOrientedCrystalCentersZ1[512] = {-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-394.6085,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-357.995,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-322.98025,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-289.3575,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-256.9455,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-225.5825,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-195.1235,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575,-165.43575};

  Double_t angle1,angle2;
  Double_t dx,dy,dz;

  TGeoRotation *r1,*r2,*r3;

  for ( Int_t iter1=0; iter1<8; iter1++) {

    angle1 = angles1[iter1]*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX1[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY1[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ1[iter1*64]/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog1, iter1*64, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX1[(iter1*64)+iter2]/10.;
      dy = fullOrientedCrystalCentersY1[(iter1*64)+iter2]/10.;
      dz = fullOrientedCrystalCentersZ1[(iter1*64)+iter2]/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog1, (iter1*64)+iter2 , ph2);

    } // iter2
  } // iter1

  AddSensitiveVolume(pcrystalLog1);
  fNbOfSensitiveVol+=512;

  // Shape: testTrap2 type: TGeoTrap
  ddz     = 6.500000;
  theta  = 2.979381;
  phi    = 19.537861;
  h1     = 0.789200;
  bl1    = 1.987000;
  tl1    = 1.987000;
  alpha1 = 0.000000;
  h2     = 1.016200;
  bl2    = 2.627200;
  tl2    = 2.627200;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap2_3 = new TGeoTrap("testTrap2", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog2
  TGeoVolume*
  pcrystalLog2 = new TGeoVolume("crystalLog2",ptestTrap2_3, pCsIMedium);
  pcrystalLog2->SetVisLeaves(kTRUE);

  //code here

  Double_t angles2[8] = {0.279255689537595,0.244344834888671,0.209438702670953,0.174530312715493,0.139626105223433,0.104719492509259,0.0698125149800366,0.0349070005606382};

  Double_t fullOrientedCrystalCentersX2[512] = {-73.170525,-124.587436,-174.804503,-223.338106,-269.720841,-313.506017,-354.271959,-391.626068,-425.208604,-454.696149,-479.804721,-500.292512,-515.962212,-526.662914,-532.291564,-532.793956,-528.165250,-518.450024,-503.741842,-484.182349,-459.959917,-431.307818,-398.501990,-361.858370,-321.729855,-278.502907,-232.593823,-184.444733,-134.519340,-83.298452,-31.275355,21.048941,73.170525,124.587436,174.804503,223.338106,269.720841,313.506017,354.271959,391.626068,425.208604,454.696149,479.804721,500.292512,515.962212,526.662914,532.291564,532.793956,528.165250,518.450024,503.741842,484.182349,459.959917,431.307818,398.501990,361.858370,321.729855,278.502907,232.593823,184.444733,134.519340,83.298452,31.275355,-21.048941,-73.170525,-124.490865,-174.612290,-223.052103,-269.343802,-313.041574,-353.724583,-391.001032,-424.511927,-453.934540,-478.985516,-499.423599,-515.051960,-525.720088,-531.325245,-531.813450,-527.180000,-517.469519,-502.775523,-483.239524,-459.049664,-430.438905,-397.682785,-361.096761,-321.033178,-277.877871,-232.046447,-183.980290,-134.142301,-83.012449,-31.083142,21.145513,73.170525,124.490865,174.612290,223.052103,269.343802,313.041574,353.724583,391.001032,424.511927,453.934540,478.985516,499.423599,515.051960,525.720088,531.325245,531.813450,527.180000,517.469519,502.775523,483.239524,459.049664,430.438905,397.682785,361.096761,321.033178,277.877871,232.046447,183.980290,134.142301,83.012449,31.083142,-21.145513,-73.170525,-124.402723,-174.436855,-222.791064,-268.999674,-312.617670,-353.224987,-390.430554,-423.876062,-453.239411,-478.237817,-498.630531,-514.221161,-524.859560,-530.443274,-530.918530,-526.280750,-516.574599,-501.893552,-482.378995,-458.218866,-429.645838,-396.935086,-360.401631,-320.397313,-277.307393,-231.546851,-183.556386,-133.798173,-82.751411,-30.907707,21.233655,73.170525,124.402723,174.436855,222.791064,268.999674,312.617670,353.224987,390.430554,423.876062,453.239411,478.237817,498.630531,514.221161,524.859560,530.443274,530.918530,526.280750,516.574599,501.893552,482.378995,458.218866,429.645838,396.935086,360.401631,320.397313,277.307393,231.546851,183.556386,133.798173,82.751411,30.907707,-21.233655,-73.170525,-124.323109,-174.278393,-222.555281,-268.688840,-312.234778,-352.773725,-389.915268,-423.301714,-452.611533,-477.562456,-497.914191,-513.470740,-524.082285,-529.646631,-530.110191,-525.468500,-515.766260,-501.096909,-481.601721,-457.468444,-428.929497,-396.259724,-359.773754,-319.822965,-276.792107,-231.095589,-183.173494,-133.487339,-82.515627,-30.749245,21.313269,73.170525,124.323109,174.278393,222.555281,268.688840,312.234778,352.773725,389.915268,423.301714,452.611533,477.562456,497.914191,513.470740,524.082285,529.646631,530.110191,525.468500,515.766260,501.096909,481.601721,457.468444,428.929497,396.259724,359.773754,319.822965,276.792107,231.095589,183.173494,133.487339,82.515627,30.749245,-21.313269,-73.170525,-124.252120,-174.137099,-222.345042,-268.411681,-311.893369,-352.371353,-389.455809,-422.789592,-452.051680,-476.960264,-497.275459,-512.801620,-523.389221,-528.936298,-529.389429,-524.744250,-515.045497,-500.386575,-480.908657,-456.799325,-428.290766,-395.657532,-359.213901,-319.310843,-276.332647,-230.693217,-182.832085,-133.210180,-82.305388,-30.607951,21.384258,73.170525,124.252120,174.137099,222.345042,268.411681,311.893369,352.371353,389.455809,422.789592,452.051680,476.960264,497.275459,512.801620,523.389221,528.936298,529.389429,524.744250,515.045497,500.386575,480.908657,456.799325,428.290766,395.657532,359.213901,319.310843,276.332647,230.693217,182.832085,133.210180,82.305388,30.607951,-21.384258,-73.170525,-124.189854,-174.013167,-222.160639,-268.168581,-311.593914,-352.018427,-389.052810,-422.340402,-451.560625,-476.432073,-496.715219,-512.214726,-522.781325,-528.313254,-528.757238,-524.109000,-514.413306,-499.763531,-480.300760,-456.212430,-427.730525,-395.129341,-358.722846,-318.861654,-275.929649,-230.340291,-182.532630,-132.967081,-82.120985,-30.484020,21.446524,73.170525,124.189854,174.013167,222.160639,268.168581,311.593914,352.018427,389.052810,422.340402,451.560625,476.432073,496.715219,512.214726,522.781325,528.313254,528.757238,524.109000,514.413306,499.763531,480.300760,456.212430,427.730525,395.129341,358.722846,318.861654,275.929649,230.340291,182.532630,132.967081,82.120985,30.484020,-21.446524,-73.170525,-124.136361,-173.906697,-222.002216,-267.959732,-311.336649,-351.715225,-388.706590,-421.954499,-451.138755,-475.978298,-496.233910,-511.710518,-522.259074,-527.777990,-528.214115,-523.563250,-513.870184,-499.228268,-479.778510,-455.708223,-427.249217,-394.675567,-358.300976,-318.475750,-275.583429,-230.037089,-182.275366,-132.758231,-81.962562,-30.377549,21.500016,73.170525,124.136361,173.906697,222.002216,267.959732,311.336649,351.715225,388.706590,421.954499,451.138755,475.978298,496.233910,511.710518,522.259074,527.777990,528.214115,523.563250,513.870184,499.228268,479.778510,455.708223,427.249217,394.675567,358.300976,318.475750,275.583429,230.037089,182.275366,132.758231,81.962562,30.377549,-21.500016,-73.170525,-124.091739,-173.817882,-221.870064,-267.785515,-311.122046,-351.462302,-388.417783,-421.632589,-450.786842,-475.599772,-495.832415,-511.289922,-521.823427,-527.331488,-527.761058,-523.108000,-513.417126,-498.781765,-479.342863,-455.287627,-426.847722,-394.297040,-357.949063,-318.153840,-275.294621,-229.784165,-182.060762,-132.584015,-81.830410,-30.288734,21.544639,73.170525,124.091739,173.817882,221.870064,267.785515,311.122046,351.462302,388.417783,421.632589,450.786842,475.599772,495.832415,511.289922,521.823427,527.331488,527.761058,523.108000,513.417126,498.781765,479.342863,455.287627,426.847722,394.297040,357.949063,318.153840,275.294621,229.784165,182.060762,132.584015,81.830410,30.288734,-21.544639};
  Double_t fullOrientedCrystalCentersY2[512] = {528.16525,518.450024,503.741842,484.182349,459.959917,431.307818,398.501990,361.858370,321.729855,278.502907,232.593823,184.444733,134.519340,83.298452,31.275355,-21.048941,-73.170525,-124.587436,-174.804503,-223.338106,-269.720841,-313.506017,-354.271959,-391.626068,-425.208604,-454.696149,-479.804721,-500.292512,-515.962212,-526.662914,-532.291564,-532.793956,-528.165250,-518.450024,-503.741842,-484.182349,-459.959917,-431.307818,-398.501990,-361.858370,-321.729855,-278.502907,-232.593823,-184.444733,-134.519340,-83.298452,-31.275355,21.048941,73.170525,124.587436,174.804503,223.338106,269.720841,313.506017,354.271959,391.626068,425.208604,454.696149,479.804721,500.292512,515.962212,526.662914,532.291564,532.793956,527.18,517.469519,502.775523,483.239524,459.049664,430.438905,397.682785,361.096761,321.033178,277.877871,232.046447,183.980290,134.142301,83.012449,31.083142,-21.145513,-73.170525,-124.490865,-174.612290,-223.052103,-269.343802,-313.041574,-353.724583,-391.001032,-424.511927,-453.934540,-478.985516,-499.423599,-515.051960,-525.720088,-531.325245,-531.813450,-527.180000,-517.469519,-502.775523,-483.239524,-459.049664,-430.438905,-397.682785,-361.096761,-321.033178,-277.877871,-232.046447,-183.980290,-134.142301,-83.012449,-31.083142,21.145513,73.170525,124.490865,174.612290,223.052103,269.343802,313.041574,353.724583,391.001032,424.511927,453.934540,478.985516,499.423599,515.051960,525.720088,531.325245,531.813450,526.28075,516.574599,501.893552,482.378995,458.218866,429.645838,396.935086,360.401631,320.397313,277.307393,231.546851,183.556386,133.798173,82.751411,30.907707,-21.233655,-73.170525,-124.402723,-174.436855,-222.791064,-268.999674,-312.617670,-353.224987,-390.430554,-423.876062,-453.239411,-478.237817,-498.630531,-514.221161,-524.859560,-530.443274,-530.918530,-526.280750,-516.574599,-501.893552,-482.378995,-458.218866,-429.645838,-396.935086,-360.401631,-320.397313,-277.307393,-231.546851,-183.556386,-133.798173,-82.751411,-30.907707,21.233655,73.170525,124.402723,174.436855,222.791064,268.999674,312.617670,353.224987,390.430554,423.876062,453.239411,478.237817,498.630531,514.221161,524.859560,530.443274,530.918530,525.4685,515.766260,501.096909,481.601721,457.468444,428.929497,396.259724,359.773754,319.822965,276.792107,231.095589,183.173494,133.487339,82.515627,30.749245,-21.313269,-73.170525,-124.323109,-174.278393,-222.555281,-268.688840,-312.234778,-352.773725,-389.915268,-423.301714,-452.611533,-477.562456,-497.914191,-513.470740,-524.082285,-529.646631,-530.110191,-525.468500,-515.766260,-501.096909,-481.601721,-457.468444,-428.929497,-396.259724,-359.773754,-319.822965,-276.792107,-231.095589,-183.173494,-133.487339,-82.515627,-30.749245,21.313269,73.170525,124.323109,174.278393,222.555281,268.688840,312.234778,352.773725,389.915268,423.301714,452.611533,477.562456,497.914191,513.470740,524.082285,529.646631,530.110191,524.74425,515.045497,500.386575,480.908657,456.799325,428.290766,395.657532,359.213901,319.310843,276.332647,230.693217,182.832085,133.210180,82.305388,30.607951,-21.384258,-73.170525,-124.252120,-174.137099,-222.345042,-268.411681,-311.893369,-352.371353,-389.455809,-422.789592,-452.051680,-476.960264,-497.275459,-512.801620,-523.389221,-528.936298,-529.389429,-524.744250,-515.045497,-500.386575,-480.908657,-456.799325,-428.290766,-395.657532,-359.213901,-319.310843,-276.332647,-230.693217,-182.832085,-133.210180,-82.305388,-30.607951,21.384258,73.170525,124.252120,174.137099,222.345042,268.411681,311.893369,352.371353,389.455809,422.789592,452.051680,476.960264,497.275459,512.801620,523.389221,528.936298,529.389429,524.109,514.413306,499.763531,480.300760,456.212430,427.730525,395.129341,358.722846,318.861654,275.929649,230.340291,182.532630,132.967081,82.120985,30.484020,-21.446524,-73.170525,-124.189854,-174.013167,-222.160639,-268.168581,-311.593914,-352.018427,-389.052810,-422.340402,-451.560625,-476.432073,-496.715219,-512.214726,-522.781325,-528.313254,-528.757238,-524.109000,-514.413306,-499.763531,-480.300760,-456.212430,-427.730525,-395.129341,-358.722846,-318.861654,-275.929649,-230.340291,-182.532630,-132.967081,-82.120985,-30.484020,21.446524,73.170525,124.189854,174.013167,222.160639,268.168581,311.593914,352.018427,389.052810,422.340402,451.560625,476.432073,496.715219,512.214726,522.781325,528.313254,528.757238,523.56325,513.870184,499.228268,479.778510,455.708223,427.249217,394.675567,358.300976,318.475750,275.583429,230.037089,182.275366,132.758231,81.962562,30.377549,-21.500016,-73.170525,-124.136361,-173.906697,-222.002216,-267.959732,-311.336649,-351.715225,-388.706590,-421.954499,-451.138755,-475.978298,-496.233910,-511.710518,-522.259074,-527.777990,-528.214115,-523.563250,-513.870184,-499.228268,-479.778510,-455.708223,-427.249217,-394.675567,-358.300976,-318.475750,-275.583429,-230.037089,-182.275366,-132.758231,-81.962562,-30.377549,21.500016,73.170525,124.136361,173.906697,222.002216,267.959732,311.336649,351.715225,388.706590,421.954499,451.138755,475.978298,496.233910,511.710518,522.259074,527.777990,528.214115,523.108,513.417126,498.781765,479.342863,455.287627,426.847722,394.297040,357.949063,318.153840,275.294621,229.784165,182.060762,132.584015,81.830410,30.288734,-21.544639,-73.170525,-124.091739,-173.817882,-221.870064,-267.785515,-311.122046,-351.462302,-388.417783,-421.632589,-450.786842,-475.599772,-495.832415,-511.289922,-521.823427,-527.331488,-527.761058,-523.108000,-513.417126,-498.781765,-479.342863,-455.287627,-426.847722,-394.297040,-357.949063,-318.153840,-275.294621,-229.784165,-182.060762,-132.584015,-81.830410,-30.288734,21.544639,73.170525,124.091739,173.817882,221.870064,267.785515,311.122046,351.462302,388.417783,421.632589,450.786842,475.599772,495.832415,511.289922,521.823427,527.331488,527.761058};
  Double_t fullOrientedCrystalCentersZ2[512] = {-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-141.25125,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-122.19795,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-103.349975,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-84.676025,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-66.145975,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-47.7296,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-29.397825,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175,-11.12175};


  for ( Int_t iter1=0; iter1<8; iter1++) {

    angle1 = angles2[iter1]*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX2[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY2[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ2[iter1*64]/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog2, iter1*64, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX2[(iter1*64)+iter2]/10.;
      dy = fullOrientedCrystalCentersY2[(iter1*64)+iter2]/10.;
      dz = fullOrientedCrystalCentersZ2[(iter1*64)+iter2]/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog2, (iter1*64)+iter2 , ph2);

    } // iter2
  } // iter1





  AddSensitiveVolume(pcrystalLog2);
  fNbOfSensitiveVol+=512;

  // Shape: testTrap3 type: TGeoTrap
  ddz     = 6.500000;
  theta  = 2.979381;
  phi    = 19.537861;
  h1     = 0.789200;
  bl1    = 1.987000;
  tl1    = 1.987000;
  alpha1 = 0.000000;
  h2     = 1.016200;
  bl2    = 2.627200;
  tl2    = 2.627200;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap3_4 = new TGeoTrap("testTrap3", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog3
  TGeoVolume*
  pcrystalLog3 = new TGeoVolume("crystalLog3",ptestTrap3_4, pCsIMedium);
  pcrystalLog3->SetVisLeaves(kTRUE);

  // code here

  Double_t angles3[8] = {0,-0.0349065669643691,-0.0698131800210238,-0.104720155525111,-0.139626765403062,-0.174530969255247,-0.209437398472098,-0.244343222090134};

  Double_t fullOrientedCrystalCentersX3[512] = {-73.170525,-124.056061,-173.746869,-221.764400,-267.646219,-310.950458,-351.260074,-388.186864,-421.375202,-450.505466,-475.297117,-495.511396,-510.953630,-521.475101,-526.974482,-527.398810,-522.744000,-513.054879,-498.424759,-478.994537,-454.951335,-426.526703,-393.994385,-357.667687,-317.896453,-275.063702,-229.581938,-181.889174,-132.444718,-81.724747,-30.217721,21.580317,73.170525,124.056061,173.746869,221.764400,267.646219,310.950458,351.260074,388.186864,421.375202,450.505466,475.297117,495.511396,510.953630,521.475101,526.974482,527.398810,522.744000,513.054879,498.424759,478.994537,454.951335,426.526703,393.994385,357.667687,317.896453,275.063702,229.581938,181.889174,132.444718,81.724747,30.217721,-21.580317,-73.170525,-124.100708,-173.835733,-221.896625,-267.820531,-311.165179,-351.513136,-388.475830,-421.697289,-450.857572,-475.675851,-495.913111,-511.374457,-521.910987,-527.421230,-527.852117,-523.199500,-513.508186,-498.871507,-479.430423,-455.372162,-426.928418,-394.373120,-358.019793,-318.218540,-275.352668,-229.835000,-182.103895,-132.619030,-81.856971,-30.306585,21.535670,73.170525,124.100708,173.835733,221.896625,267.820531,311.165179,351.513136,388.475830,421.697289,450.857572,475.675851,495.913111,511.374457,521.910987,527.421230,527.852117,523.199500,513.508186,498.871507,479.430423,455.372162,426.928418,394.373120,358.019793,318.218540,275.352668,229.835000,182.103895,132.619030,81.856971,30.306585,-21.535670,-73.170525,-124.154274,-173.942350,-222.055265,-268.029667,-311.422797,-351.816755,-388.822526,-422.083723,-451.280023,-476.130249,-496.395081,-511.879357,-522.433955,-527.957229,-528.395985,-523.746000,-514.052054,-499.407506,-479.953391,-455.877062,-427.410388,-394.827518,-358.442243,-318.604974,-275.699364,-230.138619,-182.361513,-132.828167,-82.015612,-30.413202,21.482104,73.170525,124.154274,173.942350,222.055265,268.029667,311.422797,351.816755,388.822526,422.083723,451.280023,476.130249,496.395081,511.879357,522.433955,527.957229,528.395985,523.746000,514.052054,499.407506,479.953391,455.877062,427.410388,394.827518,358.442243,318.604974,275.699364,230.138619,182.361513,132.828167,82.015612,30.413202,-21.482104,-73.170525,-124.216687,-174.066573,-222.240104,-268.273341,-311.722959,-352.170515,-389.226476,-422.533973,-451.772237,-476.659687,-496.956645,-512.467638,-523.043287,-528.581744,-529.029669,-524.382750,-514.685738,-500.032021,-480.562723,-456.465342,-427.971951,-395.356956,-358.934458,-319.055224,-276.103314,-230.492379,-182.661675,-133.071840,-82.200451,-30.537426,21.419691,73.170525,124.216687,174.066573,222.240104,268.273341,311.722959,352.170515,389.226476,422.533973,451.772237,476.659687,496.956645,512.467638,523.043287,528.581744,529.029669,524.382750,514.685738,500.032021,480.562723,456.465342,427.971951,395.356956,358.934458,319.055224,276.103314,230.492379,182.661675,133.071840,82.200451,30.537426,-21.419691,-73.170525,-124.287847,-174.208209,-222.450851,-268.551169,-312.065193,-352.573859,-389.687045,-423.047332,-452.333443,-477.263334,-497.596919,-513.138374,-523.738026,-529.293794,-529.752173,-525.108750,-515.408242,-500.744071,-481.257461,-457.136079,-428.612226,-395.960603,-359.495663,-319.568584,-276.563884,-230.895723,-183.003909,-133.349668,-82.411197,-30.679061,21.348531,73.170525,124.287847,174.208209,222.450851,268.551169,312.065193,352.573859,389.687045,423.047332,452.333443,477.263334,497.596919,513.138374,523.738026,529.293794,529.752173,525.108750,515.408242,500.744071,481.257461,457.136079,428.612226,395.960603,359.495663,319.568584,276.563884,230.895723,183.003909,133.349668,82.411197,30.679061,-21.348531,-73.170525,-124.367682,-174.367110,-222.687288,-268.862865,-312.449146,-353.026371,-390.203758,-423.623271,-452.963060,-477.940566,-498.315244,-513.890874,-524.517454,-530.092644,-530.562751,-525.923250,-516.218820,-501.542921,-482.036889,-457.888579,-429.330551,-396.637835,-360.125280,-320.144522,-277.080597,-231.348234,-183.387862,-133.661364,-82.647634,-30.837962,21.268696,73.170525,124.367682,174.367110,222.687288,268.862865,312.449146,353.026371,390.203758,423.623271,452.963060,477.940566,498.315244,513.890874,524.517454,530.092644,530.562751,525.923250,516.218820,501.542921,482.036889,457.888579,429.330551,396.637835,360.125280,320.144522,277.080597,231.348234,183.387862,133.661364,82.647634,30.837962,-21.268696,-73.170525,-124.456093,-174.543082,-222.949124,-269.208045,-312.874346,-353.527495,-390.775981,-424.261081,-453.660315,-478.690552,-499.110737,-514.724213,-525.380614,-530.977312,-531.460408,-526.825250,-517.116477,-502.427589,-482.900049,-458.721918,-430.126044,-397.387821,-360.822536,-320.782332,-277.652820,-231.849359,-183.813062,-134.006544,-82.909471,-31.013934,21.180284,73.170525,124.456093,174.543082,222.949124,269.208045,312.874346,353.527495,390.775981,424.261081,453.660315,478.690552,499.110737,514.724213,525.380614,530.977312,531.460408,526.825250,517.116477,502.427589,482.900049,458.721918,430.126044,397.387821,360.822536,320.782332,277.652820,231.849359,183.813062,134.006544,82.909471,31.013934,-21.180284,-73.170525,-124.552959,-174.735880,-223.235998,-269.586232,-313.340203,-354.076537,-391.402920,-424.959879,-454.424243,-479.512252,-499.982296,-515.637237,-526.326310,-531.946573,-532.443899,-527.813500,-518.099968,-503.396850,-483.845746,-459.634942,-430.997603,-398.209521,-361.586463,-321.481131,-278.279759,-232.398401,-184.278920,-134.384731,-83.196345,-31.206732,21.083419,73.170525,124.552959,174.735880,223.235998,269.586232,313.340203,354.076537,391.402920,424.959879,454.424243,479.512252,499.982296,515.637237,526.326310,531.946573,532.443899,527.813500,518.099968,503.396850,483.845746,459.634942,430.997603,398.209521,361.586463,321.481131,278.279759,232.398401,184.278920,134.384731,83.196345,31.206732,-21.083419};
  Double_t fullOrientedCrystalCentersY3[512] = {522.744,513.054879,498.424759,478.994537,454.951335,426.526703,393.994385,357.667687,317.896453,275.063702,229.581938,181.889174,132.444718,81.724747,30.217721,-21.580317,-73.170525,-124.056061,-173.746869,-221.764400,-267.646219,-310.950458,-351.260074,-388.186864,-421.375202,-450.505466,-475.297117,-495.511396,-510.953630,-521.475101,-526.974482,-527.398810,-522.744000,-513.054879,-498.424759,-478.994537,-454.951335,-426.526703,-393.994385,-357.667687,-317.896453,-275.063702,-229.581938,-181.889174,-132.444718,-81.724747,-30.217721,21.580317,73.170525,124.056061,173.746869,221.764400,267.646219,310.950458,351.260074,388.186864,421.375202,450.505466,475.297117,495.511396,510.953630,521.475101,526.974482,527.398810,523.1995,513.508186,498.871507,479.430423,455.372162,426.928418,394.373120,358.019793,318.218540,275.352668,229.835000,182.103895,132.619030,81.856971,30.306585,-21.535670,-73.170525,-124.100708,-173.835733,-221.896625,-267.820531,-311.165179,-351.513136,-388.475830,-421.697289,-450.857572,-475.675851,-495.913111,-511.374457,-521.910987,-527.421230,-527.852117,-523.199500,-513.508186,-498.871507,-479.430423,-455.372162,-426.928418,-394.373120,-358.019793,-318.218540,-275.352668,-229.835000,-182.103895,-132.619030,-81.856971,-30.306585,21.535670,73.170525,124.100708,173.835733,221.896625,267.820531,311.165179,351.513136,388.475830,421.697289,450.857572,475.675851,495.913111,511.374457,521.910987,527.421230,527.852117,523.746,514.052054,499.407506,479.953391,455.877062,427.410388,394.827518,358.442243,318.604974,275.699364,230.138619,182.361513,132.828167,82.015612,30.413202,-21.482104,-73.170525,-124.154274,-173.942350,-222.055265,-268.029667,-311.422797,-351.816755,-388.822526,-422.083723,-451.280023,-476.130249,-496.395081,-511.879357,-522.433955,-527.957229,-528.395985,-523.746000,-514.052054,-499.407506,-479.953391,-455.877062,-427.410388,-394.827518,-358.442243,-318.604974,-275.699364,-230.138619,-182.361513,-132.828167,-82.015612,-30.413202,21.482104,73.170525,124.154274,173.942350,222.055265,268.029667,311.422797,351.816755,388.822526,422.083723,451.280023,476.130249,496.395081,511.879357,522.433955,527.957229,528.395985,524.38275,514.685738,500.032021,480.562723,456.465342,427.971951,395.356956,358.934458,319.055224,276.103314,230.492379,182.661675,133.071840,82.200451,30.537426,-21.419691,-73.170525,-124.216687,-174.066573,-222.240104,-268.273341,-311.722959,-352.170515,-389.226476,-422.533973,-451.772237,-476.659687,-496.956645,-512.467638,-523.043287,-528.581744,-529.029669,-524.382750,-514.685738,-500.032021,-480.562723,-456.465342,-427.971951,-395.356956,-358.934458,-319.055224,-276.103314,-230.492379,-182.661675,-133.071840,-82.200451,-30.537426,21.419691,73.170525,124.216687,174.066573,222.240104,268.273341,311.722959,352.170515,389.226476,422.533973,451.772237,476.659687,496.956645,512.467638,523.043287,528.581744,529.029669,525.10875,515.408242,500.744071,481.257461,457.136079,428.612226,395.960603,359.495663,319.568584,276.563884,230.895723,183.003909,133.349668,82.411197,30.679061,-21.348531,-73.170525,-124.287847,-174.208209,-222.450851,-268.551169,-312.065193,-352.573859,-389.687045,-423.047332,-452.333443,-477.263334,-497.596919,-513.138374,-523.738026,-529.293794,-529.752173,-525.108750,-515.408242,-500.744071,-481.257461,-457.136079,-428.612226,-395.960603,-359.495663,-319.568584,-276.563884,-230.895723,-183.003909,-133.349668,-82.411197,-30.679061,21.348531,73.170525,124.287847,174.208209,222.450851,268.551169,312.065193,352.573859,389.687045,423.047332,452.333443,477.263334,497.596919,513.138374,523.738026,529.293794,529.752173,525.92325,516.218820,501.542921,482.036889,457.888579,429.330551,396.637835,360.125280,320.144522,277.080597,231.348234,183.387862,133.661364,82.647634,30.837962,-21.268696,-73.170525,-124.367682,-174.367110,-222.687288,-268.862865,-312.449146,-353.026371,-390.203758,-423.623271,-452.963060,-477.940566,-498.315244,-513.890874,-524.517454,-530.092644,-530.562751,-525.923250,-516.218820,-501.542921,-482.036889,-457.888579,-429.330551,-396.637835,-360.125280,-320.144522,-277.080597,-231.348234,-183.387862,-133.661364,-82.647634,-30.837962,21.268696,73.170525,124.367682,174.367110,222.687288,268.862865,312.449146,353.026371,390.203758,423.623271,452.963060,477.940566,498.315244,513.890874,524.517454,530.092644,530.562751,526.82525,517.116477,502.427589,482.900049,458.721918,430.126044,397.387821,360.822536,320.782332,277.652820,231.849359,183.813062,134.006544,82.909471,31.013934,-21.180284,-73.170525,-124.456093,-174.543082,-222.949124,-269.208045,-312.874346,-353.527495,-390.775981,-424.261081,-453.660315,-478.690552,-499.110737,-514.724213,-525.380614,-530.977312,-531.460408,-526.825250,-517.116477,-502.427589,-482.900049,-458.721918,-430.126044,-397.387821,-360.822536,-320.782332,-277.652820,-231.849359,-183.813062,-134.006544,-82.909471,-31.013934,21.180284,73.170525,124.456093,174.543082,222.949124,269.208045,312.874346,353.527495,390.775981,424.261081,453.660315,478.690552,499.110737,514.724213,525.380614,530.977312,531.460408,527.8135,518.099968,503.396850,483.845746,459.634942,430.997603,398.209521,361.586463,321.481131,278.279759,232.398401,184.278920,134.384731,83.196345,31.206732,-21.083419,-73.170525,-124.552959,-174.735880,-223.235998,-269.586232,-313.340203,-354.076537,-391.402920,-424.959879,-454.424243,-479.512252,-499.982296,-515.637237,-526.326310,-531.946573,-532.443899,-527.813500,-518.099968,-503.396850,-483.845746,-459.634942,-430.997603,-398.209521,-361.586463,-321.481131,-278.279759,-232.398401,-184.278920,-134.384731,-83.196345,-31.206732,21.083419,73.170525,124.552959,174.735880,223.235998,269.586232,313.340203,354.076537,391.402920,424.959879,454.424243,479.512252,499.982296,515.637237,526.326310,531.946573,532.443899};
  Double_t fullOrientedCrystalCentersZ3[512] = {9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,9.127325,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,27.4034,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,45.735225,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,64.15175,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,82.682125,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,101.3563,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,120.2049,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925,139.25925};

  for ( Int_t iter1=0; iter1<8; iter1++) {

    angle1 = angles3[iter1]*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX3[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY3[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ3[iter1*64]/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog3, iter1*64, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX3[(iter1*64)+iter2]/10.;
      dy = fullOrientedCrystalCentersY3[(iter1*64)+iter2]/10.;
      dz = fullOrientedCrystalCentersZ3[(iter1*64)+iter2]/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog3, (iter1*64)+iter2 , ph2);

    } // iter2
  } // iter1




  AddSensitiveVolume(pcrystalLog3);
  fNbOfSensitiveVol+=512;


  // Shape: testTrap4 type: TGeoTrap
  ddz     = 7.000000;
  theta  = 2.807493;
  phi    = 17.532509;
  h1     = 0.693900;
  bl1    = 1.961000;
  tl1    = 1.961000;
  alpha1 = 0.000000;
  h2     = 0.901700;
  bl2    = 2.618400;
  tl2    = 2.618400;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap4_5 = new TGeoTrap("testTrap4", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog4
  TGeoVolume*
  pcrystalLog4 = new TGeoVolume("crystalLog4",ptestTrap4_5, pCsIMedium);
  pcrystalLog4->SetVisLeaves(kTRUE);

  // code here

  Double_t angles4[8] = {-0.278449839266083,-0.308118619473451,-0.33778605239218,-0.367459101459503,-0.397128033728904,-0.426797979175989,-0.456474250668749,-0.486141257367118};

  Double_t fullOrientedCrystalCentersX4[512] = {-72.997325,-123.758233,-173.327282,-221.227094,-266.996368,-310.194321,-350.404934,-387.240955,-420.347634,-449.406136,-474.136611,-494.300891,-509.704783,-520.199940,-525.685287,-526.107997,-521.464000,-511.798019,-497.203144,-477.819930,-453.835050,-425.479489,-393.026329,-356.788110,-317.113827,-274.385564,-229.014818,-181.438535,-132.114899,-81.518924,-30.137878,21.533413,72.997325,123.758233,173.327282,221.227094,266.996368,310.194321,350.404934,387.240955,420.347634,449.406136,474.136611,494.300891,509.704783,520.199940,525.685287,526.107997,521.464000,511.798019,497.203144,477.819930,453.835050,425.479489,393.026329,356.788110,317.113827,274.385564,229.014818,181.438535,132.114899,81.518924,30.137878,-21.533413,-72.997325,-123.849977,-173.509886,-221.498800,-267.354560,-310.635548,-350.924947,-387.834747,-421.009486,-450.129674,-474.914866,-495.126369,-510.569535,-521.095636,-526.603302,-527.039490,-522.400000,-512.729512,-498.121159,-478.715626,-454.699801,-426.304968,-393.804585,-357.511648,-317.775679,-274.979356,-229.534832,-181.879762,-132.473091,-81.790631,-30.320482,21.441669,72.997325,123.849977,173.509886,221.498800,267.354560,310.635548,350.924947,387.834747,421.009486,450.129674,474.914866,495.126369,510.569535,521.095636,526.603302,527.039490,522.400000,512.729512,498.121159,478.715626,454.699801,426.304968,393.804585,357.511648,317.775679,274.979356,229.534832,181.879762,132.473091,81.790631,30.320482,-21.441669,-72.997325,-123.947602,-173.704196,-221.787924,-267.735712,-311.105060,-351.478295,-388.466603,-421.713764,-450.899592,-475.743010,-496.004763,-511.489719,-522.048749,-527.580164,-528.030694,-523.396000,-513.720716,-499.098021,-479.668739,-455.619985,-427.183361,-394.632728,-358.281567,-318.479957,-275.611212,-230.088180,-182.349273,-132.854243,-82.079754,-30.514792,21.344044,72.997325,123.947602,173.704196,221.787924,267.735712,311.105060,351.478295,388.466603,421.713764,450.899592,475.743010,496.004763,511.489719,522.048749,527.580164,528.030694,523.396000,513.720716,499.098021,479.668739,455.619985,427.183361,394.632728,358.281567,318.479957,275.611212,230.088180,182.349273,132.854243,82.079754,30.514792,-21.344044,-72.997325,-124.051059,-173.910114,-222.094319,-268.139635,-311.602619,-352.064700,-389.136205,-422.460116,-451.715505,-476.620626,-496.935631,-512.464874,-523.058799,-528.615383,-529.081112,-524.451500,-514.771134,-500.133240,-480.678790,-456.595140,-428.114229,-395.510345,-359.097479,-319.226309,-276.280814,-230.674585,-182.846833,-133.258166,-82.386150,-30.720710,21.240587,72.997325,124.051059,173.910114,222.094319,268.139635,311.602619,352.064700,389.136205,422.460116,451.715505,476.620626,496.935631,512.464874,523.058799,528.615383,529.081112,524.451500,514.771134,500.133240,480.678790,456.595140,428.114229,395.510345,359.097479,319.226309,276.280814,230.674585,182.846833,133.258166,82.386150,30.720710,-21.240587,-72.997325,-124.160201,-174.127347,-222.417551,-268.565753,-312.127519,-352.683327,-389.842602,-423.247479,-452.576252,-477.546468,-497.917650,-513.493613,-524.124352,-529.707488,-530.189250,-525.565000,-515.879272,-501.225344,-481.744343,-457.623880,-429.096248,-396.436186,-359.958226,-320.013672,-276.987211,-231.293212,-183.371733,-133.684284,-82.709382,-30.937943,21.131445,72.997325,124.160201,174.127347,222.417551,268.565753,312.127519,352.683327,389.842602,423.247479,452.576252,477.546468,497.917650,513.493613,524.124352,529.707488,530.189250,525.565000,515.879272,501.225344,481.744343,457.623880,429.096248,396.436186,359.958226,320.013672,276.987211,231.293212,183.371733,133.684284,82.709382,30.937943,-21.131445,-72.997325,-124.275028,-174.355895,-222.757620,-269.014066,-312.679760,-353.334178,-390.585793,-424.075855,-453.481833,-478.520534,-498.950821,-514.575938,-525.245408,-530.856477,-531.355109,-526.736500,-517.045131,-502.374334,-482.865398,-458.706204,-430.129419,-397.410253,-360.863808,-320.842048,-277.730403,-231.944063,-183.923974,-134.132597,-83.049450,-31.166492,21.016617,72.997325,124.275028,174.355895,222.757620,269.014066,312.679760,353.334178,390.585793,424.075855,453.481833,478.520534,498.950821,514.575938,525.245408,530.856477,531.355109,526.736500,517.045131,502.374334,482.865398,458.706204,430.129419,397.410253,360.863808,320.842048,277.730403,231.944063,183.923974,134.132597,83.049450,31.166492,-21.016617,-72.997325,-124.395344,-174.595369,-223.113944,-269.483810,-313.258400,-354.016140,-391.364511,-424.943828,-454.430704,-479.541163,-500.033379,-515.710000,-526.420052,-532.060391,-532.576698,-527.964000,-518.266720,-503.578248,-484.040042,-459.840267,-431.211978,-398.430882,-361.812678,-321.710021,-278.509121,-232.626025,-184.502614,-134.602341,-83.405775,-31.405965,20.896301,72.997325,124.395344,174.595369,223.113944,269.483810,313.258400,354.016140,391.364511,424.943828,454.430704,479.541163,500.033379,515.710000,526.420052,532.060391,532.576698,527.964000,518.266720,503.578248,484.040042,459.840267,431.211978,398.430882,361.812678,321.710021,278.509121,232.626025,184.502614,134.602341,83.405775,31.405965,-20.896301,-72.997325,-124.521076,-174.845621,-223.486307,-269.974697,-313.863084,-354.728798,-392.178279,-425.850869,-455.422283,-480.607731,-501.164664,-516.895107,-527.647568,-533.318494,-533.853271,-529.246750,-519.543293,-504.836350,-485.267558,-461.025373,-432.343262,-399.497449,-362.804257,-322.617062,-279.322889,-233.338683,-185.107298,-135.093228,-83.778137,-31.656217,20.770570,72.997325,124.521076,174.845621,223.486307,269.974697,313.863084,354.728798,392.178279,425.850869,455.422283,480.607731,501.164664,516.895107,527.647568,533.318494,533.853271,529.246750,519.543293,504.836350,485.267558,461.025373,432.343262,399.497449,362.804257,322.617062,279.322889,233.338683,185.107298,135.093228,83.778137,31.656217,-20.770570};
  Double_t fullOrientedCrystalCentersY4[512] = {521.464,511.798019,497.203144,477.819930,453.835050,425.479489,393.026329,356.788110,317.113827,274.385564,229.014818,181.438535,132.114899,81.518924,30.137878,-21.533413,-72.997325,-123.758233,-173.327282,-221.227094,-266.996368,-310.194321,-350.404934,-387.240955,-420.347634,-449.406136,-474.136611,-494.300891,-509.704783,-520.199940,-525.685287,-526.107997,-521.464000,-511.798019,-497.203144,-477.819930,-453.835050,-425.479489,-393.026329,-356.788110,-317.113827,-274.385564,-229.014818,-181.438535,-132.114899,-81.518924,-30.137878,21.533413,72.997325,123.758233,173.327282,221.227094,266.996368,310.194321,350.404934,387.240955,420.347634,449.406136,474.136611,494.300891,509.704783,520.199940,525.685287,526.107997,522.4,512.729512,498.121159,478.715626,454.699801,426.304968,393.804585,357.511648,317.775679,274.979356,229.534832,181.879762,132.473091,81.790631,30.320482,-21.441669,-72.997325,-123.849977,-173.509886,-221.498800,-267.354560,-310.635548,-350.924947,-387.834747,-421.009486,-450.129674,-474.914866,-495.126369,-510.569535,-521.095636,-526.603302,-527.039490,-522.400000,-512.729512,-498.121159,-478.715626,-454.699801,-426.304968,-393.804585,-357.511648,-317.775679,-274.979356,-229.534832,-181.879762,-132.473091,-81.790631,-30.320482,21.441669,72.997325,123.849977,173.509886,221.498800,267.354560,310.635548,350.924947,387.834747,421.009486,450.129674,474.914866,495.126369,510.569535,521.095636,526.603302,527.039490,523.396,513.720716,499.098021,479.668739,455.619985,427.183361,394.632728,358.281567,318.479957,275.611212,230.088180,182.349273,132.854243,82.079754,30.514792,-21.344044,-72.997325,-123.947602,-173.704196,-221.787924,-267.735712,-311.105060,-351.478295,-388.466603,-421.713764,-450.899592,-475.743010,-496.004763,-511.489719,-522.048749,-527.580164,-528.030694,-523.396000,-513.720716,-499.098021,-479.668739,-455.619985,-427.183361,-394.632728,-358.281567,-318.479957,-275.611212,-230.088180,-182.349273,-132.854243,-82.079754,-30.514792,21.344044,72.997325,123.947602,173.704196,221.787924,267.735712,311.105060,351.478295,388.466603,421.713764,450.899592,475.743010,496.004763,511.489719,522.048749,527.580164,528.030694,524.4515,514.771134,500.133240,480.678790,456.595140,428.114229,395.510345,359.097479,319.226309,276.280814,230.674585,182.846833,133.258166,82.386150,30.720710,-21.240587,-72.997325,-124.051059,-173.910114,-222.094319,-268.139635,-311.602619,-352.064700,-389.136205,-422.460116,-451.715505,-476.620626,-496.935631,-512.464874,-523.058799,-528.615383,-529.081112,-524.451500,-514.771134,-500.133240,-480.678790,-456.595140,-428.114229,-395.510345,-359.097479,-319.226309,-276.280814,-230.674585,-182.846833,-133.258166,-82.386150,-30.720710,21.240587,72.997325,124.051059,173.910114,222.094319,268.139635,311.602619,352.064700,389.136205,422.460116,451.715505,476.620626,496.935631,512.464874,523.058799,528.615383,529.081112,525.565,515.879272,501.225344,481.744343,457.623880,429.096248,396.436186,359.958226,320.013672,276.987211,231.293212,183.371733,133.684284,82.709382,30.937943,-21.131445,-72.997325,-124.160201,-174.127347,-222.417551,-268.565753,-312.127519,-352.683327,-389.842602,-423.247479,-452.576252,-477.546468,-497.917650,-513.493613,-524.124352,-529.707488,-530.189250,-525.565000,-515.879272,-501.225344,-481.744343,-457.623880,-429.096248,-396.436186,-359.958226,-320.013672,-276.987211,-231.293212,-183.371733,-133.684284,-82.709382,-30.937943,21.131445,72.997325,124.160201,174.127347,222.417551,268.565753,312.127519,352.683327,389.842602,423.247479,452.576252,477.546468,497.917650,513.493613,524.124352,529.707488,530.189250,526.7365,517.045131,502.374334,482.865398,458.706204,430.129419,397.410253,360.863808,320.842048,277.730403,231.944063,183.923974,134.132597,83.049450,31.166492,-21.016617,-72.997325,-124.275028,-174.355895,-222.757620,-269.014066,-312.679760,-353.334178,-390.585793,-424.075855,-453.481833,-478.520534,-498.950821,-514.575938,-525.245408,-530.856477,-531.355109,-526.736500,-517.045131,-502.374334,-482.865398,-458.706204,-430.129419,-397.410253,-360.863808,-320.842048,-277.730403,-231.944063,-183.923974,-134.132597,-83.049450,-31.166492,21.016617,72.997325,124.275028,174.355895,222.757620,269.014066,312.679760,353.334178,390.585793,424.075855,453.481833,478.520534,498.950821,514.575938,525.245408,530.856477,531.355109,527.964,518.266720,503.578248,484.040042,459.840267,431.211978,398.430882,361.812678,321.710021,278.509121,232.626025,184.502614,134.602341,83.405775,31.405965,-20.896301,-72.997325,-124.395344,-174.595369,-223.113944,-269.483810,-313.258400,-354.016140,-391.364511,-424.943828,-454.430704,-479.541163,-500.033379,-515.710000,-526.420052,-532.060391,-532.576698,-527.964000,-518.266720,-503.578248,-484.040042,-459.840267,-431.211978,-398.430882,-361.812678,-321.710021,-278.509121,-232.626025,-184.502614,-134.602341,-83.405775,-31.405965,20.896301,72.997325,124.395344,174.595369,223.113944,269.483810,313.258400,354.016140,391.364511,424.943828,454.430704,479.541163,500.033379,515.710000,526.420052,532.060391,532.576698,529.24675,519.543293,504.836350,485.267558,461.025373,432.343262,399.497449,362.804257,322.617062,279.322889,233.338683,185.107298,135.093228,83.778137,31.656217,-20.770570,-72.997325,-124.521076,-174.845621,-223.486307,-269.974697,-313.863084,-354.728798,-392.178279,-425.850869,-455.422283,-480.607731,-501.164664,-516.895107,-527.647568,-533.318494,-533.853271,-529.246750,-519.543293,-504.836350,-485.267558,-461.025373,-432.343262,-399.497449,-362.804257,-322.617062,-279.322889,-233.338683,-185.107298,-135.093228,-83.778137,-31.656217,20.770570,72.997325,124.521076,174.845621,223.486307,269.974697,313.863084,354.728798,392.178279,425.850869,455.422283,480.607731,501.164664,516.895107,527.647568,533.318494,533.853271};
  Double_t fullOrientedCrystalCentersZ4[512] = {157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,157.475,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,174.724,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,192.19475,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,209.9095,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,227.89275,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,246.16975,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,264.76825,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716,283.716};

  for ( Int_t iter1=0; iter1<8; iter1++) {

    angle1 = angles4[iter1]*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX4[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY4[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ4[iter1*64]/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog4, iter1*64, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX4[(iter1*64)+iter2]/10.;
      dy = fullOrientedCrystalCentersY4[(iter1*64)+iter2]/10.;
      dz = fullOrientedCrystalCentersZ4[(iter1*64)+iter2]/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog4, (iter1*64)+iter2 , ph2);

    } // iter2
  } // iter1



  AddSensitiveVolume(pcrystalLog4);
  fNbOfSensitiveVol+=512;

  // Shape: testTrap5 type: TGeoTrap
  ddz     = 7.000000;
  theta  = 2.521014;
  phi    = 17.188734;
  h1     = 0.678100;
  bl1    = 2.208800;
  tl1    = 2.208800;
  alpha1 = 0.000000;
  h2     = 0.861400;
  bl2    = 2.801900;
  tl2    = 2.801900;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap5_6 = new TGeoTrap("testTrap5", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog5
  TGeoVolume*
  pcrystalLog5 = new TGeoVolume("crystalLog5",ptestTrap5_6, pCsIMedium);
  pcrystalLog5->SetVisLeaves(kTRUE);

  // code here
  Double_t angles5[8] = {-0.509595345769378,-0.535774156709908,-0.561954575859661,-0.588135731025587,-0.614316941706246,-0.640490427373018,-0.666673930926659,-0.692856121099893};

  Double_t fullOrientedCrystalCentersX5[512] = {-70.153987,-120.398927,-169.484359,-216.937563,-262.301541,-305.139411,-345.038622,-381.614923,-414.516063,-443.425186,-468.063883,-488.194869,-503.624271,-514.203497,-519.830661,-520.451572,-516.060250,-506.698986,-492.457933,-473.474241,-449.930734,-422.054147,-390.112949,-354.414749,-315.303342,-273.155391,-228.376805,-181.398825,-132.673875,-82.671203,-31.872362,19.233427,70.153988,120.398927,169.484359,216.937563,262.301541,305.139411,345.038622,381.614923,414.516063,443.425186,468.063883,488.194869,503.624271,514.203497,519.830661,520.451572,516.060250,506.698986,492.457933,473.474241,449.930734,422.054147,390.112949,354.414749,315.303342,273.155391,228.376805,181.398825,132.673875,82.671203,31.872362,-19.233427,-70.154013,-120.516719,-169.718784,-217.286364,-262.761358,-305.705816,-345.706161,-382.377165,-415.365669,-444.353974,-469.062908,-489.254509,-504.734322,-515.353268,-521.009079,-521.647289,-517.261750,-507.894698,-493.636342,-474.623998,-451.040765,-423.113764,-391.111946,-355.343505,-316.152913,-273.917595,-229.044301,-181.965186,-133.133646,-83.019956,-32.106739,19.115684,70.154013,120.516719,169.718784,217.286364,262.761358,305.705816,345.706161,382.377165,415.365669,444.353974,469.062908,489.254509,504.734322,515.353268,521.009079,521.647289,517.261750,507.894698,493.636342,474.623998,451.040765,423.113764,391.111946,355.343505,316.152913,273.917595,229.044301,181.965186,133.133646,83.019956,32.106739,-19.115684,-70.154275,-120.638767,-169.961441,-217.647294,-263.237085,-306.291758,-346.396675,-383.165602,-416.244435,-445.314606,-470.096155,-490.350420,-505.882343,-516.542342,-522.227756,-522.883832,-518.504250,-509.131189,-494.854916,-475.812920,-452.188585,-424.209427,-392.144901,-356.303804,-317.031308,-274.705626,-229.734379,-182.550665,-133.608887,-83.380383,-32.348881,18.994159,70.154275,120.638767,169.961441,217.647294,263.237085,306.291758,346.396675,383.165602,416.244435,445.314606,470.096155,490.350420,505.882343,516.542342,522.227756,522.883832,518.504250,509.131189,494.854916,475.812920,452.188585,424.209427,392.144901,356.303804,317.031308,274.705626,229.734379,182.550665,133.608887,83.380383,32.348881,-18.994159,-70.154287,-120.764462,-170.211608,-218.019524,-263.727792,-306.896218,-347.109065,-383.979062,-417.151131,-446.305807,-471.162313,-491.481269,-507.066992,-517.769383,-523.485371,-524.159909,-519.786500,-510.407263,-496.112526,-477.039953,-453.373225,-425.340265,-393.211046,-357.294989,-317.937986,-275.519067,-230.446749,-183.155102,-134.099572,-83.752589,-32.599023,18.868489,70.154288,120.764462,170.211608,218.019524,263.727792,306.896218,347.109065,383.979062,417.151131,446.305807,471.162313,491.481269,507.066992,517.769383,523.485371,524.159909,519.786500,510.407263,496.112526,477.039953,453.373225,425.340265,393.211046,357.294989,317.937986,275.519067,230.446749,183.155102,134.099572,83.752589,32.599023,-18.868489,-70.154313,-120.894016,-170.469445,-218.403159,-264.233532,-307.519191,-347.843272,-384.817432,-418.085591,-447.327356,-472.261114,-492.646740,-508.287909,-519.033986,-524.781483,-525.475048,-521.108000,-511.722397,-497.408628,-478.304543,-454.594122,-426.505712,-394.309819,-358.316507,-318.872410,-276.357399,-231.180914,-183.778031,-134.605265,-84.136176,-32.856810,18.738984,70.154313,120.894016,170.469445,218.403159,264.233532,307.519191,347.843272,384.817432,418.085591,447.327356,472.261114,492.646740,508.287909,519.033986,524.781483,525.475048,521.108000,511.722397,497.408628,478.304543,454.594122,426.505712,394.309819,358.316507,318.872410,276.357399,231.180914,183.778031,134.605265,84.136176,32.856810,-18.738984,-70.154337,-121.027271,-170.734646,-218.797752,-264.753717,-308.159959,-348.598452,-385.679751,-419.046743,-448.378086,-473.391303,-493.845503,-509.543701,-520.334715,-526.114621,-526.827755,-522.467250,-513.075100,-498.741756,-479.605256,-455.849896,-427.704452,-395.439980,-359.367205,-319.833528,-277.219678,-231.936052,-184.418755,-135.125404,-84.530722,-33.121962,18.605779,70.154338,121.027271,170.734646,218.797752,264.753717,308.159959,348.598452,385.679751,419.046743,448.378086,473.391303,493.845503,509.543701,520.334715,526.114621,526.827755,522.467250,513.075100,498.741756,479.605256,455.849896,427.704452,395.439980,359.367205,319.833528,277.219678,231.936052,184.418755,135.125404,84.530722,33.121962,-18.605779,-70.15435,-121.164213,-171.007199,-219.203292,-265.288337,-308.818511,-349.374594,-386.566008,-420.034580,-449.457990,-474.552873,-495.077553,-510.834366,-521.671564,-527.484780,-528.218029,-523.864250,-514.465372,-500.111911,-480.942098,-457.140551,-428.936490,-396.601536,-360.447093,-320.821347,-278.105916,-232.712173,-185.077285,-135.660001,-84.936237,-33.394491,18.468862,70.154350,121.164213,171.007199,219.203292,265.288337,308.818511,349.374594,386.566008,420.034580,449.457990,474.552873,495.077553,510.834366,521.671564,527.484780,528.218029,523.864250,514.465372,500.111911,480.942098,457.140551,428.936490,396.601536,360.447093,320.821347,278.105916,232.712173,185.077285,135.660001,84.936237,33.394491,-18.468862,-70.154375,-121.304672,-171.286739,-219.619221,-265.836650,-309.493927,-350.170608,-387.474954,-421.047705,-450.565536,-475.744175,-496.341138,-512.158064,-523.042628,-528.890005,-529.643883,-525.297000,-515.891220,-501.517126,-482.313147,-458.464230,-430.200051,-397.792810,-361.554608,-321.834437,-279.014824,-233.508146,-185.752657,-136.208268,-85.352119,-33.673983,18.328453,70.154375,121.304672,171.286739,219.619221,265.836650,309.493927,350.170608,387.474954,421.047705,450.565536,475.744175,496.341138,512.158064,523.042628,528.890005,529.643883,525.297000,515.891220,501.517126,482.313147,458.464230,430.200051,397.792810,361.554608,321.834437,279.014824,233.508146,185.752657,136.208268,85.352119,33.673983,-18.328453};
  Double_t fullOrientedCrystalCentersY5[512] = {516.06025,506.698986,492.457933,473.474241,449.930734,422.054147,390.112949,354.414749,315.303342,273.155391,228.376805,181.398825,132.673875,82.671203,31.872362,-19.233427,-70.153988,-120.398927,-169.484359,-216.937563,-262.301541,-305.139411,-345.038622,-381.614923,-414.516063,-443.425186,-468.063883,-488.194869,-503.624271,-514.203497,-519.830661,-520.451572,-516.060250,-506.698986,-492.457933,-473.474241,-449.930734,-422.054147,-390.112949,-354.414749,-315.303342,-273.155391,-228.376805,-181.398825,-132.673875,-82.671203,-31.872362,19.233427,70.153988,120.398927,169.484359,216.937563,262.301541,305.139411,345.038622,381.614923,414.516063,443.425186,468.063883,488.194869,503.624271,514.203497,519.830661,520.451572,517.26175,507.894698,493.636342,474.623998,451.040765,423.113764,391.111946,355.343505,316.152913,273.917595,229.044301,181.965186,133.133646,83.019956,32.106739,-19.115684,-70.154013,-120.516719,-169.718784,-217.286364,-262.761358,-305.705816,-345.706161,-382.377165,-415.365669,-444.353974,-469.062908,-489.254509,-504.734322,-515.353268,-521.009079,-521.647289,-517.261750,-507.894698,-493.636342,-474.623998,-451.040765,-423.113764,-391.111946,-355.343505,-316.152913,-273.917595,-229.044301,-181.965186,-133.133646,-83.019956,-32.106739,19.115684,70.154013,120.516719,169.718784,217.286364,262.761358,305.705816,345.706161,382.377165,415.365669,444.353974,469.062908,489.254509,504.734322,515.353268,521.009079,521.647289,518.50425,509.131189,494.854916,475.812920,452.188585,424.209427,392.144901,356.303804,317.031308,274.705626,229.734379,182.550665,133.608887,83.380383,32.348881,-18.994159,-70.154275,-120.638767,-169.961441,-217.647294,-263.237085,-306.291758,-346.396675,-383.165602,-416.244435,-445.314606,-470.096155,-490.350420,-505.882343,-516.542342,-522.227756,-522.883832,-518.504250,-509.131189,-494.854916,-475.812920,-452.188585,-424.209427,-392.144901,-356.303804,-317.031308,-274.705626,-229.734379,-182.550665,-133.608887,-83.380383,-32.348881,18.994159,70.154275,120.638767,169.961441,217.647294,263.237085,306.291758,346.396675,383.165602,416.244435,445.314606,470.096155,490.350420,505.882343,516.542342,522.227756,522.883832,519.7865,510.407263,496.112526,477.039953,453.373225,425.340265,393.211046,357.294989,317.937986,275.519067,230.446749,183.155102,134.099572,83.752589,32.599023,-18.868489,-70.154288,-120.764462,-170.211608,-218.019524,-263.727792,-306.896218,-347.109065,-383.979062,-417.151131,-446.305807,-471.162313,-491.481269,-507.066992,-517.769383,-523.485371,-524.159909,-519.786500,-510.407263,-496.112526,-477.039953,-453.373225,-425.340265,-393.211046,-357.294989,-317.937986,-275.519067,-230.446749,-183.155102,-134.099572,-83.752589,-32.599023,18.868489,70.154288,120.764462,170.211608,218.019524,263.727792,306.896218,347.109065,383.979062,417.151131,446.305807,471.162313,491.481269,507.066992,517.769383,523.485371,524.159909,521.108,511.722397,497.408628,478.304543,454.594122,426.505712,394.309819,358.316507,318.872410,276.357399,231.180914,183.778031,134.605265,84.136176,32.856810,-18.738984,-70.154313,-120.894016,-170.469445,-218.403159,-264.233532,-307.519191,-347.843272,-384.817432,-418.085591,-447.327356,-472.261114,-492.646740,-508.287909,-519.033986,-524.781483,-525.475048,-521.108000,-511.722397,-497.408628,-478.304543,-454.594122,-426.505712,-394.309819,-358.316507,-318.872410,-276.357399,-231.180914,-183.778031,-134.605265,-84.136176,-32.856810,18.738984,70.154313,120.894016,170.469445,218.403159,264.233532,307.519191,347.843272,384.817432,418.085591,447.327356,472.261114,492.646740,508.287909,519.033986,524.781483,525.475048,522.46725,513.075100,498.741756,479.605256,455.849896,427.704452,395.439980,359.367205,319.833528,277.219678,231.936052,184.418755,135.125404,84.530722,33.121962,-18.605779,-70.154338,-121.027271,-170.734646,-218.797752,-264.753717,-308.159959,-348.598452,-385.679751,-419.046743,-448.378086,-473.391303,-493.845503,-509.543701,-520.334715,-526.114621,-526.827755,-522.467250,-513.075100,-498.741756,-479.605256,-455.849896,-427.704452,-395.439980,-359.367205,-319.833528,-277.219678,-231.936052,-184.418755,-135.125404,-84.530722,-33.121962,18.605779,70.154338,121.027271,170.734646,218.797752,264.753717,308.159959,348.598452,385.679751,419.046743,448.378086,473.391303,493.845503,509.543701,520.334715,526.114621,526.827755,523.86425,514.465372,500.111911,480.942098,457.140551,428.936490,396.601536,360.447093,320.821347,278.105916,232.712173,185.077285,135.660001,84.936237,33.394491,-18.468862,-70.154350,-121.164213,-171.007199,-219.203292,-265.288337,-308.818511,-349.374594,-386.566008,-420.034580,-449.457990,-474.552873,-495.077553,-510.834366,-521.671564,-527.484780,-528.218029,-523.864250,-514.465372,-500.111911,-480.942098,-457.140551,-428.936490,-396.601536,-360.447093,-320.821347,-278.105916,-232.712173,-185.077285,-135.660001,-84.936237,-33.394491,18.468862,70.154350,121.164213,171.007199,219.203292,265.288337,308.818511,349.374594,386.566008,420.034580,449.457990,474.552873,495.077553,510.834366,521.671564,527.484780,528.218029,525.297,515.891220,501.517126,482.313147,458.464230,430.200051,397.792810,361.554608,321.834437,279.014824,233.508146,185.752657,136.208268,85.352119,33.673983,-18.328453,-70.154375,-121.304672,-171.286739,-219.619221,-265.836650,-309.493927,-350.170608,-387.474954,-421.047705,-450.565536,-475.744175,-496.341138,-512.158064,-523.042628,-528.890005,-529.643883,-525.297000,-515.891220,-501.517126,-482.313147,-458.464230,-430.200051,-397.792810,-361.554608,-321.834437,-279.014824,-233.508146,-185.752657,-136.208268,-85.352119,-33.673983,18.328453,70.154375,121.304672,171.286739,219.619221,265.836650,309.493927,350.170608,387.474954,421.047705,450.565536,475.744175,496.341138,512.158064,523.042628,528.890005,529.643883};
  Double_t fullOrientedCrystalCentersZ5[512] = {297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,297.3205,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,316.1665,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,335.374,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,354.970125,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,374.9825,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,395.44175,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,416.38075,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345,437.8345};

  for ( Int_t iter1=0; iter1<8; iter1++) {

    angle1 = angles5[iter1]*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX5[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY5[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ5[iter1*64]/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog5, iter1*64, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX5[(iter1*64)+iter2]/10.;
      dy = fullOrientedCrystalCentersY5[(iter1*64)+iter2]/10.;
      dz = fullOrientedCrystalCentersZ5[(iter1*64)+iter2]/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog5, (iter1*64)+iter2 , ph2);

    } // iter2
  } // iter1


  AddSensitiveVolume(pcrystalLog5);
  fNbOfSensitiveVol+=512;


  // Shape: testTrap6 type: TGeoTrap
  ddz     = 7.000000;
  theta  = 2.177240;
  phi    = 15.928227;
  h1     = 0.623400;
  bl1    = 2.206000;
  tl1    = 2.206000;
  alpha1 = 0.000000;
  h2     = 0.770100;
  bl2    = 2.719400;
  tl2    = 2.719400;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap6_7 = new TGeoTrap("testTrap6", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog6
  TGeoVolume*
  pcrystalLog6 = new TGeoVolume("crystalLog6",ptestTrap6_7, pCsIMedium);
  pcrystalLog6->SetVisLeaves(kTRUE);

  // code here
  Double_t angles6[8] = {-0.710043552064485,-0.730995028158828,-0.751931741939961,-0.772882021870728,-0.793826995857944,-0.814766111617657,-0.835708729140811,-0.856654241636051};

  Double_t fullOrientedCrystalCentersX6[512] = {-69.72755,-119.204275,-167.532998,-214.248286,-258.900246,-301.058856,-340.318104,-376.299902,-408.657727,-437.079955,-461.292863,-481.063269,-496.200773,-506.559593,-512.039966,-512.589115,-508.201750,-498.920125,-484.833626,-466.077914,-442.833617,-415.324590,-383.815761,-348.610576,-310.048080,-268.499652,-224.365426,-178.070438,-130.060534,-80.798076,-30.757488,19.579311,69.727550,119.204275,167.532998,214.248286,258.900246,301.058856,340.318104,376.299902,408.657727,437.079955,461.292863,481.063269,496.200773,506.559593,512.039966,512.589115,508.201750,498.920125,484.833626,466.077914,442.833617,415.324590,383.815761,348.610576,310.048080,268.499652,224.365426,178.070438,130.060534,80.798076,30.757488,-19.579311,-69.727587,-119.319213,-167.761729,-214.588608,-259.348882,-301.611484,-340.969402,-377.043599,-409.486660,-437.986140,-462.267574,-482.097119,-497.283806,-507.681377,-513.189699,-513.755724,-509.374000,-500.086726,-485.983344,-467.199676,-443.916620,-416.358405,-384.790430,-349.516713,-310.876960,-269.243291,-225.016662,-178.622999,-130.509100,-81.138326,-30.986146,19.464447,69.727588,119.319213,167.761729,214.588608,259.348882,301.611484,340.969402,377.043599,409.486660,437.986140,462.267574,482.097119,497.283806,507.681377,513.189699,513.755724,509.374000,500.086726,485.983344,467.199676,443.916620,416.358405,384.790430,349.516713,310.876960,269.243291,225.016662,178.622999,130.509100,81.138326,30.986146,-19.464447,-69.7276,-119.436282,-167.994728,-214.935293,-259.805913,-302.174460,-341.632902,-377.801233,-410.331131,-438.909316,-463.260564,-483.150360,-498.387153,-508.824206,-514.361004,-514.944224,-510.568250,-501.275224,-487.154644,-468.342499,-445.019959,-417.411633,-385.783406,-350.439873,-311.721413,-270.000905,-225.680141,-179.185954,-130.966108,-81.484987,-31.219121,19.347403,69.727600,119.436282,167.994728,214.935293,259.805913,302.174460,341.632902,377.801233,410.331131,438.909316,463.260564,483.150360,498.387153,508.824206,514.361004,514.944224,510.568250,501.275224,487.154644,468.342499,445.019959,417.411633,385.783406,350.439873,311.721413,270.000905,225.680141,179.185954,130.966108,81.484987,31.219121,-19.347403,-69.727612,-119.555459,-168.231921,-215.288218,-260.271172,-302.747572,-342.308347,-378.572506,-411.190805,-439.849111,-464.271430,-484.222561,-499.510365,-509.987610,-515.553396,-516.154121,-511.784000,-502.485119,-488.347032,-469.505895,-446.143161,-418.483823,-386.794258,-351.379653,-312.581069,-270.772159,-226.355565,-179.759043,-131.431344,-81.837888,-31.456289,19.228251,69.727613,119.555459,168.231921,215.288218,260.271172,302.747572,342.308347,378.572506,411.190805,439.849111,464.271430,484.222561,499.510365,509.987610,515.553396,516.154121,511.784000,502.485119,488.347032,469.505895,446.143161,418.483823,386.794258,351.379653,312.581069,270.772159,226.355565,179.759043,131.431344,81.837888,31.456289,-19.228251,-69.727675,-119.676720,-168.473212,-215.647215,-260.744418,-303.330509,-342.995362,-379.356982,-412.065186,-440.804978,-465.299577,-485.313086,-500.652766,-511.170885,-516.766150,-517.384673,-513.020500,-503.715659,-489.559760,-470.689134,-447.285514,-419.574290,-387.822336,-352.335441,-313.455363,-271.556538,-227.042476,-180.341870,-131.904474,-82.196766,-31.697457,19.107115,69.727675,119.676720,168.473212,215.647215,260.744418,303.330509,342.995362,379.356982,412.065186,440.804978,465.299577,485.313086,500.652766,511.170885,516.766150,517.384673,513.020500,503.715659,489.559760,470.689134,447.285514,419.574290,387.822336,352.335441,313.455363,271.556538,227.042476,180.341870,131.904474,82.196766,31.697457,-19.107115,-69.727688,-119.799964,-168.718501,-216.012187,-261.225558,-303.923183,-343.693863,-380.154582,-412.954205,-441.776853,-466.344949,-486.421888,-501.814318,-512.374002,-517.999244,-518.635871,-514.277750,-504.966854,-490.792850,-471.892243,-448.447056,-420.683079,-388.867694,-353.307300,-314.344364,-272.354119,-227.740956,-180.934523,-132.385591,-82.561714,-31.942722,18.983895,69.727688,119.799964,168.718501,216.012187,261.225558,303.923183,343.693863,380.154582,412.954205,441.776853,466.344949,486.421888,501.814318,512.374002,517.999244,518.635871,514.277750,504.966854,490.792850,471.892243,448.447056,420.683079,388.867694,353.307300,314.344364,272.354119,227.740956,180.934523,132.385591,82.561714,31.942722,-18.983895,-69.7277,-119.925120,-168.967595,-216.382820,-261.714161,-304.525050,-344.403197,-380.964553,-413.857013,-442.763802,-467.406535,-487.547887,-502.993886,-513.595779,-519.251464,-519.906474,-515.554500,-506.237454,-492.045065,-473.114013,-449.626615,-421.809066,-389.929266,-354.294233,-315.247154,-273.164071,-228.450270,-181.536368,-132.874171,-82.932323,-32.191792,18.858764,69.727700,119.925120,168.967595,216.382820,261.714161,304.525050,344.403197,380.964553,413.857013,442.763802,467.406535,487.547887,502.993886,513.595779,519.251464,519.906474,515.554500,506.237454,492.045065,473.114013,449.626615,421.809066,389.929266,354.294233,315.247154,273.164071,228.450270,181.536368,132.874171,82.932323,32.191792,-18.858764,-69.727775,-120.052200,-169.220457,-216.759028,-262.210092,-305.135929,-345.123140,-381.786626,-414.773299,-443.765478,-468.483953,-488.690672,-504.191032,-514.835756,-520.522331,-521.195992,-516.850250,-507.526958,-493.315903,-474.353947,-450.823703,-422.951780,-391.006601,-355.295814,-316.163334,-273.986029,-229.170088,-182.147114,-133.369964,-83.308388,-32.444506,18.731833,69.727775,120.052200,169.220457,216.759028,262.210092,305.135929,345.123140,381.786626,414.773299,443.765478,468.483953,488.690672,504.191032,514.835756,520.522331,521.195992,516.850250,507.526958,493.315903,474.353947,450.823703,422.951780,391.006601,355.295814,316.163334,273.986029,229.170088,182.147114,133.369964,83.308388,32.444506,-18.731833};
  Double_t fullOrientedCrystalCentersY6[512] = {508.20175,498.920125,484.833626,466.077914,442.833617,415.324590,383.815761,348.610576,310.048080,268.499652,224.365426,178.070438,130.060534,80.798076,30.757488,-19.579311,-69.727550,-119.204275,-167.532998,-214.248286,-258.900246,-301.058856,-340.318104,-376.299902,-408.657727,-437.079955,-461.292863,-481.063269,-496.200773,-506.559593,-512.039966,-512.589115,-508.201750,-498.920125,-484.833626,-466.077914,-442.833617,-415.324590,-383.815761,-348.610576,-310.048080,-268.499652,-224.365426,-178.070438,-130.060534,-80.798076,-30.757488,19.579311,69.727550,119.204275,167.532998,214.248286,258.900246,301.058856,340.318104,376.299902,408.657727,437.079955,461.292863,481.063269,496.200773,506.559593,512.039966,512.589115,509.374,500.086726,485.983344,467.199676,443.916620,416.358405,384.790430,349.516713,310.876960,269.243291,225.016662,178.622999,130.509100,81.138326,30.986146,-19.464447,-69.727588,-119.319213,-167.761729,-214.588608,-259.348882,-301.611484,-340.969402,-377.043599,-409.486660,-437.986140,-462.267574,-482.097119,-497.283806,-507.681377,-513.189699,-513.755724,-509.374000,-500.086726,-485.983344,-467.199676,-443.916620,-416.358405,-384.790430,-349.516713,-310.876960,-269.243291,-225.016662,-178.622999,-130.509100,-81.138326,-30.986146,19.464447,69.727588,119.319213,167.761729,214.588608,259.348882,301.611484,340.969402,377.043599,409.486660,437.986140,462.267574,482.097119,497.283806,507.681377,513.189699,513.755724,510.56825,501.275224,487.154644,468.342499,445.019959,417.411633,385.783406,350.439873,311.721413,270.000905,225.680141,179.185954,130.966108,81.484987,31.219121,-19.347403,-69.727600,-119.436282,-167.994728,-214.935293,-259.805913,-302.174460,-341.632902,-377.801233,-410.331131,-438.909316,-463.260564,-483.150360,-498.387153,-508.824206,-514.361004,-514.944224,-510.568250,-501.275224,-487.154644,-468.342499,-445.019959,-417.411633,-385.783406,-350.439873,-311.721413,-270.000905,-225.680141,-179.185954,-130.966108,-81.484987,-31.219121,19.347403,69.727600,119.436282,167.994728,214.935293,259.805913,302.174460,341.632902,377.801233,410.331131,438.909316,463.260564,483.150360,498.387153,508.824206,514.361004,514.944224,511.784,502.485119,488.347032,469.505895,446.143161,418.483823,386.794258,351.379653,312.581069,270.772159,226.355565,179.759043,131.431344,81.837888,31.456289,-19.228251,-69.727613,-119.555459,-168.231921,-215.288218,-260.271172,-302.747572,-342.308347,-378.572506,-411.190805,-439.849111,-464.271430,-484.222561,-499.510365,-509.987610,-515.553396,-516.154121,-511.784000,-502.485119,-488.347032,-469.505895,-446.143161,-418.483823,-386.794258,-351.379653,-312.581069,-270.772159,-226.355565,-179.759043,-131.431344,-81.837888,-31.456289,19.228251,69.727613,119.555459,168.231921,215.288218,260.271172,302.747572,342.308347,378.572506,411.190805,439.849111,464.271430,484.222561,499.510365,509.987610,515.553396,516.154121,513.0205,503.715659,489.559760,470.689134,447.285514,419.574290,387.822336,352.335441,313.455363,271.556538,227.042476,180.341870,131.904474,82.196766,31.697457,-19.107115,-69.727675,-119.676720,-168.473212,-215.647215,-260.744418,-303.330509,-342.995362,-379.356982,-412.065186,-440.804978,-465.299577,-485.313086,-500.652766,-511.170885,-516.766150,-517.384673,-513.020500,-503.715659,-489.559760,-470.689134,-447.285514,-419.574290,-387.822336,-352.335441,-313.455363,-271.556538,-227.042476,-180.341870,-131.904474,-82.196766,-31.697457,19.107115,69.727675,119.676720,168.473212,215.647215,260.744418,303.330509,342.995362,379.356982,412.065186,440.804978,465.299577,485.313086,500.652766,511.170885,516.766150,517.384673,514.27775,504.966854,490.792850,471.892243,448.447056,420.683079,388.867694,353.307300,314.344364,272.354119,227.740956,180.934523,132.385591,82.561714,31.942722,-18.983895,-69.727688,-119.799964,-168.718501,-216.012187,-261.225558,-303.923183,-343.693863,-380.154582,-412.954205,-441.776853,-466.344949,-486.421888,-501.814318,-512.374002,-517.999244,-518.635871,-514.277750,-504.966854,-490.792850,-471.892243,-448.447056,-420.683079,-388.867694,-353.307300,-314.344364,-272.354119,-227.740956,-180.934523,-132.385591,-82.561714,-31.942722,18.983895,69.727688,119.799964,168.718501,216.012187,261.225558,303.923183,343.693863,380.154582,412.954205,441.776853,466.344949,486.421888,501.814318,512.374002,517.999244,518.635871,515.5545,506.237454,492.045065,473.114013,449.626615,421.809066,389.929266,354.294233,315.247154,273.164071,228.450270,181.536368,132.874171,82.932323,32.191792,-18.858764,-69.727700,-119.925120,-168.967595,-216.382820,-261.714161,-304.525050,-344.403197,-380.964553,-413.857013,-442.763802,-467.406535,-487.547887,-502.993886,-513.595779,-519.251464,-519.906474,-515.554500,-506.237454,-492.045065,-473.114013,-449.626615,-421.809066,-389.929266,-354.294233,-315.247154,-273.164071,-228.450270,-181.536368,-132.874171,-82.932323,-32.191792,18.858764,69.727700,119.925120,168.967595,216.382820,261.714161,304.525050,344.403197,380.964553,413.857013,442.763802,467.406535,487.547887,502.993886,513.595779,519.251464,519.906474,516.85025,507.526958,493.315903,474.353947,450.823703,422.951780,391.006601,355.295814,316.163334,273.986029,229.170088,182.147114,133.369964,83.308388,32.444506,-18.731833,-69.727775,-120.052200,-169.220457,-216.759028,-262.210092,-305.135929,-345.123140,-381.786626,-414.773299,-443.765478,-468.483953,-488.690672,-504.191032,-514.835756,-520.522331,-521.195992,-516.850250,-507.526958,-493.315903,-474.353947,-450.823703,-422.951780,-391.006601,-355.295814,-316.163334,-273.986029,-229.170088,-182.147114,-133.369964,-83.308388,-32.444506,18.731833,69.727775,120.052200,169.220457,216.759028,262.210092,305.135929,345.123140,381.786626,414.773299,443.765478,468.483953,488.690672,504.191032,514.835756,520.522331,521.195992};
  Double_t fullOrientedCrystalCentersZ6[512] = {446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,446.175125,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,466.209875,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,486.67775,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,507.60575,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,529.02125,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,550.956,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,573.44275,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165,596.5165};


  for ( Int_t iter1=0; iter1<8; iter1++) {

    angle1 = angles6[iter1]*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX6[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY6[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ6[iter1*64]/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog6, iter1*64, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX6[(iter1*64)+iter2]/10.;
      dy = fullOrientedCrystalCentersY6[(iter1*64)+iter2]/10.;
      dz = fullOrientedCrystalCentersZ6[(iter1*64)+iter2]/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog6, (iter1*64)+iter2 , ph2);

    } // iter2
  } // iter1


  AddSensitiveVolume(pcrystalLog6);
  fNbOfSensitiveVol+=512;



  // END CAP Definition here ...


  // Shape: testTrap7 type: TGeoTrap
  ddz     = 7.500000;
  theta  = 1.895546;
  phi    = 18.449112;
  h1     = 0.664923;
  bl1    = 2.170695;
  tl1    = 2.120681;
  alpha1 = 0.000000;
  h2     = 0.822026;
  bl2    = 2.647524;
  tl2    = 2.585692;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap7_8 = new TGeoTrap("testTrap7", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog7
  TGeoVolume*
  pcrystalLog7 = new TGeoVolume("crystalLog7",ptestTrap7_8, pCsIMedium);
  pcrystalLog7->SetVisLeaves(kTRUE);

  // code here

  Double_t fullOrientedCrystalCentersX7[64] = {-69.612256316,-118.407946335,-166.063303102,-212.119379481,-256.132630279,-297.679183832,-336.358924116,-371.799344088,-403.659133130,-431.631464058,-455.446948034,-474.876228927,-489.732192142,-499.871766631,-505.197302750,-505.657512675,-501.247964332,-492.011124083,-478.035947748,-459.457023916,-436.453277778,-409.246247986,-378.097953108,-343.308368253,-305.212536140,-264.177340459,-220.597972574,-174.894125622,-127.505952634,-78.889827620,-29.513950441,20.146162215,69.612256316,118.407946335,166.063303102,212.119379481,256.132630279,297.679183832,336.358924116,371.799344088,403.659133130,431.631464058,455.446948034,474.876228927,489.732192142,499.871766631,505.197302750,505.657512675,501.247964332,492.011124083,478.035947748,459.457023916,436.453277778,409.246247985,378.097953108,343.308368253,305.212536140,264.177340458,220.597972574,174.894125622,127.505952634,78.889827620,29.513950441,-20.146162215};
  Double_t fullOrientedCrystalCentersY7[64] = {501.247964333,492.011124083,478.035947748,459.457023916,436.453277778,409.246247986,378.097953108,343.308368253,305.212536141,264.177340459,220.597972574,174.894125622,127.505952634,78.889827620,29.513950441,-20.146162215,-69.612256316,-118.407946335,-166.063303102,-212.119379481,-256.132630279,-297.679183832,-336.358924116,-371.799344088,-403.659133130,-431.631464058,-455.446948034,-474.876228927,-489.732192142,-499.871766631,-505.197302750,-505.657512675,-501.247964332,-492.011124083,-478.035947748,-459.457023916,-436.453277778,-409.246247986,-378.097953108,-343.308368253,-305.212536140,-264.177340458,-220.597972574,-174.894125622,-127.505952634,-78.889827620,-29.513950441,20.146162215,69.612256316,118.407946335,166.063303102,212.119379481,256.132630279,297.679183832,336.358924116,371.799344088,403.659133130,431.631464058,455.446948034,474.876228927,489.732192142,499.871766631,505.197302750,505.657512675};
  Double_t fullOrientedCrystalCentersZ7 = 605.80659895;



  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 = -0.86909295809965*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX7[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY7[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ7/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog7, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX7[iter2]/10.;
      dy = fullOrientedCrystalCentersY7[iter2]/10.;
      dz = fullOrientedCrystalCentersZ7/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog7, iter2 , ph2);

    } // iter2
  } // iter1



  AddSensitiveVolume(pcrystalLog7);
  fNbOfSensitiveVol+=64;



  // Shape: testTrap8 type: TGeoTrap
  ddz     = 7.500000;
  theta  = 1.855281;
  phi    = 18.893562;
  h1     = 0.664947;
  bl1    = 2.114640;
  tl1    = 2.063660;
  alpha1 = 0.000000;
  h2     = 0.822006;
  bl2    = 2.579540;
  tl2    = 2.516560;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap8_9 = new TGeoTrap("testTrap8", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog8
  TGeoVolume*
  pcrystalLog8 = new TGeoVolume("crystalLog8",ptestTrap8_9, pCsIMedium);
  pcrystalLog8->SetVisLeaves(kTRUE);

  Double_t fullOrientedCrystalCentersX8[64] = {-68.987,-116.541009134,-162.972664642,-207.834804300,-250.695381179,-291.141624492,-328.784014808,-363.260035329,-394.237663132,-421.418566727,-444.540979154,-463.382218940,-477.760834647,-487.538352346,-492.620609197,-492.958660287,-488.549250000,-479.434843367,-465.703217107,-447.486614287,-424.960470750,-398.341725573,-367.886731823,-333.888787738,-296.675312105,-256.604691037,-214.062826520,-169.459419965,-123.224026560,-75.801918418,-27.649796364,20.768608344,68.987000000,116.541009134,162.972664643,207.834804300,250.695381179,291.141624492,328.784014808,363.260035329,394.237663132,421.418566727,444.540979154,463.382218940,477.760834647,487.538352346,492.620609197,492.958660287,488.549250000,479.434843367,465.703217107,447.486614287,424.960470750,398.341725573,367.886731823,333.888787738,296.675312105,256.604691037,214.062826520,169.459419965,123.224026560,75.801918418,27.649796364,-20.768608344};
  Double_t fullOrientedCrystalCentersY8[64] = {488.54925,479.434843367,465.703217107,447.486614287,424.960470750,398.341725573,367.886731823,333.888787738,296.675312105,256.604691037,214.062826520,169.459419965,123.224026560,75.801918418,27.649796364,-20.768608344,-68.987000000,-116.541009134,-162.972664642,-207.834804300,-250.695381179,-291.141624492,-328.784014808,-363.260035329,-394.237663132,-421.418566727,-444.540979154,-463.382218940,-477.760834647,-487.538352346,-492.620609197,-492.958660287,-488.549250000,-479.434843367,-465.703217107,-447.486614287,-424.960470750,-398.341725573,-367.886731823,-333.888787738,-296.675312105,-256.604691037,-214.062826520,-169.459419965,-123.224026560,-75.801918418,-27.649796364,20.768608344,68.987000000,116.541009134,162.972664643,207.834804300,250.695381179,291.141624493,328.784014808,363.260035329,394.237663132,421.418566727,444.540979154,463.382218940,477.760834647,487.538352347,492.620609197,492.958660287};
  Double_t fullOrientedCrystalCentersZ8 = 616.29225;
  // -0.890042669315315


  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 = -0.890042669315315*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX8[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY8[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ8/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog8, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX8[iter2]/10.;
      dy = fullOrientedCrystalCentersY8[iter2]/10.;
      dz = fullOrientedCrystalCentersZ8/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog8, iter2 , ph2);

    } // iter2
  } // iter1


  AddSensitiveVolume(pcrystalLog8);
  fNbOfSensitiveVol+=64;


  // Shape: testTrap9 type: TGeoTrap
  ddz     = 7.500000;
  theta  = 1.806107;
  phi    = 19.375136;
  h1     = 0.664930;
  bl1    = 2.057640;
  tl1    = 2.005660;
  alpha1 = 0.000000;
  h2     = 0.822020;
  bl2    = 2.510440;
  tl2    = 2.446260;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap9_10 = new TGeoTrap("testTrap9", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog9
  TGeoVolume*
  pcrystalLog9 = new TGeoVolume("crystalLog9",ptestTrap9_10, pCsIMedium);
  pcrystalLog9->SetVisLeaves(kTRUE);



  Double_t fullOrientedCrystalCentersX9[64] = {-68.351,-114.642131272,-159.829196150,-203.477018498,-245.165245925,-284.492398014,-321.079732790,-354.574894219,-384.655305587,-411.031276088,-433.448790708,-451.691956526,-465.585081882,-474.994368386,-479.829199463,-480.043013048,-475.633750000,-466.643873931,-453.159962263,-435.311872436,-413.271491311,-387.251079808,-357.501228713,-324.308445356,-287.992394385,-248.902819224,-207.416173849,-163.931997335,-118.869066072,-72.661360722,-25.753886748,21.401611234,68.351000000,114.642131272,159.829196150,203.477018498,245.165245925,284.492398014,321.079732790,354.574894219,384.655305587,411.031276088,433.448790708,451.691956526,465.585081882,474.994368386,479.829199463,480.043013048,475.633750000,466.643873931,453.159962263,435.311872435,413.271491311,387.251079808,357.501228713,324.308445356,287.992394385,248.902819224,207.416173849,163.931997335,118.869066072,72.661360722,25.753886748,-21.401611234};
  Double_t fullOrientedCrystalCentersY9[64] = {475.63375,466.643873931,453.159962263,435.311872436,413.271491311,387.251079808,357.501228713,324.308445356,287.992394385,248.902819224,207.416173849,163.931997335,118.869066072,72.661360722,25.753886748,-21.401611234,-68.351000000,-114.642131272,-159.829196150,-203.477018498,-245.165245925,-284.492398014,-321.079732790,-354.574894219,-384.655305587,-411.031276088,-433.448790708,-451.691956526,-465.585081882,-474.994368386,-479.829199463,-480.043013048,-475.633750000,-466.643873931,-453.159962263,-435.311872436,-413.271491311,-387.251079808,-357.501228713,-324.308445356,-287.992394385,-248.902819224,-207.416173849,-163.931997335,-118.869066072,-72.661360722,-25.753886748,21.401611234,68.351000000,114.642131272,159.829196150,203.477018498,245.165245925,284.492398014,321.079732790,354.574894219,384.655305587,411.031276088,433.448790708,451.691956526,465.585081882,474.994368386,479.829199463,480.043013048};
  Double_t fullOrientedCrystalCentersZ9 = 626.50975;
  //-0.910979867092162


  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 =  -0.910979867092162*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX9[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY9[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ9/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog9, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX9[iter2]/10.;
      dy = fullOrientedCrystalCentersY9[iter2]/10.;
      dz = fullOrientedCrystalCentersZ9/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog9, iter2 , ph2);

    } // iter2
  } // iter1





  AddSensitiveVolume(pcrystalLog9);
  fNbOfSensitiveVol+=64;



  // Shape: testTrap10 type: TGeoTrap
  ddz     = 7.500000;
  theta  = 1.759525;
  phi    = 19.887891;
  h1     = 0.664898;
  bl1    = 1.999640;
  tl1    = 1.946760;
  alpha1 = 0.000000;
  h2     = 0.822000;
  bl2    = 2.440140;
  tl2    = 2.374810;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap10_11 = new TGeoTrap("testTrap10", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog10
  TGeoVolume*
  pcrystalLog10 = new TGeoVolume("crystalLog10",ptestTrap10_11, pCsIMedium);
  pcrystalLog10->SetVisLeaves(kTRUE);


  Double_t fullOrientedCrystalCentersX10[64] = {-67.704375,-112.711973451,-156.634093984,-199.047742566,-239.544452577,-277.734218561,-313.249252196,-345.747524292,-374.916058725,-400.473946562,-422.175051372,-439.810379654,-453.210093554,-462.245146504,-466.828526004,-466.916091604,-462.507000000,-453.643713154,-440.411589363,-422.938061213,-401.391408332,-375.979136766,-346.945980582,-314.571544945,-279.167613368,-241.075145065,-200.660991331,-158.314362557,-114.443079929,-69.469647880,-23.827185146,22.044746407,67.704375000,112.711973451,156.634093984,199.047742566,239.544452577,277.734218561,313.249252196,345.747524292,374.916058725,400.473946562,422.175051372,439.810379654,453.210093554,462.245146504,466.828526004,466.916091604,462.507000000,453.643713154,440.411589363,422.938061213,401.391408332,375.979136766,346.945980582,314.571544945,279.167613368,241.075145065,200.660991331,158.314362557,114.443079929,69.469647880,23.827185146,-22.044746407};
  Double_t fullOrientedCrystalCentersY10[64] = {462.507,453.643713154,440.411589363,422.938061213,401.391408332,375.979136766,346.945980582,314.571544945,279.167613368,241.075145065,200.660991331,158.314362557,114.443079929,69.469647880,23.827185146,-22.044746407,-67.704375000,-112.711973451,-156.634093984,-199.047742566,-239.544452577,-277.734218561,-313.249252196,-345.747524292,-374.916058725,-400.473946562,-422.175051372,-439.810379654,-453.210093554,-462.245146504,-466.828526004,-466.916091604,-462.507000000,-453.643713154,-440.411589363,-422.938061213,-401.391408332,-375.979136766,-346.945980582,-314.571544945,-279.167613368,-241.075145065,-200.660991331,-158.314362557,-114.443079929,-69.469647880,-23.827185146,22.044746407,67.704375000,112.711973451,156.634093984,199.047742566,239.544452577,277.734218561,313.249252196,345.747524292,374.916058725,400.473946562,422.175051372,439.810379654,453.210093554,462.245146504,466.828526004,466.916091604};
  Double_t fullOrientedCrystalCentersZ10 = 636.4545;
  // -0.931925919486113


  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 =  -0.931925919486113*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX10[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY10[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ10/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog10, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX10[iter2]/10.;
      dy = fullOrientedCrystalCentersY10[iter2]/10.;
      dz = fullOrientedCrystalCentersZ10/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog10, iter2 , ph2);

    } // iter2
  } // iter1




  AddSensitiveVolume(pcrystalLog10);
  fNbOfSensitiveVol+=64;

  // Shape: testTrap11 type: TGeoTrap
  ddz     = 7.500000;
  theta  = 1.717315;
  phi    = 20.429026;
  h1     = 0.664963;
  bl1    = 1.940587;
  tl1    = 1.887213;
  alpha1 = 0.000000;
  h2     = 0.822026;
  bl2    = 2.368587;
  tl2    = 2.302563;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap11_12 = new TGeoTrap("testTrap11", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog11
  TGeoVolume*
  pcrystalLog11 = new TGeoVolume("crystalLog11",ptestTrap11_12, pCsIMedium);
  pcrystalLog11->SetVisLeaves(kTRUE);


  Double_t fullOrientedCrystalCentersX11[64] = {-67.047375,-110.751348067,-153.388725109,-194.548884877,-233.835432533,-270.870017146,-305.295975421,-336.781766561,-365.024165186,-389.751181557,-410.724680991,-427.742677221,-440.641277642,-449.296261680,-453.624277108,-453.583642771,-449.174750000,-440.440058843,-427.463689150,-410.370610455,-389.325438450,-364.530849646,-336.225629486,-304.682372716,-270.204858139,-233.125123070,-193.800265627,-152.609005683,-109.948037591,-66.228209792,-21.870568129,22.697699060,67.047375000,110.751348067,153.388725109,194.548884877,233.835432533,270.870017146,305.295975421,336.781766561,365.024165186,389.751181557,410.724680991,427.742677221,440.641277642,449.296261680,453.624277108,453.583642771,449.174750000,440.440058843,427.463689150,410.370610455,389.325438450,364.530849646,336.225629486,304.682372716,270.204858139,233.125123070,193.800265627,152.609005683,109.948037590,66.228209792,21.870568129,-22.697699060};
  Double_t fullOrientedCrystalCentersY11[64] = {449.17475,440.440058843,427.463689150,410.370610455,389.325438450,364.530849646,336.225629486,304.682372716,270.204858139,233.125123070,193.800265627,152.609005683,109.948037591,66.228209792,21.870568129,-22.697699060,-67.047375000,-110.751348067,-153.388725109,-194.548884877,-233.835432533,-270.870017146,-305.295975421,-336.781766561,-365.024165186,-389.751181557,-410.724680991,-427.742677221,-440.641277642,-449.296261680,-453.624277108,-453.583642771,-449.174750000,-440.440058843,-427.463689150,-410.370610455,-389.325438450,-364.530849646,-336.225629486,-304.682372716,-270.204858139,-233.125123070,-193.800265627,-152.609005683,-109.948037590,-66.228209792,-21.870568129,22.697699060,67.047375000,110.751348067,153.388725109,194.548884877,233.835432533,270.870017146,305.295975421,336.781766562,365.024165186,389.751181557,410.724680991,427.742677221,440.641277642,449.296261680,453.624277108,453.583642771};
  Double_t fullOrientedCrystalCentersZ11 = 646.122125;
// -0.952872560394749


  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 =  -0.952872560394749*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX11[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY11[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ11/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog11, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX11[iter2]/10.;
      dy = fullOrientedCrystalCentersY11[iter2]/10.;
      dz = fullOrientedCrystalCentersZ11/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog11, iter2 , ph2);

    } // iter2
  } // iter1





  AddSensitiveVolume(pcrystalLog11);
  fNbOfSensitiveVol+=64;



  // Shape: testTrap12 type: TGeoTrap
  ddz     = 7.500000;
  theta  = 1.669896;
  phi    = 21.019377;
  h1     = 0.664895;
  bl1    = 1.880788;
  tl1    = 1.826562;
  alpha1 = 0.000000;
  h2     = 0.822006;
  bl2    = 2.296088;
  tl2    = 2.229012;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap12_13 = new TGeoTrap("testTrap12", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog12
  TGeoVolume*
  pcrystalLog12 = new TGeoVolume("crystalLog12",ptestTrap12_13, pCsIMedium);
  pcrystalLog12->SetVisLeaves(kTRUE);


  Double_t fullOrientedCrystalCentersX12[64] = {-66.381125,-108.761987308,-150.095412223,-189.983336268,-228.041616929,-263.903732159,-297.224310184,-327.682455622,-354.984839883,-378.868526082,-399.103501264,-415.494891557,-427.884838911,-436.154021361,-440.222802159,-440.051996722,-435.643250000,-427.039020633,-414.322172055,-397.615174468,-377.078925392,-352.911200133,-325.344747095,-294.645046292,-261.107752623,-225.055848560,-186.836533648,-146.817880781,-105.385291464,-62.937784180,-19.884151630,23.360976171,66.381125000,108.761987308,150.095412223,189.983336268,228.041616929,263.903732159,297.224310184,327.682455622,354.984839884,378.868526082,399.103501264,415.494891557,427.884838911,436.154021361,440.222802159,440.051996722,435.643250000,427.039020633,414.322172055,397.615174468,377.078925392,352.911200133,325.344747095,294.645046292,261.107752623,225.055848560,186.836533648,146.817880781,105.385291464,62.937784180,19.884151630,-23.360976171};
  Double_t fullOrientedCrystalCentersY12[64] = {435.64325,427.039020633,414.322172055,397.615174468,377.078925392,352.911200133,325.344747095,294.645046292,261.107752623,225.055848560,186.836533648,146.817880781,105.385291464,62.937784181,19.884151630,-23.360976170,-66.381125000,-108.761987308,-150.095412223,-189.983336268,-228.041616929,-263.903732159,-297.224310184,-327.682455622,-354.984839883,-378.868526082,-399.103501264,-415.494891557,-427.884838911,-436.154021361,-440.222802159,-440.051996722,-435.643250000,-427.039020633,-414.322172055,-397.615174468,-377.078925392,-352.911200133,-325.344747095,-294.645046292,-261.107752623,-225.055848560,-186.836533648,-146.817880781,-105.385291464,-62.937784180,-19.884151630,23.360976171,66.381125000,108.761987308,150.095412223,189.983336268,228.041616929,263.903732159,297.224310184,327.682455622,354.984839884,378.868526082,399.103501264,415.494891557,427.884838911,436.154021361,440.222802159,440.051996722};
  Double_t fullOrientedCrystalCentersZ12 = 655.5085;
  // -0.973809310466648

  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 =  -0.973809310466648*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX12[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY12[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ12/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog12, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX12[iter2]/10.;
      dy = fullOrientedCrystalCentersY12[iter2]/10.;
      dz = fullOrientedCrystalCentersZ12/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog12, iter2 , ph2);

    } // iter2
  } // iter1




  AddSensitiveVolume(pcrystalLog12);
  fNbOfSensitiveVol+=64;


  // Shape: testTrap13 type: TGeoTrap
  ddz     = 7.500000;
  theta  = 1.631135;
  phi    = 21.642030;
  h1     = 0.664947;
  bl1    = 1.820138;
  tl1    = 1.765012;
  alpha1 = 0.000000;
  h2     = 0.822021;
  bl2    = 2.222488;
  tl2    = 2.154412;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap13_14 = new TGeoTrap("testTrap13", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog13
  TGeoVolume*
  pcrystalLog13 = new TGeoVolume("crystalLog13",ptestTrap13_14, pCsIMedium);
  pcrystalLog13->SetVisLeaves(kTRUE);


  Double_t fullOrientedCrystalCentersX13[64] = {-65.705125,-106.743944930,-146.754762318,-185.352251120,-222.164696421,-256.837574247,-289.036965831,-318.452773431,-344.801706739,-367.830011123,-387.315911424,-403.071747769,-414.945782842,-422.823663194,-426.629520530,-426.326702364,-421.918125000,-413.446245449,-400.992652541,-384.677281184,-364.657257324,-341.125384734,-314.308288210,-284.464231053,-251.880627847,-216.871276502,-179.773336211,-140.944080419,-100.757456083,-59.600482367,-17.869523424,24.033528798,65.705125000,106.743944930,146.754762318,185.352251120,222.164696421,256.837574247,289.036965832,318.452773431,344.801706739,367.830011123,387.315911424,403.071747769,414.945782842,422.823663194,426.629520530,426.326702364,421.918125000,413.446245449,400.992652541,384.677281184,364.657257324,341.125384734,314.308288210,284.464231053,251.880627847,216.871276502,179.773336211,140.944080418,100.757456083,59.600482366,17.869523423,-24.033528799};
  Double_t fullOrientedCrystalCentersY13[64] = {421.918125,413.446245449,400.992652541,384.677281184,364.657257324,341.125384734,314.308288210,284.464231053,251.880627847,216.871276502,179.773336211,140.944080419,100.757456083,59.600482367,17.869523424,-24.033528798,-65.705125000,-106.743944930,-146.754762318,-185.352251120,-222.164696421,-256.837574247,-289.036965832,-318.452773431,-344.801706739,-367.830011123,-387.315911424,-403.071747769,-414.945782842,-422.823663194,-426.629520530,-426.326702364,-421.918125000,-413.446245449,-400.992652541,-384.677281184,-364.657257324,-341.125384734,-314.308288210,-284.464231053,-251.880627847,-216.871276502,-179.773336211,-140.944080418,-100.757456083,-59.600482366,-17.869523424,24.033528799,65.705125000,106.743944930,146.754762318,185.352251120,222.164696421,256.837574247,289.036965832,318.452773432,344.801706739,367.830011123,387.315911424,403.071747769,414.945782842,422.823663194,426.629520530,426.326702364};
  Double_t fullOrientedCrystalCentersZ13 = 664.6095;
  // -0.994757042612453


  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 =  -0.994757042612453*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX13[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY13[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ13/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog13, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX13[iter2]/10.;
      dy = fullOrientedCrystalCentersY13[iter2]/10.;
      dz = fullOrientedCrystalCentersZ13/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog13, iter2 , ph2);

    } // iter2
  } // iter1



  AddSensitiveVolume(pcrystalLog13);
  fNbOfSensitiveVol+=64;


  // Shape: testTrap14 type: TGeoTrap
  ddz     = 7.500000;
  theta  = 1.580967;
  phi    = 22.321544;
  h1     = 0.664907;
  bl1    = 1.758588;
  tl1    = 1.702662;
  alpha1 = 0.000000;
  h2     = 0.822043;
  bl2    = 2.147888;
  tl2    = 2.078812;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap14_15 = new TGeoTrap("testTrap14", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog14
  TGeoVolume*
  pcrystalLog14 = new TGeoVolume("crystalLog14",ptestTrap14_15, pCsIMedium);
  pcrystalLog14->SetVisLeaves(kTRUE);

  Double_t fullOrientedCrystalCentersX14[64] = {-65.019875,-104.698257618,-143.368338782,-180.657704469,-216.207237706,-249.674577052,-280.737413735,-309.096595657,-334.479008394,-356.640205435,-375.366762338,-390.478332124,-401.829382114,-409.310595492,-412.849924084,-412.413282220,-408.004875000,-399.667157796,-387.480427382,-371.562048633,-352.065324240,-329.178018316,-303.120548129,-274.143861361,-242.527019345,-208.574509555,-172.613313219,-134.989756316,-96.066174268,-56.217422446,-15.827266114,24.715315442,65.019875000,104.698257618,143.368338782,180.657704469,216.207237706,249.674577052,280.737413735,309.096595657,334.479008394,356.640205435,375.366762339,390.478332124,401.829382114,409.310595492,412.849924084,412.413282220,408.004875000,399.667157796,387.480427382,371.562048633,352.065324240,329.178018316,303.120548129,274.143861361,242.527019345,208.574509555,172.613313219,134.989756316,96.066174268,56.217422446,15.827266114,-24.715315442};
  Double_t fullOrientedCrystalCentersY14[64] = {408.004875,399.667157796,387.480427382,371.562048633,352.065324240,329.178018316,303.120548129,274.143861361,242.527019345,208.574509555,172.613313219,134.989756316,96.066174268,56.217422446,15.827266114,-24.715315442,-65.019875000,-104.698257618,-143.368338782,-180.657704469,-216.207237706,-249.674577052,-280.737413735,-309.096595657,-334.479008394,-356.640205435,-375.366762338,-390.478332124,-401.829382114,-409.310595492,-412.849924084,-412.413282220,-408.004875000,-399.667157796,-387.480427382,-371.562048633,-352.065324240,-329.178018316,-303.120548129,-274.143861361,-242.527019345,-208.574509555,-172.613313219,-134.989756316,-96.066174268,-56.217422446,-15.827266114,24.715315442,65.019875000,104.698257618,143.368338782,180.657704470,216.207237706,249.674577052,280.737413735,309.096595657,334.479008394,356.640205435,375.366762339,390.478332124,401.829382114,409.310595492,412.849924084,412.413282220};
  Double_t fullOrientedCrystalCentersZ14 = 673.420875;
  // -1.01570065145422


  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 = -1.01570065145422*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX14[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY14[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ14/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog14, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX14[iter2]/10.;
      dy = fullOrientedCrystalCentersY14[iter2]/10.;
      dz = fullOrientedCrystalCentersZ14/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog14, iter2 , ph2);

    } // iter2
  } // iter1



  AddSensitiveVolume(pcrystalLog14);
  fNbOfSensitiveVol+=64;


  // Shape: testTrap15 type: TGeoTrap
  ddz     = 8.000000;
  theta  = 1.588524;
  phi    = 28.195120;
  h1     = 0.846303;
  bl1    = 1.676614;
  tl1    = 1.604765;
  alpha1 = 0.000000;
  h2     = 1.055776;
  bl2    = 2.076240;
  tl2    = 1.986631;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap15_16 = new TGeoTrap("testTrap15", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog15
  TGeoVolume*
  pcrystalLog15 = new TGeoVolume("crystalLog15",ptestTrap15_16, pCsIMedium);
  pcrystalLog15->SetVisLeaves(kTRUE);

  Double_t fullOrientedCrystalCentersX15[64] = {-64.160625,-102.132427487,-139.120638867,-174.769042443,-208.734324601,-240.689381108,-270.326467301,-297.360161838,-321.530115464,-342.603558311,-360.377541605,-374.680892171,-385.375860924,-392.359449469,-395.564402030,-394.959853161,-390.551625000,-382.382171193,-370.530168046,-355.109756828,-336.269444529,-314.190673656,-289.086074841,-261.197419095,-230.793289418,-198.166494200,-163.631247314,-127.520142066,-90.180948140,-51.973262386,-13.265045704,25.570920620,64.160625000,102.132427487,139.120638867,174.769042443,208.734324601,240.689381109,270.326467301,297.360161839,321.530115464,342.603558311,360.377541605,374.680892171,385.375860924,392.359449469,395.564402030,394.959853161,390.551625000,382.382171193,370.530168046,355.109756828,336.269444529,314.190673656,289.086074841,261.197419095,230.793289418,198.166494200,163.631247313,127.520142066,90.180948140,51.973262386,13.265045704,-25.570920620};
  Double_t fullOrientedCrystalCentersY15[64] = {390.551625,382.382171193,370.530168046,355.109756828,336.269444529,314.190673656,289.086074841,261.197419095,230.793289418,198.166494200,163.631247314,127.520142066,90.180948140,51.973262386,13.265045704,-25.570920620,-64.160625000,-102.132427487,-139.120638867,-174.769042443,-208.734324601,-240.689381109,-270.326467301,-297.360161839,-321.530115464,-342.603558311,-360.377541605,-374.680892171,-385.375860924,-392.359449469,-395.564402030,-394.959853161,-390.551625000,-382.382171193,-370.530168046,-355.109756828,-336.269444529,-314.190673656,-289.086074841,-261.197419095,-230.793289418,-198.166494200,-163.631247313,-127.520142066,-90.180948140,-51.973262386,-13.265045704,25.570920620,64.160625000,102.132427487,139.120638867,174.769042443,208.734324602,240.689381109,270.326467301,297.360161839,321.530115464,342.603558311,360.377541605,374.680892171,385.375860924,392.359449469,395.564402030,394.959853161};
  Double_t fullOrientedCrystalCentersZ15 = 683.9395;
  // -1.0388569453971

  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 =  -1.0388569453971*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX15[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY15[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ15/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog15, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX15[iter2]/10.;
      dy = fullOrientedCrystalCentersY15[iter2]/10.;
      dz = fullOrientedCrystalCentersZ15/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog15, iter2 , ph2);

    } // iter2
  } // iter1



  AddSensitiveVolume(pcrystalLog15);
  fNbOfSensitiveVol+=64;


  // Shape: testTrap16 type: TGeoTrap
  ddz     = 8.000000;
  theta  = 1.529197;
  phi    = 29.344741;
  h1     = 0.846295;
  bl1    = 1.599024;
  tl1    = 1.525983;
  alpha1 = 0.000000;
  h2     = 1.055824;
  bl2    = 1.980730;
  tl2    = 1.889663;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap16_17 = new TGeoTrap("testTrap16", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog16
  TGeoVolume*
  pcrystalLog16 = new TGeoVolume("crystalLog16",ptestTrap16_17, pCsIMedium);
  pcrystalLog16->SetVisLeaves(kTRUE);


  Double_t fullOrientedCrystalCentersX16[64] = {-63.2885,-99.528728722,-134.810441378,-168.793855788,-201.151693096,-231.572329638,-259.762798055,-285.451608727,-308.391364362,-328.361142575,-345.168623485,-358.651941862,-368.681245979,-375.159948157,-378.025654950,-377.250768036,-372.842750000,-364.844052465,-353.331707260,-338.416584564,-320.242325161,-298.983957104,-274.846210100,-248.061543846,-218.887909320,-187.606264571,-154.517868939,-119.941381760,-84.209793509,-47.667218912,-10.665582940,26.438768426,63.288500000,99.528728722,134.810441378,168.793855788,201.151693096,231.572329638,259.762798055,285.451608727,308.391364362,328.361142575,345.168623485,358.651941862,368.681245979,375.159948157,378.025654950,377.250768036,372.842750000,364.844052465,353.331707260,338.416584564,320.242325161,298.983957104,274.846210100,248.061543846,218.887909320,187.606264571,154.517868939,119.941381760,84.209793509,47.667218912,10.665582940,-26.438768427};
  Double_t fullOrientedCrystalCentersY16[64] = {372.84275,364.844052465,353.331707260,338.416584564,320.242325161,298.983957104,274.846210100,248.061543846,218.887909320,187.606264571,154.517868939,119.941381760,84.209793509,47.667218912,10.665582940,-26.438768426,-63.288500000,-99.528728722,-134.810441378,-168.793855788,-201.151693096,-231.572329638,-259.762798055,-285.451608727,-308.391364362,-328.361142575,-345.168623485,-358.651941862,-368.681245979,-375.159948157,-378.025654950,-377.250768036,-372.842750000,-364.844052465,-353.331707260,-338.416584564,-320.242325161,-298.983957104,-274.846210100,-248.061543846,-218.887909320,-187.606264571,-154.517868939,-119.941381760,-84.209793509,-47.667218912,-10.665582940,26.438768426,63.288500000,99.528728722,134.810441378,168.793855789,201.151693096,231.572329638,259.762798055,285.451608727,308.391364362,328.361142575,345.168623485,358.651941862,368.681245979,375.159948157,378.025654950,377.250768036};
  Double_t fullOrientedCrystalCentersZ16 = 694.52075;
  // -1.06503560645036


  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 = -1.06503560645036*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX16[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY16[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ16/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog16, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX16[iter2]/10.;
      dy = fullOrientedCrystalCentersY16[iter2]/10.;
      dz = fullOrientedCrystalCentersZ16/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog16, iter2 , ph2);

    } // iter2
  } // iter1



  AddSensitiveVolume(pcrystalLog16);
  fNbOfSensitiveVol+=64;


  // Shape: testTrap17 type: TGeoTrap
  ddz     = 8.000000;
  theta  = 1.473302;
  phi    = 30.590679;
  h1     = 0.846324;
  bl1    = 1.519190;
  tl1    = 1.445060;
  alpha1 = 0.000000;
  h2     = 1.055838;
  bl2    = 1.882764;
  tl2    = 1.790286;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap17_18 = new TGeoTrap("testTrap17", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog17
  TGeoVolume*
  pcrystalLog17 = new TGeoVolume("crystalLog17",ptestTrap17_18, pCsIMedium);
  pcrystalLog17->SetVisLeaves(kTRUE);


  Double_t fullOrientedCrystalCentersX17[64] = {-62.39325,-96.855936001,-130.385846391,-162.660069805,-193.367787827,-222.213268347,-248.918713619,-273.226935606,-294.903832841,-313.740644956,-329.555963152,-342.197477269,-351.543442615,-357.503852436,-360.021304727,-359.071555045,-354.663750000,-346.840339163,-335.676666258,-321.280243558,-303.789716482,-283.373528369,-260.228298269,-234.576927402,-206.666452491,-176.765666667,-145.162530843,-112.161400494,-78.080094544,-43.246834600,-7.997083998,27.329682894,62.393250000,96.855936001,130.385846392,162.660069805,193.367787827,222.213268347,248.918713619,273.226935606,294.903832841,313.740644956,329.555963152,342.197477269,351.543442615,357.503852436,360.021304727,359.071555045,354.663750000,346.840339163,335.676666258,321.280243558,303.789716482,283.373528368,260.228298269,234.576927402,206.666452491,176.765666667,145.162530843,112.161400494,78.080094543,43.246834600,7.997083998,-27.329682894};
  Double_t fullOrientedCrystalCentersY17[64] = {354.66375,346.840339163,335.676666258,321.280243558,303.789716482,283.373528369,260.228298269,234.576927402,206.666452491,176.765666667,145.162530843,112.161400494,78.080094544,43.246834600,7.997083998,-27.329682894,-62.393250000,-96.855936001,-130.385846392,-162.660069805,-193.367787827,-222.213268347,-248.918713619,-273.226935606,-294.903832841,-313.740644956,-329.555963152,-342.197477269,-351.543442615,-357.503852436,-360.021304727,-359.071555045,-354.663750000,-346.840339163,-335.676666258,-321.280243558,-303.789716482,-283.373528369,-260.228298269,-234.576927402,-206.666452491,-176.765666667,-145.162530843,-112.161400494,-78.080094544,-43.246834600,-7.997083998,27.329682894,62.393250000,96.855936001,130.385846392,162.660069805,193.367787827,222.213268347,248.918713619,273.226935606,294.903832841,313.740644956,329.555963152,342.197477269,351.543442615,357.503852436,360.021304727,359.071555045};
  Double_t fullOrientedCrystalCentersZ17 = 704.25025;
// -1.09121528439846


  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 =  -1.09121528439846*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX17[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY17[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ17/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog17, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX17[iter2]/10.;
      dy = fullOrientedCrystalCentersY17[iter2]/10.;
      dz = fullOrientedCrystalCentersZ17/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog17, iter2 , ph2);

    } // iter2
  } // iter1




  AddSensitiveVolume(pcrystalLog17);
  fNbOfSensitiveVol+=64;



  // Shape: testTrap18 type: TGeoTrap
  ddz     = 8.000000;
  theta  = 1.423384;
  phi    = 31.953111;
  h1     = 0.846323;
  bl1    = 1.438267;
  tl1    = 1.363043;
  alpha1 = 0.000000;
  h2     = 1.055797;
  bl2    = 1.783387;
  tl2    = 1.689603;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap18_19 = new TGeoTrap("testTrap18", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog18
  TGeoVolume*
  pcrystalLog18 = new TGeoVolume("crystalLog18",ptestTrap18_19, pCsIMedium);
  pcrystalLog18->SetVisLeaves(kTRUE);

  Double_t fullOrientedCrystalCentersX18[64] = {-61.48575,-94.146595008,-125.900756841,-156.442425560,-185.477468202,-212.726261432,-237.926384477,-260.835146375,-281.231923227,-298.920282921,-313.729876884,-325.518080631,-334.171367315,-339.606401055,-341.770839505,-340.643837940,-336.236250000,-328.590523168,-317.780289971,-303.909658866,-287.112211612,-267.549716808,-245.410571975,-220.907989178,-194.277941684,-165.776891409,-135.679319047,-104.275080673,-71.866616269,-38.766037065,-5.292119731,28.232763608,61.485750000,94.146595008,125.900756841,156.442425560,185.477468202,212.726261432,237.926384477,260.835146375,281.231923227,298.920282921,313.729876884,325.518080631,334.171367315,339.606401055,341.770839505,340.643837940,336.236250000,328.590523167,317.780289971,303.909658866,287.112211612,267.549716808,245.410571975,220.907989178,194.277941684,165.776891409,135.679319047,104.275080673,71.866616269,38.766037065,5.292119731,-28.232763608};
  Double_t fullOrientedCrystalCentersY18[64] = {336.23625,328.590523168,317.780289971,303.909658866,287.112211612,267.549716808,245.410571975,220.907989178,194.277941684,165.776891410,135.679319047,104.275080673,71.866616269,38.766037065,5.292119731,-28.232763608,-61.485750000,-94.146595008,-125.900756841,-156.442425560,-185.477468202,-212.726261432,-237.926384477,-260.835146375,-281.231923227,-298.920282921,-313.729876884,-325.518080631,-334.171367315,-339.606401055,-341.770839505,-340.643837940,-336.236250000,-328.590523167,-317.780289971,-303.909658866,-287.112211612,-267.549716808,-245.410571975,-220.907989178,-194.277941684,-165.776891409,-135.679319047,-104.275080673,-71.866616269,-38.766037065,-5.292119731,28.232763608,61.485750000,94.146595008,125.900756841,156.442425560,185.477468202,212.726261432,237.926384477,260.835146375,281.231923227,298.920282921,313.729876884,325.518080631,334.171367315,339.606401055,341.770839505,340.643837940};
  Double_t fullOrientedCrystalCentersZ18 = 713.501;
// -1.11739600813377


  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 = -1.11739600813377*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX18[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY18[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ18/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog18, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX18[iter2]/10.;
      dy = fullOrientedCrystalCentersY18[iter2]/10.;
      dz = fullOrientedCrystalCentersZ18/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog18, iter2 , ph2);

    } // iter2
  } // iter1



  AddSensitiveVolume(pcrystalLog18);
  fNbOfSensitiveVol+=64;


  // Shape: testTrap19 type: TGeoTrap
  ddz     = 8.500000;
  theta  = 1.359243;
  phi    = 33.453011;
  h1     = 0.839742;
  bl1    = 1.345857;
  tl1    = 1.270556;
  alpha1 = 0.000000;
  h2     = 1.062342;
  bl2    = 1.692714;
  tl2    = 1.597523;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap19_20 = new TGeoTrap("testTrap19", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog19
  TGeoVolume*
  pcrystalLog19 = new TGeoVolume("crystalLog19",ptestTrap19_20, pCsIMedium);
  pcrystalLog19->SetVisLeaves(kTRUE);


  Double_t fullOrientedCrystalCentersX19[64] = {-60.566625,-91.402552948,-121.358224345,-150.145149700,-177.486095186,-203.117752552,-226.793274925,-248.284654083,-267.384916296,-283.910115598,-297.701105285,-308.625070588,-316.576807750,-321.479737195,-323.286641032,-321.980117789,-317.572750000,-310.106983025,-299.654716282,-286.316610814,-270.221119867,-251.523251817,-230.403077356,-207.063995309,-181.730773794,-154.647385583,-126.074658511,-96.287763557,-65.573564805,-34.227856777,-2.552515777,29.147407345,60.566625000,91.402552948,121.358224345,150.145149700,177.486095186,203.117752552,226.793274925,248.284654083,267.384916296,283.910115598,297.701105285,308.625070588,316.576807750,321.479737195,323.286641032,321.980117789,317.572750000,310.106983025,299.654716282,286.316610814,270.221119866,251.523251817,230.403077356,207.063995309,181.730773794,154.647385583,126.074658511,96.287763557,65.573564805,34.227856777,2.552515777,-29.147407345};
  Double_t fullOrientedCrystalCentersY19[64] = {317.57275,310.106983025,299.654716282,286.316610814,270.221119867,251.523251817,230.403077356,207.063995309,181.730773794,154.647385583,126.074658511,96.287763558,65.573564805,34.227856777,2.552515777,-29.147407345,-60.566625000,-91.402552948,-121.358224345,-150.145149700,-177.486095186,-203.117752552,-226.793274925,-248.284654083,-267.384916296,-283.910115598,-297.701105285,-308.625070588,-316.576807750,-321.479737195,-323.286641032,-321.980117789,-317.572750000,-310.106983025,-299.654716282,-286.316610814,-270.221119867,-251.523251817,-230.403077356,-207.063995309,-181.730773794,-154.647385583,-126.074658511,-96.287763557,-65.573564805,-34.227856777,-2.552515777,29.147407345,60.566625000,91.402552948,121.358224345,150.145149700,177.486095186,203.117752552,226.793274925,248.284654083,267.384916296,283.910115598,297.701105285,308.625070588,316.576807750,321.479737195,323.286641032,321.980117789};
  Double_t fullOrientedCrystalCentersZ19 = 722.266;
  // -1.14357792186212


  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 =  -1.14357792186212*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX19[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY19[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ19/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog19, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX19[iter2]/10.;
      dy = fullOrientedCrystalCentersY19[iter2]/10.;
      dz = fullOrientedCrystalCentersZ19/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog19, iter2 , ph2);

    } // iter2
  } // iter1



  AddSensitiveVolume(pcrystalLog19);
  fNbOfSensitiveVol+=64;



  // Shape: testTrap20 type: TGeoTrap
  ddz     = 8.500000;
  theta  = 1.302650;
  phi    = 35.096986;
  h1     = 0.839767;
  bl1    = 1.263384;
  tl1    = 1.187203;
  alpha1 = 0.000000;
  h2     = 1.062350;
  bl2    = 1.590237;
  tl2    = 1.493826;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap20_21 = new TGeoTrap("testTrap20", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog20
  TGeoVolume*
  pcrystalLog20 = new TGeoVolume("crystalLog20",ptestTrap20_21, pCsIMedium);
  pcrystalLog20->SetVisLeaves(kTRUE);


  Double_t fullOrientedCrystalCentersX20[64] = {-59.636625,-88.625854935,-116.761569440,-143.772806202,-169.399432247,-193.394649155,-215.527369873,-235.584444199,-253.372711543,-268.720861169,-281.481082003,-291.530486144,-298.772292337,-303.136758029,-304.581851030,-303.093654305,-298.686500000,-291.402831422,-281.312794280,-268.513561148,-253.128395638,-235.305465303,-215.216414707,-193.054712388,-169.033787654,-143.384975142,-116.355286937,-88.205033712,-59.205317796,-29.635422303,0.219878506,30.073061765,59.636625000,88.625854935,116.761569440,143.772806202,169.399432247,193.394649156,215.527369873,235.584444199,253.372711543,268.720861169,281.481082003,291.530486144,298.772292337,303.136758029,304.581851030,303.093654305,298.686500000,291.402831422,281.312794280,268.513561148,253.128395638,235.305465303,215.216414707,193.054712388,169.033787654,143.384975142,116.355286937,88.205033712,59.205317795,29.635422303,-0.219878506,-30.073061765};
  Double_t fullOrientedCrystalCentersY20[64] = {298.6865,291.402831422,281.312794280,268.513561148,253.128395638,235.305465303,215.216414707,193.054712388,169.033787654,143.384975142,116.355286937,88.205033712,59.205317796,29.635422303,-0.219878506,-30.073061765,-59.636625000,-88.625854935,-116.761569440,-143.772806202,-169.399432247,-193.394649155,-215.527369873,-235.584444199,-253.372711543,-268.720861169,-281.481082003,-291.530486144,-298.772292337,-303.136758029,-304.581851030,-303.093654305,-298.686500000,-291.402831422,-281.312794280,-268.513561148,-253.128395638,-235.305465303,-215.216414707,-193.054712388,-169.033787654,-143.384975142,-116.355286937,-88.205033712,-59.205317795,-29.635422303,0.219878506,30.073061765,59.636625000,88.625854935,116.761569440,143.772806202,169.399432247,193.394649156,215.527369873,235.584444199,253.372711544,268.720861169,281.481082003,291.530486144,298.772292337,303.136758029,304.581851030,303.093654305};
  Double_t fullOrientedCrystalCentersZ20 = 730.539;
  //-1.16975796791646


  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 =  -1.16975796791646*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX20[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY20[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ20/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog20, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX20[iter2]/10.;
      dy = fullOrientedCrystalCentersY20[iter2]/10.;
      dz = fullOrientedCrystalCentersZ20/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog20, iter2 , ph2);

    } // iter2
  } // iter1




  AddSensitiveVolume(pcrystalLog20);
  fNbOfSensitiveVol+=64;



  // Shape: testTrap21 type: TGeoTrap
  ddz     = 8.500000;
  theta  = 1.248668;
  phi    = 36.906376;
  h1     = 0.839800;
  bl1    = 1.180877;
  tl1    = 1.103756;
  alpha1 = 0.000000;
  h2     = 1.062370;
  bl2    = 1.487444;
  tl2    = 1.389923;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap21_22 = new TGeoTrap("testTrap21", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog21
  TGeoVolume*
  pcrystalLog21 = new TGeoVolume("crystalLog21",ptestTrap21_22, pCsIMedium);
  pcrystalLog21->SetVisLeaves(kTRUE);



  Double_t fullOrientedCrystalCentersX21[64] = {-58.705,-85.844452208,-112.157175414,-137.389763709,-161.299213475,-183.655263639,-204.242613219,-222.862994784,-239.337083879,-253.506226021,-265.233964626,-274.407355161,-280.938052858,-284.763163531,-285.845849271,-284.175683223,-279.768750000,-272.667490777,-262.940294563,-250.680839574,-236.007191064,-219.060664290,-200.004463567,-179.022110527,-156.315676699,-132.103837454,-106.619766039,-80.108887992,-52.826517562,-25.035398890,2.996824359,31.000186551,58.705000000,85.844452208,112.157175414,137.389763709,161.299213475,183.655263640,204.242613219,222.862994784,239.337083879,253.506226021,265.233964626,274.407355161,280.938052858,284.763163531,285.845849271,284.175683223,279.768750000,272.667490777,262.940294563,250.680839574,236.007191064,219.060664290,200.004463567,179.022110527,156.315676699,132.103837454,106.619766039,80.108887992,52.826517562,25.035398890,-2.996824359,-31.000186551};
  Double_t fullOrientedCrystalCentersY21[64] = {279.76875,272.667490777,262.940294563,250.680839574,236.007191064,219.060664290,200.004463567,179.022110527,156.315676700,132.103837454,106.619766039,80.108887992,52.826517562,25.035398890,-2.996824359,-31.000186551,-58.705000000,-85.844452208,-112.157175414,-137.389763709,-161.299213475,-183.655263639,-204.242613219,-222.862994784,-239.337083879,-253.506226021,-265.233964626,-274.407355161,-280.938052858,-284.763163531,-285.845849271,-284.175683223,-279.768750000,-272.667490777,-262.940294563,-250.680839574,-236.007191064,-219.060664290,-200.004463567,-179.022110527,-156.315676699,-132.103837454,-106.619766039,-80.108887992,-52.826517562,-25.035398890,2.996824359,31.000186551,58.705000000,85.844452208,112.157175414,137.389763709,161.299213475,183.655263640,204.242613219,222.862994784,239.337083879,253.506226021,265.233964626,274.407355161,280.938052858,284.763163531,285.845849271,284.175683223};
  Double_t fullOrientedCrystalCentersZ21 = 738.7705;
  //-1.19593883074528


  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 = -1.19593883074528*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX21[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY21[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ21/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog21, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX21[iter2]/10.;
      dy = fullOrientedCrystalCentersY21[iter2]/10.;
      dz = fullOrientedCrystalCentersZ21/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog21, iter2 , ph2);

    } // iter2
  } // iter1




  AddSensitiveVolume(pcrystalLog21);
  fNbOfSensitiveVol+=64;



  // Shape: testTrap22 type: TGeoTrap
  ddz     = 8.500000;
  theta  = 1.191940;
  phi    = 38.911607;
  h1     = 0.839754;
  bl1    = 1.098234;
  tl1    = 1.020276;
  alpha1 = 0.000000;
  h2     = 1.062341;
  bl2    = 1.384287;
  tl2    = 1.285703;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap22_23 = new TGeoTrap("testTrap22", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog22
  TGeoVolume*
  pcrystalLog22 = new TGeoVolume("crystalLog22",ptestTrap22_23, pCsIMedium);
  pcrystalLog22->SetVisLeaves(kTRUE);


  Double_t fullOrientedCrystalCentersX22[64] = {-57.77125,-83.056965019,-107.542796061,-130.992931188,-153.181532779,-173.894912473,-192.933589099,-210.114209793,-225.271315787,-238.258935865,-248.951992144,-257.247504648,-263.065583056,-266.350196093,-267.069711140,-265.217198874,-260.810500000,-253.892053437,-244.528487609,-232.809978772,-218.849382572,-202.781147183,-184.760018495,-164.959549829,-143.570430522,-120.798649485,-96.863511419,-71.995524787,-46.434181894,-20.425652446,5.779587200,31.929166263,57.771250000,83.056965019,107.542796061,130.992931188,153.181532779,173.894912473,192.933589099,210.114209793,225.271315787,238.258935865,248.951992144,257.247504648,263.065583056,266.350196093,267.069711140,265.217198874,260.810500000,253.892053437,244.528487609,232.809978772,218.849382572,202.781147183,184.760018495,164.959549829,143.570430522,120.798649485,96.863511419,71.995524787,46.434181894,20.425652446,-5.779587200,-31.929166263};
  Double_t fullOrientedCrystalCentersY22[64] = {260.8105,253.892053437,244.528487609,232.809978772,218.849382572,202.781147183,184.760018495,164.959549829,143.570430522,120.798649486,96.863511419,71.995524787,46.434181894,20.425652446,-5.779587200,-31.929166263,-57.771250000,-83.056965019,-107.542796061,-130.992931188,-153.181532779,-173.894912473,-192.933589099,-210.114209793,-225.271315787,-238.258935865,-248.951992144,-257.247504648,-263.065583056,-266.350196093,-267.069711140,-265.217198874,-260.810500000,-253.892053437,-244.528487609,-232.809978772,-218.849382572,-202.781147183,-184.760018495,-164.959549829,-143.570430522,-120.798649485,-96.863511419,-71.995524787,-46.434181894,-20.425652446,5.779587200,31.929166263,57.771250000,83.056965019,107.542796061,130.992931188,153.181532780,173.894912473,192.933589099,210.114209793,225.271315787,238.258935865,248.951992144,257.247504648,263.065583056,266.350196093,267.069711140,265.217198874};
  Double_t fullOrientedCrystalCentersZ22 = 746.96475;
// -1.22211474572358


  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 =  -1.22211474572358*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX22[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY22[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ22/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog22, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX22[iter2]/10.;
      dy = fullOrientedCrystalCentersY22[iter2]/10.;
      dz = fullOrientedCrystalCentersZ22/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog22, iter2 , ph2);

    } // iter2
  } // iter1



  AddSensitiveVolume(pcrystalLog22);
  fNbOfSensitiveVol+=64;


  // Shape: testTrap23 type: TGeoTrap
  ddz     = 9.000000;
  theta  = 0.944281;
  phi    = 52.663039;
  h1     = 0.838358;
  bl1    = 1.078562;
  tl1    = 1.000178;
  alpha1 = 0.000000;
  h2     = 1.074011;
  bl2    = 1.269302;
  tl2    = 1.168958;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap23_24 = new TGeoTrap("testTrap23", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog23
  TGeoVolume*
  pcrystalLog23 = new TGeoVolume("crystalLog23",ptestTrap23_24, pCsIMedium);
  pcrystalLog23->SetVisLeaves(kTRUE);


  Double_t fullOrientedCrystalCentersX23[64] = {-57.0925,-80.295923682,-102.726053725,-124.166875715,-144.411902815,-163.266164347,-180.548083466,-196.091225844,-209.745902523,-221.380611501,-230.883304172,-238.162464410,-243.147989923,-245.791867375,-246.068634781,-243.975626718,-239.533000000,-232.783539550,-223.792246361,-212.645711503,-199.451282199,-184.336028016,-167.445517115,-148.942414349,-129.004914713,-107.825027227,-85.606725785,-62.563984777,-38.918718395,-14.898643479,9.264913518,33.339244333,57.092500000,80.295923682,102.726053725,124.166875715,144.411902815,163.266164347,180.548083466,196.091225844,209.745902523,221.380611501,230.883304172,238.162464410,243.147989923,245.791867375,246.068634781,243.975626718,239.533000000,232.783539550,223.792246361,212.645711503,199.451282199,184.336028016,167.445517115,148.942414349,129.004914713,107.825027227,85.606725785,62.563984777,38.918718395,14.898643479,-9.264913518,-33.339244333};
  Double_t fullOrientedCrystalCentersY23[64] = {239.533,232.783539550,223.792246361,212.645711503,199.451282199,184.336028016,167.445517115,148.942414349,129.004914713,107.825027227,85.606725785,62.563984777,38.918718395,14.898643479,-9.264913518,-33.339244333,-57.092500000,-80.295923682,-102.726053725,-124.166875715,-144.411902815,-163.266164347,-180.548083466,-196.091225844,-209.745902523,-221.380611501,-230.883304172,-238.162464410,-243.147989923,-245.791867375,-246.068634781,-243.975626718,-239.533000000,-232.783539550,-223.792246361,-212.645711503,-199.451282199,-184.336028016,-167.445517115,-148.942414349,-129.004914713,-107.825027227,-85.606725785,-62.563984777,-38.918718395,-14.898643479,9.264913518,33.339244333,57.092500000,80.295923682,102.726053725,124.166875715,144.411902815,163.266164347,180.548083466,196.091225844,209.745902523,221.380611501,230.883304172,238.162464410,243.147989923,245.791867375,246.068634781,243.975626718};
  Double_t fullOrientedCrystalCentersZ23 = 754.358;
//   -1.2502432150317


  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 =  -1.2502432150317*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX23[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY23[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ23/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog23, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX23[iter2]/10.;
      dy = fullOrientedCrystalCentersY23[iter2]/10.;
      dz = fullOrientedCrystalCentersZ23/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog23, iter2 , ph2);

    } // iter2
  } // iter1




  AddSensitiveVolume(pcrystalLog23);
  fNbOfSensitiveVol+=64;


  // Shape: testTrap24 type: TGeoTrap
  ddz     = 9.000000;
  theta  = 0.939442;
  phi    = 52.675847;
  h1     = 0.838358;
  bl1    = 0.972407;
  tl1    = 0.893343;
  alpha1 = 0.000000;
  h2     = 1.074042;
  bl2    = 1.163207;
  tl2    = 1.061943;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap24_25 = new TGeoTrap("testTrap24", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog24
  TGeoVolume*
  pcrystalLog24 = new TGeoVolume("crystalLog24",ptestTrap24_25, pCsIMedium);
  pcrystalLog24->SetVisLeaves(kTRUE);


  Double_t fullOrientedCrystalCentersX24[64] = {-56.02725,-77.299254006,-97.826823940,-117.412268082,-135.866967899,-153.013194541,-168.685820475,-182.733909745,-195.022171572,-205.432263276,-213.863929984,-220.235970136,-224.487017503,-226.576132175,-226.483194834,-224.209100518,-219.775750000,-213.225838867,-204.622446345,-194.048427807,-181.605616831,-167.413844490,-151.609785308,-134.345641015,-115.787674759,-96.114609899,-75.515908804,-54.189948225,-32.342108823,-10.182797232,12.074580263,34.215672949,56.027250000,77.299254006,97.826823940,117.412268082,135.866967899,153.013194541,168.685820475,182.733909745,195.022171572,205.432263276,213.863929984,220.235970136,224.487017503,226.576132175,226.483194834,224.209100518,219.775750000,213.225838867,204.622446345,194.048427807,181.605616831,167.413844490,151.609785308,134.345641015,115.787674759,96.114609899,75.515908804,54.189948225,32.342108823,10.182797232,-12.074580263,-34.215672949};
  Double_t fullOrientedCrystalCentersY24[64] = {219.77575,213.225838867,204.622446345,194.048427807,181.605616831,167.413844490,151.609785308,134.345641015,115.787674759,96.114609899,75.515908804,54.189948226,32.342108823,10.182797232,-12.074580263,-34.215672949,-56.027250000,-77.299254006,-97.826823940,-117.412268082,-135.866967899,-153.013194541,-168.685820475,-182.733909745,-195.022171572,-205.432263276,-213.863929984,-220.235970136,-224.487017503,-226.576132175,-226.483194834,-224.209100518,-219.775750000,-213.225838867,-204.622446345,-194.048427807,-181.605616831,-167.413844490,-151.609785308,-134.345641015,-115.787674759,-96.114609899,-75.515908804,-54.189948225,-32.342108823,-10.182797232,12.074580263,34.215672949,56.027250000,77.299254006,97.826823940,117.412268082,135.866967899,153.013194542,168.685820475,182.733909745,195.022171572,205.432263276,213.863929984,220.235970136,224.487017503,226.576132175,226.483194834,224.209100518};
  Double_t fullOrientedCrystalCentersZ24 = 760.605;
  //-1.27641559764965


  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 =  -1.27641559764965*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX24[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY24[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ24/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog24, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX24[iter2]/10.;
      dy = fullOrientedCrystalCentersY24[iter2]/10.;
      dz = fullOrientedCrystalCentersZ24/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog24, iter2 , ph2);

    } // iter2
  } // iter1




  AddSensitiveVolume(pcrystalLog24);
  fNbOfSensitiveVol+=64;

  // Shape: testTrap25 type: TGeoTrap
  ddz     = 9.000000;
  theta  = 0.933910;
  phi    = 52.685539;
  h1     = 0.838343;
  bl1    = 0.865407;
  tl1    = 0.785708;
  alpha1 = 0.000000;
  h2     = 1.073991;
  bl2    = 1.056207;
  tl2    = 0.954128;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap25_26 = new TGeoTrap("testTrap25", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog25
  TGeoVolume*
  pcrystalLog25 = new TGeoVolume("crystalLog25",ptestTrap25_26, pCsIMedium);
  pcrystalLog25->SetVisLeaves(kTRUE);


  Double_t fullOrientedCrystalCentersX25[64] = {-54.953625,-74.278836463,-92.888702126,-110.603998809,-127.254118522,-142.678711508,-156.729230507,-169.270361340,-180.181326060,-189.357046113,-196.709154298,-202.166845796,-205.677560053,-207.207486972,-206.741892520,-204.285260627,-199.861250000,-193.512466280,-185.300051726,-175.303096378,-163.617876381,-150.356926792,-135.647957805,-119.632624832,-102.465164283,-84.310908189,-65.344691961,-45.749170627,-25.713059772,-5.429318095,14.906710884,35.099180088,54.953625000,74.278836463,92.888702126,110.603998809,127.254118522,142.678711509,156.729230507,169.270361340,180.181326060,189.357046113,196.709154298,202.166845796,205.677560053,207.207486972,206.741892520,204.285260627,199.861250000,193.512466280,185.300051726,175.303096378,163.617876381,150.356926792,135.647957805,119.632624832,102.465164283,84.310908189,65.344691961,45.749170627,25.713059772,5.429318095,-14.906710884,-35.099180088};
  Double_t fullOrientedCrystalCentersY25[64] = {199.86125,193.512466280,185.300051726,175.303096378,163.617876381,150.356926792,135.647957805,119.632624832,102.465164283,84.310908189,65.344691961,45.749170627,25.713059772,5.429318095,-14.906710884,-35.099180088,-54.953625000,-74.278836463,-92.888702126,-110.603998809,-127.254118522,-142.678711509,-156.729230507,-169.270361340,-180.181326060,-189.357046113,-196.709154298,-202.166845796,-205.677560053,-207.207486972,-206.741892520,-204.285260627,-199.861250000,-193.512466280,-185.300051726,-175.303096378,-163.617876381,-150.356926792,-135.647957805,-119.632624832,-102.465164283,-84.310908189,-65.344691961,-45.749170627,-25.713059772,-5.429318095,14.906710884,35.099180088,54.953625000,74.278836463,92.888702126,110.603998809,127.254118522,142.678711509,156.729230508,169.270361340,180.181326060,189.357046113,196.709154298,202.166845796,205.677560053,207.207486972,206.741892520,204.285260627};
  Double_t fullOrientedCrystalCentersZ25 = 766.33225;
  // -1.30259837031123


  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 =  -1.30259837031123*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX25[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY25[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ25/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog25, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX25[iter2]/10.;
      dy = fullOrientedCrystalCentersY25[iter2]/10.;
      dz = fullOrientedCrystalCentersZ25/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog25, iter2 , ph2);

    } // iter2
  } // iter1




  AddSensitiveVolume(pcrystalLog25);
  fNbOfSensitiveVol+=64;


  // Shape: testTrap26 type: TGeoTrap
  ddz     = 9.000000;
  theta  = 0.939258;
  phi    = 52.700532;
  h1     = 0.838348;
  bl1    = 0.757607;
  tl1    = 0.677323;
  alpha1 = 0.000000;
  h2     = 1.074006;
  bl2    = 0.948407;
  tl2    = 0.845563;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap26_27 = new TGeoTrap("testTrap26", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog26
  TGeoVolume*
  pcrystalLog26 = new TGeoVolume("crystalLog26",ptestTrap26_27, pCsIMedium);
  pcrystalLog26->SetVisLeaves(kTRUE);


  Double_t fullOrientedCrystalCentersX26[64] = {-53.87225,-71.236689787,-87.915081309,-103.746802540,-118.579385347,-132.269983852,-144.686750107,-155.710103864,-165.233884201,-173.166371907,-179.431172789,-183.967953390,-186.733022033,-187.699749595,-186.858825962,-184.218349687,-179.803750000,-173.657541910,-165.838916761,-156.423172187,-145.500986955,-133.177547679,-119.571535817,-104.813984701,-89.047017615,-72.422479071,-55.100472464,-37.247818186,-19.036447057,-0.641744536,17.759138335,35.988990996,53.872250000,71.236689787,87.915081309,103.746802540,118.579385347,132.269983852,144.686750107,155.710103864,165.233884201,173.166371907,179.431172789,183.967953390,186.733022033,187.699749595,186.858825962,184.218349686,179.803750000,173.657541910,165.838916761,156.423172187,145.500986955,133.177547679,119.571535817,104.813984701,89.047017615,72.422479071,55.100472464,37.247818186,19.036447057,0.641744536,-17.759138335,-35.988990996};
  Double_t fullOrientedCrystalCentersY26[64] = {179.80375,173.657541910,165.838916761,156.423172187,145.500986955,133.177547679,119.571535817,104.813984701,89.047017615,72.422479071,55.100472464,37.247818186,19.036447057,0.641744536,-17.759138335,-35.988990996,-53.872250000,-71.236689787,-87.915081309,-103.746802540,-118.579385347,-132.269983852,-144.686750107,-155.710103864,-165.233884201,-173.166371907,-179.431172789,-183.967953390,-186.733022033,-187.699749595,-186.858825962,-184.218349686,-179.803750000,-173.657541910,-165.838916761,-156.423172187,-145.500986955,-133.177547679,-119.571535817,-104.813984701,-89.047017615,-72.422479071,-55.100472464,-37.247818186,-19.036447057,-0.641744536,17.759138335,35.988990996,53.872250000,71.236689787,87.915081309,103.746802540,118.579385347,132.269983852,144.686750107,155.710103864,165.233884201,173.166371907,179.431172789,183.967953390,186.733022033,187.699749595,186.858825962,184.218349686};
  Double_t fullOrientedCrystalCentersZ26 = 771.5365;
  //  -1.32877984663783


  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 =  -1.32877984663783*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX26[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY26[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ26/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog26, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX26[iter2]/10.;
      dy = fullOrientedCrystalCentersY26[iter2]/10.;
      dz = fullOrientedCrystalCentersZ26/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog26, iter2 , ph2);

    } // iter2
  } // iter1



  AddSensitiveVolume(pcrystalLog26);
  fNbOfSensitiveVol+=64;


  // Shape: testTrap27 type: TGeoTrap
  ddz     = 9.000000;
  theta  = 0.794535;
  phi    = 69.643850;
  h1     = 0.838315;
  bl1    = 0.733316;
  tl1    = 0.652709;
  alpha1 = 0.000000;
  h2     = 1.074013;
  bl2    = 0.832136;
  tl2    = 0.728789;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap27_28 = new TGeoTrap("testTrap27", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog27
  TGeoVolume*
  pcrystalLog27 = new TGeoVolume("crystalLog27",ptestTrap27_28, pCsIMedium);
  pcrystalLog27->SetVisLeaves(kTRUE);


  Double_t fullOrientedCrystalCentersX27[64] = {-53.167375,-68.556585950,-83.285559500,-97.212447583,-110.203126655,-122.132489373,-132.885649455,-142.359048090,-150.461451270,-157.114828424,-162.255103892,-165.832774012,-167.813383866,-168.177859096,-166.922689608,-164.059963370,-159.617250000,-153.637335257,-146.177808989,-137.310510511,-127.120836756,-115.706919851,-103.178682056,-89.656777150,-75.271428469,-60.161174784,-44.471536098,-28.353612209,-11.962627535,4.543563782,21.005998096,37.266133165,53.167375000,68.556585950,83.285559500,97.212447583,110.203126655,122.132489373,132.885649455,142.359048090,150.461451270,157.114828424,162.255103892,165.832774012,167.813383866,168.177859096,166.922689608,164.059963370,159.617250000,153.637335257,146.177808989,137.310510511,127.120836756,115.706919851,103.178682056,89.656777150,75.271428469,60.161174784,44.471536098,28.353612209,11.962627535,-4.543563782,-21.005998096,-37.266133165};
  Double_t fullOrientedCrystalCentersY27[64] = {159.61725,153.637335257,146.177808989,137.310510511,127.120836756,115.706919851,103.178682057,89.656777150,75.271428469,60.161174784,44.471536098,28.353612209,11.962627535,-4.543563782,-21.005998096,-37.266133165,-53.167375000,-68.556585950,-83.285559500,-97.212447583,-110.203126655,-122.132489373,-132.885649455,-142.359048090,-150.461451270,-157.114828424,-162.255103892,-165.832774012,-167.813383866,-168.177859096,-166.922689608,-164.059963370,-159.617250000,-153.637335257,-146.177808989,-137.310510511,-127.120836756,-115.706919851,-103.178682056,-89.656777150,-75.271428469,-60.161174784,-44.471536098,-28.353612209,-11.962627535,4.543563782,21.005998096,37.266133165,53.167375000,68.556585950,83.285559500,97.212447583,110.203126655,122.132489373,132.885649455,142.359048090,150.461451270,157.114828424,162.255103892,165.832774012,167.813383866,168.177859096,166.922689608,164.059963370};
  Double_t fullOrientedCrystalCentersZ27 = 776.2135;
  // -1.35495689146952


  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 =  -1.35495689146952*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX27[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY27[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ27/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog27, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX27[iter2]/10.;
      dy = fullOrientedCrystalCentersY27[iter2]/10.;
      dz = fullOrientedCrystalCentersZ27/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog27, iter2 , ph2);

    } // iter2
  } // iter1



  AddSensitiveVolume(pcrystalLog27);
  fNbOfSensitiveVol+=64;


  // Shape: testTrap28 type: TGeoTrap
  ddz     = 9.000000;
  theta  = 0.810614;
  phi    = 69.652142;
  h1     = 0.838385;
  bl1    = 0.624171;
  tl1    = 0.543014;
  alpha1 = 0.000000;
  h2     = 1.074026;
  bl2    = 0.722931;
  tl2    = 0.619034;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap28_29 = new TGeoTrap("testTrap28", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog28
  TGeoVolume*
  pcrystalLog28 = new TGeoVolume("crystalLog28",ptestTrap28_29, pCsIMedium);
  pcrystalLog28->SetVisLeaves(kTRUE);


  Double_t fullOrientedCrystalCentersX28[64] = {-52.072875,-65.477363275,-78.251268747,-90.271571726,-101.422510121,-111.596694300,-120.696141308,-128.633218497,-135.331487473,-140.726440245,-144.766120468,-147.411623815,-148.637472640,-148.431861351,-146.796770095,-143.747945696,-139.314750000,-133.539877104,-126.478942191,-118.199945924,-108.782619564,-98.317657110,-86.905841873,-74.657075871,-61.689321416,-48.127465073,-34.102114933,-19.748342783,-5.204383298,9.389697245,23.893349869,38.166896473,52.072875000,65.477363275,78.251268747,90.271571726,101.422510121,111.596694300,120.696141308,128.633218497,135.331487473,140.726440245,144.766120468,147.411623815,148.637472640,148.431861351,146.796770095,143.747945696,139.314750000,133.539877104,126.478942191,118.199945924,108.782619564,98.317657110,86.905841873,74.657075870,61.689321416,48.127465073,34.102114933,19.748342783,5.204383298,-9.389697245,-23.893349869,-38.166896473};
  Double_t fullOrientedCrystalCentersY28[64] = {139.31475,133.539877104,126.478942191,118.199945924,108.782619564,98.317657110,86.905841873,74.657075871,61.689321416,48.127465073,34.102114933,19.748342783,5.204383298,-9.389697245,-23.893349869,-38.166896473,-52.072875000,-65.477363275,-78.251268747,-90.271571726,-101.422510121,-111.596694300,-120.696141308,-128.633218497,-135.331487473,-140.726440245,-144.766120468,-147.411623815,-148.637472640,-148.431861351,-146.796770095,-143.747945696,-139.314750000,-133.539877104,-126.478942191,-118.199945924,-108.782619564,-98.317657110,-86.905841873,-74.657075870,-61.689321416,-48.127465073,-34.102114933,-19.748342783,-5.204383298,9.389697245,23.893349869,38.166896473,52.072875000,65.477363275,78.251268747,90.271571726,101.422510121,111.596694300,120.696141308,128.633218497,135.331487473,140.726440245,144.766120468,147.411623815,148.637472640,148.431861351,146.796770095,143.747945696};
  Double_t fullOrientedCrystalCentersZ28 = 780.361;
// -1.38114217156403


  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 =  -1.38114217156403*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX28[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY28[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ28/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog28, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX28[iter2]/10.;
      dy = fullOrientedCrystalCentersY28[iter2]/10.;
      dz = fullOrientedCrystalCentersZ28/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog28, iter2 , ph2);

    } // iter2
  } // iter1



  AddSensitiveVolume(pcrystalLog28);
  fNbOfSensitiveVol+=64;


  // Shape: testTrap29 type: TGeoTrap
  ddz     = 9.000000;
  theta  = 0.806218;
  phi    = 69.668645;
  h1     = 0.838308;
  bl1    = 0.514366;
  tl1    = 0.432824;
  alpha1 = 0.000000;
  h2     = 1.074076;
  bl2    = 0.613186;
  tl2    = 0.508724;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap29_30 = new TGeoTrap("testTrap29", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog29
  TGeoVolume*
  pcrystalLog29 = new TGeoVolume("crystalLog29",ptestTrap29_30, pCsIMedium);
  pcrystalLog29->SetVisLeaves(kTRUE);


  Double_t fullOrientedCrystalCentersX29[64] = {-50.97275,-62.382593946,-73.191659410,-83.295849184,-92.597854396,-101.008091651,-108.445565767,-114.838649802,-120.125774862,-124.256023043,-127.189617796,-128.898307002,-129.365635048,-128.587101310,-126.570203493,-123.334365427,-118.910750000,-113.341959047,-106.681623070,-98.993884745,-90.352781193,-80.841530968,-70.551732606,-59.582482492,-48.039420500,-36.033712628,-23.680980406,-11.100187396,1.587506486,14.259911812,26.794986393,39.072010607,50.972750000,62.382593946,73.191659410,83.295849184,92.597854396,101.008091651,108.445565767,114.838649802,120.125774862,124.256023043,127.189617796,128.898307002,129.365635048,128.587101310,126.570203493,123.334365427,118.910750000,113.341959047,106.681623070,98.993884745,90.352781193,80.841530968,70.551732606,59.582482492,48.039420500,36.033712628,23.680980405,11.100187396,-1.587506486,-14.259911812,-26.794986393,-39.072010607};
  Double_t fullOrientedCrystalCentersY29[64] = {118.91075,113.341959047,106.681623070,98.993884745,90.352781193,80.841530968,70.551732606,59.582482492,48.039420500,36.033712628,23.680980406,11.100187396,-1.587506486,-14.259911812,-26.794986393,-39.072010607,-50.972750000,-62.382593946,-73.191659410,-83.295849184,-92.597854396,-101.008091651,-108.445565767,-114.838649802,-120.125774862,-124.256023043,-127.189617796,-128.898307002,-129.365635048,-128.587101310,-126.570203493,-123.334365427,-118.910750000,-113.341959047,-106.681623070,-98.993884745,-90.352781193,-80.841530968,-70.551732606,-59.582482492,-48.039420500,-36.033712628,-23.680980406,-11.100187396,1.587506486,14.259911812,26.794986393,39.072010607,50.972750000,62.382593946,73.191659410,83.295849184,92.597854396,101.008091651,108.445565767,114.838649802,120.125774862,124.256023043,127.189617796,128.898307002,129.365635048,128.587101310,126.570203493,123.334365427};
  Double_t fullOrientedCrystalCentersZ29 = 783.9755;
  // -1.40731575207972

  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 =  -1.40731575207972*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX29[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY29[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ29/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog29, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX29[iter2]/10.;
      dy = fullOrientedCrystalCentersY29[iter2]/10.;
      dz = fullOrientedCrystalCentersZ29/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog29, iter2 , ph2);

    } // iter2
  } // iter1



  AddSensitiveVolume(pcrystalLog29);
  fNbOfSensitiveVol+=64;


  // Shape: testTrap30 type: TGeoTrap
  ddz     = 9.000000;
  theta  = 0.805683;
  phi    = 69.669215;
  h1     = 0.838395;
  bl1    = 0.404116;
  tl1    = 0.322234;
  alpha1 = 0.000000;
  h2     = 1.074008;
  bl2    = 0.502936;
  tl2    = 0.398014;
  alpha2 = 0.000000;
  TGeoShape *ptestTrap30_31 = new TGeoTrap("testTrap30", ddz,theta,phi,h1,bl1,tl1,alpha1,h2,bl2,tl2,alpha2);
  // Volume: crystalLog30
  TGeoVolume*
  pcrystalLog30 = new TGeoVolume("crystalLog30",ptestTrap30_31, pCsIMedium);
  pcrystalLog30->SetVisLeaves(kTRUE);


  Double_t fullOrientedCrystalCentersX30[64] = {-49.86825,-59.274869680,-68.110639962,-76.290467548,-83.735576227,-90.374265533,-96.142601257,-100.985031175,-104.854920039,-107.714998703,-109.537723046,-110.305539237,-110.011052786,-108.657099759,-106.256719463,-102.833028873,-98.419000000,-93.057142356,-86.799093561,-79.705122046,-71.843546634,-63.290078595,-54.127092500,-44.442832916,-34.330564557,-23.887674093,-13.214732270,-2.414525351,8.408934767,19.151412247,29.709451157,39.981371812,49.868250000,59.274869680,68.110639962,76.290467548,83.735576227,90.374265533,96.142601257,100.985031175,104.854920039,107.714998703,109.537723046,110.305539237,110.011052786,108.657099759,106.256719463,102.833028873,98.419000000,93.057142356,86.799093561,79.705122046,71.843546634,63.290078595,54.127092500,44.442832916,34.330564557,23.887674093,13.214732270,2.414525351,-8.408934767,-19.151412247,-29.709451157,-39.981371812};
  Double_t fullOrientedCrystalCentersY30[64] = {98.419,93.057142356,86.799093561,79.705122046,71.843546634,63.290078595,54.127092500,44.442832917,34.330564557,23.887674093,13.214732270,2.414525351,-8.408934767,-19.151412247,-29.709451157,-39.981371812,-49.868250000,-59.274869680,-68.110639962,-76.290467548,-83.735576227,-90.374265533,-96.142601257,-100.985031175,-104.854920039,-107.714998703,-109.537723046,-110.305539237,-110.011052786,-108.657099759,-106.256719463,-102.833028873,-98.419000000,-93.057142356,-86.799093561,-79.705122046,-71.843546634,-63.290078595,-54.127092500,-44.442832916,-34.330564557,-23.887674093,-13.214732270,-2.414525351,8.408934767,19.151412247,29.709451157,39.981371812,49.868250000,59.274869680,68.110639962,76.290467548,83.735576227,90.374265533,96.142601257,100.985031175,104.854920039,107.714998703,109.537723046,110.305539237,110.011052786,108.657099759,106.256719463,102.833028873};
  Double_t fullOrientedCrystalCentersZ30 = 787.05475;

// -1.43350584007434

  for ( Int_t iter1=0; iter1<1; iter1++) {

    angle1 = -1.43350584007434*fac;
    angle2 = (3.*3.14159265359/2.)*fac;

    r1 = createMatrix(0.,angle1,0.);
    r2 = createMatrix(0.,angle2,180.);
    r3 = createMatrix(0.,0.,0.);

    dx = fullOrientedCrystalCentersX30[iter1*64]/10.;
    dy = fullOrientedCrystalCentersY30[iter1*64]/10.;
    dz = fullOrientedCrystalCentersZ30/10.;

    TGeoTranslation *t1 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t2 = new TGeoTranslation(0.,0.,0.);
    TGeoTranslation *t3 = new TGeoTranslation(dx,dy,dz);
    TGeoCombiTrans *c1 = new TGeoCombiTrans(*t1,*r1);
    TGeoCombiTrans *c2 = new TGeoCombiTrans(*t2,*r2);
    TGeoCombiTrans *c3 = new TGeoCombiTrans(*t3,*r3);
    TGeoHMatrix hm = (*c3) * (*c1) * (*c2) ;
    TGeoHMatrix *ph = new TGeoHMatrix(hm);

    pWorld->AddNode(pcrystalLog30, 0, ph);

    TGeoRotation  *pIndividualCrystalRot=NULL;

    for (Int_t iter2=1; iter2<64; iter2++) {

      Double_t a = 1.570796327*fac;
      Double_t b = (iter2*5.625);
      Double_t c = -1.570796327*fac;
      pIndividualCrystalRot = createMatrix(a,b,c);

      dx = fullOrientedCrystalCentersX30[iter2]/10.;
      dy = fullOrientedCrystalCentersY30[iter2]/10.;
      dz = fullOrientedCrystalCentersZ30/10.;

      TGeoTranslation *t0 = new TGeoTranslation(0.,0.,0.);
      TGeoTranslation *t = new TGeoTranslation(dx,dy,dz);
      TGeoCombiTrans *tt = new TGeoCombiTrans(*t,*r3);
      TGeoCombiTrans *c4 = new TGeoCombiTrans(*t0,*pIndividualCrystalRot);
      TGeoHMatrix htmp =  (*c1) * (*c4) * (*c2);
      TGeoHMatrix inv = htmp.Inverse();
      TGeoHMatrix hm2  =  (*tt) * inv;
      TGeoHMatrix *ph2 = new TGeoHMatrix(hm2);

      pWorld->AddNode(pcrystalLog30, iter2 , ph2);

    } // iter2
  } // iter1



  AddSensitiveVolume(pcrystalLog30);
  fNbOfSensitiveVol+=64;


  // H - Structure
  // Combi transformation:
  dx = 0.000000;
  dy = 0.000000;
  ddz = 5.000000;
  // Rotation:
  thx = 90.000000;    phx = 0.000000;
  thy = 90.000000;    phy = 90.000000;
  thz = 0.000000;    phz = 0.000000;
  TGeoRotation *pMatrix9219 = new TGeoRotation("",thx,phx,thy,phy,thz,phz);
  TGeoCombiTrans*
  pMatrix9218 = new TGeoCombiTrans("", dx,dy,ddz,pMatrix9219);
  // Combi transformation:
  dx = 0.000000;
  dy = 0.000000;
  ddz = 57.510000;
  // Rotation:
  thx = 90.000000;    phx = 0.000000;
  thy = 90.000000;    phy = 90.000000;
  thz = 0.000000;    phz = 0.000000;
  TGeoRotation *pMatrix9221 = new TGeoRotation("",thx,phx,thy,phy,thz,phz);
  TGeoCombiTrans*
  pMatrix9220 = new TGeoCombiTrans("", dx,dy,ddz,pMatrix9221);

  // Shape: carbonFiberProtoZero type: TGeoTubeSeg
  rmin = 42.500000;
  rmax = 42.520000;
  ddz   = 45.000000;
  phi1 = 0.000000;
  phi2 = 360.000000;
  TGeoShape *pcarbonFiberProtoZero_32 = new TGeoTubeSeg("carbonFiberProtoZero",rmin,rmax,ddz,phi1,phi2);
  // Volume: carbonFiberTestLog1
  TGeoVolume*
  pcarbonFiberTestLog1 = new TGeoVolume("carbonFiberTestLog1",pcarbonFiberProtoZero_32, pCarbonFibreMedium);
  pcarbonFiberTestLog1->SetVisLeaves(kTRUE);
  pWorld->AddNode(pcarbonFiberTestLog1, 0, pMatrix9218);
  // Shape: carbonFiberTest2 type: TGeoConeSeg
  ddz    = 7.500000;
  rmin1 = 42.500000;
  rmax1 = 42.520000;
  rmin2 = 5.000000;
  rmax2 = 5.020000;
  phi1  = 0.000000;
  phi2  = 360.000000;
  TGeoShape *pcarbonFiberTest2_33 = new TGeoConeSeg("carbonFiberTest2", ddz,rmin1,rmax1,rmin2,rmax2,phi1,phi2);
  // Volume: carbonFiberTestLog2
  TGeoVolume*
  pcarbonFiberTestLog2 = new TGeoVolume("carbonFiberTestLog2",pcarbonFiberTest2_33, pCarbonFibreMedium);
  pcarbonFiberTestLog2->SetVisLeaves(kTRUE);
  pWorld->AddNode(pcarbonFiberTestLog2, 0, pMatrix9220);


  // END OF CALIFA ROOT
  // Geometry Description
  // CHECK ME ! <D.Bertini@gsi.de>

}


TGeoRotation* R3BCalo::createMatrix( Double_t phi, Double_t theta, Double_t psi)
{

  // Rotation
  TGeoRotation * matrix = new TGeoRotation("");

  Double_t rm[9];

  Double_t degrad = TMath::Pi()/180.;

  // define trigonometry
  Double_t  sinPhi   = sin( degrad*phi )  ;
  Double_t  cosPhi   = cos( degrad*phi )  ;
  Double_t  sinTheta = sin( degrad*theta );
  Double_t  cosTheta = cos( degrad*theta );
  Double_t  sinPsi   = sin( degrad*psi )  ;
  Double_t  cosPsi   = cos( degrad*psi )  ;


  // filling from Euler definition a la G4 !

  rm[0] =   cosPsi * cosPhi - cosTheta * sinPhi * sinPsi;
  rm[1] =   cosPsi * sinPhi + cosTheta * cosPhi * sinPsi;
  rm[2] =   sinPsi * sinTheta;

  rm[3] = - sinPsi * cosPhi - cosTheta * sinPhi * cosPsi;
  rm[4] = - sinPsi * sinPhi + cosTheta * cosPhi * cosPsi;
  rm[5] =   cosPsi * sinTheta;

  rm[6] =   sinTheta * sinPhi;
  rm[7] = - sinTheta * cosPhi;
  rm[8] =   cosTheta;


  matrix->SetMatrix( (const Double_t*) &rm[0] );

  return matrix;

}

// -----   Public method ConstructGeometry   ----------------------------------
void R3BCalo::ConstructUserDefinedGeometry()
{

  /****************************************************************************/
  // Material definition

  Double_t aMat;
  Double_t z, density, w;
  Int_t nel, numed;


  // Mixture: CsI
  TGeoMedium * pCsIMedium=NULL;
  if (gGeoManager->GetMedium("CsI") ) {
    pCsIMedium=gGeoManager->GetMedium("CsI");
    //if (gGeoManager->GetMedium("aluminium") ){
    //  pCsIMedium=gGeoManager->GetMedium("aluminium");
  } else {
    nel     = 2;
    density = 4.510000;
    TGeoMixture*
    pCsIMaterial = new TGeoMixture("CsIn", nel,density);
    aMat = 132.905450;   z = 55.000000;   w = 0.511549;  // CS
    pCsIMaterial->DefineElement(0,aMat,z,w);
    aMat = 126.904470;   z = 53.000000;   w = 0.488451;  // I
    pCsIMaterial->DefineElement(1,aMat,z,w);
    numed = 801;
    pCsIMaterial->SetIndex(numed);
    Double_t par[8];
    par[0]  = 0.000000; // isvol
    par[1]  = 0.000000; // ifield
    par[2]  = 0.000000; // fieldm
    par[3]  = 0.000000; // tmaxfd
    par[4]  = 0.000000; // stemax
    par[5]  = 0.000000; // deemax
    par[6]  = 0.000100; // epsil
    par[7]  = 0.000000; // stmin
    pCsIMedium = new TGeoMedium("CsIn", numed,pCsIMaterial, par);
  }

  // Mixture: CarbonFibre
  TGeoMedium * pCarbonFibreMedium=NULL;
  if (gGeoManager->GetMedium("CarbonFibre") ) {
    pCarbonFibreMedium=gGeoManager->GetMedium("CarbonFibre");
  } else {
    nel     = 3;
    density = 1.690000;
    TGeoMixture*
    pCarbonFibreMaterial = new TGeoMixture("CarbonFibre", nel,density);
    aMat = 12.010700;   z = 6.000000;   w = 0.844907;  // C
    pCarbonFibreMaterial->DefineElement(0,aMat,z,w);
    aMat = 1.007940;   z = 1.000000;   w = 0.042543;  // H
    pCarbonFibreMaterial->DefineElement(1,aMat,z,w);
    aMat = 15.999400;   z = 8.000000;   w = 0.112550;  // O
    pCarbonFibreMaterial->DefineElement(2,aMat,z,w);
    // Medium: CarbonFibre
    numed   = 802;  // medium number
    pCarbonFibreMaterial->SetIndex(numed);
    Double_t par[8];
    par[0]  = 0.000000; // isvol
    par[1]  = 0.000000; // ifield
    par[2]  = 0.000000; // fieldm
    par[3]  = 0.000000; // tmaxfd
    par[4]  = 0.000000; // stemax
    par[5]  = 0.000000; // deemax
    par[6]  = 0.000100; // epsil
    par[7]  = 0.000000; // stmin
    pCarbonFibreMedium = new TGeoMedium("CarbonFibre", numed,pCarbonFibreMaterial,par);
  }

  // Mixture: Wrapping component
  TGeoMedium * pWrappingMedium=NULL;
  if (gGeoManager->GetMedium("mylar") ) {
    pWrappingMedium=gGeoManager->GetMedium("mylar");
  } else { // CARBON FIBER DEFINITION HERE!!! CHANGE IT TO WHATEVER IS USED!!
    nel     = 3;
    density = 1.690000;
    TGeoMixture*
    pWrappingMaterial = new TGeoMixture("Wrapping", nel,density);
    aMat = 12.010700;   z = 6.000000;   w = 0.844907;  // C
    pWrappingMaterial->DefineElement(0,aMat,z,w);
    aMat = 1.007940;   z = 1.000000;   w = 0.042543;  // H
    pWrappingMaterial->DefineElement(1,aMat,z,w);
    aMat = 15.999400;   z = 8.000000;   w = 0.112550;  // O
    pWrappingMaterial->DefineElement(2,aMat,z,w);
    // Medium: CarbonFibre
    numed   = 803;  // medium number
    pWrappingMaterial->SetIndex(numed);
    Double_t par[8];
    par[0]  = 0.000000; // isvol
    par[1]  = 0.000000; // ifield
    par[2]  = 0.000000; // fieldm
    par[3]  = 0.000000; // tmaxfd
    par[4]  = 0.000000; // stemax
    par[5]  = 0.000000; // deemax
    par[6]  = 0.000100; // epsil
    par[7]  = 0.000000; // stmin
    pWrappingMedium = new TGeoMedium("Wrapping", numed,pWrappingMaterial,par);
  }


  // Mixture: Air
  TGeoMedium * pAirMedium=NULL;
  if (gGeoManager->GetMedium("Air") ) {
    pAirMedium=gGeoManager->GetMedium("Air");
  } else {
    nel     = 2;
    density = 0.001290;
    TGeoMixture*
    pAirMaterial = new TGeoMixture("Air", nel,density);
    aMat = 14.006740;   z = 7.000000;   w = 0.700000;  // N
    pAirMaterial->DefineElement(0,aMat,z,w);
    aMat = 15.999400;   z = 8.000000;   w = 0.300000;  // O
    pAirMaterial->DefineElement(1,aMat,z,w);
    pAirMaterial->SetIndex(1);
    // Medium: Air
    numed   = 1;  // medium number
    Double_t par[8];
    par[0]  = 0.000000; // isvol
    par[1]  = 0.000000; // ifield
    par[2]  = 0.000000; // fieldm
    par[3]  = 0.000000; // tmaxfd
    par[4]  = 0.000000; // stemax
    par[5]  = 0.000000; // deemax
    par[6]  = 0.000100; // epsil
    par[7]  = 0.000000; // stmin
    pAirMedium = new TGeoMedium("Air", numed,pAirMaterial, par);
  }


  //WORLD

  TGeoVolume *pAWorld  =  gGeoManager->GetTopVolume();

  // Defintion of the Mother Volume

  Double_t length = 300.;

  TGeoShape *pCBWorld = new TGeoBBox("Califa_box",
                                     length/2.0,
                                     length/2.0,
                                     length/2.0);

  TGeoVolume*
  pWorld  = new TGeoVolume("CalifaWorld",pCBWorld, pAirMedium);

  TGeoCombiTrans *t0 = new TGeoCombiTrans();
  TGeoCombiTrans *pGlobalc = GetGlobalPosition(t0);

  // add the sphere as Mother Volume
  pAWorld->AddNodeOverlap(pWorld, 0, pGlobalc);


  //finally the v7.05 code

#include "perlScripts/CALIFA.geo"

}



/*
void R3BCalo::ConstructGeometry() {

  FairGeoLoader*    geoLoad = FairGeoLoader::Instance();
  FairGeoInterface* geoFace = geoLoad->getGeoInterface();
  R3BGeoCalo*       stsGeo  = new R3BGeoCalo();
  stsGeo->setGeomFile(GetGeometryFileName());
  geoFace->addGeoModule(stsGeo);

  Bool_t rc = geoFace->readSet(stsGeo);
  if (rc) stsGeo->create(geoLoad->getGeoBuilder());
  TList* volList = stsGeo->getListOfVolumes();
  // store geo parameter
  FairRun *fRun = FairRun::Instance();
  FairRuntimeDb *rtdb= FairRun::Instance()->GetRuntimeDb();
  R3BGeoCaloPar* par=(R3BGeoCaloPar*)(rtdb->getContainer("R3BGeoCaloPar"));
  TObjArray *fSensNodes = par->GetGeoSensitiveNodes();
  TObjArray *fPassNodes = par->GetGeoPassiveNodes();

  TListIter iter(volList);
  FairGeoNode* node   = NULL;
  FairGeoVolume *aVol=NULL;

  while( (node = (FairGeoNode*)iter.Next()) ) {
      aVol = dynamic_cast<FairGeoVolume*> ( node );
       if ( node->isSensitive()  ) {
           fSensNodes->AddLast( aVol );
       }else{
           fPassNodes->AddLast( aVol );
       }
  }
  par->setChanged();
  par->setInputVersion(fRun->GetRunId(),1);
  ProcessNodes( volList );

}
*/

ClassImp(R3BCalo)
