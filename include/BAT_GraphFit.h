// ***************************************************************
// This file was created using the bat-project script.
// bat-project is part of Bayesian Analysis Toolkit (BAT).
// BAT can be downloaded from http://mpp.mpg.de/bat
// ***************************************************************

#ifndef __BAT__BAT_GRAPHFIT__H
#define __BAT__BAT_GRAPHFIT__H

#include <BAT/BCModel.h>
#include "TF1.h"
#include "TGraphAsymmErrors.h"
#include "TCanvas.h"
#include "TGraph.h"
#include <string>
#include <vector>

// This is a BAT_GraphFit header file.
// Model source code is located in file BAT_GraphFit/BAT_GraphFit.cxx

// ---------------------------------------------------------
class BAT_GraphFit : public BCModel
{

public:

  TF1 *fTF1;
  TGraphAsymmErrors *fGraph;
  int fNpar;
  double fFitLow,fFitHigh;
  bool fVerbose;
  // Constructor
  BAT_GraphFit(const std::string& name,TF1 *&f,bool verbose,TString mode);
  void SetGraph(TGraphAsymmErrors *&g);
  void SetHisto(TH1D*&h);
  void SetBinomGraph(TGraph *&gTrial,TGraph *&gSuccess);

  void Plot(TString goption="APE");
  TString fMode;
  // Destructor
  ~BAT_GraphFit();

  // Overload LogLikelihood to implement model
  double LogLikelihood(const std::vector<double>& pars);
  
  
};
// ---------------------------------------------------------

#endif