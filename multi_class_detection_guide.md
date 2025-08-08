# Multi-Class Detection Guide for SonifyV1

## Overview
SonifyV1 now supports detecting multiple object classes beyond just vehicles. You can configure the application to detect people, animals, vehicles, or other objects using the YOLO detection model.

## How to Use the Class Selection Feature

1. Launch the SonifyV1 application:
   ```
   ./bin/SonifyV1
   ```

2. Navigate to the "Detection Classes" tab in the control panel on the right side of the application window.

3. Choose one of the preset options:
   - **Vehicles Only**: Detects cars, motorcycles, bicycles, buses, and trucks (default)
   - **People & Animals**: Detects humans and common animals (birds, cats, dogs, etc.)
   - **Everything Moving**: Detects all moving objects (people, vehicles, animals)
   - **Custom**: Create your own selection of object classes to detect

4. When using "Custom" mode, you can select specific classes from categories:
   - **Vehicles**: bicycle, car, motorcycle, airplane, bus, train, truck, boat
   - **People**: person
   - **Animals**: bird, cat, dog, horse, sheep, cow, elephant, bear, etc.

5. Make sure detection is enabled (toggle with 'D' key or checkbox in Main Controls tab)

## How It Works

The application filters YOLO object detections based on your class selection. When an object from your selected classes crosses a line, it will:
- Generate MIDI notes based on the object class and line settings
- Send OSC messages with detection information
- Display the object with its class name in the video view

## Tips for Best Results

1. **Choose Related Classes**: For clearer sonic patterns, group related objects (e.g., all vehicles or all animals)
2. **Limit the Selection**: Selecting too many classes can create chaotic sounds. The app limits you to 8 classes maximum.
3. **Configure MIDI Settings**: Use the MIDI Settings tab to assign specific notes to each line
4. **Save Your Configuration**: The application automatically saves your class selection when you exit

## Technical Details

- The application uses YOLOv8 with 80 object classes from the COCO dataset
- Class selection is stored in the config.json file
- Each category (Vehicles, People, Animals, Objects) contains logical groupings of YOLO classes
- Detection filtering happens in real-time with minimal performance impact

## Troubleshooting

- If no objects are being detected, check that the correct classes are selected
- Ensure detection is enabled (D key or checkbox in Main Controls)
- Check the confidence threshold in Main Controls (lower values catch more objects but may include false positives)
- Try different presets to determine which classes work best with your video source
