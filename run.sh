#!/data/data/com.termux/files/usr/bin/bash
echo "🌟 aikre8tive"
curl -s http://localhost:3000/api/proxy > /dev/null && echo "✅ PATHOS"
echo "[aikre8tive] $(date)" >> "/data/data/com.termux/files/home/sovereign_gtp/logs/aikre8tive.log"
