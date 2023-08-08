#define _USE_MATH_DEFINES
#include "script.h"
#include "keyboard.h"
#include <string>
#include <ctime>
#include <fstream>
#include <math.h>
#include <chrono>


#pragma warning(disable : 4244 4305) // double <-> float conversions

// Set up all constants
constexpr unsigned int NUMBER_FRAME = 1000;
constexpr size_t FILE_FORMAT = 4;
constexpr unsigned int FREEZE_SIZE = 2048;
constexpr int TEST_STEP = 800;
constexpr int TRAIN_STEP = 2100;
constexpr float SPEED_BIAS = 1.5;
constexpr float SCRIPED_CAM_HEIGHT = 1.5;

void notificationOnLeft(std::string notificationText) {
	UI::_SET_NOTIFICATION_TEXT_ENTRY("CELL_EMAIL_BCON");
	const int maxLen = 99;
	for (int i = 0; i < notificationText.length(); i += maxLen) {
		std::string divideText = notificationText.substr(i, min(maxLen, notificationText.length() - i));
		const char* divideTextAsConstCharArray = divideText.c_str();
		char* divideTextAsCharArray = new char[divideText.length() + 1];
		strcpy_s(divideTextAsCharArray, divideText.length() + 1, divideTextAsConstCharArray);
		UI::_ADD_TEXT_COMPONENT_STRING(divideTextAsCharArray);
	}
	int handle = UI::_DRAW_NOTIFICATION(false, 1);
}

struct ray {
	bool hit;
	Vector3 hitCoordinates;
	Vector3 surfaceNormal;
	std::string entityTypeName;
	int rayResult;
	int hitEntityHandle;
};

ray raycast(Vector3 source, Vector3 direction, float maxDistance, int intersectFlags) {
	int hit = 0;
	int hitEntityHandle = -1;

	Vector3 hitCoordinates;
	Vector3 surfaceNormal;

	hitCoordinates.x = 0;
	hitCoordinates.y = 0;
	hitCoordinates.z = 0;
	surfaceNormal.x = 0;
	surfaceNormal.y = 0;
	surfaceNormal.z = 0;

	float targetX = source.x + (direction.x * maxDistance);
	float targetY = source.y + (direction.y * maxDistance);
	float targetZ = source.z + (direction.z * maxDistance);

	int rayHandle = WORLDPROBE::_CAST_RAY_POINT_TO_POINT(source.x, source.y, source.z, targetX, targetY, targetZ, intersectFlags, 0, 7);
	int rayResult = WORLDPROBE::_GET_RAYCAST_RESULT(rayHandle, &hit, &hitCoordinates, &surfaceNormal, &hitEntityHandle);

	ray result;
	result.rayResult = rayResult;
	result.hit = hit;
	result.hitCoordinates = hitCoordinates;
	result.surfaceNormal = surfaceNormal;
	result.hitEntityHandle = hitEntityHandle;
	std::string entityTypeName = "Unknown";
	if (ENTITY::DOES_ENTITY_EXIST(hitEntityHandle)) {
		int entityType = ENTITY::GET_ENTITY_TYPE(hitEntityHandle);
		if (entityType == 1) {
			entityTypeName = "GTA.Ped";
		}
		else if (entityType == 2) {
			entityTypeName = "GTA.Vehicle";
		}
		else if (entityType == 3) {
			entityTypeName = "GTA.Prop";
		}
	}
	result.entityTypeName = entityTypeName;
	return result;
}

ray angleOffsetRaycast(Vector3 source, Vector3 cameraRotation, double angleOffsetX, double angleOffsetZ, int range) {
	double rotationX = (cameraRotation.x + angleOffsetX) * (M_PI / 180.0);
	double rotationZ = (cameraRotation.z + angleOffsetZ) * (M_PI / 180.0);
	double multiplyXY = abs(cos(rotationX));
	Vector3 direction;
	direction.x = sin(rotationZ) * multiplyXY * -1;
	direction.y = cos(rotationZ) * multiplyXY;
	direction.z = sin(rotationX);
	return raycast(source, direction, range, -1);
}

