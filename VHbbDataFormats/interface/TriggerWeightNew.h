#ifndef TriggerWeightNew__H
#define TriggerWeightNew__H


#include <iostream>

#include "TH1.h"
#include "TF1.h"
#include "TFile.h"
#include "TTree.h"

#include "FWCore/ParameterSet/interface/ProcessDesc.h"
#include "FWCore/PythonParameterSet/interface/PythonProcessDesc.h"
#include "VHbbAnalysis/VHbbDataFormats/interface/MultiThresholdEfficiency.h"


class TriggerWeight {
  public:
    TriggerWeight(const edm::ParameterSet & ana): combiner2Thr(2), combiner1Thr(1) {
        tscaleHLTmu = openFile(ana, "hltMuFileName");
        tscaleIDmu = openFile(ana, "idMuFileName");
        tscaleHLTele1 = openFile(ana, "hltEle1FileName");
        tscaleHLTele2 = openFile(ana, "hltEle2FileName");
        tscaleHLTele1Aug = openFile(ana, "hltEle1AugFileName");
        tscaleHLTele2Aug = openFile(ana, "hltEle2AugFileName");
        tscaleID80Ele = openFile(ana, "idEle80FileName");
        tscaleID95Ele = openFile(ana, "idEle95FileName");
        tscaleHLTeleJet1 = openFile(ana, "hltJetEle1FileName");
        tscaleHLTeleJet2 = openFile(ana, "hltJetEle2FileName");
        tscaleRecoEle = openFile(ana, "recoEleFileName");
        //tscalePFMHTele=openFile(ana,"hltPFMHTEleFileName");
        tscaleSingleEleMay = openFile(ana, "hltSingleEleMayFileName");
        tscaleSingleEleV4 = openFile(ana, "hltSingleEleV4FileName");
        tscaleHLTmuOr30 = openFile(ana, "hltMuOr30FileName");

        tscaleSingleEle2012Awp95 = openFile(ana, "hltSingleEle2012Awp95");
        tscaleSingleEle2012Awp80 = openFile(ana, "hltSingleEle2012Awp80");
        tscaleSingleMuon2012A = openFile(ana, "hltSingleMuon2012A");
        tscaleDoubleEle2012A_leg8 = openFile(ana, "hltDoubleEle2012A_leg8");
        tscaleDoubleEle2012A_leg17 = openFile(ana, "hltDoubleEle2012A_leg17");
        tscaleDoubleMuon2012A_leg8 = openFile(ana, "hltDoubleMuon2012A_leg8");
        tscaleDoubleMuon2012A_leg17 = openFile(ana, "hltDoubleMuon2012A_leg17");

        tscaleDoubleMuon2012A_dZ = openFile(ana, "hltDoubleMuon2012A_dZ");
        tscaleDoubleEle2012A_dZ = openFile(ana, "hltDoubleEle2012A_dZ");

        tscaleMuPlusWCandPt2012A_legMu = openFile(ana, "hltMuPlusWCandPt2012A_legMu");
        tscaleMuPlusWCandPt2012A_legW = openFile(ana, "hltMuPlusWCandPt2012A_legW");

        tscaleMuID2012A = openFile(ana, "idMu2012A");
        tscaleEleID2012A = openFile(ana, "idEle2012A");
        tscaleEleID2012Awp80 = openFile(ana, "idEle2012Awp80");

        if (tscaleHLTmu == 0 || tscaleIDmu == 0) {
            std::cout << "ERROR: cannot load Muon Trigger efficiencies" << std::endl;
        }
    }

    static TTree * openFile(const edm::ParameterSet & ana, const char *name) {
        TFile *hltMuFile = new TFile(ana.getParameter < std::string > (name).c_str(), "read");
        if (hltMuFile)
            return (TTree *) hltMuFile->Get("tree");
        else
            return 0;
    }

    static std::pair < float, float > efficiencyFromPtEta(float pt1, float eta1, TTree * t) {
        //std::cout << " pt1 "  << pt1 << " eta1 " << eta1 <<  std::endl;
        float s1 = 1., err = 1.;
        std::pair < float, float > r(s1, err);
        if (!t)  return r;
        float ptMin, ptMax, etaMin, etaMax, scale, error;
        int count = 0;
        t->SetBranchAddress("ptMin", &ptMin);
        t->SetBranchAddress("ptMax", &ptMax);
        t->SetBranchAddress("etaMin", &etaMin);
        t->SetBranchAddress("etaMax", &etaMax);
        t->SetBranchAddress("scale", &scale);
        t->SetBranchAddress("error", &error);
        float lastPtBin = 200;  // does this value make sense?

        for (int jentry = 0; jentry < t->GetEntries(); jentry++) {
            t->GetEntry(jentry);
            //if (ptMax >= lastPtBin) 
            //    lastPtBin = ptMax;
            if (ptMax >= lastPtBin)
                ptMax = 999999.;
            if ((pt1 > ptMin) && (pt1 < ptMax) && (eta1 > etaMin) && (eta1 < etaMax)) {
                s1 = scale;
                err = error;
                count++;  // should break out?
            }
        }

        if (count == 0)
            return r;

        r.first = s1;
        r.second = err;
        return r;
    }

