from flask import Flask, url_for, render_template, redirect

app = Flask(__name__)

with app.test_request_context():
    url_for('static', filename="*")


@app.route('/')
def index():
    return render_template('index.html', wires=['red', None, 'red', 'green', None], ready=True, started=True)


@app.route('/start')
def start_time_route():
    start_time()
    return redirect('/')


@app.route('/reset')
def reset_game_route():
    reset_game()
    return redirect('/')


def start_time():
    print "Starting time"


def reset_game():
    print "Resetting game"
