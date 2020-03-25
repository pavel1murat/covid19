#!/usr/bin/ruby
# coding: utf-8
#------------------------------------------------------------------------------
# use https://www.worldometers.info/coronavirus or https://web.archive.org
# use UNIX timestaps in the data
# v1: 2020 Mar 18 -        : two tables ID 'main_table_countries_today' and 'main_table_countries_yesterday'
# v2: 2020 Mar 07 - Mar 17 : one table, ID 'main_table_countries'
# v3: 2020 Feb 29 - Mar 06 : one table, ID 'main_table_countries', but no case_per_mil
# v4: 2020 Feb 03 - Feb 28 : one table, ID 'table3'              , totals need to be calculated, 8 cells
# v5: 2020 Jan 29 - Feb 02 : one table, ID 'table3'              , totals need to be calculated, 5 cells
#------------------------------------------------------------------------------
require 'watir'
require 'nokogiri'

require 'local_tools.rb'

class WorldDataParser
  def initialize()
    @name       = "world_data_parser";
    @url        = 'https://www.worldometers.info/coronavirus/'
    @country    = 'world'
    @output_dir = '/projects/covid19/data/'+@country;
  end

  #------------------------------------------------------------------------------
# grab web page and save it to disk
# for archived paged, form the file name based on the shapshot time stamp
# https://web.archive.org/web/20200320211139/https://www.worldometers.info/coronavirus/country/us/
#------------------------------------------------------------------------------
  def fetch_url(url = '',sleep_time = 0)
    state         = "total";

    if (url == '') then url=@url; end

    b = Watir::Browser.new :chrome
 
    b.goto('worldometers.info')
    b.cookies.add 'foo', 'bar', path: '/', expires: (Time.now + 10000), secure: true
  
    b.goto(url)

    if (sleep_time > 0) then sleep sleep_time end
  
    doc = Nokogiri::HTML.parse(b.html())
    b.close();

    ts = DateTime.now();

    server = url.split('/')[2];
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
      ts   = DateTime.new(year,mon,day,hour,min,sec,'+00:00');
    end

    output_fn = @output_dir+'/html/'+ts.strftime()+'_world.html';
    f         = File.open(output_fn,"w");

    f.puts(doc);
    f.close();
  
    return ;
  end

#------------------------------------------------------------------------------  
  def parse_time_stamp(doc, debug, file)

    list    = doc.xpath("//div[@class='content-inner']/div");

    year    = -1;
    month   = -1;
    date    = -1;
    hour    = -1;
    minutes = -1;
    sec     =  0;
    zone    =  '+00:00';

    for i in 0..list.size-1
      div = list[i]
      if (debug != 0) then
        file.printf("------------- //div \n");
        file.puts(div);
      end

      word = div.text.split()
      if ((word[0] == 'Last') and (word[1] == 'updated:')) then
        # f.printf("------------- //div.text: %s\n",div.text);
        month   = get_month(word[2]);
        date    = word[3].split(',')[0].to_i;
        year    = word[4].split(',')[0].to_i;
        hour    = word[5].split(':')[0].to_i;
        minutes = word[5].split(':')[1].to_i;
        if (word[6] != "GMT") and (word[6] != "UTC") then
          printf("ERROR: undefined zone : %s\n",word[6]);
        end
      end
    end
    if (debug > 0) then
      puts("year:#{year} month:#{month} date:#{date} hour:#{hour} minutes:#{minutes} sec:#{sec} zone:#{zone}")
    end
    
    time_stamp = DateTime.new(year,month,date,hour,minutes,0,zone);
    return time_stamp
  end
  
