#!/bin/bash

# Real-time cost monitoring for opencode sessions
# Usage: ./monitor-costs.sh

echo "🔍 opencode Cost Monitor"
echo "========================"
echo ""

# Check if opencode is running
if pgrep -f "opencode" > /dev/null; then
    echo "✅ opencode is currently running"
    echo ""
    
    # Show current session info
    echo "📊 Current Session Stats:"
    echo "Project: $(basename $(pwd))"
    echo "Directory: $(pwd)"
    echo ""
    
    # Show recent usage from database
    if [ -f ~/.opencode/opencode.db ]; then
        echo "📈 Recent Usage (last 5 sessions):"
        sqlite3 ~/.opencode/opencode.db "SELECT datetime(timestamp, 'unixepoch'), model, input_tokens, output_tokens, total_cost FROM sessions ORDER BY timestamp DESC LIMIT 5;" 2>/dev/null || echo "No session data found"
    fi
    
else
    echo "❌ opencode is not currently running"
    echo ""
    echo "To start with cost tracking:"
    echo "  ./opencode-with-costs.sh"
fi

echo ""
echo "💡 Kimi Cost Reference:"
echo "  Kimi K2 0711 Preview: $0.0005 input / $0.0015 output per 1K tokens"
echo "  Kimi K2 Turbo Preview: $0.0003 input / $0.0006 output per 1K tokens"