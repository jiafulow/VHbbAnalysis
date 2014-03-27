import FWCore.ParameterSet.Config as cms

## The good vertices filter __________________________________________________||
from RecoMET.METFilters.metFilters_cff import goodVertices
goodVerticesFilter = goodVertices.clone( filter = cms.bool(True) )

## The beam scraping filter __________________________________________________||
noscraping = cms.EDFilter(
    "FilterOutScraping",
    applyfilter = cms.untracked.bool(True),
    debugOn = cms.untracked.bool(False),
    numtrack = cms.untracked.uint32(10),
    thresh = cms.untracked.double(0.25)
    )

## The HCAL laser filter (Nov 2012) __________________________________________||
from EventFilter.HcalRawToDigi.hcallasereventfilter2012_cfi import hcallasereventfilter2012

## The ECAL dead cell boundary energy filter _________________________________||
# The section below is for the filter on Boundary Energy. Available in AOD in CMSSW>44x
from RecoMET.METFilters.EcalDeadCellBoundaryEnergyFilter_cfi import EcalDeadCellBoundaryEnergyFilter
EcalDeadCellBoundaryEnergyFilter.taggingMode = cms.bool(False)
EcalDeadCellBoundaryEnergyFilter.cutBoundEnergyDeadCellsEB=cms.untracked.double(10)
EcalDeadCellBoundaryEnergyFilter.cutBoundEnergyDeadCellsEE=cms.untracked.double(10)
EcalDeadCellBoundaryEnergyFilter.cutBoundEnergyGapEB=cms.untracked.double(100)
EcalDeadCellBoundaryEnergyFilter.cutBoundEnergyGapEE=cms.untracked.double(100)
EcalDeadCellBoundaryEnergyFilter.enableGap=cms.untracked.bool(False)
EcalDeadCellBoundaryEnergyFilter.limitDeadCellToChannelStatusEB = cms.vint32(12,14)
EcalDeadCellBoundaryEnergyFilter.limitDeadCellToChannelStatusEE = cms.vint32(12,14)

## Tracking TOBTEC fakes filter ______________________________________________||
from RecoMET.METFilters.trackingPOGFilters_cfi import tobtecfakesfilter
tobtecfakesFilters = cms.Sequence( ~tobtecfakesfilter )

## JetID failure filter ______________________________________________________||
from RecoMET.METFilters.jetIDFailureFilter_cfi import jetIDFailure
jetIDFailure.JetSource = cms.InputTag('patJetsPFlow')
jetIDFailure.MinJetPt = cms.double(30.0)
jetIDFailure.MaxJetEta = cms.double(999.0)

## Muons with wrong momenta (PF only) ________________________________________||
from RecoMET.METFilters.inconsistentMuonPFCandidateFilter_cfi import inconsistentMuonPFCandidateFilter
from RecoMET.METFilters.greedyMuonPFCandidateFilter_cfi import greedyMuonPFCandidateFilter
badMuonFilters = cms.Sequence( greedyMuonPFCandidateFilter * inconsistentMuonPFCandidateFilter )

## EE noise filter ___________________________________________________________||
from RecoMET.METFilters.eeNoiseFilter_cfi import eeNoiseFilter

metOptionalFilters = cms.Sequence(
   goodVerticesFilter *
   noscraping *
   hcallasereventfilter2012 *
   EcalDeadCellBoundaryEnergyFilter *
   tobtecfakesFilters *
   jetIDFailure *
   badMuonFilters *
   eeNoiseFilter
)

