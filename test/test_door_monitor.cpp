#include <gtest/gtest.h>
#include "DoorMonitor.h"

// Test fixture for DoorMonitor tests
class DoorMonitorTest : public ::testing::Test {
protected:
    DoorMonitor* monitor;
    DoorMonitorConfig config;
    
    void SetUp() override {
        config = DEFAULT_CONFIG;
        config.accelThreshold = 0.5;
        config.stopTimeout = 2000;
        config.maxOpenTime = 10000;
        config.maxCloseTime = 10000;
        config.stallThreshold = 0.1;
        config.stallTimeout = 3000;
        config.closedPositionY = 9.8;
        config.closedPositionZ = 0.0;
        config.openPositionY = 0.0;
        config.openPositionZ = 9.8;
        config.positionTolerance = 0.5;
        
        monitor = new DoorMonitor(config);
    }
    
    void TearDown() override {
        delete monitor;
    }
    
    AccelData createAccelData(float x, float y, float z, bool valid = true) {
        AccelData data;
        data.x = x;
        data.y = y;
        data.z = z;
        data.valid = valid;
        return data;
    }
};

// ============================================================================
// Test: Initialization
// ============================================================================

TEST_F(DoorMonitorTest, InitialStateIsUnknown) {
    EXPECT_EQ(UNKNOWN, monitor->getState());
}

TEST_F(DoorMonitorTest, InitializeSetsClosed) {
    monitor->initialize(9.8, 0.0, 1000);  // Closed position (Y=9.8, Z=0)
    EXPECT_EQ(CLOSED, monitor->getState());
    EXPECT_TRUE(monitor->isSensorHealthy());
}

TEST_F(DoorMonitorTest, InitializeSetsOpen) {
    monitor->initialize(0.0, 9.8, 1000);  // Open position (Y=0, Z=9.8)
    EXPECT_EQ(OPEN, monitor->getState());
    EXPECT_TRUE(monitor->isSensorHealthy());
}

TEST_F(DoorMonitorTest, InitializeSetsStopped) {
    monitor->initialize(5.0, 5.0, 1000);  // Not in closed or open position
    EXPECT_EQ(STOPPED, monitor->getState());
    EXPECT_TRUE(monitor->isSensorHealthy());
}

TEST_F(DoorMonitorTest, ResetClearsState) {
    monitor->initialize(9.8, 0.0, 1000);
    AccelData accel = createAccelData(0, 10.5, 0.5);
    monitor->updateState(accel, 1100);
    
    monitor->reset();
    EXPECT_EQ(UNKNOWN, monitor->getState());
}

// ============================================================================
// Test: Position Detection
// ============================================================================

TEST_F(DoorMonitorTest, DetectsClosedPosition) {
    monitor->initialize(5.0, 5.0, 1000);
    EXPECT_EQ(STOPPED, monitor->getState());
    
    // Move to closed position (Y=9.8, Z=0)
    AccelData accel = createAccelData(0, 9.8, 0.0);
    monitor->updateState(accel, 1100);
    
    // Wait for stop timeout
    monitor->updateState(accel, 4000);
    EXPECT_EQ(CLOSED, monitor->getState());
}

TEST_F(DoorMonitorTest, DetectsOpenPosition) {
    monitor->initialize(5.0, 5.0, 1000);
    EXPECT_EQ(STOPPED, monitor->getState());
    
    // Move to open position (Y=0, Z=9.8)
    AccelData accel = createAccelData(0, 0.0, 9.8);
    monitor->updateState(accel, 1100);
    
    // Wait for stop timeout
    monitor->updateState(accel, 4000);
    EXPECT_EQ(OPEN, monitor->getState());
}

TEST_F(DoorMonitorTest, ClosedPositionWithinTolerance) {
    monitor->initialize(9.9, 0.2, 1000);  // Within 0.5 tolerance
    EXPECT_EQ(CLOSED, monitor->getState());
}

TEST_F(DoorMonitorTest, OpenPositionWithinTolerance) {
    monitor->initialize(0.2, 9.9, 1000);  // Within 0.5 tolerance
    EXPECT_EQ(OPEN, monitor->getState());
}

TEST_F(DoorMonitorTest, NotInSpecialPositionOutsideTolerance) {
    monitor->initialize(10.5, 0.0, 1000);  // Outside tolerance
    EXPECT_EQ(STOPPED, monitor->getState());
}

