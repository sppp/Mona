#if 0

//***************************************************************************//
//* File Name: spacial.cpp                                                  *//
//* Author:    Tom Portegys, portegys@ilstu.edu                             *//
//* Date Made: 07/25/02                                                     *//
//* File Desc: Class implementation details representing spacial            *//
//*            properties: rotation, translation, scale, and speed.         *//
//* Rev. Date:                                                              *//
//* Rev. Desc:                                                              *//
//*                                                                         *//
//***************************************************************************//

#include "Spacial.h"

// Initialize.
void cSpacial::initialize(GLdouble pitch, GLdouble yaw, GLdouble roll,
						  GLdouble pitchRate, GLdouble yawRate, GLdouble rollRate,
						  GLdouble x, GLdouble y, GLdouble z, GLdouble scale, GLdouble speed) {
	m_pitchRate = pitchRate;
	m_yawRate   = yawRate;
	m_rollRate  = rollRate;
	m_x         = x;
	m_y         = y;
	m_z         = z;
	m_scale     = scale;
	m_speed     = speed;
	m_qcalc->Clear();
	setPitch(pitch);
	setYaw(yaw);
	setRoll(roll);
	m_qcalc->BuildRotationMatrix(m_rotmatrix);
}


// Get and set Euler angles.
GLdouble cSpacial::getPitch() {
	GLdouble p, y, r;
	GetEulerAngles(p, y, r);
	return (p);
}


GLdouble cSpacial::getYaw() {
	GLdouble p, y, r;
	GetEulerAngles(p, y, r);
	return (y);
}


GLdouble cSpacial::getRoll() {
	GLdouble p, y, r;
	GetEulerAngles(p, y, r);
	return (r);
}


void cSpacial::setPitch(GLdouble pitch) {
	GLdouble p, y, r, pr, s;
	GetEulerAngles(p, y, r);
	pr          = m_pitchRate;
	s           = m_speed;
	m_speed     = 0.0;
	m_pitchRate = (pitch - p);
	Update();
	m_pitchRate = pr;
	m_speed     = s;
}


void cSpacial::setYaw(GLdouble yaw) {
	GLdouble p, y, r, yr, s;
	GetEulerAngles(p, y, r);
	yr        = m_yawRate;
	s         = m_speed;
	m_speed   = 0.0;
	m_yawRate = (yaw - y);
	Update();
	m_yawRate = yr;
	m_speed   = s;
}


void cSpacial::setRoll(GLdouble roll) {
	GLdouble p, y, r, rr, s;
	GetEulerAngles(p, y, r);
	rr         = m_rollRate;
	s          = m_speed;
	m_speed    = 0.0;
	m_rollRate = (roll - r);
	Update();
	m_rollRate = rr;
	m_speed    = s;
}


// Get direction vectors.
void cSpacial::getRight(GLdouble* v) {
	v[0] = m_rotmatrix[0][0];
	v[1] = m_rotmatrix[0][1];
	v[2] = m_rotmatrix[0][2];
}


void cSpacial::getUp(GLdouble* v) {
	v[0] = m_rotmatrix[1][0];
	v[1] = m_rotmatrix[1][1];
	v[2] = m_rotmatrix[1][2];
}


void cSpacial::getForward(GLdouble* v) {
	v[0] = m_rotmatrix[2][0];
	v[1] = m_rotmatrix[2][1];
	v[2] = m_rotmatrix[2][2];
}


// Update rotation and translation state.
void cSpacial::Update() {
	CQuaternion xq, yq, zq, q1, q2;
	GLdouble     v[3];
	v[0] = 1.0;
	v[1] = 0.0;
	v[2] = 0.0;
	xq.LoadRotation(DegreesToRadians(m_pitchRate), v);
	v[0] = 0.0;
	v[1] = 1.0;
	v[2] = 0.0;
	yq.LoadRotation(DegreesToRadians(m_yawRate), v);
	m_qcalc->MulQuats(xq, yq, q1);
	v[0] = 0.0;
	v[1] = 0.0;
	v[2] = 1.0;
	zq.LoadRotation(DegreesToRadians(m_rollRate), v);
	m_qcalc->MulQuats(q1, zq, q2);
	q1.m_quat[0] = m_qcalc->m_quat[0];
	q1.m_quat[1] = m_qcalc->m_quat[1];
	q1.m_quat[2] = m_qcalc->m_quat[2];
	q1.m_quat[3] = m_qcalc->m_quat[3];
	m_qcalc->MulQuats(q1, q2, *m_qcalc);
	m_qcalc->BuildRotationMatrix(m_rotmatrix);
	v[0] = m_rotmatrix[2][0];
	v[1] = m_rotmatrix[2][1];
	v[2] = m_rotmatrix[2][2];
	normalize(v);
	m_x += (v[0] * m_speed);
	m_y += (v[1] * m_speed);
	m_z += (v[2] * m_speed);
}


// Get billboard (face toward) rotation to target point given in local coordinates.
// Return axis and angle for rotation to accomplish billboard.
// rotation[0-2]=axis, rotation[3]=angle
void cSpacial::getBillboard(GLdouble* target, GLdouble* rotation) {
	GLdouble forward[3];

	// Check for coincidence.
	for (int i = 0; i < 4; i++)
		rotation[i] = 0.0;

	if ((target[0] == 0.0) && (target[1] == 0.0) && (target[2] == 0.0))
		return;

	// Find the rotation from the forward vector to the target.
	getForward(forward);
	getBillboard(target, forward, rotation);
}


