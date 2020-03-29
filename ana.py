#!/usr/bin/python
# coding: utf-8
#------------------------------------------------------------------------------
# a.fHist[country][key] - histogram
#------------------------------------------------------------------------------
from ROOT     import TCanvas, TGraphErrors, TGraph, TH1F, TH2F
from ROOT     import gROOT, gPad
from array    import array
from datetime import *
from math     import *

from Covid19Data import Covid19Data

class HistRecord:
    def __init__(self,country_code,key):
        self.fHist = None;
        self.fKey  = key
        self.fName = country_code+':'+key

#------------------------------------------------------------------------------
# Draw(opt = ',,', dmin='2020-03-01',dmax=2020-03-30',col=2,marker=23)
#------------------------------------------------------------------------------
    def Draw(self,*args, **kwargs):

        opt    = kwargs.get('opt'   , ''  )
        marker = kwargs.get('marker', None)
        col    = kwargs.get('col'   , None)
        dmin   = kwargs.get('dmin'  , None)  # '2020-03-20'
        dmax   = kwargs.get('dmax'  , None)  # 

        if (marker) : self.fHist.SetMarkerStyle(int(marker));
        if (col   ) :
            self.fHist.SetLineColor  (int(col));
            self.fHist.SetMarkerColor(int(col));

        # print('dmin,dmax=',dmin,dmax)
        
        if (dmin or dmax):   #
            # print('got here')
            xmin = self.fHist.GetXaxis().GetXmin();
            xmax = self.fHist.GetXaxis().GetXmax();
            
            if (dmax) :
                year  = int(dmax.split('-')[0]);
                month = int(dmax.split('-')[1]);
                day   = int(dmax.split('-')[2]);
                xmax  = (datetime(year,month,day,0,0,0,0)+timedelta(days=2)).timestamp()
                # print('xmax: year,month,day:',year,month,day)

            if (dmin) :
                year  = int(dmin.split('-')[0]);
                month = int(dmin.split('-')[1]);
                day   = int(dmin.split('-')[2]);
                xmin  = datetime(year,month,day,0,0,0,0).timestamp()
                # print('xmin: year,month,day:',year,month,day)
                
            
            self.fHist.GetXaxis().SetRangeUser(xmin,xmax);

        self.fHist.Draw(opt);

#------------------------------------------------------------------------------
class StateHistograms:

    def __init__(self):
        self.fTotalHistograms  = {};   # total histograms for the state
        self.fCountyHistograms = {}    # by county

    def hist_record(self,name):
        return self.fTotalHistograms[name];

#------------------------------------------------------------------------------
class CountryHistograms:

    def __init__(self):
        self.fTotalHistograms = {};   # total histograms for the country
        self.fStateHistograms = {}

    def state_histograms(self,state):
        return self.fStateHistograms[state]

    def list_of_states(self):
        return self.fStateHistograms.keys()

    def hist_record(self,state,name):
        if (state == 'total'):
            return self.fTotalHistograms[name];
        else:
            return self.fStateHistograms[state].hist_record(name);

#------------------------------------------------------------------------------
class HistData:

    def __init__(self):
        self.fTotalHistograms   = {};   # total histograms
        self.fCountryHistograms = {}  # by country histograms

    def list_of_countries(self):
        return self.fCountryHistograms.keys()

    def add_country(self,country):
        if (not country in self.fCountryHistograms.keys()):
            self.fCountryHistograms.update({country:CountryHistograms()})

    def country_data(self,country):
        return self.fCountryHistograms[country]

    def hist_record(self,country,state,name):
        return self.fCountryHistograms[country].hist_record(state,name)

    def add_histogram(self,country,state,hr):
        print('** add histogram');

        key = hr.fKey;

        chd =   self.fCountryHistograms[country];
        print('chd: ',chd)
        if (state == 'total'):
            chd.fTotalHistograms.update({key:hr})
        else:
            print('chd.list_of_states:',chd.list_of_states())
            if (not state in chd.list_of_states()):
                chd.fStateHistograms.update({state:StateHistograms()})
            
            shd = chd.fStateHistograms[state];
            print ('country,state',country,state,'shd:',shd)
            
            shd.fTotalHistograms.update({key:hr})
    
#------------------------------------------------------------------------------
class Ana:
    def __init__(self, debug = 0): 

        self.fCanvas   = None; # TCanvas( 'c1', 'A Simple Graph with error bars', 200, 10, 700, 500 )
        self.fGraph    = None
        self.fHistData = HistData()
        self.fDebug    = debug
           
