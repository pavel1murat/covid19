#!/usr/bin/python
# coding: utf-8
#------------------------------------------------------------------------------
# a.fHist[country][key] - histogram
#------------------------------------------------------------------------------
from ROOT import TCanvas, TGraphErrors, TGraph
from ROOT import gROOT
from array import array

from Covid19Data import Covid19Data

class HistRecord:
    def __init__(self,country,key):
        self.fHist = None;
        self.fName = country+':'+key

    def Draw(self,*args, **kwargs):
        opt = kwargs.get('opt', None)

        marker = kwargs.get('marker', None)
        col    = kwargs.get('col'   , None)

        if (marker) : self.fHist.SetMarkerStyle(int(marker));
        if (col   ) :
            self.fHist.SetLineColor  (int(col));
            self.fHist.SetMarkerColor(int(col));

        self.fHist.Draw(opt);

class Ana:
#------------------------------------------------------------------------------
    def __init__(self,region = 'world', debug = None): 

        self.fCanvas  = None; # TCanvas( 'c1', 'A Simple Graph with error bars', 200, 10, 700, 500 )
        self.fGraph   = None
        self.fHist    = {}
        self.fDebug   = debug
        
        keys = ['totc','totd','tdrate']

        if (region == 'world'):
            countries = [
                'China','Italy','USA','Spain','Germany','Iran','France','South_Korea','Switzerland','UK','Netherlands','Austria',
                'Belgium','Norway','Sweden','Australia','Portugal','Brazil','Canada','Denmark','Malaysia','Turkey','Czechia',
                'Japan','Israel','Ireland','Luxembourg','Ecuador','Pakistan','Diamond_Princess','Poland','Chile','Finland','Greece',
                'Thailand','Iceland','Indonesia','Saudi_Arabia','Qatar','Singapore','Romania','Slovenia','India','Philippines','Russia',
                'Peru','Bahrain','Egypt','Estonia','Hong_Kong','Panama','South_Africa','Argentina','Croatia','Mexico','Lebanon','Iraq',
                'Colombia','Serbia','Dominican_Republic','Algeria','Armenia','Kuwait','Bulgaria','Slovakia','San_Marino','Taiwan','Uruguay',
                'UAE','Lithuania','Latvia','Costa_Rica','Hungary','Bosnia_and_Herzegovina','Morocco','North_Macedonia','Faeroe_Islands',
                'Andorra','Vietnam','Jordan','New_Zealand','Cyprus','Moldova','Malta','Albania','Brunei','Cambodia','Sri_Lanka','Belarus',
                'Burkina_Faso','Tunisia','Ukraine','Venezuela','Senegal','Azerbaijan','Réunion','Kazakhstan','Palestine','Guadeloupe',
                'Oman','Georgia','Trinidad_and_Tobago','Martinique','Uzbekistan','Afghanistan','Cameroon','Liechtenstein','Cuba','Channel_Islands',
                'DRC','Nigeria','Mauritius','Bangladesh','Guam','Honduras','Bolivia','Ghana','Puerto_Rico','Monaco','Paraguay','Macao','Montenegro',
                'Guatemala','Guyana','Jamaica','Rwanda','French_Guiana','French_Polynesia','Togo','Gibraltar','Kenya','Barbados','Ivory_Coast',
                'Kyrgyzstan','Maldives','Tanzania','Ethiopia','Mayotte','Mongolia','Aruba','Seychelles','Bermuda','Equatorial_Guinea','U.S._Virgin_Islands',
                'Gabon','Isle_of_Man','Saint_Martin','Suriname','Bahamas','Greenland','New_Caledonia','Eswatini','Cayman_Islands','Curaçao','Cabo_Verde',
                'CAR','Congo','El_Salvador','Liberia','Madagascar','Namibia','St_Barth','Zambia','Zimbabwe','Sudan','Angola','Benin','Bhutan','Fiji',
                'Guinea','Haiti','Mauritania','Nicaragua','Niger','Saint_Lucia','Nepal','Antigua_and_Barbuda','Chad','Djibouti','Dominica','Eritrea',
                'Gambia','Grenada','Vatican_City','Montserrat','Mozambique','Papua_New_Guinea','St_Vincent_Grenadines','Sint_Maarten','Somalia',       
                'Syria','Timor-Leste','Uganda'
            ]

            dir           = '/projects/covid19/data/world/txt';
            self.fCovid19 = Covid19Data(dir,self.fDebug)

            for country in countries:
                for key in keys:
                    self.fill(country,key)
                    
        elif (region == 'USA'):
            dir           = '/projects/covid19/data/usa/txt';
            self.fCovid19 = Covid19Data(dir,self.fDebug)
            
            for country in ['USA']:
                for key in keys:
                    self.fill(country,key)
            
            
#------------------------------------------------------------------------------
    def variable(self,r,key):
        var = None;
        if (key == 'tdrate'): var = r['totd']/r['totc'];
        else                : var = r[key]

        return var
    
#------------------------------------------------------------------------------
    def hist(self,country_code,name):
           
        c = country_code.split(':')
        country = c[0];
        if (len(c) == 1): state = 'total'
        else            : state = c[1]

        return self.fHist[country][state][name]
    
#------------------------------------------------------------------------------
# 'country_code' = 'country:state:'
#------------------------------------------------------------------------------
    def fill(self, country_code, key = 'newc', name = None):

        if (name == None) : name = key;

        if (self.fDebug): print ('<ana::fill> country_code, key = %s %s'%(country_code,key))

        c       = country_code.split(':');
        country = c[0];

        # state is always a list of states
        if (len(c) == 1): list_of_states = self.fCovid19.fData[country].keys()
        else            : list_of_states = [c[1]];
            
        if (not country in self.fHist.keys()): self.fHist.update({country:{}})

        cd = self.fHist[country]

        for state in list_of_states:
            if (not state in self.fHist[country].keys()): self.fHist[country].update({state:{}})

            ccode = country+':'+state
            hr = HistRecord(country_code,key);
        
            x = array('f',[])
            y = array('f',[])

            for r in self.fCovid19.fData[country][state]:
                var = self.variable(r,key);
                x.append(float(r['uts']))               # time
                y.append(float(var))
#------------------------------------------------------------------------------
#           ^ data arrays are filled, book histogram,
#------------------------------------------------------------------------------
            hr.fHist = TGraph(len(x), x, y)

            self.fHist[country][state].update({name:hr});

            hist_name  = country+'_'+state+'_'+name;
            hist_title = country+':'+state+':'+name;
        
            hr.fHist.SetName    (hist_name)
            hr.fHist.SetMinimum (0.5)
            hr.fHist.SetTitle(hist_title)
            hr.fHist.GetXaxis().SetTimeDisplay(1);
            hr.fHist.SetMarkerStyle(20);