    float scaleMuIsoHLT(float pt1, float eta1) {
        return efficiencyFromPtEta(pt1, eta1, tscaleHLTmu).first;
    }

    float scaleMuID(float pt1, float eta1) {
        return efficiencyFromPtEta(pt1, eta1, tscaleIDmu).first;
    }
       
    float scaleMuOr30IsoHLT(float pt1, float eta1) {
        return efficiencyFromPtEta(pt1, eta1, tscaleHLTmuOr30).first;
    }

    float scaleDoubleEle17Ele8Aug(const std::vector < float > & pt, 
                                  const std::vector < float > & eta) {
        std::vector < std::vector < float > > allEleWithEffs;
        for (unsigned int j = 0; j < pt.size() && j < 10; j++) {
            std::vector < float >thisEleEffs;
            thisEleEffs.push_back(efficiencyFromPtEta(pt[j], eta[j], tscaleHLTele1Aug).first);
            thisEleEffs.push_back(efficiencyFromPtEta(pt[j], eta[j], tscaleHLTele2Aug).first);
            allEleWithEffs.push_back(thisEleEffs);
        }
        return combiner2Thr.weight < Trigger1High2Loose > (allEleWithEffs);
    }

    float scaleDoubleEle17Ele8(const std::vector < float > & pt, 
                               const std::vector < float > & eta) {
        std::vector < std::vector < float > > allEleWithEffs;
        for (unsigned int j = 0; j < pt.size() && j < 10; j++) {
            std::vector < float >thisEleEffs;
            thisEleEffs.push_back(efficiencyFromPtEta(pt[j], eta[j], tscaleHLTele1).first);
            thisEleEffs.push_back(efficiencyFromPtEta(pt[j], eta[j], tscaleHLTele2).first);
            allEleWithEffs.push_back(thisEleEffs);
        }
        return combiner2Thr.weight < Trigger1High2Loose > (allEleWithEffs);
    }
    
    float muId2012A(float pt, float eta) {
        return efficiencyFromPtEta(pt, eta, tscaleMuID2012A).first;
    }
    
    float eleId2012A(float pt, float eta) {
        return efficiencyFromPtEta(pt, eta, tscaleEleID2012A).first;
    }
    
    float eleId2012Awp80(float pt, float eta) {
        return efficiencyFromPtEta(pt, eta, tscaleEleID2012Awp80).first;
    }

    float scaleSingleEleMay(float pt, float eta) {
        return efficiencyFromPtEta(pt, eta, tscaleSingleEleMay).first;
    }
    
    float scaleSingleEleV4(float pt, float eta) {
        return efficiencyFromPtEta(pt, eta, tscaleSingleEleV4).first;
    }
    
    float scaleID80Ele(float pt, float eta) {
        return efficiencyFromPtEta(pt, eta, tscaleID80Ele).first;
    }
    
    float scaleID95Ele(float pt, float eta) {
        return efficiencyFromPtEta(pt, eta, tscaleID95Ele).first;
    }
    
    float scaleRecoEle(float pt, float eta) {
        return efficiencyFromPtEta(pt, eta, tscaleRecoEle).first;
    }
    
    float scalePFMHTEle(float MetPFPt) {
        float weightPFMHTrigger = 0.;
        //FIXME: read from file
        if (MetPFPt > 0. && MetPFPt < 5.)  weightPFMHTrigger = 0.305;
        if (MetPFPt > 5. && MetPFPt < 10.)  weightPFMHTrigger = 0.351;
        if (MetPFPt > 10. && MetPFPt < 15.)  weightPFMHTrigger = 0.461;
        if (MetPFPt > 15. && MetPFPt < 20.)  weightPFMHTrigger = 0.572;
        if (MetPFPt > 20. && MetPFPt < 25.)  weightPFMHTrigger = 0.713;
        if (MetPFPt > 25. && MetPFPt < 30.)  weightPFMHTrigger = 0.844;
        if (MetPFPt > 30. && MetPFPt < 35.)  weightPFMHTrigger = 0.914;
        if (MetPFPt > 35. && MetPFPt < 40.)  weightPFMHTrigger = 0.939;
        if (MetPFPt > 40. && MetPFPt < 45.)  weightPFMHTrigger = 0.981;
        if (MetPFPt > 45. && MetPFPt < 50.)  weightPFMHTrigger = 0.982;
        if (MetPFPt > 50. && MetPFPt < 60.)  weightPFMHTrigger = 0.993;
        if (MetPFPt > 60. && MetPFPt < 70.)  weightPFMHTrigger = 0.995;
        if (MetPFPt > 70. && MetPFPt < 100.)  weightPFMHTrigger = 0.995;
        if (MetPFPt > 100.)  weightPFMHTrigger = 1.;
        return weightPFMHTrigger;
    }

