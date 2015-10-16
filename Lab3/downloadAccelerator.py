import argparse
import sys
import os
import requests
import threading
import time

class DownloadAccelerator:
    def __init__(self):
        self.args = None
        self.threads = 1
        self.url = 'http://www.google.com'
        self.dir = None

        self.parse_arguments()

    def parse_arguments(self):
        parser = argparse.ArgumentParser(prog='DownloadAccelerator', description='HTTP downloader using concurrent threads', add_help=True)
        parser.add_argument('-n', '--threads', type=int, action='store', help='Specify the number of threads to create',default=1)
        parser.add_argument("URL", type=str, action='store', help='Specify the url of the web server', default="Google.com")
        
        args = parser.parse_args()
        self.threads = args.threads
        self.url = args.URL
        self.dir = "downloads"
        self.filename = self.dir + '/' + self.url.split('/')[-1].strip()

        if not os.path.exists(self.dir):
            os.makedirs(self.dir)

    def download(self):
        r = requests.head(self.url)
        content_length = r.headers['content-length']
        num_bytes = int(content_length)/self.threads

        threads = []
        start = 0
        end = 0

        for i in range(0, self.threads):
            if (i == 0):
                start = i * num_bytes
            else:
                start = (i*num_bytes)+i
            if (i < self.threads - 1):
                end = start + num_bytes
            else:
                end = int(content_length)

            d = DownloadThread(start, end, self.url)
            threads.append(d)

        firstTime = time.time()

        for t in threads:
            t.start()

        with open(self.filename, 'wb') as f:
            for t in threads:
                t.join()
                f.write(t.file_contents)
            f.close()

        elapsed = (time.time() - firstTime)

        sys.stdout.write(self.url + " ")
        sys.stdout.write("%d " % self.threads)
        sys.stdout.write(content_length + " ")
        sys.stdout.write("%s " % elapsed + "\n")

class DownloadThread(threading.Thread):
    def __init__(self, start, end, url):
        self.url = url
        self.start_byte = start
        self.end_byte = end
        self.num_bytes = end - start
        self.file_contents = None
        threading.Thread.__init__(self)
        self._content_consumed = False

    def run(self):
        r = requests.get(self.url, headers = {'Accept-Encoding': '', 'Range': 'bytes=%d-%d'%(self.start_byte, self.end_byte)})
        self.file_contents = r.content


if __name__ == '__main__':
    d = DownloadAccelerator()
    d.download()