void lidar(double horiFovMin, double horiFovMax, double vertFovMin, double vertFovMax, double horiStep, double vertStep, int range, std::string filePath, Cam camera) {
	std::ofstream data_file;
	data_file.open(filePath);
	TIME::PAUSE_CLOCK(true);
	GAMEPLAY::SET_GAME_PAUSED(true);
	Vector3 rot = CAM::GET_CAM_ROT(camera, 2);
	Vector3 coord = CAM::GET_CAM_COORD(camera);
	std::string cameraCenter = "cameraCenter " + std::to_string(coord.x) + " " + std::to_string(coord.y) + " " + std::to_string(coord.z) + "\n";
	std::string camerRot = "camerRotation " + std::to_string(rot.x) + " " + std::to_string(rot.y) + " " + std::to_string(rot.z) + "\n";
	data_file << cameraCenter;
	data_file << camerRot;
	data_file << "x y z r g b norm_x norm_y norm_z\n";

	for (double z = horiFovMin; z < horiFovMax; z += horiStep) {
		std::string vertexData = "";
		for (double x = vertFovMin; x < vertFovMax; x += vertStep) {
			std::string entityName3 = "None";
			int entityHash = 0;
			unsigned char r = 0; unsigned char g = 0; unsigned char b = 0;
			ray result = angleOffsetRaycast(coord, rot, x, z, range);

			if (result.hit) {
				r = 255; g = 255; b = 255;
			}
			if (result.hitEntityHandle != -1) {
				entityName3 = result.entityTypeName;
				if (entityName3 == "GTA.Vehicle") {
					r = 255; g = 0; b = 0;
				}
				else if (entityName3 == "GTA.Ped") {
					r = 0; g = 255; b = 0;
				}
				else if (entityName3 == "GTA.Prop") {
					r = 0; g = 0; b = 255;
				}
			}
			vertexData += std::to_string(result.hitCoordinates.x) + " " + std::to_string(result.hitCoordinates.y) + " " \
				+ std::to_string(result.hitCoordinates.z) + " " + std::to_string(r) + " " + std::to_string(g) + " " \
				+ std::to_string(b) + " " + std::to_string(result.surfaceNormal.x) + " " + std::to_string(result.surfaceNormal.y) \
				+ " " + std::to_string(result.surfaceNormal.z) + "\n";
		}
		data_file << vertexData;
	}
	GAMEPLAY::SET_GAME_PAUSED(false);
	TIME::PAUSE_CLOCK(false);
	data_file.close();
	std::string notice = "Data Written to " + filePath;
	notificationOnLeft(notice);
}

inline void stop() {
	int vehicles[FREEZE_SIZE];
	int vehicleCount = worldGetAllVehicles(vehicles, FREEZE_SIZE);
	for (int x = 0; x < vehicleCount; x++)
	{
		if (vehicles[x] != PED::GET_VEHICLE_PED_IS_USING(PLAYER::PLAYER_PED_ID()))
		{
			ENTITY::FREEZE_ENTITY_POSITION(vehicles[x], true);
		}
		else
		{
			WAIT(3);
			ENTITY::FREEZE_ENTITY_POSITION(vehicles[x], false);
		}
	}
}

inline void mode(int* time_step, bool* is_test) {
	while (true)
	{
		notificationOnLeft("Press F6 if for TRAINING else F7 for TEST");
		if (IsKeyJustUp(VK_F7))
		{
			*time_step = TEST_STEP;
			WAIT(1000);
			notificationOnLeft("Begin sampling TEST data");
			break;
		}
		else if (IsKeyJustUp(VK_F6))
		{
			*time_step = TRAIN_STEP;
			*is_test = false;
			WAIT(1000);
			notificationOnLeft("Begin sampling TRAINING data");
			break;
		}
		WAIT(0);
	}
}

inline void create_car(Vehicle* car) {
	if (car == NULL) return;
	notificationOnLeft("Press F6 to generate car");
	while (true) {
		if (IsKeyJustUp(VK_F6))
		{
			Ped playerid = PLAYER::PLAYER_PED_ID();
			Vector3 pos = ENTITY::GET_ENTITY_COORDS(playerid, true);
			*car = VEHICLE::CREATE_VEHICLE(3609690755, pos.x, pos.y, pos.z, ENTITY::GET_ENTITY_HEADING(playerid), false, false);
			if (*car == 0)
			{
				notificationOnLeft("Failed to generate the car, please change to a wider area");
				WAIT(1000);
				continue;
			}
			notificationOnLeft("Vehicle with ID " + std::to_string(*car) + " is created");
			WAIT(1000);
			break;
		}
		WAIT(0);
	}
}

