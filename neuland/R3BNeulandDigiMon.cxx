#include "R3BNeulandDigiMon.h"

#include <iostream>
#include <algorithm>
#include <numeric>

#include "TClonesArray.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"

#include "FairRootManager.h"
#include "FairLogger.h"

#include "R3BLandDigi.h"


R3BNeulandDigiMon::R3BNeulandDigiMon(const Option_t *option) : FairTask("R3B NeuLAND NeulandDigi Monitor")
{
    LOG(INFO) << "Using R3B NeuLAND NeulandDigi Monitor" << FairLogger::endl;

    TString opt = option;
    opt.ToUpper();

    if (opt.Contains("3DTRACK")) {
        fIs3DTrackEnabled = true;
        LOG(INFO) << "... with 3D track visualization" << FairLogger::endl;
    } else {
        fIs3DTrackEnabled = false;
    }
}


R3BNeulandDigiMon::~R3BNeulandDigiMon()
{}


InitStatus R3BNeulandDigiMon::Init()
{
    FairRootManager *rm = FairRootManager::Instance();

    fDigis = (TClonesArray *) rm->GetObject("LandDigi");

    if (fIs3DTrackEnabled) {
        // XYZ -> ZXY (side view)
        fh3 = new TH3D("hDigis", "hDigis", 60, 1400, 1700, 50, -125, 125, 50, -125, 125);
        fh3->SetTitle("NeuLAND Digis");
        fh3->GetXaxis()->SetTitle("Z");
        fh3->GetYaxis()->SetTitle("X");
        fh3->GetZaxis()->SetTitle("Y");

        rm->Register("NeulandDigiMon", "Digis in NeuLAND", fh3, kTRUE);
    }

    hDepth = new TH1D("hDepth", "Maxial penetration depth", 60, 1400, 1700);
    hForemostEnergy = new TH1D("hForemostEnergy", "Foremost energy deposition", 100, 0, 100);
    hSternmostEnergy = new TH1D("hSternmostEnergy", "Sternmost energy deposition", 100, 0, 100);
    hDepthVSForemostEnergy = new TH2D("hDepthVSFrontEnergy", "Depth vs Foremost Energy", 60, 1400, 1700, 100, 0, 100);
    hDepthVSSternmostEnergy = new TH2D("hDepthVSSternmostEnergy", "Depth vs Sternmost Energy", 60, 1400, 1700, 100, 0, 100);
    hEtot = new TH1D("hEtot", "Total Energy", 1000, 0, 1000);
    hDepthVSEtot = new TH2D("hDepthVSEtot", "Depth vs Total Energy", 60, 1400, 1700, 1000, 0, 1000);
    hPosVSEnergy = new TH2D("hPosVSEnergy", "Position vs Energy deposition", 60, 1400, 1700, 1000, 0, 1000);

    return kSUCCESS;
}


void R3BNeulandDigiMon::Exec(Option_t *)
{
    const unsigned int nDigis = fDigis->GetEntries();

    if (fIs3DTrackEnabled) {
        fh3->Reset("ICES");
        R3BLandDigi *digi;
        for (unsigned int i = 0; i < nDigis; i++) {
            digi = (R3BLandDigi *) fDigis->At(i);
            // XYZ -> ZXY (side view)
            fh3->Fill(digi->GetZZ(), digi->GetXX(), digi->GetYY(), digi->GetQdc());
        }
    }

    std::vector<R3BLandDigi *> digis;
    for (unsigned int i = 0; i < nDigis; i++) {
        digis.push_back((R3BLandDigi *) fDigis->At(i));
    }


    for (auto digi : digis) {
        hPosVSEnergy->Fill(digi->GetZZ(), digi->GetQdc());
    }



    auto maxDepthDigi = std::max_element(digis.begin(), digis.end(), [](R3BLandDigi * a, R3BLandDigi * b) {
        return a->GetZZ() < b->GetZZ();
    });
    if (maxDepthDigi != digis.end()) {
        hDepth->Fill((*maxDepthDigi)->GetZZ());
        hSternmostEnergy->Fill((*maxDepthDigi)->GetQdc());
        hDepthVSSternmostEnergy->Fill((*maxDepthDigi)->GetZZ(), (*maxDepthDigi)->GetQdc());
    }


    auto minDepthDigi = std::min_element(digis.begin(), digis.end(), [](R3BLandDigi * a, R3BLandDigi * b) {
        return a->GetZZ() < b->GetZZ();
    });
    if (minDepthDigi != digis.end()) {
        hForemostEnergy->Fill((*minDepthDigi)->GetQdc());
        hDepthVSForemostEnergy->Fill((*maxDepthDigi)->GetZZ(), (*minDepthDigi)->GetQdc());
    }


    auto Etot = std::accumulate(digis.begin(), digis.end(), Double_t(0.), [](const Double_t a, R3BLandDigi * b) {
        return a + b->GetQdc();
    });
    hEtot->Fill(Etot);
    if (maxDepthDigi != digis.end()) {
        hDepthVSEtot->Fill((*maxDepthDigi)->GetZZ(), Etot);
    }
}


void R3BNeulandDigiMon::Finish()
{
    hDepth->Write();
    hForemostEnergy->Write();
    hSternmostEnergy->Write();
    hDepthVSForemostEnergy->Write();
    hDepthVSSternmostEnergy->Write();
    hEtot->Write();
    hDepthVSEtot->Write();
    hPosVSEnergy->Write();
}


ClassImp(R3BNeulandDigiMon)
