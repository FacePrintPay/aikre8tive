#include <crow_all.h>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <future>
#include <mutex>
#include <optional>
#include <queue>
#include <string>



#include <thread>
#include <unordered_map>
#include <fstream>
#include <condition_variable>
#include <array>
using namespace std;
namespace fs = std::filesystem;

struct Job {
    string id, agent, payload, stdout_s, stderr_s, status = "queued";
    int exit_code = -1;
};
class JobStore {
    mutex m_;
    unordered_map<string, Job> jobs_;
public:
    string put(Job j){ lock_guard<mutex> lk(m_); auto id=j.id; jobs_.emplace(id, move(j)); return id; }
    optional<Job> get(const string& id){ lock_guard<mutex> lk(m_); auto it=jobs_.find(id); if(it==jobs_.end()) return nullopt; return it->second; }
    void update(const Job& j){ lock_guard<mutex> lk(m_); jobs_[j.id]=j; }
};
class Queue {
    mutex m_; condition_variable cv_; queue<string> q_;
public:
    void push(const string& id){ { lock_guard<mutex> lk(m_); q_.push(id);} cv_.notify_one(); }
    string pop(){ unique_lock<mutex> lk(m_); cv_.wait(lk,[&]{return !q_.empty();}); auto id=q_.front(); q_.pop(); return id; }
};
static string run_cmd_capture(const string& cmd, int& exit_code){
    array<char,4096> buf{}; string out; FILE* pipe=popen(cmd.c_str(),"r");
    if(!pipe){ exit_code=-1; return ""; }
    while(fgets(buf.data(), (int)buf.size(), pipe)) out+=buf.data();
    exit_code = pclose(pipe);
    return out;
}
int main(){
    const string repo_root = fs::current_path().string();
    const fs::path agents_dir = fs::path(repo_root) / "backend" / "agents";
    unordered_map<string,string> allow = {
        {"Sun","Sun.py"},{"Mercury","Mercury.py"},{"Venus","Venus.py"},{"Earth","Earth.py"},{"Mars","Mars.py"},
        {"Jupiter","Jupiter.py"},{"Saturn","Saturn.py"},{"Uranus","Uranus.py"},{"Neptune","Neptune.py"},
        {"Pluto","Pluto.py"},{"Luna","Luna.py"},{"Ceres","Ceres.py"},{"Haumea","Haumea.py"},{"Makemake","Makemake.py"},
        {"Eris","Eris.py"},{"Io","Io.py"},{"Europa","Europa.py"},{"Ganymede","Ganymede.py"},{"Callisto","Callisto.py"},
        {"Titan","Titan.py"},{"Enceladus","Enceladus.py"},{"Triton","Triton.py"},{"Charon","Charon.py"},
        {"Phobos","Phobos.py"},{"Deimos","Deimos.py"}
    };
    atomic<uint64_t> seq{1}; JobStore store; Queue queue;

    auto worker = [&]{
        for(;;){
            auto id = queue.pop();
            auto jopt = store.get(id); if(!jopt) continue; auto job=*jopt;
            auto it=allow.find(job.agent);
            if(it==allow.end()){ job.status="error"; job.stderr_s="Unknown agent"; store.update(job); continue; }
            fs::path agent_path = agents_dir / it->second;
            if(!fs::exists(agent_path)){ job.status="error"; job.stderr_s="Agent not found"; store.update(job); continue; }
            job.status="running"; store.update(job);
            fs::path tmp = fs::path(repo_root)/("tmp_payload_"+job.id+".json");
            { ofstream ofs(tmp); ofs<<job.payload; }
            string cmd = string("python3 \"")+agent_path.string()+"\" < \""+tmp.string()+"\" 2>&1";
            int code=-1;
            packaged_task<string()> task([&]{ return run_cmd_capture(cmd, code); });
            auto fut = task.get_future();
            thread(move(task)).detach();
            if(fut.wait_for(chrono::seconds(60))==future_status::timeout){
                job.status="timeout"; job.exit_code=124; job.stderr_s="agent execution timed out (60s)"; store.update(job);
            }else{
                job.stdout_s=fut.get();
                #ifdef WIFEXITED
                  job.exit_code = (WIFEXITED(code)? WEXITSTATUS(code): code);
                #else
                  job.exit_code = code;
                #endif
                job.status = (job.exit_code==0? "done":"error");
                store.update(job);
            }
            error_code ec; fs::remove(tmp, ec);
        }
    };
    thread(worker).detach(); thread(worker).detach();

    crow::SimpleApp app;
    CROW_ROUTE(app, "/health")([]{
        crow::json::wvalue j; j["ok"]=true; j["service"]="aikre8tive-gateway-cpp"; return j;
    });
    CROW_ROUTE(app, "/run/<string>").methods(crow::HTTPMethod::Post)
    ([&](const crow::request& req, crow::response& res, const string& agent){
        if(req.body.empty()){ res.code=400; res.write(R"({"error":"empty body"})"); return res.end(); }
        Job j; j.id=to_string(seq.fetch_add(1)); j.agent=agent; j.payload=req.body;
        store.put(j); queue.push(j.id);
        crow::json::wvalue out; out["job_id"]=j.id; out["agent"]=j.agent; out["status"]=j.status;
        res.code=202; res.write(crow::json::dump(out)); res.end();
    });
    CROW_ROUTE(app, "/jobs/<string>")([&](const string& id){
        auto jopt = store.get(id); if(!jopt) return crow::response(404, R"({"error":"not found"})");
        const auto& j=*jopt; crow::json::wvalue out;
        out["id"]=j.id; out["agent"]=j.agent; out["status"]=j.status; out["exit_code"]=j.exit_code;
        out["stdout"]=j.stdout_s; out["stderr"]=j.stderr_s; return crow::response{crow::json::dump(out)};
    });

    const char* port_env = getenv("PORT"); int port = port_env? atoi(port_env): 8080;
    app.port((uint16_t)port).multithreaded().run();
}
