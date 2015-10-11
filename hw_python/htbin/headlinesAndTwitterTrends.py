#!/usr/bin/env python

import requests
import threading
from bs4 import BeautifulSoup

class Shared:
    """ Shared Content"""
    def __init__(self):
	self.content = [] * 2
	self.sem = threading.Semaphore()
	self.lock = threading.Lock()

    def append(self, link, index):
	""" append content """
	self.sem.acquire()
	self.content[index] = (self.content)[index] + "<h2>%s</h2>"%link
	c = self.content
	self.sem.release()
	return c

class Google(threading.Thread):
    """ first thread. """
    def __init__(self, shared):
        threading.Thread.__init__(self)
	self.shared = shared

    def run(self):
	r = requests.get("https://news.google.com")
	css_soup = BeautifulSoup(r.content, "html.parser")
	for link in css_soup.find_all('span', {"class":"titletext"}):
		with self.shared.lock:
			c = self.shared.append(link, 0)

class Twitter(threading.Thread):
     """ Second thread."""
     def __init__(self, shared):
	threading.Thread.__init__(self)
	self.shared = shared

     def run(self):
	r = requests.get("https://twitter.com/whatstrending")
	css_soup = BeautifulSoup(r.content, "html.parser")
	for link in css_soup.find_all('p', {"class":"tweet-text"}):
		with self.shared.lock:
			c = self.shared.append(link, 1)

if __name__ == "__main__":
    threads = []
    c = Shared()
    thread1 = Google(c)
    threads.append(thread1)
    thread2 = Twitter(c)
    threads.append(thread2)
    for t in threads:
        t.start()
    for t in threads:
        t.join()
  
    print "Content-type: text/html"
    print
    print "<h1>Headlines</h1>"
    print "<p>"
    for line in c.content[0]:
	print "%s" %line
    print "</p>"
"""
    print "Content-type: text/html"
    print
    print "<h1>Twitter Trends</h1>"
    print "<p>"
    print "<h2>%s</h2>"%c.content[1]
    print "</p>" 
    print "</div>
"""

