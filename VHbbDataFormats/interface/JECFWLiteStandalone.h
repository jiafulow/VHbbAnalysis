#ifndef JECFWLITESTANDALONE_H
#define JECFWLITESTANDALONE_H

#include <iostream>
#include <string>
#include "CondFormats/JetMETObjects/interface/FactorizedJetCorrector.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"

class JECFWLiteStandalone {
  public:

    JECFWLiteStandalone(std::string base, std::string jettype="AK5PFchs") {
        //std::string prefix = base + "/Summer12V3MC";
        std::string prefix = base + "/START53_V15MC";
        parMC.push_back( JetCorrectorParameters((prefix+"_L1FastJet_"+jettype+".txt").c_str()));
        parMC.push_back( JetCorrectorParameters((prefix+"_L2Relative_"+jettype+".txt").c_str()));
        parMC.push_back( JetCorrectorParameters((prefix+"_L3Absolute_"+jettype+".txt").c_str()));
        jetCorrectorMC = new FactorizedJetCorrector(parMC);
        jecUncMC = new JetCorrectionUncertainty((prefix+"_Uncertainty_"+jettype+".txt").c_str());

        prefix = base + "/ReferenceMC";
        parMCRef.push_back( JetCorrectorParameters((prefix+"_L1FastJet_"+jettype+".txt").c_str()));
        parMCRef.push_back( JetCorrectorParameters((prefix+"_L2Relative_"+jettype+".txt").c_str()));
        parMCRef.push_back( JetCorrectorParameters((prefix+"_L3Absolute_"+jettype+".txt").c_str()));
        jetCorrectorMCRef = new FactorizedJetCorrector(parMCRef);
        jecUncMCRef = new JetCorrectionUncertainty((prefix+"_Uncertainty_"+jettype+".txt").c_str());

        prefix = base + "/GR_P_V42_AN3DATA";
        parData.push_back( JetCorrectorParameters((prefix+"_L1FastJet_"+jettype+".txt").c_str()));
        parData.push_back( JetCorrectorParameters((prefix+"_L2Relative_"+jettype+".txt").c_str()));
        parData.push_back( JetCorrectorParameters((prefix+"_L3Absolute_"+jettype+".txt").c_str()));
        parData.push_back( JetCorrectorParameters((prefix+"_L2L3Residual_"+jettype+".txt").c_str()));
        jetCorrectorData = new FactorizedJetCorrector(parData);
        jecUncData = new JetCorrectionUncertainty((prefix+"_Uncertainty_"+jettype+".txt").c_str());

        prefix = base + "/Reference";
        parDataRef.push_back( JetCorrectorParameters((prefix+"_L1FastJet_"+jettype+".txt").c_str()));
        parDataRef.push_back( JetCorrectorParameters((prefix+"_L2Relative_"+jettype+".txt").c_str()));
        parDataRef.push_back( JetCorrectorParameters((prefix+"_L3Absolute_"+jettype+".txt").c_str()));
        parDataRef.push_back( JetCorrectorParameters((prefix+"_L2L3Residual_"+jettype+".txt").c_str()));
        jetCorrectorDataRef = new FactorizedJetCorrector(parDataRef);
        jecUncDataRef = new JetCorrectionUncertainty((prefix+"_Uncertainty_"+jettype+".txt").c_str());
    }

    ~JECFWLiteStandalone() {
        delete jetCorrectorMC;
        delete jetCorrectorMCRef;
        delete jetCorrectorData;
        delete jetCorrectorDataRef;
        delete jecUncMC;
        delete jecUncMCRef;
        delete jecUncData;
        delete jecUncDataRef;
    }


    float correct(float eta, float pt, float ptRaw, float jetArea, float rho, bool isMC, bool checkRef=false) {
        FactorizedJetCorrector * corr = 0;
        if ( checkRef &&  isMC) corr = jetCorrectorMCRef;
        if (!checkRef &&  isMC) corr = jetCorrectorMC;
        if ( checkRef && !isMC) corr = jetCorrectorDataRef;
        if (!checkRef && !isMC) corr = jetCorrectorData;

        corr->setJetEta(eta);
        corr->setJetPt(ptRaw);
        corr->setJetA(jetArea);
        corr->setRho(rho);
        float scale = corr->getCorrection() * ptRaw / pt;
        return scale;
    }

    float uncert(float eta, float pt, bool isMC, bool checkRef = false) {
        JetCorrectionUncertainty * jecUnc = 0;
        if ( checkRef &&  isMC) jecUnc = jecUncMCRef;
        if (!checkRef &&  isMC) jecUnc = jecUncMC;
        if ( checkRef && !isMC) jecUnc = jecUncDataRef;
        if (!checkRef && !isMC) jecUnc = jecUncData;

        jecUnc->setJetEta(eta);
        jecUnc->setJetPt(pt); // here you must use the CORRECTED jet pt
        float unc = jecUnc->getUncertainty(true);
        return unc;
    }


    std::vector<JetCorrectorParameters> parMC;
    std::vector<JetCorrectorParameters> parMCRef;
    std::vector<JetCorrectorParameters> parData;
    std::vector<JetCorrectorParameters> parDataRef;
    FactorizedJetCorrector * jetCorrectorMC;
    FactorizedJetCorrector * jetCorrectorMCRef;
    FactorizedJetCorrector * jetCorrectorData;
    FactorizedJetCorrector * jetCorrectorDataRef;
    JetCorrectionUncertainty * jecUncMC;
    JetCorrectionUncertainty * jecUncMCRef;
    JetCorrectionUncertainty * jecUncData;
    JetCorrectionUncertainty * jecUncDataRef;
};

#endif  // JECFWLITESTANDALONE_H
