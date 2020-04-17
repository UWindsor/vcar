from flask import Flask, request, render_template
import json
import os

app = Flask(__name__)

@app.route('/', methods=['GET', 'POST'])
def send_can_message():
  if request.method == 'GET':
    return render_template('index.html')
  else:
    frame = request.form['frame']
    cmd = "cansend vcar " + frame

    exit = os.system(cmd)

    return json.dumps(
      {
        'success': True if exit == 0 else False,
        'exit_message': exit
      }
    )
  