    float scaleJet30Jet25(const std::vector < float > & pt, 
                          const std::vector < float > & eta) {
        std::vector < std::vector < float > > allJetsWithEffs;
        for (unsigned int j = 0; j < pt.size() && j < 10; j++) {
            std::vector < float >thisJetEffs;
            thisJetEffs.push_back(efficiencyFromPtEta(pt[j], eta[j], tscaleHLTeleJet1).first);
            thisJetEffs.push_back(efficiencyFromPtEta(pt[j], eta[j], tscaleHLTeleJet2).first);
            //std::cout << " jet pt " << pt[j] << " eta " << eta[j] << " eff1 "  <<  thisJetEffs[0] << " eff2 " << thisJetEffs[1] << std::endl;
            allJetsWithEffs.push_back(thisJetEffs);
        }
        float res = combiner2Thr.weight < Trigger1High2Loose > (allJetsWithEffs);
        //std::cout << "Result is " << res << std::endl;
        return res;
    }

    float doubleEle2012A(float pt1, float eta1, float pt2, float eta2) {
        //std::cout << "di ele" << std::endl;
        float eff1_17 = efficiencyFromPtEta(pt1, eta1, tscaleDoubleEle2012A_leg17).first;
        float eff2_17 = efficiencyFromPtEta(pt2, eta2, tscaleDoubleEle2012A_leg17).first;
        float eff1_8 = efficiencyFromPtEta(pt1, eta1, tscaleDoubleEle2012A_leg8).first;
        float eff2_8 = efficiencyFromPtEta(pt2, eta2, tscaleDoubleEle2012A_leg8).first;
        //std::cout << tscaleDoubleEle2012A_dZ << std::endl;
        float eff_dz = efficiencyFromPtEta(eta1, eta2, tscaleDoubleEle2012A_dZ).first;  // despite the name pt,eta is actually eta1,eta2
        return (eff1_17 * eff2_8 + eff2_17 * eff1_8 - eff1_17 * eff2_17) * eff_dz;
    }
    
    float doubleMuon2012A(float pt1, float eta1, float pt2, float eta2) {
        //std::cout << "di mu" << std::endl;
        float eff1_17 = efficiencyFromPtEta(pt1, eta1, tscaleDoubleMuon2012A_leg17).first;
        float eff2_17 = efficiencyFromPtEta(pt2, eta2, tscaleDoubleMuon2012A_leg17).first;
        float eff1_8 = efficiencyFromPtEta(pt1, eta1, tscaleDoubleMuon2012A_leg8).first;
        float eff2_8 = efficiencyFromPtEta(pt2, eta2, tscaleDoubleMuon2012A_leg8).first;
        //std::cout << tscaleDoubleMuon2012A_dZ << std::endl;
        float eff_dz = efficiencyFromPtEta(eta1, eta2, tscaleDoubleMuon2012A_dZ).first; // despite the name pt,eta is actually eta1,eta2
        return (eff1_17 * eff2_8 + eff2_17 * eff1_8 - eff1_17 * eff2_17) * eff_dz;
    }

    float muPlusWCandPt2012A_legW(float pt, float eta) {
        return efficiencyFromPtEta(pt, eta, tscaleMuPlusWCandPt2012A_legW).first;
    }
    
    float muPlusWCandPt2012A_legMu(float pt, float eta) {
        return efficiencyFromPtEta(pt, eta, tscaleMuPlusWCandPt2012A_legMu).first;
    }
    
    float singleEle2012Awp80(float pt, float eta) {
        return efficiencyFromPtEta(pt, eta, tscaleSingleEle2012Awp80).first;
    }
    
    float singleEle2012Awp95(float pt, float eta) {
        return efficiencyFromPtEta(pt, eta, tscaleSingleEle2012Awp95).first;
    }
    
    float singleMuon2012A(float pt, float eta) {
        return efficiencyFromPtEta(pt, eta, tscaleSingleMuon2012A).first;
    }
    

    //LP curve used for MET
    float scaleMetHLT(float met) {
        return 1. / (1. + (exp(0.059486 * (123.27 - met))));
    }

    //MET80 component of the factorized JET+MET trigger
    float scaleMET80(float met) {
        return 1. / (1. + exp(-0.0709 * (met - 100.7)));
    }

    //MET100 component
    float scaleMET100(float met) {
        return 1. / (1. + exp(-0.0679 * (met - 128.8)));
    }

