#!/usr/bin/python
# coding: utf-8
#------------------------------------------------------------------------------
# for some reason, needs Python 3
# effectively, this class is intended to reformat the data into a format
# to initialize a ROOT tree
# except sorting, I'm not using anything specific for Python, all could be done in Ruby
#------------------------------------------------------------------------------
import os, pathlib, string
from datetime import *
from operator import itemgetter
from inspect  import currentframe, getframeinfo

from ROOT       import gROOT, gPad, TCanvas, TGraph, TH1F, TH2F

from HistRecord import HistRecord;
#------------------------------------------------------------------------------
class StateData:
    def __init__(self):
        self.fTotals     = []   # array of totals, by day
        self.fCountyData = {}   # list of county data, by county

    #------------------------------------------------------------------------------
    def add_record(self,r):
        county = r['county']
        if (not county in self.fCountyData.keys()):
            self.fCountyData.update({county:[]})     # this is a list of 
                    
            self.fCountyData[county].append(r);      # all dates match the date in the file name

    #------------------------------------------------------------------------------
    def list_of_counties(self):
        return self.fCountyData.keys();

    #------------------------------------------------------------------------------
    def county_data(self,county='total'):
        if (county == ''): county = 'total'

        if (county == 'total'): return fTotals;
        else                  : return self.fCountyData[county]

    #------------------------------------------------------------------------------
    def add_county(self,county):
        self.fCountyData.update({county:[]})

#------------------------------------------------------------------------------
class CountryData:
    def __init__(self):
        self.fTotals    = []   # array of totals, by day
        self.fStateData = {}   # list of state data, by state

    def add_record(self,r):
        state = r['state']

        if (state == 'total'):
            self.fTotals.append(r)
        else:
            if (not state in self.fStateData.keys()): self.fStateData.update({state:StateData()})
            self.fStateData[state].add_record(r)

    def list_of_states(self):
        return self.fStateData.keys()

    def state_data(self,state='total'):
        if (state==''): state = 'total'

        if (state == 'total'): return self.fTotals;
        else                 : return self.fStateData[state]

    def add_state(self,state):
        self.fStateData.update({state:StateData()})


#------------------------------------------------------------------------------
# version = 1: data scraped from https://www.worldometers.info/world-population/population-by-country/
#------------------------------------------------------------------------------
class PopulationData:

    def __init__(self,input_file='/projects/covid19/data/population/txt/2020-04-10T12:07:50-05:00_world_total.txt', debug = 0):
        self.fCountryData = {}
        self.fDebug       = debug
        self.read(input_file)

    def list_of_countries(self):
        return self.fCountryData.keys();

    def country_data(self,country):
        return self.fCountryData[country]

    def add_country(self,country):
        self.fCountryData.update({country:CountryData()})

#------------------------------------------------------------------------------
    def add_record(self,r):

        country = r['country']
        if (not country in self.fCountryData.keys()):
            self.fCountryData.update({country:CountryData()})
            
        self.fCountryData[country].add_record(r);

#------------------------------------------------------------------------------
    def print_record(self,r):
        print("%s %i %-25s %20s %20s %6i %6i %6i %6i" %
              (r['ts'].strftime('%Y-%m-%dT%H:%M:%S'),r['uts'],
               r['country'],r['state'], r['county'],
               r['popm'],r['nkm2'],r['skm2'],r['agem'],r['furb']), end='')
        
        print();
        
#------------------------------------------------------------------------------
    def print(self,country = None):
        # print ROOT format string
        print ('<PopulationData::print> : BEGIN')
        for country in self.list_of_countries():
            country_data = self.fCountryData[country];
            print("------------------------------------------- country,ntotals : ",country,len(country_data.fTotals))
            
            for r in country_data.fTotals:
                self.print_record(r);
            
            for state in country_data.list_of_states():
                sd = country_data.state_data(state)
                print("----------------------- state, ntotals : ",state,len(sd.fTotals))

                for r in sd.fTotals:
                    self.print_record(r);
                    
                for county in sd.list_of_counties():
                    # print("-------- county : ",county)
                    county_data = sd.county_data(county)
                    for r in county_data:
                        # print('record:',r)
                        # print a single data record
                        self.print_record(r)

        print ('PopulationData::print : END')

#------------------------------------------------------------------------------
# read JHU data
# some names are enclosed in double quotes and contain spaces and commas - that
# a) sucks
# b) is time-dependent
#------------------------------------------------------------------------------
    def handle_spaces_and_commas_in_names(self,line):
        newline = line
        if ('"' in line) : 
#------------------------------------------------------------------------------
# there are double quote characters on the line
#------------------------------------------------------------------------------
            c1    = []
            inside = 0
            for char in newline:
                c = char
                if (c == '"'):
                    inside = 1-inside                   # skip the character
                else:
                    if (inside == 1):
                        if (c == ' '): c = '_'          # replace spaces with underscores
                if (inside == 0):
                    c1.append(c)
                else :
                    if (c != ','): c1.append(c)         # skip commas in names
  
            newline = "".join(c1)
#            print ('handle_spaces_and_commas: line',line)
#            print ('handle_spaces_and_commas: c1',c1)
#            print ('handle_spaces_and_commas: newline',newline)
        return newline

#------------------------------------------------------------------------------
    def unified_key(self,key,val):
        if (self.fDebug > 0):
            print('<PopulationData::unified_key> START: key=',key,' val=',val)

        new_val = val
        k       = key
        return (k,new_val)

#------------------------------------------------------------------------------
    def update_record(self,key,val,r):
#        print('update_record: key=',key,'val=',val)
        
        if   (key == 'rid'    ) : r[key] = int(val)
        elif (key == 'country') : 
