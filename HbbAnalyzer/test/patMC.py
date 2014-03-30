# ==============================================================================
# VHbbAnalysis
#   https://twiki.cern.ch/twiki/bin/view/CMS/VHbbAnalysisNewCode
#
# CMS Official Recommendations
#   https://twiki.cern.ch/twiki//bin/view/CMS/Internal/ApprovedObjects
#
# PF2PAT+PAT (a.k.a. PFBRECO)
#   https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePATPFBRECOExercise
#   https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePATTools
# ==============================================================================

import FWCore.ParameterSet.Config as cms
process = cms.Process("VH")


# ------------------------------------------------------------------------------
# MC or Data
# ------------------------------------------------------------------------------
RUN_ON_MC = True

# ------------------------------------------------------------------------------
# Global Tag
#   https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideFrontierConditions
#   TWiki revision: ?
# ------------------------------------------------------------------------------
# TODO: Please check if these are the HCP global tags, or update to the latest Global Tags

GLOBALTAGS = {
    "Summer12_DR53X"                : "START53_V7E::All",
    "Run2012A-13Jul2012-v1"         : "FT_53_V6_AN2::All",  # "GR_P_V40_AN1::All" was used for HCP?
    "Run2012A-recover-06Aug2012-v1" : "FT_53_V6C_AN2::All", # "GR_P_V40_AN1::All" was used for HCP?
    "Run2012B-13Jul2012-v1"         : "FT_53_V6_AN2::All",  # "GR_P_V40_AN1::All" was used for HCP?
    "Run2012C-24Aug2012-v1"         : "FT_53_V10_AN2::All",
    "Run2012C-PromptReco-v1"        : "GR_P_V40_AN1::All",
    "Run2012C-PromptReco-v2"        : "GR_P_V41_AN2::All",
    "Run2012D-PromptReco-v1"        : "GR_P_V42_AN4::All",
    }

GLOBALTAG = ""
if RUN_ON_MC:
    GLOBALTAG = GLOBALTAGS["Summer12_DR53X"]
else:
    GLOBALTAG = GLOBALTAGS["Run2012D-PromptReco-v1"]  # check before use


################################################################################
# PAT Skeleton                                                                 #
# taken from PhysicsTools/PatAlgos/python/patTemplate_cfg.py V08-09-07-05      #
################################################################################

## MessageLogger
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100

## Options and Output Report
process.options   = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )

## Source
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        #"/store/mc/Summer12_DR53X/ZH_ZToNuNu_HToBB_M-125_8TeV-powheg-herwigpp/AODSIM/PU_S10_START53_V7A-v1/0000/046ED31B-82FC-E111-BA0E-00215E220F78.root",
        "/store/user/lpchbb/degrutto/ZH_ZToNuNu_HToBB_M-125_8TeV-powheg-herwigppSummer12_DR53X-PU_S10_START53_V7A-v1/degrutto/ZH_ZToNuNu_HToBB_M-125_8TeV-powheg-herwigpp/HBB_EDMNtupleV42/9803889241b1fc304f795d3b3875632d/PAT.edm_11_1_lbp.root",
        )
    )

## Maximal Number of Events
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(3000) )