inline Cam create_cam(Vehicle car) {
	Vector3 coord = ENTITY::GET_ENTITY_COORDS(car, true);
	Vector3 car_rot = ENTITY::GET_ENTITY_ROTATION(car, 2);
	Cam camera = CAM::CREATE_CAM_WITH_PARAMS("DEFAULT_SCRIPTED_CAMERA", coord.x, coord.y, coord.z, car_rot.x, car_rot.y, car_rot.z, 90.0, true, 2);
	CAM::SET_CAM_ACTIVE(camera, true);
	CAM::RENDER_SCRIPT_CAMS(true, false, 3000, true, false);
	CAM::ATTACH_CAM_TO_ENTITY(camera, car, 0, 0, SCRIPED_CAM_HEIGHT, true);
	CAM::SET_FOLLOW_VEHICLE_CAM_VIEW_MODE(1);
	notificationOnLeft("camera with ID " + std::to_string(camera) + " is created");
	return camera;
}

inline void adjust_cam_rot(Vehicle car, Cam camera) {
	Vector3 car_rot = ENTITY::GET_ENTITY_ROTATION(car, 2);
	CAM::SET_CAM_ROT(camera, car_rot.x, car_rot.y, car_rot.z, 2);
}

inline void create_pcl(unsigned int* count, Cam camera) {
	std::string num = std::to_string(*count);
	size_t precision = FILE_FORMAT - num.size();
	num.insert(0, precision, '0');
	std::string file_path = "data_set/" + num + ".txt";
	lidar(0.0, 360.0, -14.0, 14.0, 0.17578125, 0.4375, 100, file_path, camera);
	++(*count);
}

inline void record_mode(Vehicle car, Cam* camera, unsigned int* count, bool is_test) {
	notificationOnLeft("F6 to start recording; F7 to record only one frame; F8 to gameplay vision; F9 to sampling vision");
	while (true) {
		if (*camera != -1) adjust_cam_rot(car, *camera);
		if (IsKeyJustUp(VK_F6))
		{
			break;
		}
		else if (IsKeyJustUp(VK_F7))
		{
			if (is_test) stop();
			create_pcl(count, *camera);
		}
		else if (IsKeyJustUp(VK_F8))
		{
			CAM::RENDER_SCRIPT_CAMS(false, true, 3000, true, false);
		}
		else if (IsKeyJustUp(VK_F9))
		{
			CAM::RENDER_SCRIPT_CAMS(true, false, 3000, true, false);
		}
		WAIT(10);
	}
}

inline bool check_car_speed(Vehicle car, bool* flag, Cam* camera) {
	float speed = ENTITY::GET_ENTITY_SPEED(car);
	if (speed <= SPEED_BIAS)
	{
		notificationOnLeft("speed: " + std::to_string(speed) + " too slow, do not record");
		SYSTEM::WAIT(1100);
		clock_t t0, t1;
		t0 = clock();
		t1 = clock();
		while (t1 - t0 <= 1000)
		{
			if (IsKeyJustUp(VK_F6))
			{
				*flag = false;
				break;
			}
			t1 = clock();
			WAIT(0);
		}
		adjust_cam_rot(car, *camera);
		WAIT(0);
		return false;
	}
	return true;
}

inline void recording_break(int time_step, bool* flag, Vehicle car, Cam* camera, bool is_test) {
	SYSTEM::WAIT(time_step);
	clock_t t0, t1;
	t0 = clock();
	t1 = clock();
	while (t1 - t0 <= time_step - 100)
	{
		if (IsKeyJustUp(VK_F6))
		{
			*flag = false;
			break;
		}
		adjust_cam_rot(car, *camera);
		t1 = clock();
		WAIT(0);
	}
	adjust_cam_rot(car, *camera);
	WAIT(0);
}

void ScriptMain() {
	// This is the main function of the hole project

	// Initialization
	Vehicle car(0);
	unsigned int count(0); 
	int time_step(0);
	Cam camera(-1);
	bool is_test = true;

	// To decide if to sample TEST data or TRAIN data
	mode(&time_step, &is_test);
	
	// Create the vehicle that we will use
	create_car(&car);

	// Create the sampling camera
	camera = create_cam(car);

	WAIT(1000);
	
	// The main loop
	do
	{
		record_mode(car, &camera, &count, is_test);
		notificationOnLeft("start recording with vehicleID " + std::to_string(car));
		WAIT(500);
		bool flag(true);
		while (flag)
		{
			if (!check_car_speed(car, &flag, &camera)) continue;
			if (is_test) stop();
			adjust_cam_rot(car, camera);
			create_pcl(&count, camera);
			recording_break(time_step, &flag, car, &camera, is_test);
		}
		notificationOnLeft("Stopped recording");
		WAIT(0);
	} while (count < NUMBER_FRAME);

	while (true)
	{
		notificationOnLeft("We have recorded " + std::to_string(NUMBER_FRAME) + " frames, please log out of the game");
		WAIT(3000);
	}
}