    //Single jet20 efficiency for MET+2CJet20
    float jet20efficiency(float pt) {
        if (pt < 10)  return 0;
        return 1. - exp(-0.157 * (pt - 19.3));
    }

    //combined 2 jets efficiency out of N jets, using jet20 efficiency curve
    float scale2CentralJet(const std::vector < float > & pt, 
                            const std::vector < float > & eta) {
        std::vector < std::vector < float > > allJetsWithEffs;
        for (unsigned int j = 0; j < pt.size() && j < 10; j++) {
            if (fabs(eta[j]) < 2.5) {
                std::vector < float > thisJetEffs;
                thisJetEffs.push_back(jet20efficiency(pt[j]));
                allJetsWithEffs.push_back(thisJetEffs);
            }
        }
        return combiner1Thr.weight < Trigger2SingleThr > (allJetsWithEffs);
    }

    //New MET 150 
    float scaleMET150(float et) {
        return 1. / (1. + exp(-0.129226 * (et - 156.699)));
    }

    //For 2012A HLT_DiCentralPFJet30_PFMHT80, valid for pfMET > 100 GeV:
    float scaleDiJet30MHT80_2012A(float x) {
        if (x < 100)  return 0;
        return (1e0 - exp(-0.04197 * (x - 75.73))) * 0.9721;
    }
    
    //For 2012B HLT_DiCentralJetSumpT100_dPhi05_DiCentralPFJet60_25_PFMET100_HBHENoiseCleaned, valid for pfMET > 100 GeV:
    float scaleSumpT100MET100_2012B(float x) {
        if (x < 100)  return 0;
        return (1e0 - exp(-0.06704 * (x - 96.84))) * 0.9199;
    }
    
    //For 2012A+B HLT_PFMET150, valid for pfMET > 150 GeV:
    float scalePFMET150_2012AB(float x) {
        if (x < 150)  return 0;
        return (1e0 - exp(-0.07135 * (x - 147.4))) * 0.9707;
    }

    //For 2012A HLT_PFMET150 OR HLT_DiCentralPFJet30_PFMHT80, valid for pfMET > 100 GeV:
    float scalePFMET150orDiJetMET_2012A(float x) {
        if (x < 100)  return 0;
        return (1e0 - exp(-0.0412 * (x - 75.52))) * 0.9772;
    }

    //For 2012B HLT_PFMET150 OR HLT_DiCentralJetSumpT100_dPhi05_DiCentralPFJet60_25_PFMET100_HBHENoiseCleaned, valid for pfMET > 100 GeV:
    float scalePFMET150orDiJetMET_2012B(float x) {
        if (x < 100)  return 0;
        return (1e0 - exp(-0.05482 * (x - 95.59))) * 0.9702;
    }

    //For 2012C HLT_PFMET150 OR HLT_DiCentralJetSumpT100_dPhi05_DiCentralPFJet60_25_PFMET100_HBHENoiseCleaned, valid for pfMET > 100 GeV:
    float scalePFMET150orDiJetMET_2012C(float x) {
        if (x < 100)  return 0;
        return (1e0 - exp(-0.05627 * (x - 95.15))) * 0.9659;
    }


  private:
    TTree * tscaleHLTele1;
    TTree * tscaleHLTele2;
    TTree * tscaleHLTeleJet1;
    TTree * tscaleHLTeleJet2;
    TTree * tscaleID80Ele;
    TTree * tscaleID95Ele;
    TTree * tscaleRecoEle;
    TTree * tscaleHLTmuOr30;

    TTree * tscaleSingleEle2012Awp95;
    TTree * tscaleSingleEle2012Awp80;
    TTree * tscaleSingleMuon2012A;
    TTree * tscaleDoubleEle2012A_leg8;
    TTree * tscaleDoubleEle2012A_leg17;
    TTree * tscaleDoubleMuon2012A_leg8;
    TTree * tscaleDoubleMuon2012A_leg17;
    TTree * tscaleMuPlusWCandPt2012A_legMu;
    TTree * tscaleMuPlusWCandPt2012A_legW;
    TTree * tscaleDoubleEle2012A_dZ;
    TTree * tscaleDoubleMuon2012A_dZ;
    //TTree * tscalePFMHTele;
    TTree * tscaleSingleEleMay;
    TTree * tscaleSingleEleV4;

    TTree * tscaleHLTele1Aug;
    TTree * tscaleHLTele2Aug;

    TTree * tscaleMuID2012A;
    TTree * tscaleEleID2012A;
    TTree * tscaleEleID2012Awp80;

    TTree * tscaleHLTmu;
    TTree * tscaleIDmu;
    MultiThresholdEfficiency combiner2Thr;
    MultiThresholdEfficiency combiner1Thr;
};
#endif  // TriggerWeightNew__H

