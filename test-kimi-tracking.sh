#!/bin/bash

# Test script to verify Kimi token tracking is working
echo "🧪 Testing Kimi token tracking..."
echo "================================"

# Check if Kimi API key is set
if [ -z "$MOONSHOT_API_KEY" ]; then
    echo "⚠️  MOONSHOT_API_KEY not set in environment"
    echo "   Please set it with: export MOONSHOT_API_KEY=your_key_here"
    echo "   Or add to .opencode/.env file"
fi

# Test with a simple query
echo "🔍 Running test query to check token counting..."
echo ""

# Run a simple test
/Users/justmac/.opencode/bin/opencode run "Count the tokens in this simple test message" --model=moonshotai/kimi-k2-0711-preview --show-tokens --show-costs

echo ""
echo "✅ If you see token counts above, Kimi tracking is working!"
echo "💡 The '0 0' at the top should update after your first interaction"