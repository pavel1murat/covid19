#!/usr/bin/ruby
# coding: utf-8
require 'watir'
require 'nokogiri'

require 'local_tools.rb'
#------------------------------------------------------------------------------
# grab web page and save it to disk
# for archived paged, form the file name based on the shapshot time stamp
# https://web.archive.org/web/20200320211139/https://www.worldometers.info/coronavirus/country/us/
#------------------------------------------------------------------------------
def parse_canada(url = '',sleep_time = 0)
  state         = "total";
  default_url   = 'https://www.canada.ca/en/public-health/services/diseases/2019-novel-coronavirus-infection.html'

  if (url == '') then url=default_url; end

  server = url.split('/')[2];

  b = Watir::Browser.new :chrome

  b.goto(server)
  b.cookies.add 'foo', 'bar', path: '/', expires: (Time.now + 10000), secure: true
  
  b.goto(url)

  if (sleep_time > 0) then sleep sleep_time end
  
  doc = Nokogiri::HTML.parse(b.html())
  b.close();

  ts = DateTime.now().strftime();

  if (server == "web.archive.org") then
#------------------------------------------------------------------------------
# redefine output file name based on the snapshot name
#------------------------------------------------------------------------------
    time = url.split('/')[4];
    year = time[0,4].to_i
    mon  = time[4,2].to_i
    day  = time[6,2].to_i
    hour = time[8,2].to_i
    min  = time[10,2].to_i
    sec  = time[12,2].to_i
    ts   = DateTime.new(year,mon,day,hour,min,sec,'+00:00').strftime();
  end

  output_fn = "data/canada/html/"+ts+'_canada.html';
  f         = File.open(output_fn,"w");

  f.puts(doc);
  f.close();
  
  return ;
end

#------------------------------------------------------------------------------
# debugging, parse local .html
# for Mar 18 and later
#------------------------------------------------------------------------------
def parse_canada_1_v1(input_fn)

  state       = "total:";
  
  input_file  = File.open(input_fn);
  doc         = Nokogiri::HTML.parse(input_file)
  input_file.close();

  # current timestamp
  ts_now    = DateTime.now().strftime();

  output_fn = File.basename(input_fn,".*")+'_debug.xml';
  f         = File.open(output_fn,"w");
#------------------------------------------------------------------------------
# determine the time stamp of the page update
#------------------------------------------------------------------------------
  list = doc.xpath("//div/table");

  nel = list.length

  printf("nelements: %i\n",nel);
  # print_elements(list,"doc.xpath(\"pat\")");

  year    = -1;
  month   = -1;
  date    = -1;
  hour    = -1;
  minutes = -1;
  sec     =  0;
  zone    =  '-05:00';

  for i in 0..list.size-1
    el = list[i]
    f.printf("------------- //div/table \n");
    f.puts(el);
  end

  f.puts("-------------------- //end of divisions");

  #------------------------------------------------------------------------------
  # there are 3 tables, need the first one
  #------------------------------------------------------------------------------
  t0 = list[0]

  caption = t0.xpath("caption").text
  word    = caption.split
    
  if ((word[7] == 'as') and (word[8] == 'of')) then
    # f.printf("------------- //div.text: %s\n",div.text);
    month   = get_month(word[9]);
    date    = word[10].split(',')[0].to_i;
    year    = word[11].split(',')[0].to_i;
    hour    = word[12].split(':')[0].to_i;
    minutes = word[12].split(':')[1].to_i;

    # printf("word[13] = %s,%s\n",word[13],word[13].split('_')[0]);
    if (word[13][0,2] == 'pm') then
      hour += 12;
    end
  end
  
  puts("year:#{year} month:#{month} date:#{date} hour:#{hour} minutes:#{minutes} sec:#{sec} zone:#{zone}")

  ts_update    = DateTime.new(year,month,date,hour,minutes,0,zone);
  time_stamp   = ts_update.strftime();
#------------------------------------------------------------------------------
# parse table data
# it looks that if I use '//td', then all rows in all tables in the list are returned
#------------------------------------------------------------------------------
  rows = t0.xpath('tbody/tr')

  printf("nrows = %i\n",rows.length)

  ##  list_of_tables = doc.xpath("//div[@class='container']/div/div/div/div/div/table[@id='main_table_countries_yesterday']");
