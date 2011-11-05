#ifndef HSkim_ZmumuHbbSkim_PATMCMatchFilter_h
#define HSkim_ZmumuHbbSkim_PATMCMatchFilter_h

#include <memory>
#include <vector>

#include "FWCore/Framework/interface/EDFilter.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

//#include "DataFormats/Common/interface/RefVector.h"

//#include "CommonTools/UtilAlgos/interface/StringCutObjectSelector.h"
//#include "CommonTools/UtilAlgos/interface/SingleObjectSelector.h"
//#include "CommonTools/UtilAlgos/interface/ObjectSelector.h"
//#include "CommonTools/UtilAlgos/interface/SingleElementCollectionSelector.h"

#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
//#include "DataFormats/PatCandidates/interface/Tau.h"
//#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
//#include "DataFormats/PatCandidates/interface/MET.h"
//#include "DataFormats/PatCandidates/interface/PFParticle.h"
//#include "DataFormats/PatCandidates/interface/GenericParticle.h"
//#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
//#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"

template<typename ObjectType>
class PATObjectMCMatchFilter : public edm::EDFilter{

public:
    explicit PATObjectMCMatchFilter(const edm::ParameterSet& iConfig)
      : src_(iConfig.getParameter<edm::InputTag>("src") )
    {
      
    }

    ~PATObjectMCMatchFilter() {}


    bool filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
    {
        edm::Handle<std::vector<ObjectType> > cands;
        iEvent.getByLabel(src_,cands);
        
        bool foundAllMCMatch = true;
        
        for(unsigned int i=0; i< cands->size(); ++i){
//            ObjectType cand = static_cast<ObjectType>((*cands)[i]);
        
            if(typeid(ObjectType) == typeid(pat::Jet) ){
                edm::Handle<std::vector<pat::Jet> > source;
                iEvent.getByLabel(src_,source);
                const pat::Jet& jet = (*source)[i];
                if(!jet.genJet() )          foundAllMCMatch = false;
            } else if(typeid(ObjectType) == typeid(pat::Muon) ){
                edm::Handle<std::vector<pat::Muon> > source;
                iEvent.getByLabel(src_,source);
                const pat::Muon& muon = (*source)[i];
                if(!muon.genLepton() )      foundAllMCMatch = false;
            } else if(typeid(ObjectType) == typeid(pat::Electron) ){
                edm::Handle<std::vector<pat::Electron> > source;
                iEvent.getByLabel(src_,source);
                const pat::Electron& electron = (*source)[i];
                if(!electron.genLepton() )  foundAllMCMatch = false;
            }
            
//            else if(  typeid(ObjectType) == typeid(pat::Muon)
//                     || typeid(ObjectType) == typeid(pat::Electron) ){
//                const ObjectType& lepton = (*cands)[i];
//                if(!lepton.genLepton() )    foundAllMCMatch = false;
//            }

        }
        
        return foundAllMCMatch;
    }

private:
    edm::InputTag src_;
    
}; // end class PATObjectMCMatchFilter

typedef PATObjectMCMatchFilter<pat::Jet> PATJetMCMatchFilter;
typedef PATObjectMCMatchFilter<pat::Muon> PATMuonMCMatchFilter;
typedef PATObjectMCMatchFilter<pat::Electron> PATElectronMCMatchFilter;


#endif
