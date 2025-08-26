from fastapi import FastAPI, Body
from backend.agents import load_agent

app = FastAPI(title="AiKre8tive Gateway")

@app.get("/api/health")
def health():
    return {"ok": True, "service": "gateway"}

@app.post("/api/run/{agent}")
def run_agent(agent: str, payload: dict = Body(default={})):
    fn = load_agent(agent)
    return {"agent": agent, "result": fn(payload)}
