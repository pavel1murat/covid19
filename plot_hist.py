#!/usr/bin/python
from ROOT     import TCanvas, TGraphErrors, TGraph, TH1F, TH2F, TLegend
from ROOT     import gROOT, gPad
from array    import array
from datetime import *
from math     import *

c = None;
leg = None;
#------------------------------------------------------------------------------
# variables: 'totc', 'totd', 'totr', 'ac', 
#            'tdrate' = totd/totc
# 'ana' - of 'Ana'
#------------------------------------------------------------------------------
def plot_us_states(ana,var):
    country = 'US'
    for state in ana.fData.fCountryData['US'].list_of_states():
        country_code=country+':'+state
        ana.fill(country_code,var)


def midwest(ana,var,start=None,end=None):
    global c;

    c = TCanvas("c_midwest","Midwest",1200,800)

    ana.fill('US:New_York' ,var).Draw(opt='',dmin=start,dmax=end)
    ana.hist('US:New_York' ,var).fHist.SetMinimum(0.5)

    ana.fill('US:Michigan' ,var).Draw(opt='same',col=2)
    ana.fill('US:Illinois' ,var).Draw(opt='same',col=3)
    ana.fill('US:Wisconsin',var).Draw(opt='same',col=4)
    ana.fill('US:Minnesota',var).Draw(opt='same',col=6)
    ana.fill('US:Ohio'     ,var).Draw(opt='same',col=7)
    ana.fill('US:Indiana'  ,var).Draw(opt='same',col=21)
    ana.fill('US:Missouri' ,var).Draw(opt='same',col=44)

    c.Modified()
    c.Update()


def us_recovery(ana,country):

    country_code = country

    ana.fill(country_code,'totc')
    ana.fill(country_code,'totr')

    ana.hist(country_code,'totc').Draw()
    ana.hist(country_code,'totr').Draw(opt='same',col=2)



# def plot_us_tdrate(start=None,end=None):
def plot_us_tdrate(ana):
    global c;

    c = TCanvas("c","c",1200,800)

    ana.fill('US:Washington',"tdrate").Draw(opt='')
    ana.fill("US:Illinois"  ,"tdrate").Draw(opt='sames',col= 2)
    ana.fill("US:California","tdrate").Draw(opt='sames',col= 4)
    ana.fill("US:New_York"  ,"tdrate").Draw(opt='sames',col=13)

    fn = datetime.now().strftime("%Y-%m-%d")+'-'+'-tdrate.pdf'
    c.Print(fn)
    c.Modified()
    c.Update()

#------------------------------------------------------------------------------
def plot_us_israel_russia(ana,hist='totc',start=None,end=None):
    global c;

    c = TCanvas("c","c",1200,800)

    ana.fill('US'    ,hist).Draw(opt='',dmin=start,dmax=end)
    ana.fill("Russia",hist).Draw(opt='sames',col= 2)
    ana.fill("Israel",hist).Draw(opt='sames',col= 4)

    fn = datetime.now().strftime("%Y-%m-%d")+'-us_israel_russia-'+hist+'.pdf'

    c.Print(fn)
    c.Modified()
    c.Update()

#------------------------------------------------------------------------------
def plot_us_scandinavia(ana,hist='totc',start=None,end=None):
    global c;

    c = TCanvas("c_"+hist,"c",1200,800)
    c.SetLogy(1);

    ana.fill('US'     ,hist).Draw(opt='',dmin=start,dmax=end)
    ana.fill("Sweden" ,hist).Draw(opt='sames',col= 2)
    ana.fill("Norway" ,hist).Draw(opt='sames',col= 4)
    ana.fill("Denmark",hist).Draw(opt='sames',col= 6)
    ana.fill("Finland",hist).Draw(opt='sames',col= 7)

    fn = datetime.now().strftime("%Y-%m-%d")+'-'+hist+'-us_scandinavia.pdf'
    c.Print(fn)
    c.Modified()
    c.Update()


#------------------------------------------------------------------------------
def plot_countries(ana,list_of_countries='US',list_of_hists='totc',*args, **kwargs):
    global c,leg;
    color = [1,2,4,6,8];

    start     = kwargs.get('start'  , None)  # '2020-03-20'
    end       = kwargs.get('end'    , None)  # '2020-03-20'
    print_pdf = kwargs.get('print'  , None)  # 
    logy      = kwargs.get('logy'   , 0   )  # 

    countries  = list_of_countries.split(',');
    hists      = list_of_hists.split(',')

    title      = ''

    ntot       = len(countries)*len(hists)
    c          = TCanvas("c_"+hists[0],"c",1200,800)

    leg        = TLegend(0.15,0.88-0.04*ntot,0.35,0.88);
    leg.SetLineWidth(0)

    h          = None;

    ihist      = 0;

    for country in countries:
        for hist in hists:
            h       = ana.fill(country,hist);
            if (hist == 'totc'): title = 'Total number of positive COVID-19 cases'

            h.fHist.SetStats(0)

            if (ihist == 0): 
                h.fHist.SetTitle(title)
                h.Draw(opt='',dmin=start,dmax=end,col=color[ihist])
            else: 
                h.Draw(opt='sames',col= color[ihist])

            leg.AddEntry(h.fHist.GetName(),country+':'+hist, 'pe')
            ihist = ihist+1

    leg.Print()
    leg.Draw()

    if (logy == 1): c.SetLogy(1);
    else          : c.SetLogy(0);

    c.Modified()
    c.Update()

    if (print_pdf):
        fn = datetime.now().strftime("%Y-%m-%d")+'-'+hist+'-countries.pdf'
        c.Print(fn)
