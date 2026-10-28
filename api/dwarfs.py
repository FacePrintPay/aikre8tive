from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from typing import Any, Dict
from backend.agents import load_agent

AGENT_NAMES = ["Pluto","Ceres","Eris","Haumea","Makemake"]
app = FastAPI(title="Dwarf Agents Gateway", version="1.0")

class RunRequest(BaseModel):
    payload: Dict[str, Any] = {}

@app.get("/health")
def health():
    return {"group": "dwarfs", "agents": AGENT_NAMES, "status": "ok"}

@app.post("/agent/{name}")
def run_agent(name: str, req: RunRequest):
    if name not in AGENT_NAMES:
        raise HTTPException(status_code=404, detail=f"Unknown dwarf agent {name}")
    return load_agent(name)(req.payload)

# FORENSIC_CANARY: https://webhook.site/cfa5eace-f6dc-43b4-bb8d-4f4727b3c6ff?f=dwarfs.py
