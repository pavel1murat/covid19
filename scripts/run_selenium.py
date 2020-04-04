#!/bin/python

import os, argparse

from selenium import webdriver
from selenium.webdriver.chrome.options import Options

chrome_options = Options()
chrome_options.add_argument("--headless")

parser = argparse.ArgumentParser();
parser.add_argument("-u","--url"   , dest="url"         , default="none" , help="URL to grab");
args   = parser.parse_args();
url    = args.url.strip()

driver = webdriver.Chrome(options=chrome_options)

# url = "http://www.alabamapublichealth.gov/infectiousdiseases/2019-coronavirus.html"

# url="http://www.dph.illinois.gov/topics-services/diseases-and-conditions/diseases-a-z-list/coronavirus"

print(url)

driver.get(url)

print(driver.page_source)
