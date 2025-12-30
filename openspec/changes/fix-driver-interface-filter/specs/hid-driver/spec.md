## ADDED Requirements

### Requirement: Interface Filtering
The driver SHALL only bind to Interface 1 (main input interface).

#### Scenario: Interface 1 probe
- **WHEN** the driver probes Interface 1
- **THEN** the driver initializes and creates an input device

#### Scenario: Other interface probe
- **WHEN** the driver probes Interface 0, 2, 3, 4, 5, or 6
- **THEN** the driver returns -ENODEV
- **AND** the kernel default driver handles the interface

### Requirement: Debug Logging
The driver SHALL provide debug logging for troubleshooting.

#### Scenario: Probe logging
- **WHEN** the driver probes any interface
- **THEN** it logs the interface number with hid_dbg()

#### Scenario: Error logging
- **WHEN** any operation fails
- **THEN** it logs the error code with hid_err()

#### Scenario: Success logging
- **WHEN** initialization succeeds
- **THEN** it logs with hid_info()
