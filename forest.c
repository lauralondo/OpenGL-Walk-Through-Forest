/* londo_hw5.c
 * Laura Londo
 * 22 March 2014
 * HW #5: Walk Through Forest
 *
 * This program takes you on a magical journey through a forest where you can
 * walk and jump around! Uses cones, cylinders, spheres and other shapes to create
 * the objects in the environment. gluLookAt is used as the camera.
 *
 * Directions:
 *		[w] move forward
 *		[s] move backwards
 *		[a] strafe left
 *		[d] strafe right
 *
 *		[mouse click & drag] look around
 *		OR 		[q] look left
 *				[e] look right      (but really, its much nicer with the mouse)
 *
 *		[spacebar] 	jump
 *  	   [h]		show / hide help menu
 *		  [esc]		quit
 *
 */

#include <math.h>
#include <GL/glut.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PI 3.14159265		//pi
#define numBigTrees 80		//number of large trees
#define numStumps 30 		//number of tree stumps
#define	numWeeds 200		//number of weeds
#define waitTime 16			//wait time between redraws
#define groundSize 80.0 	//size of the ground grid
#define movementSpeed 0.1

int screenWidth = 1000;
int screenHeight = 720;

int helpMenu = 1;									//help menu show/hide  [h]

float bigTrees[numBigTrees][5];						//big tree attributes
float stumps[numStumps][5];							//stump attributes
float weeds[numWeeds][5];							//weed attributes

int w_state, a_state, s_state,
	d_state, q_state, e_state = 0;					//key presses
int mousePressed, mouseStartX, mouseStartY = 0;		//mouse states
float xpos = -52, ypos=0, zpos = 59;				//camera position
float xrot=-20, yrot=42;							//camera angle
float xrotChange, yrotChange = 0;					//camera view attributes

int jumpRising=0;
float jumpSpeed=0;						//jump height increasing

//Function to write a string to the screen at a specified location
void bitmapText(float x, float y, float z, char* words) {
	int len = 0, i = 0;
	//Set the location where the string should appear
	glRasterPos3f(x,y,z);
	len = (int) strlen(words);
	//Set the character in the string to helvetica size 18
	for(int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,words[i]);
	}
}

//initializes all plant attributes
void initForest(void) {
	float x, z, dist;

	//big trees
	for(int i=0; i<numBigTrees; i++) {				//for each big tree,
		do {
			x = rand()%((int)groundSize*2)-groundSize;	//choose x coordinate
			z = rand()%((int)groundSize*2)-groundSize;	//choose z coordinate
			dist = sqrt(x*x + z*z);						//get distance from center
		} while( dist < 30.0);							//choose again if too close to center
		bigTrees[i][0] = x;						//x coord
		bigTrees[i][1] = z;						//z coord
		bigTrees[i][2] = (rand()%50)/10.0+0.2; 	//radius
		bigTrees[i][3] = bigTrees[i][2]*20;		//height
		bigTrees[i][4] =(dist*dist)/1200.0;		//distance from center
	}

	//stumps
	for(int i=0; i<numStumps; i++) {				//for each stump,
		do {
			x = rand()%((int)groundSize*2)-groundSize;	//choose x coordinate
			z = rand()%((int)groundSize*2)-groundSize;	//choose z coordinate
			dist = sqrt(x*x + z*z);						//get distance from center
		} while( dist < 10.0);							//choose again if too close to center
		stumps[i][0] = x;						//x coord
		stumps[i][1] = z;						//z coord
		stumps[i][2] = (rand()%80)/20.0+1.0;	//radius
		stumps[i][3] = (rand()%80)/20.0+0.5;	//height
		stumps[i][4] = (dist*dist)/1200.0;		//distance from center
	}

	//weeds
	for(int i=0; i<numWeeds; i++) {					//for each weed,
		x = rand()%((int)groundSize*2)-groundSize;		//choose x coordinate
		z = rand()%((int)groundSize*2)-groundSize;		//choose z coordinate
		dist = sqrt(x*x + z*z);							//get distance from center
		weeds[i][0] = x;						//x coord
		weeds[i][1] = z;						//z coord
		weeds[i][2] = (rand()%70)/100.0+0.2;	//radius of sphere
		weeds[i][3] = (rand()%80)/30.0+0.5;	 	//height
		weeds[i][4] = (dist*dist)/1200.0;		//distance from center
	}
}

