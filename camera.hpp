#include "math.hpp"

#ifndef CAMERA_HPP
#define CAMERA_HPP

struct Camera {
	vector3f_t position;
	vector3f_t view;
	vector3f_t up;
	vector2f_f resolution;
	vector2f_f fov;
	float apertureRadius;
	float focalDistance;
};

class InteractiveCamera {

private:

	vector3f_t center;
	vector3f_t view;
	float pitch;
	float radius;
	float aperature;
	float divi;
	float focalDistance;

public:
	vector2f_f resolution;
	vector2f_f fov;

public:
	InteractiveCamera() {
		center = vector3f_t(0, 0, 0);
		divi = 0;
		pitch = 0.3;
		radius = 4;
		aperature = 0.01; // 0.04
		focalDistance = 4.0f;

		resolution = vector2f_f(512, 512);
		fov = vector2f_f(40, 40);
	}

private:
	void fixdivi() {
		divi = mod(divi, 2 * M_PI);
	}

	void fixPitch() {
		float padding = 0.05;
		pitch = clamp(pitch, -M_PI_2 + padding, M_PI_2 - padding);
	}

	void fixRadius() {
		float minRadius = 0.2;
		float maxRadius = 100.0;
		radius = clamp(radius, minRadius, maxRadius);
	}

	void fixaperature() {
		float minaperature = 0.0;
		float maxaperature = 25.0;
		aperature = clamp(aperature, minaperature, maxaperature);
	}

	void fixFocalDistance() {
		float minFocalDist = 0.2;
		float maxFocalDist = 100.0;
		focalDistance = clamp(focalDistance, minFocalDist, maxFocalDist);
	}

private:
	float clamp(float n, float low, float high) {
		n = fminf(n, high);
		n = fmaxf(n, low);
		return n;
	}

	float radiansToDegrees(float radians) {
		return radians * 180.0 / M_PI;
	}

	float degreesToRadians(float degrees) {
		return degrees / 180.0 * M_PI;
	}

	float mod(float x, float y) {
		return x - y * floorf(x / y);
	}

public:
	void setResolution(float x, float y) {
		resolution = vector2f_f(x, y);
		setFov(fov.x);
	}

	void changeRadius(float x) {
		radius += radius * x;
		fixRadius();
	}

	void changedivi(float x) {
		divi += x;
		fixdivi();
	}

	void setFov(float newfov) {
		fov.x = newfov;
		fov.y = radiansToDegrees(
			atan(
				tan(
					degreesToRadians(newfov) * 0.5) * (resolution.y / resolution.x)
			) * 2.0
		);
	}

	void changePitch(float x) {
		pitch += x;
		fixPitch();
	}

	void changeAltitude(float x) {
		center.y += x;
	}

	void changeFocalDistance(float x) {
		focalDistance += x;
		fixFocalDistance();
	}

	void mov(float m) {
		center += view * m;
	}

	void rotateRight(float m) {
		float divi2 = divi;
		divi2 += m;
		float pitch2 = pitch;
		float xDirection = sin(divi2) * cos(pitch2);
		float yDirection = sin(pitch2);
		float zDirection = cos(divi2) * cos(pitch2);
		vector3f_t directionToCamera = vector3f_t(xDirection, yDirection, zDirection);
		view = directionToCamera * (-1.0);
	}

	void buildRenderCamera(Camera* renderCamera) {
		float xDirection = sin(divi) * cos(pitch);
		float yDirection = sin(pitch);
		float zDirection = cos(divi) * cos(pitch);
		vector3f_t directionToCamera = vector3f_t(xDirection, yDirection, zDirection);
		view = directionToCamera * (-1.0);

		renderCamera->position = center + directionToCamera * radius;
		renderCamera->resolution = vector2f_f(resolution.x, resolution.y);
		renderCamera->view = view;
		renderCamera->apertureRadius = aperature;
		renderCamera->fov = vector2f_f(fov.x, fov.y);
		renderCamera->focalDistance = focalDistance;
		renderCamera->up = vector3f_t(0, 1, 0);
	}

	void strafe(float m) {
		vector3f_t strafeAxis = cross(view, vector3f_t(0, 1, 0));
		strafeAxis.normalize();
		center += strafeAxis * m;
	}
};

#endif // CAMERA_HPP