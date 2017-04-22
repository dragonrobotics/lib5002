package org.usfirst.frc.team5002.swerve.subsystems;

import org.usfirst.frc.team5002.robot.RobotMap;

import com.ctre.CANTalon;
import com.ctre.CANTalon.FeedbackDevice;
import com.ctre.CANTalon.TalonControlMode;

import edu.wpi.first.wpilibj.DriverStation;
import edu.wpi.first.wpilibj.command.Subsystem;
import edu.wpi.first.wpilibj.networktables.NetworkTablesJNI;
import edu.wpi.first.wpilibj.smartdashboard.SmartDashboard;

/**
 * SwerveModule -- Controls a single swerve module.
 * This class holds much of the logic and bookkeeping behind the swerve modules.
 *
 * @author Sebastian Mobo <stmobo@gmail.com>
 * @version 1.0, 04/22/2017
 */
public class SwerveModule {
    private CANTalon steer;
    private CANTalon drive;

    /* CALIBRATION + CONFIG VALUES: */
    private double steerOffset = 0.0;       // in encoder units
    private double steerMaxValue = 1024.0;  //
    private double steerMinValue = 0.0;     //
    private double driveMaxSpeed = 0.0;     // in encoder units/100ms
    private boolean driveReversed = false;  // if true then all drive outputs reversed
    private boolean speedEnabled = false;   // true if encoder speed-based control enabled
    private String moduleName;              // "FL", "FR", "BL", "BR", etc.

    /* INSTANCE VARIABLES: */
    private boolean steerReversed = false;  // true if steer driven to flipped-angle target
    private boolean inhibited = false;      // kills module movement if true
    private double currentSteerTarget = 0;

    public SwerveModule(String name, int steerID, int driveID) {
        steer = new CANTalon(steerID);
        drive = new CANTalon(driveID);
        moduleName = name;

        loadConfigValues();

    	steer.changeControlMode(TalonControlMode.Position);
    	steer.setFeedbackDevice(FeedbackDevice.AnalogEncoder);
		steer.setProfile(0);

        drive.reverseOutput(driveReversed); // not even sure this works
        if(speedEnabled) {
        	drive.changeControlMode(TalonControlMode.Velocity);
        } else {
        	drive.changeControlMode(TalonControlMode.PercentVbus);
        }

        /* Sensor setup: */
    	drive.setFeedbackDevice(FeedbackDevice.QuadEncoder);
    	drive.configEncoderCodesPerRev(40);

    	drive.set(0); // Reset to stopped
    }

    public void loadConfigValues() {
        Preferences prefs = Preferences.getInstance();

        steerMaxValue = prefs.getDouble("SteerMax-"+moduleName, 1024.0);
        steerMinValue = prefs.getDouble("SteerMin-"+moduleName, 0.0);
        steerOffset = prefs.getDouble("SteerOff-"+moduleName, 0.0);
        driveMaxSpeed = prefs.getDouble("DriveSpeed-"+moduleName, 0.0);
        driveReversed = prefs.getBoolean("DriveReversed-"+moduleName, false);
        speedEnabled = prefs.getBoolean("SpeedControl-"+moduleName, false);
    }

    public void configSteerOffset(double off) {
        steerOffset = off;
        Preferences.getInstance().putDouble("SteerOff-"+moduleName, off);
    }

    public void configSteerRange(double max, double min) {
        steerMinValue = min;
        steerMaxValue = max;
        Preferences.getInstance().putDouble("SteerMin-"+moduleName, min);
        Preferences.getInstance().putDouble("SteerMax-"+moduleName, max);
    }

    public void configDriveSpeed(double max) {
        driveMaxSpeed = max;
        Preferences.getInstance().putDouble("DriveSpeed-"+moduleName, max);
    }

    public void configDriveReverse(boolean rev) {
        driveReversed = rev;
        Preferences.getInstance().putDouble("DriveReversed-"+moduleName, rev);
    }

    public void configSpeedEnabled(boolean spd) {
        speedEnabled = spd;
        Preferences.getInstance().putDouble("SpeedControl-"+moduleName, spd);
    }

    public void rezeroSteer() {
        configSteerOffset(steer.getPosition());
    }

    public CANTalon getDriveController() { return drive; }
    public CANTalon getSteerController() { return steer; }

    private double getSteerRange() { return (steerMaxValue - steerMinValue); }
    private double getCurrentSteerPositionNative() { return (steer.getPosition() - steerMinValue) - steerOffset; }
    private double getCurrentSteerPositionDegrees() { return getCurrentSteerPositionNative() * (360.0 / getSteerRange()); }

    public double getCurrentSteerPositionRaw() { return steer.getPosition(); }

    private int getCurrentSteerRotations() {
        double nativeUnits = getCurrentSteerPositionNative();

        /* Round towards 0: */
        if(nativeUnits < 0) {
            return (int)Math.ceil(nativeUnits / getSteerRange());
        }
        return (int)Math.floor(nativeUnits / getSteerRange());
    }

