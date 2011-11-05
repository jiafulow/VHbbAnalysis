/*************************************************
  Drawing plots and saving images of histograms
  can be tedious. This automates the task.
  
  Finds all the directories in a given ROOT file
  and creates a directory structure exactly like 
  it and fills it with image files of hitograms 
  found. A good example of a recursive function.

  Can be run from a bash prompt as well:
    root -b -l -q "saveHistograms.C(\"aFile.root\",\"gif\",640,480)"

  Michael B. Anderson
  May 23, 2008
*************************************************/

#include <iostream>
#include "TString.h"
#include "TFile.h"
//#include "TChain.h"
#include "TH1.h"
//#include "TTree.h"
#include "TKey.h"
//#include "Riostream.h"
#include "TSystem.h"
#include "TCanvas.h"

//TFile *sourceFile;
//TObject *obj;
//TString outputFolder;
//TH1 *h;
//TCanvas *canvasDefault;

// *******************************************
// Variables

//String outputType;
//Int_t outputXsize;
//Int_t outputYsize;

TString drawOptions1D("");   // Drawing options for 
                             // 1D histograms.
TString drawOptions2D("box");// Drawing options for
                             // 2D histograms.

//int lineColor = 4; // 2 = Red, 3 = Green, 4 = Blue
//int lineWidth = 2; // Line width for 1D histograms.

Bool_t displayStatsBox = 1;  // 0 = false, 1 = true
//Bool_t autoLabelXaxis  = 0;
Bool_t autoLogYaxis    = 0;
Bool_t printOutput     = 1;

// End of Variables
// *******************************************

using namespace std;

void recurseOverKeys(TDirectory *target, TString imageType);

void saveHistograms(TString fileName,
	            TString imageType = "png", 
	            int outputWidth  = 740, 
	            int outputHeight = 700) {

  TFile* sourceFile = TFile::Open( fileName );

//  TString outputFolder = fileName+"images/"; // Blank to use current directory,
                                     // or, for a specific dir type
                                     // something like "images/"
//  outputFolder.ReplaceAll(".root","");
    gSystem->MakeDirectory("images");

//  outputType = "."+imageType;
//  outputXsize = outputWidth; 
//  outputYsize = outputHeight;
//  canvasDefault = new TCanvas("canvasDefault","testCanvas",outputWidth,outputHeight);

  // Change settings on plotting histograms
//  gStyle->SetOptStat(111111);  // This will cause overflow and underflow to be shown
//  gStyle->SetHistLineWidth(2); // Set the line width to 2 pixels
  //gStyle->SetTitleFontSize(0.035); // Shrink Histogram Title Size

  // Now actually find all the directories, histograms, and save them..
  recurseOverKeys(sourceFile, imageType);  

  sourceFile->Close();

//  TString currentDir = gSystem->pwd();
//  cout << "Done. See images in:" << endl << currentDir << "/" << outputFolder << endl;
}

void recurseOverKeys(TDirectory *target, TString imageType) {
 
//  TString path( (char*)strstr( target->GetPath(), ":" ) );
//  path.Remove( 0, 2 );

//  cout << path << endl;

//  sourceFile->cd( path );
  target->cd();
  TDirectory *current_sourcedir = gDirectory;

  TKey *key;
  TIter nextkey(current_sourcedir->GetListOfKeys());
  TCanvas *canvasDefault = new TCanvas();

  while ( (key = (TKey*)nextkey() ) ) {

    TObject* obj = key->ReadObj();

    if (obj->IsA()->InheritsFrom("TH1") ) {

      // **************************
      // Plot & Save this Histogram
      TH1* h = (TH1*)obj;
      h->SetStats(displayStatsBox);

      TString histName = h->GetName(); 

      // Now to label the X-axis!
//      if (autoLabelXaxis) {
//	if ( histName.Contains("Phi") ) {
//	  h->GetXaxis()->SetTitle("#phi");
//	} else if ( histName.Contains("Eta") ) {
//	  h->GetXaxis()->SetTitle("#eta");
//	} else if ( histName.Contains("Pt") ) {
//	  h->GetXaxis()->SetTitle("p_{T} (GeV)");
//	} else if ( histName.Contains("Et") ) {
//	  h->GetXaxis()->SetTitle("E_{T} (GeV)");
//	}
//      }

//      h->SetLineColor(lineColor);
//      h->SetLineWidth(lineWidth);


      // ********************************
      // A trick to decide whether to have log or no-log y axis
      // get hist max y value
      if (autoLogYaxis) {
        Double_t testYvalue = h->GetMaximum();
        //cout << testYvalue << endl;

        if (testYvalue > 1.0) {
          Double_t maxy = log10(testYvalue);

          // get hist min y value
          Double_t miny = log10(h->GetMinimum(1.0));

          // log scale if more than 3 powers of 10 between low and high bins
          if ( (maxy-miny) > 3.0 ) {
            canvasDefault->SetLogy(1);
          }
        }
      }
      // End of log or no-log y axis decision
      // ********************************

      h->Draw(drawOptions1D);
      canvasDefault->Modified();
      canvasDefault->Update();
      
//      gPad->Print(outputFolder+path+"/"+histName+outputType);
      TString outputFolder = "images/";
      gPad->Print(outputFolder+histName+"."+imageType);
      // To store the root file name in image file name:
      //canvasDefault->Print(outputFolder+histFileName+histName+outputType);
//      if (printOutput) cout << outputFolder+path+"/"+histName+outputType << endl;

      canvasDefault->SetLogy(0); // reset to no-log - prevents errors
      // **************************

    } else if ( obj->IsA()->InheritsFrom( "TDirectory" ) ) {
      // it's a subdirectory

      cout << "Found subdirectory " << obj->GetName() << endl;
//      gSystem->MakeDirectory(outputFolder+path+"/"+obj->GetName());

      // obj is now the starting point of another round of merging
      // obj still knows its depth within the target file via
      // GetPath(), so we can still figure out where we are in the recursion
      recurseOverKeys((TDirectory*)obj, imageType);

    } // end of IF a TDriectory
  } // end of LOOP over keys
}

