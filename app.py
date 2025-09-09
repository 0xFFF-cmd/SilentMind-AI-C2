from flask import Flask
from server.routes.decision import decision_bp
from server.routes.upload import upload_bp

app = Flask(__name__)
app.register_blueprint(decision_bp, url_prefix="/api")
app.register_blueprint(upload_bp, url_prefix="/api")

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8000)
