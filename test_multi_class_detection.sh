#!/bin/bash
# Test script for multi-class detection in SonifyV1

echo "Testing multi-class detection in SonifyV1..."

# Check if necessary source files exist
if [ -f "src/category_methods.cpp" ]; then
  echo "✅ Found category_methods.cpp"
else
  echo "❌ Missing category_methods.cpp"
fi

if [ -f "src/detection_classes_tab.cpp" ]; then
  echo "✅ Found detection_classes_tab.cpp"
else
  echo "❌ Missing detection_classes_tab.cpp"
fi

# Compile the project
echo "Compiling SonifyV1..."
make

# Check if compilation was successful
if [ $? -eq 0 ]; then
  echo "✅ Compilation successful!"
  echo "You can now run the application with:"
  echo "./bin/SonifyV1"
  echo ""
  echo "Use the new 'Detection Classes' tab to select which objects to detect."
  echo "Presets available: 'Vehicles Only', 'People & Animals', 'Everything Moving'"
  echo ""
  echo "Remember to toggle detection on with the 'Enable Detection' checkbox or 'd' key."
else
  echo "❌ Compilation failed!"
fi
