#!/usr/bin/python

from ROOT     import TCanvas, TGraphErrors, TGraph, TH1F, TH2F

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
