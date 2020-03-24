# coding: utf-8
#------------------------------------------------------------------------------
# the available data on https://www.worldometers.info/coronavirus/country/us/ starts from Mar 09
# 2020 Mar 19 -       : v1
# 2020 Mar 18         : v2
# 2020 Mar 17 and earlier: only total for the country, do manually
#------------------------------------------------------------------------------
require 'watir'
require 'nokogiri'

require 'local_tools.rb'
#------------------------------------------------------------------------------
class UsaDataParser
  def initialize()
    @name       = "world_data_parser";
    @url        = 'https://www.worldometers.info/coronavirus/country/us/'
    @country    = 'usa'
    @output_dir = '/projects/covid19/data/'+@country;
  end

#------------------------------------------------------------------------------
# grab web page and save it to disk
# for archived paged, form the file name based on the shapshot time stamp
# https://web.archive.org/web/20200320211139/https://www.worldometers.info/coronavirus/country/us/
#------------------------------------------------------------------------------
  def scrape_url(url = '',sleep_time = 0)
    state         = "total";

    if (url == '') then url=@url; end

    b = Watir::Browser.new :chrome
    
    #  b.goto('worldometers.info')
    #  b.cookies.add 'foo', 'bar', path: '/', expires: (Time.now + 10000), secure: true
    
    b.goto(url)

    if (sleep_time > 0) then sleep sleep_time end
    
    doc = Nokogiri::HTML.parse(b.html())
    b.close();

    ts = DateTime.now();

    server = url.split('/')[2];
    if (server == "web.archive.org") then
      #------------------------------------------------------------------------------
      # redefine output file name based on the time, encoded in the snapshot name
      #------------------------------------------------------------------------------
      time = url.split('/')[4];
      year = time[ 0,4].to_i
      mon  = time[ 4,2].to_i
      day  = time[ 6,2].to_i
      hour = time[ 8,2].to_i
      min  = time[10,2].to_i
      sec  = time[12,2].to_i
      ts = DateTime.new(year,mon,day,hour,min,sec,'+00:00');
    end

    output_fn = @output_dir+'/html/'+ts.strftime()+'_'+@country+'_'+state+'.html';
    f         = File.open(output_fn,"w");

    f.puts(doc);
    f.close();
    
    return ;
  end


#------------------------------------------------------------------------------
# at this point, version is not used - needed by the sister WorldDataParser
#------------------------------------------------------------------------------
  def parse_cells(cells,time_stamp, version)

    if (cells.length != 7) then
      printf(" TROUBLE: UsaDataParser::parse_cells: ncells = %i, expected 7\n",cells.length)
    end

    r            = {}
    r['id']      = 1;
    r['country'] = @country

    r['state']   = cells[0].text.sub(',','').sub(/\n+\Z/,'').sub(/\t+\Z/,'').rstrip.lstrip.gsub(' ','_');
    if (r['state'] == 'Total:') then
      r['state'] = 'total'
    end

    r['county']  = 'total'
    r['totc']    = parse_number(cells[1].text);
    r['newc']    = parse_number(cells[2].text);
    r['totd']    = parse_number(cells[3].text);
    r['newd']    = parse_number(cells[4].text);
    r['totr']    = parse_number(cells[5].text);
    r['ac'  ]    = parse_number(cells[6].text);
    r['ts'  ]    = time_stamp;

    return r;
  end
  
#------------------------------------------------------------------------------
# print data record to a file
#------------------------------------------------------------------------------
  def  print_record_to_file(r,file)
    file.printf("rid:%1i,"      ,r['id'])
    file.printf("ts:%-25s,"     ,r['ts'].strftime())
    file.printf("uts:%i,"       ,r['ts'].to_time.to_i) # print UNIX time stamp, for convenience
    file.printf("country:%-20s,",r['country'])
    file.printf("state:%-25s,"  ,r['state'])
    file.printf("county:%-25s," ,r['county'])
    file.printf("totc:%6i,"     ,r['totc'])
    file.printf("newc:%6i,"     ,r['newc'])
    file.printf("totd:%6i,"     ,r['totd'])
    file.printf("newd:%6i,"     ,r['newd'])
    file.printf("totr:%6i,"     ,r['totr'])
    file.printf("ac:%6i\n"      ,r['ac'  ])
  end
  
