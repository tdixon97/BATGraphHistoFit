#include "src/BatGraphFitter.h"
#include "TFile.h"
#include <BAT/BCLog.h>
#include "TRandom3.h"
#include "TLine.h"
#include "TF1.h"
#include "TApplication.h"
#include "TRandom3.h"
#include "TLatex.h"


// Method to get the integral assuming                                                                                                                                                                    //-------------------------------------------------------                                                                                                                                                  

double GetIntegral(double low,double high,TH1D*&h)
{


  int lowbin = h->FindBin(low);
  int highbin=h->FindBin(high);

  double integral = (h->Integral(lowbin+1,highbin)+h->GetBinContent(lowbin)*(double)rand()/RAND_MAX)/h->Integral();

  return integral;





}
double p_2_z(double p)
{
  // convert p-value to significance
  
  return TMath::ErfcInverse(2*p)*sqrt(2);
}

void GetLimit(double &sens,TH1D *&hsens,TH1D* &hinv_sens,double t,bool & foundsens,TH1D *&test_stat,double p_old,int j,double Nsignals,double maxr)
{

  double p =GetIntegral(t,150,test_stat);
  if (!foundsens&& p<0.1)
    {
      double diff=p_old-p;
      double frac= (p_old-0.10)/diff;
      sens = ((j-1)/(double)Nsignals)*(maxr)+(maxr)*frac/((double)Nsignals);
 
      hsens->Fill((1e-27)*1/sens);
      hinv_sens->Fill(1e27*sens);
      foundsens=1;
    }









}

// METHOD TO MAKE A PROFILED LIKELIHOOD
//-----------------------------------------------------------------------------------------------------------
TGraphErrors *profile(TH2D* &h,TString axis,double logLL)
{

  TH1D *hout;
  int counter=0;
  double Nmax =h->GetMaximum();
  TGraphErrors *g = new TGraphErrors();
  if (axis=="X")
    {
      hout->Clear();
      hout->Reset();
      
      TH1D *h1;
      TH1D *h2;
      for (int i=1;i<h->GetNbinsX();i++)
	{
	  
	  h1=(TH1D*)h->ProjectionY("h1",i,i+1);
	  h2=(TH1D*)h->ProjectionX("h2",i,i+1);

	  if (h1->GetMaximum()>0)
	    {

	      g->SetPoint(counter,h2->GetBinCenter(h2->FindBin(i)),-log(h1->GetMaximum())+log(Nmax)-logLL);
	      g->SetPointError(counter,0,1/sqrt(h1->GetMaximum()));
	      counter++;
	      
	    }
	}
    }
  
  else
    {

      TH1D *h1;
      TH1D *h2;
      for (int i=1;i<h->GetNbinsY();i++)
        {
          h1=(TH1D*)h->ProjectionX("h1",i,i+1);
	  h2=(TH1D*)h->ProjectionY("h2",i,i+1);

	  if (h1->GetMaximum()>0)
	    {

	      g->SetPoint(counter,h2->GetBinCenter(h2->FindBin(i)),-log(h1->GetMaximum())+log(Nmax)-logLL);
	      g->SetPointError(counter,0,1/sqrt(h1->GetMaximum()));
	      counter++;

            }
	  
	  
	}
      
    }
  return g;


  
}	  




double  GenToy(TH1D*&h,TRandom3 *&rand,double Nb,double Elow,double Ehigh,double Ns,double Q,double dE)
{
  // generate a toy given a certain number of background counts (on avg) in range [Nb,Ns] and a certain number of signal counts at energy Q with resolution dE (sigma)
  
  int N=rand->Poisson(Nb);
  double Na=rand->Poisson(Ns);
  for (int i=1;i<h->GetNbinsX()+1;i++)
    h->SetBinContent(i,0);

  for (int i=0;i<N;i++)
    {
      h->Fill(rand->Uniform(100)+Elow);
   }
  for (int i=0;i<Na;i++)
    {
      double E=rand->Gaus(Q,dE);
      h->Fill(E);
      
    }
  return Na;
}


double scale_2_rate(double eff,double m,double eta,double W)
{
  // convert half-life to a number of events per year

  return log(2)*m*eff*eta*6.066e26/(W);
}

		  

double T12_2_mbb(double T,double M,double G)
{
  return 511*1e6*sqrt(1/(G*T))/(M*1.27*1.27);
}



