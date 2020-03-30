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

#------------------------------------------------------------------------------
class StateData:
    def __init__(self):
        self.fTotals     = []   # array of totals, by day
        self.fCountyData = {}   # list of county data, by county
        
    def add_record(self,r):
        county = r['county']
        if (not county in self.fCountyData.keys()):
            self.fCountyData.update({county:[]})     # this is a list of 
                    
            self.fCountyData[county].append(r);      # all dates match the date in the file name

    def list_of_counties(self):
        return self.fCountyData.keys();

    def county_data(self,county):
        return self.fCountyData[county]

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

    def state_data(self,state):
        return self.fStateData[state]

    def add_state(self,state):
        self.fStateData.update({state:StateData()})


#------------------------------------------------------------------------------
# version = 1: data scraped by myself
# version = 2: JHU data
#
# default directory for version=2:
# /projects/covid19/data/CSSEGISandData/csse_covid_19_data/csse_covid_19_daily_reports
#------------------------------------------------------------------------------
class Covid19Data:

    def __init__(self,dir='/projects/covid19/data/world/txt', debug = 0):
        self.fCountryData = {}
        self.fDir   = dir
        self.fDebug = debug

        self.read(dir)  # migrate to JHU data

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
               r['totc'],r['totd'],r['totr'],r['ac']), end='')
        
        if ('serc' in r.keys()):
            print(" %6i %6i"%(r['serc'],r['cpm']), end='')

        print();
        
#------------------------------------------------------------------------------
    def print(self,country = None):
        # print ROOT format string
        # print('rid/I:time/I:date/C:country/C:state/C:county/C:totc/I:newc/I:totd/I:newd/I:totr/I:ac/I:serc/I:cpm/I');
        print ('Covid19Data::print : BEGIN')
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

        print ('Covid19Data::print : END')

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
#        print('unified_key START: key=',key,' val=',val)
        new_val = val
        
        if   (key == 'Province/State'):
            k = 'state'
        elif (key == 'Province_State'):
            k = 'state'
        elif (key == 'Country/Region'):
            k = 'country'
        elif (key == 'Country_Region'):
            k = 'country'
        elif (key == 'Confirmed'     ):
            k = 'totc'
            if (val == ''): new_val = 0
        elif (key == 'Deaths'        ):
            k = 'totd'
            if (val == ''): new_val = 0
        elif (key == 'Recovered'     ):
            k = 'totr'
            if (val == ''): new_val = 0
        elif (key == 'Last Update') or (key == 'Last_Update'):
            k = 'ts'
        elif (key == 'FIPS'          ):
            k = 'fips'
            if (val == ''): new_val = -1
        elif (key == 'Admin2'        ):
            k = 'county'
        elif (key == 'Lat') or (key == 'Latitude'):
            k = 'lat'
            if (val == ''): new_val = -1.e6
        elif (key == 'Long_') or (key == 'Longitude'):
            k = 'long'
            if (val == ''): new_val = -1.e6
        elif (key == 'Combined_Key'  ):
            k = 'ckey'
        elif (key == 'Active'        ):
            k = 'ac'
            if (val == ''): new_val = 0
        else:
                print("unified_key ERROR: unknown key: ",key);

#        print('unified_key END  : key=',k,' val=',new_val)

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
        elif (key == 'fips'   ) : r[key] = int(val)
        elif (key == 'lat'    ) : r[key] = float(val)
        elif (key == 'long'   ) : r[key] = float(val)
        elif (key == 'totc'   ) : r[key] = int(val)
        elif (key == 'newc'   ) : r[key] = int(val)
        elif (key == 'totd'   ) : r[key] = int(val)
        elif (key == 'newd'   ) : r[key] = int(val)
        elif (key == 'totr'   ) : r[key] = int(val)
        elif (key == 'ac'     ) : r[key] = int(val)
        elif (key == 'serc'   ) : r[key] = int(val)
        elif (key == 'cpm'    ) : r[key] = int(val)
        elif (key == 'ts'     ) :                 # handle different time formats
            format = None
            f1     = None;
            f2     = None
            sep    = ' '
            if   (' ' in val): sep = ' '
            elif ('T' in val): sep = 'T'

            w = val.split(sep);
                            
            if ('/' in w[0]): f1='%m/%d/%Y'
            else            : f1='%Y-%m-%d'

            if (len(w[1].split(':')) == 3): f2='%H:%M:%S'
            else                          : f2='%H:%M'

            format = f1+sep+f2

            r['ts']  = datetime.strptime(val,format);
            
            r['uts'] = r['ts'].timestamp()
                            
        elif (key == 'ckey'   ) :
            # do nothing
            x=0
        else:
            err = 1
            print("Covid19Data::unified_key ERROR: unknown key \'%s\' len=%i"%(key,len(key)));

            # if (self.fDebug > 0): print('i=',i,'key=',key,'k=',k,'val=',val);