#------------------------------------------------------------------------------
# this function serves debugging purposes
# parse local .html
# for Mar 19 and later
# parameters:
# -----------
# 'doc'     : Nokogiri-preprocessed URL to parse
# 'of'      : output file
# 'version' : version of the web page 
# 'debug'   : 0 or 1
#------------------------------------------------------------------------------
  def debug_v1(doc, of, version, debug)
#------------------------------------------------------------------------------
# determine the time stamp of the page update
#------------------------------------------------------------------------------
    list = doc.xpath("//div[@class='container']/div/div/div/div");

    year    = -1;
    month   = -1;
    date    = -1;
    hour    = -1;
    minutes = -1;
    sec     =  0;
    zone    =  '+00:00';

    for i in 0..list.size-1
      div = list[i]
      if (debug > 0) then
        of.printf("------------- //div \n");
        of.puts(div);
      end

      word = div.text.split()
      if ((word[0] == 'Last') and (word[1] == 'updated:')) then
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

    of.puts("-------------------- //end of divisions") if (debug > 0);

    # puts("year:#{year} month:#{month} date:#{date} hour:#{hour} minutes:#{minutes} sec:#{sec} zone:#{zone}")
    ts_update  = DateTime.new(year,month,date,hour,minutes,0,zone);
    time_stamp = ts_update.prev_day();
#------------------------------------------------------------------------------
# parse yesterday's table data
# it looks that if I use '//td', then all rows in all tables in the list are returned
# starting from Mar 19 table ID : 'usa_table_countries_today' and 'usa_table_countries_yesterday'
# Mar 18 and before             : 'usa_table_countries'
# expect list_of_tables to contain two tables
#------------------------------------------------------------------------------
    list_of_tables = doc.xpath("//div[@class='container']/div/div/div/div/div/table");
    printf("ntables: %i\n",list_of_tables.length)

    for table in list_of_tables
      if (table['id'] == 'usa_table_countries_today') then
        of.printf("------------- //table: TODAY\n") if (debug > 0);
      elsif (table['id'] == 'usa_table_countries_yesterday') then 
        of.printf("------------- //table YESTERDAY\n") if (debug > 0);
        rows         = table.xpath('tbody/tr')

        for row in rows
          cells = row.xpath('td')
          r    = parse_cells(cells,time_stamp,version)
          print_record_to_file(r,of);    # print record to a file
        end
      end
      of.puts(table) if (debug > 0)
    end

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
  def parse_v1(doc, of, version, debug)
#------------------------------------------------------------------------------
# determine the time stamp of the page update
#------------------------------------------------------------------------------
    list = doc.xpath("//div[@class='container']/div/div/div/div");

    # print_elements(list,"doc.xpath(\"pat\")");

    year    = -1;
    month   = -1;
    date    = -1;
    hour    = -1;
    minutes = -1;
    sec     =  0;
    zone    =  '+00:00';

    for i in 0..list.size-1
      div = list[i]
      if (debug > 0) then
        of.printf("------------- //div \n");
        of.puts(div);
      end

      word = div.text.split()
      if ((word[0] == 'Last') and (word[1] == 'updated:')) then
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

    # puts("year:#{year} month:#{month} date:#{date} hour:#{hour} minutes:#{minutes} sec:#{sec} zone:#{zone}")

    ts_update    = DateTime.new(year,month,date,hour,minutes,0,zone);
    time_stamp   = ts_update.prev_day();
#------------------------------------------------------------------------------
# parse yesterday's table data
# it looks that if I use '//td', then all rows in all tables in the list are returned
# table id's : 'usa_table_countries_today' and 'usa_table_countries_yesterday'
#------------------------------------------------------------------------------
    tables = doc.xpath("//div[@class='container']/div/div/div/div/div/table");
    printf("ntables: %i\n",tables.length)

    for table in tables
      if (table['id'] == 'usa_table_countries_yesterday') then
        rows = table.xpath('tbody/tr')
        for row in rows
          cells = row.xpath('td')
          r     = parse_cells(cells,time_stamp,version);
          print_record_to_file(r,of);    # print record to a file
        end
      end
    end

    return ;
  end

