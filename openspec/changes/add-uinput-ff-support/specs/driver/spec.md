# Driver Spec Delta

## MODIFIED Requirements

### Requirement: Rumble Support

The driver SHALL support Xbox 360 style rumble via hidraw CMD 0x12 **and receive force feedback events from games via uinput**.

#### Scenario: Send rumble
- Given rumble request received
- When left/right motor values set
- Then send packet: 5a a5 12 06 LL RR

#### Scenario: Receive FF from game
- Given a game uploads FF_RUMBLE effect
- When the effect is played (EV_FF with value > 0)
- Then `poll_ff()` returns RumbleEffect with strong/weak values
- And daemon calls `send_rumble()` with scaled values

#### Scenario: Stop FF from game
- Given a rumble effect is active
- When the game stops the effect (EV_FF with value = 0)
- Then `poll_ff()` returns RumbleEffect with zero values
- And daemon calls `send_rumble(0, 0)`