##  printf("ntables: %i\n",list_of_tables.length)
##  
##  f.printf("#  timestamp:country:state:tot_cases:new_cases:tot_deaths:new_deaths:tot_recovered:active_cases:serious_cases:cases_per_mil\n");
##  #------------------------------------------------------------------------------
##  # starting from Mar 19 table ID : 'main_table_countries_today' and 'main_table_countries_yesterday'
##  # Mar 18                        : 'usa_table_countries'
##  #------------------------------------------------------------------------------
  country = 'canada'
  
  for row in rows
    cells         = row.xpath('td')
    province      = cells[0].text.sub(',','').sub(/\n+\Z/,'').sub(/\t+\Z/,'').rstrip.lstrip;
    if (province.split().length > 1) then
      province = province.split().join('_')
    end
    tot_cases        = parse_number(cells[1].text);
    confirmed_cases  = parse_number(cells[2].text);
    tot_deaths    = parse_number(cells[3].text);

    f.printf("%-25s ",time_stamp)
    f.printf("%-10s ",country)
    f.printf("%-30s ",province)
    f.printf("%8i ",tot_cases)
    f.printf("%8i ",confirmed_cases)
    f.printf("%8i\n",tot_deaths)
  end

  f.puts(t0)
  f.close();
  
  return ;
end

#------------------------------------------------------------------------------
# parse local .html ==> .txt
# use yesterday's table - it seems to be more stable in time during the day,
# today's results are strongly dependent on the state reporting times..
# output file basename is the same as the input file basename
# assume that the input file names don't have an extra '.' in them
# for Mar 18 and after
#------------------------------------------------------------------------------
def parse_canada_2_v1(input_fn)

  output_dir = 'data/world/txt';

  input_file  = File.open(input_fn);
  doc         = Nokogiri::HTML.parse(input_file)
  input_file.close();

  # current timestamp
  ts_now    = DateTime.now().strftime();

  output_fn = output_dir+'/'+File.basename(input_fn,".*")+'.txt';
  f         = File.open(output_fn,"w");
#------------------------------------------------------------------------------
# determine the time stamp of the page update
#------------------------------------------------------------------------------
  list = doc.xpath("//div[@class='content-inner']/div");

  year    = -1;
  month   = -1;
  date    = -1;
  hour    = -1;
  minutes = -1;
  sec     =  0;
  zone    =  '+00:00';

  for i in 0..list.size-1
    div = list[i]
    # f.printf("------------- //div \n");
    # f.puts(div);

    word = div.text.split()
    if ((word[0] == 'Last') and (word[1] == 'updated:')) then
      # f.printf("------------- //div.text: %s\n",div.text);
      month   = get_month(word[2]);
      date    = word[3].split(',')[0].to_i;
      year    = word[4].split(',')[0].to_i;
      hour    = word[5].split(':')[0].to_i;
      minutes = word[5].split(':')[1].to_i;
      if (word[6] != "GMT") then
        printf("ERROR: undefined zone : %s\n",word[6]);
      end
    end
  end

  puts("year:#{year} month:#{month} date:#{date} hour:#{hour} minutes:#{minutes} sec:#{sec} zone:#{zone}")
  ts_update    = DateTime.new(year,month,date,hour,minutes,0,zone);
  ts_yesterday = ts_update.prev_day();
  time_stamp   = ts_yesterday.strftime();
#------------------------------------------------------------------------------
# parse yesterday's table data
# it looks that if I use '//td', then all rows in all tables in the list are returned
#------------------------------------------------------------------------------
  list_of_tables = doc.xpath("//div[@class='container']/div/div/div/div/div/table[@id='main_table_countries_yesterday']");

  printf("ntables: %i\n",list_of_tables.length)

  state = "total:";

  f.printf("#  timestamp:country:state:tot_cases:new_cases:tot_deaths:new_deaths:tot_recovered:active_cases:serious_cases:cases_per_mil\n");
         
  for table in list_of_tables
    if (table['id'] == 'main_table_countries_yesterday') then
      # f.printf("------------- //table YESTERDAY\n");
      time_stamp   = ts_yesterday.strftime();
  
      rows = table.xpath('tbody/tr')

      for row in rows
        cells         = row.xpath('td')
        country       = cells[0].text.sub(',','').sub(/\n+\Z/,'').sub(/\t+\Z/,'').rstrip.lstrip.gsub(' ','_');
        tot_cases     = parse_number(cells[1].text);
        new_cases     = parse_number(cells[2].text);
        tot_deaths    = parse_number(cells[3].text);
        new_deaths    = parse_number(cells[4].text);
        tot_recovered = parse_number(cells[5].text);
        active_cases  = parse_number(cells[6].text);
        serious_cases = parse_number(cells[7].text);
        cases_per_mil = parse_number(cells[8].text);

        f.printf("%-25s ",time_stamp)
        f.printf("%-30s ",country)
        f.printf("%-20s ",state)
        f.printf("%8i ",tot_cases)
        f.printf("%8i ",new_cases)
        f.printf("%8i ",tot_deaths)
        f.printf("%8i ",new_deaths)
        f.printf("%8i ",tot_recovered)
        f.printf("%8i ",active_cases)
        f.printf("%8i ",serious_cases)
        f.printf("%8i\n",cases_per_mil)
      end
    end
  end

  f.close();
  
  return ;
end
