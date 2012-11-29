#ifndef VHbbCandidate__H
#define VHbbCandidate__H

#include <vector>

#include "TLorentzVector.h"
#include "TVector2.h"

#include "VHbbAnalysis/VHbbDataFormats/interface/VHbbEvent.h"


class VHbbCandidate {
  public:
    enum CandidateType { Zmumu, Zee, Wmun, Wen, Znn, UNKNOWN };
    // Zmumu = 0 
    // Zee   = 1 
    // Wmun  = 2
    // Wen   = 3
    // Znn   = 4

    VHbbCandidate() {
        candidateType = UNKNOWN;
    }

    class VectorCandidate {
      public:
        VectorCandidate(): firstLepton(9999), secondLepton(9999) {}
        double Mt(CandidateType candidateType) const {
            if (candidateType == Wen) {
                float ptl = electrons[0].p4.Pt();
                float met = mets[0].p4.Pt();
                float et = ptl + met;
                return sqrt(et * et - p4.Pt() * p4.Pt());

            } else if (candidateType == Wmun) {
                float ptl = muons[0].p4.Pt();
                float met = mets[0].p4.Pt();
                float et = ptl + met;
                return sqrt(et * et - p4.Pt() * p4.Pt());
            }
            return 0;
        }
        TLorentzVector p4;
        std::vector < VHbbEvent::MuonInfo > muons;
        std::vector < VHbbEvent::ElectronInfo > electrons;
        std::vector < VHbbEvent::TauInfo > taus;
        std::vector < VHbbEvent::METInfo > mets;
        unsigned int firstLepton, secondLepton;
    };

    class HiggsCandidate {
      public:
        HiggsCandidate(): HiggsFlag(false) {}
        VHbbEvent::SimpleJet & firstJet() { return jets[0]; }
        VHbbEvent::SimpleJet & secondJet() { return jets[1]; }
        
        TLorentzVector p4;
        std::vector < VHbbEvent::SimpleJet > jets;
        bool HiggsFlag;
        float deltaTheta;
        std::vector < float > helicities;
    };

    class FatHiggsCandidate {
      public:
        FatHiggsCandidate(): FatHiggsFlag(false) {}
        VHbbEvent::SimpleJet & firstJet() { return jets[0]; }
        VHbbEvent::SimpleJet & secondJet() { return jets[1]; }
        TLorentzVector p4;
        std::vector < VHbbEvent::SimpleJet > jets;
        bool FatHiggsFlag;
        float deltaTheta;
        std::vector < float > helicities;
        int subjetsSize;  // should be unsigned int?
    };

    void setCandidateType(CandidateType c) {
        candidateType = c;
    }

    double deltaPhi() const {
        return V.p4.DeltaPhi(H.p4);
    }

    double Mt() const {  // should be MT(V,H) instead of MT(V)?
        return V.Mt(candidateType);
    }

    int additionalLeptons() const {  // should be unsigned int?
        int expectedLeptons = 0;
        if (candidateType == Wmun || candidateType == Wen)
            expectedLeptons = 1;
        else if (candidateType == Zmumu || candidateType == Zee)
            expectedLeptons = 2;
        return (V.muons.size() + V.electrons.size() - expectedLeptons);
    }

    TLorentzVector p4() const {
        return V.p4 + H.p4;
    }
    
    CandidateType candidateType;
    HiggsCandidate H;
    FatHiggsCandidate FatH;
    VectorCandidate V;
    std::vector < VHbbEvent::SimpleJet > additionalJets;
    std::vector < VHbbEvent::SimpleJet > additionalJetsFat;
};
#endif  // VHbbCandidate__H

