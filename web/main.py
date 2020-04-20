from flask import Flask, request, render_template, redirect
from flask_cors import CORS, cross_origin
from flask_socketio import SocketIO
import json
import os

app = Flask(__name__)

app.config['SECRET_KEY'] = 'the quick brown fox jumps over the lazy   dog'
app.config['CORS_HEADERS'] = 'Content-Type'
cors = CORS(app, resources={r"/": {"origins": "http://localhost:5000"}})

socketio = SocketIO(app);

log = []

@app.route('/hook', methods=['POST'])
def hook():
    log.insert(0, request.form['message']);
    socketio.emit('message', request.form);
    return json.dumps({'ok': True});

@app.route('/', methods=['GET', 'POST'])
@cross_origin(origin='localhost',headers=['Content- Type','Authorization'])
def send_can_message():
    if (request.method == 'GET'):
        return render_template('index.html', data=log);
    else:
        frame = request.form['frame']
        cmd = "cansend vcar " + frame + ">/dev/null 2>&1"
        exit = os.system(cmd)

        return json.dumps(
            {
                'success': True if exit == 0 else False,
                'exit_message': exit
            }
        )

if __name__ == '__main__':
    socketio.run(app)
