# Structure for Software

Last Updated: 6/8/2024
- Try to update once a month during peak development periods

Goals: Extremely modular and easy to test and debug. Ideally, should be extremely optimized too.

## Triple-Stream Approach
3 "Streams" of processing, could be done in two ways (not decided):
1. Asynchronous: (I heard its cancer in C++?) All 3 streams run at the same time, and communicate with each other through share memory
2. Synchronous: Based on an internal system clock, all 3 streams run in sequence.

### Data Input Stream
Receives input from all sensors, cleans and processes them:
1. Keep in an array of `n` most recent data points
2. When data received from IMU, TEMTs, IR Sensors, Mouse Sensors, US Sensors and Camera (might have to deal with this separately),
3. Process data:
   1. Confirm data validity, based on previous data points
   2. If data is not valid, decrease *sensor weight*
   3. If data is valid, process it and increase *sensor weight* slightly
4. Add data to the array of `n` most recent data points, and pop the oldest data point

### Strategy Stream
Responsible for making decisions based on the data from the Data Input Stream, and then sending the commands to the Data Output Stream. 

- Reads from array of data points
- Processes it 
- Makes decisions based on the data, and sends commands to the Data Output Stream
   - Importantly, these decisions should at a higher level than commands to e.g. motors
   - For example, instead of telling motors to rotate at a certain speed, it should indicate the current position and the desired position, and the Data Output Stream should figure out how to get there

Components:
- [Localisation](#Localisation)
- [Strategy](#Strategy)

### Data Output Stream
Responsible for controlling the *motors* and the *kicker.* Includes:

- Receiving current location, target location and whether to kick from the Strategy Stream
- Deciding how to get to the target location in an optimal way
- Sending commands to the motors and the kicker

Components
- [Localisation](#Localisation)

## Localisation
Combining sensor data from IMU, TEMTs, IR Sensors, Mouse Sensors, US Sensors and Camera to determine the current position of the robot.

### Sensor Weight
*\*Note: Camera is a whole different matter, which is not accounted for here.*

Sensors can fail, and this should help to detect when they do. The **sensor weight** information will be used to determine how much to trust the data from a sensor, for stuff like particle filter.

**Symptoms of Failing Sensors:**
1. **Consistency with itself:** Sensor should be consistent within a certain *threshold* of its own recent previous data points
2. **Consistency with other sensors:** Sensor should be consistent with other sensors, e.g. If BNO85 said no acceleration, yet mouse and ultrasonic sensors say the robot is moving, then the BNO85 is probably wrong.

**Sensor Weight:** Each sensor has a weight, which is a number between 0 (non-reliable) and 1 (reliable).
- We start with a weight of 1 for all sensors
- If a sensor fails, we decrease its weight (how? square rooting it, fixed constant etc?)
- If a sensor proves reliable over a long period of time, increase its weight

### Localisation Strategy
- Convex Hull?
- Particle Filter?

## Strategy

**TODO**, LOL

## Movement

**TODO**, LOL