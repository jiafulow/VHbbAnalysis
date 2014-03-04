#include <iostream>

#include "VHbbAnalysis/VHbbDataFormats/interface/VHbbCandidateTools.h"


VHbbCandidate VHbbCandidateTools::getHZmumuCandidate(const VHbbCandidate & in, bool & ok,
                                                     const std::vector < unsigned int > & pos) {
    if (verbose_) {
        std::cout << " getHZmumuCandidate input mu " << in.V.muons.size() << " e " << in.V.electrons.size() << std::endl;
    }
    ok = false;
    VHbbCandidate temp = in;

    // Allow for additional leptons
    if (temp.V.muons.size() < 2)
        return in;

    // Beware: assume muons are already sorted by pT!!
    const std::vector < VHbbEvent::MuonInfo > & muons_ = temp.V.muons;
    if (muons_[0].p4.Pt() < 20 || muons_[1].p4.Pt() < 20)
        return in;

    // Look for an opposite-charge pair
    unsigned int selectMu2 = 1;
    if (muons_[0].charge * muons_[selectMu2].charge > 0) {
        if (muons_.size() == 2)
            return in;

        // Find a proper pair
        for (unsigned int it = 2; it < muons_.size(); ++it) {
            if (muons_[0].charge * muons_[it].charge < 0) {
                selectMu2 = it;
                break;
            }
            if (selectMu2 == 1)  // 'it' can never reach 3?
                return in;
        }
    }
    temp.V.p4 = muons_[0].p4 + muons_[selectMu2].p4;

    // Save all muons with pT > 20
    std::vector < VHbbEvent::MuonInfo > muons2_;
    for (std::vector < VHbbEvent::MuonInfo >::const_iterator it = muons_.begin();
         it != muons_.end(); ++it) {
        if (it->p4.Pt() > 20)
            muons2_.push_back(*it);
    }
    temp.V.muons = muons2_;

    // Save all electrons with pT > 20
    const std::vector < VHbbEvent::ElectronInfo > & electrons_ = temp.V.electrons;
    std::vector < VHbbEvent::ElectronInfo > electrons2_;
    for (std::vector < VHbbEvent::ElectronInfo >::const_iterator it = electrons_.begin();
         it != electrons_.end(); ++it) {
        if (it->p4.Pt() > 20)
            electrons2_.push_back(*it);
    }
    temp.V.electrons = electrons2_;

    temp.V.firstLepton = 0;
    temp.V.secondLepton = selectMu2;
    temp.V.firstLeptonOrig = pos[0];
    temp.V.secondLeptonOrig = pos[selectMu2];
    ok = true;
    return temp;
}


VHbbCandidate VHbbCandidateTools::getHZeeCandidate(const VHbbCandidate & in, bool & ok,
                                                   const std::vector < unsigned int > & pos) {
    if (verbose_) {
        std::cout << " getHZeeCandidate input mu " << in.V.muons.size() << " e " << in.V.electrons.size() << std::endl;
    }
    ok = false;
    VHbbCandidate temp = in;

    // Allow for additional leptons
    if (temp.V.electrons.size() < 2)
        return in;

    // Beware: assume electrons are already sorted by pT!!
    const std::vector < VHbbEvent::ElectronInfo > & electrons_ = temp.V.electrons;
    if (electrons_[0].p4.Pt() < 20 || electrons_[1].p4.Pt() < 20)
        return in;

    // Look for an opposite-charge pair
    unsigned int selectE2 = 1;
    if (electrons_[0].charge * electrons_[selectE2].charge > 0) {
        if (electrons_.size() == 2)
            return in;

        // Find a proper pair
        for (unsigned int it = 2; it < electrons_.size(); ++it) {
            if (electrons_[0].charge * electrons_[it].charge < 0) {
                selectE2 = it;
                break;
            }
            if (selectE2 == 1)  // 'it' can never reach 3?
                return in;
        }
    }
    temp.V.p4 = electrons_[0].p4 + electrons_[selectE2].p4;

    // Save all electrons with pT > 20
    std::vector < VHbbEvent::ElectronInfo > electrons2_;
    for (std::vector < VHbbEvent::ElectronInfo >::const_iterator it = electrons_.begin();
         it != electrons_.end(); ++it) {
        if (it->p4.Pt() > 20)
            electrons2_.push_back(*it);
    }
    temp.V.electrons = electrons2_;

    // Save all muons with pT > 20
    const std::vector < VHbbEvent::MuonInfo > & muons_ = temp.V.muons;
    std::vector < VHbbEvent::MuonInfo > muons2_;
    for (std::vector < VHbbEvent::MuonInfo >::const_iterator it = muons_.begin();
         it != muons_.end(); ++it) {
        if (it->p4.Pt() > 20)
            muons2_.push_back(*it);
    }
    temp.V.muons = muons2_;

    temp.V.firstLepton = 0;
    temp.V.secondLepton = selectE2;
    temp.V.firstLeptonOrig = pos[0];
    temp.V.secondLeptonOrig = pos[selectE2];
    ok = true;
    return temp;
}


