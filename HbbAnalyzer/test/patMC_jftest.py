# ==============================================================================
# VHbbAnalysis
#   https://twiki.cern.ch/twiki/bin/view/CMS/VHbbAnalysisNewCode
#
# CMS Official Recommendations (2012)
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
runOnMC = True
if runOnMC:
    fileNames = [
        "root://cmseos:1094//eos/uscms/store/user/jiafu/ZnunuHbb_CSA14/ZH_HToBB_ZToLL_PU20bx25_AODSIM_0E0D6C44-956E-E311-8ADF-848F69FD297F.root",
        ]
else:
    fileNames = [
        "root://cmseos:1094//eos/uscms/store/user/jiafu/ZnunuHbb_CSA14/MET_AOD_207454_143F6068-BB30-E211-B02B-003048D2C108.root",
        ]

# ------------------------------------------------------------------------------
# Global Tag
#   https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideFrontierConditions
# TWiki revision    : r463 - 05 Mar 2014
# ------------------------------------------------------------------------------
# NOTE: Please double check if they are the latest and greatest
if runOnMC:
    # Fall13_70X
    globaltag = "POSTLS170_V3::All"
else:
    # Run2012_70X
    globaltag = "GR_R_70_V1::All"


################################################################################
# PAT Skeleton                                                                 #
# taken from PhysicsTools/PatAlgos/python/patTemplate_cfg.py                   #
# Commit SHA        : 2d6c80515e5b4cc9e886c6840728eef0428fdec1                 #
################################################################################

## MessageLogger
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100

## Options and Output Report
process.options = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )

## Source
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(fileNames)
    )
## Maximal Number of Events
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(100) )

