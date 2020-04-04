###############################################################################
#
###############################################################################
require 'watir'

def test1() 
  browser = Watir::Browser.new
  browser.goto('http://stackoverflow.com/')
  
  puts browser.title
  browser.close
end

#-----------------------------------------------------------------------------
def print_elements(list,title)
  puts "#{title} : total number of elements = #{list.size}"
  for i in 0..list.size-1
    t = list[i];
    puts "------------------------------- element #{title}[#{i}]"
    puts t
    end
end

#------------------------------------------------------------------------------
def parse_number(text)
  number = 0
  
  txt = text.sub(',','').sub(/\n+\Z/,'').sub(/\t+\Z/,'').rstrip.lstrip;

  if (txt != '') then number = txt.to_i end

  return number
end

#------------------------------------------------------------------------------
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

