package org.usfirst.frc.team5002.robot.subsystems.swerve;

import org.usfirst.frc.team5002.robot.RobotMap;
import org.usfirst.frc.team5002.robot.commands.KillDrivetrain;
import com.ctre.CANTalon;
import com.ctre.CANTalon.FeedbackDevice;
import com.ctre.CANTalon.TalonControlMode;
import edu.wpi.first.wpilibj.DriverStation;
import edu.wpi.first.wpilibj.command.Subsystem;
import edu.wpi.first.wpilibj.networktables.NetworkTablesJNI;
import edu.wpi.first.wpilibj.smartdashboard.SmartDashboard;
import edu.wpi.first.wpilibj.smartdashboard.SendableChooser;

/**
 * @author stmobo
 * SwerveDrive.java -- swerve drive subsystem
 */
public class SwerveDrive extends Subsystem {
    SwerveModule fl;
    SwerveModule fr;
    SwerveModule bl;
    SwerveModule br;

    public SwerveDrive() {
        fl = new SwerveModule("FL", RobotMap.fl_steer, RobotMap.fl_drive);
        fr = new SwerveModule("FR", RobotMap.fr_steer, RobotMap.fr_drive);
        bl = new SwerveModule("BL", RobotMap.bl_steer, RobotMap.bl_drive);
        br = new SwerveModule("BR", RobotMap.br_steer, RobotMap.br_drive);
    }

    public void setSteerDegrees(double degrees_fl, double degrees_fr, double degrees_bl, double degrees_br) {
        fl.setSteerDegrees(degrees_fl);
        fr.setSteerDegrees(degrees_fr);
        bl.setSteerDegrees(degrees_bl);
        br.setSteerDegrees(degrees_br);
    }

    public void setDriveSpeed(double speed_fl, double speed_fr, double speed_bl, double speed_br) {
        fl.setDriveSpeed(speed_fl);
        fr.setDriveSpeed(speed_fr);
        bl.setDriveSpeed(speed_bl);
        br.setDriveSpeed(speed_br);
    }

    public void setSteerDegrees(double[] degrees) {
        fl.setSteerDegrees(degrees[0]);
        fr.setSteerDegrees(degrees[1]);
        bl.setSteerDegrees(degrees[2]);
        br.setSteerDegrees(degrees[3]);
    }

    public void setDriveSpeed(double[] speed) {
        fl.setDriveSpeed(speed[0]);
        fr.setDriveSpeed(speed[1]);
        bl.setDriveSpeed(speed[2]);
        br.setDriveSpeed(speed[3]);
    }

    public void setSteerDegrees(double degrees) {
        fl.setSteerDegrees(degrees);
        fr.setSteerDegrees(degrees);
        bl.setSteerDegrees(degrees);
        br.setSteerDegrees(degrees);
    }

    public void setDriveSpeed(double speed) {
        fl.setDriveSpeed(speed);
        fr.setDriveSpeed(speed);
        bl.setDriveSpeed(speed);
        br.setDriveSpeed(speed);
    }

    public void updateSD() {
        fl.updateSD();
        fr.updateSD();
        bl.updateSD();
        br.updateSD();
    }
}