void CreatePValuePlot(std::vector<TH1D*>p_value,std::vector<TH1D*>test_stat,std::vector<TH1D*>test_stat_bkg,int Nsignals,double cut,TString label,TCanvas *&can,TString name,double maxr,TGraphErrors *&prob_plot)
{
  // fill graphs and compute exclusion sensitivity                                                                                                                                                     
  //----------------------------------------------------------------------------------------------
  
  TGraphAsymmErrors* p1=new TGraphAsymmErrors();
  TGraphAsymmErrors* p2=new TGraphAsymmErrors();
  TGraph *gmed=new TGraph();


  prob_plot=new TGraphErrors();
  std::cout<<"p - value has size "<<p_value.size()<<std::endl;
  std::cout<<"test stat / test stat bkg has size"<<test_stat.size()<<" "<<test_stat_bkg.size()<<" Nsig = "<<Nsignals<<std::endl;

  for (int j=0;j<Nsignals;j++)
    {
      double invT12=(j/(double)Nsignals)*maxr;



      p_value[j]->SetTitle(Form("Distribution of p_S for T_{1/2}^{-1}  =  %E yr^{-1} ; p ; counts",invT12));
      p_value[j]->Draw();
      can->Draw();
      can->Print(Form("output/CUPID_sens_%s/%s_p_value_%i.pdf",name.Data(),label.Data(),j));

      double Np=p_value[j]->Integral(0,p_value[j]->FindBin(cut));
      double Nt= p_value[j]->Integral();

      double p = Np/Nt;
      double q=1-p;
      double ep=sqrt(p*q/Nt);

      prob_plot->SetPoint(j,invT12,p);
      prob_plot->SetPointError(j,0,ep);
      
      

      q;
      q=(1-0.9544)/2.;


      double m2sig;


      p_value[j]->GetQuantiles(1,&m2sig,&q);

      q=(1-0.682)/2.;
      double m1sig;
      p_value[j]->GetQuantiles(1,&m1sig,&q);

      double med;
      q=0.5;
      p_value[j]->GetQuantiles(1,&med,&q);


      double p1sig;
      q=1-((1-0.682)/2.);
      p_value[j]->GetQuantiles(1,&p1sig,&q);

      double p2sig;
      q=1-((1-0.9544)/2.);

      p_value[j]->GetQuantiles(1,&p2sig,&q);

      
      if (med<cut)
        {
          std::cout<<"Sensitivity = "<<j<<" or "<<Form("%E",1/invT12)<<std::endl;
        }
      std::cout<<"quantiles = "<<m2sig<<" "<<m1sig<<" "<<med<<" "<<p1sig<<" "<<p2sig<<std::endl;

      p1->SetPoint(j,invT12,med);
      p1->SetPointError(j,0,0,med-m1sig,p1sig-med);

      p2->SetPoint(j,invT12,med);
      p2->SetPointError(j,0,0,med-m2sig,p2sig-med);

      gmed->SetPoint(j,invT12,med);

      test_stat[j]->Draw();
      test_stat_bkg[j]->SetLineColor(2);
      test_stat_bkg[j]->Draw("same");
      can->Draw();
      can->Print(Form("output/CUPID_sens_%s/%s_test_stat_comp_%i.C",name.Data(),label.Data(),j));

    }


  // create some more plots                           
  // ------------------------------------------------------------------------------                                                                                                                                                                                                                                                                                                                                                                                       

  p1->SetLineColorAlpha(3,0.2);
  p2->SetLineColorAlpha(kOrange-3,0.2);
  p1->SetFillColorAlpha(3,0.7);
  p2->SetFillColorAlpha(kOrange-3,0.7);

  gmed->SetTitle(" ; (T_{1/2})^{-1} [yr^{-1}] ; p-value ; ");

  TLine * l =new TLine(0,cut,maxr,cut);
  l->SetLineColor(2);
  
  gmed->Draw("AL");
  p2->Draw("3same");
  p1->Draw("3same");
  gmed->Draw("Lsame");
  l->Draw("same");
  can->Draw();
  can->Print(Form("output/CUPID_sens_%s/%s_exc.C",name.Data(),label.Data()));
  can->Print(Form("output/CUPID_sens_%s/%s_toys_bkg_0_sig.pdf)",name.Data(),label.Data()),"pdf");




}

