#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/FWLite/interface/Handle.h"
#include "PhysicsTools/FWLite/interface/TFileService.h"
#include "FWCore/FWLite/interface/AutoLibraryLoader.h"
#include "FWCore/ParameterSet/interface/ProcessDesc.h"
#include "FWCore/PythonParameterSet/interface/PythonProcessDesc.h"

//#include "DataFormats/Candidate/interface/Candidate.h"
//#include "DataFormats/Math/interface/deltaR.h"
//#include "DataFormats/Math/interface/deltaPhi.h"

#include "Utilities/Parang/interface/Plotter.h"
#include "Utilities/Parang/interface/Polybook.h"

#include "VHbbAnalysis/ZmumuHbbSkim/bin/FWLiteBasicAnalyzer.h"


int main(int argc, char* argv[]) 
{

    // load framework libraries
    gSystem->Load( "libFWCoreFWLite" );
    AutoLibraryLoader::enable();
    
    gSystem->Load( "libUtilitiesParang" );
    gSystem->AddIncludePath(" -I$CMSSW_BASE/src ");

    // parse arguments
    if ( argc < 2 ) {
        std::cout << "Usage : " << argv[0] << " [parameters.py]" << std::endl;
        return 0;
    }

    // get the python configuration
    PythonProcessDesc builder(argv[1]);
    const edm::ParameterSet& in  = builder.processDesc()->getProcessPSet()->getParameter<edm::ParameterSet>("fwliteInput" );
    const edm::ParameterSet& out = builder.processDesc()->getProcessPSet()->getParameter<edm::ParameterSet>("fwliteOutput");
    const edm::ParameterSet& ana = builder.processDesc()->getProcessPSet()->getParameter<edm::ParameterSet>("Analyzer");

    // now get each parameter
    int maxEvents_( in.getParameter<int>("maxEvents") );
    unsigned int outputEvery_( in.getParameter<unsigned int>("outputEvery") );
    std::vector<std::string> inputFiles_( in.getParameter<std::vector<std::string> >("fileNames") );
    std::string outputFile_( out.getParameter<std::string>("fileName" ) );

    
    // Parang    
    PlotterF        plotter;
    static Polybook evtBook(&plotter);

  
    // loop the events
    unsigned int ievt = 0;
    double       crossSection = 1.0;

    for(unsigned int iFile=0; iFile<inputFiles_.size(); ++iFile){
    
        // open input file (can be located on castor)
        std::cout << "reading the file: " << inputFiles_[iFile] << std::endl;
        TFile* inFile = TFile::Open(inputFiles_[iFile].c_str());
        if( inFile ){


            fwlite::Event ev(inFile);
            for(ev.toBegin(); !ev.atEnd(); ++ev, ++ievt){
                
                NtpBaseAnalyzer theEvent(ev, ana);
                const ObjectPair& jetPair = theEvent.jetPairs[0];
                const ObjectPair& muPair  = theEvent.muPairs[0];
                const MET&        met     = theEvent.met;
                
                double j1Pt    = jetPair.p1.pt();
                double j2Pt    = jetPair.p2.pt();
                double j12Mass = jetPair.mass();
                double j12Pt   = jetPair.pt();
                double j1MetDPhi     = reco::deltaPhi(jetPair.p1.phi(), met.p4.phi() );
                double j2MetDPhi     = reco::deltaPhi(jetPair.p2.phi(), met.p4.phi() );
                double j12MetDPhi    = reco::deltaPhi(jetPair.phi(), met.phi() );
                double j12MetMinDPhi = fabs(j1MetDPhi) < fabs(j2MetDPhi) ? j1MetDPhi : j2MetDPhi;

                fwlite::Handle<vector<float> >  h_jet_csv;
                getHandle(h_jet_csv, ev, "JetEdmNtuple", "csv");
                double j1CSV = jetPair.getter(h_jet_csv,1);
                double j2CSV = jetPair.getter(h_jet_csv,2);

                fwlite::Handle<vector<float> >  h_jet_tche;
                getHandle(h_jet_tche, ev, "JetEdmNtuple", "tche");
                double j1TCHE = jetPair.getter(h_jet_tche,1);
                double j2TCHE = jetPair.getter(h_jet_tche,2);
                
                double m1Pt    = muPair.p1.pt();
                double m2Pt    = muPair.p2.pt();
                double m12Mass = muPair.mass();
                double m12Pt   = muPair.pt();
                double m1MetDPhi     = reco::deltaPhi(muPair.p1.phi(), met.p4.phi() );
                double m2MetDPhi     = reco::deltaPhi(muPair.p2.phi(), met.p4.phi() );
                double m12MetDPhi    = reco::deltaPhi(muPair.phi(), met.phi() );
                double m12MetMinDPhi = fabs(m1MetDPhi) < fabs(m2MetDPhi) ? m1MetDPhi : m2MetDPhi;
                
                fwlite::Handle<vector<float> >  h_mu_glbPtError;
                getHandle(h_mu_glbPtError, ev, "MuEdmNtuple", "glbPtError");
                double m1GlbPtError = muPair.getter(h_mu_glbPtError,1);
                double m2GlbPtError = muPair.getter(h_mu_glbPtError,2);
                
                fwlite::Handle<vector<float> >  h_mu_trkPtError;
                getHandle(h_mu_trkPtError, ev, "MuEdmNtuple", "trkPtError");
                double m1TrkPtError = muPair.getter(h_mu_trkPtError,1);
                double m2TrkPtError = muPair.getter(h_mu_trkPtError,2);

                LorentzVector m12j12 = jetPair.p12 + muPair.p12;
                double m12j12DPhi    = reco::deltaPhi(jetPair.phi(), muPair.phi() );
                double m12j12DR      = reco::deltaR(jetPair.p12, muPair.p12 );


                static Polybook evtBook (&plotter);
                evtBook.rewind();
                evtBook("",true);

                evtBook.fill( met.pt(), "met", "; MET [GeV/c]; Events / 2.0", 100, 0., 200. );
                evtBook.fill( met.mEtSig, "metSig", "; MET / #sqrt{ sum E_{T} }; Events / 0.1", 100, 0., 10. );
                evtBook.fill( met.significance, "metSignificance", "; MET significance; Events / 2.0", 100, 0., 200. );
                
                
            //    evtBook.fill( njets, "njets", "; # of jets; Events / 1.0", 15, 0, 15);
	            evtBook.fill( j1Pt, "j1Pt", "; p_{T}(b-jet 1) [GeV/c]; Events / 2.5", 200, 0, 500);
	            evtBook.fill( j2Pt, "j2Pt", "; p_{T}(b-jet 2) [GeV/c]; Events / 2.5", 200, 0, 500);
	            evtBook.fill( jetPair.p1.phi(), "j1Phi", "; #phi(b-jet 1); Events / 0.04", 160, -3.2, 3.2);
	            evtBook.fill( jetPair.p2.phi(), "j2Phi", "; #phi(b-jet 2); Events / 0.04", 160, -3.2, 3.2);
                evtBook.fill( jetPair.p1.eta(), "j1Eta", "; #eta(b-jet 1); Events / 0.04", 150, -3, 3);
                evtBook.fill( jetPair.p2.eta(), "j2Eta", "; #eta(b-jet 2); Events / 0.04", 150, -3, 3);
	            evtBook.fill( j1MetDPhi, "j1MetDeltaPhi", "; #Delta#phi(b-jet 1, MET); Events / 0.04", 160, -3.2, 3.2);
                evtBook.fill( j2MetDPhi, "j2MetDeltaPhi", "; #Delta#phi(b-jet 2, MET); Events / 0.04", 160, -3.2, 3.2);
	            evtBook.fill( j12MetMinDPhi, "jjMetMinDeltaPhi", "; min #Delta#phi to MET(b-jet 1, b-jet 2); Events / 0.04", 160, -3.2, 3.2);
                evtBook.fill( jetPair.deltaEta(), "jjDeltaEta", "; #Delta#eta(b-jet 1, b-jet 2); Events / 0.04", 150, -3, 3);
                evtBook.fill( jetPair.deltaPhi(), "jjDeltaPhi", "; #Delta#phi(b-jet 1, b-jet 2); Events / 0.04", 160, -3.2, 3.2);
                evtBook.fill( jetPair.deltaR(), "jjDeltaR", "; #DeltaR(b-jet 1, b-jet 2) ; Events / 0.025", 200, 0, 5);
                evtBook.fill( jetPair.deltaPt(), "jjDeltaPt" , "; #Delta p_{T}(b-jet 1, b-jet 2) [GeV/c]; Events / 2", 125, 0, 250);
                evtBook.fill( jetPair.sumPt(), "jjSumPt" , "; sum p_{T}(b-jet 1, b-jet 2) [GeV/c]; Events / 2.5", 200, 0., 500. );
                evtBook.fill( j12Mass, "dijetMass", "; invariant mass(dijet) [GeV/c^{2}]; Events / 1.0", 250, 0, 250);
                evtBook.fill( j12Pt, "dijetPt", "; p_{T}(dijet) [GeV/c]; Events / 2.5", 200, 0, 500);
                evtBook.fill( j12MetDPhi, "dijetMetDeltaPhi", "; #Delta#phi(dijet, MET); Events / 0.04", 160, -3.2, 3.2);
                evtBook.fill( fabs(jetPair.pt() - met.pt() ), "dijetMetDeltaPt", "; #Delta p_{T}(dijet, MET) [GeV/c]; Events / 1.0", 125, 0, 250);
                evtBook.fill( min(j1CSV, j2CSV), "jjCSVL", "; min CSV(b-jet 1, b-jet 2); Events / 0.01", 100, 0, 1);
                evtBook.fill( max(j1CSV, j2CSV), "jjCSVH", "; max CSV(b-jet 1, b-jet 2); Events / 0.01", 100, 0, 1);
                evtBook.fill( j1CSV + j2CSV, "jjCSVSum", "; sum CSV(b-jet 1, b-jet 2); Events / 0.02", 100, 0, 2);
                evtBook.fill( min(j1TCHE, j2TCHE), "jjTCHEL", "; min TCHE(b-jet 1, b-jet 2); Events / 0.1", 250, 0, 25);
                evtBook.fill( max(j1TCHE, j2TCHE), "jjTCHEH", "; max TCHE(b-jet 1, b-jet 2); Events / 0.1", 250, 0, 25);
                evtBook.fill( j1TCHE + j2TCHE, "jjTCHESum", "; sum TCHE(b-jet 1, b-jet 2); Events / 0.2", 250, 0, 50);
                
            //    evtBook.fill( nmuons, "nmuons", "; # of muons; Events / 1.0", 15, 0, 15);
	            evtBook.fill( m1Pt, "m1Pt", "; p_{T}(muon 1) [GeV/c]; Events / 2.5", 200, 0, 500);
	            evtBook.fill( m2Pt, "m2Pt", "; p_{T}(muon 2) [GeV/c]; Events / 2.5", 200, 0, 500);
	            evtBook.fill( muPair.p1.phi(), "m1Phi", "; #phi(muon 1); Events / 0.04", 160, -3.2, 3.2);
	            evtBook.fill( muPair.p2.phi(), "m2Phi", "; #phi(muon 2); Events / 0.04", 160, -3.2, 3.2);
                evtBook.fill( muPair.p1.eta(), "m1Eta", "; #eta(muon 1); Events / 0.04", 150, -3, 3);
                evtBook.fill( muPair.p2.eta(), "m2Eta", "; #eta(muon 2); Events / 0.04", 150, -3, 3);
	            evtBook.fill( m1MetDPhi, "m1MetDeltaPhi", "; #Delta#phi(muon 1, MET); Events / 0.04", 160, -3.2, 3.2);
                evtBook.fill( m2MetDPhi, "m2MetDeltaPhi", "; #Delta#phi(muon 2, MET); Events / 0.04", 160, -3.2, 3.2);
	            evtBook.fill( m12MetMinDPhi, "mumuMetMinDeltaPhi", "; min #Delta#phi to MET(muon 1, muon 2); Events / 0.04", 160, -3.2, 3.2);
                evtBook.fill( muPair.deltaEta(), "mumuDeltaEta", "; #Delta#eta(muon 1, muon 2); Events / 0.04", 150, -3, 3);
                evtBook.fill( muPair.deltaPhi(), "mumuDeltaPhi", "; #Delta#phi(muon 1, muon 2); Events / 0.04", 160, -3.2, 3.2);
                evtBook.fill( muPair.deltaR(), "mumuDeltaR", "; #DeltaR(muon 1, muon 2); Events / 0.025", 200, 0, 5);
                evtBook.fill( muPair.deltaPt(), "mumuDeltaPt" , "; #Delta p_{T}(muon 1, muon 2) [GeV/c]; Events / 2", 125, 0, 250);
                evtBook.fill( muPair.sumPt(), "mumuSumPt" , "; sum p_{T}(muon 1, muon 2) [GeV/c]; Events / 2.5", 200, 0., 500. );
                evtBook.fill( m12Mass, "dimuonMass", "; invariant mass(dimuon) [GeV/c^{2}]; Events / 1.0", 250, 0, 250);
                evtBook.fill( m12Pt, "dimuonPt", "; p_{T}(dimuon) [GeV/c]; Events / 2.5", 200, 0, 500);
                evtBook.fill( m12MetDPhi, "dimuonMetDeltaPhi", "; #Delta#phi(dimuon, MET); Events / 0.04", 160, -3.2, 3.2);
                evtBook.fill( fabs(muPair.pt() - met.pt() ), "dimuonMetDeltaPt", "; #Delta p_{T}(dimuon, MET) [GeV/c]; Events / 1.0", 125, 0, 250);
                
                evtBook.fill( m1GlbPtError, "m1GlbPtError", "; global track ptError(muon 1)[GeV]; Events / 0.1", 120, 0, 12);
                evtBook.fill( m2GlbPtError, "m2GlbPtError", "; global track ptError(muon 2)[GeV]; Events / 0.1", 120, 0, 12);
                evtBook.fill( m1GlbPtError/m1Pt, "m1GlbPtErrorFrac", "; global track ptError/pT(muon 1); Events / 0.002", 100, 0, 0.2);
                evtBook.fill( m2GlbPtError/m2Pt, "m2GlbPtErrorFrac", "; global track ptError/pT(muon 2); Events / 0.002", 100, 0, 0.2);
                evtBook.fill( m1TrkPtError, "m1TrkPtError", "; tracker track ptError(muon 1) [GeV]; Events / 0.1", 120, 0, 12);
                evtBook.fill( m2TrkPtError, "m2TrkPtError", "; tracker track ptError(muon 2) [GeV]; Events / 0.1", 120, 0, 12);
                evtBook.fill( m1TrkPtError/m1Pt, "m1TrkPtErrorFrac", "; tracker track ptError/pT(muon 1); Events / 0.002", 100, 0, 0.2);
                evtBook.fill( m2TrkPtError/m2Pt, "m2TrkPtErrorFrac", "; tracker track ptError/pT(muon 2); Events / 0.002", 100, 0, 0.2);
                
                evtBook.fill( m12j12.pt(), "dimuonDijetPt", "; p_{T}(dimuon+dijet) [GeV/c]; Events / 2", 100, 0, 200 );
                evtBook.fill( m12j12.mass(), "dimuonDijetMass", "; invariant mass(dimuon+dijet) [GeV/c^{2}]; Events / 5", 100, 0, 500 );
                evtBook.fill( m12j12DPhi, "dimuonDijetDPhi", "; #Delta#phi(dimuon,dijet); Events / 0.04", 160, -3.2, 3.2);
                evtBook.fill( m12j12DR, "dimuonDijetDR", "; #DeltaR(dimuon,dijet) ; Events / 0.025", 200, 0, 5);
                
                evtBook.fill( (m12j12+met.p4).pt(), "dimuonDijetMetPt", "; p_{T}(dimuon+dijet+MET) [GeV/c]; Events / 2", 100, 0, 200 );
                
	
        	} // end loop over fwlite::Event

            inFile->Close();
            
        } // end loop over if(inFile)

        // break loop if maximal number of events is reached:
        // this has to be done twice to stop the file loop as well
        if(maxEvents_>0 ? int(ievt)+1>maxEvents_ : false)  break;
           
    } // end loop over inputFiles

    int numProcessed = ievt;
    plotter.fill("N/#sigma", numProcessed/crossSection, "statistics", "", 5, 0, 5);
    plotter.fill("N"       , numProcessed             , "statistics", "", 5, 0, 5);
    plotter.fill("#sigma"  , crossSection             , "statistics", "", 5, 0, 5);
    plotter.write(outputFile_.c_str());
    

    return 0;
}