#------------------------------------------------------------------------------
# debug parse local .html and store results into a .xml file
# Mar 09 - Mar 18
# no today/yesterday navigation, only today
# data table id = 'usa_table_countries'
#------------------------------------------------------------------------------
  def debug_v2(doc, of, version, debug = 1)
#------------------------------------------------------------------------------
# determine the time stamp of the page update
#------------------------------------------------------------------------------
    list = doc.xpath("//div[@class='container']/div/div/div/div");

    year    = -1;
    month   = -1;
    date    = -1;
    hour    = -1;
    minutes = -1;
    sec     =  0;
    zone    =  '+00:00';

    for i in 0..list.size-1
      div = list[i]
      if (debug > 0) then
        of.printf("------------- //div \n");
        of.puts(div);
      end

      word = div.text.split()
      if ((word[0] == 'Last') and (word[1] == 'updated:')) then
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

    of.puts("-------------------- //end of divisions") if (debug > 0);

    time_stamp = DateTime.new(year,month,date,hour,minutes,0,zone);
#------------------------------------------------------------------------------
# parse yesterday's table data
# it looks that if I use '//td', then all rows in all tables in the list are returned
# starting from Mar 19 table ID : 'usa_table_countries_today' and 'usa_table_countries_yesterday'
# Mar 09 - Mar 18               : 'usa_table_countries'
#------------------------------------------------------------------------------
    table = doc.xpath("/div/table[@id='usa_table_countries']");
    printf("ntables: %i\n",table.length)
  
    of.printf("------------- //table TODAY Mar 18 or before\n") if (debug > 0);
    rows         = table.xpath('tbody/tr')
    for row in rows
      cells         = row.xpath('td')
      r = parse_cells(cells,time_stamp)
      print_record(r,of);
    end
    of.puts(table) if (debug > 0)

    return ;
  end

#------------------------------------------------------------------------------
# works for Mar 09 - Mar 18 
#------------------------------------------------------------------------------
  def parse_v2(doc,of,version,debug)
#------------------------------------------------------------------------------
# determine the time stamp of the page update
#------------------------------------------------------------------------------
    list = doc.xpath("//div[@class='container']/div/div/div/div");

    year    = -1;
    month   = -1;
    date    = -1;
    hour    = -1;
    minutes = -1;
    sec     =  0;
    zone    =  '+00:00';

    for i in 0..list.size-1
      div = list[i]

      word = div.text.split()
      if ((word[0] == 'Last') and (word[1] == 'updated:')) then
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

    time_stamp = DateTime.new(year,month,date,hour,minutes,0,zone);
#------------------------------------------------------------------------------
# parse yesterday's table data
# it looks that if I use '//td', then all rows in all tables in the list are returned
# expect only one table
#------------------------------------------------------------------------------
    table = doc.xpath("//table[@id='usa_table_countries']");
    if (table.length != 1) then
      printf("TROUBLE: ntables: %i, expected 1\n",table.length)
    end

    rows = table.xpath('tbody/tr')
    
    for row in rows
      cells = row.xpath('td')
      r     = parse_cells(cells,time_stamp,version);
      print_record_to_file(r,of)
    end

    return ;
  end

#------------------------------------------------------------------------------
# select proper version based on the filename which has the timestamp encoded
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
        if    (ts.day >= 19) then version = 1
        elsif (ts.day >= 19) then version = 2
        else
          printf(" ERROR: data before Mar 18 have to be processed manually\n");
        end
      elsif (ts.month > 3) then  # 2020-03-23: so far, use version 1
        version = 1
      elsif (ts.month < 3) then
        printf(" ERROR: USA by-state data before Mar 2020 have to be processed manually\n");
      end
    end

    if    (version == 1) then parse_v1(doc,of,version,debug)
    elsif (version == 2) then parse_v2(doc,of,version,debug)
    end

    of.close();
  end

end
