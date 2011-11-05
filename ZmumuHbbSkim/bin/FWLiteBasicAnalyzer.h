#ifndef NtpBaseAnalyzer_h
#define NtpBaseAnalyzer_h

/* 
 * Need to add gen p4 info
 */

#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/FWLite/interface/Handle.h"
#include "PhysicsTools/FWLite/interface/TFileService.h"
#include "FWCore/FWLite/interface/AutoLibraryLoader.h"
#include "FWCore/ParameterSet/interface/ProcessDesc.h"
#include "FWCore/PythonParameterSet/interface/PythonProcessDesc.h"

#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/deltaPhi.h"

//#include "Utilities/Parang/interface/Plotter.h"
//#include "Utilities/Parang/interface/Polybook.h"

#include "Math/Boost.h"
#include "Math/VectorUtil.h"

using namespace std;
using namespace edm;

typedef reco::Candidate::LorentzVector LorentzVector;
typedef reco::Candidate::Vector Vector;
typedef reco::Candidate::Point Point;
typedef unsigned int size;
typedef pair<double,size>  double_size;


double alphaT(const LorentzVector& p1, const LorentzVector& p2) {
   double subLeadPt  = p1.pt() < p2.pt() ? p1.pt() : p2.pt();
   double transMass2 = (p1.pt() + p2.pt()) * (p1.pt() + p2.pt()) 
                     - (p1 + p2).pt() * (p1 + p2).pt();
   return subLeadPt / sqrt(transMass2);
}

struct sortLeastToGreatest{
    bool operator()(const double_size& p1, const double_size& p2){ return (p1.first < p2.first); }
};

struct sortGreatestToLeast{
    bool operator()(const double_size& p1, const double_size& p2){ return (p1.first > p2.first); }
};

template<typename T>
void getHandle(fwlite::Handle<vector<T> >& handle, 
               const fwlite::Event& ev, 
               const char* iModuleLabel,
               const char* iProductInstanceLabel = 0,
               const char* iProcessLabel = 0)
{
    TString moduleLabel(iModuleLabel);
    handle.getByLabel(ev, iModuleLabel, iProductInstanceLabel, iProcessLabel);
}


class ObjectPair{

public:
    // Constructors
    ObjectPair(){};
    
    ObjectPair(const LorentzVector& _p1, size _id1,
               const LorentzVector& _p2, size _id2)
      : p1  (_p1),
        p2  (_p2),
        p12 (_p1+_p2),
        id1 (_id1),
        id2 (_id2)
    {
        if(p1.pt() < p2.pt() )
            cout << "WARNING: p1 pt: " << p1.pt() << " < p2 pt: " << p2.pt() << endl;
    };

    ~ObjectPair(){};
    
    // Member functions
    double deltaR()         const{ return reco::deltaR(p1, p2); };
    double deltaPhi()       const{ return reco::deltaPhi(p1.phi(), p2.phi()); };
    double deltaEta()       const{ return p1.eta() - p2.eta(); };
    double deltaPt()        const{ return fabs(p1.pt() - p2.pt()); };
    double sumPt()          const{ return p1.pt()+p2.pt(); };

    double mass()           const{ return p12.mass(); };
    double pt()             const{ return p12.pt(); };
    double eta()            const{ return p12.eta(); };
    double phi()            const{ return p12.phi(); };
        
    template<typename T>
    T getter(const fwlite::Handle<vector<T> >& _handle, unsigned int _which) const{
        assert(_which==1 || _which==2);
        unsigned int  index = _which==1 ? id1 : id2;

        return _handle->at(index);
    }
    
    double cosThetaStar()   const{
        Vector boost = p12.BoostToCM();
        ROOT::Math::Boost b( boost );
        LorentzVector p1InCM = b*p1;
        Vector zAxis(0.,0.,1.);
        
        return ROOT::Math::VectorUtil::CosTheta( zAxis, p1InCM.Vect() );
    }
    
    double ptStar()         const{
        Vector boost = p12.BoostToCM();
        ROOT::Math::Boost b( boost );
        LorentzVector p1InCM = b*p1;
        
        return p1InCM.pt();
    }
        
    
    // Public member data    
    LorentzVector   p1;
    LorentzVector   p2;
    LorentzVector   p12;
    unsigned int    id1;
    unsigned int    id2;    
    
}; // end clss ObjectPair

class MET{

public:
    // Constructors
    MET(){};
    MET(const LorentzVector& _p4, float _mEtSig,
        float _significance)
      : p4           (_p4),
        mEtSig       (_mEtSig),
        significance (_significance) {};

    ~MET(){};
    
    double pt()             const{ return p4.pt();  };
    double phi()            const{ return p4.phi(); };
    
    // Public member data
    LorentzVector   p4;
    float           mEtSig;
    float           significance;
    
}; // end class MET


