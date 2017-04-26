package org.usfirst.frc.team5002.viewport.subsystems;

import edu.wpi.first.wpilibj.command.Subsystem;
import edu.wpi.first.wpilibj.CameraServer;

import edu.wpi.cscore.*;

/**
 * ViewPort.java -- subsystem for switchable camera views
 * @author stmobo <stmobo@gmail.com>
 * @version 2.0, 04/21/2017
 */
public class ViewPort extends Subsystem {
	private class CameraPair {
        public UsbCamera camera;
        public CvSink sink;

        private int id;

        public CameraPair(int i) {
            id = i;
            camera = CameraServer.getInstance().startAutomaticCapture(i);
            sink = new CvSink("dummy-"+Integer.toString(i));

            camera.setFPS(15);
            camera.setResolution(240, 320);

            sink.setSource(camera);
            sink.setEnabled(true);
        }

        public int getID() { return id; }
    };

    CameraPair[] sources;
    private int currentSrc;
    private VideoSink server;

    public ViewPort() {
        sources = new CameraPair[2];
        sources[0] = new CameraPair(0);
        sources[1] = new CameraPair(1);

        server = CameraServer.getInstance().getServer();
        currentSrc = 0;
	}

    public int getCurrentSource() {
        return currentSrc;
    }

    public void nextView() {
        if(currentSrc == (sources.length-1)) {
            currentSrc = 0;
        } else {
            currentSrc += 1;
        }

        server.setSource(sources[currentSrc].camera);
    }

    public void prevView() {
        if(currentSrc == 0) {
            currentSrc = sources.length-1;
        } else {
            currentSrc -= 1;
        }

        server.setSource(sources[currentSrc].camera);
    }

	public void initDefaultCommand() {}
}
