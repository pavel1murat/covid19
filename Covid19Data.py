#!/usr/bin/python
# coding: utf-8
#------------------------------------------------------------------------------
# for some reason, needs Python 3
# effectively, this class is intended to reformat the data into a format
# to initialize a ROOT tree
# except sorting, I'm not using anything specific for Python, all could be done in Ruby
#------------------------------------------------------------------------------
import os
from operator import itemgetter

class Covid19Data:
#------------------------------------------------------------------------------
    def __init__(self,dir='/projects/covid19/data/world/txt',debug = None):
        self.fData  = {}
        self.fDir   = dir
        self.fDebug = debug

        self.read(dir)

#------------------------------------------------------------------------------
    def print(self,country = None):
        # print ROOT format string
        print('rid/I:time/I:date/C:country/C:state/C:county/C:totc/I:newc/I:totd/I:newd/I:totr/I:ac/I:serc/I:cpm/I');

        for c in self.fData.keys():
            if ( not country) or (c == country):
                for state in self.fData[country].keys():
                    state_data = self.fData[country][state]          # this one is already an array
                    for data in state_data:
                        print("%2i %i %s %-25s %s %s %6i %6i %6i %6i %6i %6i %6i %6i"
                              %(data['rid'],data['uts'],data['ts'],
                                data['country'],data['state'], data['county'],
                                data['totc'],data['newc'],data['totd'],data['newd'],
                                data['totr'],data['ac'],data['serc'],data['cpm']))

#------------------------------------------------------------------------------
    def read(self,dir):

        if (self.fDebug) : print("<Covid19Data::read> dir = %s",dir);
        
        nlines = 0;
        data   = {}                  # holder of all data
        for fn in os.listdir(dir):
#------------------------------------------------------------------------------
#       ^ read new file and calculate totals. All data in a single file are assumed
#         to have the same timestamp
#------------------------------------------------------------------------------
            if (self.fDebug): print(fn);
            f     = open(dir+'/'+fn, 'r')
            lines = f.readlines()
            nl    = len(lines);
#------------------------------------------------------------------------------
# first like - comma-separated keys
#------------------------------------------------------------------------------
            keys = lines[0].split(',')
            for i in range(1,nl-1):
                line   = lines[i]
                nlines += 1;
#                print(line)
                words = line.strip().split(',')

                # print(words)

                r   = {}                       # 'r' - new data record
                err = 0
                nw  = len(words)
                for i in range(0,nw) :
                    key = keys[i].strip()
                    val = words[i].strip()
                    # print('key=',key,' val = ',val);
                    if   (key == 'rid'    ) : r[key] = int(val)
                    elif (key == 'uts'    ) : r[key] = int(val)
                    elif (key == 'country') : 
#------------------------------------------------------------------------------
# correct/unify country name spelling spelling
#------------------------------------------------------------------------------
                        if   (val == 'U.K.'                  ) : val = 'UK'
                        if   (val == 'U.A.E.'                ) : val = 'UAE'
                        elif (val == 'S._Korea'              ) : val = 'South_Korea'
                        elif (val == 'St._Barth'             ) : val = 'St_Barth'
                        elif (val == 'St._Vincent_Grenadines') : val = 'St_Vincent_Grenadines'
                        
                        r[key] = val
                    elif (key == 'state'  ) : r[key] = val
                    elif (key == 'county' ) : r[key] = val
                    elif (key == 'totc'   ) : r[key] = int(val)
                    elif (key == 'newc'   ) : r[key] = int(val)
                    elif (key == 'totd'   ) : r[key] = int(val)
                    elif (key == 'newd'   ) : r[key] = int(val)
                    elif (key == 'totr'   ) : r[key] = int(val)
                    elif (key == 'ac'     ) : r[key] = int(val)
                    elif (key == 'serc'   ) : r[key] = int(val)
                    elif (key == 'cpm'    ) : r[key] = int(val)
                    elif (key == 'ts'     ) : r[key] = val
                    else:
                        err = 1
                        print("ERROR: unknown key:",key);

                if (err == 0):
                    if self.fDebug: print(r)
                    
                    country = r['country'];
                    state   = r['state'  ];
                    if (not country in data.keys()):
                        data.update({country:{}})

                    data_country = data[country]
                    
                    if (not state in data_country.keys()):
                        data_country.update({state:[]})     # this is a list of histograms

                    data_state = data_country[state];
                    
                    data_state.append(r);
#------------------------------------------------------------------------------
#           ^ end of file processing
#------------------------------------------------------------------------------
            f.close()
#------------------------------------------------------------------------------
#       ^ end of loop over the files, print totals
#------------------------------------------------------------------------------
        # print('nlines = ',nlines)
#------------------------------------------------------------------------------
# in the end, for each country, sort data in time
#------------------------------------------------------------------------------
        for country in data.keys():
            data_country = data[country]
            self.fData.update({country:{}})
            for state in data_country.keys(): 
                self.fData[country].update({state:sorted(data_country[state],key=itemgetter('uts'))})


#------------------------------------------------------------------------------
# end of the class definition
#------------------------------------------------------------------------------
if (__name__ == '__main__'):
    data = Covid19Data();
    data.print()