class NtpBaseAnalyzer{

public:
    NtpBaseAnalyzer(const fwlite::Event& ev, const edm::ParameterSet& ana)
    {
        bool usePartonP4 ( ana.getUntrackedParameter<bool>("usePartonP4",false) );
        makeObjects(ev, usePartonP4);
    }

private:
    void makeObjects(const fwlite::Event& ev, bool usePartonP4) {
        fwlite::Handle<vector<LorentzVector> >  h_jet_genp4;
        getHandle(h_jet_genp4, ev, "JetEdmNtuple", "genp4");
        fwlite::Handle<vector<LorentzVector> >  h_mu_genp4;
        getHandle(h_mu_genp4, ev, "MuEdmNtuple", "genp4");

        fwlite::Handle<vector<LorentzVector> >  h_jet_p4;
        getHandle(h_jet_p4, ev, "JetEdmNtuple", "p4");
        fwlite::Handle<vector<LorentzVector> >  h_met_p4;
        getHandle(h_met_p4, ev, "MetEdmNtuple", "p4");
        fwlite::Handle<vector<LorentzVector> >  h_metType1_p4;
        getHandle(h_metType1_p4, ev, "MetType1EdmNtuple", "p4");
        fwlite::Handle<vector<LorentzVector> >  h_mu_p4;
        getHandle(h_mu_p4, ev, "MuEdmNtuple", "p4");
        
        fwlite::Handle<vector<Point> >  h_jet_vertex; 
        getHandle(h_jet_vertex, ev, "JetEdmNtuple", "vertex");
        fwlite::Handle<vector<Point> >  h_mu_vertex; 
        getHandle(h_mu_vertex, ev, "MuEdmNtuple", "vertex");
        
        fwlite::Handle<vector<float> >  h_mu_charge;
        getHandle(h_mu_charge, ev, "MuEdmNtuple", "charge");
        fwlite::Handle<vector<float> >  h_mu_glbPt;
        getHandle(h_mu_glbPt, ev, "MuEdmNtuple", "glbPt");
        fwlite::Handle<vector<float> >  h_mu_glbPtError;
        getHandle(h_mu_glbPtError, ev, "MuEdmNtuple", "glbPtError");
        fwlite::Handle<vector<float> >  h_mu_trkPt;
        getHandle(h_mu_trkPt, ev, "MuEdmNtuple", "trkPt");
        fwlite::Handle<vector<float> >  h_mu_trkPtError;
        getHandle(h_mu_trkPtError, ev, "MuEdmNtuple", "trkPtError");
        fwlite::Handle<vector<float> >  h_mu_trackIso;
        getHandle(h_mu_trackIso, ev, "MuEdmNtuple", "trackIso");
        fwlite::Handle<vector<float> >  h_mu_ecalIso;
        getHandle(h_mu_ecalIso, ev, "MuEdmNtuple", "ecalIso");
        fwlite::Handle<vector<float> >  h_mu_hcalIso;
        getHandle(h_mu_hcalIso, ev, "MuEdmNtuple", "hcalIso");
        
        fwlite::Handle<vector<float> >  h_jet_partonFlavour;
        getHandle(h_jet_partonFlavour, ev, "JetEdmNtuple", "partonFlavour");
        fwlite::Handle<vector<float> >  h_jet_csv;
        getHandle(h_jet_csv, ev, "JetEdmNtuple", "csv");
        fwlite::Handle<vector<float> >  h_jet_tche;
        getHandle(h_jet_tche, ev, "JetEdmNtuple", "tche");
        fwlite::Handle<vector<float> >  h_jet_tchp;
        getHandle(h_jet_tchp, ev, "JetEdmNtuple", "tchp");
        
        fwlite::Handle<vector<float> >  h_met_mEtSig;
        getHandle(h_met_mEtSig, ev, "MetEdmNtuple", "mEtSig");
        fwlite::Handle<vector<float> >  h_met_significance;
        getHandle(h_met_significance, ev, "MetEdmNtuple", "significance");
        fwlite::Handle<vector<float> >  h_met_sumEt;
        getHandle(h_met_sumEt, ev, "MetEdmNtuple", "sumEt");
                
        fwlite::Handle<vector<float> >  h_metType1_mEtSig;
        getHandle(h_metType1_mEtSig, ev, "MetType1EdmNtuple", "mEtSig");
        fwlite::Handle<vector<float> >  h_metType1_significance;
        getHandle(h_metType1_significance, ev, "MetType1EdmNtuple", "significance");
        fwlite::Handle<vector<float> >  h_metType1_sumEt;
        getHandle(h_metType1_sumEt, ev, "MetType1EdmNtuple", "sumEt");


        // MET
        if(h_met_p4->size() == 0 ){
            cout << "ERROR: No MET object!" << endl;
        }
        if(h_jet_p4->size() == 0 ){
            cout << "ERROR: No jet object!" << endl;
        }
        if(h_mu_p4->size() == 0 ){
            cout << "ERROR: No Muon object!" << endl;
        }
        
        
        const LorentzVector& metP4 = h_met_p4->at(0);
        float metSig = h_met_mEtSig->at(0);
        float metSignificance = h_met_significance->at(0);
        met = MET(metP4, metSig, metSignificance);
        
        const LorentzVector& metType1P4 = h_metType1_p4->at(0);
        metSig = h_metType1_mEtSig->at(0);
        metSignificance = h_metType1_significance->at(0);
        metType1 = MET(metType1P4, metSig, metSignificance);
        
        
        // jets
        size njets = h_jet_p4->size();
        for(size i=0; i< njets-1; ++i){
            const LorentzVector&  p1 = usePartonP4 ? h_jet_genp4->at(i) : h_jet_p4->at(i);
                        
            for(size j=i+1; j< njets; ++j){
                const LorentzVector&  p2 = usePartonP4 ? h_jet_genp4->at(j) : h_jet_p4->at(j);
                
                ObjectPair jetpair(p1, i, p2, j);                
                jetPairs.push_back( jetpair );   
            }
        }
        
        // muons
        size nmuons = h_mu_p4->size();
        for(size i=0; i< nmuons-1; ++i){
            const LorentzVector&  p1 = usePartonP4 ? h_mu_genp4->at(i) : h_mu_p4->at(i);
                        
            for(size j=i+1; j< nmuons; ++j){
                const LorentzVector&  p2 = usePartonP4 ? h_mu_genp4->at(j) : h_mu_p4->at(j);
                
                ObjectPair mupair(p1, i, p2, j);                
                muPairs.push_back( mupair );   
            }
        }

    }  // end makeObjects()
        

public:
    vector<ObjectPair> jetPairs;
    vector<ObjectPair> muPairs;
    MET met;
    MET metType1;
};


#endif

