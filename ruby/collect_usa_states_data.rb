# coding: utf-8
require 'watir'
require 'nokogiri'


def test1() 
  browser = Watir::Browser.new
  browser.goto('http://stackoverflow.com/')
  
  puts browser.title
  browser.close
end

def get_month(month)
  months  = { "January"   => 1,
              "February"  => 2,
              "March"     => 3,
              "April"     => 4,
              "May"       => 5,
              "June"      => 6,
              "July"      => 7,
              "August"    => 8,
              "September" => 9,
              "October"   => 10,
              "November"  => 11,
              "December"  => 12
            }
  return months[month];
end

#------------------------------------------------------------------------------
def parse_ak()
  state = 'AK'
  url   = 'http://dhss.alaska.gov/dph/Epi/id/Pages/COVID-19/monitoring.aspx'
  
  browser = Watir::Browser.new
  browser.goto(url);
  doc = Nokogiri::HTML.parse(browser.html())
  browser.close();
  
  tnow = DateTime.now().strftime();

  output_fn = "data/"+tnow+'_USA_'+state+'.html';
  f         = File.open(output_fn,"w");
  
#  list_of_tables = doc.css("//table");
#  for table in list_of_tables
#    f.puts(table)
#  end

  f.puts(doc);

  f.close();
  
  return ;
end

#------------------------------------------------------------------------------
def parse_al()

  browser = Watir::Browser.new
  url = 'http://www.alabamapublichealth.gov/infectiousdiseases/2019-coronavirus.html';
  browser.goto(url);
  doc = Nokogiri::HTML.parse(browser.html())
  browser.close();
  
  list_of_tables = doc.css("//table");

  t0 = list_of_tables[0];

  rows = t0.css('tr');

  nr = rows.length();

  ts = rows[0].css('h2')[1].text

  x = ts.split();
  month  = get_month(x[1]);
  date   = x[2].split(',')[0].to_i;
  year   = x[3].to_i
  hour   = x[4].split(':')[0].to_i;
  minute = x[4].split(':')[1].to_i;

  if (x[5] == 'p.m.') then ; hour = hour+12; end
    
  zone = Time.local(year,month,date).zone();

  # puts "month = #{month}  date = #{date} year = #{year} hour = #{hour} minute = #{minute}zone=#{zone}"

  ts = DateTime.new(year,month,date,hour,minute,0,zone).strftime();

  # puts "----------------- ts = #{ts}"
  
  last_row = rows[nr-1];
  data     = last_row.css('td/p')
  ntot     = data[1].text.sub(/\n+\Z/, '').sub(/\t+\Z/, '').rstrip.to_i;

  tnow = DateTime.now().strftime();
  output_fn = "data/"+tnow+'_USA_AL.txt';
  
  f    = File.open(output_fn,"w");
  
  f.printf("%s USA AL %-20s %6i\n",ts,"_ntotal_",ntot)

  for i in 3..nr-2
    data   = rows[i].css('td')
    county = data[0].css('p').text.sub(/\n+\Z/, '').sub(/\t+\Z/, '').rstrip;
    ncases = data[1].css('p').text.sub(/\n+\Z/, '').sub(/\t+\Z/, '').rstrip.to_i
    
    f.printf("%s USA AL %-20s %6i\n",ts,county,ncases)
  end

  f.close();
  #  puts 'priehali'
  #  puts doc
  return ;
end

#------------------------------------------------------------------------------
# Arizona
#------------------------------------------------------------------------------
def parse_az()
  state = 'AZ'
  url   = 'https://www.azdhs.gov/'
  
  browser = Watir::Browser.new
  browser.goto(url);
  sleep 20
  doc = Nokogiri::HTML.parse(browser.html())
  browser.close();
  
  tnow = DateTime.now().strftime();

  output_fn = "data/"+tnow+'_USA_'+state+'.html';
  f         = File.open(output_fn,"w");
  
#  list_of_tables = doc.css("//table");
#  for table in list_of_tables
#    f.puts(table)
#  end

  f.puts(doc);

  f.close();
  
  return ;
end


#------------------------------------------------------------------------------
def parse_ct()
  state = "CT";
  url   = 'https://portal.ct.gov/Coronavirus'

  browser = Watir::Browser.new
  browser.goto(url);
  doc = Nokogiri::HTML.parse(browser.html())
  browser.close();
  
  tnow = DateTime.now().strftime();

  output_fn = "data/"+tnow+'_USA_'+state+'.html';
  f         = File.open(output_fn,"w");
  
  list_of_tables = doc.css("//table");

  for table in list_of_tables
    f.puts(table)
  end

  f.close();
  
  return ;
end

#------------------------------------------------------------------------------
# Arkansas
#------------------------------------------------------------------------------
def parse_ar()
  state = 'AR'
  url   = 'https://www.healthy.arkansas.gov/programs-services/topics/novel-coronavirus'
  
  browser = Watir::Browser.new
  browser.goto(url);
  doc = Nokogiri::HTML.parse(browser.html())
  browser.close();
  
  tnow = DateTime.now().strftime();

  output_fn = "data/"+tnow+'_USA_'+state+'.html';
  f         = File.open(output_fn,"w");
  