#------------------------------------------------------------------------------
# parse table row
#------------------------------------------------------------------------------
  def parse_cells(cells,time_stamp,version)

    ncells = cells.length
    
    if (version < 3) and (ncells != 9) then
      printf(" TROUBLE: WorldDataParser::parse_cells: version = %i ncells = %i, expected 9\n",version,ncells)
    elsif (version == 3) and (ncells != 8) then
      printf(" TROUBLE: WorldDataParser::parse_cells: version = %i ncells = %i, expected 9\n",version,ncells)
    elsif (version == 4) and (ncells < 6) then
      printf(" TROUBLE: WorldDataParser::parse_cells: version = %i ncells = %i, expected 8\n",version,ncells)
    end

    r            = {}
    r['id']      = 1;

    country   = cells[0].text.sub(',','').sub(/\n+\Z/,'').sub(/\t+\Z/,'').rstrip.lstrip.gsub(' ','_');
    if (country == 'Total:') then
      country = 'total'
    elsif (country == "U.S.") then
      country = "USA"
    end

    r['country'] = country;
    r['state']   = 'total'
    r['county']  = 'total'
    r['totc']    = parse_number(cells[1].text);
    r['newc']    = parse_number(cells[2].text);
    r['totd']    = parse_number(cells[3].text);

    
    r['newd']    = -1;
    r['totr']    = -1;
    r['ac'  ]    = -1;
    r['serc']    = -1;
    r['cpm' ]    = -1

    if (ncells > 5) then 
      r['newd']    = parse_number(cells[4].text);
      r['totr']    = parse_number(cells[5].text);
    end
    
    if (ncells > 6) then
      r['ac'  ]   = parse_number(cells[6].text);
      r['serc']   = parse_number(cells[7].text);
    
      if (ncells  > 8) then
        r['cpm' ] = parse_number(cells[8].text);
      end
    end

    r['ts'  ]    = time_stamp;

    return r;
  end

#------------------------------------------------------------------------------
  def print_header(file)
    file.printf("rid,ts,uts,country,state,county,totc,newc,totd,newd,totr,ac,serc,cpm\n");
  end
  
#------------------------------------------------------------------------------
# print data record to a file
#------------------------------------------------------------------------------
  def  print_record_to_file(r,file)
    file.printf("%1i,"  ,r['id'])
    file.printf("%-25s,",r['ts'].strftime())
    file.printf("%i,"   ,r['ts'].to_time.to_i) # print UNIX time stamp, for convenience
    file.printf("%-25s,",r['country'])
    file.printf("%-25s,",r['state'])
    file.printf("%-25s,",r['county'])
    file.printf("%7i,"  ,r['totc'])
    file.printf("%6i,"  ,r['newc'])
    file.printf("%6i,"  ,r['totd'])
    file.printf("%6i,"  ,r['newd'])
    file.printf("%7i,"  ,r['totr'])
    file.printf("%7i,"   ,r['ac' ])
    file.printf("%7i,"  ,r['serc'])
    file.printf("%6i\n" ,r['cpm' ])
  end
  
#------------------------------------------------------------------------------
# parse local .html ==> .txt
# use yesterday's table - it seems to be more stable in time during the day,
# today's results are strongly dependent on the state reporting times..
# output file basename is the same as the input file basename
# assume that the input file names don't have an extra '.' in them
# Mar 18 and after:
#  'now' and 'yesterday' navigation bars,
#  data table id's: 'main_table_countries_today' and 'main_table_countries_yesterday'
#------------------------------------------------------------------------------
  def parse_v1(doc, of, version, debug)
#------------------------------------------------------------------------------
# parse yesterday's table data
# it looks that if I use '//td', then all rows in all tables in the list are returned
# expect only one table
#------------------------------------------------------------------------------
    time_stamp = parse_time_stamp(doc,debug,of).prev_day();
    table      = doc.xpath("//div[@class='container']/div/div/div/div/div/table[@id='main_table_countries_yesterday']");

    printf("ntables: %i\n",table.length)

    print_header(of)
    rows = table.xpath('tbody/tr')
    for row in rows
      cells         = row.xpath('td')
      r = parse_cells(cells,time_stamp,version)
      print_record_to_file(r,of);
    end
    
    of.puts(table) if (debug > 0)
    return ;
  end

#------------------------------------------------------------------------------
# v2: 2020 Mar 08 - Mar 17
# parse local .html ==> .txt
# use today's table - only that is available
# output file basename is the same as the input file basename
# assume that the input file names don't have an extra '.' in them
#------------------------------------------------------------------------------
  def parse_v2(doc, of, version, debug)
    time_stamp = parse_time_stamp(doc,debug,of)
