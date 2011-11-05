################################################################################
# Required packages:
# 
# addpkg RecoJets/Configuration          V02-04-17
# addpkg RecoBTag/SecondaryVertex        V01-07-00
# addpkg RecoVertex/AdaptiveVertexFinder V02-01-00
# addpkg JetMETCorrections/Type1MET      V04-04-04
#
# Revision 
# 2011/11/05: configured for CMSSW_4_2_8_patch3 to run on Summer11 MC datasets
################################################################################


import FWCore.ParameterSet.Config as cms

process = cms.Process("ZmumuHbbSkim")

#-- Setup PAT ------------------------------------------------------------------
from PhysicsTools.PatAlgos.patTemplate_cfg import *
from PhysicsTools.PatAlgos.tools.coreTools import * # import removeMCMatching
from PhysicsTools.PatAlgos.tools.pfTools   import * # import switchJetCollection, switchToPFMET


#-- MC or Data -----------------------------------------------------------------
isData = False


#-- Global Tag -----------------------------------------------------------------
process.GlobalTag.globaltag = cms.string('GR_R_42_V19::All')
if not isData:
    process.GlobalTag.globaltag = cms.string('START42_V13::All')


#-- Message Logger -------------------------------------------------------------
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(True) )
process.MessageLogger.cerr.threshold = ''
process.MessageLogger.cerr.FwkReport.reportEvery = 1000
process.MessageLogger.destinations = cms.untracked.vstring('cout', 'cerr')


#-- Input Source ---------------------------------------------------------------
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
      # "/store/data/Run2011A/METBTag/AOD/PromptReco-v2/000/163/583/B8EB9D41-F572-E011-87A9-0030487A17B8.root",
      # "dcache:/pnfs/cms/WAX/11/store/user/jiafu/VHSkim/data/GluGluToHToZZTo2Nu2B_M-300_3k_events.root",
      # "dcache:/pnfs/cms/WAX/11/store/user/jiafu/VHSkim/data/ZH_ZToNuNu_HToBB_M-115_2k_events.root",
        "dcache:/pnfs/cms/WAX/11/store/user/jiafu/VHSkim/data/ZH_ZToLL_HToBB_M-115_2k_events.root",
        )
    )

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(-1) )

# Make sure to drop whatever MEtoEDM stuff is still around
#process.source.inputCommands = cms.untracked.vstring( "keep *", "drop *_MEtoEDMConverter_*_*" )
#process.source.duplicateCheckMode = cms.untracked.string('checkEachRealDataFile')


### PAT Reconstructions ########################################################

#-- General --------------------------------------------------------------------
# Turn off photon-electron cleaning (i.e., flag only)
process.cleanPatPhotons.checkOverlaps.electrons.requireNoOverlaps = False

# Embed tracks, since we drop them
#process.patElectrons.embedTrack = True
#process.patMuons.embedTrack   = True


#-- Data/MC specifics ----------------------------------------------------------
if isData:
    removeMCMatching(process, ['All'])

if not isData:
    process.patJets.addPartonJetMatch = True        # Jet Parton Match

removeAllPATObjectsBut(process, ['METs', 'Jets', 'Electrons', 'Muons'])


#-- Muons ----------------------------------------------------------------------
process.selectedPatMuons.cut = (
    "pt > 20 && abs(eta) < 2.4 && isGlobalMuon && isTrackerMuon && "    +
    "globalTrack().normalizedChi2 < 10 &&"                              +
    "innerTrack().hitPattern().numberOfValidTrackerHits > 10 && "       +
    "innerTrack().hitPattern().numberOfValidPixelHits > 0 && "          +
    "globalTrack().hitPattern().numberOfValidMuonHits > 0 && "          +
    "abs(dB) < 0.2 && "                                                 +
    "(trackIso + caloIso) < 0.15 * pt && "                              +
    "numberOfMatches > 1"
    )