//draws a big tree
void bigTree(float radius, float height, float dist) {
	int segments = 10;		//number of segments for the cylinder & cone
	float cx=0, cy=0, cz=0;	//center point
	float phi, x1, z1;

	//draw cylinder
	for (int j = 0; j<segments; j++) {						//for every segment,
		float shade = (cos(((float)j/segments)*2*PI))/5.0;		//color shade
		glColor3f(												//color darkens with distance
			(0.10+0.52*shade)/dist,
			(0.04+0.24*shade)/dist,
			(0.01+0.11*shade)/dist);

		glBegin(GL_POLYGON);
		//calculate the first top & bottom vertecies
		phi = 2 * PI * j / segments;
		x1 = radius * cos(phi) + cx;
		z1 = radius * sin(phi) + cz;
		glVertex3f(x1, 0, z1);
		glVertex3f(x1, height, z1);

		//calculate the second top & bottom vertecies
		phi = 2 * PI * (j+1) / segments;
		x1 = radius * cos(phi) + cx;
		z1 = radius * sin(phi) + cz;
		glVertex3f(x1, height, z1);
		glVertex3f(x1, 0, z1);

		glEnd();
	} //end cylinder

	//draw cone
	radius *= 6;										//increase the radius
	float center1[3] = {cx, height+(height/6.0), cz};	//center of top of the cone
	float center2[3] = {cx, height, cz};				//center of base of the cone

	//generate the first point along the radius
	phi = 0;
	x1 = radius * cos(phi) + cx;
	z1 = radius * sin(phi) + cz;
	float first[3] = {x1, height, z1};					//the first point on the circumferance

	float color1[3] = {0.2/dist,1.0/dist,0.9/dist};		//center color. darkens with distance
	float color2[3] = {0.52/dist,0.24/dist,0.11/dist};	//edge color. darkens with distance

	for (int i = 0; i<segments; i++) {		//for every segment,
		phi = 2 * PI * (i+1) / segments;
		x1 = radius * cos(phi) + cx;
		z1 = radius * sin(phi) + cz;
		float next[] = {x1, height, z1};


		glBegin(GL_POLYGON);				//draw top cone
			glColor3fv(color1);
			glVertex3fv(center1);
			glColor3fv(color2);
			glVertex3fv(first);
			glVertex3fv(next);
		glEnd();

		glBegin(GL_POLYGON);				//draw bottom circle base
			glColor3fv(color1);
			glVertex3fv(center2);
			glColor3fv(color2);
			glVertex3fv(next);
			glVertex3fv(first);
		glEnd();

		//next point becomes the first for the next interation
		first[0] = next[0];
		first[1] = next[1];
		first[2] = next[2];
	}
}

