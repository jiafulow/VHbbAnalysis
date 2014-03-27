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
process.out.outputCommands += [
    'drop *_selectedPatJets*_caloTowers_*',
    'keep recoPFCandidates_particleFlow_*_*',
    #'keep double_*_rho_*',
    'keep double_fixedGridRho*_*_*',
    'keep *_offlinePrimaryVertices_*_*'
    ]
if runOnMC:
    process.out.outputCommands += [
        #'keep *recoGenParticles_genParticles__*',
        'keep GenEventInfoProduct_*__*',
        'keep *_addPileupInfo__*',
        ]
else:
    process.out.outputCommands += [
        'keep recoBeamHaloSummary_BeamHaloSummary__*',
        'keep recoGlobalHaloData_GlobalHaloData__*',
        'keep HcalNoiseSummary_hcalnoise__*',
        'keep LumiSummary_lumiProducer__*',
        ]

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
process.out.outputCommands += ['keep *_goodOfflinePrimaryVertices_*_*']

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
    # Note: Without PF2PAT, 'goodOfflinePrimaryVertices' is not used

    # bypass PhysicsTools/PatAlgos/python/patSequences_cff.py
    process.load("PhysicsTools.PatAlgos.producersLayer1.patCandidates_cff")
    process.load("PhysicsTools.PatAlgos.selectionLayer1.selectedPatCandidates_cff")
    if not runOnMC:
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
    # 'checkClosestZVertex' set to 'False' according to
    # https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections#JetEnCorPFnoPU
    getattr(process,"pfPileUp"+postfix).checkClosestZVertex = cms.bool(False)

    process.out.fileName = cms.untracked.string('patTuple_PF2PAT.root')

    # to switch default tau (HPS) to old default tau (shrinking cone) uncomment
    # the following:
    # note: in current default taus are not preselected i.e. you have to apply
    # selection yourself at analysis level!
    #adaptPFTaus(process,"shrinkingConePFTau",postfix=postfix)

    # to use GsfElectrons instead of PF electrons
    # this will destory the feature of top projection which solves the ambiguity between leptons and jets because
    # there will be overlap between non-PF electrons and jets even though top projection is ON!
    #useGsfElectrons(process, postfix,"03") # to change isolation cone size to 0.3 as it is recommended by EGM POG, use "04" for cone size 0.4

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
# Lepton/Photon                                                                #
################################################################################
doEleIso04 = True
doMuIso04 = True
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
## Won't work for CMSSW_7_X_Y
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
# Photon
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# Cut
# ------------------------------------------------------------------------------
getattr(process,"selectedPatElectrons"+postfix).cut = cms.string("pt > 3  & abs(eta) < 2.5")
getattr(process,"selectedPatMuons"+postfix).cut = cms.string("pt > 3 & abs(eta) < 2.5")
getattr(process,"selectedPatTaus"+postfix).cut = cms.string("pt > 10 & abs(eta) < 2.5")
getattr(process,"selectedPatPhotons"+postfix).cut = cms.string("pt > 10 & abs(eta) < 2.5")
getattr(process,"patTaus"+postfix).isoDeposits = cms.PSet()
getattr(process,"patPhotons"+postfix).isoDeposits = cms.PSet()


################################################################################
# Jet                                                                          #
################################################################################
doAK4 = True
doAK5 = True
doCA8 = True
doCA8TopTag = True
doCA15 = True
# ------------------------------------------------------------------------------
#   https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections
#   https://twiki.cern.ch/twiki/bin/view/CMS/JetMETAlgorithmsReconstruction
#   https://twiki.cern.ch/twiki/bin/view/CMS/JetResolution
#   https://twiki.cern.ch/twiki/bin/view/CMS/JECDataMC
#   https://twiki.cern.ch/twiki/bin/view/CMS/JECUncertaintySources
#   https://twiki.cern.ch/twiki/bin/view/CMS/JetID
# ------------------------------------------------------------------------------

from RecoJets.Configuration.RecoPFJets_cff import \
    ak5PFJets, ak5PFJetsCHS, kt6PFJets, ak8PFJets, ak8PFJetsCHS, \
    ak8PFJetsCHSPruned, ak8PFJetsCHSFiltered, ak8PFJetsCHSTrimmed, \
    ca8PFJetsCHS, ca8PFJetsCHSPruned, ca8PFJetsCHSFiltered, ca8PFJetsCHSTrimmed, \
    ca15PFJetsCHSMassDropFiltered, ca15PFJetsCHSFiltered
