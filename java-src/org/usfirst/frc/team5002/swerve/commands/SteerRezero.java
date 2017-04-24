package org.usfirst.frc.team5002.swerve.commands;

import org.usfirst.frc.team5002.robot.Robot;
import org.usfirst.frc.team5002.robot.subsystems.swerve.*;

import edu.wpi.first.wpilibj.command.Command;
import edu.wpi.first.wpilibj.DriverStation;

/**
 * SteerRezero.java -- Quickly rezero all swerve modules.
 * Saves each swerve module's current position to persistent storage as a zeroing value.
 *
 * @author Sebastian Mobo <stmobo@gmail.com>
 * @version 1.0, 04/22/2017
 */
public class SteerRezero extends InstantCommand {

    public SteerRezero() {
        requires(Robot.drivetrain);
    }
    protected void execute() {
        Robot.drivetrain.fl.rezeroSteer();
        Robot.drivetrain.fr.rezeroSteer();
        Robot.drivetrain.bl.rezeroSteer();
        Robot.drivetrain.br.rezeroSteer();
    }

    protected void initialize() {}
    protected void end() { Robot.drivetrain.loadConfig(); }
    protected void interrupted() { end(); }
}
