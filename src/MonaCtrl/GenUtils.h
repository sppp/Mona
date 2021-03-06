#ifndef NAMEDOBJECT_H
#define NAMEDOBJECT_H

#ifdef _WIN32
	#ifndef WIN32
		#define WIN32
	#endif
#endif

#include "XMLUtils.h"
#include <iostream>
#include <fstream>
#include <vector>
#include "GLee.h"

#define deleteObject(A){ if(A){ delete   A; A = NULL; } }
#define deleteArray(A) { if(A){ delete[] A; A = NULL; } }

#define MAX_TEX_UNITS 8

class Logger {
public:
	static void writeImmidiateInfoLog(const String& info);
	static void writeFatalErrorLog(const String& logString);
	static bool writeErrorLog(const String& error);
	static void writeInfoLog(const String& info);
	static void initialize(const char* logfilename = NULL);
	static void flush();

private:
	static Vector<String> log_strings;
	static String         logPath;
};

class NamedObject {

protected:
	String name;

public:
	NamedObject(const char*  argName = NULL);
	NamedObject(const String& argName);
	NamedObject(const NamedObject& copy);
	~NamedObject();

	NamedObject& operator=(const NamedObject& copy);
	void   SetName(const char*   nameArg);
	void   SetName(const String& name);

	const String& GetName()     const;
	const char*        getCharName() const;

};

class Perlin {
private:
	static void   normalize2(double v[2]);
	static void   normalize3(double v[3]);

public:
	static void   setNoiseFrequency(int frequency);
	static int    getNoiseFrequency();

	static double noise1(double arg);
	static double noise2(double vec[2]);
	static double noise3(double vec[3]);
	static void   initialize();
	static double noise1D(double x, double alpha, double beta, int n);
	static double noise2D(double x, double y, double alpha, double beta, int n);
	static double noise3D(double x, double y, double z, double alpha, double beta, int n);
};

class MediaPathManager {
public:
	static const String lookUpMediaPath(const String&  path);
	static const String getPathAt(int index);

	static bool  registerPath(const TiXmlElement* MediaPathNode);
	static bool  registerPath(const String&  path);

	static int   getPathCount();
	static void  printAllPaths();

private:
	static Vector<String> data_paths;
};

template <class ODT>
class DistanceObject {
public:
	DistanceObject() {
		distance = 0.0;
	}

	DistanceObject(const ODT& objectArg) {
		distance = 0.0;
		object   = objectArg;
	}

	DistanceObject(const DistanceObject& copy) {
		operator=(copy);
	}

	DistanceObject& operator =(const DistanceObject& copy) {
		if (this != &copy) {
			distance = copy.distance;
			object   = copy.object;
		}

		return *this;
	}

	bool operator ==(const DistanceObject& copy) {
		return (distance == copy.distance);
	}

	bool operator >=(const DistanceObject& copy) {
		return (distance >= copy.distance);
	}

	bool operator <=(const DistanceObject& copy) {
		return (distance <= copy.distance);
	}

	bool operator >(const DistanceObject& copy) {
		return (distance > copy.distance);
	}

	bool operator < (const DistanceObject& copy) {
		return (distance < copy.distance);
	}

	void setDistance(double dist) {
		distance = dist;
	}

	double getDistance() const {
		return distance;
	}

	void setObject(const ODT& objectArg) {
		object = objectArg;
	}

	ODT& getObject() {
		return object;
	}
private:
	double distance;
	ODT     object;

};

#endif
