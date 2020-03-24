#!/bin/bash

d=`date +%Y-%m-%d-%H-%M-%S`

datadir=$PWD/data

wget -O $datadir/${d}-covid19-1point3acres.html https://coronavirus.1point3acres.com/

wget -O $datadir/${d}-covid19-world.html https://www.worldometers.info/coronavirus/

# before 2020-03-20 wget -O $datadir/${d}-covid19-usa.html   https://www.worldometers.info/coronavirus/us/
wget -O $datadir/${d}-covid19-usa.html   https://www.worldometers.info/coronavirus/country/us/

wget -O $datadir/${d}-covid19-AL.html    http://www.alabamapublichealth.gov/infectiousdiseases/2019-coronavirus.html

wget -O $datadir/${d}-covid19-CT.html    https://portal.ct.gov/Coronavirus
wget -O $datadir/${d}-covid19-DC.html    https://coronavirus.dc.gov/page/coronavirus-data

wget -O $datadir/${d}-covid19-GA.html    https://dph.georgia.gov/covid-19-daily-status-report
wget -O $datadir/${d}-covid19-ID.html    https://coronavirus.idaho.gov/
wget -O $datadir/${d}-covid19-IL.html    http://www.dph.illinois.gov/topics-services/diseases-and-conditions/diseases-a-z-list/coronavirus

# this is .pdf
wget -O $datadir/${d}-covid19-MA.pdf     https://www.mass.gov/doc/covid-19-cases-in-massachusetts-as-of-march-20-2020/download
wget -O $datadir/${d}-covid19-MA.pdf     https://www.mass.gov/doc/covid-19-cases-in-massachusetts-as-of-march-19-2020/download

wget -O $datadir/${d}-covid19-MN.html    https://www.maine.gov/dhhs/mecdc/infectious-disease/epi/airborne/coronavirus.shtml
wget -O $datadir/${d}-covid19-MS.html    https://msdh.ms.gov/msdhsite/_static/14,0,420.html

wget -O $datadir/${d}-covid19-NH.html    https://www.nh.gov/covid19/
wget -O $datadir/${d}-covid19-NH.pdf     https://www.nh.gov/covid19/documents/case-map.pdf

wget --no-check-certificate  -O $datadir/${d}-covid19-TX.html https://www.dshs.state.tx.us/news/updates.shtm\#coronavirus
wget                         -O $datadir/${d}-covid19-WA.html https://www.doh.wa.gov/emergencies/coronavirus

wget -O $datadir/${d}-covid19-VT.html    https://www.healthvermont.gov/response/infectious-disease/2019-novel-coronavirus
# starting 2020-03-20
wget -O $datadir/${d}-covid19-VT.png     https://www.healthvermont.gov/sites/default/files/images/2020/03/COVID19_Map_3.20.2020.png
