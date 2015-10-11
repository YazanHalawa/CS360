#!/usr/bin/env python

import requests
from bs4 import BeautifulSoup

print "Content-type: text/html"
print
print "<h1>Headlines</h1>"
print "<p>"
r = requests.get("https://news.google.com")
#soup = bs(r.text.encode('utf-8'))
css_soup = BeautifulSoup(r.content, "html.parser")
for link in css_soup.find_all('span', {"class":"titletext"}):
	print "<h2>%s</h2>" %link
print "</p>"

