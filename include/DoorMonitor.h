#ifndef DOOR_MONITOR_H
#define DOOR_MONITOR_H

#include <stdint.h>

// Door state enumeration
enum DoorState {
  DOOR_CLOSED,
  DOOR_OPEN,
  DOOR_STOPPED,
  DOOR_OPENING,
  DOOR_CLOSING,
  DOOR_UNKNOWN,
  DOOR_ERROR_SENSOR_FAILURE,
  DOOR_ERROR_TIMEOUT,
  DOOR_ERROR_STALLED
};

// Acceleration data structure
struct AccelData {
  float x;
  float y;
  float z;
  bool valid;
};

// Door monitor configuration
struct DoorMonitorConfig {
  float accelThreshold;        // m/s^2 threshold for detecting movement
  unsigned long stopTimeout;   // milliseconds without movement = stopped
  unsigned long maxOpenTime;   // maximum time to fully open (timeout detection)
  unsigned long maxCloseTime;  // maximum time to fully close (timeout detection)
  float stallThreshold;        // acceleration threshold for stall detection
  unsigned long stallTimeout;  // time with minimal movement = stalled
  float closedPositionY;       // Y-axis value when door is closed (typically 9.8)
  float closedPositionZ;       // Z-axis value when door is closed (typically 0)
  float openPositionY;         // Y-axis value when door is open (typically 0)
  float openPositionZ;         // Z-axis value when door is open (typically 9.8)
  float positionTolerance;     // tolerance for position detection
};

// Door monitor class
class DoorMonitor {
private:
  DoorState currentState;
  DoorState lastMovementDirection;  // Track last known movement direction
  float lastAccelY;
  float lastAccelZ;
  unsigned long lastMovementTime;
  unsigned long stateChangeTime;
  unsigned long lastStallCheckTime;
  DoorMonitorConfig config;
  bool sensorHealthy;
  int consecutiveSensorFailures;
  
public:
  DoorMonitor();
  DoorMonitor(const DoorMonitorConfig& cfg);
  
  // Core state update function
  DoorState updateState(const AccelData& accel, unsigned long currentTime);
  
  // State queries
  DoorState getState() const { return currentState; }
  const char* getStateString() const;
  const char* getDetailedStatus() const;
  bool isMoving() const { return currentState == DOOR_OPENING || currentState == DOOR_CLOSING; }
  bool isAtPosition() const { return currentState == DOOR_CLOSED || currentState == DOOR_OPEN; }
  bool isSensorHealthy() const { return sensorHealthy; }
  unsigned long getTimeInCurrentState(unsigned long currentTime) const;
  DoorState getLastMovementDirection() const { return lastMovementDirection; }
  
  // Configuration
  void setConfig(const DoorMonitorConfig& cfg) { config = cfg; }
  DoorMonitorConfig getConfig() const { return config; }
  
  // Reset/initialization
  void reset();
  void initialize(float initialAccelY, float initialAccelZ, unsigned long currentTime);
  
  // Testable helper functions
  static float calculateAccelChange(float current, float previous);
  static bool isMovementSignificant(float accelChange, float threshold);
  static bool hasTimedOut(unsigned long elapsed, unsigned long timeout);
  static bool isInClosedPosition(float accelY, float accelZ, float closedY, float closedZ, float tolerance);
  static bool isInOpenPosition(float accelY, float accelZ, float openY, float openZ, float tolerance);
  static DoorState determineDirection(float currentY, float previousY, float currentZ, float previousZ, float threshold);
};

// Default configuration
const DoorMonitorConfig DEFAULT_CONFIG = {
  0.5,    // accelThreshold (m/s^2)
  2000,   // stopTimeout (ms)
  30000,  // maxOpenTime (ms)
  30000,  // maxCloseTime (ms)
  0.1,    // stallThreshold (m/s^2)
  5000,   // stallTimeout (ms)
  9.8,    // closedPositionY (m/s^2)
  0.0,    // closedPositionZ (m/s^2)
  0.0,    // openPositionY (m/s^2)
  9.8,    // openPositionZ (m/s^2)
  1.0     // positionTolerance (m/s^2) - increased from 0.5 to 1.0 for real-world sensor noise
};

#endif // DOOR_MONITOR_H
