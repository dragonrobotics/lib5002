package org.usfirst.frc.team5002.swerve.commands;

import org.usfirst.frc.team5002.robot.Robot;

import edu.wpi.first.wpilibj.Joystick;
import edu.wpi.first.wpilibj.command.Command;
import edu.wpi.first.wpilibj.smartdashboard.SmartDashboard;

/**
 * SnakeControl.java -- teleop drive control code for linear movement + rotation
 *
 * Requires the following OI functions:
 * double getDriveSpeedCoefficient() - returns a multiplier from 0-1 that affects the overall drive speed.
 * double getForwardAxis(), double getHorizontalAxis(), double getTurnAxis() - return values from 0-1 that control robot movement.
 *
 * @author Sebastian Mobo <stmobo@gmail.com>
 * @version 1.1, 04/22/2017
 */
public class SnakeControl extends Command {
	private static final double joystickDeadband = 0.10;

	public double LENGTH_INCHES = 14.5;
	public double WIDTH_INCHES = 16.5;

	double[] angles = new double[4];
	double[] speeds = new double[4];

    private SwerveDrive drivetrain;

    public SnakeControl(double length, double width, SwerveDrive swerve) {
        LENGTH_INCHES = length;
        WIDTH_INCHES = width;
        drivetrain = swerve;

        requires(drivetrain);
    }

    private boolean autoAlignButtonDebounce = false;
    protected void execute() {
		double fwd = (Math.abs(Robot.oi.getForwardAxis()) > joystickDeadband) ? Robot.oi.getForwardAxis() : 0.0;
		double str = (Math.abs(Robot.oi.getHorizontalAxis()) > joystickDeadband) ? Robot.oi.getHorizontalAxis() : 0.0;
		double rcw = (Math.abs(Robot.oi.getTurnAxis()) > joystickDeadband) ? Robot.oi.getTurnAxis() : 0.0;

		if(Math.abs(fwd)>1.0 || Math.abs(str)>1.0 || Math.abs(rcw)>1.0){
			return;
		}

		double r = Math.sqrt(Math.pow(LENGTH_INCHES,2) + Math.pow(WIDTH_INCHES,2));

		double a = str - rcw * (LENGTH_INCHES / r);
		double b = str + rcw * (LENGTH_INCHES / r);
		double c = fwd - rcw * (WIDTH_INCHES / r);
		double d = fwd + rcw * (WIDTH_INCHES / r);
		double maxWs;

		double spd_fl = Math.sqrt(Math.pow(a, 2) + Math.pow(d, 2));
		maxWs = spd_fl > maxWs ? spd_fl : maxWs;

		double spd_fr = Math.sqrt(Math.pow(a, 2) + Math.pow(c, 2));
		maxWs = spd_fr;

		double spd_br = Math.sqrt(Math.pow(b, 2) + Math.pow(c, 2));
		maxWs = spd_br > maxWs ? spd_br : maxWs;

		double spd_bl = Math.sqrt(Math.pow(b, 2) + Math.pow(d, 2));
		maxWs = spd_bl > maxWs ? spd_bl : maxWs;

        speeds[0] = (maxWs > 1 ? spd_fl / maxWs : spd_fl) * Robot.oi.getDriveSpeedCoefficient();
		speeds[1] = (maxWs > 1 ? spd_fr / maxWs : spd_fr) * Robot.oi.getDriveSpeedCoefficient();
		speeds[2] = (maxWs > 1 ? spd_br / maxWs : spd_br) * Robot.oi.getDriveSpeedCoefficient();
		speeds[3] = (maxWs > 1 ? spd_bl / maxWs : spd_bl) * Robot.oi.getDriveSpeedCoefficient();

		if((Math.abs(fwd) > 0.0)  ||
			(Math.abs(str) > 0.0) ||
			(Math.abs(rcw) > 0.0))
        {
			angles[0] = (d==0 && a==0) ? 0.0 : (Math.atan2(a, d) * 180 / Math.PI); // front left
			angles[1] = (c==0 && a==0) ? 0.0 : (Math.atan2(a, c) * 180 / Math.PI); // front right
			angles[2] = (c==0 && b==0) ? 0.0 : (Math.atan2(b, c) * 180 / Math.PI); // back right
			angles[3] = (d==0 && b==0) ? 0.0 : (Math.atan2(b, d) * 180 / Math.PI); // back left
		}

		Robot.drivetrain.setSteerDegrees(angles);
		Robot.drivetrain.setDriveSpeed(speeds);
    }

    // Called just before this Command runs the first time
    protected void initialize() {
		Robot.drivetrain.setSteerDegrees(0.0);
    }

    // Make this return true when this Command no longer needs to run execute()
    protected boolean isFinished() {
        return false;
    }

    // Called once after isFinished returns true
    protected void end() {
    }

    // Called when another command which requires one or more of the same
    // subsystems is scheduled to run
    protected void interrupted() {
    }
}
