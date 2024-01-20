# flask run --port=8888 --app=server.py
# ab -n 1000 'http://127.0.0.1:8888/'
# curl -v 'http://127.0.0.1:8888/'

from flask import Flask

app = Flask(__name__)

@app.route("/")
def hello_world():
    return "<p>Hello, World!</p>"
