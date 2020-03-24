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
    def __init__(self,dir='/projects/covid19/data/world/txt'):
        self.fData = {}
        self.fDir  = dir
        self.read(dir)
#------------------------------------------------------------------------------
    def print(self,country = None):
        # print ROOT format string
        print('rid/I:time/I:date/C:country/C:state/C:county/C:totc/I:newc/I:totd/I:newd/I:totr/I:ac/I:serc/I:cpm/I');

        for data in self.fData:
            if ( not country) or (country == data['country']):
                print("%2i %i %s %-25s %s %s %6i %6i %6i %6i %6i %6i %6i %6i"
                      %(data['rid'],data['uts'],data['ts'],
                        data['country'],data['state'], data['county'],
                        data['totc'],data['newc'],data['totd'],data['newd'],
                        data['totr'],data['ac'],data['serc'],data['cpm']))

#------------------------------------------------------------------------------
    def read(self,dir):
        nlines = 0;
        data   = {}
        for fn in os.listdir(dir):
            f = open(dir+'/'+fn, 'r')
            for line in f.readlines():
                nlines += 1;
                # print(line.strip())
                words = line.strip().split(',')

                # print(words)

                # create new dict
                r = {}
                err = 0
                for w in words :
                    ww = w.split(':')
                    key = ww[0].strip()
                    val = ww[1].strip()
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
                    elif (key == 'ts'     ) : r[key] = w[3:].strip()
                    else:
                        err = 1
                        print("ERROR: unknown key:",key);

                if (err == 0):
                    # print(r)
                    country = r['country'];
                    if (not country in data.keys()):
                        data.update({country:[]})
                    data[country].append(r);
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
            country_data = data[country]
            self.fData.update({country:sorted(country_data,key=itemgetter('uts'))})
#------------------------------------------------------------------------------
# end of the class definition
#------------------------------------------------------------------------------
if (__name__ == '__main__'):
    data = Covid19Data();
    data.print()
