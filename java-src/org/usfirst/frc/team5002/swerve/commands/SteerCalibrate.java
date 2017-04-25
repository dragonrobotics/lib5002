package org.usfirst.frc.team5002.swerve.commands;
import org.usfirst.frc.team5002.swerve.subsystems.*;

import edu.wpi.first.wpilibj.Timer;
import edu.wpi.first.wpilibj.command.Command;

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
    SwerveDrive drivetrain;
    Timer swerveTime;

    double minObservedADC[] = {1024.0, 1024.0, 1024.0, 1024.0};
    double maxObservedADC[] = {0.0, 0.0, 0.0, 0.0};

    public SteerCalibrate(SwerveDrive swerve) {
        drivetrain = swerve;
        swerveTime = new Timer();

        requires(drivetrain);
    }

    protected void execute() {
        drivetrain.setSteerSpeed(1.0);

        double currentADC[] = {
            drivetrain.fl.getCurrentSteerPositionRaw(),
            drivetrain.fr.getCurrentSteerPositionRaw(),
            drivetrain.bl.getCurrentSteerPositionRaw(),
            drivetrain.br.getCurrentSteerPositionRaw()
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
        drivetrain.fl.rezeroSteer();
        drivetrain.fr.rezeroSteer();
        drivetrain.bl.rezeroSteer();
        drivetrain.br.rezeroSteer();

        swerveTime.reset();
        swerveTime.start();
    }

    protected boolean isFinished() {
        return swerveTime.get() < 2.5;
    }

    protected void end() {
        drivetrain.fl.configSteerRange(maxObservedADC[0], minObservedADC[0]);
        drivetrain.fr.configSteerRange(maxObservedADC[1], minObservedADC[1]);
        drivetrain.bl.configSteerRange(maxObservedADC[2], minObservedADC[2]);
        drivetrain.br.configSteerRange(maxObservedADC[3], minObservedADC[3]);

        System.out.println("Minimum observed ADC values: "
            + Double.toString(minObservedADC[0])
            + " " + Double.toString(minObservedADC[1])
            + " " + Double.toString(minObservedADC[2])
            + " " + Double.toString(minObservedADC[3])
        );

        System.out.println("Maximum observed ADC values: "
            + Double.toString(maxObservedADC[0])
            + " " + Double.toString(maxObservedADC[1])
            + " " + Double.toString(maxObservedADC[2])
            + " " + Double.toString(maxObservedADC[3])
        );

        drivetrain.setSteerSpeed(0.0);

        drivetrain.loadConfig();
    }

    protected void interrupted() {}
}
