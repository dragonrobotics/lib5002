package org.usfirst.frc.team5002.swerve.commands;

import org.usfirst.frc.team5002.robot.Robot;
import org.usfirst.frc.team5002.robot.subsystems.swerve.*;

import edu.wpi.first.wpilibj.command.Command;
import edu.wpi.first.wpilibj.DriverStation;

/**
 * KillDrivetrain.java -- Simple command for halting robot movement.
 * Useful as a default.
 *
 * @author Sebastian Mobo <stmobo@gmail.com>
 * @version 1.1, 04/22/2017
 */
public class KillDrivetrain extends InstantCommand {

    public KillDrivetrain() {
        requires(Robot.drivetrain);
    }
    protected void execute() {
        Robot.drivetrain.setDriveSpeed(0.0);
    }

    protected void initialize() {}
    protected void end() {}
    protected void interrupted() {}
}
