package org.usfirst.frc.team5002.robot.subsystems;

import org.usfirst.frc.team5002.robot.RobotMap;
import org.usfirst.frc.team5002.robot.commands.KillDrivetrain;
import com.ctre.CANTalon;
import com.ctre.CANTalon.FeedbackDevice;
import com.ctre.CANTalon.TalonControlMode;
import edu.wpi.first.wpilibj.DriverStation;
import edu.wpi.first.wpilibj.command.Subsystem;
import edu.wpi.first.wpilibj.networktables.NetworkTablesJNI;
import edu.wpi.first.wpilibj.smartdashboard.SmartDashboard;
import edu.wpi.first.wpilibj.smartdashboard.SendableChooser;

/**
 * @author stmobo @version Last Modified 1/18/17
 * SwerveDrive.java -- swerve drive subsystem
 */
public class SwerveDrive extends Subsystem {
    // The actual steer motors...
	private CANTalon fl_steer;
	private CANTalon fr_steer;
	private CANTalon bl_steer;
	private CANTalon br_steer;

    // The main drive motors...
    private CANTalon fl_drive;
    private CANTalon fr_drive;
    private CANTalon bl_drive;
    private CANTalon br_drive;

    /* Steering zero positions:
     *  FL: -0.54,
     *  FR: -0.04,
     *  BL: -0.35,
     *  BR: -0.33
     */

    /*
     * Control system values:
     * (P/I/D/FeedFwd/IZone/RampRate)
     * FL: 28 / 0.00125 / 300 / 0 / 512  / 15
     * FR: 12 / 0.0015  / 300 / 0 / 1023 / 25
     * BL: 24 / 0.0015  / 720 / 0 / 2048 / 10
     * BR: 24 / 0.0015  / 350 / 0 / 0    / 15
     */

    /* value ordering: FrontLeft, FrontRight, BackLeft, BackRight
     * Yes, for all 4 arrays (or however many there are.)
     */
    private double[] steer_offsets = { 11.0, 576.0, 73.0, 834.0 };
    private double[] currentSteerTarget = {0.0, 0.0, 0.0, 0.0};
    private double[] currentSteerDegrees = {0.0, 0.0, 0.0, 0.0};
    private double[] steerADCFiltered = {0.0, 0.0, 0.0, 0.0};
    private final double steerFilterConstant = 0.25;

    boolean driveReversalStatus[] = {false, false, false, false};
    boolean driveReversalConst[] = {true, false, true, false};

    /* NOTE: Exact object type below is SendableChooser<Boolean>.
     * Unfortunately, Java's generics are... not as powerful as I'd like,
     * and we can't /directly/ create an array of SendableChooser<Boolean>. */
    SendableChooser swerveInhibitSelectors[];


    public enum ModulePosition {
    	FL,    ///< Front-left
    	FR,    ///< Front-right
    	BL,    ///< Back-left
    	BR     ///< Back-right
    };

    private int positionToIndex(ModulePosition pos) {
    	switch(pos) {
    	case FL:
		default:
    		return 0;
    	case FR:
    		return 1;
    	case BL:
    		return 2;
    	case BR:
    		return 3;
    	}
    }

    private String positionToFriendlyName(ModulePosition pos) {
    	switch(pos) {
    	case FL:
    		return "FL";
    	case FR:
    		return "FR";
    	case BL:
    		return "BL";
    	case BR:
    		return "BR";
    	default:
    		return "UN";
    	}
    }

    /**
     * Gets the relative value of "forward" with respect to a steer encoder.
     * Driving a steer motor to this position will induce forward motion when
     * driven with positive voltage.
     *
     * @param pos position of the steer module to inspect.
     */
    private double getSteerOffset(ModulePosition pos) {
    	return steer_offsets[positionToIndex(pos)];
    }

    /**
     * Gets whether a given steer module's drive motor should be reversed by default.
     *
     * @param pos position of the steer module to inspect.
     */
    private boolean getDriveReverse(ModulePosition pos) {
    	return driveReversalStatus[positionToIndex(pos)];
    }

    /**
     * Gets whether a given steer module's drive motor should be reversed by default.
     *
     * @param pos position of the steer module to inspect.
     */
    private boolean getDriveReverseConst(ModulePosition pos) {
    	return driveReversalConst[positionToIndex(pos)];
    }

    /**
     * Gets a steer module's current steer target.
     *
     * @param pos position of the steer module to inspect.
     */
    private double getSteerTarget(ModulePosition pos) {
    	return currentSteerTarget[positionToIndex(pos)];
    }

    /**
     * Gets whether a given steer module's drive motor should be reversed by default.
     *
     * @param pos position of the steer module to inspect.
     */
    private void setDriveReverse(ModulePosition pos, boolean stat) {
    	driveReversalStatus[positionToIndex(pos)] = stat;
    }