//draws a 3D pthagoras tree
void pythagorasTree(float topLeftx,  float topLefty,	//top left corner
			 float topRightx, float topRighty,  		//top right corner
			 float n)									//color modifier
{
	//1st point						top left corner
	float Q1x = topLeftx;
	float Q1y = topLefty;

	//2nd point						top right corner
	float Q2x = topRightx;
	float Q2y = topRighty;

	//3rd point						bottom right corner
	float Q3x = Q2x - (Q1y - Q2y);
	float Q3y = Q2y + (Q1x - Q2x);

	//4th point						bottom left corner
	float Q4x = (Q1x-Q2x) + Q3x;
	float Q4y = (Q1y-Q2y) + Q3y;

	glColor3f(n*n, n*n/2, n*n/2.5);

	//draw back face
	glBegin(GL_POLYGON);
	glVertex3f(Q1x, Q1y,-0.5);
	glVertex3f(Q2x, Q2y,-0.5);
	glVertex3f(Q3x, Q3y,-0.5);
	glVertex3f(Q4x, Q4y,-0.5);
	glEnd();

	//draw front face
	glBegin(GL_POLYGON);
	glVertex3f(Q4x, Q4y,0.5);
	glVertex3f(Q3x, Q3y,0.5);
	glVertex3f(Q2x, Q2y,0.5);
	glVertex3f(Q1x, Q1y,0.5);
	glEnd();

	glColor3f(n*n-0.1, n*n/2-0.1, n*n/2.5-0.1);

	//draw left face
	glBegin(GL_POLYGON);
	glVertex3f(Q3x, Q3y, 0.5);
	glVertex3f(Q3x, Q3y, -0.5);
	glVertex3f(Q2x, Q2y, -0.5);
	glVertex3f(Q2x, Q2y, 0.5);
	glEnd();

	//draw right face
	glBegin(GL_POLYGON);
	glVertex3f(Q4x, Q4y, 0.5);
	glVertex3f(Q1x, Q1y, 0.5);
	glVertex3f(Q1x, Q1y, -0.5);
	glVertex3f(Q4x, Q4y, -0.5);
	glEnd();

	glColor3f(n*n-0.2, n*n/2-0.2, n*n/2.5-0.2);

	//draw top face
	glBegin(GL_POLYGON);
	glVertex3f(Q3x, Q3y, -0.5);
	glVertex3f(Q3x, Q3y, 0.5);
	glVertex3f(Q4x, Q4y, 0.5);
	glVertex3f(Q4x, Q4y, -0.5);
	glEnd();

	//draw bottom face
	glBegin(GL_POLYGON);
	glVertex3f(Q2x, Q2y, 0.5);
	glVertex3f(Q2x, Q2y, -0.5);
	glVertex3f(Q1x, Q1y, -0.5);
	glVertex3f(Q1x, Q1y, 0.5);
	glEnd();

	//calculate begining of next square
	float ACx = (Q3x - Q1x)/2;	//half of the diagonal
	float ACy = (Q3y - Q1y)/2;
	float newx = ACx + Q4x;		// point of iscoceles right triangle
	float newy = ACy + Q4y;		// off of the bottom of the square


	//length of a side of the square
	float dist = sqrt(((Q1x-Q2x)*(Q1x-Q2x)) + ((Q1y-Q2y)*(Q1y-Q2y)));

	if(n < 1){ 										  //stopping point
		pythagorasTree(Q4x, Q4y, newx, newy, n+0.08); //left recursion
		pythagorasTree(newx, newy, Q3x, Q3y, n+0.08); //right recursion
	}
} //end pthagorasTree

//draws a stump
void stump(float radius, float height, float dist) {
	int segments = 20;			//number of segments for the cylinder & cone
	float cx=0, cy=0, cz=0;		//center point
	float phi, x1, z1;

	//cylinder
	for (int j = 0; j<segments; j++) {	//for every segment,
		float shade = (cos(((float)j/segments)*2*PI))/5.0;
		glColor3f(
			(0.10+0.52*shade)/dist,
			(0.04+0.24*shade)/dist,
			(0.01+0.11*shade)/dist);
		glBegin(GL_POLYGON);
		//calculate the first top & bottom vertex
		phi = 2 * PI * j / segments;
		x1 = radius * cos(phi) + cx;
		z1 = radius * sin(phi) + cz;
		glVertex3f(x1, 0, z1);
		glVertex3f(x1, height, z1);

		//calculate the second top & bottom vertex
		phi = 2 * PI * (j+1) / segments;
		x1 = radius * cos(phi) + cx;
		z1 = radius * sin(phi) + cz;
		glVertex3f(x1, height, z1);
		glVertex3f(x1, 0, z1);
		glEnd();
	}

	//top of stump
	//generate the first point along the radius
	phi = 0;
	x1 = radius * cos(phi) + cx;
	z1 = radius * sin(phi) + cz;
	float first[3] = {x1, height, z1};

	float color1[3] = {0.2/dist,1.0/dist,0.9/dist};			//center color
	float color2[3] = {0.52/dist, 0.24/dist, 0.11/dist};	//edge color

	for (int i = 0; i<segments; i++) {				//for every segment,
		phi = 2 * PI * (i+1) / segments;
		x1 = radius * cos(phi) + cx;
		z1 = radius * sin(phi) + cz;
		float next[] = {x1, height, z1};			//get the next vertex

		//draw top of the stump
		glBegin(GL_POLYGON);
			glColor3fv(color1);
			glVertex3f(cx, height, cz);
			glColor3fv(color2);
			glVertex3fv(first);
			glVertex3fv(next);
		glEnd();

		//next point becomes the first for the next interation
		first[0] = next[0];
		first[1] = next[1];
		first[2] = next[2];
	}
}