#------------------------------------------------------------------------------
# parse today's table data
# it looks that if I use '//td', then all rows in all tables in the list are returned
#------------------------------------------------------------------------------
    table      = doc.xpath("//table[@id='main_table_countries']");

    if (table.length != 1) then
      printf("ERROR: ntables: %i, expected 1\n",table.length)
    end

    of.printf("------------- //table TODAY\n") if (debug > 0);
    
    print_header(of)
    rows = table.xpath('tbody/tr')
    for row in rows
      cells = row.xpath('td')
      r     = parse_cells(cells,time_stamp,version);
      print_record_to_file(r,of);
    end

    of.puts(table) if (debug > 0)
    return ;
  end

#------------------------------------------------------------------------------
# parse local .html ==> .txt
# use yesterday's table - it seems to be more stable in time during the day,
# today's results are strongly dependent on the state reporting times..
# output file basename is the same as the input file basename
# assume that the input file names don't have an extra '.' in them
# for Feb 29 - Mar 07 
#------------------------------------------------------------------------------
  def parse_v3(doc, of, version, debug)
#------------------------------------------------------------------------------
# parse today's table data
# it looks that if I use '//td', then all rows in all tables in the list are returned
#------------------------------------------------------------------------------
    time_stamp = parse_time_stamp(doc,debug,of)
    table      = doc.xpath("//div/table[@id='main_table_countries']");

    printf("ntables: %i\n",table.length)
    of.printf("------------- //table TODAY\n") if (debug > 0);

    print_header(of)
    rows = table.xpath('tbody/tr')
    for row in rows
      cells = row.xpath('td')
      r     = parse_cells(cells,time_stamp,version);
      print_record_to_file(r,of)
    end
    
    of.puts(table) if (debug > 0)
    return ;
  end

#------------------------------------------------------------------------------
# v4: Feb 02 - Feb 28 2020 : id='table3', totals need to be calculated
# on Feb 03 columns 'tot_recovered' 'active_cases' 'serious_cases' were added
# parse local .html ==> .txt
# use yesterday's table - it seems to be more stable in time during the day,
# today's results are strongly dependent on the state reporting times..
# output file basename is the same as the input file basename
# assume that the input file names don't have an extra '.' in them
#------------------------------------------------------------------------------
  def parse_v4(doc, of, version, debug)
#------------------------------------------------------------------------------
# determine the time stamp of the page update
#------------------------------------------------------------------------------
    time_stamp = parse_time_stamp(doc,debug,of)
    
    of.puts("-------------------- //end of divisions") if (debug > 0);
#------------------------------------------------------------------------------
# parse yesterday's table data
# it looks that if I use '//td', then all rows in all tables in the list are returned
#------------------------------------------------------------------------------
    table = doc.xpath("//div/table[@id='table3']");
    
    printf("ntables: %i\n",table.length)

    of.printf("------------- //table TODAY\n") if (debug > 0);

    print_header(of)
    rows = table.xpath('tbody/tr')

    rtot            = {}
    rtot['id']      = 1;
    rtot['country'] = 'total';
    rtot['state']   = 'total'
    rtot['county']  = 'total'
    rtot['totc']    = 0;
    rtot['newc']    = 0;
    rtot['totd']    = 0;
    rtot['newd']    = -1;
    rtot['totr']    = -1;
    rtot['ac'  ]    = -1;
    rtot['serc']    = -1;
    rtot['cpm' ]    = -1
    rtot['ts'  ]    = time_stamp;

    for row in rows
      cells = row.xpath('td')
      r = parse_cells(cells,time_stamp,version);
      print_record_to_file(r,of);

      if (r['totc'] >= 0) then rtot['totc'] += r['totc']; end
      if (r['newc'] >= 0) then rtot['newc'] += r['newc']; end
      if (r['totd'] >= 0) then rtot['totd'] += r['totd']; end
      if (r['newd'] >= 0) then
        if (rtot['newd'] < 0) then rtot['newd']  = r['newd'];
        else                       rtot['newd'] += r['newd'];
        end
      end
      if (r['totr'] >= 0) then
        if (rtot['totr'] < 0) then rtot['totr']  = r['totr'];
        else                       rtot['totr'] += r['totr'];
        end
      end
      
      if (r['ac'  ] >= 0) then
        if (rtot['ac'] < 0) then rtot['ac']  = r['ac'];
        else                     rtot['ac'] += r['ac'];
        end
      end
    end

    print_record_to_file(rtot,of);
    
    of.puts(table) if (debug > 0)
    return ;
  end

