

#!/usr/bin/env python
import BaseHTTPServer, json, subprocess, urllib

HOST_NAME = 'localhost'
PORT_NUMBER = 2048

solver = subprocess.Popen("./solver.exe", shell=False, \
                          stdin=subprocess.PIPE, \
                          stdout=subprocess.PIPE, \
                          stderr=subprocess.PIPE)

class MyHandler(BaseHTTPServer.BaseHTTPRequestHandler):
  def do_GET(s):
    action = s.path.split("?")[0]
    if action == "/":
      res = ''
      res += 'var script = document.createElement("script");\n'
      res += 'script.src = "http://%s:%d/autoplayer.js";\n' % (HOST_NAME, PORT_NUMBER)
      res += 'document.body.appendChild(script);\n'
      s.send_response(200)
      s.send_header("Content-type", "text/javascript")
      s.end_headers()
      s.wfile.write(res)
    elif action == "/move/":
      query = s.path.split("?")[1]
      params = dict(qc.split("=") for qc in query.split("&"))
      state = urllib.unquote(params["state"])
      solver.stdin.write(state + "\n")
      outp = solver.stdout.readline().replace("\n", "")
      s.send_response(200)
      s.send_header("Content-type", "text/javascript")
      s.end_headers()
      if "callback" in params: s.wfile.write('%s("%s");' % (params["callback"], outp))
      else: s.wfile.write(outp)
      #print state, outp
    elif action == "/autoplayer.js":
      f = open("autoplayer.js", "r")
      js = f.read()
      js = js.replace("<<<HOST>>>", HOST_NAME)
      js = js.replace("<<<PORT>>>", str(PORT_NUMBER))
      s.send_response(200)
      s.send_header("Content-type", "text/javascript")
      s.end_headers()
      s.wfile.write(js)
    else:
      s.send_response(404)
  def log_message(self, format, *args):
    return

if __name__ == '__main__':
  server_class = BaseHTTPServer.HTTPServer
  httpd = server_class((HOST_NAME, PORT_NUMBER), MyHandler)
  httpd.verbose = 0
  try:
    httpd.serve_forever()
  except KeyboardInterrupt:
    pass
  httpd.server_close()
