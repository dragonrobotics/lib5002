package org.usfirst.frc.team5002.swerve.subsystems;

import org.usfirst.frc.team5002.swerve.commands.KillDrivetrain;

import edu.wpi.first.wpilibj.command.Subsystem;

/**
 * SwerveDrive.java -- Swerve drive subsystem with 4 swerve modules in a square configuration.
 * @author Sebastian Mobo <stmobo@gmail.com>
 * @version 2.0, 04/22/2017
 */
public class SwerveDrive extends Subsystem {
    public SwerveModule fl;
    public SwerveModule fr;
    public SwerveModule bl;
    public SwerveModule br;

    /**
     * Both arrays should hold the IDs of the steer and drive controllers for
     * each swerve modules in this order:
     * Front-Left, Front-Right, Back-Left, Back-Right
     */
    public SwerveDrive(int[] steer, int[] drive) {
        fl = new SwerveModule("FL", steer[0], drive[0]);
        fr = new SwerveModule("FR", steer[1], drive[1]);
        bl = new SwerveModule("BL", steer[2], drive[2]);
        br = new SwerveModule("BR", steer[3], drive[3]);
    }

    protected void initDefaultCommand() {
        setDefaultCommand(new KillDrivetrain(this));
    }


    public void setSteerDegrees(double degrees_fl, double degrees_fr, double degrees_bl, double degrees_br) {
        fl.setSteerDegrees(degrees_fl);
        fr.setSteerDegrees(degrees_fr);
        bl.setSteerDegrees(degrees_bl);
        br.setSteerDegrees(degrees_br);
    }

    public void setSteerDegrees(double[] degrees) {
        fl.setSteerDegrees(degrees[0]);
        fr.setSteerDegrees(degrees[1]);
        bl.setSteerDegrees(degrees[2]);
        br.setSteerDegrees(degrees[3]);
    }

    public void setSteerDegrees(double degrees) {
        fl.setSteerDegrees(degrees);
        fr.setSteerDegrees(degrees);
        bl.setSteerDegrees(degrees);
        br.setSteerDegrees(degrees);
    }



    public void setDriveSpeed(double speed_fl, double speed_fr, double speed_bl, double speed_br) {
        fl.setDriveSpeed(speed_fl);
        fr.setDriveSpeed(speed_fr);
        bl.setDriveSpeed(speed_bl);
        br.setDriveSpeed(speed_br);
    }

    public void setDriveSpeed(double[] speed) {
        fl.setDriveSpeed(speed[0]);
        fr.setDriveSpeed(speed[1]);
        bl.setDriveSpeed(speed[2]);
        br.setDriveSpeed(speed[3]);
    }

    public void setDriveSpeed(double speed) {
        fl.setDriveSpeed(speed);
        fr.setDriveSpeed(speed);
        bl.setDriveSpeed(speed);
        br.setDriveSpeed(speed);
    }



    public void setSteerSpeed(double speed) {
        fl.setSteerSpeed(speed);
        fr.setSteerSpeed(speed);
        bl.setSteerSpeed(speed);
        br.setSteerSpeed(speed);
    }



    public void loadConfig() {
        fl.loadConfigValues();
        fr.loadConfigValues();
        bl.loadConfigValues();
        br.loadConfigValues();
    }

    public void updateSD() {
        fl.updateSD();
        fr.updateSD();
        bl.updateSD();
        br.updateSD();
    }
}
