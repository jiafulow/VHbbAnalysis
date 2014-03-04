#ifndef VHbbCandidateTools__H
#define VHbbCandidateTools__H

#include "VHbbAnalysis/VHbbDataFormats/interface/VHbbCandidate.h"


class VHbbCandidateTools {
  public:
    VHbbCandidateTools(bool verbose = false): verbose_(verbose) {}

    VHbbCandidate getHZmumuCandidate(const VHbbCandidate & in, bool & ok,
                                     const std::vector < unsigned int > & pos);

    VHbbCandidate getHZeeCandidate(const VHbbCandidate & in, bool & ok,
                                   const std::vector < unsigned int > & pos);

    VHbbCandidate getHWmunCandidate(const VHbbCandidate & in, bool & ok,
                                    const std::vector < unsigned int > & pos);

    VHbbCandidate getHWenCandidate(const VHbbCandidate & in, bool & ok,
                                   const std::vector < unsigned int > & pos);

    VHbbCandidate getHZnnCandidate(const VHbbCandidate & in, bool & ok);

    VHbbCandidate getHZtaumuCandidate(const VHbbCandidate & in, bool & ok,
                                      const std::vector < unsigned int > & muPos,
                                      const std::vector < unsigned int > & tauPos);

    VHbbCandidate getHWtaunCandidate(const VHbbCandidate & in, bool & ok,
                                     const std::vector < unsigned int > & pos);

    VHbbCandidate getHZbbCandidate(const VHbbCandidate & in, bool & ok);

    /// Can be called without a class instance
    static float getDeltaTheta(const VHbbEvent::SimpleJet & j1,
                               const VHbbEvent::SimpleJet & j2) {
        double deltaTheta = 1e10;
        TLorentzVector pi(0, 0, 0, 0);
        TLorentzVector v_j1 = j1.chargedTracksFourMomentum;
        TLorentzVector v_j2 = j2.chargedTracksFourMomentum;

        if (v_j1.Mag() < 1e-9 || v_j2.Mag() < 1e-9)
            return deltaTheta;

        //use j1 to calculate the pull vector
        TVector2 t = j1.tVector;
        if (t.Mod() < 1e-9)
            return deltaTheta;

        Double_t dphi = v_j2.Phi() - v_j1.Phi();
        if (dphi > M_PI) {
            dphi -= 2.0 * M_PI;
        } else if (dphi <= -M_PI) {
            dphi += 2.0 * M_PI;
        }
        Double_t deltaeta = v_j2.Rapidity() - v_j1.Rapidity();
        TVector2 BBdir(deltaeta, dphi);
        deltaTheta = t.DeltaPhi(BBdir);
        return deltaTheta;
    }

    float getHelicity(const VHbbEvent::SimpleJet & j, const TVector3 & boost) const {
        double hel = 1e10;
        TLorentzVector jet = j.p4;
        jet.Boost(-boost);
        hel = TMath::Cos(jet.Vect().Angle(boost));
        return hel;
    }

  private:
    bool verbose_;
};

#endif  // VHbbCandidateTools__H