    /**
     * Maps ModulePositions to their corresponding steer motor controllers.
     *
     * @param pos position of the steer module to inspect.
     */
    private CANTalon getSteerController(ModulePosition pos) {
    	switch(pos) {
    	case FL:
    		return fl_steer;
    	case FR:
    		return fr_steer;
    	case BL:
    		return bl_steer;
    	case BR:
    		return br_steer;
    	}

    	return null;
    }

    /**
     * Maps ModulePositions to their corresponding drive motor controllers.
     *
     * @param pos position of the steer module to inspect.
     */
    private CANTalon getDriveController(ModulePosition pos) {
    	switch(pos) {
    	case FL:
    		return fl_drive;
    	case FR:
    		return fr_drive;
    	case BL:
    		return bl_drive;
    	case BR:
    		return br_drive;
    	}

    	return null;
    }

	public SwerveDrive() {
    	/* Init steer (swerve? motor-turner?) motors */
        fl_steer = new CANTalon(RobotMap.fl_steer);
        fr_steer = new CANTalon(RobotMap.fr_steer);
        bl_steer = new CANTalon(RobotMap.bl_steer);
        br_steer = new CANTalon(RobotMap.br_steer);

        swerveInhibitSelectors = new SendableChooser[4];

		this.configureSteerMotor(ModulePosition.FR);
        this.configureSteerMotor(ModulePosition.FL);
        this.configureSteerMotor(ModulePosition.BL);
        this.configureSteerMotor(ModulePosition.BR);

        /* Init drive motors... */
        fl_drive = new CANTalon(RobotMap.fl_drive);
        fr_drive = new CANTalon(RobotMap.fr_drive);
        bl_drive = new CANTalon(RobotMap.bl_drive);
        br_drive = new CANTalon(RobotMap.br_drive);

        configureDriveMotor(ModulePosition.FL);
        configureDriveMotor(ModulePosition.FR);
        configureDriveMotor(ModulePosition.BL);
        configureDriveMotor(ModulePosition.BR);
    }

    /**
     * Performs steer motor intialization.
     *
     * @param srx reference to a steer motor controller
     * @param reverse reversal status of given motor controller (true = reversed motor outputs)
     */
    private void configureSteerMotor(ModulePosition pos) {
        CANTalon srx = getSteerController(pos);
    	srx.changeControlMode(TalonControlMode.Position);
    	srx.setFeedbackDevice(FeedbackDevice.AnalogEncoder);
		srx.setProfile(0);

        SendableChooser<Boolean> chooser = new SendableChooser<Boolean>();
        chooser.addDefault("Enabled", true);
        chooser.addObject("Disabled", false);
        SmartDashboard.putData(positionToFriendlyName(pos) + "-Inhibit", chooser);

        swerveInhibitSelectors[positionToIndex(pos)] = chooser;
    }

    /**
     * Configures a swerve module for direct control (Vbus mode on drive)
     *
     * @param pos Steer module to reconfigure
     */
    private void configureDriveMotor(ModulePosition pos) {
    	CANTalon srx = getDriveController(pos);

    	srx.reverseOutput(getDriveReverseConst(pos)); // not even sure this works
    	srx.changeControlMode(TalonControlMode.PercentVbus);

        /* Sensor setup: */
    	srx.setFeedbackDevice(FeedbackDevice.QuadEncoder);
    	srx.configEncoderCodesPerRev(40);

    	srx.set(0); // Reset to stopped
    }

    /**
     * Sets the given swerve module to drive to a given distance.
     *
     * @param pos Steer module to drive
     * @param out Output target to set to
     */
    public void setDriveDistance(ModulePosition pos, double out) {
    	CANTalon drive = getDriveController(pos);
    	CANTalon steer = getSteerController(pos);

        if(drive.getControlMode() != TalonControlMode.Position) {
            drive.changeControlMode(TalonControlMode.Position);
        }

        drive.set(out);
    }

    /**
     * Sets the given swerve module to drive at given % of max speed.
     *
     * @param pos Steer module to drive
     * @param out Output target to set to
     */
    public void setDriveSpeed(ModulePosition pos, double out) {
    	CANTalon drive = getDriveController(pos);
    	CANTalon steer = getSteerController(pos);

        if(drive.getControlMode() != TalonControlMode.PercentVbus) {
            drive.changeControlMode(TalonControlMode.PercentVbus);
        }

    	// 45 native units is about equal to 15 degrees
    	if(Math.abs(steer.getPosition() - getSteerTarget(pos)) >= 45) {
    		drive.set(0);
    	} else {
    		drive.set(out * (getDriveReverseConst(pos) ? -1 : 1) * (getDriveReverse(pos) ? -1 : 1));
    	}
    }

    private double getCurrentSteerPositionDegrees(ModulePosition pos) {
    	CANTalon steer = getSteerController(pos);
    	double nativeUnits = steer.getPosition();

    	nativeUnits -= getSteerOffset(pos);
    	double degrees = nativeUnits * (360.0 / 1024.0);

    	return degrees;
    }

