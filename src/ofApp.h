#pragma once

#include "ofMain.h"
#include "ode/ode.h"



class ofApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    ofEasyCam cam;

    dWorldID world;
    dSpaceID space;
    dBodyID player;
    dJointGroupID contactgroup;
    dGeomID ground;
    dSpaceID player_space;
    dGeomID playerGeom;
    dGeomID floor_box[5];
    dGeomID wall_box[3];
    dGeomID hazard[1];
    dGeomID objectiveSingle[2];
    dGeomID objectiveMulti[3];
    bool objectiveFlagSingle[2];
    bool objectiveFlagMulti[3];
    dReal xspeed, yspeed, zspeed, jumpForce;
    dReal player_x, player_y, player_z;
    dVector3 player_pos;
    dContact test;
    int aFlag, groundFlag;
    int floatTimer, objectiveTimerSingle;
    bool useFlag, glideFlag, floatFlag, jumpFlag, hazFlag, hazMove;

    void collide (dGeomID o1, dGeomID o2);
    void jump();
    void reset();

    void drawBox(const dReal*pos_ode, const dQuaternion rot_ode, const dReal*sides_ode);
    void drawBox(const dReal*pos_ode, const dReal*sides_ode);
};

static void nearCallback (void *, dGeomID o1, dGeomID o2);
extern ofApp *myApp;
