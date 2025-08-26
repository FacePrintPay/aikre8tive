from flask import Flask, request, jsonify

app = Flask(__name__)

@app.get("/")
def info():
    return jsonify({"gateway": "ok", "usage": "POST /api/run/<agent>", "example": "/api/run/Mercury"})

@app.post("/run/<agent>")
def run_agent(agent):
    payload = request.get_json(silent=True) or {}
    # Stubbed: here you’d route to your backend.agents.<Agent>.main(...)
    return jsonify({"agent": agent, "received": payload, "status": "queued"})