//draws a weed
void weed(float radius, float height, float dist) {
	int segments = 4;		//number of segments for the cylinder & cone
	float cx=0, cz=0;		//center point
	float size = 0.03;		//size of the stem
	float phi, x1, z1;


	//cylinder stem
	glColor3f(0.1/dist, 0.03/dist, 0.1/dist);
	for (int j = 0; j<segments; j++) {	//for every segment,
		glBegin(GL_POLYGON);
		//calculate the first top & bottom vertex
		phi = 2 * PI * j / segments;
		x1 = size * cos(phi) + cx;
		z1 = size * sin(phi) + cz;
		glVertex3f(x1, 0, z1);
		glVertex3f(x1, height, z1);

		//calculate the second top & bottom vertex
		phi = 2 * PI * (j+1) / segments;
		x1 = size * cos(phi) + cx;
		z1 = size * sin(phi) + cz;
		glVertex3f(x1, height, z1);
		glVertex3f(x1, 0, z1);

		glEnd();
	}

	//draw ball on top of stem
	glColor3f(0.62/dist, 1.4/dist, 0.82/dist);
	glPushMatrix();
	glTranslatef(0,height+radius,0);
	glutSolidSphere(radius, 6, 6);
	glPopMatrix();
}

//Model the ground. consiste of a flat gorund color and a grid
void ground(void) {
	glColor3f(0,0,0);		//grid color
	glLineWidth(1);			//line width

	//draw grid
	for (int i = 0; i < groundSize*2+1; i++) {
		glBegin(GL_LINES);
			glVertex3f(-groundSize + i, 0, groundSize);
			glVertex3f(-groundSize + i, 0, -groundSize);
		glEnd();
	}
	for (int j = 0; j < groundSize*2+1; j++) {
		glBegin(GL_LINES);
			glVertex3f(-groundSize, 0, groundSize - j);
			glVertex3f(groundSize, 0, groundSize - j);
		glEnd();
	}

	//draw ground color under the grid
	glBegin(GL_POLYGON);
	glColor3f(0.1,0.2,0.05);
	glVertex3f(-groundSize, -0.1, -groundSize);
	glVertex3f(-groundSize, -0.1, groundSize);
	glVertex3f(groundSize, -0.1, groundSize);
	glVertex3f(groundSize, -0.1, -groundSize);
	glEnd();
}

//switches to 2D when true in order to draw on the front of the screen for the menu
void menuMode(int flag) {
	if(flag == 1) {
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();

		gluOrtho2D(0, screenWidth, 0, screenHeight);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glDisable(GL_DEPTH_TEST);
	}
	else {
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glEnable(GL_DEPTH_TEST);
	}
}

//display callback
void display(void) {
 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 	glLoadIdentity();

 	glRotatef(xrot+xrotChange, 1,0,0);	//viewer x rotation
 	glRotatef(yrot+yrotChange, 0,1,0);	//viewer y rotation
 	glTranslatef(-xpos,-ypos,-zpos);	//viewer position
	gluLookAt(0,3,0,  0,3,5,  0,1,0);	//camera

	ground();							//draw ground

	//draw big trees with the saved attributes
	for(int i=0; i<numBigTrees; i++) {
		glPushMatrix();
		glTranslatef(bigTrees[i][0], 0, bigTrees[i][1]);
		bigTree(bigTrees[i][2], bigTrees[i][3], bigTrees[i][4]);
		glPopMatrix();
	}

	//draw stumps with the saved attributes
	for(int i=0; i<numStumps; i++) {
		glPushMatrix();
		glTranslatef(stumps[i][0], 0, stumps[i][1]);
		stump(stumps[i][2], stumps[i][3], stumps[i][4]);
		glPopMatrix();
	}

	//draw weeds with the saved attributes
	for(int i=0; i<numWeeds; i++) {
		glPushMatrix();
		glTranslatef(weeds[i][0], 0, weeds[i][1]);
		weed(weeds[i][2], weeds[i][3], weeds[i][4]);
		glPopMatrix();
	}

	//draw pthagoras tree
	pythagorasTree(2,0,-2,0,0.5);


	if(helpMenu) {
		menuMode(1); 						//go into menu mode (switch to 2D)

		float menuPosX = 700, menuPosY=570;	//menu start position
		glColor3f(0.8,0.8,0.8);

		bitmapText(menuPosX+10,menuPosY+50, 0,	        "mouse click & drag to rotate");

		bitmapText(menuPosX+90,menuPosY, 0,	                      "forward");
		bitmapText(menuPosX,	menuPosY-25, 0,     "rotate  [q]      [w]      [e]  rotate");

		bitmapText(menuPosX+23, 	menuPosY-75, 0,  "left  [a]       [s]      [d]  right");
		bitmapText(menuPosX+105,menuPosY-100, 0,                     "back");

		bitmapText(menuPosX+80,menuPosY-150, 0,	                  "[spacebar]");
		bitmapText(menuPosX+105,menuPosY-175, 0,	                 "jump");

		bitmapText(menuPosX+111,menuPosY-225, 0,	                 "[h]");
		bitmapText(menuPosX+30,menuPosY-250, 0,	             "show / hide this menu");

		bitmapText(menuPosX+102,menuPosY-300, 0,	                "[esc]");
		bitmapText(menuPosX+105,menuPosY-325, 0,	                "quit");

		menuMode(0);						//switch back to 3D mode
	}

	glutSwapBuffers();
} //end display

