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
        # RelVal
        #"/store/relval/CMSSW_7_0_0/RelValZMM_13/GEN-SIM-RECO/POSTLS170_V3-v2/00000/0486BA73-C998-E311-B68E-02163E00EB06.root",
        #"/store/relval/CMSSW_7_0_0/RelValZMM_13/GEN-SIM-RECO/POSTLS170_V3-v2/00000/A67E73F6-CF98-E311-B597-02163E008D87.root",
        ]
else:
    fileNames = [
        "root://cmseos:1094//eos/uscms/store/user/jiafu/ZnunuHbb_CSA14/MET_AOD_207454_143F6068-BB30-E211-B02B-003048D2C108.root",
        # RelVal
        #"/store/relval/CMSSW_7_0_0/MET/RECO/GR_R_70_V1_RelVal_met2012A-v2/00000/0077B633-2A99-E311-B2AA-0026189438D7.root",
        #"/store/relval/CMSSW_7_0_0/MET/RECO/GR_R_70_V1_RelVal_met2012A-v2/00000/08BA3447-2A99-E311-99A7-0025905A60D0.root",
        #"/store/relval/CMSSW_7_0_0/MET/RECO/GR_R_70_V1_RelVal_met2012A-v2/00000/0AC40533-2A99-E311-B8F0-00304867920C.root",

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
process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True),
    ## switch to uncheduled mode
    allowUnscheduled = cms.untracked.bool(True)
    )
#process.Tracer = cms.Service("Tracer")

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
#process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:com10_8E33v2')
process.GlobalTag = GlobalTag(process.GlobalTag, globaltag)
if runOnMC:
    process.load("Configuration.StandardSequences.MagneticField_cff")  # TODO: check this
else:
    process.load("Configuration.StandardSequences.MagneticField_AutoFromDBCurrent_cff")  # TODO: check this

## Output Module Configuration
from PhysicsTools.PatAlgos.patEventContent_cff import patEventContentNoCleaning
process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('patTuple.root'),
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
# PAT                                                                          #
# taken from PhysicsTools/PatAlgos/test/patTuple_standard_cfg.py               #
# Commit SHA        : b81879adf7f42935c3c408692e2d22640b0d03ee                 #
#                                                                              #
# PF2PAT (a.k.a. PFBRECO)                                                      #
# taken from PhysicsTools/PatAlgos/test/patTuple_PATandPF2PAT_cfg.py           #
# Commit SHA        : fe96f6e1e3d1a1028f0b13e4b3acea36b2bd9a5d                 #
################################################################################
ignorePF2PAT = True
if ignorePF2PAT:
    postfix = ""
    jetAlgo = "AK5"
    # bypass PhysicsTools/PatAlgos/python/patSequences_cff.py
    process.load("PhysicsTools.PatAlgos.producersLayer1.patCandidates_cff")
    process.load("PhysicsTools.PatAlgos.selectionLayer1.selectedPatCandidates_cff")
    if not runOnMC:
        process.out.outputCommands += [
            'drop recoGenJets_*_*_*'
            ]
        from PhysicsTools.PatAlgos.tools.coreTools import runOnData
        runOnData(process)

else:
    ## import skeleton process
    #from PhysicsTools.PatAlgos.patTemplate_cfg import *

    ## switch to uncheduled mode
    #process.options.allowUnscheduled = cms.untracked.bool(True)
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
    process.out.fileName = cms.untracked.string('patTuple_PF2PAT.root')
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
    if not runOnMC:
        process.out.outputCommands += [
            'drop recoGenJets_*_*_*'
            ]


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
# END PAT/PF2PAT                                                               #
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
getattr(process,"selectedPatElectrons"+postfix).cut = cms.string("pt > 3")
getattr(process,"selectedPatMuons"+postfix).cut = cms.string("pt > 3")
getattr(process,"selectedPatTaus"+postfix).cut = cms.string("pt > 10")


################################################################################
# Jet                                                                          #
################################################################################

# ------------------------------------------------------------------------------
# AK5, AK7
#   https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections
#   https://twiki.cern.ch/twiki/bin/view/CMS/JetMETAlgorithmsReconstruction
#   https://twiki.cern.ch/twiki/bin/view/CMS/JetResolution
#   https://twiki.cern.ch/twiki/bin/view/CMS/JECDataMC
#   https://twiki.cern.ch/twiki/bin/view/CMS/JECUncertaintySources
#   https://twiki.cern.ch/twiki/bin/view/CMS/JetID
#   https://twiki.cern.ch/twiki/bin/view/CMS/PileupJetID
#   https://twiki.cern.ch/twiki/bin/view/CMS/GluonTag
# ------------------------------------------------------------------------------

