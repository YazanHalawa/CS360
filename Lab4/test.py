#!/usr/bin/env python
import socket

def main():
    data = "GET / HTTP/1.1\r\nHost: gunicorn.org\r\n\r\n"
    try:
        from http_parser.parser import HttpParser
    except ImportError:
        from http_parser.pyparser import HttpParser

    p = HttpParser()
    nparsed = p.execute(data,len(data))
    print p.get_method(),p.get_path(),p.get_headers()

if __name__ == "__main__":
    main()