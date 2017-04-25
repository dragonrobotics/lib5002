package org.usfirst.frc.team5002.swerve.commands;

import org.usfirst.frc.team5002.swerve.subsystems.*;
import edu.wpi.first.wpilibj.command.InstantCommand;

/**
 * SteerRezero.java -- Quickly rezero all swerve modules.
 * Saves each swerve module's current position to persistent storage as a zeroing value.
 *
 * @author Sebastian Mobo <stmobo@gmail.com>
 * @version 1.0, 04/22/2017
 */
public class SteerRezero extends InstantCommand {
    SwerveDrive drivetrain;

    public SteerRezero(SwerveDrive swerve) {
        drivetrain = swerve;
        requires(drivetrain);
    }

    protected void execute() {
        drivetrain.fl.rezeroSteer();
        drivetrain.fr.rezeroSteer();
        drivetrain.bl.rezeroSteer();
        drivetrain.br.rezeroSteer();
    }

    protected void initialize() {}
    protected void end() { drivetrain.loadConfig(); }
    protected void interrupted() { end(); }
}