#------------------------------------------------------------------------------
# correct/unify country name spelling spelling
#------------------------------------------------------------------------------
            if   (val == 'U.K.'                  ) : val = 'UK'
            elif (val == 'United Kingdom'        ) : val = 'UK'
            elif (val == 'U.A.E.'                ) : val = 'UAE'
            elif (val == 'United Arab Emirates'  ) : val = 'UAE'
            elif (val == 'United States'         ) : val = 'US'
            elif (val == 'S._Korea'              ) : val = 'South_Korea'
            elif (val == 'St._Barth'             ) : val = 'St_Barth'
            elif (val == 'St._Vincent_Grenadines') : val = 'St_Vincent_Grenadines'
            elif (val == 'Korea, South'          ) : val = 'South_Korea'
            
            r[key] = val
        elif (key == 'state'  ) :
            if   (val == ''                      ) : val = 'total'
            elif (val == 'United Kingdom'        ) : val = 'UK'
            r[key] = val.replace(' ','_')
        elif (key == 'county' ) : r[key] = val
        elif (key == 'popm'   ) : r[key] = float(val)
        elif (key == 'nkm2'   ) : r[key] = float(val)
        elif (key == 'skm2'   ) : r[key] = float(val)
        elif (key == 'agem'   ) : r[key] = float(val)
        elif (key == 'furb'   ) : r[key] = float(val)
        elif (key == 'ts'     ) :                 # handle different time formats
            format = None
            f1     = None;
            f2     = None
            sep    = ' '
            if   (' ' in val): sep = ' '
            elif ('T' in val): sep = 'T'

            w = val.split(sep);
                            
            if ('/' in w[0]): 
                if (len(w[0].split('/')[2]) == 2): f1='%m/%d/%y'
                else                             : f1='%m/%d/%Y'
            else            : f1='%Y-%m-%d'

            if (len(w[1].split(':')) == 3): f2='%H:%M:%S'
            if (len(w[1].split(':')) == 4): f2='%H:%M:%S%z'
            else                          : f2='%H:%M'

            format = f1+sep+f2

            r['ts']  = datetime.strptime(val,format);
            
            r['uts'] = r['ts'].timestamp()
                            
        else:
            err = 1
            print("PopulationData::unified_key ERROR: unknown key \'%s\' len=%i"%(key,len(key)));

        if (self.fDebug > 0): 
            print('<PopulationData::update_record> key=',key,' val=',val);

#------------------------------------------------------------------------------
# read population data
#------------------------------------------------------------------------------
    def read(self,input_file):

        if (self.fDebug > 0) : print("<PopulationData::read> input_file = %s",input_file);
        
        nlines = 0;

        if (self.fDebug > 0): print('---------',input_file);

        fb    = os.path.basename(input_file).split('_world_total.')[0];
        fdate = datetime.strptime(fb,'%Y-%m-%dT%H:%M:%S%z')    # date corresponding to the .csv file

        f     = open(input_file,'r')
        lines = f.readlines()
        nl    = len(lines);
        f.close()
#------------------------------------------------------------------------------
# first line - comma-separated keys
# data in each file correspond to the same day, use that to calculate totals
#------------------------------------------------------------------------------
        keys  = lines[0].split(',')
        nk = len(keys)
        for i in range(0,nk):
            keys[i] = keys[i].strip()
                
            if (self.fDebug > 0): print("<PopulationData::keys>",keys);     
            
        if (self.fDebug > 10):
            print("    nl:",nl);
            print("    lines[0]:",lines[0]);
            print("    keys:",keys);

        for iline in range(1,nl-1):
#------------------------------------------------------------------------------
# deal with spaces and commas in names
#------------------------------------------------------------------------------
            line    = self.handle_spaces_and_commas_in_names(lines[iline].strip())
            if (self.fDebug > 10):
                print("i:%3i line:%s"%(iline,lines[iline]))
                print('line:'+line)

            words = line.strip().split(',')

            r   = {}                       # 'r' - new data record
            err = 0
            nw  = len(words)
            for i in range(0,nw) :
                key = "".join(filter(lambda x: x in string.printable, keys[i].strip()))
                val = words[i].strip()

                (k,v) = self.unified_key(key,val);
                self.update_record(k,v,r) 
#------------------------------------------------------------------------------
# ignore the time, use the date only
# make sure that the date of the record corresponds the date in the .csv file name
#------------------------------------------------------------------------------
            dr = datetime.fromtimestamp(r['uts'])

            if (dr.date() == fdate.date()):
                if (not 'county' in r.keys()):
                    r['county'] = 'total'
                        
                if (self.fDebug > 10): print(r)
                    
                todays_data.add_record(r);
            else:
                # print ('ERROR: fn=',os.path.basename(input_file),':',dr.strftime('%Y-%m-%d'),':',lines[iline].strip(), 'FIXING DATE')
                if (not 'county' in r.keys()):
                    r['county'] = 'total'
                        
                if (self.fDebug > 10): print(r)
                r['ts']  = fdate
                r['uts'] = fdate.timestamp();

            country = r['country']
            if (not country in self.fCountryData.keys()):
                self.fCountryData.update({country:CountryData()})

            self.fCountryData[country].add_record(r);
#------------------------------------------------------------------------------
# the end
#------------------------------------------------------------------------------


#------------------------------------------------------------------------------
# end of the class definition
# so far, read only per-country data
#------------------------------------------------------------------------------
if (__name__ == '__main__'):
    input_file = '/projects/covid19/data/population/txt/2020-04-10T12:07:50-05:00_world_total.txt'
    # dir = 'test'
    data = PopulationData(input_file);
    data.print()
