from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
import cgi

from sqlalchemy import create_engine

from sqlalchemy.orm import sessionmaker
from database_setup import Base, Restaurant, MenuItem

engine = create_engine('sqlite:///restaurantmenu.db')

Base.metadata.bind = engine
DBSession = sessionmaker(bind=engine)
session = DBSession()


class webServerHandler(BaseHTTPRequestHandler):

    def do_GET(self):
        try:
            if self.path.endswith("/edit"):
                restaurantId = self.path.split("/")[2]
                restaurant = session.query(Restaurant).filter_by(id=restaurantId).one()
                self.send_response(200)
                self.send_header('Content-type', 'text/html')
                self.end_headers()
                output = ""
                output += "<html><body>"
                output += "<h1>Rename Restaurant</h1>"

                output += '''<form method='POST' enctype='multipart/form-data' action=/restaurants/%s/edit>''' % restaurantId
                output += '''<input name="edit_restaurant" type="text", placeholder = "%s" ><input type="submit" value="Rename"> </form>''' % restaurant.name
                output += "</body></html>"
                self.wfile.write(output)
                print output

            if self.path.endswith("/delete"):
                restaurantId = self.path.split("/")[2]
                restaurant = session.query(Restaurant).filter_by(id=restaurantId).one()
                self.send_response(200)
                self.send_header('Content-type', 'text/html')
                self.end_headers()
                output = ""
                output += "<html><body>"
                output += "<h1>Are you sure do you want to delete restaurant" + restaurant.name + "? </h1>"

                output += '''<form method='POST' enctype='multipart/form-data' action=/restaurants/%s/delete>''' % restaurantId
                output += '''<input type="submit" value="Delete"> </form>'''
                output += "</body></html>"
                self.wfile.write(output)
                print output

            if self.path.endswith("/restaurants/new"):
                self.send_response(200)
                self.send_header('Content-type', 'text/html')
                self.end_headers()
                output = ""
                output += "<html><body>"
                output += "<h1>Make New Restaurant</h1>"
                output += '''<form method='POST' enctype='multipart/form-data' action='/restaurants/new'><input name="new_restaurant" type="text" ><input type="submit" value="Create"> </form>'''
                output += "</body></html>"
                self.wfile.write(output)
                print output

            if self.path.endswith("/restaurants"):

                items = session.query(Restaurant).all()

                self.send_response(200)
                self.send_header('Content-type', 'text/html')
                self.end_headers()
                output = ""
                output += "<html><body>"
                output += '<a href="/restaurants/new">Make a new restaurant here!</a></br>'
                output += "<h1>Restaurants list:</h1>"

                output += "<ul>"
                for item in items:
                    print(item.name)
                    output += "<li>" + str(item.name) + "</br>"
                    output += "<a href= 'restaurants/%s/edit ''>edit</a></br>" % item.id
                    output += '<a href="restaurants/%s/delete ">delete</a></br></li>' % item.id
                output += "</ul>"

                output += "</body></html>"
                self.wfile.write(output)
                print output

                return

            if self.path.endswith("/hello"):
                self.send_response(200)
                self.send_header('Content-type', 'text/html')
                self.end_headers()
                output = ""
                output += "<html><body>"
                output += "<h1>Hello!</h1>"
                output += '''<form method='POST' enctype='multipart/form-data' action='/hello'><h2>What would you like me to say?</h2><input name="message" type="text" ><input type="submit" value="Submit"> </form>'''
                output += "</body></html>"
                self.wfile.write(output)
                print output
                return

            if self.path.endswith("/hola"):
                self.send_response(200)
                self.send_header('Content-type', 'text/html')
                self.end_headers()
                output = ""
                output += "<html><body>"
                output += "<h1>&#161 Hola !</h1>"
                output += "</body></html>"
                self.wfile.write(output)
                print output
                return

        except IOError:
            self.send_error(404, 'File Not Found: %s' % self.path)

    def do_POST(self):

        try:
            if self.path.endswith("/restaurants/new"):
                ctype, pdict = cgi.parse_header(
                    self.headers.getheader('content-type'))
                if ctype == 'multipart/form-data':
                    fields = cgi.parse_multipart(self.rfile, pdict)
                    messagecontent = fields.get('new_restaurant')

                # INSERT #

                newRestaurant = Restaurant(name=messagecontent[0])

                session.add(newRestaurant)
                session.commit()

                self.send_response(301)
                self.send_header('Content-type', 'text/html')
                self.send_header('Location', '/restaurants')
                self.end_headers()

                return

            if self.path.endswith("/edit"):

                ctype, pdict = cgi.parse_header(
                    self.headers.getheader('content-type'))
                if ctype == 'multipart/form-data':
                    fields = cgi.parse_multipart(self.rfile, pdict)
                    messagecontent = fields.get('edit_restaurant')

                # UPDATE #

                newRestaurant = Restaurant(name=messagecontent[0])
                restaurantId = self.path.split("/")[2]
                restaurant = session.query(Restaurant).filter_by(id=restaurantId).one()
                restaurant.name = messagecontent[0]

                session.add(restaurant)
                session.commit()

                self.send_response(301)
                self.send_header('Content-type', 'text/html')
                self.send_header('Location', '/restaurants')
                self.end_headers()

                return

            if self.path.endswith("/delete"):

                ctype, pdict = cgi.parse_header(
                    self.headers.getheader('content-type'))

                # Delete #

                restaurantId = self.path.split("/")[2]
                restaurant = session.query(Restaurant).filter_by(id=restaurantId).one()
                # restaurant.name = messagecontent[0]

                session.delete(restaurant)
                session.commit()

                self.send_response(301)
                self.send_header('Content-type', 'text/html')
                self.send_header('Location', '/restaurants')
                self.end_headers()

                return

            if self.path.endswith("/hello"):
                self.send_response(301)
                self.send_header('Content-type', 'text/html')
                self.end_headers()
                ctype, pdict = cgi.parse_header(self.headers.getheader('content-type'))
                if ctype == 'multipart/form-data':
                    fields = cgi.parse_multipart(self.rfile, pdict)
                    messagecontent = fields.get('message')
                output = ""
                output += "<html><body>"
                output += " <h2> Okay, how about this: </h2>"
                output += "<h1> %s </h1>" % messagecontent[0]
                output += '''<form method='POST' enctype='multipart/form-data' action='/hello'><h2>What would you like me to say?</h2><input name="message" type="text" ><input type="submit" value="Submit"> </form>'''
                output += "</body></html>"
                self.wfile.write(output)
                print output

                return

        except:
            pass


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