#------------------------------------------------------------------------------
    def variable(self,data,i,key):
        var = None;
        err = None;

        r = data[i]
        
        if (key == 'tdrate'):
            if (r['totc'] == 0):
                var = 0
                err = 0
            else:
                var = r['totd']/r['totc'];
                if (r['totd'] == 0): err = 0
                else               : err = var*sqrt(1./r['totd']+1./r['totc'])
        elif (key == 'newc'):
            if (i == len(data)-1):
                var = 0;
                err = 0
            else:
                rnext = data[i+1]
                var = rnext['totc']-r['totc']
                if (var < 0):
                    print ("ERROR:",r['ts'].strftime("%Y-%m-%d"),"rnext[\'totc\'], r[\'totc\']:",rnext['totc'],r['totc'])
                    err = 0
                else:
                    err = sqrt(var)
                    
        elif (key == 'newd'):
            if (i == len(data)-1): 
                var = 0;
                err = 0
            else:
                rnext = data[i+1]
                var  = rnext['totd']-r['totd']
                if (var < 0):
                    print ("ERROR:",r['ts'].strftime("%Y-%m-%d"), "next[\'totd\'], r[\'totd\']:",rnext['totd'],r['totd'])
                    err = 0
                else:
                    err = sqrt(var)
        else                :
            var = r[key]
            err = sqrt(var)

        return (var,err)

#------------------------------------------------------------------------------
    def hist(self,country_code,name):  # returns HistRecord
           
        c = country_code.split(':')
        country = c[0];
        if (len(c) == 1): state = 'total'
        else            : state = c[1]

        return self.fHistData.hist_record(country,state,name)

#------------------------------------------------------------------------------
# 'country_code' : 'country:state:'
# 'name'         : optional name assigned to the histogram
#------------------------------------------------------------------------------
    def fill(self, country_code, key = 'newc', name = None):

        t  = datetime(2019,12,31,).timestamp()
        d0 = datetime.fromtimestamp(t,timezone.utc)  # this is where the histogram starts
        t0 = d0.timestamp()

        if (name == None) : name = key;

        if (self.fDebug > 0): print ('<ana::fill> country_code, key = %s %s'%(country_code,key))

        c              = country_code.split(':');
        country        = c[0];
        country_data   = self.fData.country_data(country);
        list_of_states = None    # by default, assume fill only country totals

        if (len(c) > 1):
            if (c[1] == 'all'): list_of_states = country_data.list_of_states()
            else              : list_of_states = [c[1]]; # assume one state
            
        self.fHistData.add_country(country)

        cd = self.fHistData.country_data(country)

        if (list_of_states == None):     # fill only country totals

            state = 'total'
            hr = HistRecord(country_code,key);
            hist_name  = country+'_'+state+'_'+name;
            hist_title = country+':'+state+':'+name;

            nb = 100
            hr.fHist = TH1F(hist_name,hist_title,nb,t0,t0+nb*3600*24.)
            hr.fHist.GetXaxis().SetTimeDisplay(1);
            hr.fHist.SetMarkerStyle(20);
            
            data = country_data.fTotals
            nr = len(data)
            for i in range(0,nr):
                r = data[i]
                
                (var,err) = self.variable(data,i,key);

                nd = (r['ts'].date()-d0.date()).days

                if (self.fDebug > 0) : print ("nd, var,err:",(nd,var,err)) 

                bin = nd+1;
                hr.fHist.SetBinContent(bin,var)
                hr.fHist.SetBinError  (bin,err)
                
            self.fHistData.add_histogram(country,'total',hr);
        else:
            
            for state in list_of_states:
                hr         = HistRecord(country_code,key);
        
                hist_name  = country+'_'+state+'_'+name;
                hist_title = country+':'+state+':'+name;

                nb = 100
                hr.fHist = TH1F(hist_name,hist_title,nb,t0,t0+nb*3600*24.)
                hr.fHist.GetXaxis().SetTimeDisplay(1);
                hr.fHist.SetMarkerStyle(20);

                # x = array('i',[])

                state_totals = country_data.state_data(state).fTotals   # state totals

                nr = len(state_totals)
                
                for i in range(0,nr):
                    r         = state_totals[i]
                    (var,err) = self.variable(state_totals,i,key);

                    nd = (r['ts'].date()-d0.date()).days

                    if (self.fDebug > 0) : print ("nd, var,err:",(nd,var,err)) 

                    bin = nd+1;
                    hr.fHist.SetBinContent(bin,var)
                    hr.fHist.SetBinError  (bin,err)
#------------------------------------------------------------------------------
#           ^ histogram filled, save it
#------------------------------------------------------------------------------
                self.fHistData.add_histogram(country,state,hr);

#------------------------------------------------------------------------------
# end of the class definition
#------------------------------------------------------------------------------
if (__name__ == '__main__'):
    dir = '/projects/covid19/data/CSSEGISandData/csse_covid_19_data/csse_covid_19_daily_reports'
    data = Covid19Data.Covid19Data(dir);

    a       = Ana();
    a.fData = data;

    hist_names    = ['totc','totd','tdrate']

    #        countries = self.fData.list_of_countries()
    countries = ['US']

    for country in countries:
        for hist in hist_names:
            self.fill(country,hist)
            