from RecoJets.Configuration.RecoPFJets_cff import \
    ak4PFJets, ak5PFJets, ak7PFJets, ak8PFJets, \
    ak4PFJetsCHS, ak5PFJetsCHS, ak8PFJetsCHS, \
    ak8PFJetsCHSPruned, ak8PFJetsCHSFiltered, ak8PFJetsCHSTrimmed, \
    ca8PFJets, ca8PFJetsCHS, ca8PFJetsCHSPruned, ca8PFJetsCHSFiltered, ca8PFJetsCHSTrimmed, \
    ca15PFJetsCHSMassDropFiltered, ca15PFJetsCHSFiltered


#setattr(process, "ak7PFJets"+postfix, ak7PFJets)
#setattr(process, "ak8PFJets"+postfix, ak8PFJets)

# Add b-tagging
#getattr(process, "patJets"+postfix).addBTagInfo = True

# FIXME: addJetCollection assumes 'patJetPartons' exists, but it doesn't
#from PhysicsTools.PatAlgos.mcMatchLayer0.jetFlavourId_cff import patJetPartons
#process.patJetPartons = patJetPartons

#addJetCollection(process, labelName="AK7PF", postfix=postfix,
#    jetSource           = cms.InputTag("ak7PFJets"+postfix),
#    jetCorrections      = ("AK7PFchs", inputJetCorrLabel, ''),
#    btagDiscriminators  = btagDiscr.keys(),
#    btagInfos           = btagInfos,
#    jetTrackAssociation = False,
#    )

from PhysicsTools.PatAlgos.tools.jetTools import switchJetCollection
from PhysicsTools.PatAlgos.tools.jetTools import addJetCollection

# addJetCollection(...) has to happen before switchJetCollection(...),
# otherwise it gives errors when trying to add btag discriminators
outputModules = ['out']
btagInfos = ['secondaryVertexTagInfos', 'inclusiveSecondaryVertexFinderTagInfos', 'inclusiveSecondaryVertexFinderFilteredTagInfos']
btagDiscriminators = ['combinedSecondaryVertexBJetTags', 'combinedSecondaryVertexPositiveBJetTags', 'combinedSecondaryVertexMVABJetTags', 'combinedInclusiveSecondaryVertexBJetTags', 'combinedInclusiveSecondaryVertexPositiveBJetTags']


addJetCollection(
    process,
    labelName = 'AK5PF',
    postfix = postfix,
    jetSource = cms.InputTag('ak5PFJets'),
    algo = 'AK5',
    jetCorrections = ('AK5PF', inputJetCorrLabel, ''),
    btagDiscriminators = btagDiscriminators,
    btagInfos = btagInfos,
    jetTrackAssociation = True,
    outputModules = outputModules,
    )

addJetCollection(
    process,
    labelName = 'AK8PFCHS',
    postfix = postfix,
    jetSource = cms.InputTag('ak8PFJetsCHS'),
    algo = 'AK8',
    jetCorrections = ('AK7PFchs', inputJetCorrLabel, ''),
    btagDiscriminators = btagDiscriminators,
    btagInfos = btagInfos,
    jetTrackAssociation = True,
    outputModules = outputModules,
    )

addJetCollection(
    process,
    labelName = 'CA8PFCHS',
    postfix = postfix,
    jetSource = cms.InputTag('ak8PFJetsCHS'),
    algo = 'CA8',
    jetCorrections = ('AK7PFchs', inputJetCorrLabel, ''),
    btagDiscriminators = btagDiscriminators,
    btagInfos = btagInfos,
    jetTrackAssociation = True,
    outputModules = outputModules,
    )

switchJetCollection(
    process,
    jetSource = cms.InputTag('ak5PFJetsCHS'),
    algo = jetAlgo,
    postfix = postfix,
    jetCorrections = ('AK5PFchs', inputJetCorrLabel, ''),
    btagDiscriminators = btagDiscriminators,
    btagInfos = btagInfos,
    jetTrackAssociation = True,
    outputModules = outputModules,
    )



# ------------------------------------------------------------------------------
# Cut
# ------------------------------------------------------------------------------
getattr(process,"selectedPatJets"+postfix).cut = cms.string("pt > 10")



################################################################################
# MET                                                                          #
################################################################################

# ------------------------------------------------------------------------------
#   https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMetAnalysis
# ------------------------------------------------------------------------------

# apply type I/type I + II PFMEt corrections to pat::MET object
# and estimate systematic uncertainties on MET
#from PhysicsTools.PatUtils.tools.metUncertaintyTools import runMEtUncertainties
#runMEtUncertainties(process)


process.out.outputCommands += [
    'keep *_goodOfflinePrimaryVertices_*_*',
    #'keep double_*_rho_*',
    ]

################################################################################
# HLT                                                                          #
################################################################################

#from PhysicsTools.PatAlgos.tools.trigTools import switchOnTriggerStandAlone
#switchOnTriggerStandAlone( process )
#process.patTrigger.packTriggerPathNames = cms.bool(True)


# ------------------------------------------------------------------------------
# Dump flat python cfg
# ------------------------------------------------------------------------------
temp = process.dumpPython()
with open("dump.py",'w') as f:
    f.write(temp)