#setattr(process, "ak4PFJets"+postfix, ak5PFJets.clone( rParam = 0.4 ))
process.ak4PFJets = ak5PFJets.clone( rParam = 0.4 )
#process.kt6PFJets = kt6PFJets
process.ca8PFJetsCHSPruned = ca8PFJetsCHSPruned.clone( doAreaFastjet = cms.bool(True) )
process.ca15PFJetsCHSFiltered = ca15PFJetsCHSFiltered.clone(
    doAreaFastjet = cms.bool(True),
    ## Uncomment the following three lines to turn 'Filtered' into 'MassDropFiltered'
    #useMassDropTagger = cms.bool(True),
    #muCut = cms.double(0.667),
    #yCut = cms.double(0.08),
    )

if runOnMC:
    # Create GenJets
    from RecoJets.Configuration.RecoGenJets_cff import \
        ak4GenJets, ak4GenJetsNoNu, \
        ca4GenJets, ca4GenJetsNoNu
    from RecoJets.Configuration.GenJetParticles_cff import \
        genParticlesForJets, genParticlesForJetsNoNu
    process.genParticlesForJets = genParticlesForJets
    process.genParticlesForJetsNoNu = genParticlesForJetsNoNu
    process.ak4GenJets = ak4GenJets
    process.ca8GenJets = ca4GenJets.clone( rParam = 0.8 )
    process.ca8GenJetsNoNu = ca4GenJetsNoNu.clone( rParam = 0.8 )
    from RecoJets.JetProducers.SubJetParameters_cfi import SubJetParameters
    process.ca8GenJetsNoNuPruned = process.ca8GenJetsNoNu.clone(
        SubJetParameters,
        usePruning = cms.bool(True),
        useExplicitGhosts = cms.bool(True),
        writeCompound = cms.bool(True),
        jetCollInstanceName=cms.string("SubJets")
        )
    from RecoJets.JetProducers.GenJetParameters_cfi import GenJetParameters
    from RecoJets.JetProducers.caTopTaggers_cff import CATopJetParameters, AnomalousCellParameters
    process.ca8GenJetsNoNuTopTag = cms.EDProducer(
        "CATopJetProducer",
        GenJetParameters.clone( src = cms.InputTag("genParticlesForJetsNoNu") ),
        AnomalousCellParameters,
        CATopJetParameters,
        jetAlgorithm = cms.string("CambridgeAachen"),
        rParam = cms.double(0.8),
        writeCompound = cms.bool(True)
    )
    process.ca15GenJetsNoNu = ca4GenJetsNoNu.clone( rParam = 1.5 )
    process.ca15GenJetsNoNuFiltered = process.ca15GenJetsNoNu.clone(
        useFiltering = cms.bool(True),
        nFilt = cms.int32(3),
        rFilt = cms.double(0.3),
        useExplicitGhosts = cms.bool(True),
        writeCompound = cms.bool(True),
        jetCollInstanceName=cms.string("SubJets")
        )

# addJetCollection(...) has to happen before switchJetCollection(...),
# otherwise it gives errors when trying to add btag discriminators
from PhysicsTools.PatAlgos.tools.jetTools import addJetCollection, switchJetCollection
outputModules = ['out']
btagInfos = ['impactParameterTagInfos', 'secondaryVertexTagInfos', 'inclusiveSecondaryVertexFinderTagInfos', 'inclusiveSecondaryVertexFinderFilteredTagInfos']
btagDiscriminators = ['jetBProbabilityBJetTags', 'jetProbabilityBJetTags', 'combinedSecondaryVertexBJetTags', 'combinedSecondaryVertexPositiveBJetTags', 'combinedSecondaryVertexMVABJetTags', 'combinedInclusiveSecondaryVertexBJetTags', 'combinedInclusiveSecondaryVertexPositiveBJetTags']

if doAK5:
    addJetCollection(
        process,
        labelName = 'AK5PF',
        postfix = postfix,
        jetSource = cms.InputTag('ak5PFJets'),
        algo = 'AK5',
        jetCorrections = ('AK5PF', inputJetCorrLabel, ''),
        #btagDiscriminators = btagDiscriminators,
        #btagInfos = btagInfos,
        #jetTrackAssociation = True,
        outputModules = outputModules,
        )

