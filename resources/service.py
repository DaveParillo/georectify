import cherrypy
import process

import string


p = process.Transform()
cherrypy.__version__ = ''
cherrypy._cperror._HTTPErrorTemplate = cherrypy._cperror._HTTPErrorTemplate.replace('Powered by <a href="http://www.cherrypy.org">CherryPy %(version)s</a>\n','%(version)s')

class service(object):

   @cherrypy.expose
   def index(self):
      return "\nPOST to /process to make transformation requests to the service"


   @cherrypy.expose
   @cherrypy.tools.json_out()
   @cherrypy.tools.json_in()
   def process(self):
      data = cherrypy.request.json
      output = p.run(data)
      return output

if __name__ == '__main__':
    config = {
            'server.socket_host': '0.0.0.0',
            'response.headers.server': 'TBD',
            'request.show_tracebacks': False
           }
    cherrypy.config.update(config)
    config = conf = {
        '/': {
            'tools.sessions.on': False,
            'tools.response_headers.on': True,
        }
    }
    cherrypy.quickstart(service(), '/', config)

