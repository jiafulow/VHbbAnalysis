#ifndef BtagReshaping__H
#define BtagReshaping__H


#include <cassert>
#include <vector>
#include <iostream>

#include "TFile.h"
#include "TH1.h"
#include "Math/Interpolator.h"

#include "btag_payload_b.h"
#include "btag_payload_light.h"

#define CSVT 0.898
#define CSVM 0.679
#define CSVL 0.244
// Reference: https://twiki.cern.ch/twiki/bin/viewauth/CMS/BtagPOG#2012_Data_and_MC


class BTagShape {
  public:
    BTagShape() {}
    BTagShape(TFile * file, const char *name, const std::vector < std::pair < float, float > > & cutsAndSF,
              float boundX, float boundY) {
        bool verbose = false;
        TH1F * m_h = (TH1F *) file->Get(name);

        /// Find the equivalent CSV values that give the same efficiencies (area).
        std::vector < std::pair < float, float > > eq;
        const int lastbin = m_h->GetNbinsX()+1; // was 2001;
        float integral = m_h->Integral(-1, lastbin);
        for (unsigned int i = 0; i < cutsAndSF.size(); i++) {
            float oldCut = cutsAndSF[i].first;
            float sf = cutsAndSF[i].second;
            int   originalBin = m_h->FindBin(oldCut);
            float originalIntegral = m_h->Integral(originalBin, lastbin);
            float originalLowEdge = m_h->GetBinLowEdge(originalBin);
            float target = originalIntegral * sf;

            if (verbose) {
                std::cout << " Scale Factor: " << sf << std::endl;
                std::cout << " Target " << target << " orig " << originalIntegral << std::endl;
            }
            
            for (int j = lastbin; j > -1; j--) {
                if (m_h->Integral(j, lastbin) >= target) {
                    eq.push_back(std::pair < float, float >(m_h->GetBinLowEdge(j), originalLowEdge));
                    if (verbose) {
                        std::cout << "Found at " << j << " was " << m_h->FindBin(oldCut) << std::endl;
                        std::cout << m_h->GetBinLowEdge(j) << " was " << originalLowEdge << " cut: " << oldCut << std::endl;
                    }
                    break;
                }
            }
        }

        /// Interpolate the CSV values between the given working points.
        std::vector < double > x;
        std::vector < double > y;
        x.push_back(0.);
        y.push_back(0.);
        for (unsigned int i = 0; i < eq.size(); i++) {
            x.push_back(eq[eq.size() - i - 1].first);
            y.push_back(eq[eq.size() - i - 1].second);
        }
        x.push_back(boundX);
        y.push_back(boundY);

        /// Linear interpolation
        m_i = new ROOT::Math::Interpolator(x, y, ROOT::Math::Interpolation::kLINEAR);
    }

    ~BTagShape() {
        delete m_i;
    }

    float eval(float x) {
        return m_i->Eval(x);
    }

  protected:
    ROOT::Math::Interpolator * m_i;
};  // BTagShape


class EtaPtBin {
  public:
    EtaPtBin() {}
    EtaPtBin(float emin, float emax, float ptmin, float ptmax)
      : etaMin(emin), etaMax(emax), ptMin(ptmin), ptMax(ptmax) {}
    bool contains(float eta, float pt) {
        return eta < etaMax && eta >= etaMin && pt < ptMax && pt >= ptMin;
    }
    float centerEta() {
        return (etaMax + etaMin) / 2.;
    }
    float centerPt() {
        return (ptMax + ptMin) / 2.;
    }

  protected:
    float etaMin;
    float etaMax;
    float ptMin;
    float ptMax;
};  // EtaPtBin


class BinnedBTagShape {
  public:
    BinnedBTagShape() {}
    BinnedBTagShape(std::vector < EtaPtBin > & bins,
                    std::vector < std::vector < std::pair < float, float > > > & cutsAndSF, 
                    TFile * f, const char *name, float boundX, float boundY)
      : m_bins(bins) {
        for (unsigned int i = 0; i < bins.size(); i++) {
            m_shapes.push_back(BTagShape(f, name, cutsAndSF[i], boundX, boundY));
        }
    }

