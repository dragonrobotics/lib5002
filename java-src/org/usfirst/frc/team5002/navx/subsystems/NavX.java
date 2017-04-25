package org.usfirst.frc.team5002.navx.subsystems;

import org.usfirst.frc.team5002.robot.Robot;
import org.usfirst.frc.team5002.robot.RobotMap;

import com.kauailabs.navx.frc.AHRS;

import edu.wpi.first.wpilibj.smartdashboard.*;
import edu.wpi.first.wpilibj.command.Subsystem;
import edu.wpi.first.wpilibj.DriverStation;
import edu.wpi.first.wpilibj.AnalogInput;
import edu.wpi.first.wpilibj.SPI;
import edu.wpi.first.wpilibj.SerialPort;
import edu.wpi.first.wpilibj.Timer;
import edu.wpi.first.wpilibj.SPI.Port;


import org.opencv.core.*;
import org.opencv.core.MatOfPoint;
import org.opencv.imgproc.Imgproc;

public class NavX extends Subsystem {
    private double startYaw;
    public static AHRS navx;
    private AHRS.BoardYawAxis yawAxis;

    public NavX() {
        try {
			/* NOTE: With respect to the NavX, the robot's front is in the -X direction.
			 * The robot's right side is in the +Y direction,
			 * and the robot's top side is in the +Z direction as usual.
		     * Clockwise rotation = increasing yaw.
		     */
			navx = new AHRS(SerialPort.Port.kMXP);

            navx.zeroYaw();
            startYaw = navx.getYaw();
            yawAxis = navx.getBoardYawAxis();

		} catch (RuntimeException ex) {
			DriverStation.reportError("Error instantiating navX MXP:  " + ex.getMessage(), true);

            navx = null;
            startYaw = 0;
            yawAxis = null;
		}
    }

    /* A more reliable form of navx.getAngle() */
	public double getRobotHeading() {
		if(navx != null) {
			if(Math.abs(navx.getAngle()) <= 0.001) {
				return (navx.getYaw() - startYaw);
			} else {
				return navx.getAngle();
			}
		} else {
			return 0;
		}
	}

    /**
     * Rotates a field control vector to the robot reference frame.
     * @return a two-element array containing the forward and horizontal control axes (in order)
     */
    public double[] getFOCVector(double x, double y) {
        double hdg = Robot.sensors.getRobotHeading();

        double[] ret = new double[2];
        ret[0] = (x * -Math.sin(hdg*Math.PI/180.0)) + (y * Math.cos(hdg*Math.PI/180.0));
        ret[1] = (x * Math.cos(hdg*Math.PI/180.0)) + (y * Math.sin(hdg*Math.PI/180.0));
    }

    public void updateSD() {
        SmartDashboard.putNumber("Start Yaw", startYaw);
		if(navx != null) {
			SmartDashboard.putBoolean("NavX Present", true);
			SmartDashboard.putBoolean("Calibrating", navx.isCalibrating());
			SmartDashboard.putBoolean("Connected", navx.isConnected());

			SmartDashboard.putNumber("X-Displacement", navx.getDisplacementX());
			SmartDashboard.putNumber("Y-Displacement", navx.getDisplacementY());

			SmartDashboard.putNumber("Heading", navx.getAngle());
			SmartDashboard.putNumber("Compass", navx.getCompassHeading());
			SmartDashboard.putNumber("Yaw", navx.getYaw());
			SmartDashboard.putNumber("Fused", navx.getFusedHeading());
			if(Robot.oi.focEnabled) {
				SmartDashboard.putString("Control Mode", "Field-Oriented");
			} else {
				SmartDashboard.putString("Control Mode", "Robot-Oriented (FOC available)");
			}
		} else {
			SmartDashboard.putBoolean("NavX Present", false);
			SmartDashboard.putString("Control Mode", "Robot-Oriented (FOC unavailable)");
		}
    }

    public void initDefaultCommand() {
    }
}
