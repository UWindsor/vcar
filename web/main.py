from flask import Flask
from flask import request
import ipdb
import json
import os

app = Flask(__name__)

@app.route('/', methods=['POST'])
def hello_world():
  frame = request.get_json()['frame']
  cmd = "cansend vcar " + frame

  exit = os.system(cmd)

  output_body = {

  }

  return json.dumps(
    {
      'success': True if exit == 1 else False,
      'exit_message': exit
    }
  )

