#ifndef VHbbEventAuxInfo__H
#define VHbbEventAuxInfo__H

#include <vector>
#include <map>

#include "TLorentzVector.h"
#include "TVector2.h"
#include "Math/SMatrix.h"


class VHbbEventAuxInfo {
  public:
    class PrimaryVertexInfo {
      public:
        PrimaryVertexInfo(): nVertices(-99) {}
        int nVertices;
        TVector3 firstPVInPT2, firstPVInProb;
        ROOT::Math::SMatrix < double, 3, 3, ROOT::Math::MatRepSym < double, 3 > > efirstPVInPT2, efirstPVInProb;
    };

    class PUInfo {
      public:
        PUInfo(): rho(-99), rho25(-99), rho25Iso(-99), rhoNeutral(-99), intimePU(99), truePU(9999), pus() {}
      public:
        float rho, rho25, rho25Iso, rhoNeutral;
        int intimePU;
        float truePU;
        std::map < int, unsigned int > pus;
    };

    class ParticleMCInfo {
      public:
        ParticleMCInfo(): status(-99), momid(-99), gmomid(-99), charge(-99), dauid(), dauFourMomentum() {}
      public:
        int status;
        int momid;
        int gmomid;
        float charge;
        TLorentzVector p4;
        //int ndau;
        std::vector < int > dauid;
        std::vector < TLorentzVector > dauFourMomentum;
    };

    double genBBDeltaR() const {
        if (mcB.size() > 0 && mcBbar.size() > 0)
            return mcB[0].p4.DeltaR(mcBbar[0].p4);
        else
            return -99;
    }
    
    double genCCDeltaR() const {
        if (mcC.size() >= 2)
            return mcC[0].p4.DeltaR(mcC[1].p4);
        else
            return -99;
    }

    PrimaryVertexInfo pvInfo;
    PUInfo puInfo;
    std::vector < ParticleMCInfo > mcH;
    std::vector < ParticleMCInfo > mcW;
    std::vector < ParticleMCInfo > mcZ;
    std::vector < ParticleMCInfo > mcTau;
    std::vector < ParticleMCInfo > mcBbar;
    std::vector < ParticleMCInfo > mcB;
    std::vector < ParticleMCInfo > mcC;
    float weightMCProd;

};
#endif // VHbbEventAuxInfo__H