#  list_of_tables = doc.css("//table");
#  for table in list_of_tables
#    f.puts(table)
#  end

  f.puts(doc);

  f.close();
  
  return ;
end

#------------------------------------------------------------------------------
def parse_dc()

  browser = Watir::Browser.new
  url = 'https://coronavirus.dc.gov/page/coronavirus-data'
  browser.goto(url);
  doc = Nokogiri::HTML.parse(browser.html())
  browser.close();
  
  tnow = DateTime.now().strftime();

  output_fn = "data/"+tnow+'_USA_DC.html';
  f         = File.open(output_fn,"w");

  f.puts(doc)
  
#  list_of_tables = doc.css("//table");

#  for table in list_of_tables
#    f.puts(table)
#  end

  f.close();
  
  return ;
end

#------------------------------------------------------------------------------
def parse_ga()
  state = "GA";
  url   = 'https://dph.georgia.gov/covid-19-daily-status-report'

  browser = Watir::Browser.new
  browser.goto(url);
  doc = Nokogiri::HTML.parse(browser.html())
  browser.close();
  
  tnow = DateTime.now().strftime();

  output_fn = "data/"+tnow+'_USA_'+state+'.html';
  f         = File.open(output_fn,"w");

  f.puts(doc)
  
#  list_of_tables = doc.css("//table");

#  for table in list_of_tables
#    f.puts(table)
#  end

  f.close();
  
  return ;
end


#------------------------------------------------------------------------------
def parse_id()
  state = "ID";
  url   = 'https://coronavirus.idaho.gov/'

  browser = Watir::Browser.new
  browser.goto(url);
  doc = Nokogiri::HTML.parse(browser.html())
  browser.close();
  
  tnow = DateTime.now().strftime();

  output_fn = "data/"+tnow+'_USA_'+state+'.html';
  f         = File.open(output_fn,"w");

  f.puts(doc)
  
#  list_of_tables = doc.css("//table");

#  for table in list_of_tables
#    f.puts(table)
#  end

  f.close();
  
  return ;
end

#------------------------------------------------------------------------------
def parse_il()
  state = "IL";
  url   = 'http://www.dph.illinois.gov/topics-services/diseases-and-conditions/diseases-a-z-list/coronavirus'

  browser = Watir::Browser.new
  browser.goto(url);
  doc = Nokogiri::HTML.parse(browser.html())
  browser.close();
  
  tnow = DateTime.now().strftime();

  output_fn = "data/"+tnow+'_USA_'+state+'.html';
  f         = File.open(output_fn,"w");

  f.puts(doc)
  
#  list_of_tables = doc.css("//table");

#  for table in list_of_tables
#    f.puts(table)
#  end

  f.close();
  
  return ;
end

#------------------------------------------------------------------------------
# Massachusets
#------------------------------------------------------------------------------
def parse_ma()
  state = "MA";
  url   = 'https://www.mass.gov/doc/covid-19-cases-in-massachusetts-as-of-march-18-2020/download'

  browser = Watir::Browser.new
  browser.goto(url);
  doc = Nokogiri::HTML.parse(browser.html())
  browser.close();
  
  tnow = DateTime.now().strftime();

  output_fn = "data/"+tnow+'_USA_'+state+'.pdf';
  f         = File.open(output_fn,"w");

  f.puts(doc)
  
#  list_of_tables = doc.css("//table");

#  for table in list_of_tables
#    f.puts(table)
#  end

  f.close();
  
  return ;
end

#------------------------------------------------------------------------------
# Maine
#------------------------------------------------------------------------------
def parse_me()
  state = "ME";
  url   = 'https://www.maine.gov/dhhs/mecdc/infectious-disease/epi/airborne/coronavirus.shtml'

  browser = Watir::Browser.new
  browser.goto(url);
  doc = Nokogiri::HTML.parse(browser.html())
  browser.close();
  
  tnow = DateTime.now().strftime();

  output_fn = "data/"+tnow+'_USA_'+state+'.html';
  f         = File.open(output_fn,"w");

  f.puts(doc)
  
#  list_of_tables = doc.css("//table");

#  for table in list_of_tables
#    f.puts(table)
#  end

  f.close();
  
  return ;
end

#------------------------------------------------------------------------------
# Mississippi
#------------------------------------------------------------------------------
def parse_ms()
  state = "MS";
  url   = 'https://msdh.ms.gov/msdhsite/_static/14,0,420.html'

  browser = Watir::Browser.new
  browser.goto(url);
  doc = Nokogiri::HTML.parse(browser.html())
  browser.close();
  
  tnow = DateTime.now().strftime();

  output_fn = "data/"+tnow+'_USA_'+state+'.html';
  f         = File.open(output_fn,"w");

  f.puts(doc)
  
#  list_of_tables = doc.css("//table");

