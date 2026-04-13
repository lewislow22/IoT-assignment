from flask import Flask, render_template, request, jsonify

app = Flask(__name__)

latest_data = {}
led_pattern = "checker"

@app.route('/')
def index():
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

@app.route('/set_led', methods=['POST'])
def set_LEDs():
    global led_pattern
    led_pattern = request.form['submit_button']

    return render_template("index.html")

@app.route('/get_led')
def get_led():
    return jsonify({"pattern": led_pattern})



if __name__ == '__main__':
    app.run(debug=True)
