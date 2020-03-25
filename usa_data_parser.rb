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
    @country    = 'USA'
    @output_dir = '/projects/covid19/data/'+@country;
  end

#------------------------------------------------------------------------------
# fetch web page and save it to disk
# for archived paged, form the file name based on the shapshot time stamp
# https://web.archive.org/web/20200320211139/https://www.worldometers.info/coronavirus/country/us/
#------------------------------------------------------------------------------
  def fetch_url(url = '',sleep_time = 0)
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
# determine the time stamp of the last update
#------------------------------------------------------------------------------
  def parse_time_stamp(doc,debug,file)
    year    = -1;
    month   = -1;
    date    = -1;
    hour    = -1;
    minutes = -1;
    sec     =  0;
    zone    =  '+00:00';

    list = doc.xpath("//div[@class='container']/div/div/div/div");

    for i in 0..list.size-1
      div = list[i]

      if (debug > 0) then
        file.printf("------------- //div \n");
        file.puts(div);
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
    
    file.puts("-------------------- //end of divisions") if (debug > 0);

    return DateTime.new(year,month,date,hour,minutes,0,zone);
  end


#------------------------------------------------------------------------------
# at this point, version is not used - needed by the sister WorldDataParser
#------------------------------------------------------------------------------
  def parse_cells(cells,time_stamp, version)

    ncells = cells.length
    if (version < 3) and (ncells != 7) then
      printf(" TROUBLE: UsaDataParser::parse_cells: ncells = %i, version = %i, expected 7\n",version,ncells)
    elsif (version == 3) and (ncells != 6) then
      printf(" TROUBLE: UsaDataParser::parse_cells: ncells = %i, version = %i, expected 6\n",version,ncells)
    end

    r            = {}
    r['id']      = 1;
    r['ts'  ]    = time_stamp;
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
    
    r['ac'  ]    = -1;
    if (ncells > 6) then
      r['ac'  ]    = parse_number(cells[6].text);
    end

    return r;
  end
  
#------------------------------------------------------------------------------
# print data record to a file
#------------------------------------------------------------------------------
  def print_header(file)
    file.print("rid,ts,uts,country,state,county,totc,newc,totd,newd,totr,ac\n");
  end

#------------------------------------------------------------------------------
  def print_record_to_file(r,file)
    file.printf("%1i,"  ,r['id'])
    file.printf("%-25s,",r['ts'].strftime())
    file.printf("%i,"   ,r['ts'].to_time.to_i) # print UNIX time stamp, for convenience
    file.printf("%-20s,",r['country'])
    file.printf("%-25s,",r['state'])
    file.printf("%-25s,",r['county'])
    file.printf("%6i,"  ,r['totc'])
    file.printf("%6i,"  ,r['newc'])
    file.printf("%6i,"  ,r['totd'])
    file.printf("%6i,"  ,r['newd'])
    file.printf("%6i,"  ,r['totr'])
    file.printf("%6i"   ,r['ac'  ])
    file.printf("\n");
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
# parse yesterday's table data
# it looks that if I use '//td', then all rows in all tables in the list are returned
# table id's : 'usa_table_countries_today' and 'usa_table_countries_yesterday'
#------------------------------------------------------------------------------
    time_stamp = parse_time_stamp(doc,debug,of).prev_day();
    tables     = doc.xpath("//div[@class='container']/div/div/div/div/div/table");
    printf("ntables: %i\n",tables.length)

    for table in tables
      
      if (debug > 0) then
        of.printf("------------- //table ID: %s\n",table['id'])
        of.puts(table)
      end
      
      if (table['id'] == 'usa_table_countries_yesterday') then
        print_header(of)
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
# parse yesterday's table data
# it looks that if I use '//td', then all rows in all tables in the list are returned
# starting from Mar 19 table ID : 'usa_table_countries_today' and 'usa_table_countries_yesterday'
# Mar 09 - Mar 18               : 'usa_table_countries'
#------------------------------------------------------------------------------
    time_stamp = parse_time_stamp(doc,debug,of);
    table      = doc.xpath("/div/table[@id='usa_table_countries']");
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

    time_stamp = parse_time_stamp(doc,debug,of);
#------------------------------------------------------------------------------
# parse yesterday's table data
# it looks that if I use '//td', then all rows in all tables in the list are returned
# expect only one table
#------------------------------------------------------------------------------
    table = doc.xpath("//table[@id='usa_table_countries']");
    if (table.length != 1) then
      printf("TROUBLE: ntables: %i, expected 1\n",table.length)
    end

    print_header(of)
    
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
        if    (ts.day >= 24) then version = 3
        elsif (ts.day >= 19) then version = 1
        elsif (ts.day >= 18) then version = 2
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
    elsif (version == 3) then parse_v1(doc,of,version,debug)
    end

    of.close();
  end

end