int FreqLimit()
{
  // SET THE PARAMETERS
  // -----------------------------------------------------------------------------------------------------------------------------------------
  
  TString name="reach_frequentist";
  double Qbb=3034.4;
  double eff=0.9*0.79;
  double dE = 5/2.355;
  double T=10;
  double m = 472;
  double eta=0.95;
  double W=177.7;
  double b=2e-5;

 

  // Create some objects
  //------------------------------------------------------------------------------------------------------------------------------------------
  TRandom3 *rand=new TRandom3(0);
  TCanvas *can = new TCanvas();
  can->Print(Form("output/CUPID_sens_%s/toys_0_sig.pdf(",name.Data()),"pdf");
  gStyle->SetOptStat(0);
  TH1D * h = new TH1D("h","h",100,2984,3084);
  double scale = T*log(2)*m*eff*eta*6.066e26/(W);

  double Nb=100*b*m*T;


  // CREATE FIT FUNCTIONS
  //-------------------------------------------------------------------------------------------------------------------------------------------
  
  TF1 *fit = new TF1("f",Form("[0]*%f+(%f*[1])*TMath::Gaus(x,%f,%f)/(sqrt(2*TMath::Pi())*%f)",m*T,scale,Qbb,dE,dE),2984,3084);
  TF1 *fit_fix = new TF1("fit_fix",Form("[0]*%f+%f*TMath::Gaus(x,%f,%f)/(sqrt(2*TMath::Pi())*%f)",m*T,0.,Qbb,dE,dE),2984,3084);
  TF1 *fit_bk_only = new TF1("fbk",Form("[0]*%f",m*T),2984,3084);
  
  fit->SetParLimits(0,0,1e-3);
  fit->SetParLimits(1,-0.5e-26,1e-25);
  fit_fix->SetParLimits(0,0,2e-3);
  fit->SetParNames("b","T_{1/2}");
  fit_fix->SetParNames("b");
  fit_bk_only->SetParNames("b");
  fit_bk_only->SetParLimits(0,0,2e-3);



  // Some parameters
  //----------------------------------------------------------------------
  double G=15.92*pow(10,-15);
  double Nlow=3.90;
  double Nhigh=6.588;

  can->Print(Form("output/CUPID_sens_%s/toys_0_sig.pdf(",name.Data()),"pdf");
  GenToy(h,rand,Nb,2984,3084,0,3034.4,dE);

  double maxr=2.5e-27;

  gErrorIgnoreLevel=kFatal;

  // make the fitter                                                                                                                                                                                        //------------------------------------------------------------
  BatGraphFitter *fitter2= new BatGraphFitter(h,fit);
  BatGraphFitter *fitterbkg= new BatGraphFitter(h,fit_fix);
  BatGraphFitter *fitterbkg2= new BatGraphFitter(h,fit_bk_only);


  // CONTAINERS FOR OUTPUT
  //-----------------------------------------------------------------------------------------
  
  TLatex *tlat =new TLatex();
  std::vector<TH1D*> test_stat;
  std::vector<TH1D*> test_stat_bkg;
  std::vector<TH1D*> test_stat_0;
  std::vector<TH1D*> test_stat_fit_0;
  
  std::vector<TH1D*> p_b;
  std::vector<TH1D*> p_value;
  
  TGraphAsymmErrors* p1=new TGraphAsymmErrors();
  TGraphAsymmErrors* p2=new TGraphAsymmErrors();
  TGraph *gmed=new TGraph();




  // Loop over the toys                                                                                                                                                                                   
  //-----------------------------------------------------------------------------------------
  TH1D *ht0 = new TH1D(Form("ht0"),Form("ht0"),101000,-1,150);

  // Parameters
  int Nsignals=100;
  int Ntoys=200000;

  TGraph *gsens=new TGraph();
  TGraph *ginvsens=new TGraph();
  
  // Loop over signal strength
  //----------------------------
  for (int j=0;j<Nsignals;j++)
    {


      // some outputs
      // ht is the test stat of S+B toy fitted with fixe S
      // htb  B only toy fitted S+B
      // ht_fit_0 is a S+B toy fitted B only
      // ht0 is B only toy fitted B only
      TH1D *ht = new TH1D(Form("h%i",j),Form("h%i",j),101000,-1,150);
      TH1D *htb = new TH1D(Form("hb%i",j),Form("hb%i",j),101000,-1,150);
      TH1D *ht_fit_0 = new TH1D(Form("ht%i_fit0",j),Form("ht%i_fit0",j),101000,-1,150);
      TH1D *ht0n = new TH1D(Form("ht0n"),Form("ht0n"),101000,-1,150);

      double invT12=(j/(double)Nsignals)*maxr;
      TH1D *hp = new TH1D(Form("hp%i",j),Form("hp%i",j),10000,0,1);
      TH1D *hpb = new TH1D(Form("hpb%i",j),Form("hpb%i",j),10000,0,1);
      

      gsens->SetPoint(j,(1e-27)/invT12,0);
      ginvsens->SetPoint(j,(1e27)*invT12,0);
      // loop over the toys
      //--------------------------------
      
      for (int i=0;i<Ntoys;i++)
	{

	  bool quiet;
	  if (i%100000==0)
	    quiet=0;
	  else
	    quiet=1;
	  
	  if (!quiet)
	    {
	      
	      std::cout<<"Making fit "<<i<<" with "<<invT12<<std::endl;
	      std::cout<<"**************************************"<<std::endl;
	    }

	  

	  // generate toy
	  //----------------------------------
	  
	  double Ns,Nin;
	  Ns=scale*invT12;
	  Nin=GenToy(h,rand,Nb,2984,3084,Ns,3034.4,dE);
	  h->SetTitle(Form("toy %i T_{1/2}=%E ; Energy [keV] ; counts/0.1 keV ; ",i,invT12));
	  h->Draw("E");



	  // Fit with S+B model
	  //-----------------------------------
	  fitter2->SetTH1(h);
	  fitter2->fModel->SetNChains(2);
	  fitter2->SetPrecison(1);

	  if (!quiet)
	    {
	      std::cout<<" "<<std::endl;
	      std::cout<<"making fit to toy with "<<Form("%E",double(1/invT12))<<" with S+B model"<<std::endl;
	    }
	  fitter2->Fit(" "," ",2984,3084,0,quiet);

	  std::vector<double>modes =fitter2->fModel->GetBestFitParameters();
	  double logLL=fitter2->fModel->LogLikelihood(modes);

	  // fit with 0 signal                                                                                                                                                                                      //---------------------------------                                                                                                                                                              

          fitterbkg2->SetTH1(h);
          fitterbkg2->fModel->SetNChains(2);
          fitterbkg2->SetPrecison(1);
          if (!quiet)
            {
              std::cout<<" "<<std::endl;
              std::cout<<"making fit to toy with "<<Form("%E",1/invT12)<<" with B model"<<std::endl;
            }

          fitterbkg2->Fit(" "," ",2984,3084,0,quiet);
          std::vector<double>modes_fix2 =fitterbkg2->fModel->GetBestFitParameters();
	  double logLL_0=fitterbkg2->fModel->LogLikelihood(modes_fix2);


	  if (modes[1]<0)
	    {
	      logLL=logLL_0;
	    }


	  // Fit fixing signal
	  //-----------------------------------
	  
	  TF1 * fit2_fix = new TF1("fit2_fix",Form("[0]*%f+(%f)*TMath::Gaus(x,%f,%f)/(sqrt(2*TMath::Pi())*%f)",m*T,scale*invT12,Qbb,dE,dE),2984,3084);
	  fit2_fix->SetParLimits(0,0,1e-3);
	  fit2_fix->SetParNames("b");
	  fitterbkg->ResetTF1(fit2_fix);
	  fitterbkg->SetTH1(h);
	  fitterbkg->fModel->SetNChains(2);
          fitterbkg->SetPrecison(1);
	  if (!quiet)
            {
	      std::cout<<" "<<std::endl;
              std::cout<<"making fit to toy with "<<Form("%E",1/invT12)<<" with S fixed model"<<std::endl;
            }

          fitterbkg->Fit(" "," ",2984,3084,0,quiet);





	  // get output - save test stats
	  // -----------------------------------
	  
	  std::vector<double>modes_fix =fitterbkg->fModel->GetBestFitParameters();
	  double logLL_fix=fitterbkg->fModel->LogLikelihood(modes_fix);

	  double t =-2*(logLL_fix-logLL);






	  // Svae the t0
	  double t0=-2*(logLL_0-logLL);
	  if (!quiet)
            {
	      std::cout<<"t0 = "<<t0<<std::endl;
	      if (j>0)
		std::cout<<"pb = "<<GetIntegral(t0,150,ht0)<<std::endl;
	    }
	  
	  
	  if (!quiet)
	    {
	      std::cout<<"maxlogL = "<<logLL<<" maxlogL(S= "<<invT12<<" ) = "<<logLL_fix<<" maxlogL(S= 0 ) = "<<logLL_0<<std::endl;
	      std::cout<<"test stat (S) = "<<-2*(logLL_fix-logLL)<<std::endl;
	      std::cout<<"t0            = "<<t0<<std::endl;
			   
	      std::cout<<" "<<std::endl;
	      std::cout<<" "<<std::endl;
	      std::cout<<" "<<std::endl;
	    }

	  // Fill histograms
	  //----------------------------------
	  ht->Fill(t);
	  if (j==0)
	    ht0->Fill(t0);
	  
	  ht_fit_0->Fill(t0);

	  

	  // Get bkg only -value
	  // ---------------------------------

	  if (j!=0)
	    {
	      double p = GetIntegral(t0,150,ht0);

	      hpb->Fill(p);
	      if (fabs(p-1.4e-3)/1.4e-3 < 0.05)
		{
		  gsens->GetY()[j]+=1;
		  ginvsens->GetY()[j]+=1;
		}
					 
	    }
	  else
	    hpb->Fill(rand->Uniform());
	
	  // reset results
	  fitter2->fModel->ResetResults();
	  fitterbkg->fModel->ResetResults();
	  fitterbkg2->fModel->ResetResults();



	  //save plots
	  //-----------------------------------
	  if (!quiet)
	    {
	      h->Draw();
	      fit->SetParameters(modes[0],modes[1]);
	      fit->Draw("Csame");
	      fit2_fix->SetParameter(0,modes_fix[0]);
	      fit2_fix->SetLineColor(3);
	      fit2_fix->Draw("Csame");
	      fit_bk_only->SetParameter(0,modes_fix2[0]);
	      fit_bk_only->SetLineColor(4);
	      fit_bk_only->Draw("Csame");
	      can->Draw();
	      can->Print(Form("output/CUPID_sens_%s/toys_0_sig.pdf",name.Data()),"pdf");
	    }
	  

	  delete fit2_fix;
	}


      // save more plots
      // ------------------------------------------------------------------------------------------------------------------------
      
      ht->SetTitle(Form("Distribution of test statistic for T_{1/2}  =  %E yr ; -2log(L(S)/max(L)) ; counts",1/invT12));
      ht->Draw();
      can->Draw();
      can->Print(Form("output/CUPID_sens_%s/test_stat_%i.pdf",name.Data(),j));
      test_stat.push_back(ht);
      test_stat_bkg.push_back(htb);
      
      ht0n=(TH1D*)ht0->Clone(Form("ht0%i",j));
      test_stat_0.push_back(ht0n);
      test_stat_fit_0.push_back(ht_fit_0);

      p_value.push_back(hp);
      p_b.push_back(hpb);
     
    }
  can->Print(Form("output/CUPID_sens_%s/toys_0_sig.pdf)",name.Data()),"pdf");

  gsens->SaveAs(Form("output/CUPID_sens_%s/disc_sens.C",name.Data()));
  ginvsens->SaveAs(Form("output/CUPID_sens_%s/disc_invsens.C",name.Data()));



  
  
  // Now generate a new array of bkg only toys
  //---------------------------------------------------------------------------
  
  can->Print(Form("output/CUPID_sens_%s/toys_bkg_0_sig.pdf(",name.Data()),"pdf");

  TH1D *hinvsens = new TH1D("hinvsens","hinvsens",1000,0,40);

  TH1D *hsens = new TH1D("hsens","hsens",1000,0,40);

  // Loop over the toys
  //---------------------------

  for (int i=0;i<Ntoys;i++)
    {

      bool quiet=1;
      if (i%100000==0)
	{
	  quiet=0;
	}

      // Generate the toy
      //---------------------------------------------------------------
      
      double Ns,Nin;
      Ns=0;
      Nin=GenToy(h,rand,Nb,2984,3084,Ns,3034.4,dE);
      h->SetTitle(Form("toy %i  ; Energy [keV] ; counts/0.1 keV ; ",i));
      h->Draw("E");


      //Fit with S+B model
      // ------------------------------------------------------------
      
      fitter2->SetTH1(h);
      fitter2->fModel->SetNChains(2);
      fitter2->SetPrecison(1);
      if (!quiet)
	{
	  std::cout<<"making fit to toy with 0 signal to S+B model"<<std::endl;
	}

      fitter2->Fit(" "," ",2984,3084,0,quiet);
      std::vector<double>modes =fitter2->fModel->GetBestFitParameters();
      double logLL=fitter2->fModel->LogLikelihood(modes);
      bool foundsens=0;
      double p=0;


      // fit with S+B model fixing S
      // --------------------------------------------------------------
      for (int j=0;j<Nsignals;j++)
	{

	  double invT12=(j/(double)Nsignals)*maxr;


	  if (!quiet)
	    {
	      std::cout<<"Making fit "<<i<<" with "<<invT12<<std::endl;
	      std::cout<<"**************************************"<<std::endl;
	    }

	  // create fit function + fit
	  // ------------------------------------------------------------
	  
	  TF1 *fit3_fix = new TF1("fit3_fix",Form("[0]*%f+(%f)*TMath::Gaus(x,%f,%f)/(sqrt(2*TMath::Pi())*%f)",m*T,scale*invT12,Qbb,dE,dE),2984,3084);
          fitterbkg->ResetTF1(fit3_fix);
          fitterbkg->SetTH1(h);
          fitterbkg->fModel->SetNChains(2);
          fitterbkg->SetPrecison(1);
	  if (!quiet)
            {
              std::cout<<"making fit to toy with 0 signal to model with "<<Form("%E",1/invT12)<<std::endl;
            }

          fitterbkg->Fit(" "," ",2984,3084,0,quiet);



	  // get test stat
	  //----------------------------------------------------------------
	  
          std::vector<double>modes_fix =fitterbkg->fModel->GetBestFitParameters();
	  
	  double logLL_fix=fitterbkg->fModel->LogLikelihood(modes_fix);

	   if (j==0 &&modes[1]<0)
            {
	      logLL=logLL_fix;
	    }
				

	  double t=-2*(logLL_fix-logLL);
	  test_stat_bkg[j]->Fill(t);
	 

	  // get the p-value
	  //--------------------------------------------------------------------------
	  double sens;
	  GetLimit(sens,hsens,hinvsens,t,foundsens,test_stat[j],p,j,(double)Nsignals,maxr);
	  
	  
	 
	  p = GetIntegral(t,150,test_stat[j]); 
	  p_value[j]->Fill(p);
	 


	  if (!quiet)
	    {
	      std::cout<<" "<<std::endl;
	      std::cout<<" "<<std::endl;
	    }
	 fitter2->fModel->ResetResults();
	 fitterbkg->fModel->ResetResults();


	 

	 // make plots
	 //-----------------------------------------------------------------------
	 if (!quiet)
	   {
	     h->SetTitle(Form("toy %i T_{1/2}=%E ; Energy [keV] ; counts/0.1 keV ; ",i,invT12));
	     h->Draw();
	     fit->SetParameter(0,modes[0]);
	     fit->SetParameter(1,modes[1]);
	     fit3_fix->SetLineColor(3);
	     fit3_fix->SetParameter(0,modes_fix[0]);
	     fit3_fix->Draw("Csame");
	     fit->Draw("Csame");
	     can->Draw();
	     can->Print(Form("output/CUPID_sens_%s/toys_bkg_0_sig.pdf",name.Data()),"pdf");
	   }
	 delete fit3_fix;
	}
    }
    hsens->SaveAs(Form("output/CUPID_sens_%s/sens.C",name.Data()));
    hinvsens->SaveAs(Form("output/CUPID_sens_%s/invsens.C",name.Data()));

    TGraphErrors *gex;
    TGraphErrors *gd;
    CreatePValuePlot(p_value,test_stat,test_stat_bkg,Nsignals,0.1,"exclude",can,name,maxr,gex);
    CreatePValuePlot(p_b,test_stat_0,test_stat_fit_0,Nsignals,0.14e-2,"discover",can,name,maxr,gd);

    gex->SetTitle("Probability of exclusion ; (T_{1/2})^{-1} ; Prob  ");
    gd->SetTitle("Probability of discovery ; (T_{1/2})^{-1} ; Prob  ");
	
    gex->SaveAs(Form("output/CUPID_sens_%s/ex_prob.C",name.Data()));
    gd->SaveAs(Form("output/CUPID_sens_%s/disc_prob.C",name.Data()));




}

      
      