TEST_F(DoorMonitorTest, IsInClosedPositionHelper) {
    EXPECT_TRUE(DoorMonitor::isInClosedPosition(9.8, 0.0, 9.8, 0.0, 0.5));
    EXPECT_TRUE(DoorMonitor::isInClosedPosition(9.5, 0.2, 9.8, 0.0, 0.5));
    EXPECT_FALSE(DoorMonitor::isInClosedPosition(10.5, 0.0, 9.8, 0.0, 0.5));
    EXPECT_FALSE(DoorMonitor::isInClosedPosition(9.8, 1.0, 9.8, 0.0, 0.5));
}

TEST_F(DoorMonitorTest, IsInOpenPositionHelper) {
    EXPECT_TRUE(DoorMonitor::isInOpenPosition(0.0, 9.8, 0.0, 9.8, 0.5));
    EXPECT_TRUE(DoorMonitor::isInOpenPosition(0.2, 9.5, 0.0, 9.8, 0.5));
    EXPECT_FALSE(DoorMonitor::isInOpenPosition(1.0, 9.8, 0.0, 9.8, 0.5));
    EXPECT_FALSE(DoorMonitor::isInOpenPosition(0.0, 10.5, 0.0, 9.8, 0.5));
}

// ============================================================================
// Test: Opening State Detection
// ============================================================================

TEST_F(DoorMonitorTest, DetectsOpeningMovement) {
    monitor->initialize(9.8, 0.0, 1000);  // Start closed
    
    // Simulate door opening (Y increases during acceleration OR Z increases)
    AccelData accel = createAccelData(0, 10.5, 0.5);
    DoorState state = monitor->updateState(accel, 1100);
    
    EXPECT_EQ(OPENING, state);
    EXPECT_EQ(OPENING, monitor->getState());
}

TEST_F(DoorMonitorTest, DetectsOpeningViaZChange) {
    monitor->initialize(9.8, 0.0, 1000);  // Start closed
    
    // Door transitioning to horizontal (Z increases)
    AccelData accel = createAccelData(0, 9.5, 1.0);
    DoorState state = monitor->updateState(accel, 1100);
    
    EXPECT_EQ(OPENING, state);
}

TEST_F(DoorMonitorTest, ContinuesOpeningWithSustainedMovement) {
    monitor->initialize(9.8, 0.0, 1000);
    
    AccelData accel1 = createAccelData(0, 10.5, 0.5);
    monitor->updateState(accel1, 1100);
    
    AccelData accel2 = createAccelData(0, 10.3, 1.5);
    monitor->updateState(accel2, 1200);
    
    EXPECT_EQ(OPENING, monitor->getState());
}

TEST_F(DoorMonitorTest, OpeningStopsAfterTimeout) {
    monitor->initialize(9.8, 0.0, 1000);
    
    // Start opening
    AccelData accel1 = createAccelData(0, 10.5, 0.5);
    monitor->updateState(accel1, 1100);
    EXPECT_EQ(OPENING, monitor->getState());
    
    // No movement for stopTimeout duration
    AccelData accel2 = createAccelData(0, 10.5, 0.5);
    monitor->updateState(accel2, 4000);
    
    EXPECT_EQ(STOPPED, monitor->getState());
}

// ============================================================================
// Test: Closing State Detection
// ============================================================================

TEST_F(DoorMonitorTest, DetectsClosingMovement) {
    monitor->initialize(0.0, 9.8, 1000);  // Start open
    
    // Simulate door closing (Y decreases OR Z decreases)
    AccelData accel = createAccelData(0, 0.5, 8.5);
    DoorState state = monitor->updateState(accel, 1100);
    
    EXPECT_EQ(CLOSING, state);
    EXPECT_EQ(CLOSING, monitor->getState());
}

TEST_F(DoorMonitorTest, DetectsClosingViaYIncrease) {
    monitor->initialize(5.0, 5.0, 1000);  // Mid-position
    
    // Door returning to vertical (Y increases toward 9.8)
    AccelData accel = createAccelData(0, 6.0, 4.5);
    DoorState state = monitor->updateState(accel, 1100);
    
    EXPECT_EQ(CLOSING, state);
}