    double eval(float eta, float pt, float x) {
        for (unsigned int i = 0; i < m_bins.size(); i++) {
            if (m_bins[i].contains(fabs(eta), pt))
                return m_shapes[i].eval(x);
        }
        if (pt > 20.)
            std::cerr << "Cannot reshape eta pt discr "  << eta << " " << pt << " " << x << std::endl; 
        return x;
    }

  protected:
    std::vector < BTagShape > m_shapes;
    std::vector < EtaPtBin > m_bins;
};  // BinnedBTagShape


class BTagShapeInterface {
  public:
    BTagShapeInterface() {}
    BTagShapeInterface(const char *file, float scaleBC, float scaleL, 
                       bool use4points=false, float boundX=1.001, float boundY=1.001)
      : m_file(new TFile(file)), m_b(0), m_c(0), m_l(0)
    {
        bool verbose = false;
        
        /// SFs for b & c efficiencies
        std::vector < EtaPtBin > binsBC;
        std::vector < std::vector < std::pair < float, float > > > cutsAndSFB;
        std::vector < std::vector < std::pair < float, float > > > cutsAndSFC;
        float charmFactor = 2. - 1.0;  // additional uncertainty for charm
        for (unsigned int i = 0; i < beff::bins; i++) {
            std::vector < std::pair < float, float > > cutsAndSFbinB;
            std::vector < std::pair < float, float > > cutsAndSFbinC;
            EtaPtBin bin(-2.5, 2.5, beff::ptmin[i], beff::ptmax[i]);

            float sftt = 0.94;
            if (use4points) {
                sftt += scaleBC * beff::CSVT_SFb_error[i];  // add error
                cutsAndSFbinB.push_back(std::pair < float, float >(0.98, sftt));
                sftt += scaleBC * beff::CSVT_SFb_error[i] * charmFactor;  // charm additional error
                cutsAndSFbinC.push_back(std::pair < float, float >(0.98, sftt));
            }
            
            float sft = beff::CSVT_SFb(bin.centerPt());
            sft += scaleBC * beff::CSVT_SFb_error[i];  // add error
            cutsAndSFbinB.push_back(std::pair < float, float >(CSVT, sft));
            sft += scaleBC * beff::CSVT_SFb_error[i] * charmFactor;  // charm additional error
            cutsAndSFbinC.push_back(std::pair < float, float >(CSVT, sft));

            float sfm = beff::CSVM_SFb(bin.centerPt());
            sfm += scaleBC * beff::CSVM_SFb_error[i];  // add error
            cutsAndSFbinB.push_back(std::pair < float, float >(CSVM, sfm));
            sfm += scaleBC * beff::CSVM_SFb_error[i] * charmFactor;  // charm additional error
            cutsAndSFbinC.push_back(std::pair < float, float >(CSVM, sfm));

            float sfl = beff::CSVL_SFb(bin.centerPt());
            sfl += scaleBC * beff::CSVL_SFb_error[i];  // add error
            cutsAndSFbinB.push_back(std::pair < float, float >(CSVL, sfl));
            sfl += scaleBC * beff::CSVL_SFb_error[i] * charmFactor;  // charm additional error
            cutsAndSFbinC.push_back(std::pair < float, float >(CSVL, sfl));

            if (verbose)  std::cout << "SFs " << i << " " << sfl << " " << sfm << " " << sft << std::endl;
            binsBC.push_back(bin);
            cutsAndSFB.push_back(cutsAndSFbinB);
            cutsAndSFC.push_back(cutsAndSFbinC);
        }
        /// underflow: use an absolute error of 0.12
        {
            std::vector < std::pair < float, float > > cutsAndSFbinB;
            std::vector < std::pair < float, float > > cutsAndSFbinC;
            EtaPtBin bin(-2.5, 2.5, -9999., beff::ptmin[0]);
            
            float sft = beff::CSVT_SFb(beff::ptmin[0]);
            sft += scaleBC * 0.12;  // add error
            cutsAndSFbinB.push_back(std::pair < float, float >(CSVT, sft));
            sft += scaleBC * 0.12 * charmFactor;  // charm additional error
            cutsAndSFbinC.push_back(std::pair < float, float >(CSVT, sft));

            float sfm = beff::CSVM_SFb(beff::ptmin[0]);
            sfm += scaleBC * 0.12;  // add error
            cutsAndSFbinB.push_back(std::pair < float, float >(CSVM, sfm));
            sfm += scaleBC * 0.12 * charmFactor;  // charm additional error
            cutsAndSFbinC.push_back(std::pair < float, float >(CSVM, sfm));

            float sfl = beff::CSVL_SFb(beff::ptmin[0]);
            sfl += scaleBC * 0.12;  // add error
            cutsAndSFbinB.push_back(std::pair < float, float >(CSVL, sfl));
            sfl += scaleBC * 0.12 * charmFactor;  // charm additional error
            cutsAndSFbinC.push_back(std::pair < float, float >(CSVL, sfl));

            if (verbose)  std::cout << "Firstbin SFs " << sfl << " " << sfm << " " << sft << std::endl;
            binsBC.push_back(bin);
            cutsAndSFB.push_back(cutsAndSFbinB);
            cutsAndSFC.push_back(cutsAndSFbinC);
        }
        /// overflow: use 2 times the error from the last pt bin
        {
            std::vector < std::pair < float, float > > cutsAndSFbinB;
            std::vector < std::pair < float, float > > cutsAndSFbinC;
            EtaPtBin bin(-2.5, 2.5, beff::ptmax[beff::bins - 1], 9999.);

            float sft = beff::CSVT_SFb(beff::ptmax[beff::bins - 1]);
            sft += scaleBC * beff::CSVT_SFb_error[beff::bins - 1] * 2;  // add error
            cutsAndSFbinB.push_back(std::pair < float, float >(CSVT, sft));
            sft += scaleBC * beff::CSVT_SFb_error[beff::bins - 1] * charmFactor;  // charm additional error
            cutsAndSFbinC.push_back(std::pair < float, float >(CSVT, sft));

            float sfm = beff::CSVM_SFb(beff::ptmax[beff::bins - 1]);
            sfm += scaleBC * beff::CSVM_SFb_error[beff::bins - 1] * 2;  // add error
            cutsAndSFbinB.push_back(std::pair < float, float >(CSVM, sfm));
            sfm += scaleBC * beff::CSVM_SFb_error[beff::bins - 1] * charmFactor;  // charm additional error
            cutsAndSFbinC.push_back(std::pair < float, float >(CSVM, sfm));

            float sfl = beff::CSVL_SFb(beff::ptmax[beff::bins - 1]);
            sfl += scaleBC * beff::CSVL_SFb_error[beff::bins - 1] * 2;  // add error
            cutsAndSFbinB.push_back(std::pair < float, float >(CSVL, sfl));
            sfl += scaleBC * beff::CSVL_SFb_error[beff::bins - 1] * charmFactor;  // charm additional error
            cutsAndSFbinC.push_back(std::pair < float, float >(CSVL, sfl));

            if (verbose)  std::cout << "Lastbin SFs " << sfl << " " << sfm << " " << sft << std::endl;
            binsBC.push_back(bin);
            cutsAndSFB.push_back(cutsAndSFbinB);
            cutsAndSFC.push_back(cutsAndSFbinC);
        }

        m_b = new BinnedBTagShape(binsBC, cutsAndSFB, m_file, "hb", boundX, boundY);
        m_c = new BinnedBTagShape(binsBC, cutsAndSFC, m_file, "hc", boundX, boundY);

        /// SFs for u, d, s & g efficiencies
        std::vector < EtaPtBin > binsL;
        std::vector < std::vector < std::pair < float, float > > > cutsAndSFL;
        
        /// 20-30 is also covered for mistag
        float ptmin[] = { 20, 30, 40, 50, 60, 70, 80, 100, 120, 160, 210, 260, 320, 400, 500 };
        float ptmax[] = { 30, 40, 50, 60, 70, 80, 100, 120, 160, 210, 260, 320, 400, 500, 670 };
        float etamin[] = { 0.0, 0.5, 1.0, 1.5 };
        float etamax[] = { 0.5, 1.0, 1.5, 2.5 };
        unsigned int bins = 15;

        for (unsigned int j = 0; j < 4; j++) {  // loop over eta bins
            for (unsigned int i = 0; i < bins; i++) {  // loop over pt bins
                std::vector < std::pair < float, float > > cutsAndSFbinL;
                EtaPtBin bin(etamin[j], etamax[j], ptmin[i], ptmax[i]);

                float sft = mistag_CSVT(bin.centerEta(), bin.centerPt(), scaleL * 1.5);
                cutsAndSFbinL.push_back(std::pair < float, float >(CSVT, sft));

                float sfm = mistag_CSVM(bin.centerEta(), bin.centerPt(), scaleL);
                cutsAndSFbinL.push_back(std::pair < float, float >(CSVM, sfm));

                float sfl = mistag_CSVL(bin.centerEta(), bin.centerPt(), scaleL);
                cutsAndSFbinL.push_back(std::pair < float, float >(CSVL, sfl));

                if (verbose)  std::cout << "SFs light " << j << " " << i << " " << sfl << " " << sfm << " " << sft << std::endl;
                binsL.push_back(bin);
                cutsAndSFL.push_back(cutsAndSFbinL);
            }
            /// Overflow
            {
                std::vector < std::pair < float, float > > cutsAndSFbinL;
                EtaPtBin bin(etamin[j], etamax[j], ptmax[bins - 1], 9999.);

                float sft = mistag_CSVT(bin.centerEta(), ptmax[bins - 1], scaleL * 2);  // should inflate error for CSVT?
                cutsAndSFbinL.push_back(std::pair < float, float >(CSVT, sft));

                float sfm = mistag_CSVM(bin.centerEta(), ptmax[bins - 1], scaleL * 2);
                cutsAndSFbinL.push_back(std::pair < float, float >(CSVM, sfm));

                float sfl = mistag_CSVL(bin.centerEta(), ptmax[bins - 1], scaleL * 2);
                cutsAndSFbinL.push_back(std::pair < float, float >(CSVL, sfl));

                if (verbose)  std::cout << "SFs light " << sfl << " " << sfm << " " << sft << std::endl;
                binsL.push_back(bin);
                cutsAndSFL.push_back(cutsAndSFbinL);
            }
            /// No underflow prescription
        }
        m_l = new BinnedBTagShape(binsL, cutsAndSFL, m_file, "hl", boundX, boundY);
    }

    
    ~BTagShapeInterface()
    {
        delete m_b; delete m_c; delete m_l;
        //delete m_file;
    }

    float reshape(float eta, float pt, float csv, int flav) {
        if (m_b == 0 || m_c == 0 || m_l == 0){
            std::cout << "ERROR: BTagShape pointers are not set up correctly!" << std::endl;
            return -9999.;
        }
        if (csv < 0)  return csv;
        if (csv > 1)  return csv;
        if (flav == 0)  return csv;
        if (abs(flav) == 5)  return m_b->eval(eta, pt, csv);
        if (abs(flav) == 4)  return m_c->eval(eta, pt, csv);
        if (abs(flav) != 4 && abs(flav) != 5)  return m_l->eval(eta, pt, csv);
        return -9999.;
    }

  protected:
    TFile * m_file;
    BinnedBTagShape * m_b;
    BinnedBTagShape * m_c;
    BinnedBTagShape * m_l;
};  // BTagShapeInterface
#endif //  BtagReshaping__H