    private int getCurrentSteerRotations(ModulePosition pos) {
        CANTalon steer = getSteerController(pos);
        double nativeUnits = steer.getPosition();

        /* Round towards 0: */
        if(nativeUnits < 0) {
            return (int)Math.ceil(nativeUnits / 1024);
        }
        return (int)Math.floor(nativeUnits / 1024);
    }

    /**
     * Sets the given swerve module's steering position.
     *
     * This call will command the appropriate steer motor to move to a given
     * target in degrees (where 0 degrees equates to "straight forward").
     *
     * Unlike setDriveOutput(), the semantics of this call are the same between
     * teleop and auto.
     */
    public void setSteerDegrees(ModulePosition pos, double degrees) {
    	CANTalon steer = getSteerController(pos);
    	CANTalon drive = getDriveController(pos);


    	double currentAngle = getCurrentSteerPositionDegrees(pos);
        double angleAdjustment = getCurrentSteerRotations(pos) * 360.0;

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
            setDriveReverse(pos, true);
        } else {
            setDriveReverse(pos, false);
        }

        if(steer.getControlMode() != TalonControlMode.Position) {
            steer.changeControlMode(TalonControlMode.Position);
        }

    	SmartDashboard.putNumber("SteerRawTarget-"+positionToFriendlyName(pos), degrees);
    	SmartDashboard.putNumber("SteerTarget-"+positionToFriendlyName(pos), angles[minIdx]);

    	double nativePos = angles[minIdx] * (1024.0 / 360.0);
    	nativePos += getSteerOffset(pos);

        currentSteerTarget[positionToIndex(pos)] = nativePos;

    	steer.set(nativePos);
    }

    public void setSteerDegreesCollective(double degrees) {
        setSteerDegrees(ModulePosition.FL, degrees);
        setSteerDegrees(ModulePosition.FR, degrees);
        setSteerDegrees(ModulePosition.BL, degrees);
        setSteerDegrees(ModulePosition.BR, degrees);
    }

    public void setDriveSpeedCollective(double speed) {
        setDriveSpeed(ModulePosition.FL, speed);
        setDriveSpeed(ModulePosition.FR, speed);
        setDriveSpeed(ModulePosition.BL, speed);
        setDriveSpeed(ModulePosition.BR, speed);
    }

    public void initDefaultCommand() {
    	this.setDefaultCommand(new KillDrivetrain());
    }

    /**
     * Outputs debugging data to the SmartDashboard for one swerve module.
     *
     * Each output key is suffixed with a given string for identification.
     * @param pos swerve module to inspect
     * @param suffix string identifier suffix
     */
    public void UpdateSDSingle(ModulePosition pos) {
        CANTalon steer = getSteerController(pos);
    	CANTalon drive = getDriveController(pos);
        String suffix = positionToFriendlyName(pos);

        @SuppressWarnings("unchecked")
        SendableChooser<Boolean> chooser = swerveInhibitSelectors[positionToIndex(pos)];
        if(chooser.getSelected().booleanValue()) {
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

        steerADCFiltered[positionToIndex(pos)] += steerFilterConstant * (steer.getAnalogInRaw() - steerADCFiltered[positionToIndex(pos)]);

    	SmartDashboard.putNumber("SteerErr-"+suffix, steer.getClosedLoopError());
    	SmartDashboard.putNumber("SteerPos-"+suffix, steer.getPosition());
    	SmartDashboard.putNumber("SteerVel-"+suffix, steer.getAnalogInVelocity());
    	SmartDashboard.putNumber("SteerADC-"+suffix, steer.getAnalogInRaw());
    	SmartDashboard.putNumber("SteerDeg-"+suffix, getCurrentSteerPositionDegrees(pos));
    	SmartDashboard.putNumber("SteerRot-"+suffix, getCurrentSteerRotations(pos));

        SmartDashboard.putNumber("DriveSpeed-"+suffix, drive.getSpeed());
        SmartDashboard.putNumber("DrivePos-"+suffix, drive.getPosition());
        SmartDashboard.putNumber("DriveErr-"+suffix, drive.getClosedLoopError());
    }

    /*
     * sends data to the SmartDashboard
     */
    public void updateSD(){
    	UpdateSDSingle(ModulePosition.FL);
    	UpdateSDSingle(ModulePosition.FR);
    	UpdateSDSingle(ModulePosition.BL);
    	UpdateSDSingle(ModulePosition.BR);

    	SmartDashboard.putString("Steer Encoder Calibration",
    		"{ " + Double.toString(steerADCFiltered[0]) + ", "
				 + Double.toString(steerADCFiltered[1]) + ", "
				 + Double.toString(steerADCFiltered[2]) + ", "
				 + Double.toString(steerADCFiltered[3]) + " }"
    	);
    }
}
