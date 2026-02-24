#include "DoorMonitor.h"
#include <stdlib.h>

DoorMonitor::DoorMonitor() : DoorMonitor(DEFAULT_CONFIG) {}

DoorMonitor::DoorMonitor(const DoorMonitorConfig& cfg) 
  : currentState(DOOR_UNKNOWN),
    lastMovementDirection(DOOR_UNKNOWN),
    lastAccelY(0),
    lastAccelZ(0),
    lastMovementTime(0),
    stateChangeTime(0),
    lastStallCheckTime(0),
    config(cfg),
    sensorHealthy(true),
    consecutiveSensorFailures(0) {}

void DoorMonitor::reset() {
  currentState = DOOR_UNKNOWN;
  lastMovementDirection = DOOR_UNKNOWN;
  lastAccelY = 0;
  lastAccelZ = 0;
  lastMovementTime = 0;
  stateChangeTime = 0;
  lastStallCheckTime = 0;
  sensorHealthy = true;
  consecutiveSensorFailures = 0;
}

void DoorMonitor::initialize(float initialAccelY, float initialAccelZ, unsigned long currentTime) {
  lastAccelY = initialAccelY;
  lastAccelZ = initialAccelZ;
  lastMovementTime = currentTime;
  stateChangeTime = currentTime;
  lastStallCheckTime = currentTime;
  
  // Check if door is in closed or open position
  if (isInClosedPosition(initialAccelY, initialAccelZ, config.closedPositionY, config.closedPositionZ, config.positionTolerance)) {
    currentState = DOOR_CLOSED;
  } else if (isInOpenPosition(initialAccelY, initialAccelZ, config.openPositionY, config.openPositionZ, config.positionTolerance)) {
    currentState = DOOR_OPEN;
  } else {
    currentState = DOOR_STOPPED;
  }
  
  sensorHealthy = true;
  consecutiveSensorFailures = 0;
}

const char* DoorMonitor::getStateString() const {
  switch (currentState) {
    case DOOR_CLOSED: return "CLOSED";
    case DOOR_OPEN: return "OPEN";
    case DOOR_OPENING: return "OPENING";
    case DOOR_CLOSING: return "CLOSING";
    case DOOR_STOPPED: return "STOPPED";
    case DOOR_ERROR_SENSOR_FAILURE: return "ERROR_SENSOR_FAILURE";
    case DOOR_ERROR_TIMEOUT: return "ERROR_TIMEOUT";
    case DOOR_ERROR_STALLED: return "ERROR_STALLED";
    default: return "UNKNOWN";
  }
}

const char* DoorMonitor::getDetailedStatus() const {
  switch (currentState) {
    case DOOR_CLOSED: return "Door is CLOSED (vertical position, Y=9.8, Z=0)";
    case DOOR_OPEN: return "Door is OPEN (horizontal position, Y=0, Z=9.8)";
    case DOOR_OPENING: return "Door is OPENING (moving toward open position)";
    case DOOR_CLOSING: return "Door is CLOSING (moving toward closed position)";
    case DOOR_STOPPED: 
      if (lastMovementDirection == DOOR_OPENING) {
        return "Door STOPPED mid-open (was opening, not at full open)";
      } else if (lastMovementDirection == DOOR_CLOSING) {
        return "Door STOPPED mid-close (was closing, not at full closed)";
      }
      return "Door STOPPED (intermediate position)";
    case DOOR_ERROR_SENSOR_FAILURE: return "ERROR: Sensor failure detected";
    case DOOR_ERROR_TIMEOUT: 
      if (lastMovementDirection == DOOR_OPENING) {
        return "ERROR: Door took too long to open (timeout)";
      }
      return "ERROR: Door took too long to close (timeout)";
    case DOOR_ERROR_STALLED: 
      if (lastMovementDirection == DOOR_OPENING) {
        return "ERROR: Door stalled while opening";
      }
      return "ERROR: Door stalled while closing";
    default: return "Status UNKNOWN";
  }
}

unsigned long DoorMonitor::getTimeInCurrentState(unsigned long currentTime) const {
  return currentTime - stateChangeTime;
}

float DoorMonitor::calculateAccelChange(float current, float previous) {
  return abs(current - previous);
}

bool DoorMonitor::isMovementSignificant(float accelChange, float threshold) {
  return accelChange > threshold;
}

bool DoorMonitor::hasTimedOut(unsigned long elapsed, unsigned long timeout) {
  return elapsed > timeout;
}

bool DoorMonitor::isInClosedPosition(float accelY, float accelZ, float closedY, float closedZ, float tolerance) {
  return (abs(accelY - closedY) <= tolerance) && (abs(accelZ - closedZ) <= tolerance);
}

bool DoorMonitor::isInOpenPosition(float accelY, float accelZ, float openY, float openZ, float tolerance) {
  return (abs(accelY - openY) <= tolerance) && (abs(accelZ - openZ) <= tolerance);
}

