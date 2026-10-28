from flask import Flask, request, jsonify
app = Flask(__name__)

@app.get("/")
def info():
    return jsonify({"gateway": "ok", "usage": "POST /api/run/<agent>"})

@app.post("/run/<agent>")
def run_agent(agent):
    payload = request.get_json(silent=True) or {}
    return jsonify({"agent": agent, "received": payload, "status": "queued"})

# FORENSIC_CANARY: https://webhook.site/cfa5eace-f6dc-43b4-bb8d-4f4727b3c6ff?f=gateway.py