// Get billboard rotation from source vector to target vector.
// Return axis and angle for rotation to accomplish billboard.
// rotation[0-2]=axis, rotation[3]=angle
void cSpacial::getBillboard(GLdouble* target, GLdouble* source, GLdouble* rotation) {
	Vector3f  v1, v2, v3;
	GLdouble d;

	// Check for invalid condition.
	for (int i = 0; i < 4; i++)
		rotation[i] = 0.0;

	if ((fabs(target[0]) < tol) && (fabs(target[1]) < tol) && (fabs(target[2]) < tol))
		return;

	if ((fabs(source[0]) < tol) && (fabs(source[1]) < tol) && (fabs(source[2]) < tol))
		return;

	// The axis to rotate about is the cross product
	// of the forward vector and the vector to the target.
	v1.x = target[0];
	v1.y = target[1];
	v1.z = target[2];
	v1.Normalize();
	v2.x = source[0];
	v2.y = source[1];
	v2.z = source[2];
	v2.Normalize();
	v3 = v1 ^ v2;
	v3.Normalize();
	rotation[0] = v3.x;
	rotation[1] = v3.y;
	rotation[2] = v3.z;
	// The angle to rotate is the dot product of the vectors.
	d = v1 * v2;

	if (d > 0.9999)
		rotation[3] = 0.0;
	else
		rotation[3] = RadiansToDegrees(acos(d));
}


// Get model transformation matrix.
void cSpacial::getModelTransform(GLdouble* matrix) {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(m_x, m_y, m_z);
	m_qcalc->BuildRotationMatrix(m_rotmatrix);
	glMultMatrixf(&m_rotmatrix[0][0]);
	glScalef(m_scale, m_scale, m_scale);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	glPopMatrix();
}


// Get world coordinates from local.
void cSpacial::localToWorld(GLdouble* local, GLdouble* world) {
	int     i, j;
	GLdouble m[16];
	Matrix  x(4, 4), p(4, 1), t(4, 1);
	getModelTransform(m);

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++)
			x(i, j) = m[(j * 4) + i];
	}

	p(0, 0)  = local[0];
	p(1, 0)  = local[1];
	p(2, 0)  = local[2];
	p(3, 0)  = 1.0;
	t        = x * p;
	world[0] = t(0, 0);
	world[1] = t(1, 0);
	world[2] = t(2, 0);
}


// Inverse transform local point.
void cSpacial::inverseTransformPoint(GLdouble* point) {
	int     i, j;
	GLdouble m[16];
	Matrix  x(4, 4), y(4, 4), p(4, 1), t(4, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	getModelTransform(m);

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++)
			x(i, j) = m[(j * 4) + i];
	}

	y        = !x;
	p(0, 0)  = point[0];
	p(1, 0)  = point[1];
	p(2, 0)  = point[2];
	p(3, 0)  = 1.0;
	t        = y * p;
	point[0] = t(0, 0);
	point[1] = t(1, 0);
	point[2] = t(2, 0);
	glPopMatrix();
}


// Point-to-point distance.
GLdouble cSpacial::pointDistance(GLdouble* p1, GLdouble* p2) {
	GLdouble dx = p1[0] - p2[0];
	dx *= dx;
	GLdouble dy = p1[1] - p2[1];
	dy *= dy;
	GLdouble dz = p1[2] - p2[2];
	dz *= dz;
	return (sqrt(dx + dy + dz));
}


// Clone spacial.
cSpacial* cSpacial::Clone() {
	int i, j;
	cSpacial* spacial = new cSpacial();
	ASSERT(spacial != NULL);
	spacial->m_pitchRate = m_pitchRate;
	spacial->m_yawRate   = m_yawRate;
	spacial->m_rollRate  = m_rollRate;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++)
			spacial->m_rotmatrix[i][j] = m_rotmatrix[i][j];
	}

	delete spacial->m_qcalc;
	spacial->m_qcalc = m_qcalc->Clone();
	ASSERT(spacial->m_qcalc != NULL);
	spacial->m_x     = m_x;
	spacial->m_y     = m_y;
	spacial->m_z     = m_z;
	spacial->m_scale = m_scale;
	spacial->m_speed = m_speed;
	return (spacial);
}


// Load.
void cSpacial::Serialize(Stream& fp) {
	FREAD_FLOAT(&m_pitchRate, fp);
	FREAD_FLOAT(&m_yawRate, fp);
	FREAD_FLOAT(&m_rollRate, fp);
	FREAD_FLOAT(&m_x, fp);
	FREAD_FLOAT(&m_y, fp);
	FREAD_FLOAT(&m_z, fp);
	FREAD_FLOAT(&m_scale, fp);
	FREAD_FLOAT(&m_speed, fp);
	m_qcalc->Load(fp);
}


// Save object.
void cSpacial::Store(Stream& fp) {
	FWRITE_FLOAT(&m_pitchRate, fp);
	FWRITE_FLOAT(&m_yawRate, fp);
	FWRITE_FLOAT(&m_rollRate, fp);
	FWRITE_FLOAT(&m_x, fp);
	FWRITE_FLOAT(&m_y, fp);
	FWRITE_FLOAT(&m_z, fp);
	FWRITE_FLOAT(&m_scale, fp);
	FWRITE_FLOAT(&m_speed, fp);
	m_qcalc->Store(fp);
}
#endif
