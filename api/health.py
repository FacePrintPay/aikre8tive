from flask import Flask, jsonify
app = Flask(__name__)

@app.get("/")
def root():
    return jsonify({"ok": True, "service": "aikre8tive"})

# FORENSIC_CANARY: https://webhook.site/cfa5eace-f6dc-43b4-bb8d-4f4727b3c6ff?f=health.py
