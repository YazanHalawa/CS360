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
		self.media = {}
		self.hosts = {}
		self.validMethods = {"GET", "POST", "DELETE", "HEAD", "PUT"} #allowed methods in HTTP request
		self.parameters = {}
		self.size = 10000
		self.configFile = "web-server-testing/tests/web.conf"

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
			self.server.listen(10)
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
		while True:
			# poll sockets
			try:
				fds = self.poller.poll(timeout=self.timeout)
			except:
				return
			for (fd, event) in fds:
				# handle errors
				if event & (select.POLLHUP | selct.POLLERR):
					self.handleError(fd)
					continue
				# handle the server socket
				if fd == self.server.fileno():
					self.handleServer()
					continue
				# handle client socket
				result = self.handleClient(fd)

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
			self.clients[fd].close()
			del self.clients[fd]

	def handleServer(self):
		if (self.debug):
			print "entering handleServer-------\n"
		# accept as many clients as possible
		try:
			(client,address) = self.server.accept()
		except socket.error, (value,message):
			# if socket blocks because no clients are available,
			# then return
			if value == errno.EAGAIN or errno.EWOULDBLOCK:
				return
			print traceback.format_exc()
			sys.exit()
		# set client socket to be non blocking
		client.setblocking(0)
		self.client[client.fileno()] = client
		self.poller.register(client.fileno(), self.pollmask)

	def handleClient(self, fd):
		if (self.debug):
			print "entering handle Client--------\n"

		while True:

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
					if '\r\n\r\n' in data: # check if request is complete
						(response, path) = self.parseRequest(self.cache[fd])
						self.sendResponse(fd, response, path)
						del self.cahce[fd]
						break
					else:
						continue

				else:
					if (self.debug):
						print "Client request was not GET\n"
					self.poller.unregister(fd)
					self.closeSocket(fd)
					break

			except socket.error, (value, message):
				# if no data is available, move on to another client
				if value == errno.EAGAIN or errno.EWOULDBLOCK:
					if (self.debug):
						print "Got EAGAIN or EWOULDBLOCK in handle client\n"
					break

			if data:
				self.clients[fd].send(data)
			else:
				self.poller.unregister(fd)
				self.clients[fd].close()
				del self.clients[fd]

	def sendResponse(self, fd, response, path):
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
		if responseCode == "200":
			f = open(path, 'rb')

			while True:
				message = f.read(self.size)
				if not message:
					break

				amountSent = 0
				while amoundSent < len(message):
					try:
						currSent = self.clients[fd].send(message[amoundSent:])
					except socket.error, e:
						if e.args[0] == errno.EAGAIN or e.args[0] == errno.EWOULDBLOCK:
							if self.debug:
								print "error while sending to client\n"
							continue
					amountSent += currSent
			f.close()

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

		# Get Protocol Version
		version = "HTTP/" + parser.get_version()[0] + "." + parser.get_version()[1]

		#check if request is valid
		method = parser.get_method()
		if method not in self.validMethods:
			if (debug):
				print "received a non valid method: %s\n" %method
			response = self.createError("400", "Bad Request")

		elif method != "GET":
			if (debug):
				print "received a method which we do not implement\n"
			response = self.createError("501", "Not Implemented")
		
		else:
			url = parser.get_path()

			# Check for url errors
			if (url == ""):
				if self.debug:
					print "url is empty\n"
				resposne = self.createError("400", "Bad Request")

			elif (url == "/"):
				url = "/index.html"
			headers = parser.get_headers()

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

					response = self.createResponse(path)

			else: #use given host
				path = self.hosts[host]
				if (self.debug):
					print "path is: %s\n"%path

				path += url

				if (self.debug):
					print "full path is: %s\n"%path

				response = self.createResponse(path)
		if self.debug:
			print "end of parse request\n"

		return response, path

	def createError(self, errID, errDescr):
		""" Function to Create Error Message. """
		if (self.debug):
			print "entering create Error------\n"

		t = time.time()
		currTime = self.get_time(t)

		htmlErr = '<html> <body> <h1>' + errNum + ' ' + errMsg + '</h1> </body> </html>'

		if self.debug:
			print "htmlErr: ", htmlErr

		error = 'HTTP/1.1' + ' ' + errNum + ' ' + errMsg + '\r\n'
		error += 'Date: ' + currTime + '\r\n'
		error += 'Server: Apache/2.2.22 (Ubuntu) \r\n'
		error += 'Content-Type: text/html \r\n'
		error += 'Content-Length: ' + str(len(htmlErr)) + '\r\n'
		error += '\r\n'
		error += htmlErr
		error += '\r\n\r\n'

		return error 


	def createResponse(self, path):
		""" Function to create response messages. """
		if (self.debug):
			print "entering create Response\n"

		pathValidationError = self.verifyPath(path)

		if pathValidationError == None: # No Error

			t = time.time()
			currTime = self.get_time(t)
			fileType = None
			fileExt = path.split('.')[-1]

			if fileExt in self.media:
				fileType = self.media[fileExt]
			else:
				fileType = 'text/plain'

			response = 'HTTP/1.1 200 OK \r\n'

			response += 'Date: %s\r\n'%currTime
 			response += 'Server: Apache/2.2.22 (Ubuntu)\r\n'
			response += 'Content-Type: %s\r\n'%fileType

			response += 'Content-Length: %s\r\n' %str(os.stat(path).st_size)
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

		gmtTime = time.gmtime(t)
		format = '%a, %d %b %Y %H :%M :%S GMT'
		time_string = time.strftime(format,gmt)
		return time_string



