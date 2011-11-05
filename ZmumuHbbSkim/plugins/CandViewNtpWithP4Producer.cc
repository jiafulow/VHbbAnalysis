/* \class CandViewNtpProducer
 * 
 * Configurable Candidate ntuple creator
 *
 * \author: Luca Lista, INFN
 *
 */
#include "FWCore/Framework/interface/MakerMacros.h"
//#include "CommonTools/UtilAlgos/interface/NtpProducer.h"
#include "DataFormats/Candidate/interface/Candidate.h"
//#include "DataFormats/Common/interface/View.h"

#include "VHbbAnalysis/ZmumuHbbSkim/plugins/NtpWithP4Producer.h"

typedef NtpWithP4Producer<reco::CandidateView> CandViewNtpWithP4Producer;

DEFINE_FWK_MODULE( CandViewNtpWithP4Producer );

