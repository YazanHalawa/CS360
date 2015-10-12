#!/usr/bin/env python

import requests
import threading
from bs4 import BeautifulSoup

class Shared:
    """ Shared Content"""
    def __init__(self):
	self.content = []
	self.sem = threading.Semaphore()
	self.lock = threading.Lock()

    def append(self, link):
	""" append content """
	self.sem.acquire()
	self.content.append(link)
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
			c = self.shared.append("G<h2>%s</h2>"%link)

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
			c = self.shared.append("T<h2>%s</h2>"%link)

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

    firstThread = []
    secondThread = []
    for line in c.content:
        if line[0] == 'G':
            firstThread.append(line[1:])
        else:
            secondThread.append(line[1:])

    print "Content-type: text/html"
    print
    print "<h1>Headlines and Twitter Trends</h1>"
    print "<p>"
    print "<div style=\"width:45%;float:left;\">"
    for line in firstThread:
        print "<br>%s</br>"%line
    print "</div>"
    print "<div style=\"width:45%;float:right;\">"
    for line in secondThread:
        print "<br>%s</br>"%line
    print "</div>"