TEST_F(DoorMonitorTest, ContinuesClosingWithSustainedMovement) {
    monitor->initialize(0.0, 9.8, 1000);
    
    AccelData accel1 = createAccelData(0, 1.0, 8.5);
    monitor->updateState(accel1, 1100);
    
    AccelData accel2 = createAccelData(0, 2.0, 7.5);
    monitor->updateState(accel2, 1200);
    
    EXPECT_EQ(CLOSING, monitor->getState());
}

TEST_F(DoorMonitorTest, ClosingStopsAtClosed) {
    monitor->initialize(0.0, 9.8, 1000);
    
    // Start closing
    AccelData accel1 = createAccelData(0, 5.0, 5.0);
    monitor->updateState(accel1, 1100);
    EXPECT_EQ(CLOSING, monitor->getState());
    
    // Reach closed position and stop
    AccelData accel2 = createAccelData(0, 9.8, 0.0);
    monitor->updateState(accel2, 2000);
    monitor->updateState(accel2, 5000);
    
    EXPECT_EQ(CLOSED, monitor->getState());
}

// ============================================================================
// Test: Direction Changes
// ============================================================================

TEST_F(DoorMonitorTest, TransitionsFromOpeningToClosing) {
    monitor->initialize(9.8, 0.0, 1000);
    
    // Start opening
    AccelData accel1 = createAccelData(0, 10.5, 0.5);
    monitor->updateState(accel1, 1100);
    EXPECT_EQ(OPENING, monitor->getState());
    
    // Reverse to closing
    AccelData accel2 = createAccelData(0, 10.0, 0.2);
    monitor->updateState(accel2, 1200);
    EXPECT_EQ(CLOSING, monitor->getState());
}

TEST_F(DoorMonitorTest, TransitionsFromClosingToOpening) {
    monitor->initialize(0.0, 9.8, 1000);
    
    // Start closing
    AccelData accel1 = createAccelData(0, 1.0, 8.5);
    monitor->updateState(accel1, 1100);
    EXPECT_EQ(CLOSING, monitor->getState());
    
    // Reverse to opening
    AccelData accel2 = createAccelData(0, 0.5, 9.0);
    monitor->updateState(accel2, 1200);
    EXPECT_EQ(OPENING, monitor->getState());
}

// ============================================================================
// Test: Error - Sensor Failure
// ============================================================================

TEST_F(DoorMonitorTest, DetectsSensorFailure) {
    monitor->initialize(9.8, 0.0, 1000);
    
    // Send invalid data multiple times
    AccelData invalidAccel = createAccelData(0, 0, 0, false);
    for (int i = 0; i < 5; i++) {
        monitor->updateState(invalidAccel, 1000 + i * 100);
    }
    
    EXPECT_EQ(ERROR_SENSOR_FAILURE, monitor->getState());
    EXPECT_FALSE(monitor->isSensorHealthy());
}

TEST_F(DoorMonitorTest, RecoverFromSensorFailure) {
    monitor->initialize(9.8, 0.0, 1000);
    
    // Cause sensor failure
    AccelData invalidAccel = createAccelData(0, 0, 0, false);
    for (int i = 0; i < 5; i++) {
        monitor->updateState(invalidAccel, 1000 + i * 100);
    }
    EXPECT_EQ(ERROR_SENSOR_FAILURE, monitor->getState());
    
    // Recover with valid data
    AccelData validAccel = createAccelData(0, 9.8, 0.0, true);
    monitor->updateState(validAccel, 2000);
    
    EXPECT_TRUE(monitor->isSensorHealthy());
    EXPECT_NE(ERROR_SENSOR_FAILURE, monitor->getState());
}

// ============================================================================
// Test: Error - Timeout
// ============================================================================

TEST_F(DoorMonitorTest, DetectsOpeningTimeout) {
    monitor->initialize(9.8, 0.0, 1000);
    
    // Start opening
    AccelData accel = createAccelData(0, 10.5, 0.5);
    monitor->updateState(accel, 1100);
    EXPECT_EQ(OPENING, monitor->getState());
    
    // Still opening after maxOpenTime
    monitor->updateState(accel, 12000);
    
    EXPECT_EQ(ERROR_TIMEOUT, monitor->getState());
}

