#include "ofApp.h"
#define LENGTH 0.3      // player length
#define STARTZ 12      // starting height of player
#define CMASS 0.5         // player mass

static const dVector3 yunit = { 0, 1, 0 }, zunit = { 0, 0, 1 };

//--------------------------------------------------------------
void ofApp::setup(){

    // initialise variables to default values
    int i, n;
    dMass m;
    xspeed=0, yspeed=0, jumpForce=12.5;
    aFlag=0;
    floatTimer=0, objectiveTimerSingle=0;
    useFlag=false, glideFlag=false, floatFlag=false, jumpFlag=false, hazFlag=true, hazMove=false;
    groundFlag = 0;
    n = sizeof(objectiveFlagSingle)/sizeof(objectiveFlagSingle[0]);
    for (i=0; i<n; i++){
        objectiveFlagSingle[i] = false;
    }
    n = sizeof(objectiveFlagMulti)/sizeof(objectiveFlagMulti[0]);
    for (i=0; i<n; i++){
        objectiveFlagMulti[i] = false;
    }

    // create world
    dInitODE2(0);
    world = dWorldCreate();
    space = dHashSpaceCreate (0);
    contactgroup = dJointGroupCreate (0);
    dWorldSetGravity (world,0,0,-0.5);
    ground = dCreatePlane (space,0,0,1,0);

    // player body
    player = dBodyCreate (world);
    dBodySetPosition (player,0,0,STARTZ);
    dMassSetSphere(&m,1,LENGTH/2);
    dMassAdjust (&m,CMASS);
    dBodySetMass (player,&m);
    playerGeom = dCreateSphere (0,LENGTH/2);
    dGeomSetBody (playerGeom,player);

    // create player space and add it to the top level space
    player_space = dSimpleSpaceCreate (space);
    dSpaceSetCleanup (player_space,0);
    dSpaceAdd (player_space,playerGeom);

    // level
    floor_box[0] = dCreateBox (space,10,10,0.25);
    floor_box[1] = dCreateBox (space,5,5,0.25);
    floor_box[2] = dCreateBox (space,5,10,0.25);
    floor_box[3] = dCreateBox (space,15,10,0.25);
    floor_box[4] = dCreateBox (space,5,5,0.25);
    dGeomSetPosition (floor_box[0],0,0,10);
    dGeomSetPosition (floor_box[1],-5,-5,11.5);
    dGeomSetPosition (floor_box[2],-10,0,10);
    dGeomSetPosition (floor_box[3],-2.5,0,8);
    dGeomSetPosition (floor_box[4],-5,-9,13);

    // lights or ability indicators
    wall_box[0] = dCreateBox (space,0.5,0.5,0.5);
    wall_box[1] = dCreateBox (space,0.5,0.5,0.5);
    wall_box[2] = dCreateBox (space,0.5,0.5,0.5);
    dGeomSetPosition (wall_box[0],-3,-1,10.25);
    dGeomSetPosition (wall_box[1],-3,0,10.25);
    dGeomSetPosition (wall_box[2],-3,1,10.25);

    // objectives
    objectiveSingle[0] = dCreateBox (space,0.5,0.5,0.5);
    objectiveSingle[1] = dCreateBox (space,0.5,0.5,0.5);
    dGeomSetPosition (objectiveSingle[0],3,0,10.25);
    dGeomSetPosition (objectiveSingle[1],0.5,-3,10.25);

    objectiveMulti[0] = dCreateBox (space,0.25,0.25,0.25);
    objectiveMulti[1] = dCreateBox (space,0.25,0.25,0.25);
    objectiveMulti[2] = dCreateBox (space,0.25,0.25,0.25);
    dGeomSetPosition (objectiveMulti[0],1,3,10.125);
    dGeomSetPosition (objectiveMulti[1],0,3,10.125);
    dGeomSetPosition (objectiveMulti[2],-1,3,10.125);

    // hazards
    hazard[0] = dCreateBox (space,0.5,0.5,0.5);
    dGeomSetPosition (hazard[0],-11.5,-4,10.275);

    // Set up the OpenFrameworks camera
    ofVec3f upVector;
    upVector.set(0, 0, 1);
    cam.setAutoDistance(false);
    cam.setNearClip(0.01);
    cam.setPosition(10,10,20);
    cam.lookAt({0,0,10},upVector);
    cam.setUpAxis(upVector);

    dAllocateODEDataForThread(dAllocateMaskAll);
}

