"""
A TCP echo server that handles multiple clients with polling. Typing Control-C
will quit the server.
"""

import argparse

from web import Poller

class Main:
	""" a class to parse the command line arguments and run the server. """
	def __init__(self):
		self.parseArguments()

	def parseArguments(self):
		""" parse arguments, which include '-p' for port, and 'd' for debug. """

		parser = argparse.ArgumentParser(prog='Web Server', description='A simple web server that handles one client at a time', add_help =True)
		parser.add_argument('-p', '--port', type=int, action='store', help='the port number to bind the server to', default=8080)
		parser.add_argument('--debug', action='store_true', help='print debug messages')
		self.args = parser.parse_args()

	def run(self):
		poller = Poller(self.args.port, self.args.debug)
		poller.run()
def __name__ == "__main__":
	main_ = Main()
	main_.parse_arguments()
	try:
		main_.run()
	except KeyboardInterrupt:
		pass