#-- Electrons ------------------------------------------------------------------
# Cut-based Electron ID in the VBTF prescription
process.load("ElectroWeakAnalysis.WENu.simpleEleIdSequence_cff")
process.patElectrons.electronIDSources = cms.PSet(
    eidTight = cms.InputTag("eidTight"),
    eidLoose = cms.InputTag("eidLoose"),
    eidRobustTight = cms.InputTag("eidRobustTight"),
    eidRobustHighEnergy = cms.InputTag("eidRobustHighEnergy"),
    eidRobustLoose = cms.InputTag("eidRobustLoose"),
    simpleEleId95relIso= cms.InputTag("simpleEleId95relIso"),
    simpleEleId90relIso= cms.InputTag("simpleEleId90relIso"),
    simpleEleId85relIso= cms.InputTag("simpleEleId85relIso"),
    simpleEleId80relIso= cms.InputTag("simpleEleId80relIso"),
    simpleEleId70relIso= cms.InputTag("simpleEleId70relIso"),
    simpleEleId60relIso= cms.InputTag("simpleEleId60relIso"),
    simpleEleId95cIso= cms.InputTag("simpleEleId95cIso"),
    simpleEleId90cIso= cms.InputTag("simpleEleId90cIso"),
    simpleEleId85cIso= cms.InputTag("simpleEleId85cIso"),
    simpleEleId80cIso= cms.InputTag("simpleEleId80cIso"),
    simpleEleId70cIso= cms.InputTag("simpleEleId70cIso"),
    simpleEleId60cIso= cms.InputTag("simpleEleId60cIso")
    )

process.patDefaultSequence.replace(process.patElectrons,process.simpleEleIdSequence+process.patElectrons)

process.selectedPatElectrons.cut = ( 
    "pt > 20 && abs(eta) < 2.5 &&"   +
    "(isEE || isEB) && !isEBEEGap &&"  +
  # "gsfTrackRef().isNonnull && gsfTrackRef().trackerExpectedHitsInner().numberOfHits <= 0 &&" +
    "electronID('simpleEleId95cIso') == 7" 
    )


#-- MET ------------------------------------------------------------------------
# Switch to using PFMET 
switchToPFMET(process, cms.InputTag('pfMet'), "")

# Add PFMET Type-1 corrections
from JetMETCorrections.Type1MET.MetType1Corrections_cff import metJESCorAK5PFJet
process.metJESCorAK5PF = metJESCorAK5PFJet.clone(
    inputUncorMetLabel  = 'pfMet',
    inputUncorJetsLabel = 'ak5PFJets',
    metType             = 'PFMET',
    useTypeII           = False,
    jetPTthreshold      = cms.double(10.0),
    )

process.patMETsType1 = process.patMETs.clone(
    metSource = cms.InputTag("metJESCorAK5PF")
    )

process.patDefaultSequence.replace(process.patMETs, process.patMETs+process.metJESCorAK5PF+process.patMETsType1)


#-- Jets -----------------------------------------------------------------------

# L1 FastJet jet corrections
# kt jets for fastjet corrections (needed for CMSSW < 4_2_0)
process.load('RecoJets.Configuration.RecoPFJets_cff')
process.load('JetMETCorrections.Configuration.DefaultJEC_cff')
    
# Compute areas for Fastjet PU subtraction
process.kt6PFJets.doRhoFastjet = True
#process.kt6PFJets.doAreaFastjet = True
#process.kt6PFJets.Rho_EtaMax = cms.double(3.0)
#process.kt6PFJets.Ghost_EtaMax = cms.double(5.0)
    
process.ak5PFJets.doAreaFastjet = True
#process.ak5PFJets.doRhoFastjet = False
#process.ak5PFJets.Rho_EtaMax = cms.double(3.0)
    
#process.patJetCorrFactors.levels = cms.vstring('L1FastJet', 'L2Relative', 'L3Absolute')
#process.patJetCorrFactors.rho = cms.InputTag('kt6PFJets', 'rho')
    
process.patDefaultSequence.replace(process.patJetCorrFactors, process.kt6PFJets + process.ak5PFJets + process.patJetCorrFactors)

