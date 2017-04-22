package org.usfirst.frc.team5002.swerve.commands;

import org.usfirst.frc.team5002.robot.Robot;
import org.usfirst.frc.team5002.robot.subsystems.swerve.*;

import edu.wpi.first.wpilibj.command.Command;
import edu.wpi.first.wpilibj.DriverStation;

/**
 *
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
