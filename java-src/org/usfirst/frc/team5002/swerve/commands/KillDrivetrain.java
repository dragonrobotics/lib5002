package org.usfirst.frc.team5002.swerve.commands;

import org.usfirst.frc.team5002.swerve.subsystems.*;
import edu.wpi.first.wpilibj.command.InstantCommand;

/**
 * KillDrivetrain.java -- Simple command for halting robot movement.
 * Useful as a default.
 *
 * @author Sebastian Mobo <stmobo@gmail.com>
 * @version 1.1, 04/22/2017
 */
public class KillDrivetrain extends InstantCommand {
    SwerveDrive drivetrain;

    public KillDrivetrain(SwerveDrive swerve) {
        drivetrain = swerve;
        requires(drivetrain);
    }

    protected void execute() {
        drivetrain.setDriveSpeed(0.0);
    }

    protected void initialize() {}
    protected void end() {}
    protected void interrupted() {}
}