# Switch to PFJets
if isData:
    switchJetCollection(process,cms.InputTag('ak5PFJets'),
        doJTA        = True,
        doBTagging   = True,
        jetCorrLabel = ('AK5PF', ['L1Offset', 'L2Relative', 'L3Absolute', 'L2L3Residual']),
      # jetCorrLabel = ('AK5PF', ['L2Relative', 'L3Absolute', 'L2L3Residual'])
        doType1MET   = True,
        genJetCollection=cms.InputTag(""),
        doJetID      = True
        )

if not isData: 
    switchJetCollection(process,cms.InputTag('ak5PFJets'),
        doJTA        = True,
        doBTagging   = True,
        jetCorrLabel = ('AK5PF', ['L1FastJet', 'L2Relative', 'L3Absolute']),
      # jetCorrLabel = ('AK5PF', ['L2Relative', 'L3Absolute'])
        doType1MET   = True,  # not sure about this
        genJetCollection=cms.InputTag("ak5GenJets"),
        doJetID      = True
        )

process.patJets.addTagInfos = True
process.patJets.tagInfoSources = cms.VInputTag(
    cms.InputTag("secondaryVertexTagInfosAOD"),
    )

process.patJetGenJetMatch.maxDPtRel = cms.double(10.0)
process.patJetGenJetMatch.maxDeltaR = cms.double(0.6)

# Clean the Jets from the selected leptons, applying here only pt cut pf 20 
process.cleanPatJets = cms.EDProducer("PATJetCleaner",
    src = cms.InputTag("selectedPatJets"),
    #preselection = cms.string('pt > 20.0 && abs(eta) < 2.5 && ( (neutralEmEnergy/energy < 0.99) &&  (neutralHadronEnergy/energy < 0.99) && numberOfDaughters>1) '),
    preselection = cms.string("pt > 20.0"),
    checkOverlaps = cms.PSet(
        muons = cms.PSet(
            src       = cms.InputTag("selectedPatMuons"),
            algorithm = cms.string("byDeltaR"),
            preselection        = cms.string(""),
            deltaR              = cms.double(0.5),
            checkRecoComponents = cms.bool(False),
            pairCut             = cms.string(""),
            requireNoOverlaps   = cms.bool(True),
            ),
        electrons = cms.PSet(
            src       = cms.InputTag("selectedPatElectrons"),
            algorithm = cms.string("byDeltaR"),
            preselection        = cms.string(""),
            deltaR              = cms.double(0.5),
            checkRecoComponents = cms.bool(False),
            pairCut             = cms.string(""),
            requireNoOverlaps   = cms.bool(True),
            ),
        ),
    finalCut = cms.string('')
    )


# Select pat jets, applying eta loose ID and b-tag
process.cleanCentralPatJets = cms.EDFilter("PATJetSelector",
    src = cms.InputTag("cleanPatJets"),
    cut = cms.string("abs(eta)< 2.5" )
    #cut = cms.string("abs(eta)< 2.5 & pt > 20.0 && bDiscriminator(\"trackCountingHighEffBJetTags\")>1.7 " ) ### try wit very loose b-tag now..... 
    )
    
#-- Primary Vertices -----------------------------------------------------------

# Get a list of good primary vertices, in 42x, these are DAF vertices
#from PhysicsTools.SelectorUtils.pvSelector_cfi import pvSelector
process.goodOfflinePrimaryVertices = cms.EDFilter("VertexSelector",
    src = cms.InputTag("offlinePrimaryVertices"),
    cut = cms.string("!isFake && ndof >= 4 && abs(z) <= 24 && abs(position.rho) <= 2"),
    filter = cms.bool(False),
    )


### Event Filters ##############################################################

process.electronFilter = cms.EDFilter("PATCandViewCountFilter",
    src = cms.InputTag("selectedPatElectrons"),
    minNumber = cms.uint32(1),
    maxNumber = cms.uint32(999999),
    )

process.muonFilter = cms.EDFilter("PATCandViewCountFilter",
    src = cms.InputTag("selectedPatMuons"),
    minNumber = cms.uint32(2),
    maxNumber = cms.uint32(999999),
    )

process.zToMuMu = cms.EDProducer("CandViewShallowCloneCombiner",
    decay = cms.string("selectedPatMuons@+ selectedPatMuons@-"),
    cut = cms.string("10 < mass < 190"),
    )