if doAK4:
    #addJetCollection(
    #    process,
    #    labelName = 'AK4PF',
    #    postfix = postfix,
    #    jetSource = cms.InputTag('ak4PFJets'),
    #    algo = 'AK4',
    #    jetCorrections = ('AK5PF', inputJetCorrLabel, ''),
    #    btagDiscriminators = btagDiscriminators,
    #    btagInfos = btagInfos,
    #    jetTrackAssociation = True,
    #    outputModules = outputModules,
    #    )

    addJetCollection(
        process,
        labelName = 'AK4PFCHS',
        postfix = postfix,
        jetSource = cms.InputTag('ak4PFJetsCHS'),
        algo = 'AK4',
        jetCorrections = ('AK5PFchs', inputJetCorrLabel, ''),
        btagDiscriminators = btagDiscriminators,
        btagInfos = btagInfos,
        jetTrackAssociation = True,
        outputModules = outputModules,
        )

if doCA8 or doCA8TopTag:
    #addJetCollection(
    #    process,
    #    labelName = 'AK8PF',
    #    postfix = postfix,
    #    jetSource = cms.InputTag('ak8PFJets'),
    #    algo = 'AK8',
    #    jetCorrections = ('AK7PF', inputJetCorrLabel, ''),
    #    #btagDiscriminators = btagDiscriminators,
    #    #btagInfos = btagInfos,
    #    #jetTrackAssociation = True,
    #    outputModules = outputModules,
    #    )

    addJetCollection(
        process,
        labelName = 'AK8PFCHS',
        postfix = postfix,
        jetSource = cms.InputTag('ak8PFJetsCHS'),
        algo = 'AK8',
        jetCorrections = ('AK7PFchs', inputJetCorrLabel, ''),
        #btagDiscriminators = btagDiscriminators,
        #btagInfos = btagInfos,
        #jetTrackAssociation = True,
        outputModules = outputModules,
        )

    addJetCollection(
        process,
        labelName = 'CA8PFCHS',
        postfix = postfix,
        jetSource = cms.InputTag('ca8PFJetsCHS'),
        algo = 'CA8',
        jetCorrections = ('AK7PFchs', inputJetCorrLabel, ''),
        #btagDiscriminators = btagDiscriminators,
        #btagInfos = btagInfos,
        #jetTrackAssociation = True,
        outputModules = outputModules,
        )
    #if runOnMC:  getattr(process, 'patJetGenJetMatchPatJetsCA8PFCHS'+postfix).matched = cms.InputTag('ca8GenJets')

    addJetCollection(
        process,
        labelName = 'CA8PFCHSPruned',
        postfix = postfix,
        jetSource = cms.InputTag('ca8PFJetsCHSPruned'),
        algo = 'CA8Pruned',
        jetCorrections = ('AK7PFchs', inputJetCorrLabel, ''),
        btagDiscriminators = btagDiscriminators,
        btagInfos = btagInfos,
        jetTrackAssociation = True,
        outputModules = outputModules,
        )
    if runOnMC:  getattr(process, 'patJetGenJetMatchPatJetsCA8PFCHSPruned'+postfix).matched = cms.InputTag('ca8GenJetsNoNu')

    addJetCollection(
        process,
        labelName = 'CA8PFCHSPrunedSubJets',
        postfix = postfix,
        jetSource = cms.InputTag('ca8PFJetsCHSPruned', 'SubJets'),
        algo = 'CA8Pruned',
        jetCorrections = ('AK5PFchs', inputJetCorrLabel, ''),
        btagDiscriminators = btagDiscriminators,
        btagInfos = btagInfos,
        jetTrackAssociation = True,
        outputModules = outputModules,
        )
    if runOnMC:  getattr(process, 'patJetGenJetMatchPatJetsCA8PFCHSPrunedSubJets'+postfix).matched = cms.InputTag('ca8GenJetsNoNuPruned', 'SubJets')

    ## BoostedJetMerger
    ## see https://twiki.cern.ch/twiki/bin/view/CMS/BoostedBTagSWSetup
    #process.selectedPatJetsCA8PFCHSPrunedPacked = cms.EDProducer('BoostedJetMerger',
    #    jetSrc = cms.InputTag('selectedPatJetsCA8PFCHSPruned'+postfix),
    #    subjetSrc = cms.InputTag('selectedPatJetsCA8PFCHSPrunedSubJets'+postfix)
    #)
    #process.out.outputCommands += ['keep *_selectedPatJetsCA8PFCHSPrunedPacked%s_*_*' % postfix]