//sets key press states to true when the key gets pressed
void keyboard(unsigned char key, int x, int y) {
   	if(key == 'a') a_state = 1;		//strafe left
   	if(key == 'd') d_state = 1;		//strafe right
   	if(key == 'w') w_state = 1;		//move forward
   	if(key == 's') s_state = 1;		//move backward
   	if(key == 'e') e_state = 1;		//rotate view right
   	if(key == 'q') q_state = 1;		//rotate view left
   	if(key == 32) {					//spacebar. jump
   		if(ypos == 0.0)	{			//if viewer is on the ground,
   			jumpRising = 1;			//set rising state to true
   			jumpSpeed = 1;			//set initial up speed to 1
   		}
   	}
   	if(key == 'h') {				//hide / show the help menu
   		if(helpMenu == 1) helpMenu = 0;
   		else helpMenu = 1;
   	}
   	if((int)key == 27) exit(0);		//exit program
}

//sets the key press states to false when the key is released
void keyboardUp(unsigned char key, int x, int y) {
	if(key == 'a') a_state = 0;		//stop strafe left
	if(key == 'd') d_state = 0;		//stop strafe right
	if(key == 'w') w_state = 0;		//stop move forward
	if(key == 's') s_state = 0;		//stop move backwards
	if(key == 'e') e_state = 0;		//stop rotate right
	if(key == 'q') q_state = 0;		//stop rotate left
}

// Handles the begining and end of a left mouse click for view rotation.
// The temporaty view rotation is applied when mouse click ends
void mouse(int butt, int state, int x,  int y) {
	if (state == GLUT_DOWN  &&  butt == GLUT_LEFT_BUTTON) {	//left click
		if(mousePressed == 0) {		//if this is the innitial click down,
			mouseStartX = x;		//save starting mouse x coordinate
			mouseStartY = y;		//save starting mouse y coordinate
		}
		mousePressed = 1;			//set mouse pressed state to true
	}
	else {							//else the left click is no longer pressed
		mousePressed = 0;			//set pressed state to false
		xrot += xrotChange;			//apply the x rotation change to make it permanent
		yrot += yrotChange;			//apply the y rotation change to make it permanent
		xrotChange = yrotChange = 0;//reset temporary rotation change to 0
	}
}

// Changes the temporary view rotation while the left mouse button is pressed.
// The temporary rotation angle is proportional to the distance of the mouse
// pointer from the starting click point.
void motion(int x, int y) {
	if(mousePressed) {								//if the left button is pressed,
		xrotChange = (float)(y - mouseStartY)/3.0;	//set the temp x-axis rot to the mouse y distance

		//limit the x-axis rotation to prevent the camera from being able to flip upside-down
		if(xrot+xrotChange > 90.0) {	//if camera tries to flip over from above
			xrotChange = 90.0 - xrot;
		}
		if(xrot+xrotChange < -90.0) {	//if camera tries to flip over from below
			xrotChange = -90 - xrot;
		}
		yrotChange = (float)(x - mouseStartX)/3.0;	//set the temp y-axis rot to the mouse x distance
	}
}

