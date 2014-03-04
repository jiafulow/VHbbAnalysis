#include "VHbbAnalysis/VHbbDataFormats/interface/VHbbEvent.h"
#include "VHbbAnalysis/VHbbDataFormats/interface/VHbbEventAuxInfo.h"
#include "VHbbAnalysis/VHbbDataFormats/interface/VHbbCandidate.h"
#include "DataFormats/Common/interface/Wrapper.h"

namespace { 
  VHbbEvent pippo1;
  VHbbEventAuxInfo pippo2;
  edm::Wrapper<VHbbEvent> pippo3;
  edm::Wrapper<VHbbEventAuxInfo> pippo4;
  VHbbCandidate pippo5;
  std::vector<VHbbCandidate> pippo6;
  edm::Wrapper<std::vector<VHbbCandidate> > pippo7;
    
}