## Geometry and Detector Conditions (needed for a few patTuple production steps)
#process.load("Configuration.StandardSequences.Geometry_cff")
process.load("Configuration.Geometry.GeometryIdeal_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
#from Configuration.AlCa.autoCond import autoCond
#process.GlobalTag.globaltag = cms.string( autoCond[ 'startup' ] )
process.GlobalTag.globaltag = cms.string( GLOBALTAG )
process.load("Configuration.StandardSequences.MagneticField_cff")

## Test JEC from test instances of the global DB
#process.load("PhysicsTools.PatAlgos.patTestJEC_cfi")

## Test JEC from local sqlite file
#process.load("PhysicsTools.PatAlgos.patTestJEC_local_cfi")

## Standard PAT Configuration File
#process.load("PhysicsTools.PatAlgos.patSequences_cff")

## Output Module Configuration (expects a path 'p')
from PhysicsTools.PatAlgos.patEventContent_cff import patEventContent
process.out = cms.OutputModule("PoolOutputModule",
                               fileName = cms.untracked.string('patTuple.root'),
                               # save only events passing the full path
                               SelectEvents   = cms.untracked.PSet( SelectEvents = cms.vstring('p') ),
                               # save PAT Layer 1 output; you need a '*' to
                               # unpack the list of commands 'patEventContent'
                               outputCommands = cms.untracked.vstring('drop *', *patEventContent )
                               )

process.outpath = cms.EndPath(process.out)
################################################################################
# END PAT Skeleton                                                             #
################################################################################

# ------------------------------------------------------------------------------
# Primary Vertex
# ------------------------------------------------------------------------------
# Create good primary vertices to be used for PF association
from PhysicsTools.SelectorUtils.pvSelector_cfi import pvSelector

process.goodOfflinePrimaryVertices = cms.EDFilter(
    "PrimaryVertexObjectFilter",
    filterParams = pvSelector.clone( minNdof = cms.double(4.0), maxZ = cms.double(24.0) ),
    src = cms.InputTag('offlinePrimaryVertices')
    )

# ------------------------------------------------------------------------------
# Jet Energy Correction Label
# ------------------------------------------------------------------------------
inputJetCorrLabel = None
if RUN_ON_MC:
    inputJetCorrLabel = ['L1FastJet', 'L2Relative', 'L3Absolute']
else:
    inputJetCorrLabel = ['L1FastJet', 'L2Relative', 'L3Absolute', 'L2L3Residual']


################################################################################
# PF2PAT+PAT (a.k.a. PFBRECO)                                                  #
# taken from PhysicsTools/PatAlgos/test/patTuple_PATandPF2PAT_cfg.py V08-09-23 #
################################################################################

# load the PAT config
process.load("PhysicsTools.PatAlgos.patSequences_cff")


# Configure PAT to use PF2PAT instead of AOD sources
# this function will modify the PAT sequences.
from PhysicsTools.PatAlgos.tools.pfTools import *

# An empty postfix means that only PF2PAT is run,
# otherwise both standard PAT and PF2PAT are run. In the latter case PF2PAT
# collections have standard names + postfix (e.g. patElectronPFlow)
postfix = "PFlow"
jetAlgo = "AK5"
usePF2PAT(process,runPF2PAT=True, jetAlgo=jetAlgo, runOnMC=RUN_ON_MC, postfix=postfix)

# Uncomment this block to use the proper JEC labels and use
# goodOfflinePrimaryVertices instead of offlinePrimaryVertices
#usePF2PAT(process,runPF2PAT=True,
#    jetAlgo=jetAlgo, runOnMC=RUN_ON_MC, postfix=postfix,
#    jetCorrections=('AK5PFchs', inputJetCorrLabel),
#    pvCollection=cms.InputTag('goodOfflinePrimaryVertices'),
#    typeIMetCorrections=False,
#    )

# to use particle-based isolation in patDefaultSequence
usePFIso( process )

# to turn on type-1 MET corrections, use the following call
#usePF2PAT(process,runPF2PAT=True, jetAlgo=jetAlgo, runOnMC=RUN_ON_MC, postfix=postfix, typeIMetCorrections=True)

# to run second PF2PAT+PAT with different postfix uncomment the following lines
# and add the corresponding sequence to path
#postfix2 = "PFlow2"
#jetAlgo2="AK7"
#usePF2PAT(process,runPF2PAT=True, jetAlgo=jetAlgo2, runOnMC=RUN_ON_MC, postfix=postfix2)

# to use tau-cleaned jet collection uncomment the following:
#getattr(process,"pfNoTau"+postfix).enable = True

# to switch default tau (HPS) to old default tau (shrinking cone) uncomment
# the following:
# note: in current default taus are not preselected i.e. you have to apply
# selection yourself at analysis level!
#adaptPFTaus(process,"shrinkingConePFTau",postfix=postfix)

# to use GsfElectrons instead of PF electrons
#useGsfElectrons(process,postfix,"03") # to change isolation cone size to 0.3 as it is recommended by EGM POG, use "04" for cone size 0.4

if not RUN_ON_MC:
    # removing MC matching for standard PAT sequence
    # for the PF2PAT+PAT sequence, it is done in the usePF2PAT function
    removeMCMatchingPF2PAT( process, '' )

# top projections in PF2PAT:
getattr(process,"pfNoPileUp"+postfix).enable = True
getattr(process,"pfNoMuon"+postfix).enable = False  # do traditional cleaning
getattr(process,"pfNoElectron"+postfix).enable = False  # do traditional cleaning
getattr(process,"pfNoTau"+postfix).enable = False
getattr(process,"pfNoJet"+postfix).enable = True

# verbose flags for the PF2PAT modules
#getattr(process,"pfNoMuon"+postfix).verbose = False

# enable delta beta correction for muon selection in PF2PAT?
#getattr(process,"pfIsolatedMuons"+postfix).doDeltaBetaCorrection = False
################################################################################
# END PF2PAT+PAT                                                               #
################################################################################

# ------------------------------------------------------------------------------
# Switch to PF jets in patDefaultSequence
# ------------------------------------------------------------------------------
switchJetCollection(process,
    jetCollection    = cms.InputTag('pfJetsPFlow'),
    doJTA            = True,
    doBTagging       = True,
    jetCorrLabel     = ('AK5PFchs', inputJetCorrLabel),
    doType1MET       = False,
    genJetCollection = (cms.InputTag("ak5GenJets") if RUN_ON_MC else None),
    doJetID          = False,
    jetIdLabel       = "ak5",
    )
process.patJets.addTagInfos = True

# ------------------------------------------------------------------------------
# PF Jets with charged hadron subtraction (a.k.a. PFnoPU)
#   https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections#JetEnCorPFnoPU2012
# ------------------------------------------------------------------------------
# NB: current JetMET version of PFNoPU leaves about 20% pile-up charged hadrons
#     untouched. This compromise was necessary to avoid over-subtracting high
#     pT tracks from jets. The L1chs corrections account for these remaining
#     pile-up charged hadrons so the following settings must be enabled (these
#     are different from lepton isolation settings).
process.pfPileUpPFlow.Vertices = cms.InputTag('goodOfflinePrimaryVertices')
process.pfPileUpPFlow.checkClosestZVertex = False


################################################################################
# Lepton                                                                       #
################################################################################
# TODO: Review both electron and muon isolation
# TODO: Add/Modify tau ID
# Apparently we are using detector-based reconstruction for electrons and muons
# from PAT (not from PF2PAT), but PF-based isolation (due to usePFIso)

# ------------------------------------------------------------------------------
# Electron ID
#   https://twiki.cern.ch/twiki/bin/view/CMS/EgammaIDRecipes
#   https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentification
# ------------------------------------------------------------------------------
# Cut-based Spring10
import VHbbAnalysis.HbbAnalyzer.simpleCutBasedElectronIDSpring10_cfi as vbtfid
process.eidVBTFRel95 = vbtfid.simpleCutBasedElectronID.clone( electronQuality = '95relIso' )
process.eidVBTFRel85 = vbtfid.simpleCutBasedElectronID.clone( electronQuality = '85relIso' )
process.eidVBTFRel80 = vbtfid.simpleCutBasedElectronID.clone( electronQuality = '80relIso' )
process.eidVBTFRel70 = vbtfid.simpleCutBasedElectronID.clone( electronQuality = '70relIso' )
process.eidVBTFCom95 = vbtfid.simpleCutBasedElectronID.clone( electronQuality = '95cIso'   )
process.eidVBTFCom85 = vbtfid.simpleCutBasedElectronID.clone( electronQuality = '85cIso'   )
process.eidVBTFCom80 = vbtfid.simpleCutBasedElectronID.clone( electronQuality = '80cIso'   )
process.eidVBTFCom70 = vbtfid.simpleCutBasedElectronID.clone( electronQuality = '70cIso'   )
process.eidSequence = cms.Sequence(
    process.eidVBTFRel95 +
    process.eidVBTFRel85 +
    process.eidVBTFRel80 +
    process.eidVBTFRel70 +
    process.eidVBTFCom95 +
    process.eidVBTFCom85 +
    process.eidVBTFCom80 +
    process.eidVBTFCom70
    )

# MVA 2012
#process.load('EGamma.EGammaAnalysisTools.electronIdMVAProducer_cfi')
process.load('EgammaAnalysis.ElectronTools.electronIdMVAProducer_cfi')
process.mvaID = cms.Sequence( process.mvaTrigV0 + process.mvaTrigNoIPV0 + process.mvaNonTrigV0 )

# ID Sources
process.patElectrons.electronIDSources = cms.PSet(
    # Cut-based Spring10
    eidVBTFRel95 = cms.InputTag("eidVBTFRel95"),
    eidVBTFRel85 = cms.InputTag("eidVBTFRel85"),
    eidVBTFRel80 = cms.InputTag("eidVBTFRel80"),
    eidVBTFRel70 = cms.InputTag("eidVBTFRel70"),
    eidVBTFCom95 = cms.InputTag("eidVBTFCom95"),
    eidVBTFCom85 = cms.InputTag("eidVBTFCom85"),
    eidVBTFCom80 = cms.InputTag("eidVBTFCom80"),
    eidVBTFCom70 = cms.InputTag("eidVBTFCom70"),
    # MVA 2012
    mvaTrigV0 = cms.InputTag("mvaTrigV0"),
    mvaTrigNoIPV0 = cms.InputTag("mvaTrigNoIPV0"),
    mvaNonTrigV0 = cms.InputTag("mvaNonTrigV0"),
    )

# ------------------------------------------------------------------------------
# Muon ID
#   https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideMuonId
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# Tau ID
#   https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePFTauID
#   https://twiki.cern.ch/twiki/bin/view/CMS/TauIDRecommendation
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# Electron Isolation
#   https://twiki.cern.ch/twiki/bin/view/CMS/EgammaEARhoCorrection
#   https://twiki.cern.ch/twiki/bin/view/CMS/EgammaPFBasedIsolation
# ------------------------------------------------------------------------------
# NB: Electron isolation cone size of 0.4 (default) is used. POG supports both
#     0.3 and 0.4. The WPxx definitions were based on the cone of 0.4.
#     Effective area numbers were changed to be consistent.
process.patElectrons.isolationValues = cms.PSet(
    pfNeutralHadrons = cms.InputTag("elPFIsoValueNeutral04PFIdPFIso"),
    pfChargedAll = cms.InputTag("elPFIsoValueChargedAll04PFIdPFIso"),
    pfPUChargedHadrons = cms.InputTag("elPFIsoValuePU04PFIdPFIso"),
    pfPhotons = cms.InputTag("elPFIsoValueGamma04PFIdPFIso"),
    pfChargedHadrons = cms.InputTag("elPFIsoValueCharged04PFIdPFIso"),
    )

process.patElectrons.isolationValuesNoPFId = cms.PSet(
    pfNeutralHadrons = cms.InputTag("elPFIsoValueNeutral04NoPFIdPFIso"),
    pfChargedAll = cms.InputTag("elPFIsoValueChargedAll04NoPFIdPFIso"),
    pfPUChargedHadrons = cms.InputTag("elPFIsoValuePU04NoPFIdPFIso"),
    pfPhotons = cms.InputTag("elPFIsoValueGamma04NoPFIdPFIso"),
    pfChargedHadrons = cms.InputTag("elPFIsoValueCharged04NoPFIdPFIso"),
    )

# ------------------------------------------------------------------------------
# Muon Isolation
# ------------------------------------------------------------------------------
# NB: Muon isolation cone size of 0.4 (default) is used. This agrees with POG recommendation.
# NB: Ignore the above statement. Actually patMuonsPFlow is never used!
#process.patMuonsPFlow.isolationValues.user = cms.VInputTag(
#    cms.InputTag("muPFIsoValueChargedAll03"+postfix),
#    cms.InputTag("muPFIsoValueCharged03"+postfix),
#    cms.InputTag("muPFIsoValueNeutral03"+postfix),
#    cms.InputTag("muPFIsoValueGamma03"+postfix),
#    cms.InputTag("muPFIsoValuePU03"+postfix),
#    cms.InputTag("muPFIsoValuePU04"+postfix),
#    )

# ------------------------------------------------------------------------------
# Cut
# ------------------------------------------------------------------------------
process.selectedPatElectrons.cut = cms.string(
    "(ecalDrivenSeed==1) &&"
    "pt > 5.0 && abs(eta) < 2.5 &&"
    "(isEE || isEB) && !isEBEEGap"
    )

process.selectedPatMuons.cut = cms.string("")


################################################################################
# Jet                                                                          #
################################################################################
# TODO: Check rho25
# TODO: Remove kt6PFJetsForIsolation, kt6PFJetsCentralNeutral
# TODO: Review jet cleaning. What about selectedPatJetsAK7PF cleaning?
# TODO: Update pileup jet ID to the stable version
# TODO: Update JEC to final 53X set

from PhysicsTools.PatAlgos.tools.jetTools import *

# ------------------------------------------------------------------------------
# Info
#   https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections
#   https://twiki.cern.ch/twiki/bin/view/CMS/JetResolution
#   https://twiki.cern.ch/twiki/bin/view/CMS/JECDataMC
#   https://twiki.cern.ch/twiki/bin/view/CMS/JECUncertaintySources
#   https://twiki.cern.ch/twiki/bin/view/CMS/JetID
#   https://twiki.cern.ch/twiki/bin/view/CMS/PileupJetID
#   http://pandolf.web.cern.ch/pandolf/instr/QGLikelihood.txt Quark/Gluon discriminator
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# Energy density
# ------------------------------------------------------------------------------
# Compute the mean pt per unit area (rho) in |eta| < 2.5
# taken from RecoJets/Configuration/python/RecoPFJets_cff.py
from RecoJets.JetProducers.kt4PFJets_cfi import kt4PFJets
process.kt6PFJets25 = kt4PFJets.clone(
    src              = cms.InputTag("pfNoElectron"+postfix),
    rParam           = 0.6,
    doRhoFastjet     = True,
    #doAreaFastjet    = True,  # default is False
    Ghost_EtaMax     = 2.5,
    Rho_EtaMax       = 2.5,
    )

# For CMSSW_5XY, this should be taken from RECO or AOD: double_kt6PFJets_rho_RECO
process.kt6PFJetsForIsolation = kt4PFJets.clone(
    rParam           = 0.6,
    doRhoFastjet     = True,
    #doAreaFastjet    = True,  # default is False
    #Ghost_EtaMax     = 2.5,  # default is 5.0
    Rho_EtaMax       = 2.5,
    #voronoiRfact     = 0.9,  # default is -0.9
    )

# For CMSSW_5XY, this should be taken from RECO or AOD: double_kt6PFJetsCentralNeutral_rho_RECO
process.kt6PFJetsCentralNeutral = kt4PFJets.clone(
    src              = cms.InputTag("pfAllNeutralHadronsAndPhotons"+postfix),
    rParam           = 0.6,
    doRhoFastjet     = True,
    doAreaFastjet    = True,
    Ghost_EtaMax     = 3.1,
    Rho_EtaMax       = 2.5,
    inputEtMin       = 0.5,
    #voronoiRfact     = 0.9,  # default is -0.9
    )

from RecoJets.JetProducers.ak5PFJets_cfi import ak5PFJets
process.ak7PFJets = ak5PFJets.clone(  # clone from process.ak5PFJets instead?
    rParam           = 0.7,
    )

addJetCollection(process, cms.InputTag("ak7PFJets"),
    "AK7", "PF",
    doJTA            = True,
    doBTagging       = True,
    jetCorrLabel     = ("AK7PF", inputJetCorrLabel),
    doType1MET       = False,
    doL1Cleaning     = False,
    doL1Counters     = False,
    genJetCollection = (cms.InputTag("ak7GenJets") if RUN_ON_MC else None),
    doJetID          = False,
    jetIdLabel       = "ak7",
    #btagdiscriminators = [
    #    'trackCountingHighPurBJetTags','trackCountingHighEffBJetTags',
    #    'jetProbabilityBJetTags', 'jetBProbabilityBJetTags',
    #    'simpleSecondaryVertexHighEffBJetTags','simpleSecondaryVertexHighPurBJetTags',
    #    'combinedSecondaryVertexBJetTags','combinedSecondaryVertexMVABJetTags',
    #    'combinedInclusiveSecondaryVertexBJetTags','combinedMVABJetTags'
    #    ],
    )

# ------------------------------------------------------------------------------
# Jet Substructure
#   https://twiki.cern.ch/twiki/bin/viewauth/CMS/JetMETAlgorithmsReconstruction
#   https://indico.cern.ch/getFile.py/access?contribId=3&resId=0&materialId=slides&confId=208789
# ------------------------------------------------------------------------------
#process.load("RecoJets.Configuration.GenJetParticles_cff")

#from RecoJets.JetProducers.ca4GenJets_cfi import ca4GenJets
#process.ca4GenJets = ca4GenJets
#process.ca8GenJets = ca4GenJets.clone(rParam=cms.double(0.8))

#process.load('RecoJets.JetProducers.caSubjetFilterPFJets_cfi')
from RecoJets.JetProducers.caSubjetFilterPFJets_cfi import caSubjetFilterPFJets
process.caVHPFJets = caSubjetFilterPFJets.clone(
    src = cms.InputTag("pfNoElectron"+postfix),
    useAdjacency = cms.int32(0)
    )

#process.load('RecoJets.JetProducers.caSubjetFilterGenJets_cfi')
from RecoJets.JetProducers.caSubjetFilterGenJets_cfi import caSubjetFilterGenJets
process.caVHGenJets = caSubjetFilterGenJets.clone()

# NB: caVHCaloJets can be removed safely
#process.load('RecoJets.JetProducers.caSubjetFilterCaloJets_cfi')
from RecoJets.JetProducers.caSubjetFilterCaloJets_cfi import caSubjetFilterCaloJets
process.caVHCaloJets = caSubjetFilterCaloJets.clone(
    useAdjacency = cms.int32(0)
    )

addJetCollection(process, cms.InputTag("caVHPFJets:fat"),
    "CAVHFat", "PF",
    doJTA            = True,
    doBTagging       = True,
    jetCorrLabel     = ("AK5PF", inputJetCorrLabel),
    doType1MET       = False,
    doL1Cleaning     = False,
    doL1Counters     = False,
    doJetID          = False,
    jetIdLabel       = "ak5",
    )

addJetCollection(process, cms.InputTag("caVHPFJets:sub"),
    "CAVHSub", "PF",
    doJTA            = True,
    doBTagging       = True,
    jetCorrLabel     = ("AK5PF", inputJetCorrLabel),
    doType1MET       = False,
    doL1Cleaning     = False,
    doL1Counters     = False,
    genJetCollection = (cms.InputTag("caVHGenJets:sub") if RUN_ON_MC else None),
    doJetID          = False,
    jetIdLabel       = "ak5",
    )

addJetCollection(process, cms.InputTag("caVHPFJets:filter"),
    "CAVHFilter","PF",
    doJTA            = True,
    doBTagging       = True,
    jetCorrLabel     = ("AK5PF", inputJetCorrLabel),
    doType1MET       = False,
    doL1Cleaning     = False,
    doL1Counters     = False,
    genJetCollection = (cms.InputTag("caVHGenJets:filter") if RUN_ON_MC else None),
    doJetID          = False,
    jetIdLabel       = "ak5",
    )

# ------------------------------------------------------------------------------
# Jet Substructure (FastJet 3)
# ------------------------------------------------------------------------------
from RecoJets.JetProducers.ca4GenJets_cfi import ca4GenJets
from RecoJets.JetProducers.ca4PFJets_cfi import ca4PFJets

from RecoJets.JetProducers.ak5PFJetsPruned_cfi import ak5PFJetsPruned
ak5PrunedPFlow = ak5PFJetsPruned.clone(doAreaFastjet = cms.bool(True))

from RecoJets.JetProducers.ak5PFJetsFiltered_cfi import ak5PFJetsFiltered
ak5FilteredPFlow = ak5PFJetsFiltered.clone(doAreaFastjet = cms.bool(True))

from RecoJets.JetProducers.ak5PFJetsFiltered_cfi import ak5PFJetsMassDropFiltered
ak5MassDropFilteredPFlow = ak5PFJetsMassDropFiltered.clone(doAreaFastjet = cms.bool(True))

#process.ca12GenJetsNoNu = ca4GenJets.clone( rParam = cms.double(1.2),src = cms.InputTag("genParticlesForJetsNoNu"))
process.ca12GenJets = ca4GenJets.clone( rParam = cms.double(1.2),src = cms.InputTag("genParticlesForJets"))
process.ca12PFJetsPFlow = ca4PFJets.clone(
    rParam = cms.double(1.2),
    src = cms.InputTag('pfNoElectron'+postfix),
    doAreaFastjet = cms.bool(True),
    doRhoFastjet = cms.bool(True),
    Rho_EtaMax = cms.double(6.0),
    Ghost_EtaMax = cms.double(7.0)
    )
## this thing produces subjets by default
process.ca12PFJetsPrunedPFlow = ak5PrunedPFlow.clone(
    src = cms.InputTag('pfNoElectron'+postfix),
    doAreaFastjet = cms.bool(True),
    rParam = cms.double(1.2),
    jetAlgorithm = cms.string("CambridgeAachen"),
    #writeCompound = cms.bool(True), # this is used by default
    #jetCollInstanceName = cms.string("SubJets"), # this is used by default
    )
## this thing produces subjets by default
process.ca12PFJetsFilteredPFlow = ak5FilteredPFlow.clone(
    src = cms.InputTag('pfNoElectron'+postfix),
    doAreaFastjet = cms.bool(True),
    rParam = cms.double(1.2),
    jetAlgorithm = cms.string("CambridgeAachen"),
    )
## this thing produces subjets by default
process.ca12PFJetsMassDropFilteredPFlow = ak5MassDropFilteredPFlow.clone(
    src = cms.InputTag('pfNoElectron'+postfix),
    doAreaFastjet = cms.bool(True),
    rParam = cms.double(1.2),
    jetAlgorithm = cms.string("CambridgeAachen"),
    )

addJetCollection(process,
    cms.InputTag('ca12PFJetsPFlow'), # Jet collection; must be already in the event when patLayer0 sequence is executed
    'CA12', 'PF',
    doJTA=True, # Run Jet-Track association & JetCharge
    doBTagging=True, # Run b-tagging
    jetCorrLabel=None,
    doType1MET=True,
    doL1Cleaning=False,
    doL1Counters=False,
    genJetCollection = (cms.InputTag("ca12GenJets") if RUN_ON_MC else None),
    doJetID = False
    )

addJetCollection(process,
    cms.InputTag('ca12PFJetsMassDropFilteredPFlow'), # Jet collection; must be already in the event when patLayer0 sequence is executed
    'CA12MassDropFiltered', 'PF',
    doJTA=True, # Run Jet-Track association & JetCharge
    doBTagging=False, # Run b-tagging
    jetCorrLabel=None,
    doType1MET=True,
    doL1Cleaning=False,
    doL1Counters=False,
    #genJetCollection = cms.InputTag("ak5GenJetsNoNu"),
    doJetID = False
    )

## adding the subjet collections which are b-tagged...
addJetCollection(process,
    cms.InputTag('ca12PFJetsMassDropFilteredPFlow', 'SubJets'), # Jet collection; must be already in the event when patLayer0 sequence is executed
    'CA12MassDropFilteredSubjets', 'PF',
    doJTA=True, # Run Jet-Track association & JetCharge
    doBTagging=True, # Run b-tagging
    jetCorrLabel=( 'AK5PF', inputJetCorrLabel ),
    doType1MET=True,
    doL1Cleaning=False,
    doL1Counters=False,
    #genJetCollection = cms.InputTag("ak5GenJetsNoNu"),
    doJetID = False
    )

addJetCollection(process,
    cms.InputTag('ca12PFJetsFilteredPFlow', 'SubJets'), # Jet collection; must be already in the event when patLayer0 sequence is executed
    'CA12FilteredSubjets', 'PF',
    doJTA=True, # Run Jet-Track association & JetCharge
    doBTagging=True, # Run b-tagging
    jetCorrLabel=( 'AK5PF', inputJetCorrLabel ),
    doType1MET=True,
    doL1Cleaning=False,
    doL1Counters=False,
    #genJetCollection = cms.InputTag("ak5GenJetsNoNu"),
    doJetID = False
    )

addJetCollection(process,
    cms.InputTag('ca12PFJetsPrunedPFlow', 'SubJets'), # Jet collection; must be already in the event when patLayer0 sequence is executed
    'CA12PrunedSubjets', 'PF',
    doJTA=True, # Run Jet-Track association & JetCharge
    doBTagging=True, # Run b-tagging
    jetCorrLabel=( 'AK5PF', inputJetCorrLabel ),
    doType1MET=True,
    doL1Cleaning=False,
    doL1Counters=False,
    #genJetCollection = cms.InputTag("ak5GenJetsNoNu"),
    doJetID = False
    )

# ------------------------------------------------------------------------------
# Cleaning
# ------------------------------------------------------------------------------
# NB: A further lepton cleaning is done at Step 2
process.cleanPatJets.checkOverlaps.electrons.requireNoOverlaps = cms.bool(True)
process.cleanPatJets.checkOverlaps.electrons.preselection = (
    "pt > 15.0 && abs(eta) < 2.5 &&"
    "(isEE || isEB) && !isEBEEGap &&"
    " (chargedHadronIso + neutralHadronIso + photonIso)/pt <0.10 &&"
    "dB < 0.02 && "  #dB is computed wrt PV but is transverse only, no info about dZ(vertex)
    "( "
    "(isEE && ("
    "abs(deltaEtaSuperClusterTrackAtVtx) < 0.005 &&  abs(deltaPhiSuperClusterTrackAtVtx) < 0.02 && sigmaIetaIeta < 0.03 && hadronicOverEm < 0.10 &&  abs(1./ecalEnergy*(1.-eSuperClusterOverP)) < 0.05 "
    ")) || "
    "(isEB && (  "
    "abs(deltaEtaSuperClusterTrackAtVtx) < 0.004 &&  abs(deltaPhiSuperClusterTrackAtVtx) < 0.03 && sigmaIetaIeta < 0.01 && hadronicOverEm < 0.12 && abs(1./ecalEnergy*(1.-eSuperClusterOverP)) < 0.05"
    "))"
    #or use mvaNonTrigV0 and mvaTrigV0
    ")"
    )

#   "ecalDrivenSeed==1 && (abs(superCluster.eta)<2.5)"
#   " && !(1.4442<abs(superCluster.eta)<1.566)"
#   " && (ecalEnergy*sin(superClusterPosition.theta)>20.0)"
#   " && (gsfTrack.trackerExpectedHitsInner.numberOfHits == 0)"
#   " && ((chargedHadronIso + neutralHadronIso + photonIso < 0.15 * pt)) "
#   " && ((isEB"
#   " && (sigmaIetaIeta<0.01)"
#   " && ( -0.8<deltaPhiSuperClusterTrackAtVtx<0.8 )"
#   " && ( -0.007<deltaEtaSuperClusterTrackAtVtx<0.007 )"
#   ")"
#   " || (isEE"
#   " && (sigmaIetaIeta<0.03)"
#   " && ( -0.7<deltaPhiSuperClusterTrackAtVtx<0.7 )"
#   " && ( -0.01<deltaEtaSuperClusterTrackAtVtx<0.01 )"
#   "))")

process.cleanPatJets.checkOverlaps.muons.requireNoOverlaps = cms.bool(True)
process.cleanPatJets.checkOverlaps.muons.preselection = (
    "pt > 20 && isGlobalMuon && globalTrack().normalizedChi2 < 10 && isPFMuon && "
    "innerTrack().hitPattern().trackerLayersWithMeasurement > 5 && "
    "innerTrack().hitPattern().numberOfValidPixelHits > 0 && "
    "globalTrack().hitPattern().numberOfValidMuonHits > 0 && "
    "numberOfMatchedStations > 1 && "
    "dB < 0.2 && abs(eta) < 2.4 "
    "&& ( chargedHadronIso + neutralHadronIso + photonIso ) < 0.10 * pt  "
    )

# ------------------------------------------------------------------------------
# Pileup Jet ID
# ------------------------------------------------------------------------------
process.load("RecoJets.JetProducers.PileupJetID_cfi")
process.pileupJetIdProducerChs.jets = cms.InputTag("cleanPatJets")
#process.pileupJetIdProducerChs.jets = cms.InputTag("pfNoTauPFlow")
#process.pileupJetIdProducerChs.inputIsCorrected = False
#process.pileupJetIdProducerChs.applyJec = True

# ------------------------------------------------------------------------------
# Cut
# ------------------------------------------------------------------------------
# NB: no cut on number of constituents
defaultJetCut = cms.string('pt > 15. & abs(eta) < 5.0')
defaultFatJetCut = cms.string('pt > 100. & abs(eta) < 5.0')
process.selectedPatJets.cut = defaultJetCut
process.selectedPatJetsAK7PF.cut = defaultJetCut
process.selectedPatJetsCAVHFatPF.cut = defaultFatJetCut
process.selectedPatJetsCAVHSubPF.cut = cms.string('pt > 15. & abs(eta) < 5.0')
process.selectedPatJetsCAVHFilterPF.cut = cms.string('pt > 5. & abs(eta) < 5.0')

jetCutCA12 = cms.string('pt > 100.')
subjetCutCA12 = cms.string('pt > 5.')
process.selectedPatJetsCA12PF.cut = jetCutCA12
process.selectedPatJetsCA12MassDropFilteredPF.cut = jetCutCA12
process.selectedPatJetsCA12MassDropFilteredSubjetsPF.cut = subjetCutCA12
process.selectedPatJetsCA12FilteredSubjetsPF.cut = subjetCutCA12
process.selectedPatJetsCA12PrunedSubjetsPF.cut = subjetCutCA12


################################################################################
# b-Jet                                                                        #
################################################################################
# TODO: Update to PoolBTagPerformanceDB062012, BTagPerformanceDB062012?
#         https://hypernews.cern.ch/HyperNews/CMS/get/btag/879.html
# TODO: Update to 2012 b-tag and mistag SFs

# ------------------------------------------------------------------------------
# Info
#   https://twiki.cern.ch/twiki/bin/view/CMS/BtagPOG
# ------------------------------------------------------------------------------
# Load b-tag payloads
process.load ("RecoBTag.PerformanceDB.PoolBTagPerformanceDB1107")
process.load ("RecoBTag.PerformanceDB.BTagPerformanceDB1107")

# IVF and BCandidate producer for Vbb cross check analysis
process.load("RecoVertex.AdaptiveVertexFinder.inclusiveVertexing_cff")
process.load("RecoBTag.SecondaryVertex.bVertexFilter_cfi")
process.selectedVertices = process.bVertexFilter.clone()
process.selectedVertices.secondaryVertices = cms.InputTag("inclusiveMergedVertices")
process.selectedVertices.minVertices = 0
process.selectedVertices.vertexFilter.multiplicityMin = 3

#process.bcandidates = cms.EDProducer('BCandidateProducer',
#    src = cms.InputTag('selectedVertices','',''),
#    primaryVertices = cms.InputTag('offlinePrimaryVerticesWithBS','',''),
#    minDRUnique = cms.untracked.double(0.4),
#    minvecSumIMifsmallDRUnique = cms.untracked.double(5.5),
#    minCosPAtomerge = cms.untracked.double(0.99),
#    maxPtreltomerge = cms.untracked.double(7777.0)
#    )
#
#process.bhadrons = cms.EDProducer('MCBHadronProducer',
#    quarkId = cms.uint32(5)
#    )

################################################################################
# MET                                                                          #
################################################################################
# TODO: MET systematic x/y shift correction is currently not applied, but is recommended
# TODO: MET smearing should be applied to MC only. If it's dropped for data, then
#       process.producePatPFMETCorrections has to be added into process.common sequence.
# TODO: Update MET filters to HCP recommendation, in particularly:
#       to include anomalous ECAL laser corrections in 2012A+B rereco datasets
#         https://hypernews.cern.ch/HyperNews/CMS/get/physics-validation/1892.html
#       to include HCAL laser event filter in 2012 Data
#         https://hypernews.cern.ch/HyperNews/CMS/get/physics-validation/1905.html
# TODO: No pileup MET https://twiki.cern.ch/twiki/bin/view/CMS/NoPileUpMet
# TODO: MVA MET https://twiki.cern.ch/twiki/bin/view/CMS/MVAMet

# ------------------------------------------------------------------------------
# Info
#   https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMetAnalysis
#   https://twiki.cern.ch/twiki/bin/view/CMS/MissingET
# ------------------------------------------------------------------------------

from PhysicsTools.PatAlgos.tools.metTools import *
#addTcMET(process, 'TC')
#addPfMET(process, 'PF')

# load modules for producing Type 1 / Type 1 + 2 corrections for pat::PFMET objects
process.load("PhysicsTools.PatUtils.patPFMETCorrections_cff")

# Also do Type-0 correction
process.producePatPFMETCorrections.replace(
    process.pfCandMETcorr,
    process.type0PFMEtCorrection *
    process.patPFMETtype0Corr *
    process.pfCandMETcorr
    )

if not RUN_ON_MC:
    # NOTE: use "ak5PFL1FastL2L3" for MC / "ak5PFL1FastL2L3Residual" for Data
    process.pfJetMETcorr.jetCorrLabel = cms.string("ak5PFL1FastL2L3Residual")  # not used
    process.pfJetMETcorrPFlow.jetCorrLabel = cms.string("ak5PFL1FastL2L3Residual")  # not used
    # NOTE: use "L3Absolute" for MC / "L2L3Residual" for Data
    process.patPFJetMETtype1p2Corr.jetCorrLabel = cms.string("L2L3Residual")

# Track-MET with charged hadron subtraction
process.pfMETNoPU = process.pfMETPFlow.clone(
    src = cms.InputTag("pfNoPileUp"+postfix),
    #jets = cms.InputTag("pfJets"+postfix),
    )

process.pfNoPileUpCharge = cms.EDFilter("GenericPFCandidateSelector",
    src = cms.InputTag("pfNoPileUp"+postfix),
    cut = cms.string("charge!=0")
    )

process.pfMETNoPUCharge = process.pfMETPFlow.clone(
    src = cms.InputTag("pfNoPileUpCharge"),
    calculateSignificance = cms.bool(False)
    )

## MHT
#process.patMETsHT = cms.EDProducer("MHTProducer",
#    JetCollection = cms.InputTag("patJets"),  # use selectedPatJets instead?
#    MinJetPt      = cms.double(30),
#    MaxJetEta     = cms.double(5)
#    )

# ------------------------------------------------------------------------------
# MET Smearing Correction
#   https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePATTools#MET_Systematics_Tools
# ------------------------------------------------------------------------------
# NB: The photonCollection parameter is set to None per default, in order to
#     avoid overlap with the electron collection.
# NB: The energies of pat::Jets are smeared by the Data/MC difference in PFJet
#     resolution per default. The smearing factors are taken from JME-10-014.
# NB: Type-0 MET correction are applied (default).
# NB: MET systematic x/y shift correction is not applied (default).
from PhysicsTools.PatUtils.tools.metUncertaintyTools import runMEtUncertainties
runMEtUncertainties(process, 'selectedPatElectrons', None, 'selectedPatMuons', 'selectedPatTaus', 'selectedPatJets', jetCorrLabel=("L3Absolute" if RUN_ON_MC else "L2L3Residual"), doSmearJets=True, doApplyType0corr=True, doApplySysShiftCorr=False)  # change doSmearJets=True to doSmearJets=RUN_ON_MC ?


################################################################################
# Trigger                                                                      #
################################################################################
from PhysicsTools.PatAlgos.tools.trigTools import *

# In general, it should always be used after any modification done to paths
# (including the out-path), sequences and the modules they contain, since the
# current configuration is used in these tools to determine e.g. the correct
# order of module executions or the output to be saved in the event.
switchOnTrigger( process )


################################################################################
# Event Filters                                                                #
################################################################################

# ------------------------------------------------------------------------------
# MET Filters
#   https://twiki.cern.ch/twiki/bin/view/CMS/MissingETOptionalFilters
#   TWiki revision: ?
# The following is taken from RecoMET/METFilters/test/exampleICHEPrecommendation_cfg.py V00-00-08
# ------------------------------------------------------------------------------

## The beam scraping filter __________________________________________________||
process.noscraping = cms.EDFilter(
    "FilterOutScraping",
    applyfilter = cms.untracked.bool(True),
    debugOn = cms.untracked.bool(False),
    numtrack = cms.untracked.uint32(10),
    thresh = cms.untracked.double(0.25)
    )

## The iso-based HBHE noise filter ___________________________________________||
process.load('CommonTools.RecoAlgos.HBHENoiseFilter_cfi')

## The CSC beam halo tight filter ____________________________________________||
process.load('RecoMET.METAnalyzers.CSCHaloFilter_cfi')

## The HCAL laser filter _____________________________________________________||
process.load("RecoMET.METFilters.hcalLaserEventFilter_cfi")
process.hcalLaserEventFilter.vetoByRunEventNumber=cms.untracked.bool(False)
process.hcalLaserEventFilter.vetoByHBHEOccupancy=cms.untracked.bool(True)

## The ECAL dead cell trigger primitive filter _______________________________||
process.load('RecoMET.METFilters.EcalDeadCellTriggerPrimitiveFilter_cfi')
## For AOD and RECO recommendation to use recovered rechits
process.EcalDeadCellTriggerPrimitiveFilter.tpDigiCollection = cms.InputTag("ecalTPSkimNA")

## The EE bad SuperCrystal filter ____________________________________________||
process.load('RecoMET.METFilters.eeBadScFilter_cfi')

## The Good vertices collection needed by the tracking failure filter ________||
process.goodVertices = cms.EDFilter(
    "VertexSelector",
    filter = cms.bool(False),
    src = cms.InputTag("offlinePrimaryVertices"),
    cut = cms.string("!isFake && ndof > 4 && abs(z) <= 24 && position.rho < 2")
    )

## The tracking failure filter _______________________________________________||
process.load('RecoMET.METFilters.trackingFailureFilter_cfi')


################################################################################
# Hbb Specific                                                                 #
################################################################################

# Save gen leptons with pt > 5 GeV, b and c quarks, B and D hadrons (and Lambda b,c), Z, W, H, all status 3 particles. Daugthers of Z, W, H.
process.load("SimGeneral.HepPDTESSource.pythiapdt_cfi")

process.savedGenParticles = cms.EDProducer(
    "GenParticlePruner",
    src = cms.InputTag("genParticles"),
    select = cms.vstring(
        "drop  *",  # this is the default
        "keep++ pdgId >= 23 && pdgId <= 25",  # keep W,Z,H and their decay products
        "keep++ pdgId == 22 && pt > 15",  # keep gamma above 15 GeV
        "drop++ status == 2",  # drop all non stable decay products (and daughters) [quarks are going to be added below]
        "keep++ abs(pdgId) == 15",  # keep tau and its decay products
        "keep  numberOfMothers() > 0 && abs(mother(0).pdgId) == 15",  # keep first generation of tau daugthers (this is redundant I think)
        "drop  numberOfMothers() > 0 && abs(mother(0).pdgId) == {pi0}",  # drop pi0 daugthers photons
        "keep  (abs(pdgId) == 11 || abs(pdgId) == 13 || abs(pdgId) == 15) && pt > 5.0",  # keep leptons of decent pT
        "keep  (abs(pdgId) > 400 && abs(pdgId) < 600) || (abs(pdgId) > 4000 &&  abs(pdgId) < 6000)",  # track-back the origin of B/D
        "keep  (abs(pdgId) >= 4 && abs(pdgId) <= 6)",  # keep heavy quarks
        "keep  (status == 3)"  # keep event summary status3 (for pythia)
        )
    )

process.gen = cms.Sequence(
    process.genParticlesForJets *
    process.caVHGenJets *
    process.ca12GenJets *
    #process.bhadrons *
    process.savedGenParticles
    )

process.HbbAnalyzerNew = cms.EDProducer("HbbAnalyzerNew",
    runOnMC             = cms.bool(RUN_ON_MC),
    #electronTag         = cms.InputTag("selectedElectronsMatched"),
    electronTag         = cms.InputTag("selectedPatElectrons"),
    #muonTag             = cms.InputTag("selectedMuonsMatched"),
    muonTag             = cms.InputTag("selectedPatMuons"),
    tauTag              = cms.InputTag("patTaus"),
    simplejet2Tag       = cms.InputTag("cleanPatJets"),
    simplejet3Tag       = cms.InputTag("selectedPatJetsAK7PF"),
    fatjetTag           = cms.InputTag("selectedPatJetsCAVHFatPF"),
    subjetTag           = cms.InputTag("selectedPatJetsCAVHSubPF"),
    filterjetTag        = cms.InputTag("selectedPatJetsCAVHFilterPF"),
    photonTag           = cms.InputTag("selectedPatPhotons"),
    metTag              = cms.InputTag("patPFMet"),
    metType1Tag         = cms.InputTag("patType1CorrectedPFMet"),
    metType1p2Tag       = cms.InputTag("patType1p2CorrectedPFMet"),
    electronNoCutsTag   = cms.InputTag("gsfElectrons"),
    muonNoCutsTag       = cms.InputTag("muons"),
    hltResultsTag       = cms.InputTag("TriggerResults::HLT"),
    lep_ptCutForBjets   = cms.double(5),
    verbose             = cms.untracked.bool(False),
    simplejet1Tag       = cms.InputTag("UNUSED_WAS_selectedPatJets"),
    )


################################################################################
# Path, Sequence                                                               #
################################################################################

process.common = cms.Sequence(
    process.goodOfflinePrimaryVertices *
    process.inclusiveVertexing *
    process.eidSequence *
    process.patPF2PATSequencePFlow *
    process.pfMETNoPU *
    process.pfNoPileUpCharge *
    process.pfMETNoPUCharge *
    process.kt6PFJetsCentralNeutral *
    process.kt6PFJetsForIsolation *
    process.kt6PFJets25 *
    process.ak7PFJets *
    #process.caVHCaloJets *
    process.caVHPFJets *
    process.ca12PFJetsPFlow *
    process.ca12PFJetsPrunedPFlow *
    process.ca12PFJetsFilteredPFlow *
    process.ca12PFJetsMassDropFilteredPFlow *
    process.mvaID *
    process.patDefaultSequence *
    #process.producePatPFMETCorrections *
    #process.patMETsHT *
    process.selectedVertices *
    #process.bcandidates *
    process.pileupJetIdProducerChs *
    process.HbbAnalyzerNew
    )

if RUN_ON_MC:
   process.p = cms.Path(process.gen * process.common)
else:
   process.p = cms.Path(process.common)

#process.noscrapingFilter = cms.Path(process.noscraping)
process.hbhepath = cms.Path(process.HBHENoiseFilter)
process.ecalFilter = cms.Path(process.EcalDeadCellTriggerPrimitiveFilter)
process.cschaloFilter = cms.Path(process.CSCTightHaloFilter)
process.hcallaserFilter = cms.Path(process.hcalLaserEventFilter)
process.trackingfailureFilter = cms.Path(process.goodVertices * process.trackingFailureFilter)
process.eebadscFilter = cms.Path(process.eeBadScFilter)

process.out.fileName = 'PAT.edm.root'
process.out.dropMetaData = cms.untracked.string('ALL')
process.out.outputCommands = cms.untracked.vstring(
    'drop *',
    'keep double_kt6PFJets*_rho_*',
    'keep *_pileupJetIdProducerChs_*_*',
    'keep *_savedGenParticles_*_*',
    'keep *_HbbAnalyzerNew_*_*',
    #'keep VHbbCandidates_*_*_*',
    #'keep PileupSummaryInfos_*_*_*',
    'keep edmTriggerResults_TriggerResults_*_*',
    #'keep *_hltTriggerSummaryAOD_*_*',
    #'keep *_selectedVertices_*_*',
    #'keep *_TriggerResults_*_*',
    #'keep *_bcandidates_*_*',
    #'keep *_bhadrons_*_*',
    #"keep *_HLTDiCentralJet20MET80_*_*",
    #"keep *_HLTDiCentralJet20MET100HBHENoiseFiltered_*_*",
    #"keep *_HLTPFMHT150_*_*",
    #"keep *_HLTQuadJet40_*_*",
    #"keep *_HLTDoubleMu7_*_*",
    #"keep *_EcalDeadCellEventFilter_*_*",
    #"keep *_patType1CorrectedPFMet*_*_*",
    #"keep *_patType1p2CorrectedPFMet*_*_*",
    #"keep *_patMETsHT*_*_*",
    "keep *_patType1CorrectedPFMet_*_*",
    "keep LHEEventProduct_source_*_LHE",
    'keep patTriggerAlgorithms_patTrigger_*_*',
    'keep patTriggerConditions_patTrigger_*_*',
    'keep patTriggerObjects_patTrigger_*_*',
    'keep patTriggerFilters_patTrigger_*_*',
    'keep patTriggerPaths_patTrigger_*_*',
    'keep *_patTriggerEvent_*_*',
    )

# ------------------------------------------------------------------------------
# Dump flat python cfg
# ------------------------------------------------------------------------------
temp = process.dumpPython()
with open("patexpstdmc.py",'w') as f:
    f.write(temp)

