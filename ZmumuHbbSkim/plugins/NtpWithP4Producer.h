#ifndef ZmumuHbbSkim_NtpWithP4Producer_h
#define ZmumuHbbSkim_NtpWithP4Producer_h
/** \class NtpProducer
 *
 * Creates histograms defined in config file 
 *
 * \author: Luca Lista, INFN
 * 
 * Template parameters:
 * - C : Concrete candidate collection type
 *
 */
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "CommonTools/Utils/interface/StringObjectFunction.h"

template<typename C>
class NtpWithP4Producer : public edm::EDProducer {
public:
  /// constructor from parameter set
  NtpWithP4Producer( const edm::ParameterSet& );
  /// destructor
  ~NtpWithP4Producer();
  
protected:
  /// process an event
  virtual void produce( edm::Event&, const edm::EventSetup& );

private:
  /// label of the collection to be read in
  edm::InputTag src_;
  /// variable tags
  std::vector<std::pair<std::string, StringObjectFunction<typename C::value_type> > > tags_;
  std::vector<std::pair<std::string, std::string> > p3_tags_;
  std::vector<std::pair<std::string, std::string> > p4_tags_;
  bool lazyParser_;
  std::string prefix_;
  bool eventInfo_;
};

template<typename C>
NtpWithP4Producer<C>::NtpWithP4Producer( const edm::ParameterSet& par ) : 
  src_       ( par.template getParameter<edm::InputTag>( "src" ) ),
  lazyParser_( par.template getUntrackedParameter<bool>( "lazyParser", false ) ),
  prefix_    ( par.template getUntrackedParameter<std::string>( "prefix","" ) ),
  eventInfo_ ( par.template getUntrackedParameter<bool>( "eventInfo",true ) )
{
  std::vector<edm::ParameterSet> variables = 
      par.template getParameter<std::vector<edm::ParameterSet> >("variables");
  std::vector<edm::ParameterSet>::const_iterator 
      q = variables.begin(), q_end = variables.end();
  
  std::vector<edm::ParameterSet> p4_variables = 
      par.template getParameter<std::vector<edm::ParameterSet> >("p4_variables");
  std::vector<edm::ParameterSet>::const_iterator 
      q4 = p4_variables.begin(), q4_end = p4_variables.end();
      
  std::vector<edm::ParameterSet> p3_variables = 
      par.template getParameter<std::vector<edm::ParameterSet> >("p3_variables");
  std::vector<edm::ParameterSet>::const_iterator 
      q3 = p3_variables.begin(), q3_end = p3_variables.end();
  
  if(eventInfo_){
    produces<unsigned int>( prefix_ + "EventNumber" ).setBranchAlias( prefix_ + "EventNumber" );
    produces<unsigned int>( prefix_ + "RunNumber"   ).setBranchAlias( prefix_ + "RunNumber" );
    produces<unsigned int>( prefix_ + "LumiBlock"   ).setBranchAlias( prefix_ + "Lumiblock" );
  }
  
  for(; q!=q_end; ++q) {
    std::string tag = prefix_ + q->getUntrackedParameter<std::string>("tag");
    StringObjectFunction<typename C::value_type> quantity(q->getUntrackedParameter<std::string>("quantity"), lazyParser_);
    tags_.push_back(std::make_pair(tag, quantity));
    produces<std::vector<float> >(tag).setBranchAlias(tag); 
  }
  
  for(; q4!=q4_end; ++q4) {
    std::string tag = prefix_ + q4->getUntrackedParameter<std::string>("tag");
    std::string quantity = q4->getUntrackedParameter<std::string>("quantity");
    p4_tags_.push_back(std::make_pair(tag, quantity));
    produces<std::vector<math::XYZTLorentzVector> >(tag).setBranchAlias(tag); 
  }
  
  for(; q3!=q3_end; ++q3) {
    std::string tag = prefix_ + q3->getUntrackedParameter<std::string>("tag");
    std::string quantity = q3->getUntrackedParameter<std::string>("quantity");
    p3_tags_.push_back(std::make_pair(tag, quantity));
    produces<std::vector<math::XYZPoint> >(tag).setBranchAlias(tag); 
  }
}

template<typename C>
NtpWithP4Producer<C>::~NtpWithP4Producer() {
}

template<typename C>
void NtpWithP4Producer<C>::produce( edm::Event& iEvent, const edm::EventSetup& ) {
  edm::Handle<C> coll;
  iEvent.getByLabel(src_, coll);
  
  if(eventInfo_){   
    std::auto_ptr<unsigned int> event( new unsigned int );
    std::auto_ptr<unsigned int> run  ( new unsigned int );
    std::auto_ptr<unsigned int> lumi ( new unsigned int );
    *event = iEvent.id().event();
    *run   = iEvent.id().run();
    *lumi  = iEvent.luminosityBlock();
    iEvent.put( event, prefix_ + "EventNumber" );
    iEvent.put( run  , prefix_ + "RunNumber" );
    iEvent.put( lumi , prefix_ + "LumiBlock" );
  }
  
   
  typename std::vector<std::pair<std::string, StringObjectFunction<typename C::value_type> > >::const_iterator 
      q = tags_.begin(), q_end = tags_.end();
  for(; q!=q_end; ++q) {
    std::auto_ptr<std::vector<float> > x(new std::vector<float>);
    x->reserve(coll->size());
    for (typename C::const_iterator elem=coll->begin(); elem!=coll->end(); ++elem ) {
      x->push_back(q->second(*elem));
    }
    iEvent.put(x, q->first);
  }
  

  typename std::vector<std::pair<std::string, std::string> >::const_iterator 
      q4 = p4_tags_.begin(), q4_end = p4_tags_.end();
  for(; q4!=q4_end; ++q4) {
    std::auto_ptr<std::vector<math::XYZTLorentzVector> > x(new std::vector<math::XYZTLorentzVector>);
    x->reserve(coll->size());
    for (typename C::const_iterator elem=coll->begin(); elem!=coll->end(); ++elem ) {
      std::string quantity = q4->second;
      StringObjectFunction<typename C::value_type> pxSOF(quantity+".Px", true);
      StringObjectFunction<typename C::value_type> pySOF(quantity+".Py", true);
      StringObjectFunction<typename C::value_type> pzSOF(quantity+".Pz", true);
      StringObjectFunction<typename C::value_type> eSOF (quantity+".E" , true);

      double px = pxSOF(*elem);
      double py = pySOF(*elem);
      double pz = pzSOF(*elem);
      double e  = eSOF (*elem);
      x->push_back( math::XYZTLorentzVector(px,py,pz,e) );
    
    }
    iEvent.put(x, q4->first);
  }
  
  
  typename std::vector<std::pair<std::string, std::string> >::const_iterator 
      q3 = p3_tags_.begin(), q3_end = p3_tags_.end();
  for(; q3!=q3_end; ++q3) {
    std::auto_ptr<std::vector<math::XYZPoint> > x(new std::vector<math::XYZPoint>);
    x->reserve(coll->size());
    for (typename C::const_iterator elem=coll->begin(); elem!=coll->end(); ++elem ) {
      std::string quantity = q3->second;
      StringObjectFunction<typename C::value_type> pxSOF(quantity+".X", true);
      StringObjectFunction<typename C::value_type> pySOF(quantity+".Y", true);
      StringObjectFunction<typename C::value_type> pzSOF(quantity+".Z", true);

      double px = pxSOF(*elem);
      double py = pySOF(*elem);
      double pz = pzSOF(*elem);
      x->push_back( math::XYZPoint(px,py,pz) );
    
    }
    iEvent.put(x, q3->first);
  }

}

#endif
