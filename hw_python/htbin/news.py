#!/usr/bin/env python

import requests
print "Content-type: text/html"
print
print "<h1>News</h1>"
print "<p>"
r = requests.get("http://news.google.com")
print r.content
print "</p>"

