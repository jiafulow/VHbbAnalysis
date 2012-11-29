#ifndef HbbCandidateFinderAlgo__H
#define HbbCandidateFinderAlgo__H

#include "VHbbAnalysis/VHbbDataFormats/interface/VHbbCandidate.h"
#include "VHbbAnalysis/VHbbDataFormats/interface/VHbbEvent.h"
#include "VHbbAnalysis/VHbbDataFormats/interface/VHbbEventAuxInfo.h"


class HbbCandidateFinderAlgo {
  public:
    explicit HbbCandidateFinderAlgo(bool verbose, float jetPt, bool useH)
      : verbose_(verbose), jetPtThreshold_(jetPt), useHighestPtHiggs_(useH) {}

    void run(const VHbbEvent *, std::vector < VHbbCandidate > &, const VHbbEventAuxInfo & aux);

    static bool jetID(const VHbbEvent::SimpleJet &);

    VHbbCandidate changeHiggs(bool useHighestPtHiggs, const VHbbCandidate & old);

    bool findDiJets(const std::vector < VHbbEvent::SimpleJet > &,
                    VHbbEvent::SimpleJet &, VHbbEvent::SimpleJet &,
                    std::vector < VHbbEvent::SimpleJet > &);

    bool findDiJetsHighestPt(const std::vector < VHbbEvent::SimpleJet > &,
                             VHbbEvent::SimpleJet &, VHbbEvent::SimpleJet &,
                             std::vector < VHbbEvent::SimpleJet > &);

    bool findFatJet(const std::vector < VHbbEvent::HardJet > &,
                    const std::vector < VHbbEvent::SimpleJet > &,
                    const std::vector < VHbbEvent::SimpleJet > &,
                    VHbbEvent::HardJet &,
                    std::vector < VHbbEvent::SimpleJet > &,
                    const std::vector < VHbbEvent::SimpleJet > &,
                    std::vector < VHbbEvent::SimpleJet > &,
                    const std::vector < VHbbEvent::MuonInfo > &muons,
                    const std::vector < VHbbEvent::ElectronInfo > &electrons);

  protected:
    void findMuons(const std::vector < VHbbEvent::MuonInfo > & muons,
                   std::vector < VHbbEvent::MuonInfo > & out,
                   std::vector < unsigned int > & positions,
                   const VHbbEventAuxInfo & aux);

    void findElectrons(const std::vector < VHbbEvent::ElectronInfo > & electrons,
                       std::vector < VHbbEvent::ElectronInfo > & out,
                       std::vector < unsigned int > & positions,
                       const VHbbEventAuxInfo & aux);

    void findMET(const VHbbEvent::METInfo & met,
                 std::vector < VHbbEvent::METInfo > & out);

    void findTaus(const std::vector < VHbbEvent::TauInfo > &taus,
                  std::vector < VHbbEvent::TauInfo > & out,
                  std::vector < unsigned int >& positions);

    void removeTauOverlapWithJets(const std::vector < VHbbEvent::TauInfo > & taus,
                                  const std::vector < VHbbEvent::SimpleJet > & jets,
                                  std::vector < VHbbEvent::TauInfo > & out,
                                  const std::vector < unsigned int > & oldPositions,
                                  std::vector < unsigned int > & positions);

  private:
    bool verbose_;
    float jetPtThreshold_;
    bool useHighestPtHiggs_;
};
#endif  // HbbCandidateFinderAlgo__H