#  for table in list_of_tables
#    f.puts(table)
#  end

  f.close();
  
  return ;
end

#------------------------------------------------------------------------------
# New Hampshire (only state summary, by-county - pdf)
# https://www.nh.gov/covid19/documents/case-map.pdf
#------------------------------------------------------------------------------
def parse_nh()
  state = "NH";
  url   = 'https://www.nh.gov/covid19/'

  browser = Watir::Browser.new
  browser.goto(url);
  doc = Nokogiri::HTML.parse(browser.html())
  browser.close();
  
  tnow = DateTime.now().strftime();

  output_fn = "data/"+tnow+'_USA_'+state+'.html';
  f         = File.open(output_fn,"w");

  f.puts(doc)
  
#  list_of_tables = doc.css("//table");

#  for table in list_of_tables
#    f.puts(table)
#  end

  f.close();
  
  return ;
end
#------------------------------------------------------------------------------
# New Jersey (dashboard only state summary, by-county - pdf)
# https://www.nh.gov/covid19/documents/case-map.pdf
# separate frame for the dashboard, dashboard readable, but no history...
#------------------------------------------------------------------------------
def parse_nj()
  state = "NJ";
#  url   = 'https://www.nj.gov/health/cd/topics/covid2019_dashboard.shtml'
  url = 'https://maps.arcgis.com/apps/opsdashboard/index.html#/ec4bffd48f7e495182226eee7962b422'
  browser = Watir::Browser.new
  browser.goto(url);
  sleep 10
  
  doc = Nokogiri::HTML.parse(browser.html())
  browser.close();
  
  tnow = DateTime.now().strftime();

  output_fn = "data/"+tnow+'_USA_'+state+'.html';
  f         = File.open(output_fn,"w");

  f.puts(doc)
  
#  list_of_tables = doc.css("//table");

#  for table in list_of_tables
#    f.puts(table)
#  end

  f.close();
  
  return ;
end

#------------------------------------------------------------------------------
# Texas
#------------------------------------------------------------------------------
def parse_tx()
  state = "TX";
  url   = 'https://www.dshs.state.tx.us/news/updates.shtm#coronavirus'

  browser = Watir::Browser.new
  browser.goto(url);
  doc = Nokogiri::HTML.parse(browser.html())
  browser.close();
  
  tnow = DateTime.now().strftime();

  output_fn = "data/"+tnow+'_USA_'+state+'.html';
  f         = File.open(output_fn,"w");

  f.puts(doc)
  
#  list_of_tables = doc.css("//table");

#  for table in list_of_tables
#    f.puts(table)
#  end

  f.close();
  
  return ;
end

#------------------------------------------------------------------------------
# Vermont 
# summary nly, breakdown at .png:
# https://www.healthvermont.gov/sites/default/files/images/2020/03/COVID19_Map_3.20.2020.png
#------------------------------------------------------------------------------
def parse_vt()
  state = "VT";
  url   = 'https://www.healthvermont.gov/response/infectious-disease/2019-novel-coronavirus'

  browser = Watir::Browser.new
  browser.goto(url);

  doc = Nokogiri::HTML.parse(browser.html)
  
  tnow = DateTime.now().strftime();

  output_fn = "data/"+tnow+'_USA_'+state+'.html';
  f         = File.open(output_fn,"w");

  f.puts(doc)
  
#  list_of_tables = doc.css("//table");

#  for table in list_of_tables
#    f.puts(table)
#  end

  f.close();
  browser.close();  
  return ;
end

#------------------------------------------------------------------------------
# Washington - need to wait for 5-10 sec
#------------------------------------------------------------------------------
def parse_wa()
  state = "WA";
  url   = 'https://www.doh.wa.gov/emergencies/coronavirus'

  browser = Watir::Browser.new
  browser.goto(url);

  sleep 10

  html = browser.html
  doc = Nokogiri::HTML.parse(html)
  
  tnow = DateTime.now().strftime();

  output_fn = "data/"+tnow+'_USA_'+state+'.html';
  f         = File.open(output_fn,"w");

  f.puts(doc)
  
#  list_of_tables = doc.css("//table");

#  for table in list_of_tables
#    f.puts(table)
#  end

  f.close();
  browser.close();  
  return ;
end

#------------------------------------------------------------------------------
def parse_worldometers_us(url = '',sleep_time = 0)
  state         = "total";
  default_url   = 'https://www.worldometers.info/coronavirus/country/us/'


  if (url == '') then url=default_url; end
  
  browser = Watir::Browser.new
  browser.goto(url);

  if (sleep_time > 0) then sleep sleep_time end
  
  doc = Nokogiri::HTML.parse(browser.html())
  browser.close();
  
  tnow = DateTime.now().strftime();

  output_fn = "data/"+tnow+'_USA_'+state+'.html';
  f         = File.open(output_fn,"w");
  
  list_of_tables = doc.css("//table");

  for table in list_of_tables
    f.puts(table)
  end

  f.close();
  
  return ;
end

