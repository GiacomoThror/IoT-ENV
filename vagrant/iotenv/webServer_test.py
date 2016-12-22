from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer


class webServerHandler(BaseHTTPRequestHandler):

    def do_GET(self):

        if self.path.endswith("/iotenv"):

                self.send_response(200)
                self.send_header('Content-type', 'text/html')
                self.end_headers()

                output = ""
                output += "<html><body>"
                output += "<h1>IoT EnV Test</h1>"
                output += "<ul>"
                output += "</body></html>"

                self.wfile.write(output)

                print output

                return


def main():

    try:
        port = 8080
        server = HTTPServer(('', port), webServerHandler)
        print "Web Server running on port %s" % port
        server.serve_forever()
    except KeyboardInterrupt:
        print " ^C entered, stopping web server...."
        server.socket.close()

if __name__ == '__main__':
    main()
