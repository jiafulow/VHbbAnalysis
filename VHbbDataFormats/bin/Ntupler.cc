#include <sstream>
#include <string>

#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TVector2.h"
#include "TVector3.h"
#include "TLorentzVector.h"
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TSystem.h"
#include "TStopwatch.h"

#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/FWLite/interface/Handle.h"
#include "DataFormats/FWLite/interface/LuminosityBlock.h"
//#include "DataFormats/FWLite/interface/Run.h"
#include "FWCore/FWLite/interface/AutoLibraryLoader.h"
#include "FWCore/ParameterSet/interface/ProcessDesc.h"
#include "FWCore/PythonParameterSet/interface/PythonProcessDesc.h"
//#include "PhysicsTools/FWLite/interface/TFileService.h"

//#include "DataFormats/MuonReco/interface/Muon.h"
//#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/Math/interface/deltaPhi.h"
#include "DataFormats/Math/interface/deltaR.h"

//#include "DataFormats/Luminosity/interface/LumiSummary.h"
#include "PhysicsTools/Utilities/interface/LumiReWeighting.h"
//#include "PhysicsTools/Utilities/interface/Lumi3DReWeighting.h"

#include "VHbbAnalysis/VHbbDataFormats/interface/HbbCandidateFinderAlgo.h"
#include "VHbbAnalysis/VHbbDataFormats/interface/VHbbEvent.h"
#include "VHbbAnalysis/VHbbDataFormats/interface/VHbbEventAuxInfo.h"
#include "VHbbAnalysis/VHbbDataFormats/interface/VHbbCandidate.h"
#include "VHbbAnalysis/VHbbDataFormats/interface/BTagWeight.h"
#include "VHbbAnalysis/VHbbDataFormats/interface/BTagReshaping.h"
#include "VHbbAnalysis/VHbbDataFormats/interface/TriggerReaderNew.h"
//#include "VHbbAnalysis/VHbbDataFormats/interface/TriggerReader.h"
#include "VHbbAnalysis/VHbbDataFormats/interface/TriggerWeightNew.h"
//#include "VHbbAnalysis/VHbbDataFormats/interface/TriggerWeight.h"
#include "VHbbAnalysis/VHbbDataFormats/interface/TopMassReco.h"
#include "VHbbAnalysis/VHbbDataFormats/interface/JECFWLite.h"

// LHE Info
#include "SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h"

// IVF
#include "RecoBTag/SecondaryVertex/interface/SecondaryVertex.h"
//#include "DataFormats/GeometryCommonDetAlgo/interface/Measurement1D.h"
#include "DataFormats/GeometryVector/interface/GlobalVector.h"
//#include "DataFormats/GeometryVector/interface/VectorUtil.h"
//#include "DataFormats/GeometrySurface/interface/Surface.h"
//#include "Math/SMatrix.h"

// ZSV
#include "ZSV/BAnalysis/interface/SimBHadron.h"

// Move class definition to Ntupler.h ?
//#include "VHbbAnalysis/VHbbDataFormats/interface/Ntupler.h"


#define MAXJ 130
#define MAXL 110
#define MAXB 110
#define MAXT 160


// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------
const GlobalVector flightDirection(const TVector3 pv, const reco::Vertex & sv) {
    GlobalVector fdir(sv.position().X() - pv.X(),
                      sv.position().Y() - pv.Y(), 
                      sv.position().Z() - pv.Z());
    return fdir;
}

bool jsonContainsEvent(const std::vector < edm::LuminosityBlockRange > &jsonVec, 
                       const edm::EventBase & event) {
    // if the jsonVec is empty, then no JSON file was provided so all
    // events should pass
    if (jsonVec.empty()) {
        return true;
    }
    bool(*funcPtr) (edm::LuminosityBlockRange const &,
                    edm::LuminosityBlockID const &) = &edm::contains;
    edm::LuminosityBlockID lumiID(event.id().run(), event.id().luminosityBlock());
    std::vector < edm::LuminosityBlockRange >::const_iterator iter =
        std::find_if(jsonVec.begin(), jsonVec.end(), boost::bind(funcPtr, _1, lumiID));
    return jsonVec.end() != iter;
}

// Note: This should be updated to match 
//    https://twiki.cern.ch/twiki/bin/view/CMS/JetResolution
float resolutionBias(float eta) {
    if (eta < 0)  // for protection
        return resolutionBias(-eta);
    if (eta < 1.1)
        return 0.05;
    if (eta < 2.5)
        return 0.10;
    if (eta < 5)
        return 0.30;
    return 0;  // Nominal
}


// -----------------------------------------------------------------------------
// Classes
// -----------------------------------------------------------------------------
struct CompareDeltaR {
    CompareDeltaR(TLorentzVector p4dir_):p4dir(p4dir_) {}
    bool operator() (const VHbbEvent::SimpleJet & j1,
                     const VHbbEvent::SimpleJet & j2) const {
        return j1.p4.DeltaR(p4dir) > j2.p4.DeltaR(p4dir);
    }
    TLorentzVector p4dir;
};

struct CompareJetPt {
    bool operator() (const VHbbEvent::SimpleJet & j1,
                     const VHbbEvent::SimpleJet & j2) const {
        return j1.p4.Pt() > j2.p4.Pt();
    }
};

typedef struct {
    int HiggsFlag;
    float mass;
    float pt;
    float eta;
    float phi;
    float dR;
    float dPhi;
    float dEta;
} HiggsInfo;

typedef struct {
    int FatHiggsFlag;
    float mass;
    float pt;
    float eta;
    float phi;
    float filteredmass;
    float filteredpt;
    float filteredeta;
    float filteredphi;
    //float dR;
    //float dPhi;
    //float dEta;
} FatHiggsInfo;

typedef struct {
    float mass;  //MT in case of W
    float pt;
    float eta;
    float phi;
} VInfo;

typedef struct {
    int run;
    int lumi;
    int event;
    int json;
} EventInfo;

typedef struct {
    float et;
    float sumet;
    float sig;
    float phi;
} METInfo;

typedef struct {
    float et[12];
    float phi[12];
    float sumet[12];
    //< ElectronEnUp/Down, JetEn, JetRes, MuonEn, TauEn, UnclusteredEn [0-11]
    template < class Input > void set(const Input & i, int j) {
        et[j] = i.p4.Pt();
        phi[j] = i.p4.Phi();
        sumet[j] = i.sumEt;
    }
} METUncInfo;

typedef struct {
    float mht;
    float ht;
    float sig;
    float phi;
} MHTInfo;

typedef struct {
    void reset() {
        for (int i = 0; i < MAXL; i++)
        {
            mass[i]=-99; pt[i]=-99; eta[i]=-99; phi[i]=-99; aodCombRelIso[i]=-99; pfCombRelIso[i]=-99; photonIso[i]=-99; neutralHadIso[i]=-99; chargedHadIso[i]=-99; chargedPUIso[i]=-99; particleIso[i]=-99; dxy[i]=-99; dz[i]=-99; type[i]=-99;  genPt[i]=-99; genEta[i]=-99; genPhi[i]=-99;  
            id80[i]=-99; id95[i]=-99; vbtf[i]=-99; id80NoIso[i]=-99;
            charge[i]=-99;wp70[i]=-99; wp80[i]=-99;wp85[i]=-99;wp90[i]=-99;wp95[i]=-99;wpHWW[i]=-99;
            pfCorrIso[i]=-99.; id2012tight[i]=-99; idMVAnotrig[i]=-99; idMVAtrig[i]=-99;innerHits[i]=-99;
        }
    }

    template < class Input > void set(const Input & i, int j, int t,
                                      const VHbbEventAuxInfo & aux) {
        type[j] = t;
        pt[j] = i.p4.Pt();
        mass[j] = i.p4.M();
        eta[j] = i.p4.Eta();
        phi[j] = i.p4.Phi();
        aodCombRelIso[j] = (i.hIso + i.eIso + i.tIso) / i.p4.Pt();
        pfCombRelIso[j] = (i.pfChaIso + i.pfPhoIso + i.pfNeuIso) / i.p4.Pt();
        photonIso[j] = i.pfPhoIso;
        neutralHadIso[j] = i.pfNeuIso;
        chargedHadIso[j] = i.pfChaIso;
        chargedPUIso[j] = i.pfChaPUIso;
        charge[j] = i.charge;
        if (i.mcFourMomentum.Pt() > 0) {
            genPt[j] = i.mcFourMomentum.Pt();
            genEta[j] = i.mcFourMomentum.Eta();
            genPhi[j] = i.mcFourMomentum.Phi();
        }
        setSpecific(i, j, aux);
    }
    template < class Input > void setSpecific(const Input & i, int j,
                                              const VHbbEventAuxInfo & aux);


    float mass[MAXL];  //MT in case of W
    float pt[MAXL];
    float eta[MAXL];
    float phi[MAXL];
    float aodCombRelIso[MAXL];
    float pfCombRelIso[MAXL];
    float photonIso[MAXL];
    float neutralHadIso[MAXL];
    float chargedHadIso[MAXL];
    float chargedPUIso[MAXL];
    float particleIso[MAXL];
    float genPt[MAXL];
    float genEta[MAXL];
    float genPhi[MAXL];
    float dxy[MAXL];
    float dz[MAXL];
    int type[MAXL];
    float id80[MAXL];
    float id95[MAXL];
    float vbtf[MAXL];
    float id80NoIso[MAXL];
    float charge[MAXL];
    float pfCorrIso[MAXL];
    float id2012tight[MAXL];
    float idMVAnotrig[MAXL];
    float idMVAtrig[MAXL];
    float idMVApresel[MAXL];
    float wp70[MAXL];
    float wp80[MAXL];
    float wp85[MAXL];
    float wp90[MAXL];
    float wp95[MAXL];
    float wpHWW[MAXL];
    float innerHits[MAXL];
    float photonIsoDoubleCount[MAXL];
} LeptonInfo;

template <> void LeptonInfo::setSpecific < VHbbEvent::ElectronInfo >
    (const VHbbEvent::ElectronInfo & i, int j, const VHbbEventAuxInfo & aux) {
    photonIsoDoubleCount[j] = i.pfPhoIsoDoubleCounted;
    id80[j] = i.id80;
    id95[j] = i.id95;
    id80NoIso[j] = (i.innerHits ==0 && !(fabs(i.convDist)<0.02 && fabs(i.convDcot)<0.02) &&
((i.isEB && i.sihih<0.01 && fabs(i.Dphi)<0.06 && fabs(i.Deta)<0.004) || (i.isEE && i.sihih<0.03 && fabs(i.Dphi)<0.03  && fabs(i.Deta)<0.007)));
    innerHits[j] = i.innerHits;
    float mincor = 0.0;
    float minrho = 0.0;
    float rho = std::max(aux.puInfo.rho25Iso, minrho);
    float eta = i.p4.Eta();
    float areagamma = 0.5;
    float areaNH = 0.5;
    float areaComb = 0.5;

    if(fabs(eta) <= 1.0 ) {areagamma=0.14; areaNH=0.044; areaComb=0.18;}
 	if(fabs(eta) > 1.0 &&  fabs(eta) <= 1.479 ) {areagamma=0.13; areaNH=0.065; areaComb=0.20;}
    if(fabs(eta) > 1.479 &&  fabs(eta) <= 2.0 ) {areagamma=0.079; areaNH=0.068; areaComb=0.15;}
    if(fabs(eta) > 2.0 &&  fabs(eta) <= 2.2 ) {areagamma=0.13; areaNH=0.057; areaComb=0.19;}
    if(fabs(eta) > 2.2 &&  fabs(eta) <= 2.3 ) {areagamma=0.15; areaNH=0.058; areaComb=0.21;}
    if(fabs(eta) > 2.3 &&  fabs(eta) <= 2.4 ) {areagamma=0.16; areaNH=0.061; areaComb=0.22;}
    if(fabs(eta) > 2.4  ) {areagamma=0.18; areaNH=0.11; areaComb=0.29;}
/*
    if(fabs(eta) <= 1.0 ) {areagamma=0.081; areaNH=0.024; areaComb=0.10;}
    if(fabs(eta) > 1.0 &&  fabs(eta) <= 1.479 ) {areagamma=0.084; areaNH=0.037; areaComb=0.12;}
    if(fabs(eta) > 1.479 &&  fabs(eta) <= 2.0 ) {areagamma=0.048; areaNH=0.037; areaComb=0.085;}
    if(fabs(eta) > 2.0 &&  fabs(eta) <= 2.2 ) {areagamma=0.089; areaNH=0.023; areaComb=0.11;}
    if(fabs(eta) > 2.2 &&  fabs(eta) <= 2.3 ) {areagamma=0.092; areaNH=0.023; areaComb=0.12;}
    if(fabs(eta) > 2.3 &&  fabs(eta) <= 2.4 ) {areagamma=0.097; areaNH=0.021; areaComb=0.12;}
    if(fabs(eta) > 2.4  ) {areagamma=0.11; areaNH=0.021; areaComb=0.13;}
*/

    float pho = i.pfPhoIso;
    if (i.innerHits > 0) {
        pho -= i.pfPhoIsoDoubleCounted;
    }

    pfCorrIso[j] = (i.pfChaIso+ std::max(pho-rho*areagamma,mincor )+std::max(i.pfNeuIso-rho*areaNH,mincor))/i.p4.Pt();

    id2012tight[j] = fabs(i.dxy) < 0.02  &&fabs(i.dz) < 0.1  &&(
        (i.isEE  &&fabs(i.Deta) < 0.005 &&fabs(i.Dphi) < 0.02 &&i.sihih < 0.03  &&i.HoE < 0.10  &&fabs(i.fMVAVar_IoEmIoP) < 0.05
        ) ||
        (i.isEB  &&fabs(i.Deta) < 0.004 &&fabs(i.Dphi) < 0.03 &&i.sihih < 0.01  &&i.HoE < 0.12  &&fabs(i.fMVAVar_IoEmIoP) < 0.05
         ));

    bool mvaPreSel = (
         (i.isEE  &&
        //fabs(electrons[it].Deta) < 0.009 &&
        //fabs(electrons[it].Dphi) < 0.1 &&
        i.sihih < 0.03  &&
        i.HoE < 0.10  &&
        i.innerHits == 0  &&
        i.tIso/i.p4.Pt() < 0.2 && 
        i.eIso/i.p4.Pt() < 0.2 &&
        i.hIso/i.p4.Pt() < 0.2) 
          || 
        (i.isEB &&
        //fabs(electrons[it].Deta) < 0.007 &&
        //fabs(electrons[it].Dphi) < 0.015 &&
        i.sihih < 0.01  &&
        i.HoE < 0.12  &&
        i.innerHits == 0  &&
        i.tIso/i.p4.Pt() < 0.2 && 
        i.eIso/i.p4.Pt() < 0.2 &&
        i.hIso/i.p4.Pt() < 0.2) 
         );

    float id = i.mvaOutTrig;
    if (!mvaPreSel)
        id = 0;
    float iso = pfCorrIso[j];
    wp70[j]=((fabs(eta) < 0.8 && id>0.977 && iso < 0.093) ||  (fabs(eta) >= 0.8 && fabs(eta) < 1.479 && id>0.956 && iso < 0.095) || (fabs(eta) >= 1.479 && fabs(eta) < 2.5 && id>0.966 && iso < 0.171));
    wp80[j]=((fabs(eta) < 0.8 && id>0.913 && iso < 0.105) ||  (fabs(eta) >= 0.8 && fabs(eta) < 1.479 && id>0.964 && iso < 0.178) || (fabs(eta) >= 1.479 && fabs(eta) < 2.5 && id>0.899 && iso < 0.150));
    wp85[j]=((fabs(eta) < 0.8 && id>0.929 && iso < 0.135) ||  (fabs(eta) >= 0.8 && fabs(eta) < 1.479 && id>0.931 && iso < 0.159) || (fabs(eta) >= 1.479 && fabs(eta) < 2.5 && id>0.805 && iso < 0.155));
    wp90[j]=((fabs(eta) < 0.8 && id>0.877 && iso < 0.177) ||  (fabs(eta) >= 0.8 && fabs(eta) < 1.479 && id>0.794 && iso < 0.180) || (fabs(eta) >= 1.479 && fabs(eta) < 2.5 && id>0.846 && iso < 0.244));
    wp95[j]=((fabs(eta) < 0.8 && id>0.858 && iso < 0.253) ||  (fabs(eta) >= 0.8 && fabs(eta) < 1.479 && id>0.425 && iso < 0.225) || (fabs(eta) >= 1.479 && fabs(eta) < 2.5 && id>0.759 && iso < 0.308));
    wpHWW[j]=((fabs(eta) < 0.8 && id>0.94 && iso < 0.15) ||  (fabs(eta) >= 0.8 && fabs(eta) < 1.479 && id>0.85 && iso < 0.15) || (fabs(eta) >= 1.479 && fabs(eta) < 2.5 && id>0.92 && iso < 0.15));

    idMVAnotrig[j] = i.mvaOut;
    idMVAtrig[j] = i.mvaOutTrig;
    idMVApresel[j] = mvaPreSel;
}

template <> void LeptonInfo::setSpecific < VHbbEvent::MuonInfo >
    (const VHbbEvent::MuonInfo & i, int j, const VHbbEventAuxInfo & aux) {
    dxy[j] = i.ipDb;
    dz[j] = i.zPVPt;
    vbtf[j] = ( i.globChi2<10 && i.nPixelHits>= 1 && i.globNHits != 0 && i.nHits > 10 && i.cat & 0x1 && i.cat & 0x2 && i.nMatches >=2 && i.ipDb<.2 &&
        (i.pfChaIso+i.pfPhoIso+i.pfNeuIso)/i.p4.Pt()<.15  && fabs(i.p4.Eta())<2.4 && i.p4.Pt()>20 ) ;
    float mincor = 0.0;
    float minrho = 0.0;
    float rhoN = std::max(aux.puInfo.rhoNeutral, minrho);
    float eta = i.p4.Eta();
    float area = 0.5;
    if(fabs(eta)>0.0 && fabs(eta) <= 1.0) {area=0.674;}
    if(fabs(eta)>1.0 && fabs(eta) <= 1.5) {area=0.565;}
    if(fabs(eta)>1.5 && fabs(eta) <= 2.0) {area=0.442;}
    if(fabs(eta)>2.0 && fabs(eta) <= 2.2) {area=0.515;}
    if(fabs(eta)>2.2 && fabs(eta) <= 2.3) {area=0.821;}
    if(fabs(eta)>2.3 && fabs(eta) <= 2.4) {area=0.660;}
    pfCorrIso[j] = (i.pfChaIso+ std::max(i.pfPhoIso+i.pfNeuIso-rhoN*area,mincor))/i.p4.Pt();
    id2012tight[j] = i.isPF && i. globChi2<10 && i.nPixelHits>= 1 && i.globNHits != 0 && i.nValidLayers > 5 && (i.cat & 0x2) && i.nMatches >=2 && i.ipDb<.2;
}

/// CSV Reshaping
BTagShapeInterface *nominalShape = 0;
BTagShapeInterface *downBCShape = 0;
BTagShapeInterface *upBCShape = 0;
BTagShapeInterface *downLShape = 0;
BTagShapeInterface *upLShape = 0;