if doCA8TopTag:
    addJetCollection(
        process,
        labelName = 'CA8PFCHSTopTag',
        postfix = postfix,
        jetSource = cms.InputTag('cmsTopTagPFJetsCHS'),
        algo = 'CA8TopTag',
        jetCorrections = ('AK7PFchs', inputJetCorrLabel, ''),
        #btagDiscriminators = btagDiscriminators,
        #btagInfos = btagInfos,
        #jetTrackAssociation = True,
        outputModules = outputModules,
        )
    if runOnMC:  getattr(process, 'patJetGenJetMatchPatJetsCA8PFCHSTopTag'+postfix).matched = cms.InputTag('ca8GenJetsNoNu')

    addJetCollection(
        process,
        labelName = 'CA8PFCHSTopTagSubJets',
        postfix = postfix,
        jetSource = cms.InputTag('cmsTopTagPFJetsCHS', 'caTopSubJets'),
        algo = 'CA8TopTag',
        jetCorrections = ('AK5PFchs', inputJetCorrLabel, ''),
        #btagDiscriminators = btagDiscriminators,
        #btagInfos = btagInfos,
        #jetTrackAssociation = True,
        outputModules = outputModules,
        )
    if runOnMC:  getattr(process, 'patJetGenJetMatchPatJetsCA8PFCHSTopTagSubJets'+postfix).matched = cms.InputTag('ca8GenJetsNoNuTopTag', 'caTopSubJets')

if doCA15:
    addJetCollection(
        process,
        labelName = 'CA15PFCHSFiltered',
        postfix = postfix,
        jetSource = cms.InputTag('ca15PFJetsCHSFiltered'),
        algo = 'CA15Filtered',
        jetCorrections = ('AK7PFchs', inputJetCorrLabel, ''),
        #btagDiscriminators = btagDiscriminators,
        #btagInfos = btagInfos,
        #jetTrackAssociation = True,
        outputModules = outputModules,
        )
    if runOnMC:  getattr(process, 'patJetGenJetMatchPatJetsCA15PFCHSFiltered'+postfix).matched = cms.InputTag('ca15GenJetsNoNu')

    addJetCollection(
        process,
        labelName = 'CA15PFCHSFilteredSubJets',
        postfix = postfix,
        jetSource = cms.InputTag('ca15PFJetsCHSFiltered', 'SubJets'),
        algo = 'CA15Filtered',
        jetCorrections = ('AK5PFchs', inputJetCorrLabel, ''),
        btagDiscriminators = btagDiscriminators,
        btagInfos = btagInfos,
        jetTrackAssociation = True,
        outputModules = outputModules,
        )
    if runOnMC:  getattr(process, 'patJetGenJetMatchPatJetsCA15PFCHSFilteredSubJets'+postfix).matched = cms.InputTag('ca15GenJetsNoNuFiltered', 'SubJets')

# Do switchJetCollection(...) at the end
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
# Pileup Jet ID
#   https://twiki.cern.ch/twiki/bin/view/CMS/PileupJetID
# ------------------------------------------------------------------------------
from RecoJets.JetProducers.PileupJetID_53x_cfi import pileupJetIdProducerChs, pileupJetIdProducer
process.pileupJetIdProducerChs = pileupJetIdProducerChs.clone( jets = cms.InputTag('selectedPatJets'+postfix) )
process.pileupJetIdProducer = pileupJetIdProducer.clone( jets = cms.InputTag('selectedPatJetsAK5PF'+postfix) )

# ------------------------------------------------------------------------------
# Q/G Tagger
#   https://twiki.cern.ch/twiki/bin/view/CMS/GluonTag
# ------------------------------------------------------------------------------
#process.load('RecoJets.JetProducers.QGTagger_cfi')

# ------------------------------------------------------------------------------
# Njettiness
# ------------------------------------------------------------------------------
#process.load('RecoJets.JetProducers.nJettinessAdder_cfi')
##process.selectedPatJetsCA8CHSwithNsub = cms.EDProducer("NjettinessAdder",
##    src=cms.InputTag("selectedPatJetsCA8CHSWithBeta"),
##    cone=cms.double(0.8)
##    )