//--------------------------------------------------------------
void ofApp::update(){
    int i, n, c;

    // world collision testing
    n = sizeof(floor_box)/sizeof(floor_box[0]);
    for (i=0; i<n; i++){
        groundFlag += dCollide (floor_box[i],playerGeom,1,&test.geom,sizeof(dContact));
    }
    if (groundFlag != 0) { useFlag=true; glideFlag=false; floatFlag=false; jumpFlag=true; groundFlag = 0;}
    else { jumpFlag=false; }

    c = dCollide (ground,playerGeom,1,&test.geom,sizeof(dContact));
    if(c != 0){ reset(); }

    // objective testing
    n = sizeof(objectiveSingle)/sizeof(objectiveSingle[0]);
    for (i=0; i<n; i++){
        c = dCollide (objectiveSingle[i],playerGeom,1,&test.geom,sizeof(dContact));
        if(c != 0){
            objectiveFlagSingle[i] = true;
            if(objectiveSingle[i] == objectiveSingle[1]){
                objectiveTimerSingle=500;
            }
        }
    }

    n = sizeof(objectiveMulti)/sizeof(objectiveMulti[0]);
    for (i=0; i<n; i++){
        c = dCollide (objectiveMulti[i],playerGeom,1,&test.geom,sizeof(dContact));
        if(c != 0){
            objectiveFlagMulti[i] = true;
        }
    }

    if(objectiveTimerSingle > 0){
        objectiveTimerSingle -= 1;
        hazFlag = false;
    }
    else{
        objectiveFlagSingle[1] = false;
        hazFlag = true;
    }

    // hazards
    if (hazFlag){
        if (dGeomGetPosition(hazard[0])[1] <= -4){ hazMove = true; }
        else if(dGeomGetPosition(hazard[0])[1] >= 4){ hazMove = false; }
        if (hazMove){
            dGeomSetPosition(hazard[0], dGeomGetPosition(hazard[0])[0], dGeomGetPosition(hazard[0])[1] + 0.1, dGeomGetPosition(hazard[0])[2]);
        }
        else{
            dGeomSetPosition(hazard[0], dGeomGetPosition(hazard[0])[0], dGeomGetPosition(hazard[0])[1] - 0.1, dGeomGetPosition(hazard[0])[2]);
        }

        n = sizeof(hazard)/sizeof(hazard[0]);
        for (i=0; i<n; i++){
            c = dCollide (hazard[i],playerGeom,1,&test.geom,sizeof(dContact));
            if(c != 0){
                reset();
            }
        }
    }

    // movement
    zspeed = dBodyGetLinearVel(player)[2];

    dBodyCopyPosition(player, player_pos);
    player_x = player_pos[0];
    player_y = player_pos[1];
    player_z = player_pos[2];

    if (glideFlag) {
        dBodySetLinearVel(player, xspeed*1.5, yspeed*1.5, zspeed*0.9);
    }
    else if (floatFlag){
        dBodyAddForce(player, 0, 0, CMASS * 0.5);
        floatTimer -= 1;
        if (floatTimer == 0) {floatFlag = false;}
    }
    else { dBodySetLinearVel(player, xspeed, yspeed, zspeed); }

    // world simulation
    dSpaceCollide (space,0,&nearCallback);
    dWorldStep (world,0.05);

    // remove all contact joints
    dJointGroupEmpty (contactgroup);

}
//--------------------------------------------------------------
void ofApp::jump(){

    // set up the spacebar to do multiple jump and use selected ability
    if (jumpFlag) { dBodyAddForce(player, 0, 0, jumpForce); jumpFlag=false; }
    else {
        if(useFlag){
            switch(aFlag){
            case 1:
                dBodyAddForce(player, 0, 0, jumpForce);
                break;
            case 2:
                glideFlag=true;
                break;
            case 3:
                floatFlag=true;
                dBodySetLinearVel(player, xspeed, yspeed, zspeed);
                floatTimer = 75;
                break;
            }
        useFlag=false;
        }
    }
}
//--------------------------------------------------------------
void ofApp::reset(){

    // reset variables to default values
    int i, n;
    dBodySetPosition(player,0,0,STARTZ);
    xspeed=0, yspeed=0, jumpForce=12.5;
    aFlag=0;
    floatTimer=0, objectiveTimerSingle=0;
    useFlag=false, glideFlag=false, floatFlag=false, jumpFlag=false, hazFlag=true, hazMove=false;
    groundFlag = 0;
    n = sizeof(objectiveFlagSingle)/sizeof(objectiveFlagSingle[0]);
    for (i=0; i<n; i++){
        objectiveFlagSingle[i] = false;
    }
    n = sizeof(objectiveFlagMulti)/sizeof(objectiveFlagMulti[0]);
    for (i=0; i<n; i++){
        objectiveFlagMulti[i] = false;
    }
}
//--------------------------------------------------------------
void ofApp::draw(){
    int i, n;
    dReal c;
    dGeomID currentBox;
    dVector3 lengths; dQuaternion rots;

    // draw the scene
    ofBackground(20);
    cam.begin();

    ofEnableDepthTest();

    ofPushMatrix();

    ofSetColor(ofColor::lightGrey);
    ofDrawGrid(0.2f,100, false, false,false,true);
    ofDrawAxis(10);

    // player
    ofSetColor(ofColor::yellow);//FRONT
    const dReal sides[3] = {LENGTH,LENGTH,LENGTH};
    const dReal* pos_ode = dBodyGetPosition(player);
    drawBox(pos_ode, sides);

    // level (colour is determined by z value)
    ofSetColor(ofColor(0,0,255));
    n = sizeof(floor_box)/sizeof(floor_box[0]);
    for (i=0; i<n; i++){
        currentBox = floor_box[i];
        c = (dGeomGetPosition(currentBox)[2] - 8) / 10;
        dGeomBoxGetLengths(currentBox,lengths);
        dGeomGetQuaternion(currentBox,rots);
        ofSetColor(ofFloatColor(0 + c/2,0 + c/2,0.5 + c));
        drawBox(dGeomGetPosition(currentBox),rots,lengths);
    }

    // lights or ability indicators
    n = sizeof(wall_box)/sizeof(wall_box[0]);
    for (i=0; i<n; i++){
        currentBox = wall_box[i];
        dGeomBoxGetLengths(currentBox,lengths);
        dGeomGetQuaternion(currentBox,rots);
        if(i+1 == aFlag){ ofSetColor(ofColor::white); } else { ofSetColor(ofColor::grey); }
        drawBox(dGeomGetPosition(currentBox),rots,lengths);
    }

    // objectives
    n = sizeof(objectiveSingle)/sizeof(objectiveSingle[0]);
    for (i=0; i<n; i++){
        currentBox = objectiveSingle[i];
        dGeomBoxGetLengths(currentBox,lengths);
        dGeomGetQuaternion(currentBox,rots);
        if(objectiveFlagSingle[i]){ ofSetColor(ofColor::green); } else { ofSetColor(ofColor::grey); }
        drawBox(dGeomGetPosition(currentBox),rots,lengths);
    }

    if (objectiveTimerSingle != 0){
        ofSetColor(ofColor::green);
        ofDrawBox(-0.5, -3, 10.125 + (0.0025 * objectiveTimerSingle) ,0.25, 0.25, 0.005 * objectiveTimerSingle);
    }


    n = sizeof(objectiveMulti)/sizeof(objectiveMulti[0]);
    for (i=0; i<n; i++){
        currentBox = objectiveMulti[i];
        dGeomBoxGetLengths(currentBox,lengths);
        dGeomGetQuaternion(currentBox,rots);
        if(objectiveFlagMulti[i]){ ofSetColor(ofColor::orange); } else { ofSetColor(ofColor::grey); }
        if((i == 0 || i  == 1 || i == 2) && (objectiveFlagMulti[0] && objectiveFlagMulti[1] && objectiveFlagMulti[2])) { ofSetColor(ofColor::green); }
        drawBox(dGeomGetPosition(currentBox),rots,lengths);
    }

    // hazards
    n = sizeof(hazard)/sizeof(hazard[0]);
    for (i=0; i<n; i++){
        currentBox = hazard[i];
        dGeomBoxGetLengths(currentBox,lengths);
        dGeomGetQuaternion(currentBox,rots);
        if(hazFlag){ ofSetColor(ofColor::red); } else { ofSetColor(ofColor::grey); }
        drawBox(dGeomGetPosition(currentBox),rots,lengths);
    }

    ofDisableDepthTest();
    cam.end();

    ofPopMatrix();
}
//--------------------------------------------------------------
void ofApp::exit() {
        dGeomDestroy (playerGeom);
        dJointGroupDestroy (contactgroup);
        dSpaceDestroy (space);
        dWorldDestroy (world);
        dCloseODE();
}
//--------------------------------------------------------------
static void nearCallback (void *, dGeomID o1, dGeomID o2) {

    myApp->collide(o1,o2);
}