## Geometry and Detector Conditions (needed for a few patTuple production steps)
process.load("Configuration.Geometry.GeometryIdeal_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
from Configuration.AlCa.GlobalTag import GlobalTag
#process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:startup')
process.GlobalTag = GlobalTag(process.GlobalTag, globaltag)
process.load("Configuration.StandardSequences.MagneticField_cff")  # TODO: change this

## Output Module Configuration (expects a path 'p')
from PhysicsTools.PatAlgos.patEventContent_cff import patEventContentNoCleaning
process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('patTuple.root'),
    ## save only events passing the full path
    #SelectEvents = cms.untracked.PSet( SelectEvents = cms.vstring('p') ),
    ## save PAT output; you need a '*' to unpack the list of commands
    ## 'patEventContentNoCleaning'
    outputCommands = cms.untracked.vstring('drop *', *patEventContentNoCleaning )
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
if runOnMC:
    inputJetCorrLabel = ['L1FastJet', 'L2Relative', 'L3Absolute']
else:
    inputJetCorrLabel = ['L1FastJet', 'L2Relative', 'L3Absolute', 'L2L3Residual']


################################################################################
# PF2PAT (a.k.a. PFBRECO)                                                      #
# taken from PhysicsTools/PatAlgos/test/patTuple_PATandPF2PAT_cfg.py           #
# Commit SHA        : fe96f6e1e3d1a1028f0b13e4b3acea36b2bd9a5d                 #
################################################################################

## import skeleton process
#from PhysicsTools.PatAlgos.patTemplate_cfg import *

## switch to uncheduled mode
process.options.allowUnscheduled = cms.untracked.bool(True)
#process.Tracer = cms.Service("Tracer")

# Configure PAT to use PF2PAT instead of AOD sources
# this function will modify the PAT sequences.
from PhysicsTools.PatAlgos.tools.pfTools import *
postfix = "PFlow"
jetAlgo = "AK5"
usePF2PAT(process,runPF2PAT=True,
    jetAlgo=jetAlgo, runOnMC=runOnMC, postfix=postfix,
    jetCorrections=('AK5PFchs', inputJetCorrLabel),
    pvCollection=cms.InputTag('goodOfflinePrimaryVertices'),
    typeIMetCorrections=True)
# checkClosestZVertex set to 'False' according to https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections#JetEnCorPFnoPU
getattr(process,"pfPileUp"+postfix).checkClosestZVertex = cms.bool(False)

# to switch default tau (HPS) to old default tau (shrinking cone) uncomment
# the following:
# note: in current default taus are not preselected i.e. you have to apply
# selection yourself at analysis level!
#adaptPFTaus(process,"shrinkingConePFTau",postfix=postfix)

# to use GsfElectrons instead of PF electrons
# this will destory the feature of top projection which solves the ambiguity between leptons and jets because
# there will be overlap between non-PF electrons and jets even though top projection is ON!
#useGsfElectrons(process, postfix,"03") # to change isolation cone size to 0.3 as it is recommended by EGM POG, use "04" for cone size 0.4


# Add PF2PAT output to the created file
process.out.outputCommands = cms.untracked.vstring(
    'drop *',
    'keep recoPFCandidates_particleFlow_*_*',
    'keep *_selectedPatJets*_*_*',
    'drop *_selectedPatJets*_caloTowers_*',
    'keep *_selectedPatElectrons*_*_*',
    'keep *_selectedPatMuons*_*_*',
    'keep *_selectedPatTaus*_*_*',
    'keep *_patMETs*_*_*',
    'keep *_selectedPatPhotons*_*_*',
    'keep *_selectedPatTaus*_*_*',
    )


# top projections in PF2PAT:
getattr(process,"pfNoPileUp"+postfix).enable = True
getattr(process,"pfNoMuon"+postfix).enable = False  # do traditional cleaning
getattr(process,"pfNoElectron"+postfix).enable = False  # do traditional cleaning
getattr(process,"pfNoTau"+postfix).enable = False
getattr(process,"pfNoJet"+postfix).enable = True
# to use tau-cleaned jet collection uncomment the following:
#getattr(process,"pfNoTau"+postfix).enable = True

# verbose flags for the PF2PAT modules
#getattr(process,"pfNoMuon"+postfix).verbose = False

# enable delta beta correction for muon selection in PF2PAT?
getattr(process,"pfIsolatedMuons"+postfix).doDeltaBetaCorrection = cms.bool(False)
################################################################################
# END PF2PAT+PAT                                                               #
################################################################################


################################################################################
# Lepton                                                                       #
################################################################################
# Note: CMS recommendations for isolation are R=0.3 for e, R=0.4 for mu,
#   but VHbb uses R=0.4 for both e and mu. Please double check that the
#   WPxx definitions (for e) are consistent.
#   For PU subtraction, VHbb uses EA correction for e, deltaBeta for mu


# ------------------------------------------------------------------------------
# Electron ID
#   https://twiki.cern.ch/twiki/bin/view/CMS/EgammaIDRecipes
#   https://twiki.cern.ch/twiki/bin/view/CMS/EgammaEARhoCorrection
#   https://twiki.cern.ch/twiki/bin/view/CMS/EgammaPFBasedIsolation
#   https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentification
# ------------------------------------------------------------------------------

# The default PFBased isolation cone size is 0.4.
# This can be switched to 0.3 by the following
#getattr(process,"pfIsolatedElectrons"+postfix).isolationValueMapsCharged = cms.VInputTag(cms.InputTag("elPFIsoValueCharged03PFId"+postfix))
#getattr(process,"pfIsolatedElectrons"+postfix).deltaBetaIsolationValueMap = cms.InputTag("elPFIsoValuePU03PFId"+postfix)
#getattr(process,"pfIsolatedElectrons"+postfix).isolationValueMapsNeutral = cms.VInputTag(cms.InputTag("elPFIsoValueNeutral03PFId"+postfix), cms.InputTag("elPFIsoValueGamma03PFId"+postfix))
#
#getattr(process,"pfElectrons"+postfix).isolationValueMapsCharged = cms.VInputTag(cms.InputTag("elPFIsoValueCharged03PFId"+postfix))
#getattr(process,"pfElectrons"+postfix).deltaBetaIsolationValueMap = cms.InputTag("elPFIsoValuePU03PFId"+postfix)
#getattr(process,"pfElectrons"+postfix).isolationValueMapsNeutral = cms.VInputTag(cms.InputTag("elPFIsoValueNeutral03PFId"+postfix), cms.InputTag("elPFIsoValueGamma03PFId"+postfix))
#
#getattr(process,"patElectrons"+postfix).isolationValues = cms.PSet(
#        pfChargedHadrons = cms.InputTag("elPFIsoValueCharged03PFId"+postfix),
#        pfChargedAll = cms.InputTag("elPFIsoValueChargedAll03PFId"+postfix),
#        pfPUChargedHadrons = cms.InputTag("elPFIsoValuePU03PFId"+postfix),
#        pfNeutralHadrons = cms.InputTag("elPFIsoValueNeutral03PFId"+postfix),
#        pfPhotons = cms.InputTag("elPFIsoValueGamma03PFId"+postfix)
#        )
#
#getattr(process,"patElectrons"+postfix).isolationValuesNoPFId = cms.PSet(
#        pfChargedHadrons = cms.InputTag("elPFIsoValueNeutral03NoPFId"+postfix),
#        pfChargedAll = cms.InputTag("elPFIsoValueChargedAll03NoPFId"+postfix),
#        pfPUChargedHadrons = cms.InputTag("elPFIsoValuePU03NoPFId"+postfix),
#        pfNeutralHadrons = cms.InputTag("elPFIsoValueNeutral03NoPFId"+postfix),
#        pfPhotons = cms.InputTag("elPFIsoValueGamma03NoPFId"+postfix)
#        )

## MVA electron ID
## Won't work now because gsfElectrons are removed
#process.load('EgammaAnalysis.ElectronTools.electronIdMVAProducer_cfi')
#process.mvaID = cms.Sequence( process.mvaTrigV0 + process.mvaTrigNoIPV0 + process.mvaNonTrigV0 )
## append them
#getattr(process,"patElectrons"+postfix).electronIDSources.mvaTrigV0 = cms.InputTag("mvaTrigV0")
#getattr(process,"patElectrons"+postfix).electronIDSources.mvaNonTrigV0 = cms.InputTag("mvaNonTrigV0")
#getattr(process,"patElectrons"+postfix).electronIDSources.mvaTrigNoIPV0 = cms.InputTag("mvaTrigNoIPV0")


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
# Cut
# ------------------------------------------------------------------------------
getattr(process,"selectedPatElectrons"+postfix).cut = cms.string("")
getattr(process,"selectedPatMuons"+postfix).cut = cms.string("")
getattr(process,"selectedPatTaus"+postfix).cut = cms.string("")


################################################################################
# Jet                                                                          #
################################################################################

# ------------------------------------------------------------------------------
# AK5
#   https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections
#   https://twiki.cern.ch/twiki/bin/view/CMS/JetResolution
#   https://twiki.cern.ch/twiki/bin/view/CMS/JECDataMC
#   https://twiki.cern.ch/twiki/bin/view/CMS/JECUncertaintySources
#   https://twiki.cern.ch/twiki/bin/view/CMS/JetID
#   https://twiki.cern.ch/twiki/bin/view/CMS/PileupJetID
#   http://pandolf.web.cern.ch/pandolf/instr/QGLikelihood.txt Quark/Gluon discriminator
# ------------------------------------------------------------------------------


################################################################################
# MET                                                                          #
################################################################################

# apply type I/type I + II PFMEt corrections to pat::MET object
# and estimate systematic uncertainties on MET
#from PhysicsTools.PatUtils.tools.metUncertaintyTools import runMEtUncertainties
#runMEtUncertainties(process)


process.out.outputCommands += [
    'keep *_goodOfflinePrimaryVertices_*_*',
    #'keep double_*_rho_*',
    ]

# ------------------------------------------------------------------------------
# Dump flat python cfg
# ------------------------------------------------------------------------------
temp = process.dumpPython()
with open("dump.py",'w') as f:
    f.write(temp)