VHbbCandidate VHbbCandidateTools::getHWmunCandidate(const VHbbCandidate & in, bool & ok,
                                                    const std::vector < unsigned int > & pos) {
    if (verbose_) {
        std::cout << " getHWmunCandidate input mu " << in.V.muons.size() << " e " << in.V.electrons.size() << std::endl;
    }
    ok = false;
    VHbbCandidate temp = in;

    // Require exactly one muon and no electrons
    if (temp.V.muons.size() != 1)  // no check against pT?
        return in;
    if (temp.V.electrons.size() != 0)  // no check against pT?
        return in;
    if (temp.V.mets.size() < 1)
        return in;

    temp.V.p4 = temp.V.muons[0].p4 + temp.V.mets[0].p4;
    temp.V.firstLepton = 0;
    temp.V.firstLeptonOrig = pos[0];
    ok = true;
    return temp;
}


VHbbCandidate VHbbCandidateTools::getHWenCandidate(const VHbbCandidate & in, bool & ok,
                                                   const std::vector < unsigned int > & pos) {
    if (verbose_) {
        std::cout << " getHWenCandidate input mu " << in.V.muons.size() << " e " << in.V.electrons.size() << std::endl;
    }
    ok = false;
    VHbbCandidate temp = in;

    // Require exactly one electron and no muons
    if (temp.V.electrons.size() != 1)  // no check against pT?
        return in;
    if (temp.V.muons.size() != 0)  // no check against pT?
        return in;
    if (temp.V.mets.size() < 1)
        return in;

    temp.V.p4 = temp.V.electrons[0].p4 + temp.V.mets[0].p4;
    temp.V.firstLepton = 0;
    temp.V.firstLeptonOrig = pos[0];
    ok = true;
    return temp;
}


VHbbCandidate VHbbCandidateTools::getHZnnCandidate(const VHbbCandidate & in, bool & ok) {
    if (verbose_) {
        std::cout << " getHZnnCandidate input mu " << in.V.muons.size() << " e " << in.V.electrons.size()
                  << " met " << in.V.mets.size() << std::endl;
    }
    ok = false;
    VHbbCandidate temp = in;
    if (temp.V.mets.size() != 1)
        return in;
    temp.V.p4 = temp.V.mets[0].p4;

    if (temp.V.p4.Pt() < 80)
        return in;

    // Always build a Znn candidate if MET > 80 GeV.
    // Exclusion comes from if/else series on the caller side.
    // This allows candidates for mu+e  and same sign dileptons.
    ok = true;
    return temp;
}


VHbbCandidate VHbbCandidateTools::getHZtaumuCandidate(const VHbbCandidate & in, bool & ok,
                                                      const std::vector < unsigned int > & muPos,
                                                      const std::vector < unsigned int > & tauPos) {
    if (verbose_) {
        std::cout << " getHZtaumuCandidate input mu " << in.V.muons.size() << " e " << in.V.electrons.size()
                  << " tau " << in.V.taus.size() << std::endl;
    }
    ok = false;
    VHbbCandidate temp = in;
    // Require exactly one tau and one muon and no electrons
    if (temp.V.taus.size() != 1)  // no check against pT?
        return in;
    if (temp.V.muons.size() != 1)  // no check against pT?
        return in;
    if (temp.V.electrons.size() != 0)  // no check against pT?
        return in;

    temp.V.p4 = temp.V.taus[0].p4 + temp.V.muons[0].p4;
    temp.V.firstLepton = 0;
    temp.V.secondLepton = 0;
    temp.V.firstLeptonOrig = muPos[0];
    temp.V.secondLeptonOrig = tauPos[0];
    ok = true;
    return temp;
}


VHbbCandidate VHbbCandidateTools::getHWtaunCandidate(const VHbbCandidate & in, bool & ok,
                                                     const std::vector < unsigned int > & pos) {
    if (verbose_) {
        std::cout << " getHWtaunCandidate input mu " << in.V.muons.size() << " e " << in.V.electrons.size()
                  << " tau " << in.V.taus.size() << std::endl;
    }
    ok = false;
    VHbbCandidate temp = in;
    // Require exactly one tau and no electrons or muons
    if (temp.V.taus.size() != 1)  // no check against pT?
        return in;
    if (temp.V.muons.size() != 0)  // no check against pT?
        return in;
    if (temp.V.electrons.size() != 0)  // no check against pT?
        return in;
    if (temp.V.mets.size() < 1)
        return in;

    temp.V.p4 = temp.V.taus[0].p4 + temp.V.mets[0].p4;
    temp.V.firstLepton = 0;
    temp.V.firstLeptonOrig = pos[0];
    ok = true;
    return temp;
}


VHbbCandidate VHbbCandidateTools::getHZbbCandidate(const VHbbCandidate & in, bool & ok) {
    if (verbose_) {
        std::cout << " getHZbbCandidate " << std::endl;
    }
    ok = true;
    return in;  // get all the candidates
}
