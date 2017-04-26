package org.usfirst.frc.team5002.swerve.commands;

import org.usfirst.frc.team5002.swerve.subsystems.*;
import edu.wpi.first.wpilibj.command.Command;

/**
 * LockDown.java -- Puts all 4 swerve modules into an X orientation, preventing
 * any kind of movement (voluntary and not).
 *
 * @author Sebastian Mobo <stmobo@gmail.com>
 * @version 1.0, 04/25/2017
 */
public class LockDown extends Command {
    SwerveDrive drivetrain;

    public LockDown(SwerveDrive swerve) {
        drivetrain = swerve;
        requires(drivetrain);
    }

    protected void execute() { initialize(); }

    protected void initialize() {
        drivetrain.fl.setSteerDegrees(135);
        drivetrain.fr.setSteerDegrees(-135);
        drivetrain.bl.setSteerDegrees(45);
        drivetrain.br.setSteerDegrees(-45);

        drivetrain.setDriveSpeed(0);
    }

    protected boolean isFinished() { return false; }
    protected void end() {}
    protected void interrupted() {}
}
