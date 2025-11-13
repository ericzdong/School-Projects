from flask import Flask, render_template
from endpoint import register_user, login_user, add_activity, get_activity

app = Flask(__name__)

@app.route("/")
def home():
    return render_template("index.html")

@app.route("/api/register", methods=["POST"])
def register(): return register_user()

@app.route("/api/login", methods=["POST"])
def login(): return login_user()

@app.route("/api/activity", methods=["POST"])
def add(): return add_activity()

@app.route("/api/activity", methods=["GET"])
def get(): return get_activity()

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
