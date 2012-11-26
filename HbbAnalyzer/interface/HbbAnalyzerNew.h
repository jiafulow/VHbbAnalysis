#include <memory>
#include <iostream>
//#include <cmath>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
//#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/LorentzVector.h"
//#include "DataFormats/Math/interface/Vector3D.h"
//#include "Math/GenVector/PxPyPzM4D.h"

//#include "DataFormats/BeamSpot/interface/BeamSpot.h"
//#include "DataFormats/Candidate/interface/Candidate.h"
//#include "DataFormats/Candidate/interface/Particle.h"
//#include "DataFormats/Common/interface/View.h"
//#include "DataFormats/GeometryVector/interface/Phi.h"
//#include "DataFormats/GeometryVector/interface/VectorUtil.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
//#include "DataFormats/METReco/interface/PFMET.h"
//#include "DataFormats/PatCandidates/interface/PATObject.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
//#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/MET.h"
//#include "DataFormats/TrackReco/interface/TrackFwd.h"

#include "VHbbAnalysis/VHbbDataFormats/interface/VHbbEvent.h"
#include "VHbbAnalysis/VHbbDataFormats/interface/VHbbEventAuxInfo.h"

#include "TH1.h"
#include "TMath.h"
#include "TTree.h"
#include "TString.h"
#include "TVector2.h"
#include "TArrayD.h"
#include "TLorentzVector.h"

using namespace std;


//
// class decleration
//

class BtagPerformance;

class GenericMVAJetTagComputer;

class JetCorrectionUncertainty;

struct BTagSFContainer {
    const BtagPerformance *BTAGSF_CSVL;
    const BtagPerformance *BTAGSF_CSVM;
    const BtagPerformance *BTAGSF_CSVT;
    const BtagPerformance *MISTAGSF_CSVL;
    const BtagPerformance *MISTAGSF_CSVM;
    const BtagPerformance *MISTAGSF_CSVT;
};

class HbbAnalyzerNew: public edm::EDProducer {

  public:
    explicit HbbAnalyzerNew(const edm::ParameterSet &);
     ~HbbAnalyzerNew();

  protected:
    TVector2 getTvect(const pat::Jet * patJet);

    TLorentzVector getChargedTracksMomentum(const pat::Jet * patJet);

  private:
    virtual void beginJob();
    virtual void produce(edm::Event &, const edm::EventSetup &);
    virtual void endJob();
    virtual void fillMuBlock(edm::View < pat::Muon >::const_iterator mu,
                             int muInfo[15]);
    virtual void fillScaleFactors(VHbbEvent::SimpleJet &, BTagSFContainer);

    edm::InputTag eleLabel_;
    edm::InputTag muoLabel_;
    edm::InputTag elenoCutsLabel_;
    edm::InputTag muonoCutsLabel_;
    edm::InputTag jetLabel_;
    edm::InputTag subjetLabel_;
    edm::InputTag filterjetLabel_;
    edm::InputTag simplejet1Label_;
    edm::InputTag simplejet2Label_;
    edm::InputTag simplejet3Label_;
    edm::InputTag simplejet4Label_;
    edm::InputTag tauLabel_;
    edm::InputTag metLabel_;
    edm::InputTag phoLabel_;
    edm::InputTag hltResults_;

    bool runOnMC_;
    double lep_ptCutForBjets_;
    
    //TMatrixD *pointerPt;
    TMatrixD *pointerEta;
    TMatrixD *pointerPhi;

    // The computer for the CSV variables
    const GenericMVAJetTagComputer *computer;

    bool verbose_;
  
  protected:
    void fillSimpleJet(VHbbEvent::SimpleJet & sj,
                       edm::View < pat::Jet >::const_iterator iter);
    void setJecUnc(VHbbEvent::SimpleJet & sj,
                   JetCorrectionUncertainty * jecunc);
    float metSignificance(const reco::MET * met);
};