#------------------------------------------------------------------------------
# 'data' contains only one day data corresponding to 'date' (which is of datetime type)
#------------------------------------------------------------------------------
    def calculate_totals(self,date):
        # print(" CalculateTotals: start")
        for country in self.list_of_countries():
            # print('------- country = '+country)
            crd = self.country_data(country)
            for state in crd.list_of_states():
                # print('------- state='+state)
                sd             = crd.fStateData[state]

                str            = {}
                str['ts']      = date                    # datetime
                str['uts']     = date.timestamp()        # timestamp
                str['country'] = country
                str['state'  ] = state
                str['county' ] = 'total'
                        
                totc = 0;
                totd = 0;
                totr = 0;
                ac   = 0
                    
                for county in sd.list_of_counties():
                    # print('-- county='+county)
                    ctd = sd.county_data(county)                      # this is a 'by-county' array
                                                          # of today's data records
                    for r in ctd:
                        totc = totc+r['totc']
                        totd = totd+r['totd']
                        totr = totr+r['totr']
                        if ('ac' in r.keys()) : ac   = ac  +r['ac'  ]
                        #
                    str['totc'] = totc
                    str['totd'] = totd
                    str['totr'] = totr
                    str['ac'  ] = ac
                    #                   loop over the counties done, store state totals for today
                sd.fTotals.append(str)
                    
                #                for r in sd.fTotals:   # there should be only one record
                #                    self.print_record(r);
                        
            # at this point, totals for each state are calculated, update totals for the country
            
            ctr            = {}
            ctr['ts'     ] = date
            ctr['uts'    ] = date.timestamp()
            ctr['country'] = country
            ctr['state'  ] = 'total'
            ctr['county' ] = 'total'

            totc = 0;
            totd = 0;
            totr = 0;
            ac   = 0
            
#            print('calculate totals for the country:')
            
            for state in crd.list_of_states():
                r  = crd.state_data(state).fTotals[0]            ; # this is the state's 'total' record

#                print ('-- state, totc:',state,r[0]['totc'])

                totc = totc+r['totc']
                totd = totd+r['totd']
                totr = totr+r['totr']
                if ('ac' in r.keys()): ac   = ac  +r['ac'  ]

            ctr['totc'] = totc
            ctr['totd'] = totd
            ctr['totr'] = totr
            ctr['ac'  ] = ac

            crd.fTotals.append(ctr)

            #            for r in crd.fTotals: self.print_record(r)  # there should be only one record
                        
        return
    
#------------------------------------------------------------------------------
    def add_todays_totals(self,todays_data):
        
        for country in todays_data.list_of_countries():
            if (not country in self.list_of_countries()): self.add_country(country)
                
            crd1 = self.country_data(country)
            crd0 = todays_data.country_data(country)

            crd1.fTotals.append(crd0.fTotals[0]);
            
            for state in crd0.list_of_states():
                if (not state in crd1.list_of_states()): crd1.add_state(state)

                sd1 = crd1.state_data(state)
                sd0 = crd0.state_data(state)

                sd1.fTotals.append(sd0.fTotals[0]);
                
                for county in sd0.list_of_counties():
                    if (not county in sd1.list_of_counties()): sd1.add_county(county)

                    ctd1 = sd1.county_data(county)
                    r    = sd0.county_data(county)[0]          # this is a single data record for today
                    ctd1.append(r)

#------------------------------------------------------------------------------
    def read(self,dir):

        if (self.fDebug > 0) : print("<Covid19Data::read> dir = %s",dir);
        
        nlines = 0;
        data   = {}                  # holder of all data
        for fn in sorted(pathlib.Path(dir).glob('*.csv')):
#------------------------------------------------------------------------------
#       ^ read new file and calculate totals. All data in a single file are assumed
#         to have the same timestamp
#------------------------------------------------------------------------------
            if (self.fDebug > 0): print('---------',fn);

            fb    = os.path.basename(fn).split('.')[0];
            fdate = datetime.strptime(fb,'%m-%d-%Y')    # date corresponding to the .csv file

            f     = open(fn,'r')
            lines = f.readlines()
            nl    = len(lines);
#------------------------------------------------------------------------------
# first line - comma-separated keys
# data in each file correspond to the same day, use that to calculate totals
#------------------------------------------------------------------------------
            keys  = lines[0].split(',')
            nk = len(keys)
            for i in range(0,nk):
                keys[i] = keys[i].strip()
                
            # print("    keys:",keys);     
            
            if (self.fDebug > 10):
                print("    nl:",nl);
                print("    lines[0]:",lines[0]);
                print("    keys:",keys);

            todays_data = Covid19Data();
            
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
                    print ('ERROR: fn=',os.path.basename(fn),':',dr.strftime('%Y-%m-%d'),':',lines[iline].strip(), 'FIXING DATE')
                    if (not 'county' in r.keys()):
                        r['county'] = 'total'
                        
                    if (self.fDebug > 10): print(r)
                    r['ts']  = fdate
                    r['uts'] = fdate.timestamp();

                    todays_data.add_record(r);
#------------------------------------------------------------------------------
#           ^ end of file processing
#------------------------------------------------------------------------------
            f.close()
#------------------------------------------------------------------------------
# calculate totals, first for the states
#------------------------------------------------------------------------------
            todays_data.calculate_totals(fdate);
#------------------------------------------------------------------------------
#       ^ end of loop over the files, add today's data to the global data
#------------------------------------------------------------------------------
            self.add_todays_totals(todays_data)
#------------------------------------------------------------------------------
# the end
#------------------------------------------------------------------------------


#------------------------------------------------------------------------------
# end of the class definition
#------------------------------------------------------------------------------
if (__name__ == '__main__'):
    dir = '/projects/covid19/data/CSSEGISandData/csse_covid_19_data/csse_covid_19_daily_reports'
    # dir = 'test'
    data = Covid19Data(dir);
    data.print()