BTagShapeInterface *nominalShape4p = 0;
BTagShapeInterface *downBCShape4p = 0;
BTagShapeInterface *upBCShape4p = 0;

typedef struct {
    void set(const VHbbEvent::SimpleJet & j, int i) {
        pt[i] = j.p4.Pt();
        eta[i] = j.p4.Eta();
        phi[i] = j.p4.Phi();
        e[i] = j.p4.E();
        csv[i] = j.csv;
        if (nominalShape)
        {
            csv_nominal[i]=nominalShape->reshape(eta[i],pt[i],j.csv,j.flavour);
            csv_upBC[i]=upBCShape->reshape(eta[i],pt[i],j.csv,j.flavour);
            csv_downBC[i]=downBCShape->reshape(eta[i],pt[i],j.csv,j.flavour);
            csv_upL[i]=upLShape->reshape(eta[i],pt[i],j.csv,j.flavour);
            csv_downL[i]=downLShape->reshape(eta[i],pt[i],j.csv,j.flavour);
            
            csv_nominal4p[i]=nominalShape4p->reshape(eta[i],pt[i],j.csv,j.flavour);
            csv_upBC4p[i]=upBCShape4p->reshape(eta[i],pt[i],j.csv,j.flavour);
            csv_downBC4p[i]=downBCShape4p->reshape(eta[i],pt[i],j.csv,j.flavour);
        } else {
            csv_nominal[i] = csv[i];
            csv_downBC[i] = csv[i];
            csv_upBC[i] = csv[i];
            csv_downL[i] = csv[i];
            csv_upL[i] = csv[i];
            
            csv_nominal4p[i] = csv[i];
            csv_downBC4p[i] = csv[i];
            csv_upBC4p[i] = csv[i];
        }
        csvivf[i] = j.csvivf;
        cmva[i] = j.cmva;
        numTracksSV[i] = j.vtxNTracks;
        vtxMass[i] = j.vtxMass;
        vtxPt[i] = j.vtxP4.Pt();
        if (j.vtxP4.Pt() > 0.1) vtxEta[i] = j.vtxP4.Eta();  else vtxEta[i] = -99;
        vtxPhi[i] = j.vtxP4.Phi();
        vtxE[i] = j.vtxP4.E();
        vtx3dL[i] = j.vtx3dL;
        vtx3deL[i] = j.vtx3deL;
        chf[i] = j.chargedHadronEFraction;
        nhf[i] = j.neutralHadronEFraction;
        cef[i] = j.chargedEmEFraction;
        nef[i] = j.neutralEmEFraction;
        nconstituents[i] = j.nConstituents;
        nch[i] = j.ntracks;
        SF_CSVL[i] = j.SF_CSVL;
        SF_CSVM[i] = j.SF_CSVM;
        SF_CSVT[i] = j.SF_CSVT;
        SF_CSVLerr[i] = j.SF_CSVLerr;
        SF_CSVMerr[i] = j.SF_CSVMerr;
        SF_CSVTerr[i] = j.SF_CSVTerr;

        flavour[i] = j.flavour;
        isSemiLeptMCtruth[i] = j.isSemiLeptMCtruth;
        isSemiLept[i] = j.isSemiLept;
        SoftLeptpdgId[i] = j.SoftLeptpdgId;
        SoftLeptIdlooseMu[i] = j.SoftLeptIdlooseMu;
        SoftLeptId95[i] = j.SoftLeptId95;
        SoftLeptPt[i] = j.SoftLeptPt;
        SoftLeptdR[i] = j.SoftLeptdR;
        SoftLeptptRel[i] = j.SoftLeptptRel;
        SoftLeptRelCombIso[i] = j.SoftLeptRelCombIso;
        if (j.bestMCp4.Pt() > 0) {
            genPt[i] = j.bestMCp4.Pt();
            genEta[i] = j.bestMCp4.Eta();
            genPhi[i] = j.bestMCp4.Phi();
        }
        JECUnc[i] = j.jecunc;
        id[i] = jetId(i);
        ptRaw[i] = j.ptRaw;
        ptLeadTrack[i] = j.ptLeadTrack;
        puJetIdL[i] = j.puJetIdL;
        puJetIdM[i] = j.puJetIdM;
        puJetIdT[i] = j.puJetIdT;
        puJetIdMva[i] = j.puJetIdMva;
        charge[i] = j.charge;
        jetArea[i] = j.jetArea;
    }

    bool jetId(int i) {
        if (nhf[i] > 0.99)
            return false;
        if (nef[i] > 0.99)
            return false;
        if (nconstituents[i] <= 1)
            return false;
        if (fabs(eta[i]) < 2.4) {
            if (cef[i] > 0.99)
                return false;
            if (chf[i] == 0)
                return false;
            if (nch[i] == 0)
                return false;
        }
        return true;
    }
    
    void reset() {
        for (int i = 0; i < MAXJ; i++) {
            pt[i]=-99; eta[i]=-99; phi[i]=-99;e[i]=-99;csv[i]=-99;csv_nominal[i]=-99.;csv_upBC[i]=-99.;csv_downBC[i]=-99.;csv_upL[i]=-99.;csv_downL[i]=-99.;csv_nominal4p[i]=-99.;csv_upBC4p[i]=-99.;csv_downBC4p[i]=-99.;
            csvivf[i]=-99; cmva[i]=-99;
            cosTheta[i]=-99; numTracksSV[i]=-99; chf[i]=-99; nhf[i]=-99; cef[i]=-99; nef[i]=-99; nch[i]=-99; nconstituents[i]=-99; flavour[i]=-99; 
            isSemiLeptMCtruth[i]=-99; isSemiLept[i]=-99; SoftLeptpdgId[i] = -99; SoftLeptIdlooseMu[i] = -99; SoftLeptId95[i] = -99; SoftLeptPt[i] = -99; SoftLeptdR[i] = -99; SoftLeptptRel[i] = -99; SoftLeptRelCombIso[i] = -99;  
            genPt[i]=-99; genEta[i]=-99; genPhi[i]=-99; JECUnc[i]=-99; 
            vtxMass[i] = -99.; vtxPt[i] = -99.; vtxEta[i] = -99.; vtxPhi[i] = -99.; vtxE[i] = -99.; vtx3dL[i] = -99.; vtx3deL[i] = -99.; id[i] = -99.;
            ptRaw[i]=-99.; ptLeadTrack[i]=-99.; puJetIdL[i]=-99; puJetIdM[i]=-99; puJetIdT[i]=-99; puJetIdMva[i]=-99; charge[i]=-99; jetArea[i]=-99;
        }
    }
    float pt[MAXJ];
    float eta[MAXJ];
    float phi[MAXJ];
    float e[MAXJ];
    float csv[MAXJ];
    float csv_nominal[MAXJ];
    float csv_upBC[MAXJ];
    float csv_downBC[MAXJ];
    float csv_upL[MAXJ];
    float csv_downL[MAXJ];
    float csv_nominal4p[MAXJ];
    float csv_upBC4p[MAXJ];
    float csv_downBC4p[MAXJ];
    float csvivf[MAXJ];
    float cmva[MAXJ];
    float cosTheta[MAXJ];
    int numTracksSV[MAXJ];
    float chf[MAXJ];
    float nhf[MAXJ];
    float cef[MAXJ];
    float nef[MAXJ];
    float nch[MAXJ];
    float nconstituents[MAXJ];  // should be int?
    float flavour[MAXJ];  // should be int?
    int isSemiLept[MAXJ];
    int isSemiLeptMCtruth[MAXJ];
    int SoftLeptpdgId[MAXJ];
    int SoftLeptIdlooseMu[MAXJ];
    int SoftLeptId95[MAXJ];
    float SoftLeptPt[MAXJ];
    float SoftLeptdR[MAXJ];
    float SoftLeptptRel[MAXJ];
    float SoftLeptRelCombIso[MAXJ];
    float genPt[MAXJ];
    float genEta[MAXJ];
    float genPhi[MAXJ];
    float JECUnc[MAXJ];
    float vtxMass[MAXJ];
    float vtxPt[MAXJ];
    float vtxEta[MAXJ];
    float vtxPhi[MAXJ];
    float vtxE[MAXJ];
    float vtx3dL[MAXJ];
    float vtx3deL[MAXJ];
    bool id[MAXJ];
    float SF_CSVL[MAXJ];
    float SF_CSVM[MAXJ];
    float SF_CSVT[MAXJ];
    float SF_CSVLerr[MAXJ];
    float SF_CSVMerr[MAXJ];
    float SF_CSVTerr[MAXJ];
    float ptRaw[MAXJ];
    float ptLeadTrack[MAXJ];
    float puJetIdL[MAXJ];
    float puJetIdM[MAXJ];
    float puJetIdT[MAXJ];
    float puJetIdMva[MAXJ];
    float charge[MAXJ];
    float jetArea[MAXJ];
} JetInfo;

typedef struct {
    float mass;
    float pt;
    float eta;
    float phi;
    float status;
    float charge;
    float momid;
} genParticleInfo;

typedef struct {
    float bmass;
    float bpt;
    float beta;
    float bphi;
    float bstatus;
    float wdau1mass;
    float wdau1pt;
    float wdau1eta;
    float wdau1phi;
    float wdau1id;
    float wdau2mass;
    float wdau2pt;
    float wdau2eta;
    float wdau2phi;
    float wdau2id;
} genTopInfo;

typedef struct {
    float mass;
    float pt;
    float wMass;
} TopInfo;

typedef struct {
    void set(const SimBHadron & sbhc, int i) {
        mass[i] = sbhc.mass();
        pt[i] = sbhc.pt();
        eta[i] = sbhc.eta();
        phi[i] = sbhc.phi();
        vtx_x[i] = sbhc.decPosition.x();
        vtx_y[i] = sbhc.decPosition.y();
        vtx_z[i] = sbhc.decPosition.z();
        pdgId[i] = sbhc.pdgId();
        status[i] = sbhc.status();
    };
    void reset() {
        for (int i = 0; i < MAXB; ++i) {
            mass[i] = -99; pt[i] = -99; eta[i] = -99; phi[i] = -99; vtx_x[i] = -99; vtx_y[i] = -99; vtx_z[i] = -99; pdgId[i] = -99; status[i] = -99;
        }
    };
    float mass[MAXB];
    float pt[MAXB];
    float eta[MAXB];
    float phi[MAXB];
    float vtx_x[MAXB];
    float vtx_y[MAXB];
    float vtx_z[MAXB];
    int pdgId[MAXB];
    int status[MAXB];
    //int quarkStatus[MAXB];
    //int brotherStatus[MAXB];
    //int otherId[MAXB];
    //bool etaOk[MAXB];
    //bool simOk[MAXB];
    //bool trackOk[MAXB];
    //bool cutOk[MAXB];
    //bool cutNewOk[MAXB];
    //bool mcMatchOk[MAXB];
    //bool matchOk[MAXB];
} SimBHadronInfo;

typedef struct {
    void set(const reco::SecondaryVertex & recoSv, const TVector3 recoPv, int isv) {
        pt[isv] = recoSv.p4().Pt();
        eta[isv] = flightDirection(recoPv, recoSv).eta();
        phi[isv] = flightDirection(recoPv, recoSv).phi();
        massBcand[isv] = recoSv.p4().M();
        massSv[isv] = recoSv.p4().M();
        dist3D[isv] = recoSv.dist3d().value();
        distSig3D[isv] = recoSv.dist3d().significance();
        dist2D[isv] = recoSv.dist2d().value();
        distSig2D[isv] = recoSv.dist2d().significance();
        dist3D_norm[isv] = recoSv.dist3d().value() / recoSv.p4().Gamma();
    };
    void reset() {
        for (int i = 0; i < MAXB; ++i) {
            massBcand[i] = -99; massSv[i]= -99; pt[i] = -99; eta[i] = -99; phi[i] = -99; dist3D[i] = -99; distSig3D[i] = -99; dist2D[i] = -99; distSig2D[i] = -99; dist3D_norm[i] = -99;
        }
    };
    float massBcand[MAXB];
    float massSv[MAXB];
    float pt[MAXB];
    float eta[MAXB];
    float phi[MAXB];
    float dist3D[MAXB];
    float distSig3D[MAXB];
    float dist2D[MAXB];
    float distSig2D[MAXB];
    float dist3D_norm[MAXB];
} IVFInfo;

typedef struct {
    bool HiggsCSVtkSharing;
    bool HiggsIPtkSharing;
    bool HiggsSVtkSharing;
    bool FatHiggsCSVtkSharing;
    bool FatHiggsIPtkSharing;
    bool FatHiggsSVtkSharing;
} TrackSharingInfo;


// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    gROOT->Reset();

    // -------------------------------------------------------------------------
    // Declare variables
    // -------------------------------------------------------------------------
    IVFInfo IVF;
    SimBHadronInfo SimBs;
    float rho, rho25, rhoN;
    int nPVs, nGoodPVs;
    METInfo MET;
    METInfo fakeMET;
    METInfo METnoPU;
    METInfo METnoPUCh;
    METInfo METtype1corr;
    METInfo METtype1p2corr;
    METInfo caloMET;
    MHTInfo MHT;
    METUncInfo metUnc;
    
    JetInfo hJets, aJets, fathFilterJets, aJetsFat;
    LeptonInfo vLeptons, aLeptons;
    int naJets = 0, nhJets = 0, nfathFilterJets = 0, naJetsFat = 0;

    TopInfo top;
    EventInfo EVENT;    
    HiggsInfo H, SVH, SimBsH;
    FatHiggsInfo FatH;
    genParticleInfo genZ, genZstar, genWstar, genW, genH, genB, genBbar;  // add here the fatjet higgs
    genTopInfo genTop, genTbar;
    VInfo V;
    int nvlep = 0, nalep = 0;
    float lheV_pt = 0;          // for the Madgraph sample stitching
    float lheHT = 0;            // for the Madgraph sample stitching
    int lheNj = 0;              // for the Madgraph sample stitching
    TrackSharingInfo TkSharing; // track sharing info;

    float HVdPhi, HVMass, HMETdPhi, VMt, deltaPullAngle, deltaPullAngleAK7,
        deltaPullAngle2, deltaPullAngle2AK7, gendrcc, gendrbb, genZpt, genWpt,
        genHpt; 
    float weightTrig, weightTrigMay, weightTrigV4, weightTrigMET,
        weightTrigOrMu30, minDeltaPhijetMET, jetPt_minDeltaPhijetMET;
    float PUweight, PUweightP, PUweightM, PUweightAB, PUweight2011B,
        PUweight1DObs;
    float PU0, PUp1, PUm1;

    float weightEleRecoAndId, weightEleTrigJetMETPart, weightEleTrigElePart,
        weightEleTrigEleAugPart;
    float weightTrigMET80, weightTrigMET100, weightTrig2CJet20,
        weightTrigMET150, weightTrigMET802CJet, weightTrigMET1002CJet,
        weightTrigMETLP;

    float weightTrig2012A, weightTrig2012ADiMuon, weightTrig2012ADiEle,
        weightTrig2012ASingleMuon, weightTrig2012AMuonPlusWCandPt,
        weightTrig2012ASingleEle;
    float weightTrig2012AB, weightTrig2012ABDiMuon, weightTrig2012ABDiEle,
        weightTrig2012ABSingleMuon, weightTrig2012ABMuonPlusWCandPt,
        weightTrig2012ABSingleEle;
    float weightTrig2012, weightTrig2012DiMuon, weightTrig2012DiEle,
        weightTrig2012SingleMuon, weightTrig2012MuonPlusWCandPt,
        weightTrig2012SingleEle;

    float weightTrig2012DiJet30MHT80, weightTrig2012PFMET150,
        weightTrig2012SumpT100MET100;
    float weightTrig2012APFMET150orDiJetMET,
        weightTrig2012BPFMET150orDiJetMET, weightTrig2012CPFMET150orDiJetMET;

    int WplusMode, WminusMode;
    int Vtype, nSvs = 0;
    int nSimBs = 0;
    int numJets, numBJets, eventFlav;
    bool triggerFlags[500], hbhe, ecalFlag, cschaloFlag,
        hcallaserFlag, trackingfailureFlag, eebadscFlag;

    float btag1T2CSF = 1., btag2TSF = 1., btag1TSF = 1., btagA0CSF = 1., 
        btagA0TSF = 1., btag2CSF = 1., btag1TA1C = 1.;


    // Load framework libraries
    gSystem->Load("libFWCoreFWLite");
    gSystem->Load("libDataFormatsFWLite");
    AutoLibraryLoader::enable();

    // -------------------------------------------------------------------------
    // Parse arguments
    // -------------------------------------------------------------------------
    if (argc < 2) {
        return 0;
    }

    /// Get the python configuration
    PythonProcessDesc builder(argv[1]);
    const edm::ParameterSet & in    = builder.processDesc()->getProcessPSet()->getParameter < edm::ParameterSet > ("fwliteInput");
    const edm::ParameterSet & out   = builder.processDesc()->getProcessPSet()->getParameter < edm::ParameterSet > ("fwliteOutput");
    const edm::ParameterSet & ana   = builder.processDesc()->getProcessPSet()->getParameter < edm::ParameterSet > ("Analyzer");
    const edm::ParameterSet & anaAB = builder.processDesc()->getProcessPSet()->getParameter < edm::ParameterSet > ("Analyzer2012ABOnly");
    
    /// Parameters from "in"
    const std::vector < std::string > inputFiles_(in.getParameter < std::vector < std::string > >("fileNames"));
    std::string PUmcfileName2011B_ = in.getParameter < std::string > ("PUmcfileName2011B");
    std::string PUdatafileName2011B_ = in.getParameter < std::string > ("PUdatafileName2011B");
    std::string PUmcfileName_ = in.getParameter < std::string > ("PUmcfileName");
    std::string PUdatafileNameAB = in.getParameter < std::string > ("PUdatafileNameAB");
    std::string PUdatafileName_ = in.getParameter < std::string > ("PUdatafileName");
    std::string PUdatafileName_minusOneSigma = in.getParameter < std::string > ("PUdatafileNameMinus");
    std::string PUdatafileName_plusOneSigma = in.getParameter < std::string > ("PUdatafileNamePlus");
    std::string Weight3DfileName_ = in.getParameter < std::string > ("Weight3DfileName");
    int maxEvents_(in.getParameter < int > ("maxEvents"));
    int runMin_(in.getParameter < int > ("runMin"));
    int runMax_(in.getParameter < int > ("runMax"));
    int skipEvents_(in.getParameter < int > ("skipEvents"));
    unsigned int outputEvery_(in.getParameter < unsigned int > ("outputEvery"));  // not implemented
    std::vector < edm::LuminosityBlockRange > jsonVector;
    if (in.exists("lumisToProcess") && ana.getParameter < bool > ("isMC")) {
        std::vector < edm::LuminosityBlockRange > const &lumisTemp = in.getUntrackedParameter < std::vector < edm::LuminosityBlockRange > >("lumisToProcess");
        jsonVector.resize(lumisTemp.size());
        copy(lumisTemp.begin(), lumisTemp.end(), jsonVector.begin());
    }
    
    /// Parameters from "out"
    std::string outputFile_(out.getParameter < std::string > ("fileName"));
    
    /// Parameters from "ana"
    const std::vector < std::string > triggers(ana.getParameter < std::vector < std::string > >("triggers"));
    bool isMC_(ana.getParameter < bool > ("isMC"));
    bool verbose = ana.getParameter < bool > ("verbose");
    bool fromCandidate = ana.getParameter < bool > ("readFromCandidates");
    double btagThr = ana.getParameter < double >("bJetCountThreshold");
    bool useHighestPtHiggsZ = ana.getParameter < bool > ("useHighestPtHiggsZ");
    bool useHighestPtHiggsW = ana.getParameter < bool > ("useHighestPtHiggsW");
    HbbCandidateFinderAlgo *algoZ =
        new HbbCandidateFinderAlgo(verbose, ana.getParameter < double >("jetPtThresholdZ"), useHighestPtHiggsZ);
    HbbCandidateFinderAlgo *algoW =
        new HbbCandidateFinderAlgo(verbose, ana.getParameter < double >("jetPtThresholdW"), useHighestPtHiggsW);
    HbbCandidateFinderAlgo *algoRecoverLowPt =
        new HbbCandidateFinderAlgo(verbose, 15, true);

    const std::vector < std::string > patFilters = {"hbhe", "ecalFilter", "cschaloFilter", "hcallaserFilter", "trackingfailureFilter", "eebadscFilter"};
    TriggerWeight triggerWeight(ana);
    TriggerWeight triggerWeightAB(anaAB);
    bool passAllTriggers = false;
    TriggerReader trigger(passAllTriggers);
    TriggerReader patFilter(passAllTriggers);

    JECFWLite jec(ana.getParameter < std::string > ("jecFolder"));

    BTagWeight btag(2);  // 2 operating points "Custom" = 0.5 and "Tight = 0.898"
    BTagSampleEfficiency btagEff(ana.getParameter < std::string >("btagEffFileName").c_str());

    if (isMC_) {
        nominalShape = new BTagShapeInterface(ana.getParameter < std::string >("csvDiscr").c_str(), 0.0, 0.0);
        upBCShape = new BTagShapeInterface(ana.getParameter < std::string >("csvDiscr").c_str(), 1.5, 0.0);
        downBCShape = new BTagShapeInterface(ana.getParameter < std::string >("csvDiscr").c_str(), -1.5, 0.0);
        upLShape = new BTagShapeInterface(ana.getParameter < std::string >("csvDiscr").c_str(), 0.0, 1.0);
        downLShape = new BTagShapeInterface(ana.getParameter < std::string >("csvDiscr").c_str(), 0.0, -1.0);
        nominalShape4p = new BTagShapeInterface(ana.getParameter < std::string >("csvDiscr").c_str(), 0.0, 0.0, true, 1.003, 1.003);
        upBCShape4p = new BTagShapeInterface(ana.getParameter < std::string >("csvDiscr").c_str(), 0.0, 0.0, true, 1.001, 1.001);
        downBCShape4p = new BTagShapeInterface(ana.getParameter < std::string >("csvDiscr").c_str(), 0.0, 0.0, true, 1.005, 1.005);
    }
    
    edm::LumiReWeighting lumiWeights, lumiWeightsPl, lumiWeightsMi, lumiWeightsAB;
    edm::LumiReWeighting lumiWeights1DObs;
    //edm::Lumi3DReWeighting lumiWeights2011B;
    if (isMC_) {
        lumiWeights = edm::LumiReWeighting(PUmcfileName_, PUdatafileName_, "pileup", "pileup");
        lumiWeightsPl = edm::LumiReWeighting(PUmcfileName_, PUdatafileName_plusOneSigma, "pileup", "pileup");
        lumiWeightsMi = edm::LumiReWeighting(PUmcfileName_, PUdatafileName_minusOneSigma, "pileup", "pileup");
        lumiWeightsAB = edm::LumiReWeighting(PUmcfileName_, PUdatafileNameAB, "pileup", "pileup");
        lumiWeights1DObs = edm::LumiReWeighting(PUmcfileName2011B_, PUdatafileName2011B_, "pileup", "pileup");
        //lumiWeights2011B = edm::Lumi3DReWeighting(PUmcfileName2011B_,PUdatafileName2011B_ , "pileup", "pileup");
        //if(Weight3DfileName_ != "") { lumiWeights2011B.weight3D_init(Weight3DfileName_.c_str()); }
        //else { lumiWeights2011B.weight3D_init(1.0); }  // generate the weights the fisrt time;
    }
    
    
    // -------------------------------------------------------------------------
    // Create TTree
    // -------------------------------------------------------------------------
    TFile *_outFile = new TFile(outputFile_.c_str(), "recreate");
    TH1F *count = new TH1F("Count", "Count", 1, 0, 2);
    TH1F *countWithPU = new TH1F("CountWithPU", "CountWithPU", 1, 0, 2);
    TH1F *countWithPUP = new TH1F("CountWithPUP", "CountWithPU plus one sigma", 1, 0, 2);
    TH1F *countWithPUM = new TH1F("CountWithPUM", "CountWithPU minus one sigma", 1, 0, 2);
    TH1F *countWithPUAB = new TH1F("CountWithPUAB", "CountWithPU 2012AB", 1, 0, 2);
    TH1F *countWithPU2011B = new TH1F("CountWithPU2011B", "CountWithPU2011B", 1, 0, 2);
    TH3F *input3DPU = new TH3F("Input3DPU", "Input3DPU", 36, -0.5, 35.5, 36, -0.5, 35.5, 36, -0.5, 35.5);
    TH1F *pu = new TH1F("pileup", "", 51, -0.5, 50.5);

    TTree *_outTree = new TTree("tree", "myTree");
    _outTree->Branch("H", &H, "HiggsFlag/I:mass/F:pt/F:eta/F:phi/F:dR/F:dPhi/F:dEta/F");
    _outTree->Branch("V", &V, "mass/F:pt/F:eta/F:phi/F");
    _outTree->Branch("FatH", &FatH, "FatHiggsFlag/I:mass/F:pt/F:eta/F:phi/F:filteredmass/F:filteredpt/F:filteredeta/F:filteredphi/F");
    _outTree->Branch("lheV_pt", &lheV_pt, "lheV_pt/F");
    _outTree->Branch("lheHT", &lheHT, "lheHT/F");
    _outTree->Branch("lheNj", &lheNj, "lheNj/I");
    _outTree->Branch("genZ", &genZ, "mass/F:pt/F:eta/F:phi/F:status/F:charge/F:momid/F");
    _outTree->Branch("genZstar", &genZstar, "mass/F:pt/F:eta/F:phi/F:status/F:charge/F:momid/F");
    _outTree->Branch("genW", &genW, "mass/F:pt/F:eta/F:phi/F:status/F:charge/F:momid/F");
    _outTree->Branch("genWstar", &genWstar, "mass/F:pt/F:eta/F:phi/F:status/F:charge/F:momid/F");
    _outTree->Branch("genH", &genH, "mass/F:pt/F:eta/F:phi/F:status/F:charge/F:momid/F");
    _outTree->Branch("genB", &genB, "mass/F:pt/F:eta/F:phi/F:status/F:charge/F:momid/F");
    _outTree->Branch("genBbar", &genBbar, "mass/F:pt/F:eta/F:phi/F:status/F:charge/F:momid/F");
    _outTree->Branch("genTop", &genTop, "bmass/F:bpt/F:beta/F:bphi/F:bstatus/F:wdau1mass/F:wdau1pt/F:wdau1eta/F:wdau1phi/F:wdau1id/F:wdau2mass/F:wdau2pt/F:wdau2eta/F:wdau2phi/F:wdau2id/F");
    _outTree->Branch("genTbar", &genTbar, "bmass/F:bpt/F:beta/F:bphi/F:bstatus/F:wdau1mass/F:wdau1pt/F:wdau1eta/F:wdau1phi/F:wdau1id/F:wdau2mass/F:wdau2pt/F:wdau2eta/F:wdau2phi/F:wdau2id/F");
    _outTree->Branch("TkSharing", &TkSharing, "HiggsCSVtkSharing/b:HiggsIPtkSharing/b:HiggsSVtkSharing/b:FatHiggsCSVtkSharing/b:FatHiggsIPtkSharing/b:FatHiggsSVtkSharing/b");
    _outTree->Branch("nhJets", &nhJets, "nhJets/I");
    _outTree->Branch("nfathFilterJets", &nfathFilterJets, "nfathFilterJets/I");
    _outTree->Branch("naJets", &naJets, "naJets/I");

    _outTree->Branch("hJet_pt", hJets.pt, "pt[nhJets]/F");
    _outTree->Branch("hJet_eta", hJets.eta, "eta[nhJets]/F");
    _outTree->Branch("hJet_phi", hJets.phi, "phi[nhJets]/F");
    _outTree->Branch("hJet_e", hJets.e, "e[nhJets]/F");
    _outTree->Branch("hJet_csv", hJets.csv, "csv[nhJets]/F");
    _outTree->Branch("hJet_csv_nominal", hJets.csv_nominal, "csv_nominal[nhJets]/F");
    _outTree->Branch("hJet_csv_upBC", hJets.csv_upBC, "csv_upBC[nhJets]/F");
    _outTree->Branch("hJet_csv_downBC", hJets.csv_downBC, "csv_downBC[nhJets]/F");
    _outTree->Branch("hJet_csv_upL", hJets.csv_upL, "csv_upL[nhJets]/F");
    _outTree->Branch("hJet_csv_downL", hJets.csv_downL, "csv_downL[nhJets]/F");
    _outTree->Branch("hJet_csv_nominal4p", hJets.csv_nominal4p, "csv_nominal4p[nhJets]/F");
    _outTree->Branch("hJet_csv_upBC4p", hJets.csv_upBC4p, "csv_upBC4p[nhJets]/F");
    _outTree->Branch("hJet_csv_downBC4p", hJets.csv_downBC4p, "csv_downBC4p[nhJets]/F");

    _outTree->Branch("hJet_csvivf", hJets.csvivf, "csvivf[nhJets]/F");
    _outTree->Branch("hJet_cmva", hJets.cmva, "cmva[nhJets]/F");
    _outTree->Branch("hJet_cosTheta", hJets.cosTheta, "cosTheta[nhJets]/F");
    _outTree->Branch("hJet_numTracksSV", hJets.numTracksSV, "numTracksSV[nhJets]/I");
    _outTree->Branch("hJet_chf", hJets.chf, "chf[nhJets]/F");
    _outTree->Branch("hJet_nhf", hJets.nhf, "nhf[nhJets]/F");
    _outTree->Branch("hJet_cef", hJets.cef, "cef[nhJets]/F");
    _outTree->Branch("hJet_nef", hJets.nef, "nef[nhJets]/F");
    _outTree->Branch("hJet_nch", hJets.nch, "nch[nhJets]/F");
    _outTree->Branch("hJet_nconstituents", hJets.nconstituents, "nconstituents[nhJets]/F");
    _outTree->Branch("hJet_flavour", hJets.flavour, "flavour[nhJets]/F");
    _outTree->Branch("hJet_isSemiLept", hJets.isSemiLept, "isSemiLept[nhJets]/I");
    _outTree->Branch("hJet_isSemiLeptMCtruth", hJets.isSemiLeptMCtruth, "isSemiLeptMCtruth[nhJets]/I");
    _outTree->Branch("hJet_SoftLeptpdgId", hJets.SoftLeptpdgId, "SoftLeptpdgId[nhJets]/I");
    _outTree->Branch("hJet_SoftLeptIdlooseMu", hJets.SoftLeptIdlooseMu, "SoftLeptIdlooseMu[nhJets]/I");
    _outTree->Branch("hJet_SoftLeptId95", hJets.SoftLeptId95, "SoftLeptId95[nhJets]/I");
    _outTree->Branch("hJet_SoftLeptPt", hJets.SoftLeptPt, "SoftLeptPt[nhJets]/F");
    _outTree->Branch("hJet_SoftLeptdR", hJets.SoftLeptdR, "SoftLeptdR[nhJets]/F");
    _outTree->Branch("hJet_SoftLeptptRel", hJets.SoftLeptptRel, "SoftLeptptRel[nhJets]/F");
    _outTree->Branch("hJet_SoftLeptRelCombIso", hJets.SoftLeptRelCombIso, "SoftLeptRelCombIso[nhJets]/F");
    _outTree->Branch("hJet_genPt", hJets.genPt, "genPt[nhJets]/F");
    _outTree->Branch("hJet_genEta", hJets.genEta, "genEta[nhJets]/F");
    _outTree->Branch("hJet_genPhi", hJets.genPhi, "genPhi[nhJets]/F");
    _outTree->Branch("hJet_JECUnc", hJets.JECUnc, "JECUnc[nhJets]/F");
    _outTree->Branch("hJet_vtxMass", hJets.vtxMass, "vtxMass[nhJets]/F");
    _outTree->Branch("hJet_vtxPt", hJets.vtxPt, "vtxPt[nhJets]/F");
    _outTree->Branch("hJet_vtxEta", hJets.vtxEta, "vtxEta[nhJets]/F");
    _outTree->Branch("hJet_vtxPhi", hJets.vtxPhi, "vtxPhi[nhJets]/F");
    _outTree->Branch("hJet_vtxE", hJets.vtxE, "vtxE[nhJets]/F");
    _outTree->Branch("hJet_vtx3dL", hJets.vtx3dL, "vtx3dL[nhJets]/F");
    _outTree->Branch("hJet_vtx3deL", hJets.vtx3deL, "vtx3deL[nhJets]/F");
    _outTree->Branch("hJet_id", hJets.id, "id[nhJets]/b");
    _outTree->Branch("hJet_SF_CSVL", hJets.SF_CSVL, "SF_CSVL[nhJets]/b");
    _outTree->Branch("hJet_SF_CSVM", hJets.SF_CSVM, "SF_CSVM[nhJets]/b");
    _outTree->Branch("hJet_SF_CSVT", hJets.SF_CSVT, "SF_CSVT[nhJets]/b");
    _outTree->Branch("hJet_SF_CSVLerr", hJets.SF_CSVLerr, "SF_CSVLerr[nhJets]/b");
    _outTree->Branch("hJet_SF_CSVMerr", hJets.SF_CSVMerr, "SF_CSVMerr[nhJets]/b");
    _outTree->Branch("hJet_SF_CSVTerr", hJets.SF_CSVTerr, "SF_CSVTerr[nhJets]/b");
    _outTree->Branch("hJet_ptRaw", hJets.ptRaw, "ptRaw[nhJets]/F");
    _outTree->Branch("hJet_ptLeadTrack", hJets.ptLeadTrack, "ptLeadTrack[nhJets]/F");
    _outTree->Branch("hJet_puJetIdL", hJets.puJetIdL, "puJetIdL[nhJets]/F");
    _outTree->Branch("hJet_puJetIdM", hJets.puJetIdM, "puJetIdM[nhJets]/F");
    _outTree->Branch("hJet_puJetIdT", hJets.puJetIdT, "puJetIdT[nhJets]/F");
    _outTree->Branch("hJet_puJetIdMva", hJets.puJetIdMva, "puJetIdMva[nhJets]/F");
    _outTree->Branch("hJet_charge", hJets.charge, "charge[nhJets]/F");

    _outTree->Branch("fathFilterJets_pt", fathFilterJets.pt, "pt[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_eta", fathFilterJets.eta, "eta[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_phi", fathFilterJets.phi, "phi[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_e", fathFilterJets.e, "e[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_csv", fathFilterJets.csv, "csv[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_chf", fathFilterJets.chf, "chf[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_ptRaw", fathFilterJets.ptRaw, "ptRaw[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_ptLeadTrack", fathFilterJets.ptLeadTrack, "ptLeadTrack[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_flavour", fathFilterJets.flavour, "flavour[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_isSemiLept", fathFilterJets.isSemiLept, "isSemiLept[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_isSemiLeptMCtruth", fathFilterJets.isSemiLeptMCtruth, "isSemiLeptMCtruth[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_genPt", fathFilterJets.genPt, "genPt[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_genEta", fathFilterJets.genEta, "genEta[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_genPhi", fathFilterJets.genPhi, "genPhi[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_vtxMass", fathFilterJets.vtxMass, "vtxMass[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_vtx3dL", fathFilterJets.vtx3dL, "vtx3dL[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_vtx3deL", fathFilterJets.vtx3deL, "vtx3deL[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_vtxPt", fathFilterJets.vtxPt, "vtxPt[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_vtxEta", fathFilterJets.vtxEta, "vtxEta[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_vtxPhi", fathFilterJets.vtxPhi, "vtxPhi[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_vtxE", fathFilterJets.vtxE, "vtxE[nfathFilterJets]/F");

    _outTree->Branch("fathFilterJets_csvivf", fathFilterJets.csvivf, "csvivf[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_cmva", fathFilterJets.cmva, "cmva[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_cosTheta", fathFilterJets.cosTheta, "cosTheta[nfathFilterJets]/F");
    _outTree->Branch("fathFilterJets_jetArea", fathFilterJets.jetArea, "jetArea[nfathFilterJets]/F");

    _outTree->Branch("aJet_pt", aJets.pt, "pt[naJets]/F");
    _outTree->Branch("aJet_eta", aJets.eta, "eta[naJets]/F");
    _outTree->Branch("aJet_phi", aJets.phi, "phi[naJets]/F");
    _outTree->Branch("aJet_e", aJets.e, "e[naJets]/F");
    _outTree->Branch("aJet_csv", aJets.csv, "csv[naJets]/F");
    _outTree->Branch("aJet_csv_nominal", aJets.csv_nominal, "csv_nominal[naJets]/F");
    _outTree->Branch("aJet_csv_upBC", aJets.csv_upBC, "csv_upBC[naJets]/F");
    _outTree->Branch("aJet_csv_downBC", aJets.csv_downBC, "csv_downBC[naJets]/F");
    _outTree->Branch("aJet_csv_upL", aJets.csv_upL, "csv_upL[naJets]/F");
    _outTree->Branch("aJet_csv_downL", aJets.csv_downL, "csv_downL[naJets]/F");
    _outTree->Branch("aJet_csv_nominal4p", aJets.csv_nominal4p, "csv_nominal4p[naJets]/F");
    _outTree->Branch("aJet_csv_upBC4p", aJets.csv_upBC4p, "csv_upBC4p[naJets]/F");
    _outTree->Branch("aJet_csv_downBC4p", aJets.csv_downBC4p, "csv_downBC4p[naJets]/F");

    _outTree->Branch("aJet_csvivf", aJets.csvivf, "csvivf[naJets]/F");
    _outTree->Branch("aJet_cmva", aJets.cmva, "cmva[naJets]/F");
    _outTree->Branch("aJet_cosTheta", aJets.cosTheta, "cosTheta[naJets]/F");
    _outTree->Branch("aJet_numTracksSV", aJets.numTracksSV, "numTracksSV[naJets]/I");
    _outTree->Branch("aJet_chf", aJets.chf, "chf[naJets]/F");
    _outTree->Branch("aJet_nhf", aJets.nhf, "nhf[naJets]/F");
    _outTree->Branch("aJet_cef", aJets.cef, "cef[naJets]/F");
    _outTree->Branch("aJet_nef", aJets.nef, "nef[naJets]/F");
    _outTree->Branch("aJet_nch", aJets.nch, "nch[naJets]/F");
    _outTree->Branch("aJet_nconstituents", aJets.nconstituents, "nconstituents[naJets]/F");
    _outTree->Branch("aJet_flavour", aJets.flavour, "flavour[naJets]/F");
    _outTree->Branch("aJet_isSemiLept", aJets.isSemiLept, "isSemiLept[naJets]/I");
    _outTree->Branch("aJet_isSemiLeptMCtruth", aJets.isSemiLeptMCtruth, "isSemiLeptMCtruth[naJets]/I");
    _outTree->Branch("aJet_SoftLeptpdgId", aJets.SoftLeptpdgId, "SoftLeptpdgId[naJets]/I");
    _outTree->Branch("aJet_SoftLeptIdlooseMu", aJets.SoftLeptIdlooseMu, "SoftLeptIdlooseMu[naJets]/I");
    _outTree->Branch("aJet_SoftLeptId95", aJets.SoftLeptId95, "SoftLeptId95[naJets]/I");
    _outTree->Branch("aJet_SoftLeptPt", aJets.SoftLeptPt, "SoftLeptPt[naJets]/F");
    _outTree->Branch("aJet_SoftLeptdR", aJets.SoftLeptdR, "SoftLeptdR[naJets]/F");
    _outTree->Branch("aJet_SoftLeptptRel", aJets.SoftLeptptRel, "SoftLeptptRel[naJets]/F");
    _outTree->Branch("aJet_SoftLeptRelCombIso", aJets.SoftLeptRelCombIso, "SoftLeptRelCombIso[naJets]/F");
    _outTree->Branch("aJet_puJetIdL", aJets.puJetIdL, "puJetIdL[naJets]/F");
    _outTree->Branch("aJet_puJetIdM", aJets.puJetIdM, "puJetIdM[naJets]/F");
    _outTree->Branch("aJet_puJetIdT", aJets.puJetIdT, "puJetIdT[naJets]/F");
    _outTree->Branch("aJet_puJetIdMva", aJets.puJetIdMva, "puJetIdMva[naJets]/F");
    _outTree->Branch("aJet_charge", aJets.charge, "charge[naJets]/F");

    _outTree->Branch("aJet_genPt", aJets.genPt, "genPt[naJets]/F");
    _outTree->Branch("aJet_genEta", aJets.genEta, "genEta[naJets]/F");
    _outTree->Branch("aJet_genPhi", aJets.genPhi, "genPhi[naJets]/F");
    _outTree->Branch("aJet_JECUnc", aJets.JECUnc, "JECUnc[naJets]/F");
    _outTree->Branch("aJet_vtxMass", aJets.vtxMass, "vtxMass[naJets]/F");
    _outTree->Branch("aJet_vtx3dL", aJets.vtx3dL, "vtx3dL[naJets]/F");
    _outTree->Branch("aJet_vtx3deL", aJets.vtx3deL, "vtx3deL[naJets]/F");
    _outTree->Branch("aJet_id", aJets.id, "id[naJets]/b");
    _outTree->Branch("aJet_SF_CSVL", aJets.SF_CSVL, "SF_CSVL[naJets]/b");
    _outTree->Branch("aJet_SF_CSVM", aJets.SF_CSVM, "SF_CSVM[naJets]/b");
    _outTree->Branch("aJet_SF_CSVT", aJets.SF_CSVT, "SF_CSVT[naJets]/b");
    _outTree->Branch("aJet_SF_CSVLerr", aJets.SF_CSVLerr, "SF_CSVLerr[naJets]/b");
    _outTree->Branch("aJet_SF_CSVMerr", aJets.SF_CSVMerr, "SF_CSVMerr[naJets]/b");
    _outTree->Branch("aJet_SF_CSVTerr", aJets.SF_CSVTerr, "SF_CSVTerr[naJets]/b");

    _outTree->Branch("naJetsFat", &naJetsFat, "naJetsFat/I");
    _outTree->Branch("aJetFat_pt", aJetsFat.pt, "pt[naJetsFat]/F");
    _outTree->Branch("aJetFat_eta", aJetsFat.eta, "eta[naJetsFat]/F");
    _outTree->Branch("aJetFat_phi", aJetsFat.phi, "phi[naJetsFat]/F");
    _outTree->Branch("aJetFat_e", aJetsFat.e, "e[naJetsFat]/F");
    _outTree->Branch("aJetFat_csv", aJetsFat.csv, "csv[naJetsFat]/F");

    _outTree->Branch("numJets", &numJets, "numJets/I");
    _outTree->Branch("numBJets", &numBJets, "numBJets/I");
    _outTree->Branch("deltaPullAngle", &deltaPullAngle, "deltaPullAngle/F");
    _outTree->Branch("deltaPullAngle2", &deltaPullAngle2, "deltaPullAngle2/F");
    _outTree->Branch("gendrcc", &gendrcc, "gendrcc/F");
    _outTree->Branch("gendrbb", &gendrbb, "gendrbb/F");
    _outTree->Branch("genZpt", &genZpt, "genZpt/F");
    _outTree->Branch("genWpt", &genWpt, "genWpt/F");
    _outTree->Branch("genHpt", &genHpt, "genHpt/F");
    _outTree->Branch("weightTrig", &weightTrig, "weightTrig/F");
    _outTree->Branch("weightTrigMay", &weightTrigMay, "weightTrigMay/F");
    _outTree->Branch("weightTrigV4", &weightTrigV4, "weightTrigV4/F");
    _outTree->Branch("weightTrigMET", &weightTrigMET, "weightTrigMET/F");
    _outTree->Branch("weightTrigOrMu30", &weightTrigOrMu30, "weightTrigOrMu30/F");
    _outTree->Branch("weightEleRecoAndId", &weightEleRecoAndId, "weightEleRecoAndId/F");
    _outTree->Branch("weightEleTrigJetMETPart", &weightEleTrigJetMETPart, "weightEleTrigJetMETPart/F");
    _outTree->Branch("weightEleTrigElePart", &weightEleTrigElePart, "weightEleTrigElePart/F");
    _outTree->Branch("weightEleTrigEleAugPart", &weightEleTrigEleAugPart, "weightEleTrigEleAugPart/F");

    _outTree->Branch("weightTrigMET80", &weightTrigMET80, "weightTrigMET80/F");
    _outTree->Branch("weightTrigMET100", &weightTrigMET100, "weightTrigMET100/F");
    _outTree->Branch("weightTrig2CJet20", &weightTrig2CJet20, "weightTrig2CJet20/F");
    _outTree->Branch("weightTrigMET150", &weightTrigMET150, "weightTrigMET150/F");
    _outTree->Branch("weightTrigMET802CJet", &weightTrigMET802CJet, "weightTrigMET802CJet/F");
    _outTree->Branch("weightTrigMET1002CJet", &weightTrigMET1002CJet, "weightTrigMET1002CJet/F");
    _outTree->Branch("weightTrigMETLP", &weightTrigMETLP, "weightTrigMETLP/F");

    _outTree->Branch("weightTrig2012A", &weightTrig2012A, "weightTrig2012A/F");
    _outTree->Branch("weightTrig2012ADiMuon", &weightTrig2012ADiMuon, "weightTrig2012ADiMuon/F");
    _outTree->Branch("weightTrig2012ADiEle", &weightTrig2012ADiEle, "weightTrig2012ADiEle/F");
    _outTree->Branch("weightTrig2012ASingleMuon", &weightTrig2012ASingleMuon, "weightTrig2012ASingleMuon/F");
    _outTree->Branch("weightTrig2012ASingleEle", &weightTrig2012ASingleEle, "weightTrig2012ASingleEle/F");
    _outTree->Branch("weightTrig2012AMuonPlusWCandPt", &weightTrig2012AMuonPlusWCandPt, "weightTrig2012AMuonPlusWCandPt/F");

    _outTree->Branch("weightTrig2012", &weightTrig2012, "weightTrig2012/F");
    _outTree->Branch("weightTrig2012DiMuon", &weightTrig2012DiMuon, "weightTrig2012DiMuon/F");
    _outTree->Branch("weightTrig2012DiEle", &weightTrig2012DiEle, "weightTrig2012DiEle/F");
    _outTree->Branch("weightTrig2012SingleMuon", &weightTrig2012SingleMuon, "weightTrig2012SingleMuon/F");
    _outTree->Branch("weightTrig2012SingleEle", &weightTrig2012SingleEle, "weightTrig2012SingleEle/F");
    _outTree->Branch("weightTrig2012MuonPlusWCandPt", &weightTrig2012MuonPlusWCandPt, "weightTrig2012MuonPlusWCandPt/F");

    _outTree->Branch("weightTrig2012AB", &weightTrig2012AB, "weightTrig2012AB/F");
    _outTree->Branch("weightTrig2012ABDiMuon", &weightTrig2012ABDiMuon, "weightTrig2012ABDiMuon/F");
    _outTree->Branch("weightTrig2012ABDiEle", &weightTrig2012ABDiEle, "weightTrig2012ABDiEle/F");
    _outTree->Branch("weightTrig2012ABSingleMuon", &weightTrig2012ABSingleMuon, "weightTrig2012ABSingleMuon/F");
    _outTree->Branch("weightTrig2012ABSingleEle", &weightTrig2012ABSingleEle, "weightTrig2012ABSingleEle/F");
    _outTree->Branch("weightTrig2012ABMuonPlusWCandPt", &weightTrig2012ABMuonPlusWCandPt, "weightTrig2012ABMuonPlusWCandPt/F");

    _outTree->Branch("weightTrig2012DiJet30MHT80", &weightTrig2012DiJet30MHT80, "weightTrig2012DiJet30MHT80/F");
    _outTree->Branch("weightTrig2012PFMET150", &weightTrig2012PFMET150, "weightTrig2012PFMET150/F");
    _outTree->Branch("weightTrig2012SumpT100MET100", &weightTrig2012SumpT100MET100, "weightTrig2012SumpT100MET100/F");
    _outTree->Branch("weightTrig2012APFMET150orDiJetMET", &weightTrig2012APFMET150orDiJetMET, "weightTrig2012APFMET150orDiJetMET/F");
    _outTree->Branch("weightTrig2012BPFMET150orDiJetMET", &weightTrig2012BPFMET150orDiJetMET, "weightTrig2012BPFMET150orDiJetMET/F");
    _outTree->Branch("weightTrig2012CPFMET150orDiJetMET", &weightTrig2012CPFMET150orDiJetMET, "weightTrig2012CPFMET150orDiJetMET/F");

    _outTree->Branch("deltaPullAngleAK7", &deltaPullAngleAK7, "deltaPullAngleAK7/F");
    _outTree->Branch("deltaPullAngle2AK7", &deltaPullAngle2AK7, "deltaPullAngle2AK7/F");
    _outTree->Branch("PU0", &PU0, "PU0/F");
    _outTree->Branch("PUm1", &PUm1, "PUm1/F");
    _outTree->Branch("PUp1", &PUp1, "PUp1/F");
    _outTree->Branch("PUweight", &PUweight, "PUweight/F");
    _outTree->Branch("PUweightP", &PUweightP, "PUweightP/F");
    _outTree->Branch("PUweightM", &PUweightM, "PUweightM/F");
    _outTree->Branch("PUweightAB", &PUweightAB, "PUweightAB/F");
    _outTree->Branch("PUweight2011B", &PUweight2011B, "PUweight2011B/F");
    _outTree->Branch("PUweight1DObs", &PUweight1DObs, "PUweight1DObs/F");
    _outTree->Branch("eventFlav", &eventFlav, "eventFlav/I");

    _outTree->Branch("Vtype", &Vtype, "Vtype/I");
    _outTree->Branch("HVdPhi", &HVdPhi, "HVdPhi/F");
    _outTree->Branch("HVMass", &HVMass, "HVMass/F");
    _outTree->Branch("HMETdPhi", &HMETdPhi, "HMETdPhi/F");
    _outTree->Branch("VMt", &VMt, "VMt/F");

    _outTree->Branch("nvlep", &nvlep, "nvlep/I");
    _outTree->Branch("nalep", &nalep, "nalep/I");

    _outTree->Branch("vLepton_mass", vLeptons.mass, "mass[nvlep]/F");
    _outTree->Branch("vLepton_pt", vLeptons.pt, "pt[nvlep]/F");
    _outTree->Branch("vLepton_eta", vLeptons.eta, "eta[nvlep]/F");
    _outTree->Branch("vLepton_phi", vLeptons.phi, "phi[nvlep]/F");
    _outTree->Branch("vLepton_aodCombRelIso", vLeptons.aodCombRelIso, "aodCombRelIso[nvlep]/F");
    _outTree->Branch("vLepton_pfCombRelIso", vLeptons.pfCombRelIso, "pfCombRelIso[nvlep]/F");
    _outTree->Branch("vLepton_photonIso", vLeptons.photonIso, "photonIso[nvlep]/F");
    _outTree->Branch("vLepton_neutralHadIso", vLeptons.neutralHadIso, "neutralHadIso[nvlep]/F");
    _outTree->Branch("vLepton_chargedHadIso", vLeptons.chargedHadIso, "chargedHadIso[nvlep]/F");
    _outTree->Branch("vLepton_chargedPUIso", vLeptons.chargedPUIso, "chargedPUIso[nvlep]/F");
    _outTree->Branch("vLepton_particleIso", vLeptons.particleIso, "particleIso[nvlep]/F");
    _outTree->Branch("vLepton_dxy", vLeptons.dxy, "dxy[nvlep]/F");
    _outTree->Branch("vLepton_dz", vLeptons.dz, "dz[nvlep]/F");
    _outTree->Branch("vLepton_type", vLeptons.type, "type[nvlep]/I");
    _outTree->Branch("vLepton_id80", vLeptons.id80, "id80[nvlep]/F");
    _outTree->Branch("vLepton_id95", vLeptons.id95, "id95[nvlep]/F");
    _outTree->Branch("vLepton_vbtf", vLeptons.vbtf, "vbtf[nvlep]/F");
    _outTree->Branch("vLepton_id80NoIso", vLeptons.id80NoIso, "id80NoIso[nvlep]/F");
    _outTree->Branch("vLepton_genPt", vLeptons.genPt, "genPt[nvlep]/F");
    _outTree->Branch("vLepton_genEta", vLeptons.genEta, "genEta[nvlep]/F");
    _outTree->Branch("vLepton_genPhi", vLeptons.genPhi, "genPhi[nvlep]/F");
    _outTree->Branch("vLepton_charge", vLeptons.charge, "charge[nvlep]/F");
    _outTree->Branch("vLepton_pfCorrIso", vLeptons.pfCorrIso, "pfCorrIso[nvlep]/F");
    _outTree->Branch("vLepton_id2012tight", vLeptons.id2012tight, "id2012tight[nvlep]/F");
    _outTree->Branch("vLepton_idMVAnotrig", vLeptons.idMVAnotrig, "idMVAnotrig[nvlep]/F");
    _outTree->Branch("vLepton_idMVAtrig", vLeptons.idMVAtrig, "idMVAtrig[nvlep]/F");
    _outTree->Branch("vLepton_idMVApresel", vLeptons.idMVApresel, "idMVApresel[nvlep]/F");
    _outTree->Branch("vLepton_innerHits", vLeptons.innerHits, "innerHits[nvlep]/F");
    _outTree->Branch("vLepton_photonIsoDoubleCount", vLeptons.photonIsoDoubleCount, "photonIsoDoubleCount[nvlep]/F");
    _outTree->Branch("vLepton_wpHWW", vLeptons.wpHWW, "wpHWW[nvlep]/F");
    _outTree->Branch("vLepton_wp95", vLeptons.wp95, "wp95[nvlep]/F");
    _outTree->Branch("vLepton_wp90", vLeptons.wp90, "wp90[nvlep]/F");
    _outTree->Branch("vLepton_wp85", vLeptons.wp85, "wp85[nvlep]/F");
    _outTree->Branch("vLepton_wp80", vLeptons.wp80, "wp80[nvlep]/F");
    _outTree->Branch("vLepton_wp70", vLeptons.wp70, "wp70[nvlep]/F");

    _outTree->Branch("aLepton_mass", aLeptons.mass, "mass[nalep]/F");
    _outTree->Branch("aLepton_pt", aLeptons.pt, "pt[nalep]/F");
    _outTree->Branch("aLepton_eta", aLeptons.eta, "eta[nalep]/F");
    _outTree->Branch("aLepton_phi", aLeptons.phi, "phi[nalep]/F");
    _outTree->Branch("aLepton_aodCombRelIso", aLeptons.aodCombRelIso, "aodCombRelIso[nalep]/F");
    _outTree->Branch("aLepton_pfCombRelIso", aLeptons.pfCombRelIso, "pfCombRelIso[nalep]/F");
    _outTree->Branch("aLepton_photonIso", aLeptons.photonIso, "photonIso[nalep]/F");
    _outTree->Branch("aLepton_neutralHadIso", aLeptons.neutralHadIso, "neutralHadIso[nalep]/F");
    _outTree->Branch("aLepton_chargedHadIso", aLeptons.chargedHadIso, "chargedHadIso[nalep]/F");
    _outTree->Branch("aLepton_chargedPUIso", aLeptons.chargedPUIso, "chargedPUIso[nalep]/F");
    _outTree->Branch("aLepton_particleIso", aLeptons.particleIso, "particleIso[nalep]/F");
    _outTree->Branch("aLepton_dxy", aLeptons.dxy, "dxy[nalep]/F");
    _outTree->Branch("aLepton_dz", aLeptons.dz, "dz[nalep]/F");
    _outTree->Branch("aLepton_type", aLeptons.type, "type[nalep]/I");
    _outTree->Branch("aLepton_id80", aLeptons.id80, "id80[nalep]/F");
    _outTree->Branch("aLepton_id95", aLeptons.id95, "id95[nalep]/F");
    _outTree->Branch("aLepton_vbtf", aLeptons.vbtf, "vbtf[nalep]/F");
    _outTree->Branch("aLepton_id80NoIso", aLeptons.id80NoIso, "id80NoIso[nalep]/F");
    _outTree->Branch("aLepton_genPt", aLeptons.genPt, "genPt[nalep]/F");
    _outTree->Branch("aLepton_genEta", aLeptons.genEta, "genEta[nalep]/F");
    _outTree->Branch("aLepton_genPhi", aLeptons.genPhi, "genPhi[nalep]/F");
    _outTree->Branch("aLepton_charge", aLeptons.charge, "charge[nalep]/F");
    _outTree->Branch("aLepton_pfCorrIso", aLeptons.pfCorrIso, "pfCorrIso[nalep]/F");
    _outTree->Branch("aLepton_id2012tight", aLeptons.id2012tight, "id2012tight[nalep]/F");
    _outTree->Branch("aLepton_idMVAnotrig", aLeptons.idMVAnotrig, "idMVAnotrig[nalep]/F");
    _outTree->Branch("aLepton_idMVAtrig", aLeptons.idMVAtrig, "idMVAtrig[nalep]/F");
    _outTree->Branch("aLepton_idMVApresel", aLeptons.idMVApresel, "idMVApresel[nalep]/F");
    _outTree->Branch("aLepton_innerHits", aLeptons.innerHits, "innerHits[nalep]/F");
    _outTree->Branch("aLepton_photonIsoDoubleCount", aLeptons.photonIsoDoubleCount, "photonIsoDoubleCount[nalep]/F");
    _outTree->Branch("aLepton_wpHWW", aLeptons.wpHWW, "wpHWW[nalep]/F");
    _outTree->Branch("aLepton_wp95", aLeptons.wp95, "wp95[nalep]/F");
    _outTree->Branch("aLepton_wp90", aLeptons.wp90, "wp90[nalep]/F");
    _outTree->Branch("aLepton_wp85", aLeptons.wp85, "wp85[nalep]/F");
    _outTree->Branch("aLepton_wp80", aLeptons.wp80, "wp80[nalep]/F");
    _outTree->Branch("aLepton_wp70", aLeptons.wp70, "wp70[nalep]/F");

    _outTree->Branch("top", &top, "mass/F:pt/F:wMass/F");
    _outTree->Branch("WplusMode", &WplusMode, "WplusMode/I");
    _outTree->Branch("WminusMode", &WminusMode, "WminusMode/I");

    // IVF
    _outTree->Branch("nSvs", &nSvs, "nSvs/I");
    _outTree->Branch("Sv_massBCand", &IVF.massBcand, "massBcand[nSvs]/F");
    _outTree->Branch("Sv_massSv", &IVF.massSv, "massSv[nSvs]/F");
    _outTree->Branch("Sv_pt", &IVF.pt, "pt[nSvs]/F");
    _outTree->Branch("Sv_eta", &IVF.eta, "eta[nSvs]/F");
    _outTree->Branch("Sv_phi", &IVF.phi, "phi[nSvs]/F");
    _outTree->Branch("Sv_dist3D", &IVF.dist3D, "dist3D[nSvs]/F");
    _outTree->Branch("Sv_dist2D", &IVF.dist2D, "dist2D[nSvs]/F");
    _outTree->Branch("Sv_distSim2D", &IVF.distSig2D, "distSig2D[nSvs]/F");
    _outTree->Branch("Sv_distSig3D", &IVF.distSig3D, "distSig3D[nSvs]/F");
    _outTree->Branch("Sv_dist3D_norm", &IVF.dist3D_norm, "dist3D_norm[nSvs]/F");
    // IVF Higgs candidate
    _outTree->Branch("SVH", &SVH, "mass/F:pt/F:eta/F:phi/F:dR/F:dPhi/F:dEta/F");

    // SimBHadron
    _outTree->Branch("nSimBs", &nSimBs, "nSimBs/I");
    _outTree->Branch("SimBs_mass", &SimBs.mass, "mass[nSimBs]/F");
    _outTree->Branch("SimBs_pt", &SimBs.pt, "pt[nSimBs]/F");
    _outTree->Branch("SimBs_eta", &SimBs.eta, "eta[nSimBs]/F");
    _outTree->Branch("SimBs_phi", &SimBs.phi, "phi[nSimBs]/F");
    _outTree->Branch("SimBs_vtx_x", &SimBs.vtx_x, "vtx_x[nSimBs]/F");
    _outTree->Branch("SimBs_vtx_y", &SimBs.vtx_y, "vtx_y[nSimBs]/F");
    _outTree->Branch("SimBs_vtx_z", &SimBs.vtx_z, "vtx_z[nSimBs]/F");
    _outTree->Branch("SimBs_pdgId", &SimBs.pdgId, "pdgId[nSimBs]/F");
    _outTree->Branch("SimBs_status", &SimBs.status, "status[nSimBs]/F");
    // SimBHadron Higgs Candidate
    _outTree->Branch("SimBsH", &SimBsH, "mass/F:pt/F:eta/F:phi/F:dR/F:dPhi/F:dEta/F");

    _outTree->Branch("rho", &rho, "rho/F");
    _outTree->Branch("rho25", &rho25, "rho25/F");
    _outTree->Branch("rhoN", &rhoN, "rhoN/F");
    _outTree->Branch("nPVs", &nPVs, "nPVs/I");
    _outTree->Branch("nGoodPVs", &nGoodPVs, "nGoodPVs/I");
    _outTree->Branch("MET", &MET, "et/F:sumet/F:sig/F:phi/F");
    _outTree->Branch("fakeMET", &fakeMET, "et/F:sumet/F:sig/F:phi/F");
    _outTree->Branch("METtype1corr", &METtype1corr, "et/F:sumet/F:sig/F:phi/F");
    _outTree->Branch("METtype1p2corr", &METtype1p2corr, "et/F:sumet/F:sig/F:phi/F");
    _outTree->Branch("METnoPU", &METnoPU, "et/F:sumet/F:sig/F:phi/F");
    _outTree->Branch("METnoPUCh", &METnoPUCh, "et/F:sumet/F:sig/F:phi/F");
    _outTree->Branch("caloMET", &caloMET, "et/F:sumet/F:sig/F:phi/F");
    _outTree->Branch("MHT", &MHT, "mht/F:ht/F:sig/F:phi/F");
    _outTree->Branch("metUnc_et", &metUnc.et, "et[12]/F");
    _outTree->Branch("metUnc_phi", &metUnc.phi, "phi[12]/F");
    _outTree->Branch("metUnc_sumet", &metUnc.sumet, "sumet[12]/F");

    _outTree->Branch("minDeltaPhijetMET", &minDeltaPhijetMET, "minDeltaPhijetMET/F");
    _outTree->Branch("jetPt_minDeltaPhijetMET", &jetPt_minDeltaPhijetMET, "jetPt_minDeltaPhijetMET/F");

    std::stringstream s;
    s << "triggerFlags[" << triggers.size() << "]/b";
    _outTree->Branch("triggerFlags", triggerFlags, s.str().c_str());

    _outTree->Branch("EVENT", &EVENT, "run/I:lumi/I:event/I:json/I");
    _outTree->Branch("hbhe", &hbhe, "hbhe/b");
    _outTree->Branch("ecalFlag", &ecalFlag, "ecalFlag/b");
    _outTree->Branch("cschaloFlag", &cschaloFlag, "cschaloFlag/b");
    _outTree->Branch("hcallaserFlag", &hcallaserFlag, "hcallaserFlag/b");
    _outTree->Branch("trackingfailureFlag", &trackingfailureFlag, "trackingfailureFlag/b");
    _outTree->Branch("eebadscFlag", &eebadscFlag, "eebadscFlag/b");
    _outTree->Branch("btag1TSF", &btag1TSF, "btag1TSF/F");
    _outTree->Branch("btag2TSF", &btag2TSF, "btag2TSF/F");
    _outTree->Branch("btag1T2CSF", &btag1T2CSF, "btag1T2CSF/F");
    _outTree->Branch("btag2CSF", &btag2CSF, "btag2CSF/F");
    _outTree->Branch("btagA0CSF", &btagA0CSF, "btagA0CSF/F");
    _outTree->Branch("btagA0TSF", &btagA0TSF, "btagA0TSF/F");
    _outTree->Branch("btag1TA1C", &btag1TA1C, "btag1TA1C/F");


    // -------------------------------------------------------------------------
    // Analysis
    // -------------------------------------------------------------------------
    int ievt = 0;
    int jevt = 0;
    TStopwatch sw;
    sw.Start();

    for (unsigned int iFile = 0; iFile < inputFiles_.size(); ++iFile) {
        std::cout << "Initiating request to open file " << iFile << std::endl;
        std::cout << "Events: " << ievt << std::endl;
        std::cout << "Saved : " << jevt << std::endl;
        TFile *inFile = TFile::Open(inputFiles_[iFile].c_str());
        if (inFile == 0) {
            std::cout << "Failed to open " << inputFiles_[iFile] << std::endl;
            continue;
        } else {
            std::cout << "Successfully opened file " << inputFiles_[iFile] << std::endl;
        }
        
        fwlite::Event ev(inFile);
        fwlite::Handle < VHbbEventAuxInfo > vhbbAuxHandle;
        
        /// Loop over events
        for (ev.toBegin(); !ev.atEnd(); ++ev, ++ievt) {
            if (ievt <= skipEvents_)  continue;
            if ((maxEvents_ >= 0) && (ievt > maxEvents_ + skipEvents_))  break;

            vhbbAuxHandle.getByLabel(ev, "HbbAnalyzerNew", 0, 0);
            const VHbbEventAuxInfo & aux = *(vhbbAuxHandle.product());
            EVENT.run = ev.id().run();
            EVENT.lumi = ev.id().luminosityBlock();
            EVENT.event = ev.id().event();
            EVENT.json = jsonContainsEvent(jsonVector, ev);
            if (EVENT.run < runMin_ && runMin_ > 0)  continue;
            if (EVENT.run > runMax_ && runMax_ > 0)  continue;
            count->Fill(1.);

            /// Higgs Candidate Selection
            const std::vector < VHbbCandidate > * candZ;
            const std::vector < VHbbCandidate > * candW;
            std::vector < VHbbCandidate > * candZlocal = new std::vector < VHbbCandidate >;
            std::vector < VHbbCandidate > * candWlocal = new std::vector < VHbbCandidate >;
            
            VHbbEvent modifiedEvent;
            const VHbbEvent * iEvent = 0;
            
            if (fromCandidate) {  // legacy, not in use
                //candZlocal->clear();
                //candWlocal->clear();
            
                fwlite::Handle < std::vector < VHbbCandidate > > vhbbCandHandleZ;
                vhbbCandHandleZ.getByLabel(ev, "hbbBestCSVPt20Candidates");
                candZ = vhbbCandHandleZ.product();

                fwlite::Handle < std::vector < VHbbCandidate > > vhbbCandHandle;
                vhbbCandHandle.getByLabel(ev, "hbbHighestPtHiggsPt30Candidates");
                candW = vhbbCandHandle.product();
                
            } else {
                candZlocal->clear();
                candWlocal->clear();
                fwlite::Handle < VHbbEvent > vhbbHandle;
                vhbbHandle.getByLabel(ev, "HbbAnalyzerNew");
                modifiedEvent = *vhbbHandle.product();
                
                if (isMC_) {
                    iEvent = &modifiedEvent;

                    for (size_t j = 0; j < modifiedEvent.simpleJets2.size(); j++) {
                        //VHbbEvent::SimpleJet orig = modifiedEvent.simpleJets2[j];
                        //VHbbEvent::SimpleJet origRemade = jec.correct(modifiedEvent.simpleJets2[j], aux.puInfo.rho, true, true); // do ref check, can be commented out 
                        //VHbbEvent::SimpleJet corr2011 = jec.correctRight(modifiedEvent.simpleJets2[j], aux.puInfo.rho, true, true); // do ref check, can be commented out 
                        /// Use correctRight() or correct()?
                        //modifiedEvent.simpleJets2[j] = jec.correct(modifiedEvent.simpleJets2[j], aux.puInfo.rho, true); 
                        //std::cout << "Original " << orig.p4.Pt() << " == " << origRemade.p4.Pt() << " using CHS2011 " << corr2011.p4.Pt() << " final: " << modifiedEvent.simpleJets2[j].p4.Pt() << std::endl;
                        
                        /// Smear jets to match data resolution
                        TLorentzVector & p4 = modifiedEvent.simpleJets2[j].p4;
                        const TLorentzVector & mcp4 = modifiedEvent.simpleJets2[j].bestMCp4;
                        if ((fabs(p4.Pt()-mcp4.Pt())/p4.Pt()) < 0.5) {  // limit the effect to the core 
                            float cor = (p4.Pt() + resolutionBias(fabs(p4.Eta()))*(p4.Pt()-mcp4.Pt()))/p4.Pt();
                            p4.SetPtEtaPhiE(p4.Pt()*cor, p4.Eta(), p4.Phi(), p4.E()*cor);
                        }
                    }
                
                } else {
                    iEvent = &modifiedEvent;

                    //for (size_t j = 0; j < modifiedEvent.simpleJets2.size(); j++) {
                        /// Use correctRight() or correct()?
                    //  modifiedEvent.simpleJets2[j] = jec.correct(modifiedEvent.simpleJets2[j], aux.puInfo.rho, false); 
                    //}
                }

                algoZ->run(iEvent, *candZlocal, aux);
                algoW->run(iEvent, *candWlocal, aux);

                /// Recover low pt
                if (candZlocal->size() == 0 || candZlocal->at(0).H.jets.size() < 2)
                {
                    candZlocal->clear();
                    algoRecoverLowPt->run(iEvent, *candZlocal, aux);
                }
                if (candWlocal->size() == 0 || candWlocal->at(0).H.jets.size() < 2)
                {
                    candWlocal->clear();
                    algoRecoverLowPt->run(iEvent, *candWlocal, aux);
                }
                
                candZ = candZlocal;
                candW = candWlocal;
            }  // end if not fromCandidate

            /// Start from candZ
            const std::vector < VHbbCandidate > * cand = candZ;
            bool isW = false;
            //if(cand->size() == 0 || cand->at(0).H.jets.size() < 2)  continue;
            if (cand->size() == 0)  continue;
            //std::cout << "cand->size() " << cand->size() << std::endl;
            //std::cout << "cand->at(0).H.jets.size() " << cand->at(0).H.jets.size() << std::endl;
            
            if (cand->size() > 1) {
                std::cout << "MULTIPLE CANDIDATES: " << cand->size() << std::endl;
            }
            
            /// Switch to candW if the first candidate is Wmun or Wen
            if (cand->at(0).candidateType == VHbbCandidate::Wmun || cand->at(0).candidateType == VHbbCandidate::Wen) {
                cand = candW;
                isW = true;
            }
            if (cand->size() == 0) {
                //std::cout << "W event loss due to tigther cuts" << std::endl;
                continue;
            }
            
            /// The VHbb Candidate
            /// Note that only the first one is used.
            const VHbbCandidate & vhCand = cand->at(0);
            Vtype = vhCand.candidateType;
            if (vhCand.H.HiggsFlag) H.HiggsFlag = 1;  else H.HiggsFlag = 0;
            if (vhCand.FatH.FatHiggsFlag) FatH.FatHiggsFlag = 1;  else FatH.FatHiggsFlag = 0;
                        
            /// FIXME: Skip events if neither HiggsFlag nor FatHiggsFlag is true?
            //if (H.HiggsFlag != 1 && FatH.FatHiggsFlag != 1)  continue;

            /// Dijet H
            hJets.reset();
            aJets.reset();
            if (vhCand.H.HiggsFlag) {
                H.mass = vhCand.H.p4.M();
                H.pt = vhCand.H.p4.Pt();
                H.eta = vhCand.H.p4.Eta();
                H.phi = vhCand.H.p4.Phi();

                nhJets = 2;
                hJets.set(vhCand.H.jets[0], 0);
                hJets.set(vhCand.H.jets[1], 1);

                naJets = vhCand.additionalJets.size();
                numBJets = 0;
                if (vhCand.H.jets[0].csv > btagThr)
                    numBJets++;
                if (vhCand.H.jets[1].csv > btagThr)
                    numBJets++;
                for (int j = 0; j < naJets && j < MAXJ; j++) {
                    aJets.set(vhCand.additionalJets[j], j);
                    if (vhCand.additionalJets[j].csv > btagThr)
                        numBJets++;
                }
                numJets = vhCand.additionalJets.size() + 2;
                
            	H.dR = reco::deltaR(vhCand.H.jets[0].p4.Eta(), vhCand.H.jets[0].p4.Phi(), vhCand.H.jets[1].p4.Eta(), vhCand.H.jets[1].p4.Phi());
	            H.dPhi = reco::deltaPhi(vhCand.H.jets[0].p4.Phi(),vhCand.H.jets[1].p4.Phi());
	            H.dEta = TMath::Abs(vhCand.H.jets[0].p4.Eta() - vhCand.H.jets[1].p4.Eta());
	            HVdPhi = fabs(reco::deltaPhi(vhCand.H.p4.Phi(), vhCand.V.p4.Phi()) );
	            HVMass = (vhCand.H.p4 + vhCand.V.p4).M();
	            HMETdPhi = fabs(reco::deltaPhi(vhCand.H.p4.Phi(), vhCand.V.mets.at(0).p4.Phi()) );

                deltaPullAngle  = VHbbCandidateTools::getDeltaTheta(vhCand.H.jets[0],vhCand.H.jets[1]);
                deltaPullAngle2 = VHbbCandidateTools::getDeltaTheta(vhCand.H.jets[1],vhCand.H.jets[0]);
                hJets.cosTheta[0] = vhCand.H.helicities[0];
                hJets.cosTheta[1] = vhCand.H.helicities[1];
            }  // end if HiggsFlag
            
            /// Fatjet H
            fathFilterJets.reset();
            aJetsFat.reset();
            if (vhCand.FatH.FatHiggsFlag) {
                FatH.mass = vhCand.FatH.p4.M();
                FatH.pt = vhCand.FatH.p4.Pt();
                FatH.eta = vhCand.FatH.p4.Eta();
                FatH.phi = vhCand.FatH.p4.Phi();

                nfathFilterJets = vhCand.FatH.subjetsSize;
                for (int j = 0; j < nfathFilterJets; j++) {
                    fathFilterJets.set(vhCand.FatH.jets[j], j);
                }

                if (nfathFilterJets == 2) {
                    FatH.filteredmass = (vhCand.FatH.jets[0].p4 + vhCand.FatH.jets[1].p4).M();
                    FatH.filteredpt = (vhCand.FatH.jets[0].p4 + vhCand.FatH.jets[1].p4).Pt();
                    FatH.filteredeta = (vhCand.FatH.jets[0].p4 + vhCand.FatH.jets[1].p4).Eta();
                    FatH.filteredphi = (vhCand.FatH.jets[0].p4 + vhCand.FatH.jets[1].p4).Phi();
                    fathFilterJets.cosTheta[0] = vhCand.FatH.helicities[0];
                    fathFilterJets.cosTheta[1] = vhCand.FatH.helicities[1];
                } else if (nfathFilterJets == 3) {
                    FatH.filteredmass = (vhCand.FatH.jets[0].p4 + vhCand.FatH.jets[1].p4 + vhCand.FatH.jets[2].p4).M();
                    FatH.filteredpt = (vhCand.FatH.jets[0].p4 + vhCand.FatH.jets[1].p4 + vhCand.FatH.jets[2].p4).Pt();
                    FatH.filteredeta = (vhCand.FatH.jets[0].p4 + vhCand.FatH.jets[1].p4 + vhCand.FatH.jets[2].p4).Eta();
                    FatH.filteredphi = (vhCand.FatH.jets[0].p4 + vhCand.FatH.jets[1].p4 + vhCand.FatH.jets[2].p4).Phi();
                    fathFilterJets.cosTheta[0] = vhCand.FatH.helicities[0];
                    fathFilterJets.cosTheta[1] = vhCand.FatH.helicities[1];
                    fathFilterJets.cosTheta[2] = vhCand.FatH.helicities[2];
                }

                naJetsFat = vhCand.additionalJetsFat.size();
                for (int j = 0; j < naJetsFat && j < MAXJ; j++) {
                    aJetsFat.set(vhCand.additionalJetsFat[j], j);
                }
            }  // end if FatHiggsFlag

            /// Associated V
            V.mass = vhCand.V.p4.M();
            if (isW)  V.mass = vhCand.Mt();
            V.pt = vhCand.V.p4.Pt();
            V.eta = vhCand.V.p4.Eta();
            V.phi = vhCand.V.p4.Phi();
            VMt = vhCand.Mt();
            

            /// Pileup Reweighting
            PUweight = 1.0;
            PUweightP = 1.0;
            PUweightM = 1.0;
            PUweightAB = 1.0;
            PUweight2011B = 1.0;
            PUweight1DObs = 1.0;
            if (isMC_) {
                std::map < int, unsigned int >::const_iterator puit = aux.puInfo.pus.find(0);
                int npu = puit->second;
                PUweight = lumiWeights.weight(aux.puInfo.truePU);     // use new method with "true PU"
                PUweightP = lumiWeightsPl.weight(aux.puInfo.truePU);  // use new method with "true PU"
                PUweightM = lumiWeightsMi.weight(aux.puInfo.truePU);  // use new method with "true PU"
                PUweightAB = lumiWeightsAB.weight(aux.puInfo.truePU); // use new method with "true PU"
                pu->Fill(puit->second);

                /// PU weights Run2011B
                std::map < int, unsigned int >::const_iterator puit0 = aux.puInfo.pus.find(0);
                std::map < int, unsigned int >::const_iterator puitm1 = aux.puInfo.pus.find(-1);
                std::map < int, unsigned int >::const_iterator puitp1 = aux.puInfo.pus.find(+1);
                PU0 = puit0->second;
                PUp1 = puitp1->second;
                PUm1 = puitm1->second;
                input3DPU->Fill(PUm1, PU0, PUp1);
                //PUweight2011B = lumiWeights2011B.weight3D(puitm1->second, puit0->second,puitp1->second);
                PUweight1DObs = lumiWeights1DObs.weight(npu);
            }
            countWithPU->Fill(1.0, PUweight);
            countWithPUP->Fill(1.0, PUweightP);
            countWithPUM->Fill(1.0, PUweightM);
            countWithPUAB->Fill(1.0, PUweightAB);
            countWithPU2011B->Fill(1.0, PUweight2011B);
            
            /// Energy densities
            rho = aux.puInfo.rho;
            rho25 = aux.puInfo.rho25;
            rhoN = aux.puInfo.rhoNeutral;
            nPVs = aux.pvInfo.nVertices;
            //nGoodPVs = aux.pvInfo.nGoodVertices;

            /// LHE Info
            fwlite::Handle < LHEEventProduct > evt;
            //std::cout << "Label for lhe = " << evt.getBranchNameFor(ev,"source") << std::endl;
            if (!((evt.getBranchNameFor(ev, "source")).empty())) {
                evt.getByLabel(ev, "source");
                //std::cout << "LHEEventProduct found!" << std::endl;
                bool lCheck = false;
                bool lbarCheck = false;
                bool vlCheck = false;
                bool vlbarCheck = false;
                //int idl, idlbar;
                lheHT = 0.;
                lheNj = 0;
                TLorentzVector l, lbar, vl, vlbar, V_tlv;
                const lhef::HEPEUP hepeup_ = evt->hepeup();
                const std::vector < lhef::HEPEUP::FiveVector > pup_ = hepeup_.PUP; // px, py, pz, E, M
                for (unsigned int i = 0; i < pup_.size(); ++i) {
                    int id = hepeup_.IDUP[i];  // pdgId
                    int status = hepeup_.ISTUP[i];
                    int idabs = TMath::Abs(id);
                    if (status == 1 && ((idabs == 21) || (idabs > 0 && idabs < 7))) {   // gluons and quarks
                        lheHT += TMath::Sqrt(TMath::Power(hepeup_.PUP[i][0], 2) + TMath::Power(hepeup_.PUP[i][1], 2));  // first entry is px, second py
                        lheNj++;
                    }
                    if (id == 11) {
                        l.SetPxPyPzE(hepeup_.PUP[i][0], hepeup_.PUP[i][1], hepeup_.PUP[i][2], hepeup_.PUP[i][3]);
                        lCheck = true;
                    }
                    if (id == -11) {
                        lbar.SetPxPyPzE(hepeup_.PUP[i][0], hepeup_.PUP[i][1], hepeup_.PUP[i][2], hepeup_.PUP[i][3]);
                        lbarCheck = true;
                    }
                    if (id == 12) {
                        vl.SetPxPyPzE(hepeup_.PUP[i][0], hepeup_.PUP[i][1], hepeup_.PUP[i][2], hepeup_.PUP[i][3]);
                        vlCheck = true;
                    }
                    if (id == -12) {
                        vlbar.SetPxPyPzE(hepeup_.PUP[i][0], hepeup_.PUP[i][1], hepeup_.PUP[i][2], hepeup_.PUP[i][3]);
                        vlbarCheck = true;
                    }

                    if (id == 13) {
                        l.SetPxPyPzE(hepeup_.PUP[i][0], hepeup_.PUP[i][1], hepeup_.PUP[i][2], hepeup_.PUP[i][3]);
                        lCheck = true;
                    }
                    if (id == -13) {
                        lbar.SetPxPyPzE(hepeup_.PUP[i][0], hepeup_.PUP[i][1], hepeup_.PUP[i][2], hepeup_.PUP[i][3]);
                        lbarCheck = true;
                    }
                    if (id == 14) {
                        vl.SetPxPyPzE(hepeup_.PUP[i][0], hepeup_.PUP[i][1], hepeup_.PUP[i][2], hepeup_.PUP[i][3]);
                        vlCheck = true;
                    }
                    if (id == -14) {
                        vlbar.SetPxPyPzE(hepeup_.PUP[i][0], hepeup_.PUP[i][1], hepeup_.PUP[i][2], hepeup_.PUP[i][3]);
                        vlbarCheck = true;
                    }
                    if (id == 15) {
                        l.SetPxPyPzE(hepeup_.PUP[i][0], hepeup_.PUP[i][1], hepeup_.PUP[i][2], hepeup_.PUP[i][3]);
                        lCheck = true;
                    }
                    if (id == -15) {
                        lbar.SetPxPyPzE(hepeup_.PUP[i][0], hepeup_.PUP[i][1], hepeup_.PUP[i][2], hepeup_.PUP[i][3]);
                        lbarCheck = true;
                    }
                    if (id == 16) {
                        vl.SetPxPyPzE(hepeup_.PUP[i][0], hepeup_.PUP[i][1], hepeup_.PUP[i][2], hepeup_.PUP[i][3]);
                        vlCheck = true;
                    }
                    if (id == -16) {
                        vlbar.SetPxPyPzE(hepeup_.PUP[i][0], hepeup_.PUP[i][1], hepeup_.PUP[i][2], hepeup_.PUP[i][3]);
                        vlbarCheck = true;
                    }
                }
                if (lCheck && lbarCheck)
                    V_tlv = l + lbar;   // Zll
                if (vlCheck && vlbarCheck)
                    V_tlv = vl + vlbar; // Znn
                if (lCheck && vlbarCheck)
                    V_tlv = l + vlbar;  // Wlnu
                if (lbarCheck && vlCheck)
                    V_tlv = lbar + vl;  // Wlnu
                lheV_pt = V_tlv.Pt(); // why not directly take from id==23 or 24?
            }
            //std::cout << "lhe V pt = " << lheV_pt << std::endl;

            /// Gen Info
            eventFlav = 0;
            if (aux.mcBbar.size() > 0 || aux.mcB.size() > 0)
                eventFlav = 5;
            else if (aux.mcC.size() > 0)
                eventFlav = 4;

            /// Triggers
            trigger.setEvent(&ev, "HLT", triggers);
            for (unsigned int j = 0; j < triggers.size(); j++)
                triggerFlags[j] = trigger.accept(triggers[j]);
            
            /// Filters
            patFilter.setEvent(&ev, "VH", patFilters);
            hbhe = patFilter.accept("hbhe");
            ecalFlag = patFilter.accept("ecalFilter");
            cschaloFlag = patFilter.accept("cschaloFilter");
            hcallaserFlag = patFilter.accept("hcallaserFilter");
            trackingfailureFlag = patFilter.accept("trackingfailureFilter");
            eebadscFlag = patFilter.accept("eebadscFilter");
            
            /// Track Sharing Flags
            TkSharing.HiggsCSVtkSharing = TkSharing.HiggsIPtkSharing = TkSharing.HiggsSVtkSharing = TkSharing.FatHiggsCSVtkSharing = TkSharing.FatHiggsIPtkSharing = TkSharing.FatHiggsSVtkSharing = false;

            if(vhCand.H.HiggsFlag){
              // csv tracks
              if (vhCand.H.jets[0].csvNTracks > 0 && vhCand.H.jets[1].csvNTracks > 0){
                for (int t=0;t!=vhCand.H.jets[0].csvNTracks;t++){
                  for (int ti=0;ti!=vhCand.H.jets[1].csvNTracks;ti++){
                    if ((int)vhCand.H.jets[0].csvTrackIds[t] == (int)vhCand.H.jets[1].csvTrackIds[ti]){
                      TkSharing.HiggsCSVtkSharing = true;
                    }// same trackID
                  }// loop tracks in second hjet
                }// loop tracks in first hjet
              }// if tracks in jet
  
              // ip tracks
              if (vhCand.H.jets[0].btagNTracks > 0 && vhCand.H.jets[1].btagNTracks > 0){
                for (int t=0;t!=vhCand.H.jets[0].btagNTracks;t++){
                  for (int ti=0;ti!=vhCand.H.jets[1].btagNTracks;ti++){
                    if ((int)vhCand.H.jets[0].btagTrackIds[t] == (int)vhCand.H.jets[1].btagTrackIds[ti]){
                      TkSharing.HiggsIPtkSharing = true;
                    }// same trackID
                  }// loop tracks in second hjet
                }// loop tracks in first hjet
              }// if tracks in jet
              
              // sv tracks
              if (vhCand.H.jets[0].vtxNTracks > 0 && vhCand.H.jets[1].vtxNTracks > 0){
                for (int t=0;t!=vhCand.H.jets[0].vtxNTracks;t++){
                  for (int ti=0;ti!=vhCand.H.jets[1].vtxNTracks;ti++){
                    if ((int)vhCand.H.jets[0].vtxTrackIds[t] == (int)vhCand.H.jets[1].vtxTrackIds[ti]){
                      TkSharing.HiggsSVtkSharing = true;
                    }// same trackID
                  }// loop tracks in second hjet
                }// loop tracks in first hjet
              }// if tracks in jet
            } // end if HiggsFlag
       
            if(vhCand.FatH.FatHiggsFlag && nfathFilterJets > 1){
              // csv tracks
              if (vhCand.FatH.jets[0].csvNTracks > 0 && vhCand.FatH.jets[1].csvNTracks > 0){
                for (int t=0;t!=vhCand.FatH.jets[0].csvNTracks;t++){
                  for (int ti=0;ti!=vhCand.FatH.jets[1].csvNTracks;ti++){
                    if ((int)vhCand.FatH.jets[0].csvTrackIds[t] == (int)vhCand.FatH.jets[1].csvTrackIds[ti]){
                      TkSharing.FatHiggsCSVtkSharing = true;
                    }// same trackID
                  }// loop tracks in second hjet
                }// loop tracks in first hjet
              }// if tracks in jet
              
              // ip tracks
              if (vhCand.FatH.jets[0].btagNTracks > 0 && vhCand.FatH.jets[1].btagNTracks > 0){
                for (int t=0;t!=vhCand.FatH.jets[0].btagNTracks;t++){
                  for (int ti=0;ti!=vhCand.FatH.jets[1].btagNTracks;ti++){
                    if ((int)vhCand.FatH.jets[0].btagTrackIds[t] == (int)vhCand.FatH.jets[1].btagTrackIds[ti]){
                      TkSharing.FatHiggsIPtkSharing = true;
                    }// same trackID
                  }// loop tracks in second hjet
                }// loop tracks in first hjet
              }// if tracks in jet
              
              // sv tracks
              if (vhCand.FatH.jets[0].vtxNTracks > 0 && vhCand.FatH.jets[1].vtxNTracks > 0){
                for (int t=0;t!=vhCand.FatH.jets[0].vtxNTracks;t++){
                  for (int ti=0;ti!=vhCand.FatH.jets[1].vtxNTracks;ti++){
                    if ((int)vhCand.FatH.jets[0].vtxTrackIds[t] == (int)vhCand.FatH.jets[1].vtxTrackIds[ti]){
                      TkSharing.FatHiggsSVtkSharing = true;
                    }// same trackID
                  }// loop tracks in second hjet
                }// loop tracks in first hjet
              }// if tracks in jet
            }// end if FatHiggsFlag
            
            /// IVF
            fwlite::Handle < std::vector < reco::Vertex > > SVC;
            SVC.getByLabel(ev, "bcandidates");
            const std::vector < reco::Vertex > svc = *(SVC.product());

            IVF.reset();
            nSvs = svc.size();
            const TVector3 recoPv = aux.pvInfo.firstPVInPT2;
            const math::XYZPoint myPv(recoPv);

            //FAKE ERROR MATRIX
// 	//look here for Matrix filling info http://project-mathlibs.web.cern.ch/project-mathlibs/sw/html/SMatrixDoc.html
// 	std::vector<double> fillMatrix(6);
// 	for (int i = 0; i<6; ++i) fillMatrix[i] = 0.;
// 	fillMatrix[0] = TMath::Power(0.002,2);
// 	fillMatrix[2] = TMath::Power(0.002,2);
// 	fillMatrix[5] = TMath::Power(0.002,2);
// 	const ROOT::Math::SMatrix<double, 3, 3, ROOT::Math::MatRepSym<double, 3> > myFakeMatrixError(fillMatrix.begin(),fillMatrix.end());
// 	const reco::Vertex recoVtxPv(myPv, myFakeMatrixError);

            //REAL ERROR MATRIX
            const reco::Vertex recoVtxPv(myPv, aux.pvInfo.efirstPVInPT2);
            for (int j = 0; j < nSvs && j < MAXB; ++j) {
                const GlobalVector flightDir = flightDirection(recoPv, svc[j]);
                reco::SecondaryVertex recoSv(recoVtxPv, svc[j], flightDir, true);
                IVF.set(recoSv, recoPv, j);
            }
            if (nSvs > 1) {
                TLorentzVector BCands_H1, BCands_H2, BCands_H;
                BCands_H1.SetPtEtaPhiM(IVF.pt[0], IVF.eta[0], IVF.phi[0], IVF.massBcand[0]);
                BCands_H2.SetPtEtaPhiM(IVF.pt[1], IVF.eta[1], IVF.phi[1], IVF.massBcand[1]);
                BCands_H = BCands_H1 + BCands_H2;
                SVH.dR = deltaR(IVF.eta[0], IVF.phi[0], IVF.eta[1], IVF.phi[1]);
                SVH.dPhi = deltaPhi(IVF.phi[0], IVF.phi[1]);
                SVH.dEta = TMath::Abs(IVF.eta[0] - IVF.eta[1]);
                SVH.mass = BCands_H.M();
                SVH.pt = BCands_H.Pt();
                SVH.eta = BCands_H.Eta();
                SVH.phi = BCands_H.Phi();
            }
            
            /// SimBHadron
            const SimBHadronCollection * sbhc;
            SimBs.reset();
            if (isMC_) {
                fwlite::Handle < SimBHadronCollection > SBHC;
                SBHC.getByLabel(ev, "bhadrons");
                sbhc = SBHC.product();
                nSimBs = sbhc->size();
                for (int j = 0; j < nSimBs && j < MAXB; ++j)
                    SimBs.set(sbhc->at(j), j);
                if (nSimBs > 1) {
                    TLorentzVector SimBs_H1, SimBs_H2, SimBs_H;
	                SimBs_H1.SetPtEtaPhiM(SimBs.pt[0], SimBs.eta[0], SimBs.phi[0], SimBs.mass[0]);
	                SimBs_H2.SetPtEtaPhiM(SimBs.pt[1], SimBs.eta[1], SimBs.phi[1], SimBs.mass[1]);
	                SimBs_H = SimBs_H1 + SimBs_H2;
	                SimBsH.dR = deltaR(SimBs.eta[0], SimBs.phi[0], SimBs.eta[1], SimBs.phi[1] );
	                SimBsH.dPhi = deltaPhi(SimBs.phi[0], SimBs.phi[1] );
	                SimBsH.dEta = TMath::Abs(SimBs.eta[0] - SimBs.eta[1] );
	                SimBsH.mass = SimBs_H.M();
	                SimBsH.pt = SimBs_H.Pt();
	                SimBsH.eta = SimBs_H.Eta();
	                SimBsH.phi = SimBs_H.Phi();
                }
            }
            
            
            /// MET
            MET.et = vhCand.V.mets.at(0).p4.Pt();  // default to Type-1 corrected MET
            MET.phi = vhCand.V.mets.at(0).p4.Phi();
            MET.sumet = vhCand.V.mets.at(0).sumEt;
            MET.sig = vhCand.V.mets.at(0).metSig;

            fakeMET.sumet = 0;
            fakeMET.sig = 0;
            fakeMET.et = 0;
            fakeMET.phi = 0;
            if (Vtype == VHbbCandidate::Zmumu) {
                TVector3 mu1 = vhCand.V.muons[0].p4.Vect();
                TVector3 mu2 = vhCand.V.muons[1].p4.Vect();
                TVector3 sum = vhCand.V.mets.at(0).p4.Vect() + mu1 + mu2;
                fakeMET.et = sum.Pt();
                fakeMET.phi = sum.Phi();
                fakeMET.sumet = vhCand.V.mets.at(0).sumEt - mu1.Pt() - mu2.Pt();
            }

            METtype1corr.et = iEvent->metType1.p4.Pt();
            METtype1corr.phi = iEvent->metType1.p4.Phi();
            METtype1corr.sumet = iEvent->metType1.sumEt;
            METtype1corr.sig = iEvent->metType1.metSig;

            METtype1p2corr.et = iEvent->metType1p2.p4.Pt();
            METtype1p2corr.phi = iEvent->metType1p2.p4.Phi();
            METtype1p2corr.sumet = iEvent->metType1p2.sumEt;
            METtype1p2corr.sig = iEvent->metType1p2.metSig;

            METnoPU.et = iEvent->pfmetNoPU.p4.Pt();
            METnoPU.phi = iEvent->pfmetNoPU.p4.Phi();
            METnoPU.sumet = iEvent->pfmetNoPU.sumEt;
            METnoPU.sig = iEvent->pfmetNoPU.metSig;
            
            METnoPUCh.et = iEvent->pfmetNoPUCh.p4.Pt();
            METnoPUCh.phi = iEvent->pfmetNoPUCh.p4.Phi();
            METnoPUCh.sumet = iEvent->pfmetNoPUCh.sumEt;
            METnoPUCh.sig = iEvent->pfmetNoPUCh.metSig;

            //caloMET.et = iEvent->calomet.p4.Pt();
            //caloMET.phi = iEvent->calomet.p4.Phi();
            //caloMET.sumet = iEvent->calomet.sumEt;
            //caloMET.sig = iEvent->calomet.metSig;

            MHT.mht = iEvent->mht.p4.Pt();
            MHT.phi = iEvent->mht.p4.Phi();
            MHT.ht = iEvent->mht.sumEt;
            MHT.sig = iEvent->mht.metSig;
            
            //std::cout << " iEvent->metUncInfo.size() " << iEvent->metUncInfo.size() << std::endl;
            for (size_t m = 0; m < iEvent->metUncInfo.size(); m++) {
                metUnc.set(iEvent->metUncInfo[m], m);
                //std::cout << "metUncInfo[" << m <<" ].et = " << metUnc.et[m] << std::endl; 
            }
            
            // Loop on jets
            double maxBtag = -999.;
            minDeltaPhijetMET = 999.;
            TLorentzVector bJet;  // what is this for?
            std::vector < std::vector < BTagWeight::JetInfo > > btagJetInfos;
            std::vector < float > jet10eta;
            std::vector < float > jet10pt;
            std::vector < float > jet30eta;
            std::vector < float > jet30pt;
            if (fromCandidate) {  // legacy, not in use
                // Loop on Higgs jets
                for (unsigned int j = 0; j < vhCand.H.jets.size(); j++) {
                    if (vhCand.H.jets[j].csv > maxBtag) {
                        bJet = vhCand.H.jets[j].p4;
                        maxBtag = vhCand.H.jets[j].csv;
                    }
                    if (fabs(deltaPhi(vhCand.V.mets.at(0).p4.Phi(), vhCand.H.jets[j].p4.Phi())) < minDeltaPhijetMET) {
                        minDeltaPhijetMET = fabs(deltaPhi(vhCand.V.mets.at(0).p4.Phi(), vhCand.H.jets[j].p4.Phi()));
                        jetPt_minDeltaPhijetMET = vhCand.H.jets[j].p4.Pt();
                    }
                    btagJetInfos.push_back(btagEff.jetInfo(vhCand.H.jets[j]));
                }
                // Loop on additional jets
                for (unsigned int j = 0; j < vhCand.additionalJets.size(); j++) {
                    if (vhCand.additionalJets[j].csv > maxBtag) {
                        bJet = vhCand.additionalJets[j].p4;
                        maxBtag = vhCand.additionalJets[j].csv;
                    }
                    //if (fabs(deltaPhi(vhCand.V.mets.at(0).p4.Phi(), vhCand.additionalJets[j].p4.Phi())) < minDeltaPhijetMET) {
                    //    minDeltaPhijetMET = fabs(deltaPhi(vhCand.V.mets.at(0).p4.Phi(), vhCand.additionalJets[j].p4.Phi()));
                    //    jetPt_minDeltaPhijetMET = vhCand.additionalJets[j].p4.Pt();
                    //}
                    if ((isW && !useHighestPtHiggsW) || (!isW && !useHighestPtHiggsZ)) {  // btag SF computed using only H-jets if best-H made with dijetPt rather than best CSV
                        if(vhCand.additionalJets[j].p4.Pt() > 20)
		                    btagJetInfos.push_back(btagEff.jetInfo(vhCand.additionalJets[j]));
                    }
                }
            
            } else {
                // Loop on all jets
                for (unsigned int j = 0; j < iEvent->simpleJets2.size(); j++) {
                    if (iEvent->simpleJets2[j].csv > maxBtag) {
                        bJet = iEvent->simpleJets2[j].p4;
                        maxBtag = iEvent->simpleJets2[j].csv;
                    }
                    /// Consider jet pT > 30, |eta| < 2.5
                    if (iEvent->simpleJets2[j].p4.Pt() > 30 && fabs(iEvent->simpleJets2[j].p4.Eta()) < 2.5 &&
                        fabs(reco::deltaPhi(vhCand.V.mets.at(0).p4.Phi(), iEvent->simpleJets2[j].p4.Phi())) < minDeltaPhijetMET) {
                        minDeltaPhijetMET = fabs(reco::deltaPhi(vhCand.V.mets.at(0).p4.Phi(), iEvent->simpleJets2[j].p4.Phi()));
                        jetPt_minDeltaPhijetMET = iEvent->simpleJets2[j].p4.Pt();
                    }
                    if (iEvent->simpleJets2[j].p4.Pt() > 10) {
                        jet10eta.push_back(iEvent->simpleJets2[j].p4.Eta());
                        jet10pt.push_back(iEvent->simpleJets2[j].p4.Pt());
                    }
                    if (iEvent->simpleJets2[j].p4.Pt() > 30) {
                        jet30eta.push_back(iEvent->simpleJets2[j].p4.Eta());
                        jet30pt.push_back(iEvent->simpleJets2[j].p4.Pt());
                    }
                    // For events made with highest CSV, all jets in the event should be taken into account for "tagging" SF (anti tagging is a mess)
                    // because for example a light jet not used for the Higgs can have in reality a higher CSV due to SF > 1 and become a higgs jet
                    if ((isW && !useHighestPtHiggsW) || (!isW && !useHighestPtHiggsZ)) {
                        if (iEvent->simpleJets2[j].p4.Pt() > 20 && fabs(iEvent->simpleJets2[j].p4.Eta()) < 2.5)
                            btagJetInfos.push_back(btagEff.jetInfo(iEvent->simpleJets2[j]));
                    }
                }

                // Loop on Higgs jets
                if (vhCand.H.HiggsFlag) {
                    for (unsigned int j = 0; j < vhCand.H.jets.size(); j++) {
                        // if we use the highest pt pair, only the two higgs jet should be used to compute the SF because the other jets are excluded 
                        // by a criteria (pt of the dijet) that is not btag SF dependent 
                        if (!((isW && !useHighestPtHiggsW) || (!isW && !useHighestPtHiggsZ))) {
                            btagJetInfos.push_back(btagEff.jetInfo(vhCand.H.jets[j]));
                        }
                    }
                }  // end if HiggsFlag
            }
            
            /// Trigger weights and set leptons
            vLeptons.reset();
            weightTrig = 1.;    // better to default to 1 
            weightTrigMay = -1.;
            weightTrigV4 = -1.;
            weightTrigOrMu30 = 1.;
            TLorentzVector leptonForTop;
            size_t firstAddMu = 0;
            size_t firstAddEle = 0;
            if (Vtype == VHbbCandidate::Zmumu) {
                vLeptons.set(vhCand.V.muons[0], 0, 13, aux);
                vLeptons.set(vhCand.V.muons[1], 1, 13, aux);
                float cweightID = triggerWeight.scaleMuID(vLeptons.pt[0],vLeptons.eta[0]) * triggerWeight.scaleMuID(vLeptons.pt[1],vLeptons.eta[1]) ;
                float weightTrig1 = triggerWeight.scaleMuIsoHLT(vLeptons.pt[0],vLeptons.eta[0]);
                float weightTrig2 = triggerWeight.scaleMuIsoHLT(vLeptons.pt[1],vLeptons.eta[1]);
                float cweightTrig = weightTrig1 + weightTrig2 - weightTrig1*weightTrig2;
                weightTrig = cweightID * cweightTrig;

                // 2012
                weightTrig2012DiMuon = triggerWeight.doubleMuon2012A(vLeptons.pt[0],vLeptons.eta[0],vLeptons.pt[1],vLeptons.eta[1]);
                float weightTrig2012SingleMuonMu1 = triggerWeight.singleMuon2012A(vLeptons.pt[0],vLeptons.eta[0]);         
                float weightTrig2012SingleMuonMu2 = triggerWeight.singleMuon2012A(vLeptons.pt[1],vLeptons.eta[1]);         
                weightTrig2012SingleMuon = weightTrig2012SingleMuonMu1+weightTrig2012SingleMuonMu2-weightTrig2012SingleMuonMu1*weightTrig2012SingleMuonMu2;
                weightTrig2012 = weightTrig2012SingleMuon * triggerWeight.muId2012A(vLeptons.pt[0],vLeptons.eta[0]) * triggerWeight.muId2012A(vLeptons.pt[1],vLeptons.eta[1]) ;

                weightTrig2012ABDiMuon = triggerWeightAB.doubleMuon2012A(vLeptons.pt[0],vLeptons.eta[0],vLeptons.pt[1],vLeptons.eta[1]);
                float weightTrig2012ABSingleMuonMu1 = triggerWeightAB.singleMuon2012A(vLeptons.pt[0],vLeptons.eta[0]);         
                float weightTrig2012ABSingleMuonMu2 = triggerWeightAB.singleMuon2012A(vLeptons.pt[1],vLeptons.eta[1]);         
                weightTrig2012ABSingleMuon = weightTrig2012ABSingleMuonMu1+weightTrig2012ABSingleMuonMu2-weightTrig2012ABSingleMuonMu1*weightTrig2012ABSingleMuonMu2;
                weightTrig2012AB = weightTrig2012ABSingleMuon * triggerWeightAB.muId2012A(vLeptons.pt[0],vLeptons.eta[0]) * triggerWeightAB.muId2012A(vLeptons.pt[1],vLeptons.eta[1]) ;

                weightTrig2012A = weightTrig2012;
                weightTrig2012ADiMuon = weightTrig2012DiMuon;
                weightTrig2012ASingleMuon = weightTrig2012SingleMuon;

                nvlep = 2;
                firstAddMu = 2;
            }
            
            if (Vtype == VHbbCandidate::Zee) {
                vLeptons.set(vhCand.V.electrons[0], 0, 11, aux);
                vLeptons.set(vhCand.V.electrons[1], 1, 11, aux);
                std::vector < float > pt, eta;
                pt.push_back(vLeptons.pt[0]);
                eta.push_back(vLeptons.eta[0]);
                pt.push_back(vLeptons.pt[1]);
                eta.push_back(vLeptons.eta[1]);
                weightEleRecoAndId=triggerWeight.scaleID95Ele(vLeptons.pt[0],vLeptons.eta[0]) * triggerWeight.scaleRecoEle(vLeptons.pt[0],vLeptons.eta[0]) *
                    triggerWeight.scaleID95Ele(vLeptons.pt[1],vLeptons.eta[1]) * triggerWeight.scaleRecoEle(vLeptons.pt[1],vLeptons.eta[1]);
                weightEleTrigElePart = triggerWeight.scaleDoubleEle17Ele8(pt,eta); 
                weightEleTrigEleAugPart = triggerWeight.scaleDoubleEle17Ele8Aug(pt,eta); 
                weightTrig = (weightEleTrigElePart*1.14+weightEleTrigEleAugPart*0.98 )/2.12 * weightEleRecoAndId;

                // 2012
                weightTrig2012ADiEle = triggerWeight.doubleEle2012A(vLeptons.pt[0],vLeptons.eta[0],vLeptons.pt[1],vLeptons.eta[1]);
                float weightTrig2012ASingleEle1 = triggerWeight.singleEle2012Awp95(vLeptons.pt[0],vLeptons.eta[0]);
                float weightTrig2012ASingleEle2 = triggerWeight.singleEle2012Awp95(vLeptons.pt[1],vLeptons.eta[1]);
                weightTrig2012ASingleEle = weightTrig2012ASingleEle1+weightTrig2012ASingleEle2-weightTrig2012ASingleEle1*weightTrig2012ASingleEle2;
                weightTrig2012A = weightTrig2012ADiEle * triggerWeight.eleId2012A(vLeptons.pt[0],vLeptons.eta[0]) * triggerWeight.eleId2012A(vLeptons.pt[1],vLeptons.eta[1]) ; 

                weightTrig2012DiEle = triggerWeight.doubleEle2012A(vLeptons.pt[0],vLeptons.eta[0],vLeptons.pt[1],vLeptons.eta[1]);
                float weightTrig2012SingleEle1 = triggerWeight.singleEle2012Awp95(vLeptons.pt[0],vLeptons.eta[0]);
                float weightTrig2012SingleEle2 = triggerWeight.singleEle2012Awp95(vLeptons.pt[1],vLeptons.eta[1]);
                weightTrig2012SingleEle = weightTrig2012SingleEle1+weightTrig2012SingleEle2-weightTrig2012SingleEle1*weightTrig2012SingleEle2;
                weightTrig2012 = weightTrig2012DiEle * triggerWeight.eleId2012A(vLeptons.pt[0],vLeptons.eta[0]) * triggerWeight.eleId2012A(vLeptons.pt[1],vLeptons.eta[1]) ; 

                weightTrig2012ABDiEle = triggerWeightAB.doubleEle2012A(vLeptons.pt[0],vLeptons.eta[0],vLeptons.pt[1],vLeptons.eta[1]);
                float weightTrig2012ABSingleEle1 = triggerWeightAB.singleEle2012Awp95(vLeptons.pt[0],vLeptons.eta[0]);
                float weightTrig2012ABSingleEle2 = triggerWeightAB.singleEle2012Awp95(vLeptons.pt[1],vLeptons.eta[1]);
                weightTrig2012ABSingleEle = weightTrig2012ABSingleEle1+weightTrig2012ABSingleEle2-weightTrig2012ABSingleEle1*weightTrig2012ABSingleEle2;
                weightTrig2012AB = weightTrig2012ABDiEle * triggerWeightAB.eleId2012A(vLeptons.pt[0],vLeptons.eta[0]) * triggerWeightAB.eleId2012A(vLeptons.pt[1],vLeptons.eta[1]) ;
                
                nvlep = 2;
                firstAddEle = 2;
            }
            
            if (Vtype == VHbbCandidate::Wmun) {
                leptonForTop = vhCand.V.muons[0].p4;
                vLeptons.set(vhCand.V.muons[0], 0, 13, aux);
                float cweightID = triggerWeight.scaleMuID(vLeptons.pt[0],vLeptons.eta[0]);
                float weightTrig1 = triggerWeight.scaleMuIsoHLT(vLeptons.pt[0],vLeptons.eta[0]);
                float cweightTrig = weightTrig1;
                weightTrig = cweightID * cweightTrig;
                float weightTrig1OrMu30 = triggerWeight.scaleMuOr30IsoHLT(vLeptons.pt[0],vLeptons.eta[0]);
                weightTrigOrMu30 = cweightID*weightTrig1OrMu30;
                
                // 2012
                weightTrig2012ASingleMuon = triggerWeight.singleMuon2012A(vLeptons.pt[0],vLeptons.eta[0]);
                weightTrig2012AMuonPlusWCandPt = weightTrig2012ASingleMuon + 
                    triggerWeight.muPlusWCandPt2012A_legMu(vLeptons.pt[0],vLeptons.eta[0])*triggerWeight.muPlusWCandPt2012A_legW(vhCand.V.p4.Pt(),0);
                weightTrig2012A = weightTrig2012ASingleMuon * triggerWeight.muId2012A(vLeptons.pt[0],vLeptons.eta[0]) ;

                weightTrig2012SingleMuon = weightTrig2012ASingleMuon;
                weightTrig2012MuonPlusWCandPt = weightTrig2012AMuonPlusWCandPt;
                weightTrig2012 = weightTrig2012A;

                weightTrig2012ABSingleMuon = triggerWeightAB.singleMuon2012A(vLeptons.pt[0],vLeptons.eta[0]);
                weightTrig2012ABMuonPlusWCandPt = weightTrig2012ABSingleMuon +
                    triggerWeightAB.muPlusWCandPt2012A_legMu(vLeptons.pt[0],vLeptons.eta[0])*triggerWeightAB.muPlusWCandPt2012A_legW(vhCand.V.p4.Pt(),0);
                weightTrig2012AB = weightTrig2012ABSingleMuon *  triggerWeightAB.muId2012A(vLeptons.pt[0],vLeptons.eta[0]) ;

                nvlep = 1;
                firstAddMu = 1;
            }
            
            if (Vtype == VHbbCandidate::Wen) {
                leptonForTop = vhCand.V.electrons[0].p4;
                vLeptons.set(vhCand.V.electrons[0], 0, 11, aux);
                weightTrigMay = triggerWeight.scaleSingleEleMay(vLeptons.pt[0],vLeptons.eta[0]);
                weightTrigV4 = triggerWeight.scaleSingleEleV4(vLeptons.pt[0],vLeptons.eta[0]);
                weightEleRecoAndId = triggerWeight.scaleID80Ele(vLeptons.pt[0],vLeptons.eta[0]) * triggerWeight.scaleRecoEle(vLeptons.pt[0],vLeptons.eta[0]);
                weightEleTrigJetMETPart = triggerWeight.scaleJet30Jet25(jet30pt,jet30eta)*triggerWeight.scalePFMHTEle(MET.et);
                weightEleTrigElePart = weightTrigV4; //this is for debugging only, checking only the V4 part
                weightTrigMay *= weightEleRecoAndId;
                weightTrigV4 *= weightEleRecoAndId;
                weightTrigV4 *= weightEleTrigJetMETPart;
                //weightTrig = weightTrigMay * 0.187 + weightTrigV4 * (1.-0.187); //FIXME: use proper lumi if we reload 2.fb
                weightTrig = (weightTrigMay * 0.215 + weightTrigV4 * 1.915)/ 2.13; //FIXME: use proper lumi if we reload 2.fb

                // 2012
                weightTrig2012ASingleEle = triggerWeight.singleEle2012Awp80(vLeptons.pt[0],vLeptons.eta[0]);
                weightTrig2012A = weightTrig2012ASingleEle * triggerWeight.eleId2012Awp80(vLeptons.pt[0],vLeptons.eta[0]) ;

                weightTrig2012SingleEle = weightTrig2012ASingleEle;
                weightTrig2012 = weightTrig2012A;

                weightTrig2012ABSingleEle = triggerWeightAB.singleEle2012Awp80(vLeptons.pt[0],vLeptons.eta[0]);
                weightTrig2012AB = weightTrig2012ABSingleEle * triggerWeightAB.eleId2012Awp80(vLeptons.pt[0],vLeptons.eta[0]) ;
                
                nvlep = 1;
                firstAddEle = 1;
            }

            if (isMC_) {
                weightTrigMET80 = triggerWeight.scaleMET80(MET.et);
                weightTrigMET100 = triggerWeight.scaleMET80(MET.et);
                weightTrig2CJet20 = triggerWeight.scale2CentralJet(jet10pt, jet10eta);
                weightTrigMET150 = triggerWeight.scaleMET150(MET.et);
                weightTrigMET802CJet = weightTrigMET80 * weightTrig2CJet20;
                weightTrigMET1002CJet = weightTrigMET100 * weightTrig2CJet20;
                
                // 2012
                weightTrig2012DiJet30MHT80 = triggerWeight.scaleDiJet30MHT80_2012A(vhCand.V.mets.at(0).p4.Pt());
                weightTrig2012PFMET150 = triggerWeight.scalePFMET150_2012AB(vhCand.V.mets.at(0).p4.Pt());  // demonstrated to hold also for RunC (used averaged efficiency)
                weightTrig2012SumpT100MET100 = triggerWeight.scaleSumpT100MET100_2012B(vhCand.V.mets.at(0).p4.Pt());  // demonstrated to hold also for RunC (used averaged efficiency)
                weightTrig2012APFMET150orDiJetMET = triggerWeight.scalePFMET150orDiJetMET_2012A(vhCand.V.mets.at(0).p4.Pt());
                weightTrig2012BPFMET150orDiJetMET=triggerWeight.scalePFMET150orDiJetMET_2012B(vhCand.V.mets.at(0).p4.Pt()); 
                weightTrig2012CPFMET150orDiJetMET=triggerWeight.scalePFMET150orDiJetMET_2012C(vhCand.V.mets.at(0).p4.Pt()); 
            }
            
            if (Vtype == VHbbCandidate::Znn) {
                float weightTrig1 = triggerWeight.scaleMetHLT(vhCand.V.mets.at(0).p4.Pt());
                weightTrigMETLP = weightTrig1;
                weightTrig = weightTrigMET150 + weightTrigMET802CJet - weightTrigMET802CJet * weightTrigMET150;
                
                nvlep = 0;
            }

            if (weightTrigMay < 0)
                weightTrigMay = weightTrig;
            if (weightTrigV4 < 0)
                weightTrigV4 = weightTrig;
            if (!isMC_) {
                weightTrig = 1.;
                weightTrigMay = 1.;
                weightTrigV4 = 1.;
                weightEleRecoAndId = 1.;
                weightEleTrigJetMETPart = 1.;
                weightEleTrigElePart = 1.;
                weightEleTrigEleAugPart = 1.;
                weightTrigMET80 = 1.;
                weightTrigMET100 = 1.;
                weightTrig2CJet20 = 1.;
                weightTrigMET150 = 1.;
                weightTrigMET802CJet = 1.;
                weightTrigMET1002CJet = 1.;
                weightTrigMETLP = 1.;
            }
            

            aLeptons.reset();
            nalep = 0;
            if (fromCandidate) {  // legacy, not in use
                for (size_t j = firstAddMu; j < vhCand.V.muons.size(); j++)
                    aLeptons.set(vhCand.V.muons[j], nalep++, 13, aux);
                for (size_t j = firstAddEle; j < vhCand.V.electrons.size(); j++)
                    aLeptons.set(vhCand.V.electrons[j], nalep++, 11, aux);
            } else {
                for (size_t j = 0; j < iEvent->muInfo.size(); j++) {
                    if ((j != vhCand.V.firstLepton && j != vhCand.V.secondLepton)
                        || ((Vtype != VHbbCandidate::Wmun) && (Vtype != VHbbCandidate::Zmumu)))
                        aLeptons.set(iEvent->muInfo[j], nalep++, 13, aux);  // nalep++ in a function is bad practice. Fix?
                }
                for (size_t j = 0; j < iEvent->eleInfo.size(); j++) {
                    if ((j != vhCand.V.firstLepton && j != vhCand.V.secondLepton)
                        || ((Vtype != VHbbCandidate::Wen) && (Vtype != VHbbCandidate::Zee)))
                        aLeptons.set(iEvent->eleInfo[j], nalep++, 11, aux);  // nalep++ in a function is bad practice. Fix?
                }
            }

            if (isMC_) {
                //std::cout << "BTAGSF " <<  btagJetInfos.size() << " " << btag.weight<BTag1Tight2CustomFilter>(btagJetInfos) << std::endl;
                if (btagJetInfos.size() < 10) {
                    btag1T2CSF = btag.weight < BTag1Tight2CustomFilter >(btagJetInfos);
                    btag2TSF = btag.weight < BTag2TightFilter > (btagJetInfos);
                    btag1TSF = btag.weight < BTag1TightFilter > (btagJetInfos);
                    btagA0CSF = btag.weight < BTagAntiMax0CustomFilter >(btagJetInfos);
                    btagA0TSF = btag.weight < BTagAntiMax0TightFilter >(btagJetInfos);
                    btag2CSF = btag.weight < BTag2CustomFilter > (btagJetInfos);
                    btag1TA1C = btag.weight < BTag1TightAndMax1CustomFilter >(btagJetInfos);
                } else {
                    std::cout << "WARNING:  combinatorics for " << btagJetInfos.size() << " jets is too high (>=10). use SF=1 " << std::endl;
                    //TODO: revert to random throw for this cases
                    btag1T2CSF = 1.;
                    btag2TSF = 1.;
                    btag1TSF = 1.;
                    btagA0CSF = 1.;
                    btagA0TSF = 1.;
                    btag2CSF = 1.;
                    btag1TA1C = 1.;
                }
            }

            if (maxBtag > -99999) {
                TopHypo topQuark = TopMassReco::topMass(leptonForTop, bJet, vhCand.V.mets.at(0).p4);
                top.mass = topQuark.p4.M();
                top.pt = topQuark.p4.Pt();
                top.wMass = topQuark.p4W.M();
            } else {
                top.mass = -99;
                top.pt = -99;
                top.wMass = -99;
            }

            //FIXME: Too much warnings coming from calling Eta(), fix?
            //gendrcc=aux.genCCDeltaR(); 
            //gendrbb=aux.genBBDeltaR(); 

            genHpt = aux.mcH.size() > 0 ? aux.mcH[0].p4.Pt() : -99;
            genZpt = aux.mcZ.size() > 0 ? aux.mcZ[0].p4.Pt() : -99;
            genWpt = aux.mcW.size() > 0 ? aux.mcW[0].p4.Pt() : -99;

            // Z* is status=3 and Nmother=2 (q and qbar)  
            // Z is status=2 and Ndau=2 (mup and mum)  
            for (unsigned int i = 0; i < aux.mcZ.size(); i++) {
                if (aux.mcZ[i].status == 3) {
                    genZstar.mass = aux.mcZ[i].p4.M();
                    genZstar.pt = aux.mcZ[i].p4.Pt();
                    if (genZstar.pt > 0.1) genZstar.eta = aux.mcZ[i].p4.Eta();  else genZstar.eta = -99;
                    genZstar.phi = aux.mcZ[i].p4.Phi();
                    genZstar.status = aux.mcZ[i].status;
                    genZstar.charge = aux.mcZ[i].charge;
                    if (aux.mcZ[i].momid != -99)
                        genZstar.momid = aux.mcZ[i].momid;
                }

                if (aux.mcZ[i].dauid.size() > 1 &&
                    (abs(aux.mcZ[i].dauid[0]) == 11 || abs(aux.mcZ[i].dauid[0]) == 12 ||
                     abs(aux.mcZ[i].dauid[0]) == 13 || abs(aux.mcZ[i].dauid[0]) == 14 ||
                     abs(aux.mcZ[i].dauid[0]) == 15 || abs(aux.mcZ[i].dauid[0]) == 16 ) ) {
                    genZ.mass = aux.mcZ[i].p4.M();
                    genZ.pt = aux.mcZ[i].p4.Pt();
                    if (genZ.pt > 0.1) genZ.eta = aux.mcZ[i].p4.Eta();  else genZ.eta = -99;
                    genZ.phi = aux.mcZ[i].p4.Phi();
                    genZ.status = aux.mcZ[i].status;
                    genZ.charge = aux.mcZ[i].charge;
                    if (aux.mcZ[i].momid != -99)
                        genZ.momid = aux.mcZ[i].momid;
                }
            }

            for (unsigned int i = 0; i < aux.mcW.size(); i++) {
                if (aux.mcW[i].momid == 6 && aux.mcW[i].dauid.size() > 1) {
                    genTop.wdau1mass = aux.mcW[i].dauFourMomentum[0].M();
                    genTop.wdau1pt = aux.mcW[i].dauFourMomentum[0].Pt();
                    if (genTop.wdau1pt > 0.1) genTop.wdau1eta = aux.mcW[i].dauFourMomentum[0].Eta();  else genTop.wdau1eta = -99;
                    genTop.wdau1phi = aux.mcW[i].dauFourMomentum[0].Phi();
                    genTop.wdau1id = aux.mcW[i].dauid[0];

                    genTop.wdau2mass = aux.mcW[i].dauFourMomentum[1].M();
                    genTop.wdau2pt = aux.mcW[i].dauFourMomentum[1].Pt();
                    if (genTop.wdau2pt > 0.1) genTop.wdau2eta = aux.mcW[i].dauFourMomentum[1].Eta();  else genTop.wdau2eta = -99;
                    genTop.wdau2phi = aux.mcW[i].dauFourMomentum[1].Phi();
                    genTop.wdau2id = aux.mcW[i].dauid[1];
                }

                if (aux.mcW[i].momid == -6 && aux.mcW[i].dauid.size() > 1) {
                    genTbar.wdau1mass = aux.mcW[i].dauFourMomentum[0].M();
                    genTbar.wdau1pt = aux.mcW[i].dauFourMomentum[0].Pt();
                    if (genTbar.wdau1pt > 0.1) genTbar.wdau1eta = aux.mcW[i].dauFourMomentum[0].Eta();  else genTbar.wdau1eta = -99;
                    genTbar.wdau1phi = aux.mcW[i].dauFourMomentum[0].Phi();
                    genTbar.wdau1id = aux.mcW[i].dauid[0];

                    genTbar.wdau2mass = aux.mcW[i].dauFourMomentum[1].M();
                    genTbar.wdau2pt = aux.mcW[i].dauFourMomentum[1].Pt();
                    if (genTbar.wdau2pt > 0.1) genTbar.wdau2eta = aux.mcW[i].dauFourMomentum[1].Eta();  else genTbar.wdau2eta = -99;
                    genTbar.wdau2phi = aux.mcW[i].dauFourMomentum[1].Phi();
                    genTbar.wdau2id = aux.mcW[i].dauid[1];
                }

                if (aux.mcW[i].status == 3) {
                    genWstar.mass = aux.mcW[i].p4.M();
                    genWstar.pt = aux.mcW[i].p4.Pt();
                    if (genWstar.pt > 0.1) genWstar.eta = aux.mcW[i].p4.Eta();  else genWstar.eta = -99;
                    genWstar.phi = aux.mcW[i].p4.Phi();
                    genWstar.status = aux.mcW[i].status;
                    genWstar.charge = aux.mcW[i].charge;
                    if (aux.mcW[i].momid != -99)
                        genWstar.momid = aux.mcW[i].momid;
                }
                
                if (aux.mcW[i].dauid.size() > 1 && 
                    (abs(aux.mcW[i].dauid[0]) == 13 || abs(aux.mcW[i].dauid[0]) == 11)) {
                    genW.mass = aux.mcW[i].p4.M();
                    genW.pt = aux.mcW[i].p4.Pt();
                    if (genW.pt > 0.1) genW.eta = aux.mcW[i].p4.Eta();  else genW.eta = -99;
                    genW.phi = aux.mcW[i].p4.Phi();
                    genW.status = aux.mcW[i].status;
                    genW.charge = aux.mcW[i].charge;
                    if (aux.mcW[i].momid != -99)
                        genW.momid = aux.mcW[i].momid;
                }
            }
            
            // b coming from Higgs
            for (unsigned int i = 0; i < aux.mcB.size(); i++) {
                if (abs(aux.mcB[i].momid) == 25) {
                    genB.mass = aux.mcB[i].p4.M();
                    genB.pt = aux.mcB[i].p4.Pt();
                    if (genB.pt > 0.1) genB.eta = aux.mcB[i].p4.Eta();  else genB.eta = -99;
                    genB.phi = aux.mcB[i].p4.Phi();
                    genB.status = aux.mcB[i].status;
                    genB.charge = aux.mcB[i].charge;
                    if (aux.mcB[i].momid != -99)
                        genB.momid = aux.mcB[i].momid;
                }

                if (aux.mcB[i].momid == 6) {
                    genTop.bmass = aux.mcB[i].p4.M();
                    genTop.bpt = aux.mcB[i].p4.Pt();
                    if (genTop.bpt > 0.1) genTop.beta = aux.mcB[i].p4.Eta();  else genTop.beta = -99;
                    genTop.bphi = aux.mcB[i].p4.Phi();
                    genTop.bstatus = aux.mcB[i].status;
                }
            }

            for (unsigned int i = 0; i < aux.mcBbar.size(); i++) {
                if (abs(aux.mcBbar[i].momid) == 25) {
                    genBbar.mass = aux.mcBbar[i].p4.M();
                    genBbar.pt = aux.mcBbar[i].p4.Pt();
                    if (genBbar.pt > 0.1) genBbar.eta = aux.mcBbar[i].p4.Eta();  else genBbar.eta = -99;
                    genBbar.phi = aux.mcBbar[i].p4.Phi();
                    genBbar.status = aux.mcBbar[i].status;
                    if (aux.mcBbar[i].momid != -99)
                        genBbar.momid = aux.mcBbar[i].momid;
                }
                
                if (aux.mcBbar[i].momid == -6) {
                    genTbar.bmass = aux.mcBbar[i].p4.M();
                    genTbar.bpt = aux.mcBbar[i].p4.Pt();
                    if (genTbar.bpt > 0.1) genTbar.beta = aux.mcBbar[i].p4.Eta();  else genTbar.beta = -99;
                    genTbar.bphi = aux.mcBbar[i].p4.Phi();
                    genTbar.bstatus = aux.mcBbar[i].status;
                }
            }

            if (aux.mcH.size() > 0) {  // come from Z*
                genH.mass = aux.mcH[0].p4.M();
                genH.pt = aux.mcH[0].p4.Pt();
                if (genH.pt > 0.1) genH.eta = aux.mcH[0].p4.Eta();  else genH.eta = -99;
                genH.phi = aux.mcH[0].p4.Phi();
                genH.status = aux.mcH[0].status;
                genH.charge = aux.mcH[0].charge;
                if (aux.mcH[0].momid != -99)
                    genH.momid = aux.mcH[0].momid;
            }

            WminusMode = -99;
            WplusMode = -99;
            for (unsigned int j = 0; j < aux.mcW.size(); j++) {
                for (unsigned int k = 0; k < aux.mcW[j].dauid.size(); k++) {
                    int idd = abs(aux.mcW[j].dauid[k]);
                    if (idd == 11 || idd == 13 || idd == 15 || (idd <= 5 && idd >= 1)) {
                        if (WminusMode == -99 && aux.mcW[j].charge == -1)
                            WminusMode = idd;
                        if (WplusMode == -99 && aux.mcW[j].charge == +1)
                            WplusMode = idd;
                    }
                }
                /*
                   /// now check if a semileptonic W is also in a bjets....      
                   if ( ( (WminusMode==11 || WminusMode==13 || WminusMode==15  ) || (WplusMode==11 || WplusMode==13 || WplusMode==15  ))  && deltaR(vhCand.H.jets[0].p4.Eta(),vhCand.H.jets[0].p4.Phi(), aux.mcW[j].p4.Eta(), aux.mcW[j].p4.Phi())<0.3 ) hJets.isSemiLeptMCtruth[0]=1;
                   if ( ( (WminusMode==11 || WminusMode==13 || WminusMode==15  ) || (WplusMode==11 || WplusMode==13 || WplusMode==15  ))  && deltaR(vhCand.H.jets[1].p4.Eta(),vhCand.H.jets[1].p4.Phi(), aux.mcW[j].p4.Eta(), aux.mcW[j].p4.Phi())<0.3 ) hJets.isSemiLeptMCtruth[1]=1;

                   for( int j=0; j < naJets && j < MAXJ; j++ ) 
                   {
                   if ((idd==11 || idd==13 || idd==15  )  && deltaR(vhCand.additionalJets[j].p4.Eta(),vhCand.additionalJets[j].p4.Phi(), aux.mcW[j].p4.Eta(), aux.mcW[j].p4.Phi()) <0.3) aJets.isSemiLept[j]=1;

                   }
                 */
            }
            
            /// Compute pull angle from AK7
            if (vhCand.H.HiggsFlag) {
                if (!fromCandidate) {
                    std::vector < VHbbEvent::SimpleJet > ak7wrt1(iEvent->simpleJets3);
                    std::vector < VHbbEvent::SimpleJet > ak7wrt2(iEvent->simpleJets3);
                    if (ak7wrt1.size() > 1) {
                        CompareDeltaR deltaRComparatorJ1(vhCand.H.jets[0].p4);
                        CompareDeltaR deltaRComparatorJ2(vhCand.H.jets[1].p4);
                        std::sort(ak7wrt1.begin(), ak7wrt1.end(), deltaRComparatorJ1);
                        std::sort(ak7wrt2.begin(), ak7wrt2.end(), deltaRComparatorJ2);
                        std::vector < VHbbEvent::SimpleJet > ak7_matched;
                        // if the matches are different save them
                        if (ak7wrt1[0].p4.DeltaR(ak7wrt2[0].p4) > 0.1) {
                            ak7_matched.push_back(ak7wrt1[0]);
                            ak7_matched.push_back(ak7wrt2[0]);
                        
                        // else look at the second best
                        } else {
                            // ak7wrt1 is best
                            if (ak7wrt1[1].p4.DeltaR(vhCand.H.jets[0].p4) <
                                ak7wrt2[1].p4.DeltaR(vhCand.H.jets[1].p4)) {
                                ak7_matched.push_back(ak7wrt1[1]);
                                ak7_matched.push_back(ak7wrt2[0]);
                            } else {
                                ak7_matched.push_back(ak7wrt1[0]);
                                ak7_matched.push_back(ak7wrt2[1]);
                            }
                        }
                        CompareJetPt ptComparator;
                        std::sort(ak7_matched.begin(), ak7_matched.end(), ptComparator);
                        if (ak7_matched[0].p4.DeltaR(vhCand.H.jets[0].p4) < 0.5 && ak7_matched[1].p4.DeltaR(vhCand.H.jets[1].p4) < 0.5) {
                            deltaPullAngleAK7 = VHbbCandidateTools::getDeltaTheta(ak7_matched[0], ak7_matched[1]);
                            deltaPullAngle2AK7 = VHbbCandidateTools::getDeltaTheta(ak7_matched[1], ak7_matched[0]);
                        }
                    }
                }
            }  // end if HiggsFlag

            _outTree->Fill();
            ++jevt;
        }  // end loop over events

        std::cout << "Closing file " << inputFiles_[iFile] << std::endl;
        inFile->Close();  // close input file
    }  // end loop over files
    
    std::cout << "------------------------------- " << std::endl;
    std::cout << "Events: " << ievt << std::endl;
    std::cout << "Saved : " << jevt << std::endl;
    // Get elapsed time
    sw.Stop();
    sw.Print();

    _outFile->cd();
    //_outTree->Write();
    _outFile->Write();
    _outFile->Close();
    
    delete algoZ; delete algoW; delete algoRecoverLowPt;
    delete nominalShape; delete upBCShape; delete downBCShape; delete upLShape; delete downLShape;
    delete nominalShape4p; delete upBCShape4p; delete downBCShape4p;
    
    return 0;
}