#------------------------------------------------------------------------------
# v5: 2020 Jan 29 - Feb 02 : id='table3', 5 cells, UTC  instead of GMT
# 'f' : output file
#------------------------------------------------------------------------------
  def parse_v5(doc, of, version, debug)
    time_stamp = parse_time_stamp(doc,debug,of)

    of.puts("-------------------- //end of divisions") if (debug > 0);
#------------------------------------------------------------------------------
# parse yesterday's table data
# it looks that if I use '//td', then all rows in all tables in the list are returned
#------------------------------------------------------------------------------
    table = doc.xpath("//div/table[@id='table3']");
    
    printf("ntables: %i\n",table.length)

    f.printf("------------- //table TODAY\n") if (debug > 0);

    print_header(of)
    rows = table.xpath('tbody/tr')

    rtot            = {}
    rtot['id']      = 1;
    rtot['country'] = 'total';
    rtot['state']   = 'total'
    rtot['county']  = 'total'
    rtot['totc']    = 0;
    rtot['newc']    = 0;
    rtot['totd']    = 0;
    rtot['newd']    = 0;
    rtot['totr']    = -1;
    rtot['ac'  ]    = -1;
    rtot['serc']    = -1;
    rtot['cpm' ]    = -1
    rtot['ts'  ]    = time_stamp;

    for row in rows
      cells = row.xpath('td')
      r = parse_cells(cells,time_stamp,version);
      print_record_to_file(r,of);

      if (r['totc'] >= 0) then rtot['totc'] += r['totc']; end
      if (r['newc'] >= 0) then rtot['newc'] += r['newc']; end
      if (r['totd'] >= 0) then rtot['totd'] += r['totd']; end
      if (r['newd'] >= 0) then rtot['newd'] += r['newd']; end
      if (r['totr'] >= 0) then rtot['totr'] += r['totr']; end
      if (r['ac'  ] >= 0) then rtot['ac'  ] += r['ac'  ]; end
    end

    print_record_to_file(rtot,of);
    
    of.puts(table) if (debug > 0)
    return ;
  end

#------------------------------------------------------------------------------
  def parse(input_fn,debug = 0)
    
    input_file = File.open(input_fn);
    doc        = Nokogiri::HTML.parse(input_file)
    input_file.close();

    if (debug == 0) then output_fn = @output_dir+'/txt/'+File.basename(input_fn,".*")+'.txt';
    else                 output_fn = File.basename(input_fn,".*")+'_debug.xml';
    end

    of = File.open(output_fn,"w");

    # determine version
    version = -1

    ts = DateTime.parse(File.basename(input_fn,".*").split('_')[0]);

    if (ts.year == 2020) then
      if (ts.month == 3) then
        if    (ts.day >= 24) then version = 6 # one more column (comments) added
        elsif (ts.day >= 18) then version = 1
        elsif (ts.day >=  7) then version = 2
        else                      version = 3
        end
      elsif (ts.month == 2) then
        if    (ts.day >= 29) then version = 3
        elsif (ts.day >=  3) then version = 4;
        else                      version = 5;
        end
      elsif (ts.month == 1) then  version = 5;
      end
    end

    if    (version == 1) then parse_v1(doc,of,version,debug)
    elsif (version == 2) then parse_v2(doc,of,version,debug)
    elsif (version == 3) then parse_v3(doc,of,version,debug)
    elsif (version == 4) then parse_v4(doc,of,version,debug)
    elsif (version == 5) then parse_v5(doc,of,version,debug)
    elsif (version == 6) then parse_v1(doc,of,version,debug)
    end

    of.close();
    
  end

end
