#if 0

//***************************************************************************//
//* File Name: baseObject.cpp                                               *//
//* Author:    Tom Portegys, portegys@ilstu.edu                             *//
//* Date Made: 07/25/02                                                     *//
//* File Desc: Class implementation details representing a basic object.    *//
//* Rev. Date:                                                              *//
//* Rev. Desc:                                                              *//
//*                                                                         *//
//***************************************************************************//

#include "BaseObject.h"

// Set spacial state.
void BaseObject::setSpacial(GLdouble pitch, GLdouble yaw, GLdouble roll,
							GLdouble pitchRate, GLdouble yawRate, GLdouble rollRate,
							GLdouble x, GLdouble y, GLdouble z, GLdouble scale, GLdouble speed) {
	m_spacial->initialize(pitch, yaw, roll,
						  pitchRate, yawRate, rollRate,
						  x, y, z, scale, speed);
}


void BaseObject::setSpacial(cSpacial* spacial) {
	m_spacial->initialize(spacial->getPitch(), spacial->getYaw(), spacial->getRoll(),
						  spacial->getPitchRate(), spacial->getYawRate(), spacial->getRollRate(),
						  spacial->getX(), spacial->getY(), spacial->getZ(),
						  spacial->getScale(), spacial->getSpeed());
}


// Draw object axes.
void BaseObject::drawAxes(GLdouble span) {
	int     numhash;
	GLdouble d, f, o;
	// Set drawing options.
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	// Set marking parameters.
	numhash = 10;
	d       = span / numhash;
	f       = span * 0.05;
	o       = span + 0.00025;
	// Transform.
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(m_spacial->getX(), m_spacial->getY(), m_spacial->getZ());
	glMultMatrixf(&m_spacial->m_rotmatrix[0][0]);
	// X axis is red.
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(span, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	#ifdef NEVER

	for (int i = 0; i < numhash; i++) {
		glVertex3f(i * d, f, 0.0);
		glVertex3f(i * d, -f, 0.0);
		glVertex3f(i * d, 0.0, f);
		glVertex3f(i * d, 0.0, -f);
	}

	#endif
	glEnd();
	glRasterPos3f(o, 0.0, 0.0);
	glutBitmapCharacter(GLUT_BITMAP_8_BY_13, 'X');
	// Y axis is green.
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(0.0, span, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	#ifdef NEVER

	for (int i = 0; i < numhash; i++) {
		glVertex3f(f, i * d, 0.0);
		glVertex3f(-f, i * d, 0.0);
		glVertex3f(0.0, i * d, f);
		glVertex3f(0.0, i * d, -f);
	}

	#endif
	glEnd();
	glRasterPos3f(0.0, o, 0.0);
	glutBitmapCharacter(GLUT_BITMAP_8_BY_13, 'Y');
	// Z axis is blue.
	glColor3f(0.0, 0.0, 1.0);
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, span);
	glVertex3f(0.0, 0.0, 0.0);
	#ifdef NEVER

	for (int i = 0; i < numhash; i++) {
		glVertex3f(f, 0.0, i * d);
		glVertex3f(-f, 0.0, i * d);
		glVertex3f(0.0, f, i * d);
		glVertex3f(0.0, -f, i * d);
	}

	#endif
	glEnd();
	glRasterPos3f(0.0, 0.0, o);
	glutBitmapCharacter(GLUT_BITMAP_8_BY_13, 'Z');
	glPopMatrix();
	glColor3f(1.0, 1.0, 1.0);
}


// Set block vertices.
void BaseObject::setBlockVertices(Vector3f* vertices, double xmin, double xmax, double ymin, double ymax, double zmin, double zmax) {
	vertices[0].x = xmax;
	vertices[0].y = ymax;
	vertices[0].z = zmin;
	vertices[1].x = xmax;
	vertices[1].y = ymax;
	vertices[1].z = zmax;
	vertices[2].x = xmax;
	vertices[2].y = ymin;
	vertices[2].z = zmax;
	vertices[3].x = xmax;
	vertices[3].y = ymin;
	vertices[3].z = zmin;
	vertices[4].x = xmin;
	vertices[4].y = ymax;
	vertices[4].z = zmin;
	vertices[5].x = xmin;
	vertices[5].y = ymax;
	vertices[5].z = zmax;
	vertices[6].x = xmin;
	vertices[6].y = ymin;
	vertices[6].z = zmax;
	vertices[7].x = xmin;
	vertices[7].y = ymin;
	vertices[7].z = zmin;
}


// Draw a block.
void BaseObject::drawBlock(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax) {
	GLdouble x, y, z;
	Vector3f  vertices[8];
	// Set the vertices.
	setBlockVertices(vertices, xmin, xmax, ymin, ymax, zmin, zmax);
	glBegin(GL_QUADS);
	x = vertices[0].x;
	y = vertices[0].y;
	z = vertices[0].z;
	glNormal3f(1.0f, 0.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[1].x;
	y = vertices[1].y;
	z = vertices[1].z;
	glNormal3f(1.0f, 0.0f, 0.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[2].x;
	y = vertices[2].y;
	z = vertices[2].z;
	glNormal3f(1.0f, 0.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(x, y, z);
	x = vertices[3].x;
	y = vertices[3].y;
	z = vertices[3].z;
	glNormal3f(1.0f, 0.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(x, y, z);
	glEnd();
	glBegin(GL_QUADS);
	x = vertices[3].x;
	y = vertices[3].y;
	z = vertices[3].z;
	glNormal3f(0.0, -1.0, 0.0);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(x, y, z);
	x = vertices[2].x;
	y = vertices[2].y;
	z = vertices[2].z;
	glNormal3f(0.0f, -1.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[6].x;
	y = vertices[6].y;
	z = vertices[6].z;
	glNormal3f(0.0f, -1.0f, 0.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[7].x;
	y = vertices[7].y;
	z = vertices[7].z;
	glNormal3f(0.0f, -1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(x, y, z);
	glEnd();
	glBegin(GL_QUADS);
	x = vertices[7].x;
	y = vertices[7].y;
	z = vertices[7].z;
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(x, y, z);
	x = vertices[6].x;
	y = vertices[6].y;
	z = vertices[6].z;
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[5].x;
	y = vertices[5].y;
	z = vertices[5].z;
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[4].x;
	y = vertices[4].y;
	z = vertices[4].z;
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(x, y, z);
	glEnd();
	glBegin(GL_QUADS);
	x = vertices[0].x;
	y = vertices[0].y;
	z = vertices[0].z;
	glNormal3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[4].x;
	y = vertices[4].y;
	z = vertices[4].z;
	glNormal3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[5].x;
	z = vertices[5].z;
	y = vertices[5].y;
	glNormal3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(x, y, z);
	x = vertices[1].x;
	y = vertices[1].y;
	z = vertices[1].z;
	glNormal3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(x, y, z);
	glEnd();
	glBegin(GL_QUADS);
	x = vertices[2].x;
	y = vertices[2].y;
	z = vertices[2].z;
	glNormal3f(0.0f, 0.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(x, y, z);
	x = vertices[1].x;
	y = vertices[1].y;
	z = vertices[1].z;
	glNormal3f(0.0f, 0.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[5].x;
	y = vertices[5].y;
	z = vertices[5].z;
	glNormal3f(0.0f, 0.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[6].x;
	y = vertices[6].y;
	z = vertices[6].z;
	glNormal3f(0.0f, 0.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(x, y, z);
	glEnd();
	glBegin(GL_QUADS);
	x = vertices[7].x;
	y = vertices[7].y;
	z = vertices[7].z;
	glNormal3f(0.0f, 0.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(x, y, z);
	x = vertices[4].x;
	y = vertices[4].y;
	z = vertices[4].z;
	glNormal3f(0.0f, 0.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[0].x;
	y = vertices[0].y;
	z = vertices[0].z;
	glNormal3f(0.0f, 0.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[3].x;
	y = vertices[3].y;
	z = vertices[3].z;
	glNormal3f(0.0f, 0.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(x, y, z);
	glEnd();
}


// Draw block edges.
void BaseObject::drawBlockEdges(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax) {
	GLdouble x, y, z;
	Vector3f  vertices[8];
	// Set the vertices.
	setBlockVertices(vertices, xmin, xmax, ymin, ymax, zmin, zmax);
	glBegin(GL_LINE_LOOP);
	x = vertices[0].x;
	y = vertices[0].y;
	z = vertices[0].z;
	glNormal3f(1.0f, 0.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[1].x;
	y = vertices[1].y;
	z = vertices[1].z;
	glNormal3f(1.0f, 0.0f, 0.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[2].x;
	y = vertices[2].y;
	z = vertices[2].z;
	glNormal3f(1.0f, 0.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(x, y, z);
	x = vertices[3].x;
	y = vertices[3].y;
	z = vertices[3].z;
	glNormal3f(1.0f, 0.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(x, y, z);
	glEnd();
	glBegin(GL_LINE_LOOP);
	x = vertices[3].x;
	y = vertices[3].y;
	z = vertices[3].z;
	glNormal3f(0.0, -1.0, 0.0);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(x, y, z);
	x = vertices[2].x;
	y = vertices[2].y;
	z = vertices[2].z;
	glNormal3f(0.0f, -1.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[6].x;
	y = vertices[6].y;
	z = vertices[6].z;
	glNormal3f(0.0f, -1.0f, 0.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[7].x;
	y = vertices[7].y;
	z = vertices[7].z;
	glNormal3f(0.0f, -1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(x, y, z);
	glEnd();
	glBegin(GL_LINE_LOOP);
	x = vertices[7].x;
	y = vertices[7].y;
	z = vertices[7].z;
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(x, y, z);
	x = vertices[6].x;
	y = vertices[6].y;
	z = vertices[6].z;
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[5].x;
	y = vertices[5].y;
	z = vertices[5].z;
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[4].x;
	y = vertices[4].y;
	z = vertices[4].z;
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(x, y, z);
	glEnd();
	glBegin(GL_LINE_LOOP);
	x = vertices[0].x;
	y = vertices[0].y;
	z = vertices[0].z;
	glNormal3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[4].x;
	y = vertices[4].y;
	z = vertices[4].z;
	glNormal3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[5].x;
	z = vertices[5].z;
	y = vertices[5].y;
	glNormal3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(x, y, z);
	x = vertices[1].x;
	y = vertices[1].y;
	z = vertices[1].z;
	glNormal3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(x, y, z);
	glEnd();
	glBegin(GL_LINE_LOOP);
	x = vertices[2].x;
	y = vertices[2].y;
	z = vertices[2].z;
	glNormal3f(0.0f, 0.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(x, y, z);
	x = vertices[1].x;
	y = vertices[1].y;
	z = vertices[1].z;
	glNormal3f(0.0f, 0.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[5].x;
	y = vertices[5].y;
	z = vertices[5].z;
	glNormal3f(0.0f, 0.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[6].x;
	y = vertices[6].y;
	z = vertices[6].z;
	glNormal3f(0.0f, 0.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(x, y, z);
	glEnd();
	glBegin(GL_LINE_LOOP);
	x = vertices[7].x;
	y = vertices[7].y;
	z = vertices[7].z;
	glNormal3f(0.0f, 0.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(x, y, z);
	x = vertices[4].x;
	y = vertices[4].y;
	z = vertices[4].z;
	glNormal3f(0.0f, 0.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[0].x;
	y = vertices[0].y;
	z = vertices[0].z;
	glNormal3f(0.0f, 0.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(x, y, z);
	x = vertices[3].x;
	y = vertices[3].y;
	z = vertices[3].z;
	glNormal3f(0.0f, 0.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(x, y, z);
	glEnd();
}


// Load object.
void BaseObject::Load(String filename) {
	Stream& fp;

	if ((fp = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "Cannot load object from file %s\n", filename);
		exit(1);
	}

	Load(fp);
}


void BaseObject::Serialize(Stream& fp) {
	m_spacial->Load(fp);
}


// Save object.
void BaseObject::Store(String filename) {
	Stream& fp;

	if ((fp = fopen(filename, "w")) == NULL) {
		fprintf(stderr, "Cannot save object to file %s\n", filename);
		exit(1);
	}

	Store(fp);
}


void BaseObject::Store(Stream& fp) {
	m_spacial->Store(fp);
}
#endif
