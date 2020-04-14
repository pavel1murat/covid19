#!/usr/bin/python
# coding: utf-8
#------------------------------------------------------------------------------
# a.fHist[country][key] - histogram
#------------------------------------------------------------------------------
from inspect  import currentframe, getframeinfo
from datetime import *
from math     import *

from ROOT     import TGraphErrors, TGraph, TH1F, TH2F
from ROOT     import gROOT, gPad, TCanvas
from array    import array

from Covid19Data    import Covid19Data
from PopulationData import PopulationData
from HistRecord     import HistRecord

#------------------------------------------------------------------------------
class StateHistograms:

    def __init__(self):
        self.fTotalHistograms  = {};   # total histograms for the state
        self.fCountyHistograms = {}    # by county

    def hist_record(self,name):
        return self.fTotalHistograms[name];

    def list_of_counties(self):
        return self.fCountyHistograms.keys()

    def hist_record(self,county,name):
        if (county == 'total'): return self.fTotalHistograms[name];
        else:                   return self.fCountyHistograms[county][name];

#------------------------------------------------------------------------------
class CountryHistograms:

    def __init__(self):
        self.fTotalHistograms = {};   # total histograms for the country
        self.fStateHistograms = {}

    def state_histograms(self,state):
        return self.fStateHistograms[state]

    def list_of_states(self):
        return self.fStateHistograms.keys()

    def hist_record(self,state_code,name):

        items = state_code.split(':')
        state = items[0]

        county = 'total'
        if (len(items) > 1): county = items[1]

        print('<CountryHistograms::hist_record> state_code:',state_code,' state:',state,' county:',county,' name:',name)

        if (state == 'total'): 
            return self.fTotalHistograms[name];
        else:
            return self.fStateHistograms[state].hist_record(county,name);

#------------------------------------------------------------------------------
class HistData:

    def __init__(self,debug = 0):
        self.fTotalHistograms   = {};   # total histograms
        self.fCountryHistograms = {}  # by country histograms
        self.fDebug             = debug;

    def list_of_countries(self):
        return self.fCountryHistograms.keys()

    def add_country(self,country):
        if (not country in self.fCountryHistograms.keys()):
            self.fCountryHistograms.update({country:CountryHistograms()})

    def country_data(self,country):
        return self.fCountryHistograms[country]

    def hist_record(self,country_code,name):

        items      = country_code.split(':')
        country    = items[0];
        state_code = 'total'

        if   (len(items) > 2): state_code=items[1]+':'+items[2]
        else                 : state_code=items[1]

        print('<HistData::hist_record>: country_code=',country_code,' state_code:',state_code,' name:',name)

        return self.fCountryHistograms[country].hist_record(state_code,name)

    def add_histogram(self,country_code,hr):
        # print('** add histogram');

        items   = country_code.split(':');
        country = items[0]
        state   = 'total'
        county  = 'total'

        if (len(items) > 1): state   = items[1]
        if (len(items) > 2): county  = items[2]

        key = hr.fKey;

        chd =   self.fCountryHistograms[country];
        print('chd: ',chd)

        if (state == 'total') or (state == None):
            chd.fTotalHistograms.update({key:hr})
        else:
            # state is defined

            if (self.fDebug > 0):
                print('<HistData::add_histogram> >chd.list_of_states:',chd.list_of_states())

            if (not state in chd.list_of_states()):
                chd.fStateHistograms.update({state:StateHistograms()})
            
            shd = chd.fStateHistograms[state];
            print ('country,state',country,state,'shd:',shd)

            if (county == 'total') or (county == None):
                shd.fTotalHistograms.update({key:hr})
            else:
                # county is defined
                if (not county in shd.list_of_counties()):
                    shd.fCountyHistograms.update({county:{}})

                cnty_hd = shd.fCountyHistograms[county];
                cnty_hd.update({key:hr})
    
#------------------------------------------------------------------------------
class Ana:
    def __init__(self, debug = 0): 

        self.fDebug    = debug
        self.fCanvas   = None;           # TCanvas( 'c1', 'A Simple Graph with error bars', 200, 10, 700, 500 )
        self.fGraph    = None;
        self.fData     = None;           # Covid19Data
        self.fPopData  = None;           # population data
        self.fHistData = HistData(debug)
           
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
            if (i == 0):
                var = 0;
                err = 0
            else:
                rprev = data[i-1]
                var = r['totc']-rprev['totc']
                if (var < 0):
                    print ("ERROR:",r['ts'].strftime("%Y-%m-%d"),"rprev[\'totc\'], r[\'totc\']:",rprev['totc'],r['totc'])
                    err = 0
                else:
                    err = sqrt(var)
                    
        elif (key == 'newc3d'):
            if (i < 3):
                var = 0;
                err = 0
            else:
                r3 = data[i-3]
                var = (r['totc']-r3['totc'])/3.
                if (var < 0):
                    print ("ERROR:",r['ts'].strftime("%Y-%m-%d"),"r3[\'totc\'], r[\'totc\']:",r3['totc'],r['totc'])
                    err = 0
                else:
                    err = sqrt(var)
                    
        elif (key == 'newd'):
            if (i == 0): 
                var = 0;
                err = 0
            else:
                rprev = data[i-1]
                var  = r['totd']-rprev['totd']
                if (var < 0):
                    print ("ERROR:",r['ts'].strftime("%Y-%m-%d"), "rprev[\'totd\'], r[\'totd\']:",rprev['totd'],r['totd'])
                    err = 0
                else:
                    err = sqrt(var)
        elif (key == 'newd3d'):
            if (i < 3): 
                var = 0;
                err = 0
            else:
                r3 = data[i-3]
                var  = (r['totd']-r3['totd'])/3;
                if (var < 0):
                    print ("ERROR:",r['ts'].strftime("%Y-%m-%d"), "r3[\'totd\'], r[\'totd\']:",r3['totd'],r['totd'])
                    err = 0
                else:
                    err = sqrt(var)
        else                :
            var = r[key]
            err = sqrt(var)

        return (var,err)

