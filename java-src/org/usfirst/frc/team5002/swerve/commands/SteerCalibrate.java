package org.usfirst.frc.team5002.swerve.commands;

import org.usfirst.frc.team5002.robot.Robot;

import edu.wpi.first.wpilibj.command.Command;
import edu.wpi.first.wpilibj.DriverStation;
import edu.wpi.first.wpilibj.Timer;

/**
 * SteerCalibrate.java -- Find and record swerve module ranges and zeroing values.
 *
 * Steers the swerve modules through (at least) one rotation and tracks min/max ADC values,
 * then saves these values, along with the detected zeroing values, to persistent storage.
 *
 * @author Sebastian Mobo <stmobo@gmail.com>
 * @version 1.1, 04/22/2017
 */
public class SteerCalibrate extends Command {

    Timer swerveTime;
    double minObservedADC[] = {1024.0, 1024.0, 1024.0, 1024.0};
    double maxObservedADC[] = {0.0, 0.0, 0.0, 0.0};

    public SteerCalibrate() {
        requires(Robot.drivetrain);
        swerveTime = new Timer();
    }

    protected void execute() {
        Robot.drivetrain.setSteerSpeed(1.0);

        int currentADC[] = {
            Robot.drivetrain.fl.getCurrentSteerPositionRaw(),
            Robot.drivetrain.fr.getCurrentSteerPositionRaw(),
            Robot.drivetrain.bl.getCurrentSteerPositionRaw(),
            Robot.drivetrain.br.getCurrentSteerPositionRaw()
        };

        for (int i=0; i<4; i++) {
            if(minObservedADC[i] > currentADC[i]) {
                minObservedADC[i] = currentADC[i];
            } else if(maxObservedADC[i] < currentADC[i]) {
                maxObservedADC[i] = currentADC[i];
            }
        }
    }

    protected void initialize() {
        Robot.drivetrain.fl.rezeroSteer();
        Robot.drivetrain.fr.rezeroSteer();
        Robot.drivetrain.bl.rezeroSteer();
        Robot.drivetrain.br.rezeroSteer();

        swerveTime.reset();
        swerveTime.start();
    }

    protected boolean isFinished() {
        return swerveTime.get() < 2.5;
    }

    protected void end() {
        Robot.drivetrain.fl.configSteerRange(maxObservedADC[0], minObservedADC[0]);
        Robot.drivetrain.fr.configSteerRange(maxObservedADC[1], minObservedADC[1]);
        Robot.drivetrain.bl.configSteerRange(maxObservedADC[2], minObservedADC[2]);
        Robot.drivetrain.br.configSteerRange(maxObservedADC[3], minObservedADC[3]);

        System.out.println("Minimum observed ADC values: "
            + Integer.toString(minObservedADC[0])
            + " " + Integer.toString(minObservedADC[1])
            + " " + Integer.toString(minObservedADC[2])
            + " " + Integer.toString(minObservedADC[3])
        );

        System.out.println("Maximum observed ADC values: "
            + Integer.toString(maxObservedADC[0])
            + " " + Integer.toString(maxObservedADC[1])
            + " " + Integer.toString(maxObservedADC[2])
            + " " + Integer.toString(maxObservedADC[3])
        );

        Robot.drivetrain.setSteerSpeed(0.0);

        Robot.drivetrain.loadConfig();
    }

    protected void interrupted() {}
}