TEST_F(DoorMonitorTest, DetectsClosingTimeout) {
    monitor->initialize(0.0, 9.8, 1000);
    
    // Start closing
    AccelData accel = createAccelData(0, 1.0, 8.5);
    monitor->updateState(accel, 1100);
    EXPECT_EQ(CLOSING, monitor->getState());
    
    // Still closing after maxCloseTime
    monitor->updateState(accel, 12000);
    
    EXPECT_EQ(ERROR_TIMEOUT, monitor->getState());
}

// ============================================================================
// Test: Error - Stall
// ============================================================================

TEST_F(DoorMonitorTest, DetectsStall) {
    monitor->initialize(9.8, 0.0, 1000);
    
    // Start opening
    AccelData accel1 = createAccelData(0, 10.5, 0.5);
    monitor->updateState(accel1, 1100);
    EXPECT_EQ(OPENING, monitor->getState());
    
    // Very slow movement
    AccelData accel2 = createAccelData(0, 10.52, 0.52);
    monitor->updateState(accel2, 1200);
    
    // Continue slow movement past stallTimeout
    monitor->updateState(accel2, 5000);
    
    EXPECT_EQ(ERROR_STALLED, monitor->getState());
}

// ============================================================================
// Test: Helper Functions
// ============================================================================

TEST_F(DoorMonitorTest, CalculateAccelChange) {
    EXPECT_NEAR(1.2, DoorMonitor::calculateAccelChange(11.0, 9.8), 0.01);
    EXPECT_NEAR(1.2, DoorMonitor::calculateAccelChange(9.8, 11.0), 0.01);
}

TEST_F(DoorMonitorTest, IsMovementSignificant) {
    EXPECT_TRUE(DoorMonitor::isMovementSignificant(1.2, 0.5));
    EXPECT_FALSE(DoorMonitor::isMovementSignificant(0.3, 0.5));
}

TEST_F(DoorMonitorTest, DetermineDirectionOpening) {
    // Y increases = opening
    DoorState state = DoorMonitor::determineDirection(10.5, 9.8, 0.5, 0.5, 0.5);
    EXPECT_EQ(OPENING, state);
    
    // Z increases = opening
    state = DoorMonitor::determineDirection(9.8, 9.8, 1.0, 0.0, 0.5);
    EXPECT_EQ(OPENING, state);
}

TEST_F(DoorMonitorTest, DetermineDirectionClosing) {
    // Y decreases = closing (when coming from high Y)
    DoorState state = DoorMonitor::determineDirection(5.0, 6.0, 5.0, 5.0, 0.5);
    EXPECT_EQ(CLOSING, state);
    
    // Z decreases = closing
    state = DoorMonitor::determineDirection(5.0, 5.0, 5.0, 6.0, 0.5);
    EXPECT_EQ(CLOSING, state);
}

TEST_F(DoorMonitorTest, HasTimedOut) {
    EXPECT_TRUE(DoorMonitor::hasTimedOut(3000, 2000));
    EXPECT_FALSE(DoorMonitor::hasTimedOut(1000, 2000));
}

// ============================================================================
// Test: State Strings
// ============================================================================

TEST_F(DoorMonitorTest, GetStateStringClosed) {
    monitor->initialize(9.8, 0.0, 1000);
    EXPECT_STREQ("CLOSED", monitor->getStateString());
}

TEST_F(DoorMonitorTest, GetStateStringOpen) {
    monitor->initialize(0.0, 9.8, 1000);
    EXPECT_STREQ("OPEN", monitor->getStateString());
}

TEST_F(DoorMonitorTest, GetStateStringOpening) {
    monitor->initialize(9.8, 0.0, 1000);
    AccelData accel = createAccelData(0, 10.5, 0.5);
    monitor->updateState(accel, 1100);
    EXPECT_STREQ("OPENING", monitor->getStateString());
}

TEST_F(DoorMonitorTest, GetStateStringClosing) {
    monitor->initialize(0.0, 9.8, 1000);
    AccelData accel = createAccelData(0, 1.0, 8.5);
    monitor->updateState(accel, 1100);
    EXPECT_STREQ("CLOSING", monitor->getStateString());
}

TEST_F(DoorMonitorTest, GetStateStringStopped) {
    monitor->initialize(5.0, 5.0, 1000);
    AccelData accel = createAccelData(0, 5.0, 5.0);
    monitor->updateState(accel, 5000);
    EXPECT_STREQ("STOPPED", monitor->getStateString());
}

// Main function
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
