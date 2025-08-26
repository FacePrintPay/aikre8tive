from fastapi import FastAPI

app = FastAPI(title="AiKre8tive Meta-Gateway", version="1.0")

@app.get("/health")
def health():
    return {
        "groups": ["core","giants","moons","dwarfs"],
        "routes": {
            "core": "/api/core/agent/{name}",
            "giants": "/api/giants/agent/{name}",
            "moons": "/api/moons/agent/{name}",
            "dwarfs": "/api/dwarfs/agent/{name}"
        }
    }