# ------------------------------------------------------------------------------
# Cut
# ------------------------------------------------------------------------------
for labelName in ['', 'AK5PF', 'AK4PF', 'AK4PFCHS', 'CA8PFCHSPrunedSubJets', 'CA8PFCHSTopTagSubJets', 'CA15PFCHSFilteredSubJets']:
    if hasattr(process,"selectedPatJets"+labelName+postfix):
        getattr(process,"selectedPatJets"+labelName+postfix).cut = cms.string("pt > 15 & abs(eta) < 5.0")
for labelName in ['AK8PF', 'AK8PFCHS', 'CA8PFCHS', 'CA8PFCHSPruned']:
    if hasattr(process,"selectedPatJets"+labelName+postfix):
        getattr(process,"selectedPatJets"+labelName+postfix).cut = cms.string("pt > 25 & abs(eta) < 5.0")  # jetPtMin = 15.0 at clustering
for labelName in ['CA8PFCHSTopTag', 'CA15PFCHSFiltered']:
    if hasattr(process,"selectedPatJets"+labelName+postfix):
        getattr(process,"selectedPatJets"+labelName+postfix).cut = cms.string("pt > 100 & abs(eta) < 5.0")  # jetPtMin = 100.0 at clustering


################################################################################
# MET                                                                          #
################################################################################
doMetCorr = False
doMetUncert = True
# ------------------------------------------------------------------------------
# MET Corrections
#   https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMetAnalysis
#   https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideMETRecipe
# ------------------------------------------------------------------------------

if doMetCorr:
    # Taken from https://github.com/TaiSakuma/WorkBookMet/blob/master/corrMet_cfg.py
    ##________________________________________________________________________||
    process.load("JetMETCorrections.Type1MET.correctionTermsPfMetType1Type2_cff")

    if runOnMC:
        process.corrPfMetType1.jetCorrLabel = cms.string("ak5PFL1FastL2L3")
    else:
        process.corrPfMetType1.jetCorrLabel = cms.string("ak5PFL1FastL2L3Residual")

    ##________________________________________________________________________||
    process.load("JetMETCorrections.Type1MET.correctionTermsPfMetType0PFCandidate_cff")

    ##________________________________________________________________________||
    process.load("JetMETCorrections.Type1MET.correctionTermsPfMetType0RecoTrack_cff")

    ##________________________________________________________________________||
    process.load("JetMETCorrections.Type1MET.correctionTermsPfMetShiftXY_cff")

    if runOnMC:
        process.corrPfMetShiftXY.parameter = process.pfMEtSysShiftCorrParameters_2012runABCDvsNvtx_mc
    else:
        process.corrPfMetShiftXY.parameter = process.pfMEtSysShiftCorrParameters_2012runABCDvsNvtx_data

    ##________________________________________________________________________||
    process.load("JetMETCorrections.Type1MET.correctedMet_cff")

    ##________________________________________________________________________||
    process.corrMetSequence = cms.Sequence(
        process.correctionTermsPfMetType1Type2 +
        process.correctionTermsPfMetType0RecoTrack +
        process.correctionTermsPfMetType0PFCandidate +
        process.correctionTermsPfMetShiftXY +
        #process.correctionTermsCaloMet +
        #process.caloMetT1 +
        #process.caloMetT1T2 +
        process.pfMetT0rt +
        process.pfMetT0rtT1 +
        process.pfMetT0pc +
        process.pfMetT0pcT1 +
        process.pfMetT0rtTxy +
        process.pfMetT0rtT1Txy +
        process.pfMetT0pcTxy +
        process.pfMetT0pcT1Txy +
        process.pfMetT1 +
        process.pfMetT1Txy
    )
    process.p_corrMet = cms.Path(process.corrMetSequence)
    process.out.outputCommands += ['keep *_pfMetT*__*' ]

# ------------------------------------------------------------------------------
# MET Uncertainties
# ------------------------------------------------------------------------------
if doMetUncert:
    pass


# apply type I/type I + II PFMEt corrections to pat::MET object
# and estimate systematic uncertainties on MET
#from PhysicsTools.PatUtils.tools.metUncertaintyTools import runMEtUncertainties
#runMEtUncertainties(process)