int BayesianLimits(TString mode,int Ntoys)
{
  // CUPID toys

  gErrorIgnoreLevel=kFatal;

  // SET THE PARAMETERS
  // ---------------------------------------------------------------------------
  
  double Qbb=3034.4;
  double eff=0.9*0.79;
  double dE = 5/2.355;
  double T=10;
  double m = 472;
  double eta=0.95;
  double W=177.7;
  double b=1e-4;
  double G=15.92*pow(10,-15);
  double Nlow=3.90;
  double Nhigh=6.588;
  double mlow=18.4;
  double mmax=50;

  
  TRandom3 *rand=new TRandom3(0);
  TString name="discovery";
  TCanvas *can = new TCanvas();
  can->Print(Form("output/CUPID_sens_%s/toys_0_sig.pdf(",name.Data()),"pdf");
  gStyle->SetOptStat(0);
  TH1D * h = new TH1D("h","h",100,2984,3084);
  double scale = T*log(2)*m*eff*eta*6.066e26/(W);

  double Nb=100*b*m*T;



  // Create the fits
  // ---------------------------------------------------------------------------
  
  TF1 *fit = new TF1("f",Form("[0]*%f+(%f*[1])*TMath::Gaus(x,%f,%f)/(sqrt(2*TMath::Pi())*%f)",m*T,scale,Qbb,dE,dE),2984,3084);
  TF1 *fit_bk_only = new TF1("f",Form("[0]*%f",m*T),2984,3084);

  fit->SetParLimits(0,0,1e-3);
  fit->SetParLimits(1,0,1e-26);
  fit_bk_only->SetParLimits(0,0,1e-3);
  fit->SetParNames("b","T_{1/2}");
  fit_bk_only->SetParNames("b");





  // Create all the objects
  //-----------------------------------------------------------------------------
  
  TH1D *hlimits= new TH1D("hlimits","hlimits",1000,0,pow(10,28));
  TH1D *hmodes= new TH1D("hmodes","hmodes",1000,0,1e-26);
  TH1D *herrors= new TH1D("herrors","herrors",1000,0,pow(10,28));

  TH1D *hNin= new TH1D("hNin","hNin",100,0,50);
  TH1D *hNout= new TH1D("hNout","hNout",100,0,50);

  TH1D *hmlow= new TH1D("hmlow","hmlow",1000,0,100);
  TH1D *hmhigh= new TH1D("hmhigh","hmhigh",1000,0,100);
  TH1D *margdistro;
  TH1D *margdistrob;
  TH1D*margdistrobonly;
  TH2D *corrdistro;
  TGraphErrors *hprofile;
  TH1D *hbkg= new TH1D("hbkg","hbkg",1000,0,5e-4);;
  TH1D *hbkg_only= new TH1D("hbkg_only","hbkg_only",1000,0,5e-4);
  
  TH1D *hprob_SB= new TH1D("hprob_SB","hprob_SB",10000,0,100);
  TH1D *hprob_B= new TH1D("hprob_B","hprob_B",10000,0,100);

  TH2D *hprob_SB_2D= new TH2D("hprob_SB_2D","hprob_SB_2D",1000,0,3e-27,2000,0,100);
  TH2D *hprob_B_2D= new TH2D("hprob_B_2D","hprob_B_2D",1000,0,3e-27,2000,0,100);
  TH2D *hprob_2D= new TH2D("hprob_2D","hprob_2D",1000,0,3e-27,2000,0,100);

  TH2D *hlimits_2D= new TH2D("hlimits_2D","hlimits_2D",1000,0,3e-27,2000,0,1e-26);
  TH2D *hmodes_2D= new TH2D("hmodes_2D","hmodes_2D",1000,0,3e-27,2000,0,1e-26);

  TH2D *ht_2D= new TH2D("ht_2D","ht_2D",1000,0,3e-27,2000,0,100);
  TH1D *ht_0= new TH1D("ht_0","ht_0",2000,0,100);

  can->Print(Form("output/CUPID_sens_%s/toys_0_sig.pdf(",name.Data()),"pdf");
  
  GenToy(h,rand,Nb,2984,3084,0,3034.4,dE);

  // make the fitter
  // -----------------------------------------------------------------------------------
  BatGraphFitter *fitter2= new BatGraphFitter(h,fit);
  BatGraphFitter *fitterbkg= new BatGraphFitter(h,fit_bk_only);


  TLatex *tlat =new TLatex();
	
  // Loop over the toys
  //-----------------------------------------------------------------------------------------
  for (int i=0;i<Ntoys;i++)
    {
      double Ns;
      double inV;
      if (i%100==0)
	{
	  std::cout<<" "<<std::endl;
	  std::cout<<" "<<std::endl;
	  std::cout<<" "<<std::endl;
	  std::cout<<"Fitting toy "<<i<<std::endl;
	  std::cout<<"-------------------------------------"<<std::endl;
	}
      double Nin;
      double Nout;

      bool zero_sig=0;
      
      // Create the toy
      if (mode=="E")
	{
	  inV=0;
	  

	  Ns=scale*inV;
	  Nin=GenToy(h,rand,Nb,2984,3084,Ns,3034.4,dE);
	}
      else
	{
	  if (i<Ntoys*0.2)
	    {
	      inV=0;
	      zero_sig=1;
	    }
	  else
	    {
	      inV=rand->Uniform(0,3e-27);
	    }
	  Ns=scale*inV;
	  GenToy(h,rand,Nb,2984,3084,Ns,3034.4,dE);
	  
	}

      


      
      if (mode=="E")
	h->SetTitle(Form("toy %i ; Energy [keV] ; counts/0.1 keV ; ",i));
      else
	h->SetTitle(Form("toy %i T_{1/2}=%f ; Energy [keV] ; counts/0.1 keV ; ",i,1/inV));

      h->Draw("E");
      bool quiet=!(i%100==0);

      fitterbkg->SetTH1(h);
      fitterbkg->fModel->SetNChains(2);
      fitterbkg->SetPrecison(2);
      fitterbkg->Fit(" "," ",2984,3084,1,quiet);


      // Get the evidence
      double evidence_B = fitterbkg->fModel->Normalize();


      fitter2->SetTH1(h);
 
      fitter2->fModel->SetNChains(2);
      fitter2->SetPrecison(2);
      fitter2->Fit(" "," ",2984,3084,1,quiet);
      double evidence_SB = fitter2->fModel->Normalize();
      std::vector<double>modes =fitter2->fModel->GetBestFitParameters();

      double logLL=fitter2->fModel->LogLikelihood(modes);


      
	    
      
      fit->SetParameters(modes[0],modes[1]);
      fit->Draw("Csame");
      tlat->DrawLatex(3000,3,Form("p(B)=%03f, p(S)=%03f, N=%f",100*evidence_B/(evidence_B+evidence_SB),100*evidence_SB/(evidence_B+evidence_SB),Nin));
      
      
      
      
      // draw the outputs
      can->Draw();
      if (i%100==0)
	{
	  can->Print(Form("output/CUPID_sens_%s/toys_0_sig.pdf",name.Data()),"pdf");
      

	}

      
      margdistro = (TH1D*)fitter2->fModel->GetMarginalizedHistogram("T_{1/2}" );
      margdistro->SetTitle(Form("Posterior on T_{1/2}^{-1} for toy %i ; T_{1/2}^{-1} ; Probability [arb. units] ; ",i));
      
      margdistro->Draw();
      
      can->Draw();


      // get the limits                                                                                                                                                                                
      double x,q;
      q=0.9;
      margdistro->GetQuantiles(1,&x,&q);

      double mode_in=margdistro->GetBinCenter(margdistro->GetMaximumBin());
      q=0.5;
      Nout=mode_in*scale;
      hNin->Fill(Nin);
      hNout->Fill(Nout);

      double med;
      margdistro->GetQuantiles(1,&med,&q);



      double mlow =T12_2_mbb(1/x,Nlow,G);
      double mhigh=T12_2_mbb(1/x,Nhigh,G);
      


      // Save some objects
      //------------------------------------------------------------------------------
      
      if (mode=="E")
	{
	  
	  corrdistro = (TH2D*)fitter2->fModel->GetMarginalizedHistogram( "b","T_{1/2}" );
	  
	  corrdistro->SaveAs(Form("output/CUPID_sens_%s/toys_%i.root",name.Data(),i));

	  corrdistro->Draw("colz");

	  can->Print(Form("output/CUPID_sens_%s/toys_0_sig.pdf",name.Data()),"pdf");
	  
	  hprofile=(TGraphErrors*)profile(corrdistro,"Y",logLL);
	  hprofile->SetName("hprofile");
	  hprofile->SaveAs(Form("output/CUPID_sens_%s/toys_profile_%i.root",name.Data(),i));
	  hprofile->Draw("APL");

	 
	  

	  
	  // save them
	  hlimits->Fill(1/x);
	  hmlow->Fill(mlow);
	  hmhigh->Fill(mhigh);
	  hmodes->Fill(mode_in);    
	  margdistrob = (TH1D*)fitter2->fModel->GetMarginalizedHistogram("b" );
	  margdistrobonly = (TH1D*)fitterbkg->fModel->GetMarginalizedHistogram("b" );
	  
	  double b_est = margdistrob->GetBinCenter(margdistrob->GetMaximumBin());
	  double bonly_est = margdistrob->GetBinCenter(margdistrobonly->GetMaximumBin());
	  
	  hbkg->Fill(b_est);
	  hbkg_only->Fill(bonly_est);

	}
      else
	{
	  hlimits_2D->Fill(inV,x);
	  hmodes_2D->Fill(inV,mode_in);
	}

      if (i%100==0)
	can->Print(Form("output/CUPID_sens_%s/toys_0_sig.pdf",name.Data()),"pdf");



      // Get the evidences
      
      if (mode=="E")
	{
	  hprob_SB->Fill(100*evidence_SB/(evidence_SB+evidence_B));
	  hprob_B->Fill(100*evidence_B/(evidence_SB+evidence_B));
	}
      else
	{
	  hprob_SB_2D->Fill(inV,100*evidence_SB/(evidence_SB+evidence_B));
	  hprob_B_2D->Fill(inV,100*evidence_B/(evidence_SB+evidence_B));

	  if (zero_sig)
	    {
	      hprob_SB->Fill(100*evidence_SB/(evidence_SB+evidence_B));

	      hprob_B->Fill(100*evidence_B/(evidence_SB+evidence_B));
	    }
	}

      fitter2->fModel->ResetResults();
      fitterbkg->fModel->ResetResults();
	  
    }
      
  


  // Make plots
  //-------------------------------------------------------------------
  
  if (mode=="E")
    {

      // make some plots
      hlimits->SetTitle("Distribution of limits for CUPID baseline ; T_{1/2} [90 % c.i.] ; Number of toys ; ");
      hlimits->Draw();

	      
      
      can->Draw();
      can->Print(Form("output/CUPID_sens_%s/toys_0_sig.pdf)",name.Data()),"pdf");
      can->Print(Form("output/CUPID_sens_%s/sensitivity.pdf",name.Data()));
      can->Print(Form("output/CUPID_sens_%s/sens.C",name.Data()));

      hmodes->SetTitle("Most probable T_{1/2}  ; T_{1/2} mode ; Number of toys ; ");
      hmodes->Draw();
      can->Draw();
      can->Print(Form("output/CUPID_sens_%s/mode.C",name.Data()));

      herrors->SetTitle("Lower error T_{1/2}  ; T_{1/2} mode ; Number of toys ; ");
      herrors->Draw();
      can->Draw();
      can->Print(Form("output/CUPID_sens_%s/err.C",name.Data()));

	    

      
      hbkg->SetTitle("Mode background index ; b [counts/keV/kg/yr] ; Probability [arb. units] ;");
      TLegend *lb = new TLegend(0.7,0.7,0.9,0.9);
      lb->AddEntry(hbkg,"Signal + background");
      hbkg_only->SetLineColor(2);
      lb->AddEntry(hbkg_only,"Background only");
      hbkg->Draw();
      hbkg_only->Draw("same");
      lb->Draw();
      
      can->Draw();
      can->Print(Form("output/CUPID_sens_%s/bkg.C",name.Data()));
      
      hmlow->SetTitle("Distribution of exclusion of m_{#beta#beta}; m_{#beta#beta}[ 90 % c.i.] ; Number of toys ; ");
      hmhigh->SetTitle("Distribution of exclusion of m_{#beta#beta} with largest NME; m_{#beta#beta}[ 90 % c.i.] ; Number of toys ; ");

      double maxi=1.2*hmhigh->GetMaximum();
      
      hmhigh->GetYaxis()->SetRangeUser(0,maxi);
	      
      TLegend * l = new TLegend(0.7,0.7,0.9,0.9);
      l->AddEntry(hmlow,"Smallest NMEs");
      l->AddEntry(hmhigh,"Largest NMEs");
      hmhigh->Draw();
      hmlow->SetLineColor(2);
      hmlow->Draw("same");
      TBox *b = new TBox(mlow,0,mmax,maxi);
      b->SetFillColorAlpha(9,0.3);
      l->AddEntry(b,"IO region");
      
      b->Draw();
      l->Draw();
      can->Draw();
      
      can->Print(Form("output/CUPID_sens_%s/mhigh.pdf",name.Data()));
      can->Print(Form("output/CUPID_sens_%s/mhigh.C",name.Data()));
		 

      hprob_SB->SetTitle("Probability of models ; Probability(model|Data) [%] ; Number of toys ; ");
      hprob_B->SetLineColor(2);

      
      TLegend *le =new TLegend(0.7,0.7,0.9,0.9);
      le->AddEntry(hprob_SB,"Signal + Background");
      le->AddEntry(hprob_B,"Background");
      
      hprob_SB->Draw();
      hprob_B->Draw("same");
      can->Draw();
      le->Draw();
      can->Draw();

      can->Print(Form("output/CUPID_sens_%s/evidences.C",name.Data()));

      hNout->SetLineColor(2);
      TLegend *lN= new TLegend(0.7,0.7,0.9,0.9);
      lN->AddEntry(hNin,"Input counts");
      lN->AddEntry(hNout,"Outout counts");

      hNin->Draw();
      hNout->Draw("same");
      lN->Draw();
      can->Draw();
      can->Print(Form("output/CUPID_sens_%s/N_in_out.C",name.Data()));



    }
  
  else
    {
      
  
      hlimits_2D->SetTitle("90 % limits on (T_{1/2})^{-1} as a function of injected signal ; Injected (T_{1/2}^{-1}) [yr^{-1}] ; T_{1/2}^{-1} 90 % [yr^{-1}]; ");
      hlimits_2D->Draw("colz");
      TF1 *fline = new TF1("fline","x",0,1e-26);
      fline->Draw("same");
      can->Draw();
      can->Print(Form("output/CUPID_sens_%s/limits.C",name.Data()));
      

      hmodes_2D->SetTitle("Mode of (T_{1/2})^{-1} as a function of injected signal ; Injected (T_{1/2})^{-1} [yr^{-1}] ; T_{1/2}^{-1} [yr^{-1}]; ");
      hmodes_2D->Draw("colz");
      //TF1 *fline = new TF1("fline","x",0,1e-26);
      fline->Draw("same");
      can->Draw();
      can->Print(Form("output/CUPID_sens_%s/modes.C",name.Data()));

      hprob_SB_2D->SetTitle("Probability of S+B model ; Injected Signal [yr^{-1}] ; Probability(model|Data) [%] ; ");
      hprob_B_2D->SetTitle("Probability of B model ; Injected Signal [yr^{-1}] ; Probability(model|Data) [%] ; ");

      hprob_SB_2D->Draw("colz");
      can->Draw();
      can->Print(Form("output/CUPID_sens_%s/SigBkg_prob.C",name.Data()));
      
      hprob_B_2D->Draw("colz");

      
      
      TF1 *fline2 = new TF1("fline2","0.27",0,1e-26);
      fline2->Draw("same");
      can->Draw();
      can->Print(Form("output/CUPID_sens_%s/Bkg_prob.C",name.Data()));
      

      hprob_B->Draw();
      can->Draw();
      can->Print(Form("output/CUPID_sens_%s/Bkg_zero_prob.C",name.Data()));

      hprob_SB->Draw();
      can->Draw();
      can->Print(Form("output/CUPID_sens_%s/SigBkg_zero_prob.C",name.Data()));

      can->Print(Form("CUPID_sens_%s/toys_0_sig.pdf)",name.Data()),"pdf");


    }
      
  return 1;
}

 int main()
 {
   //BayesianLimits("D",10000);

   FreqLimit();
 }

