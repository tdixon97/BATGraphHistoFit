#include "BatGraphFitter.h"
#include "BAT_GraphFit.h"
#include "TFile.h"
#include "TCanvas.h"
#include <BAT/BCH1D.h>
#include "TH1D.h"
#include <BAT/BCParameter.h>
#include <BAT/BCModel.h>

BatGraphFitter::BatGraphFitter(TGraphAsymmErrors *&g)
{
  // constructor

  
  // in the constructor we need to create the

  //gaussian likelihood
  fMode="G";
  
  fGraph = g;
  
}
BatGraphFitter::BatGraphFitter(TH1D*&h)
{
  fMode="P";
  fHisto=h;
}

BatGraphFitter::BatGraphFitter(TGraph *&g,double error)
{
  //constructor for graph

  fGraph=new TGraphAsymmErrors();
  fMode="G";
  std::cout<<"Warning : Attempt to fit a plain TGraph without errors - likelihood fit cannot be performed erros assumed constant (and 1):"<<std::endl;
  std::cout<<"You can set the value of the error to avoid numerical issies"<<std::endl;
  for (int i=0;i<g->GetN();i++)
    {
      fGraph->SetPoint(i,g->GetX()[i],g->GetY()[i]);
      fGraph->SetPointEYhigh(i,error);
      fGraph->SetPointEYlow(i,error);
    }	

}


BatGraphFitter::BatGraphFitter(TGraph *&gTrial,TGraph *&gSuccess)
{

  fGraphBinomTrial = gTrial;
  fGraphBinomSuccess=gSuccess;
  if (gTrial->GetN()!=gSuccess->GetN())
    {
      std::cout<<"ERROR number of points of trials not the same as successes"<<std::endl;
    }
  for (int i=0;i<gTrial->GetN();i++)
    {
      fGraph->SetPoint(i,gTrial->GetX()[i],gSuccess->GetY()[i]/gTrial->GetY()[i]);
      fGraph->SetPointEYhigh(i,1);
      fGraph->SetPointEYlow(i,1);
    }

  fMode="B";
}

BatGraphFitter::BatGraphFitter(TGraphErrors *&g)
{
  // constructor for graph
  

  fMode="G";
  // in the constructor we need to create the
  fGraph=new TGraphAsymmErrors();
  for (int i=0;i<g->GetN();i++)
    {
      fGraph->SetPoint(i,g->GetX()[i],g->GetY()[i]);
      fGraph->SetPointEYhigh(i,g->GetEY()[i]);
      fGraph->SetPointEYlow(i,g->GetEY()[i]);
    }

}

void BatGraphFitter::Fit(TF1 *&f,TString option,TString goption,double low,double high,int prec)
{


  // parse the option - currently the only one to be implemented is R

  if (option.Contains("R")==1)
    {
      f->SetRange(low,high);
    }
  
  
  fTF1=f;

  // add some code to delete the old model (or reset it)

  if (fMode=="G")
    fModel= new BAT_GraphFit("fit",fTF1,fGraph,1,fMode);
  else if(fMode=="P")
    fModel= new BAT_GraphFit("fit",fTF1,fTH1,1,fMode);
  else if (fMode=="B")
    fModel= new BAT_GraphFit("fit",fTF1,fGraph,fBinomTrial,fBinomSuccess,1,fMode);

  fModel->WriteMarkovChain("output_mcmc.root", "RECREATE");

  fModel->MarginalizeAll(BCIntegrate::kMargMetropolis);
  fModel->SetPrecision(BCEngineMCMC::kHigh);
  
  if (prec==1)
    {
      fModel->SetPrecision(BCEngineMCMC::kLow);
    }
  else if (prec==2)
    {
      fModel->SetPrecision(BCEngineMCMC::kMedium);
    }
  else if (prec==3)
    {
      fModel->SetPrecision(BCEngineMCMC::kHigh);
    }
  else if(prec==4)
    {
      fModel->SetPrecision(BCEngineMCMC::kVeryHigh);
    }





  fModel->FindMode(fModel->GetBestFitParameters());
  std::cout<<"                      "<<std::endl;
  std::cout<<"Fit with BAT best fit value"<<std::endl;
  std::cout<<"****************************************"<<std::endl;
  for (int i=0;i<fModel->GetBestFitParameters().size(); i++)
    {
      BCParameter  parameter = fModel->GetParameter(i);
      fTF1->SetParameter(i,fModel->GetBestFitParameters()[i]);
      fTF1->SetParError(i,fModel->GetMarginalized(parameter.GetName().data()).GetHistogram()->GetRMS());
      std::cout<<"parameter "<<parameter.GetName().data()<<
	" "<<fModel->GetBestFitParameters()[i]<<"           +/- "<<
	fModel->GetMarginalized(parameter.GetName().data()).GetHistogram()->GetRMS()<<std::endl;
    }      std::cout<<" "<<std::endl;
      
      // todo replace with a better error estimate
    

  //  fModel->WriteMarkovChain("output_mcmc.root", "RECREATE");                                                                                                                                     
  
  fModel->Plot(goption);

}