process.zToMuMuFilter = cms.EDFilter("PATCandViewCountFilter",
    src = cms.InputTag("zToMuMu"),
    minNumber = cms.uint32(1),
    maxNumber = cms.uint32(1),
    )

process.jetFilter = cms.EDFilter("PATCandViewCountFilter",
    src = cms.InputTag("cleanCentralPatJets"),
    minNumber = cms.uint32(2),
    maxNumber = cms.uint32(2),
    )

process.jetCSVLooseFilter = cms.EDFilter("PATJetBDiscriminatorFilter",
    src = cms.InputTag("cleanCentralPatJets"),
    disc = cms.string("combinedSecondaryVertexBJetTags"),
    discCut = cms.double(0.5),
    minNumber = cms.uint32(2),
    )

process.jetCSVTightFilter = process.jetCSVLooseFilter.clone(
    discCut = cms.double(0.898),
    minNumber = cms.uint32(1),
    )

process.noNullGenJetFilter = cms.EDFilter("PATJetMCMatchFilter",
    src = cms.InputTag("cleanCentralPatJets")
    )

process.noNullGenMuonFilter = cms.EDFilter("PATMuonMCMatchFilter",
    src = cms.InputTag("selectedPatMuons")
    )

process.eventFilter = cms.Sequence(
    ~process.electronFilter     +
    process.muonFilter          +
    process.zToMuMu             +
    process.zToMuMuFilter       +
    process.jetFilter           +
    process.jetCSVLooseFilter   +
    process.jetCSVTightFilter   +
    process.noNullGenJetFilter  +
    process.noNullGenMuonFilter
    )
    

### Cleaning ##################################################################

# Apply loose PF jet ID
from PhysicsTools.SelectorUtils.pfJetIDSelector_cfi import pfJetIDSelector
process.goodPatJets = cms.EDFilter("PFJetIDSelectionFunctorFilter",
                                   filterParams = pfJetIDSelector.clone(),
                                   src = cms.InputTag("selectedPatJets"),
                                   filter = cms.bool(True)
                                   )

# HB + HE noise filtering
process.load('CommonTools/RecoAlgos/HBHENoiseFilter_cfi')
#process.HBHENoiseFilter.minIsolatedNoiseSumE = cms.double(999999.)
#process.HBHENoiseFilter.minNumIsolatedNoiseChannels = cms.int32(999999.)
#process.HBHENoiseFilter.minIsolatedNoiseSumEt = cms.double(999999.)

# ECAL noise filtering
#process.load('JetMETAnalysis.ecalDeadCellTools.EcalDeadCellEventFilter_cfi')

# ECAL Severity Level
process.load("RecoLocalCalo.EcalRecAlgos.EcalSeverityLevelESProducer_cfi")

### Ntupler ####################################################################

process.load("VHbbAnalysis.ZmumuHbbSkim.ZmumuHbbEdmNtuples_cff")

### Paths ######################################################################

#process.eventCountProducer = cms.EDProducer("EventCountProducer")

process.p = cms.Path(
    process.patDefaultSequence  +
    process.cleanPatJets        +
    process.cleanCentralPatJets +
    process.eventFilter         *
    process.MetEdmNtuple        *
    process.MetType1EdmNtuple   *
    process.MuEdmNtuple         *
    process.JetEdmNtuple        
    )

#process.options = cms.untracked.PSet( SkipEvent = cms.untracked.vstring('ProductNotFound') )


#-- Output Module --------------------------------------------------------------
process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('ZmumuHbbEdmNtuples.root'),
    outputCommands = cms.untracked.vstring(
        "drop *",
        "keep *_MetEdmNtuple_*_*",
        "keep *_MetType1EdmNtuple_*_*",
        "keep *_MuEdmNtuple_*_*",
        "keep *_JetEdmNtuple_*_*",
        ),
    SelectEvents = cms.untracked.PSet(
        SelectEvents = cms.vstring("p")
        ),
    dropMetaData = cms.untracked.string("ALL"),
    )

process.endPath = cms.EndPath(process.out)

