package org.usfirst.frc.team5002.robot.commands;

import org.usfirst.frc.team5002.robot.Robot;

import edu.wpi.first.wpilibj.command.Command;
import edu.wpi.first.wpilibj.DriverStation;

/**
 *
 */
public class SteerRezero extends InstantCommand {

    public SteerRezero() {
        requires(Robot.drivetrain)
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
