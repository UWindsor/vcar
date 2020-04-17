from flask import Flask, request, render_template
import json
import os

app = Flask(__name__)

@app.route('/', methods=['GET', 'POST'])
def index():
    if request.method == 'GET':
        return render_template('index.html')
        
    frame = request.get_json()['frame']
    cmd = "cansend vcar " + frame

    exit = os.system(cmd)

    return json.dumps(
        {
            'success': True if exit == 0 else False,
            'exit_message': exit
        }
    )

if __name__ == '__main__':
    app.run()