DoorState DoorMonitor::determineDirection(float currentY, float previousY, float currentZ, float previousZ, float threshold) {
  float yChange = currentY - previousY;
  float zChange = currentZ - previousZ;
  
  // Opening: Y increases (upward acceleration) OR Z increases (transitioning to horizontal)
  if (yChange > threshold || zChange > threshold) {
    return DOOR_OPENING;
  }
  // Closing: Y decreases (downward) OR Z decreases (transitioning to vertical)
  else if (yChange < -threshold || zChange < -threshold) {
    return DOOR_CLOSING;
  }
  
  return DOOR_UNKNOWN;
}

DoorState DoorMonitor::updateState(const AccelData& accel, unsigned long currentTime) {
  // Check sensor health
  if (!accel.valid) {
    consecutiveSensorFailures++;
    if (consecutiveSensorFailures >= 5) {
      sensorHealthy = false;
      if (currentState != DOOR_ERROR_SENSOR_FAILURE) {
        currentState = DOOR_ERROR_SENSOR_FAILURE;
        stateChangeTime = currentTime;
      }
      return currentState;
    }
  } else {
    consecutiveSensorFailures = 0;
    sensorHealthy = true;
  }
  
  // Return early if in error state
  if (currentState == DOOR_ERROR_SENSOR_FAILURE && sensorHealthy) {
    // Recovered from sensor failure
    currentState = DOOR_UNKNOWN;
    stateChangeTime = currentTime;
  }
  
  float accelY = accel.y;
  float accelZ = accel.z;
  float yChange = calculateAccelChange(accelY, lastAccelY);
  float zChange = calculateAccelChange(accelZ, lastAccelZ);
  float totalChange = yChange + zChange;
  
  // Check for significant movement
  if (isMovementSignificant(totalChange, config.accelThreshold)) {
    lastMovementTime = currentTime;
    lastStallCheckTime = currentTime;
    
    // Determine direction based on Y and Z changes
    DoorState newDirection = determineDirection(accelY, lastAccelY, accelZ, lastAccelZ, config.accelThreshold);
    
    if (newDirection != DOOR_UNKNOWN && newDirection != currentState) {
      if (currentState != DOOR_OPENING && currentState != DOOR_CLOSING) {
        // Transitioning from stopped/error to moving
        currentState = newDirection;
        lastMovementDirection = newDirection;
        stateChangeTime = currentTime;
      } else {
        // Already moving, update direction if changed
        currentState = newDirection;
        lastMovementDirection = newDirection;
        stateChangeTime = currentTime;
      }
    } else if (newDirection != DOOR_UNKNOWN) {
      // Still moving in same direction
      lastMovementDirection = newDirection;
    }
  } else {
    // No significant movement detected
    unsigned long timeSinceMovement = currentTime - lastMovementTime;
    
    // Check for timeout (taking too long to complete)
    if (currentState == DOOR_OPENING || currentState == DOOR_CLOSING) {
      unsigned long timeInState = getTimeInCurrentState(currentTime);
      unsigned long maxTime = (currentState == DOOR_OPENING) ? config.maxOpenTime : config.maxCloseTime;
      
      if (hasTimedOut(timeInState, maxTime)) {
        currentState = DOOR_ERROR_TIMEOUT;
        stateChangeTime = currentTime;
        lastAccelY = accelY;
        lastAccelZ = accelZ;
        return currentState;
      }
      
      // Check for stall (moving but very slow)
      if (totalChange < config.stallThreshold) {
        unsigned long stallTime = currentTime - lastStallCheckTime;
        if (hasTimedOut(stallTime, config.stallTimeout)) {
          currentState = DOOR_ERROR_STALLED;
          stateChangeTime = currentTime;
          lastAccelY = accelY;
          lastAccelZ = accelZ;
          return currentState;
        }
      } else {
        lastStallCheckTime = currentTime;
      }
    }
    
    // Check if door has stopped
    if (hasTimedOut(timeSinceMovement, config.stopTimeout)) {
      if (currentState != DOOR_STOPPED && 
          currentState != DOOR_CLOSED &&
          currentState != DOOR_OPEN &&
          currentState != DOOR_ERROR_SENSOR_FAILURE && 
          currentState != DOOR_ERROR_TIMEOUT &&
          currentState != DOOR_ERROR_STALLED) {
        
        // Check if stopped in closed or open position
        if (isInClosedPosition(accelY, accelZ, config.closedPositionY, config.closedPositionZ, config.positionTolerance)) {
          currentState = DOOR_CLOSED;
        } else if (isInOpenPosition(accelY, accelZ, config.openPositionY, config.openPositionZ, config.positionTolerance)) {
          currentState = DOOR_OPEN;
        } else {
          currentState = DOOR_STOPPED;
        }
        stateChangeTime = currentTime;
      }
    }
  }
  
  lastAccelY = accelY;
  lastAccelZ = accelZ;
  return currentState;
}
