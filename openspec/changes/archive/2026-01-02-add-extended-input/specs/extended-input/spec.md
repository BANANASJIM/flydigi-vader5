## ADDED Requirements

### Requirement: Extended Button Input
The driver SHALL expose extended buttons (C, Z, M1-M4, LM, RM, O, Home) as standard Linux input events.

#### Scenario: Extended buttons visible in evtest
- **WHEN** controller is connected and driver is running
- **THEN** extended buttons appear as BTN_TRIGGER_HAPPY1-10 in evtest

#### Scenario: Steam Input recognizes extended buttons
- **WHEN** Steam Controller Configurator is opened
- **THEN** extended buttons can be bound to actions

### Requirement: Gyro/Accelerometer Input
The driver SHALL expose IMU data (gyroscope and accelerometer) via evdev.

#### Scenario: Gyro data in evtest
- **WHEN** controller is tilted in test mode
- **THEN** ABS_RX/RY/RZ values change accordingly

### Requirement: DSU Protocol Support
The system SHALL provide a DSU server for Steam Input motion control compatibility.

#### Scenario: Steam detects motion controller
- **WHEN** vader5-dsu is running
- **THEN** Steam Input shows "Motion Controls" option in controller settings

### Requirement: Automatic Test Mode
The driver SHALL automatically enter test mode on connection to access extended input data.

#### Scenario: Test mode enabled on connect
- **WHEN** controller is connected
- **THEN** driver sends init sequence (01, a1, 02, 04) and test mode command (11 07)
- **AND** extended input report (5a a5 ef) is received