    public void setDriveSpeed(double percSpeed) {
        double out = percSpeed;
        if(speedEnabled) {
            if(drive.getControlMode() != TalonControlMode.Velocity) {
                drive.changeControlMode(TalonControlMode.Velocity);
            }
            out *= driveMaxSpeed;
        } else {
            if(drive.getControlMode() != TalonControlMode.PercentVbus) {
                drive.changeControlMode(TalonControlMode.PercentVbus);
            }
        }

    	// 45 native units is about equal to 15 degrees
    	if(Math.abs(getCurrentSteerPositionNative() - currentSteerTarget) >= 45) {
    		drive.set(0);
    	} else {
    		drive.set(out * (driveReversed ? -1 : 1) * (steerReversed ? -1 : 1));
    	}
    }

    public void setDriveDistance(double distance) {
        if(drive.getControlMode() != TalonControlMode.Position) {
            drive.changeControlMode(TalonControlMode.Position);
        }

        drive.set(out);
    }

    public void setSteerSpeed(double percSpeed) {
        if(steer.getControlMode() != TalonControlMode.PercentVbus) {
            steer.changeControlMode(TalonControlMode.PercentVbus);
        }

    	steer.set(percSpeed);
    }

    public void setSteerDegrees(double angle) {
        double currentAngle = getCurrentSteerPositionDegrees();
        double angleAdjustment = getCurrentSteerRotations() * 360.0;

        /*
         * Angle 0: Original angle, original rotation
         * Angle 1: Opposite angle, original rotation
         * Angle 2: Original angle, opposite rotation
         * Angle 3: Opposite angle, opposite rotation
         * Angle 4: Original angle + 1 rotation, original rotation
         *
         * Angle 0 handles the normal case.
         *
         * Angle 1 handles the same-sign "opposite angle" case.
         * Angle 2 handles the opposite-sign "opposite angle" case.
         * (In these cases, the drive direction has to be reversed.)
         *
         * Angle 3 is necessary to handle Q4 -> Q3 transitions (across signs)
         *  (Going to -179 from +179 is better done as going to +181 from +179)
         * Angle 4 is necessary to handle Q2 -> Q1 transitions (across rotations)
         *  (Going to +1 from +359 is better done as going to +361 from +359)
         */

        double angles[] = { 0, 0, 0, 0, 0 };

        angles[0] = degrees + angleAdjustment; // adjust target to be relative to module rotation
        angles[1] = (angles[0] + 180.0);
        angles[2] = (angles[0] - 180.0);
        angles[3] = (angles[0] - 360.0);
        angles[4] = (angles[0] + 360.0);

        /* Find target angle with smallest distance from current: */
        int minIdx = 0;
        for(int i=0;i<angles.length;i++) {
            if( Math.abs(angles[i] - currentAngle) < Math.abs(angles[minIdx] - currentAngle) ) {
                minIdx = i;
            }
        }

        if(minIdx == 1 || minIdx == 2) {
            steerReversed = true;
        } else {
            steerReversed = false;
        }

        if(steer.getControlMode() != TalonControlMode.Position) {
            steer.changeControlMode(TalonControlMode.Position);
        }

    	SmartDashboard.putNumber("SteerRawTarget-"+moduleName, degrees);
    	SmartDashboard.putNumber("SteerTarget-"+moduleName, angles[minIdx]);

    	double nativePos = angles[minIdx] * (getSteerRange() / 360.0);
    	nativePos += steerOffset;
        nativePos += steerMinValue;

        currentSteerTarget = nativePos;
    	steer.set(nativePos);
    }

    public void updateSD() {
            if(!inhibited) {
                /* not sure if the below conditional is needed, but it doesn't hurt
                 * to make sure we're not going to do something to get us DQ'd */
                if(!DriverStation.getInstance().isDisabled()) {
                    /* Enable stuff if necessary */
                    if(!steer.isEnabled()) {
                        steer.enable();
                    }

                    if(!drive.isEnabled()) {
                        drive.enable();
                    }
                }
            } else {
                if(steer.isEnabled()) {
                    steer.disable();
                }

                if(drive.isEnabled()) {
                    drive.disable();
                }
            }

        	SmartDashboard.putNumber("SteerErr-"+moduleName, steer.getClosedLoopError());
        	SmartDashboard.putNumber("SteerPos-"+moduleName, steer.getPosition());
        	SmartDashboard.putNumber("SteerVel-"+moduleName, steer.getAnalogInVelocity());
        	SmartDashboard.putNumber("SteerADC-"+moduleName, steer.getAnalogInRaw());
        	SmartDashboard.putNumber("SteerDeg-"+moduleName, getCurrentSteerPositionDegrees());
        	SmartDashboard.putNumber("SteerRot-"+moduleName, getCurrentSteerRotations());

            SmartDashboard.putNumber("DriveSpeed-"+moduleName, drive.getSpeed());
            SmartDashboard.putNumber("DrivePos-"+moduleName, drive.getPosition());
            SmartDashboard.putNumber("DriveErr-"+moduleName, drive.getClosedLoopError());
    }
}
