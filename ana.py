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
    def __init__(self):
        self.fCovid19 = Covid19Data('/projects/covid19/data/world/txt')
        self.fCanvas  = None; # TCanvas( 'c1', 'A Simple Graph with error bars', 200, 10, 700, 500 )
        self.fGraph   = None
        self.fHist    = {}
        # fill histograms which are always needed

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

        keys = ['totc','totd','tdrate']
        
        for country in countries:
            for key in keys:
                self.fill(country,key)
                
#------------------------------------------------------------------------------
    def variable(self,r,key):
        var = None;
        if (key == 'tdrate'): var = r['totd']/r['totc'];
        else                : var = r[key]

        return var
    
#------------------------------------------------------------------------------
    def fill(self, country, key = 'newc', name = None):

        if (not country in self.fHist.keys()):
            self.fHist.update({country:{}})

        hr = HistRecord(country,key);
        
        x = array('f',[])
        y = array('f',[])

        for r in self.fCovid19.fData[country]:
            var = self.variable(r,key);
            x.append(float(r['uts']))               # time
            y.append(float(var))

        hr.fHist = TGraph(len(x), x, y)

        if (name == None) : name = key;
            
        self.fHist[country].update({name:hr});

        hist_name  = country+'_'+name;
        hist_title = country+':'+name;
        
        hr.fHist.SetName    (hist_name)
        hr.fHist.SetMinimum (0.5)
        hr.fHist.SetTitle(hist_title)
        hr.fHist.GetXaxis().SetTimeDisplay(1);
        hr.fHist.SetMarkerStyle(20);

