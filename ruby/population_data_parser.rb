# coding: utf-8
#------------------------------------------------------------------------------
# the available data on https://www.worldometers.info/coronavirus/country/us/ starts from Mar 09
# 2020 Mar 19 -       : v1
# 2020 Mar 18         : v2
# 2020 Mar 17 and earlier: only total for the country, do manually
#------------------------------------------------------------------------------
require 'watir'
require 'nokogiri'
require 'getoptlong'

require 'local_tools.rb'
#------------------------------------------------------------------------------
class PopulationDataParser
  def initialize()
    @name       = "population_data_parser";
    @url        = 'https://www.worldometers.info/world-population/population-by-country/'
    @country    = 'world'
    @output_dir = '/projects/covid19/data/population';
    @verbose    = nil
    @sleep_time = 0
  end

#------------------------------------------------------------------------------
  def parse_command_line() 

    opts = GetoptLong.new(
      [ "--sleep_time"    , "-s",        GetoptLong::REQUIRED_ARGUMENT ],
      [ "--url"           , "-u",        GetoptLong::REQUIRED_ARGUMENT ],
      [ "--verbose"       , "-v",        GetoptLong::NO_ARGUMENT       ]
    )

    opts.each do |opt, arg|
      if    (opt == "--sleep_time")
        @sleep_time = arg.to_i
      elsif (opt == "--url"       )
        @url  = arg
      elsif (opt == "--verbose"   )
        @verbose = 1
      end
      
      if @verbose ; puts "Option: #{opt}, arg #{arg.inspect}" ; end
    end
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
# table format:
# id country population yearly_change net_change pop_density_per_km2 land_area_km2 migrants fertility_rate median_age fr_urban_pop_fr
#------------------------------------------------------------------------------
  def parse_cells(cells,time_stamp, version)

    ncells = cells.length
    # printf(" PopulatioinDataParser::parse_cells: version = %i, ncells = %i\n",version,ncells)

    r            = {}
    r['id']      = 1;
    r['ts'  ]    = time_stamp;
    r['country'] = cells[1].text;
    r['state'  ] = 'total'
    r['county' ] = 'total'
    r['pop'    ] = parse_number(cells[ 2].text,'f')/1.0e6; # population, M
    r['nkm2'   ] = parse_number(cells[ 5].text,'i');       # population density per 
    r['km2'    ] = parse_number(cells[ 6].text,'i');       # area, km2
    r['agem'   ] = parse_number(cells[ 9].text,'i');       # median age
    r['furb'   ] = parse_number(cells[10].text,'f')/100.0;
    
    printf("cells[2].text = %s r['pop'] = %i [cells[5].text = %s cells[10].text = %s, r['furb']=%6.3f\n",
           cells[ 2].text,r['pop'], cells[ 5].text,cells[10].text,r['furb'])

    return r;
  end
  
#------------------------------------------------------------------------------
# print data record to a file
#------------------------------------------------------------------------------
  def print_header(file)
    file.print("rid,ts,country,state,county,pop,nkm2,skm2,agem,furb\n");
  end

#------------------------------------------------------------------------------
  def print_record_to_file(r,file)
    file.printf("%1i,"  ,r['id'])
    file.printf("%-25s,",r['ts'].strftime())
    # file.printf("%i,"   ,r['ts'].to_time.to_i) # print UNIX time stamp, for convenience
    file.printf("%-30s,",r['country'])
    file.printf("%-10s,",r['state'])
    file.printf("%-20s,",r['county'])
    file.printf("%8.1f,",r['pop'])
    file.printf("%9i,"  ,r['nkm2'])
    file.printf("%9i,"  ,r['km2' ])
    file.printf("%9.0f,",r['agem'])
    file.printf("%9.3f" ,r['furb'])
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
    time_stamp = DateTime.now;         
    tables     = doc.xpath("//table[@id='example2']");

    if (debug > 0) then printf("<PopulationDataParser::parse_v1> ntables: %i\n",tables.length) end

    for table in tables
      
      if (debug > 0) then
        of.printf("------------- //table ID: %s\n",table['id'])
        of.puts(table)
      end
      
      if (table['id'] == 'example2') then
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
    version = 1

    if    (version == 1) then parse_v1(doc,of,version,debug)
    end

    of.close();
  end

end
#------------------------------------------------------------------------------
# one can run this script
#------------------------------------------------------------------------------
  if __FILE__ == $0 then
    x = ENV["RUBYLIB"];
    #  puts "RUBYLIB = #{x} "
  
    p = PopulationDataParser.new();
    p.parse_command_line();
    p.fetch_url(@url,@sleep_time);
  end
