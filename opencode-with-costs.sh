#!/bin/bash

# opencode launcher with token and cost tracking for Kimi
# Usage: ./opencode-with-costs.sh

echo "🚀 Starting opencode with Kimi token & cost tracking..."
echo "📊 Token usage and costs will be displayed in real-time"
echo "🤖 Using Moonshot AI (Kimi)"
echo ""

# Set environment variables for cost tracking with Kimi
export OPENCODE_SHOW_TOKENS=true
export OPENCODE_SHOW_COSTS=true
export OPENCODE_TRACK_USAGE=true
export OPENCODE_LOG_LEVEL=info
export OPENCODE_PROVIDER=moonshot
export OPENCODE_MODEL=moonshotai/kimi-k2-0711-preview

# Launch opencode with project context and Kimi settings
/Users/justmac/.opencode/bin/opencode --project-dir="$(pwd)" --show-tokens --show-costs --provider=moonshot --model=moonshotai/kimi-k2-0711-preview

echo ""
echo "💰 Kimi cost tracking enabled!"
echo "📈 Check session summary for total tokens and costs"
echo "💡 Kimi pricing: $0.0005 input / $0.0015 output per 1K tokens"