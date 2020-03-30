#!/usr/bin/python
from ROOT     import TCanvas, TGraphErrors, TGraph, TH1F, TH2F
from ROOT     import gROOT, gPad
from array    import array
from datetime import *
from math     import *

c = None;
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


def draw_midwest(ana,var):

    ana.hist('US:New_York',var).Draw(opt='')
    ana.hist('US:New_York',var).fHist.SetMinimum(0.5)

    ana.hist('US:Michigan',var).Draw(opt='same',col=2)
    ana.hist('US:Illinois',var).Draw(opt='same',col=3)
    ana.hist('US:Wisconsin',var).Draw(opt='same',col=4)
    ana.hist('US:Minnesota',var).Draw(opt='same',col=6)
    ana.hist('US:Ohio',var).Draw(opt='same',col=7)
    ana.hist('US:Indiana',var).Draw(opt='same',col=21)
    ana.hist('US:Missouri',var).Draw(opt='same',col=44)


def plot_us_recovery(ana,country):

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
def plot_countries(ana,countries='US',hist='totc',start=None,end=None):
    global c;

    c = TCanvas("c_"+hist,"c",1200,800)
    c.SetLogy(1);

    list=countries.split(',');
    for i in range(0,len(list)):
        country = list[i]
        if (i == 0): ana.fill(country,hist).Draw(opt='',dmin=start,dmax=end)
        else       : ana.fill(country,hist).Draw(opt='sames',col= i+1)

    fn = datetime.now().strftime("%Y-%m-%d")+'-'+hist+'-countries.pdf'
    c.Print(fn)
    c.Modified()
    c.Update()

#------------------------------------------------------------------------------
def plot_country_hists(ana,country='US',histograms='totc,totd',start=None,end=None):
    global c;

    c = TCanvas("c_"+country,"c",1200,800)
    c.SetLogy(1);

    list_of_hists= histograms.split(',');
    for i in range(0,len(list_of_hists)):
        hist = list_of_hists[i]
        if (i == 0): ana.fill(country,hist).Draw(opt='',dmin=start,dmax=end)
        else       : ana.fill(country,hist).Draw(opt='sames',col= i+1)

    fn = datetime.now().strftime("%Y-%m-%d")+'-'+country+'-hists.pdf'
    c.Print(fn)
    c.Modified()
    c.Update()
