#!/usr/bin/env python

import socket
import sys
import errno
import select
import os
import stat
import re
import time
import traceback

class Poller:
	""" A Polling Server """

	def __init__(self, port, debug):
		""" Do the required initialization to run the Server. """
		# declare the class' data members
		self.host = ""
		self.port = port
		self.debug = debug
		self.cache = {} # array of separate client caches
		self.clients = {}
		self.lastUsed = {} #store last time the request was made
		self.media = {}
		self.hosts = {}
		self.validMethods = {"GET", "POST", "DELETE", "HEAD", "PUT"} #allowed methods in HTTP request
		self.parameters = {}
		self.timeout = 1
		self.size = 10000
		self.configFile = "tests/web.conf"

		self.open_socket()
		self.read_config()

	def read_config(self):
		if self.debug:
			print "reading the configuration file: %s---------\n"%self.configFile

		reader = open(self.configFile, 'r')
		for line in reader.readlines():
			if (len(line) > 1): # if not a blank line

				wordsInLine = line.split() # split the line into words using the white space delimieter
				
				if wordsInLine[0] == "host":
					
					pathToHost = wordsInLine[2] 
					if pathToHost[0] != '/': #path is a local file
						pathToHost = os.getcwd() + '/' + pathToHost # acquire full path
						self.hosts[wordsInLine[1]] = pathToHost

				elif wordsInLine[0] == "media":
					self.media[wordsInLine[1]] = wordsInLine[2]

				elif wordsInLine[0] == "parameter":
					self.parameters[wordsInLine[1]] = wordsInLine[2]
					if wordsInLine[1] == "timeout":
						self.timeout = wordsInLine[2]
				
				else:
					if self.debug:
						print "not valid header type in config file: %s\n" %wordsInLine[0]

			if self.debug:
				print "\nhosts: \n", self.hosts
				print "\nmedia: \n", self.media
				print "\nparameters: \n", self.parameters
				print "\ntimeout: ", self.timeout

			reader.close()

	def open_socket(self):
		""" Setup the socket for incoming clients. """
		if (self.debug):
			print "entering open_socket-------\n"

		try:
			self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			self.server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
			self.server.bind((self.host, self.port))
			self.server.listen(5)
			self.server.setblocking(0)
		except socket.error, (value,message):
			if self.server:
				self.server.close()
			print "Could not open socket:" + message
			sys.exit(1)

	def run(self):
		""" User poll() to handle each incoming client. """
		if (self.debug):
			print "entering run--------\n"

		self.poller = select.epoll()
		self.pollmask = select.EPOLLIN | select.EPOLLHUP | select.EPOLLERR
		self.poller.register(self.server, self.pollmask)

		if (self.debug):
			print "in run before loop\n"
		while True:
			# poll sockets
			try:
				fds = self.poller.poll(timeout=1)
			except:
				if (self.debug):
					print "failed to poll on client\n"
				return


			for (fd, event) in fds:
				# handle errors
				if event & (select.POLLHUP | select.POLLERR):
					self.handleError(fd)
					continue
				# handle the server socket
				if fd == self.server.fileno():
					self.handleServer()
					continue
				# handle client socket
				result = self.handleClient(fd)
				self.lastUsed[fd] = time.time()


			now = time.time()
			toDelete = []
			for socket in self.lastUsed:
				if float(now-self.lastUsed[socket]) > float(self.timeout):
				# close the socket
					toDelete.append(socket)
			for socket in toDelete:
				self.closeSocket(socket)
			toDelete = []

	def handleError(self,fd):
		""" Handle a client """
		if (self.debug):
			print "entering handle Error-------\n"

		self.poller.unregister(fd)
		if fd == self.server.fileno():
			# recreate server socket
			self.server.close()
			self.open_socket()
			self.poller.register(self.server,self.pollmask)
		else:
			# close the socket
			self.closeSocket(fd)

	def handleServer(self):
		if (self.debug):
			print "entering handleServer-------\n"
		# accept as many clients as possible
		try:
			(client,address) = self.server.accept()
			self.lastUsed[client] = time.time()
		except socket.error, (value,message):
			# if socket blocks because no clients are available,
			# then return
			if value == errno.EAGAIN or value == errno.EWOULDBLOCK:
				if (self.debug):
					print "%s\n" %value
				return
			print traceback.format_exc()
			sys.exit()
		# set client socket to be non blocking
		client.setblocking(0)
		self.clients[client.fileno()] = client
		self.poller.register(client.fileno(), self.pollmask)

	def handleClient(self, fd):
		if (self.debug):
			print "entering handle Client--------\n"



		try:
			data = self.clients[fd].recv(self.size)

			if (self.debug):
				print "recieved %s from client\n" %data

			# Add the data to the cache corresponding to the current running client
			if fd in self.cache:
				self.cache[fd] += data
			else:
				self.cache[fd] = data

			# If data was received
			if data:
				if (self.debug):
					print "data is: %s\n"%data
				if '\r\n\r\n' in self.cache[fd]: # check if request is complete
					(response, path, isRangeReq) = self.parseRequest(self.cache[fd])
					self.sendResponse(fd, response, path, isRangeReq)
					if (self.debug):
						print "before delete\n"
					del self.cache[fd]
					if (self.debug):
						print "after delete\n"
					return
				else:
					if (self.debug):
						print "received data not end\n"

			else:
				if (self.debug):
					print "Client request was not GET\n"
				self.poller.unregister(fd)
				self.closeSocket(fd)
				return

		except socket.error, (value, message):
			# if no data is available, move on to another client
			if value == errno.EAGAIN or value == errno.EWOULDBLOCK:
				if (self.debug):
					print "%s\n" %message
					print "Got EAGAIN or EWOULDBLOCK in handle client\n"
				return

	def sendResponse(self, fd, response, path, isRangeReq):
		""" Function to send Response back to client. """
		if (self.debug):
			print "entering send Response\n"
		
		while True:
			try:
				self.clients[fd].send(response)
				break
			except socket.error, e:
				if e.args[0] == errno.EAGAIN or e.args[0] == errno.EWOULDBLOCK:
					if self.debug:
						print "Socket error when sending to client\n"
					continue

		responseCode = response.split()[1]
		if path and responseCode == "200":
			f = open(path, 'rb')

			while True:
				if (self.debug):
					print "in loop\n"
				message = f.read(self.size)
				if not message:
					break

				amountSent = 0
				while amountSent < len(message):
					try:
						currSent = self.clients[fd].send(message[amountSent:])
					except socket.error, e:
						if e.args[0] == errno.EAGAIN or e.args[0] == errno.EWOULDBLOCK:
							if self.debug:
								print "error while sending to client\n"
							continue
					amountSent += currSent
			f.close()

		#Extra credit, handle range requests
		if path and responseCode == "206":
			self.sendRangeResponse(fd, response, path, isRangeReq)

		if (self.debug):
			print "passed send response\n"

	def sendRangeResponse(self, fd, response, path, isRangeReq):
		""" Function to send range response. """
		if (self.debug):
			print "entered send Range Response. "

		(start, end, diff) = self.splitRangeRequest(isRangeReq)

		if (self.debug):
			print "start: ", start
			print "end: ", end
			print "diff: ", diff

		reader = open(path, 'rb')
		bytesRead = ''
		totalBytesLeft = diff
		totalBytesRead = 0

		# Do the Reading
		while (totalBytesRead < totalBytesLeft):

			reader.seek(start + totalBytesRead, 0)

			if (diff < self.size):
				bytesRead = reader.read(diff)
				totalBytesRead += len(bytesRead)
				diff = 0

			else:
				bytesRead = reader.read(self.size)
				totalBytesRead += len(bytesRead)
				diff -= self.size

			if not bytesRead:
				break
		reader.close()
		
		# Do the sending
		totalBytesSent = 0
		while totalBytesSent < len(bytesRead):
			try:
				currSent = self.clients[fd].send(bytesRead[totalBytesSent:])

			except socket.error, e:
				if e.args[0] == errno.EAGAIN or e.args[0] == errno.EWOULDBLOCK:
					if self.debug:
						print "error while sending to client\n"
					continue
			totalBytesSent += currSent

	def closeSocket(self, fd):
		""" Function to close the socket. """
		if (self.debug):
			print "Entering close socket\n"

		if fd in self.clients:
			self.clients[fd].close()
			del self.clients[fd]
		if fd in self.lastUsed:
			del self.lastUsed[fd]
		if fd in self.cache:
			del self.cache[fd]
 
	def parseRequest(self, data):
		if (self.debug):
			print "entering parse Request--------\n"

		# Create a parser object

		try:
    			from http_parser.parser import HttpParser
		except ImportError:
    			from http_parser.pyparser import HttpParser

		parser = HttpParser()
		nparser = parser.execute(data, len(data))

		response = None
		path = None
		host = None
		isRangeReq = None
		isHeadReq = False

		# Get Protocol Version
		version = "HTTP/1.1"

		#check if request is valid
		method = parser.get_method()
		if method not in self.validMethods:
			if (self.debug):
				print "received a non valid method: %s\n" %method
			response = self.createError("400", "Bad Request")

		elif method != "GET" and method != "HEAD":
			if (self.debug):
				print "received a method which we do not implement\n"
			response = self.createError("501", "Not Implemented")
		
		else:
			if method == "HEAD":
				isHeadReq = True
			url = parser.get_path()

			# Check for url errors
			if (url == ""):
				if self.debug:
					print "url is empty\n"
				resposne = self.createError("400", "Bad Request")

			elif (url == "/"):
				url = "/index.html"

			headers = parser.get_headers()

			if "Range" in headers:
				isRangeReq = headers["Range"]
				if self.debug:
					print "Range Request = %s" %isRangeReq

			#get Host
			if "Host" in headers:
				host = headers["Host"].split(':')[0]
				if (self.debug):
					print "host is: %s\n"%host

			# Handle errors in host
			if host not in self.hosts:
				if 'default' not in self.hosts:
					if self.debug:
						print " not host or default\n"
					response = self.createError("400", "Bad Request")

				else:
					# Use the default host
					if self.debug:
						print "using default host\n"

					path = self.hosts['default']

					if (self.debug):
						print "path is: %s\n"%path

					path += url

					if (self.debug):
						print "full path is: %s\n"%path

					response = self.createResponse(path, isRangeReq)

			else: #use given host
				path = self.hosts[host]
				if (self.debug):
					print "path is: %s\n"%path

				path += url

				if (self.debug):
					print "full path is: %s\n"%path

				response = self.createResponse(path, isRangeReq)
		
		if isHeadReq:
			path = None

		if self.debug:
			print "end of parse request\n"

		return response, path, isRangeReq

	def splitRangeRequest(self, rangeReq):
		""" Function to split the range request. """
		if (self.debug):
			print "entered split Range Request------\n"
		
		rangeReq = rangeReq.split('=')
		rangeReq = rangeReq[1].split('-')
		start = int(rangeReq[0])
		end = int(rangeReq[1])
		diff = (end-start) + 1

		return start, end, diff

	def createError(self, errID, errDescr):
		""" Function to Create Error Message. """
		if (self.debug):
			print "entering create Error------\n"

		t = time.time()
		currTime = self.get_time(t)

		htmlErr = '<html> <body> <h1>' + errID + ' ' + errDescr + '</h1> </body> </html>'

		if self.debug:
			print "htmlErr: ", htmlErr

		error = 'HTTP/1.1' + ' ' + errID + ' ' + errDescr + '\r\n'
		error += 'Date: ' + currTime + '\r\n'
		error += 'Server: Apache/2.2.22 (Ubuntu) \r\n'
		error += 'Content-Type: text/html \r\n'
		error += 'Content-Length: ' + str(len(htmlErr)) + '\r\n'
		error += '\r\n'
		error += htmlErr
		error += '\r\n\r\n'

		return error 


	def createResponse(self, path, isRangeReq):
		""" Function to create response messages. """
		if (self.debug):
			print "entering create Response\n"

		pathValidationError = self.verifyPath(path)

		if pathValidationError == None: # No Error

			t = time.time()
			currTime = self.get_time(t)
			fileType = None
			fileExt = path.split('.')[-1]
			# Variables for range request
			start = None
			end = None
			diff = None

			if fileExt in self.media:
				fileType = self.media[fileExt]
			else:
				fileType = 'text/plain'

			if isRangeReq == None:
				response = 'HTTP/1.1 200 OK \r\n'
			else:
				response = 'HTTP/1.1 206 Parial Message \r\n'
				(start, end, diff) = self.splitRangeRequest(isRangeReq)

			response += 'Date: %s\r\n'%currTime
 			response += 'Server: Apache/2.2.22 (Ubuntu)\r\n'
			response += 'Content-Type: %s\r\n'%fileType

			if isRangeReq == None:
				response += 'Content-Length: %s\r\n' %str(os.stat(path).st_size)
			else:
				response += 'Content-Length: %s\r\n'%str(diff)

			response += 'Last-Modified: %s\r\n'%self.get_time(os.stat(path).st_mtime)
			response += '\r\n'
			
			return response
		else:
			return pathValidationError



	def verifyPath(self, path):
		""" Function to verify path and its permissions. """
		if (self.debug):
			print "entering verify path function\n"

		try:
			open(path)
		except IOError as (errno, strerror):
			if errno == 13:
				return self.createError("403", "Forbidden")
			elif errno == 2:
				return self.createError("404", "Not Found")
			else:
				return self.createError("500", "Internal Server Error")
		return None 

	def get_time(self, t):
		""" Function to get time in GMT Format. """
		if (self.debug):
			print "Entering get_time function\n"

		gmtTime = time.localtime(t)
		format = '%a, %d %b %Y %H :%M :%S GMT'
		time_string = time.strftime(format,gmtTime)
		return time_string



