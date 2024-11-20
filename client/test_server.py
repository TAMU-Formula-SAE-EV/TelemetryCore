from client_connection import ClientConnection
                
from flask import Flask, render_template, request, jsonify

app = Flask(__name__)

entries = []

def on_rx(head, content):
    entries.append({'head': head.decode(), 'content': content.decode()})

client = ClientConnection(on_rx)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/get-latest/<int:last_n>', methods=['GET'])
def get_latest(last_n):
    return jsonify(entries[-last_n:])

@app.route('/post-data', methods=['POST'])
def post_data():
    data = request.get_json()
    print(data)
    client.send_packet('command', data['data'])
    return jsonify({'status': 'ok'})

if __name__ == '__main__':
    client.connect('localhost', 8080)
    app.run(debug=True)