# ------------------------------------------------------------------------------
# MET Filters
#   https://twiki.cern.ch/twiki/bin/view/CMS/MissingETOptionalFilters
# ------------------------------------------------------------------------------
process.load("RecoMET.METFilters.metFilters_cff")
process.p_HBHENoiseFilter = cms.Path( process.HBHENoiseFilter )
process.p_CSCTightHaloFilter = cms.Path( process.CSCTightHaloFilter )
process.p_hcalLaserEventFilter = cms.Path( process.hcalLaserEventFilter )
process.p_EcalDeadCellTriggerPrimitiveFilter = cms.Path( process.EcalDeadCellTriggerPrimitiveFilter )
process.p_trackingFailureFilter = cms.Path( process.goodVertices * process.trackingFailureFilter )
process.p_eeBadScFilter = cms.Path( process.eeBadScFilter )
process.p_ecalLaserCorrFilter = cms.Path( process.ecalLaserCorrFilter )
if not runOnMC:
    process.p_trkPOGFilters = cms.Path( process.trkPOGFilters )
process.p_metFilters = cms.Path( process.metFilters )  # combined

# Even more filters
process.load("RecoMET.METFilters.metOptionalFilters_cff")
process.jetIDFailure.JetSource = cms.InputTag('patJets'+postfix)
process.p_goodVerticesFilter = cms.Path( process.goodVerticesFilter )
process.p_noscraping = cms.Path( process.noscraping )
if not runOnMC:
    process.p_hcallasereventfilter2012 = cms.Path( process.hcallasereventfilter2012 )
process.p_EcalDeadCellBoundaryEnergyFilter = cms.Path( process.EcalDeadCellBoundaryEnergyFilter )
process.p_tobtecfakesFilters = cms.Path( process.tobtecfakesFilters )
process.p_jetIDFailure = cms.Path( process.jetIDFailure )
process.p_badMuonFilters = cms.Path( process.badMuonFilters )
process.p_eeNoiseFilter = cms.Path( process.eeNoiseFilter )
process.p_metOptionalFilters = cms.Path( process.metOptionalFilters )  # combined


################################################################################
# Miscellaneous                                                                #
################################################################################

# ------------------------------------------------------------------------------
# Trigger
#   https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePATTrigger
# ------------------------------------------------------------------------------
#from PhysicsTools.PatAlgos.tools.trigTools import switchOnTrigger
#switchOnTrigger( process )
from PhysicsTools.PatAlgos.tools.trigTools import switchOnTriggerStandAlone
switchOnTriggerStandAlone( process )
process.patTrigger.packTriggerPathNames = cms.bool(True)

# ------------------------------------------------------------------------------
# GenParticles
#   https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideGenParticlePruner
# ------------------------------------------------------------------------------
if runOnMC:
    #process.load("SimGeneral.HepPDTESSource.pythiapdt_cfi")
    prunedGenParticles = cms.EDProducer("GenParticlePruner",
        src = cms.InputTag("genParticles"),
        select = cms.vstring(
            "drop *", # this is the default
            "keep status == 3",  #keep event summary status3 (for pythia)
            "keep status == 21", #keep particles from the hard matrix element (for pythia8)
            "keep status == 22", #keep particles from the hard matrix element (for pythia8)
            "keep status == 23", #keep particles from the hard matrix element (for pythia8)
            "++keep 23 <= abs(pdgId) <= 25",                                   # keep W,Z,H and their ancestors
            "++keep abs(pdgId) == 11 || abs(pdgId) == 13 || abs(pdgId) == 15", # keep leptons and their ancestors
            "++keep abs(pdgId) == 12 || abs(pdgId) == 14 || abs(pdgId) == 16", # keep neutrinos and their ancestors
            "++keep pdgId == 22 && status == 1 && pt > 10",                    # keep gamma above 10 GeV
            "drop   status == 2",                                              # drop the shower part of the history
            "keep++ abs(pdgId) == 15",                                         # but keep keep taus with their daughters
            "++keep 4 <= abs(pdgId) <= 6 ",                                    # keep also heavy quarks
            "++keep (400 < abs(pdgId) < 600) || (4000 < abs(pdgId) < 6000)",   # and their hadrons
            "drop   status == 2 && abs(pdgId) == 21",                          # but remove again gluons in the inheritance chain
        )
    )
    process.out.outputCommands += ['keep *_prunedGenParticles_*_*']

# IVF and BHadron -- FIXME

# HbbAnalyzers -- FIXME

# ------------------------------------------------------------------------------
# Dump flat python cfg
# ------------------------------------------------------------------------------
print "process.out.outputCommands = ",
print process.out.outputCommands
print

temp = process.dumpPython()
with open("dump.py",'w') as f:
    f.write(temp)
