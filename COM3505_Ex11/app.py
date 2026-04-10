from flask import Flask, render_template, request, jsonify

app = Flask(__name__)

latest_data = {}

@app.route('/')
def hello_world():
    return render_template("index.html")

@app.route('/data', methods=['POST'])
def receive_data():
    global latest_data
    latest_data = request.get_json()
    print(latest_data)
    return "OK"

@app.route('/get_data')
def get_data():
    return jsonify(latest_data)

if __name__ == '__main__':
    app.run(debug=True)
