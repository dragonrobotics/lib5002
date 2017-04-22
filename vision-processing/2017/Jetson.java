package org.usfirst.frc.team5002.robot.subsystems;

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

import edu.wpi.first.wpilibj.networktables.*;

/**
 * Jetson.java -- Subsystem class for interfacing with a remote Jetson running VisProc code.
 * @author Sebastian Mobo <stmobo@gmail.com>
 * @version 1.0, 04/22/2017
 */
public class Jetson extends Subsystem {
    private NetworkTable table;

    public Jetson() {
        table = NetworkTable.getTable("vision");
    }

    /**
     * Returns true if the jetson is connected, false if not.
     */
    public boolean getJetsonStatus() {
        return table.getBoolean("connected", false);
    }

    /**
     * Returns true if the vision targets have been detected.
     */
    public boolean canSeeTargets() {
        return table.getBoolean("valid", false);
    }

    /**
     * Returns the distance to the vision targets, as reported by the Jetson.
     *
     * If the jetson is not connected, this returns 0.
     */
    public double getVisualDistance() {
        return table.getNumber("distance", 0.0);
    }

    /**
     * Returns the lateral (left/right) offset from the vision targets,
     * as reported by the Jetson.
     */
    public double getVisualOffset() {
        return table.getNumber("offset", 0.0);
    }

    public void initDefaultCommand() {}
}
