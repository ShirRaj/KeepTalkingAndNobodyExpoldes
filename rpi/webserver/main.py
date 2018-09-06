from flask import Flask, url_for, render_template

app = Flask(__name__)

with app.test_request_context():
    url_for('static', filename="*")

@app.route('/')
def index():
    return render_template('index.html')

