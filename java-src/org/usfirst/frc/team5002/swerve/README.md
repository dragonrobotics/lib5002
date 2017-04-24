# Swerve Drive
These commands and subsystems are intended to be used with a Swerve Drive system.

## Subsystems
This portion of the library exposes two subsystems to client code:
 * SwerveModule, which controls individual swerve modules (steer and drive motor pairs)
 * SwerveDrive, which implements a 4-module drive base composed of SwerveModules.

### SwerveModule
The SwerveModule class provides the following functionality:
 * Persistent configuration via the Preferences interface
 * Configurable steer calibration offsets and analog encoder ranges
 * Intelligent steer control using Talon Closed-Loop Position capabilities
 * Shortest-path swerve angle selection and automatic drive reversal
 * Run-time inhibition of malfunctioning Modules
 * Configurable drive control using either PercentVbus, Closed-Loop Velocity, or Closed-Loop Position control
 * SmartDashboard-based datalogging / debugging.

### SwerveDrive
The SwerveDrive class acts as a container for 4 SwerveModule objects, and provides
convenient methods for controlling all four swerve modules simultaneously.

### Mechanical Configuration
The swerve module interface assumes the following mechanical setup:
 * Steer and Drive motors controlled using Talons over CAN.
 * Each swerve module is equipped with an analog absolute encoder wired to the steer motor Talon.

The swerve drive interface makes the following additional assumptions:
 * The robot base is rectangular.
 * The drive base is equipped with 4 separate swerve modules at each corner.

## Commands

### SnakeControl
The SnakeControl command implements "snake drive", allowing for true 2D drive control,
using three separate control input axes for forward, horizontal, and rotational movement.

This command also supports an operator-controlled Drive Speed Coefficient, which
serves as a throttle: all speeds are multiplied by the coefficient before being sent
to the swerve drive.

The swerve drive requires the following functions to be exposed by the OI:
 * `double getDriveSpeedCoefficient()` - returns a multiplier in the range [0, 1] that affects the overall drive speed.
 * `double getForwardAxis()` - Control input in the range [-1, +1] for forwards (positive values) and backwards (negative values) movement.
 * `double getHorizontalAxis()` - Control input in the range [-1, +1] for rightward (positive values) and leftward (negative values) strafing.
 * `double getTurnAxis()` - - Control input in the range [-1, +1] for clockwise (positive values) and counterclockwise (negative values) rotation.

Note that the swerve module zero-angle positions should be along the robot forward-backwards axis.

### SteerCalibrate and SteerRezero
SteerCalibrate is intended for use with analog encoders that return values in unpredictable ranges.

It will save the current analog steer position as the steer offset, and then
sweep each module through a rotation, recording the minimum and maximum observed
encoder values.

SteerRezero will only reconfigure the steer offsets for each module, again
using the current analog position as the offset.

### KillDrivetrain
This command is intended as a default and as a safety mechanism: when run, it
will simply halt all drive motor activity.
