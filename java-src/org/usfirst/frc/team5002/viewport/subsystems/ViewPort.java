package org.usfirst.frc.team5002.viewport.subsystems;

import edu.wpi.first.wpilibj.command.Subsystem;
import edu.wpi.first.wpilibj.CameraServer;
import edu.wpi.first.wpilibj.networktables.NetworkTablesJNI;
import edu.wpi.first.wpilibj.smartdashboard.SmartDashboard;
import edu.wpi.first.wpilibj.smartdashboard.SendableChooser;

import org.usfirst.frc.team5002.robot.Robot;

import edu.wpi.cscore.*;

/**
 * ViewPort.java -- subsystem for switchable camera views
 * @author stmobo <stmobo@gmail.com>
 * @version 2.0, 04/21/2017
 */
public class ViewPort extends Subsystem {
	private UsbCamera cam1;
	private UsbCamera cam2;

	private int currentSrc;

    private VideoSink server;
    private CvSink dummy1;
    private CvSink dummy2;

    public ViewPort() {
        cam1 = CameraServer.getInstance().startAutomaticCapture(0);
        cam2 = CameraServer.getInstance().startAutomaticCapture(1);

        cam1.setFPS(15);
        cam1.setResolution(240, 320);

        cam2.setFPS(15);
        cam2.setResolution(240, 320);

        server = CameraServer.getInstance().getServer();

        dummy1 = new CvSink("dummy1");
        dummy2 = new CvSink("dummy2");

        dummy1.setSource(cam1);
        dummy1.setEnabled(true);

        dummy2.setSource(cam2);
        dummy2.setEnabled(true);

        currentSrc = 0;
	}

    public int getCurrentSource() {
        return currentSrc;
    }

    public void nextView() {
    	if(currentSrc == 0) {
    		server.setSource(cam2);
    		currentSrc = 1;
    	} else {
    		server.setSource(cam1);
    		currentSrc = 0;
    	}
    }

	public void initDefaultCommand() {}
}
