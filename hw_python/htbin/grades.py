#!/usr/bin/env python

print "Content-type: text/html"
print
print "<h1>Grades</h1>"
print "<p>"
print 
file = open("grades.txt", 'r')
for line in file:
	if line[0] == '#':
		print "<h2>%s</h2>" %line	
	else:
		print "<br>%s</br>" %line

print "</p>"
file.close();