void ofApp::drawBox(const dReal*pos_ode, const dQuaternion rot_ode, const dReal*sides_ode)
{
    ofBoxPrimitive b;
    b.setScale(glm::vec3(0.01*sides_ode[0],0.01*sides_ode[1],0.01*sides_ode[2]));
    b.setGlobalOrientation(glm::quat(rot_ode[0],rot_ode[1],rot_ode[2],rot_ode[3]));
    b.setGlobalPosition(glm::vec3(pos_ode[0],pos_ode[1],pos_ode[2]));
    b.draw();

}

void ofApp::drawBox(const dReal*pos_ode, const dReal*sides_ode)
{
    // method is overloaded to draw the player as a box without having it rotate
    ofBoxPrimitive b;
    b.setScale(glm::vec3(0.01*sides_ode[0],0.01*sides_ode[1],0.01*sides_ode[2]));
    b.setGlobalPosition(glm::vec3(pos_ode[0],pos_ode[1],pos_ode[2]));
    b.draw();

}

void ofApp::collide(dGeomID o1, dGeomID o2)
{
  int i,n;

  // collisions
  const int N = 10;
  dContact contact[N];
  n = dCollide (o1,o2,N,&contact[0].geom,sizeof(dContact));
  if (n > 0) {
    for (i=0; i<n; i++) {
      contact[i].surface.mode = dContactSlip1 | dContactSlip2 |
        dContactSoftERP | dContactSoftCFM | dContactApprox1;
      contact[i].surface.mu = dInfinity;
      contact[i].surface.slip1 = 0.1;
      contact[i].surface.slip2 = 0.1;
      contact[i].surface.soft_erp = 0.0;
      contact[i].surface.soft_cfm = 0.0;
      dJointID c = dJointCreateContact (world,contactgroup,&contact[i]);
      dJointAttach (c,
                    dGeomGetBody(contact[i].geom.g1),
                    dGeomGetBody(contact[i].geom.g2));
    }
  }
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    // player input
    switch(key) {
    case 's':
      xspeed = 1.0;
      break;
    case 'w':
      xspeed = -1.0;
      break;
    case 'a':
      yspeed = -1.0;
      break;
    case 'd':
      yspeed = 1.0;
      break;
    case ' ':
        jump();
        break;
    case '1':
        if(useFlag){
            if(aFlag == 1){
                aFlag = 0;
            }
            else{
                aFlag = 1;
            }
        }
        break;
    case '2':
        if(useFlag){
            if(aFlag == 2){
                aFlag = 0;
            }
            else{
                aFlag = 2;
            }
        }
        break;
    case '3':
        if(useFlag){
            if(aFlag == 3){
                aFlag = 0;
            }
            else{
                aFlag = 3;
            }
        }
        break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

    switch(key) {
    case 's':
      xspeed = 0;
      break;
    case 'w':
      xspeed = 0;
      break;
    case 'a':
      yspeed = 0;
      break;
    case 'd':
      yspeed = 0;
      break;
    case 'o':
        ofExit();
        break;
    case 'r':
        reset();
        break;
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