#------------------------------------------------------------------------------
    def hist(self,country_code,name):                        # returns HistRecord
        return self.fHistData.hist_record(country_code,name)

#------------------------------------------------------------------------------
# 'country_code' : 'country:state:'
# 'name'         : optional name assigned to the histogram
#------------------------------------------------------------------------------
    def fill(self, country_code, key = 'newc', name = None):

        t  = datetime(2019,12,31,).timestamp()
        d0 = datetime.fromtimestamp(t,timezone.utc)  # this is where the histogram starts
        t0 = d0.timestamp()

        if (name == None) : name = key;

        if (self.fDebug > 0): 
            print ('<ana::fill START> country_code, key = %s %s'%(country_code,key))

        c                = country_code.split(':');
        country          = c[0];
        country_data     = self.fData.country_data(country);
        list_of_states   = None    # by default, assume fill only country totals
        list_of_counties = None    # by default, assume fill only country totals

        if (len(c) > 1):
            if (c[1] == 'all'): list_of_states = country_data.list_of_states()
            else              : list_of_states = c[1].split(',')   # assume one state
            
            if (len(c) > 2):
                list_of_counties = c[2]

        self.fHistData.add_country(country)

        cd = self.fHistData.country_data(country)

        if (self.fDebug > 0): print('<ana::fill> list_of_states =',list_of_states)

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

            self.fHistData.add_histogram(country+':total',hr);
        else:
#------------------------------------------------------------------------------
# one or several states
#------------------------------------------------------------------------------
            if (self.fDebug > 0): 
                frameinfo = getframeinfo(currentframe())
                print('<ana::fill> line:',frameinfo.lineno,' list_of_states:',list_of_states,' list_of_counties:',list_of_counties)

            if (list_of_counties == None) or (list_of_counties == 'total'):   # assume state or country totals

                for state in list_of_states:
                    ccode      = country+':'+state+':total'
                    hr         = HistRecord(ccode,key);
                    
                    hist_name  = country+'_'+state+'_'+name;
                    hist_title = country+':'+state+':'+name;

                    nb         = 100
                    hr.fHist   = TH1F(hist_name,hist_title,nb,t0,t0+nb*3600*24.)
                    hr.fHist.GetXaxis().SetTimeDisplay(1);
                    hr.fHist.SetMarkerStyle(20);

                    # x = array('i',[])

                    
                    data = country_data.fTotals   # country totals
                    if (state != 'total'): data = country_data.state_data(state).fTotals # state totals

                    nr = len(data)
                
                    if (self.fDebug>0): 
                        frameinfo = getframeinfo(currentframe())
                        print('<ana::fill> line:',frameinfo.lineno,'state='+state,' hist_name=',hist_name,' nr=',nr,)

                    for i in range(0,nr):
                        r         = data[i]
                        (var,err) = self.variable(data,i,key);

                        nd = (r['ts'].date()-d0.date()).days

                        if (self.fDebug > 0) : print ("<ana::fill> nd, var,err:",(nd,var,err)) 

                        bin = nd+1;
                        hr.fHist.SetBinContent(bin,var)
                        hr.fHist.SetBinError  (bin,err)
                    #------------------------------------------------------------------------------
                    #                   ^ histogram filled, save it and return the pointer to it
                    #------------------------------------------------------------------------------
                    ccode=country+':'+state
                    if (self.fDebug>0): 
                        frameinfo = getframeinfo(currentframe())
                        print('<ana::fill> line:',frameinfo.lineno,'ccode='+ccode)
                    self.fHistData.add_histogram(ccode,hr);
            # list of counties defined, assume one state
            else:
                #------------------------------------------------------------------------------
                # <country:state:list_of_counties> : assume one state, one or several counties
                #------------------------------------------------------------------------------
                state    = c[1]
                sdata    = country_data.state_data(state)
                counties = list_of_counties.split(',');

                print('<ana::fill> fill county hists, list_of_counties:',list_of_counties,' counties:',counties)

                for county in counties:
                    ccode      = country+':'+state+':'+county
                    print ('<ana::fill> county='+county,' ccode='+ccode,' key='+key)
                    hr         = HistRecord(ccode,key);

                    print('<ana::fill> hr = ',hr)

                    hist_title = ccode+':'+name;
                    hist_name  = hist_title.replace(':','_');

                    nb         = 100
                    hr.fHist   = TH1F(hist_name,hist_title,nb,t0,t0+nb*3600*24.)
                    hr.fHist.GetXaxis().SetTimeDisplay(1);
                    hr.fHist.SetMarkerStyle(20);
                        
                    # x = array('i',[])
                        
                    data = sdata.county_data(county)   # state totals

                    nr = len(data)
                    
                    for i in range(0,nr):
                        r         = data[i]
                        (var,err) = self.variable(data,i,key);
                                
                        nd = (r['ts'].date()-d0.date()).days
                                
                        if (self.fDebug > 0): print ("<ana::fill> nd, var,err:",(nd,var,err)) 

                        bin = nd+1;
                        hr.fHist.SetBinContent(bin,var)
                        hr.fHist.SetBinError  (bin,err)
                    #------------------------------------------------------------------------------
                    # county histogram filled, save it and return the pointer to it
                    #------------------------------------------------------------------------------
                    if (self.fDebug > 0): print('<ana::fill> ccode='+ccode);
                    self.fHistData.add_histogram(ccode,hr);

        if (self.fDebug > 0): print('<ana::fill> END')
        return hr
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
            
