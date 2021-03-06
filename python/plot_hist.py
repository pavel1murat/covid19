#!/usr/bin/python
from inspect  import currentframe, getframeinfo

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

#------------------------------------------------------------------------------
# not finished: want to plot a distribution of today's death rate for all contries
# filling the histogram has a completely different logic
# not finished yet
#------------------------------------------------------------------------------
def plot_countries_date(ana,var,last_date=None):

    for country in ana.fData.fCountryData.keys():
        ccode = country;

        t  = datetime(2019,12,31,).timestamp()
        d0 = datetime.fromtimestamp(t,timezone.utc)  # this is where the histogram starts
        t0 = d0.timestamp()


#------------------------------------------------------------------------------
# countries separated by ';'
#------------------------------------------------------------------------------
def plot_countries(ana,list_of_ccountries='US',list_of_hists='totc',*args, **kwargs):
    global c,leg;
    color = [1,2,4,6,8,9,44,46,55,67,74,88];

    start     = kwargs.get('minx'   , None)  # '2020-03-20'
    end       = kwargs.get('maxx'   , None)  # '2020-03-20'
    print_pdf = kwargs.get('pdf'    , None)  # 
    print_png = kwargs.get('png'    , None)  # 
    logy      = kwargs.get('logy'   , 0   )  # 
    miny      = kwargs.get('miny'   , None)  # 
    maxy      = kwargs.get('maxy'   , None)  # 
    opt_glob  = kwargs.get('opt'    , ''  )  # 

    ntot      = 0;
    ccountries  = list_of_ccountries.split(';');
    for ccountry in ccountries:
        print('<plot_hist::plot_countries> ccountry:'+ccountry)
        items          = ccountry.split(':')
        cntr           = items[0]
        
        if (len(items) > 1): list_of_states = items[1]
        else               : list_of_states = 'total'

        nstates        = len(list_of_states.split(','))

        if (nstates > 1): # assume state-level only histograms, no county-level ones
            ntot += nstates
        else:
            # for a single county and single state, there could be a comma-separated list of counties
            
            list_of_counties = 'total'
            if (len(items) > 2): list_of_counties = items[2]

            n = len(list_of_counties.split(','))
            ntot += n;

    # done with the loop over the countries 
    frameinfo = getframeinfo(currentframe())
    print ('<plot_hist::plot_countries> line:',frameinfo.lineno,' Ntot = ',ntot)

    hists      = list_of_hists.split(',')

    ntot       = ntot*len(hists);

    title      = ''
    c          = TCanvas("c_"+hists[0],"c",1200,800)

    leg        = TLegend(0.15,0.88-0.04*ntot,0.35,0.88);
    leg.SetLineWidth(0)

    h          = None;

    ihist      = 0;
    for ccountry in ccountries:
        items          = ccountry.split(':')
        country        = items[0]

        list_of_states = 'total'
        if (len(items) > 1): list_of_states = items[1]

        states         = list_of_states.split(',');
        nstates        = len(states)
            
        print('<plot_hist::plot_countries> line:',getframeinfo(currentframe()).lineno,
              ' country=',ccountry,'list_of_states:',list_of_states,' nstates:',nstates,' states:',states)

        if (nstates > 1): # assume no county-level histograms
            for state in states:
                for hist in hists:
                    ccode=country+':'+state
                    print('<plot_hist::plot_countries> line:',getframeinfo(currentframe()).lineno,
                          ' ccountry:'+ccountry,' hist:',hist)
                    
                    h       = ana.fill(ccode,hist);
                    if (hist == 'totc'): title = 'Total number of positive COVID-19 cases'

                    h.fHist.SetStats(0)

                    if (ihist == 0): 
                        h.fHist.SetTitle(title)
                        if (miny): h.fHist.SetMinimum(miny)
                        if (maxy): h.fHist.SetMaximum(maxy)
                        h.Draw(opt=opt_glob,dmin=start,dmax=end,col=color[ihist])
                    else: 
                        h.Draw(opt='sames',col= color[ihist])

                    text=country;
                    if (state  != 'total'): text=text+':'+state
                    # if (county != 'total'): text=text+':'+county
                    text=text+':'+hist

                    leg.AddEntry(h.fHist.GetName(),text,'pe')
                    ihist = ihist+1

                # end of loop over hists
            # end of loop over states
        else:                   # single state
            state            = states[0]
            list_of_counties = 'total'
            if (len(items) > 2): list_of_counties=items[2]

            counties  = list_of_counties.split(',');
            ncounties = len(counties);

            for county in counties:
                print ('<plot_hist::plot_countries>: county='+county)
                code=country+':'+state+':'+county
                for hist in hists:
                    print('<plot_hist::plot_countries>: filing histogram code='+code,' hist='+hist);
                    h       = ana.fill(code,hist);
                    if (hist == 'totc'): title = 'Total number of positive COVID-19 cases'

                    h.fHist.SetStats(0)

                    print('<plot_hist::plot_countries>: ihist=',ihist);
                    if (ihist == 0): 
                        h.fHist.SetTitle(title)
                        if (miny): h.fHist.SetMinimum(miny)
                        if (maxy): h.fHist.SetMaximum(maxy)
                        h.Draw(opt=opt_glob,dmin=start,dmax=end,col=color[ihist])
                    else: 
                        h.Draw(opt='sames',col= color[ihist])

                    # histogram is filled
                    text=country;
                    if (state  != 'total'): text=text+':'+state
                    if (county != 'total'): text=text+':'+county
                    text=text+':'+hist

                    leg.AddEntry(h.fHist.GetName(),text,'pe')
                    ihist = ihist+1

                # end of loop over hists
            # end of loop over counties
        # 
    # end of loop over ccountries

    # leg.Print()
    leg.Draw()

    if (logy == 1): c.SetLogy(1);
    else          : c.SetLogy(0);

    c.Modified()
    c.Update()

    if (print_pdf):
        fn = datetime.now().strftime("%Y-%m-%d")+'-'+hist+'-countries.pdf'
        c.Print(fn)

    if (print_png):
        fn = datetime.now().strftime("%Y-%m-%d")+'-'+print_png+'.png'
        c.Print(fn)
