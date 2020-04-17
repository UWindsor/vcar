from flask import Flask
from flask import request
import json
import os

app = Flask(__name__)

@app.route('/', methods=['POST'])
def send_can_message():
  frame = request.get_json()['frame']
  cmd = "cansend vcar " + frame

  exit = os.system(cmd)

  return json.dumps(
    {
      'success': True if exit == 0 else False,
      'exit_message': exit
    }
  )