//applies key press movements and rotation changes and redraws the world at set intervals
void timer(int value) {
	//rotation angles = permanent rotation + temporary rotation
	float newxrot = xrot + xrotChange;
	float newyrot = yrot + yrotChange;

	//viewer position change using the w a s d keys. Moves relative to the viewing angle.
	if (a_state) {								//a key is pressed (strafe left)
		float yrotrad;
		yrotrad = (newyrot / 180 * PI);
		xpos -= (float)(cos(yrotrad)) * movementSpeed;
		zpos -= (float)(sin(yrotrad)) * movementSpeed;
	}
	if (d_state) {								//d key is pressed (strafe right)
		float yrotrad;
		yrotrad = (newyrot / 180 * PI);
		xpos += (float)cos(yrotrad) * movementSpeed;
		zpos += (float)sin(yrotrad) * movementSpeed;
	}
	if (w_state) {								//w key is pressed (move forward)
		float xrotrad, yrotrad;
        yrotrad = (newyrot / 180 * PI);
        xrotrad = (newxrot / 180 * PI);
        xpos += (float)(sin(yrotrad)) * movementSpeed;
        zpos -= (float)(cos(yrotrad)) * movementSpeed;
	}
	if (s_state) {								//s key is pressed (move backwards)
		float xrotrad, yrotrad;
        yrotrad = (newyrot / 180 * PI);
        xrotrad = (newxrot / 180 * PI);
        xpos -= (float)(sin(yrotrad)) * movementSpeed;
        zpos += (float)(cos(yrotrad)) * movementSpeed;
	}

	//view rotation using the e and q keys
	if (q_state && !mousePressed) {				//q key is pressed (rotate left)
		yrot -= 1;
        if (yrot < -360)yrot += 360;
	}
	if (e_state && !mousePressed) {				//e key is pressed (rotate right)
		yrot += 1;
        if (yrot >360) yrot -= 360;
	}

	if (jumpRising){				//if jumping up,
		ypos += jumpSpeed;				//move higher
		jumpSpeed *= 0.9;				//decrease jump speed
		if(jumpSpeed < 0.1) {			//when jump speed slows,
			jumpRising = 0;					//no longer rising
			jumpSpeed *= -1;				//reverse speed
		}
	}
	else {							//not jumping up,
		if (ypos > 0.0){				//until we reach the ground,
			ypos += jumpSpeed;				//move lower
			jumpSpeed /= 0.9;				//increase falling speed
			if(ypos < 0.0) {				//if we reach the ground
				ypos = 0;						//land at 0
			}
		}
	}

	glutPostRedisplay();						//redraw scene
	glutTimerFunc(waitTime, timer, 1);			//set next timer
}

//reshape callback. adjusts both the clipping box and viewport. keeps proportions unchanged
void reshape(int w, int h) {
	float left = -0.1, right = 0.1, bottom = -0.1, top = 0.1, znear = 0.1, zfar = 150;
	float ratio = (float)h / (float)w;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w <= h)
		glFrustum(left, right, bottom * ratio,
			top * ratio, znear, zfar);
	else
		glFrustum(left / ratio, right / ratio,
			bottom, top, znear, zfar);

	glMatrixMode(GL_MODELVIEW);
}

//main method
int main(int argc, char **argv) {
 	glutInit(&argc, argv);
 	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
 	glutInitWindowSize(screenWidth, screenHeight);
 	glutCreateWindow("Forest Adventure!");
 	glEnable(GL_DEPTH_TEST);
 	glClearColor(0,0,0,0);

 	glutIgnoreKeyRepeat(1);		// disables glut from simulating key press and release
 								// repetitions when holding down a key
 	srand(3);
 	initForest();				//initialize random configuration of plants in the forest

 	//function callbacks
 	glutDisplayFunc(display);			//displays the world
 	glutKeyboardFunc(keyboard);			//key presses
 	glutKeyboardUpFunc(keyboardUp);		//key release
 	glutMouseFunc(mouse);				//mouse press and release
 	glutMotionFunc(motion);				//mouse movement
 	glutTimerFunc(waitTime, timer, 1);	//redraws world at intervals
 	glutReshapeFunc(reshape);			//reshape window

 	glutMainLoop();
	return 0;
}
