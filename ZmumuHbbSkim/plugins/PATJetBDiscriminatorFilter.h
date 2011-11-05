#ifndef ZmumuHbbSkim_PATJetBDiscriminatorFilter_h
#define ZmumuHbbSkim_PATJetBDiscriminatorFilter_h
/** \class PATJetBDiscriminatorFilter
 *
 * Select jets with b-tagging discriminator > min cut
 * See https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookAnalysisStarterKitPatTutorial14May08?redirectedfrom=CMS.WorkBookAnalysisStarterKitPatTutorial14May08#lepton_jets_with_btagging_Cecili
 *
 * \author: Cecili Gerber
 * 
 */

#include "CommonTools/UtilAlgos/interface/ObjectCountFilter.h"
#include "CommonTools/UtilAlgos/interface/MinNumberSelector.h"

#include "DataFormats/PatCandidates/interface/Jet.h"

struct BDiscriminatorFilter {
  BDiscriminatorFilter( std::string disc, double discCut ) 
  : disc_(disc), discCut_(discCut) {}

  template<typename T>
  bool operator()( const T & t ) const { return t.bDiscriminator(disc_) > discCut_; }
  
private:
  std::string   disc_;
  double        discCut_;
};

namespace reco {
  namespace modules {
  
    template<>
      struct ParameterAdapter<BDiscriminatorFilter>{
      static BDiscriminatorFilter make( const edm::ParameterSet & cfg ) {
        return BDiscriminatorFilter( cfg.getParameter<std::string>( "disc" ),
                                     cfg.getParameter<double>     ( "discCut" ) );
      }
    };
    
  }
}

namespace pat {

  typedef ObjectCountFilter<
            std::vector<Jet>,
            BDiscriminatorFilter,
            MinNumberSelector
          >::type PATJetBDiscriminatorFilter;

}

#endif

