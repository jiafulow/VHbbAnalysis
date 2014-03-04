#ifndef HbbCandidateFinderAlgo__H
#define HbbCandidateFinderAlgo__H

#include "VHbbAnalysis/VHbbDataFormats/interface/VHbbCandidate.h"
#include "VHbbAnalysis/VHbbDataFormats/interface/VHbbCandidateTools.h"
#include "VHbbAnalysis/VHbbDataFormats/interface/VHbbEvent.h"
#include "VHbbAnalysis/VHbbDataFormats/interface/VHbbEventAuxInfo.h"


class HbbCandidateFinderAlgo {
  public:
    explicit HbbCandidateFinderAlgo(bool verbose, double jetPt, bool useH)
      : verbose_(verbose), jetPtThreshold_(jetPt), useHighestPtHiggs_(useH) {}

    static bool jetID(const VHbbEvent::SimpleJet & j);

    VHbbCandidate changeHiggs(bool useHighestPtHiggs, const VHbbCandidate & old);

    void run(const VHbbEvent * event,
             std::vector < VHbbCandidate > & candidates,
             const VHbbEventAuxInfo & aux);

    bool findDiJets(const std::vector < VHbbEvent::SimpleJet > & jetsin,
                    VHbbEvent::SimpleJet & j1, VHbbEvent::SimpleJet & j2,
                    std::vector < VHbbEvent::SimpleJet > & addJets,
                    size_t * indices=0);

    bool findDiJetsHighestPt(const std::vector < VHbbEvent::SimpleJet > & jetsin,
                             VHbbEvent::SimpleJet & j1, VHbbEvent::SimpleJet & j2,
                             std::vector < VHbbEvent::SimpleJet > & addJets,
                             size_t * indices=0);

    bool findFatJet(const std::vector < VHbbEvent::HardJet > & jetsin,
                    const std::vector < VHbbEvent::SimpleJet > & subjetsin,
                    const std::vector < VHbbEvent::SimpleJet > & filterjetsin,
                    VHbbEvent::HardJet & fatj1,
                    std::vector < VHbbEvent::SimpleJet > & subJetsout,
                    const std::vector < VHbbEvent::SimpleJet > & ak5jetsin,
                    std::vector < VHbbEvent::SimpleJet > & addJetsFat,
                    const std::vector < VHbbEvent::MuonInfo > & muons,
                    const std::vector < VHbbEvent::ElectronInfo > & electrons);

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

    void findTaus(const std::vector < VHbbEvent::TauInfo > & taus,
                  std::vector < VHbbEvent::TauInfo > & out,
                  std::vector < unsigned int > & positions);

    void removeTauOverlapWithJets(const std::vector < VHbbEvent::TauInfo > & taus,
                                  const std::vector < VHbbEvent::SimpleJet > & jets,
                                  std::vector < VHbbEvent::TauInfo > & out,
                                  const std::vector < unsigned int > & oldPositions,
                                  std::vector < unsigned int > & positions);

  private:
    bool verbose_;
    double jetPtThreshold_;
    bool useHighestPtHiggs_;
};
#endif  // HbbCandidateFinderAlgo__H

