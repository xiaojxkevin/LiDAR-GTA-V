#define _USE_MATH_DEFINES
#include "script.h"
#include "keyboard.h"
#include <string>
#include <ctime>
#include <fstream>
#include <math.h>

#pragma warning(disable : 4244 4305) // double <-> float conversions
constexpr unsigned int NUMBER_FRAME = 1000;

void notificationOnLeft(std::string notificationText) {
	UI::_SET_NOTIFICATION_TEXT_ENTRY("CELL_EMAIL_BCON");
	const int maxLen = 99;
	for (int i = 0;i < notificationText.length(); i += maxLen) {
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
		} else if (entityType == 2) {
			entityTypeName = "GTA.Vehicle";
		} else if (entityType == 3) {
			entityTypeName = "GTA.Prop";
		}
	}
	result.entityTypeName = entityTypeName;
	return result;
}

ray angleOffsetRaycast(Vector3 source, Vector3 cameraRotation, double angleOffsetX, double angleOffsetZ, int range){
	double rotationX = (cameraRotation.x + angleOffsetX) * (M_PI / 180.0);
	double rotationZ = (cameraRotation.z + angleOffsetZ) * (M_PI / 180.0);
	double multiplyXY = abs(cos(rotationX));
	Vector3 direction;
	direction.x = sin(rotationZ) * multiplyXY * -1;
	direction.y = cos(rotationZ) * multiplyXY;
	direction.z = sin(rotationX);
	return raycast(source, direction, range, -1);
}

void lidar(double horiFovMin, double horiFovMax, double vertFovMin, double vertFovMax, double horiStep, double vertStep,int range,std::string filePath) {
	std::ofstream data_file;
	data_file.open(filePath);
	// To stop the game
	GAMEPLAY::SET_GAME_PAUSED(true);
	TIME::PAUSE_CLOCK(true);
	
	// double vertexCount = (horiFovMax - horiFovMin) * (1 / horiStep) * (vertFovMax - vertFovMin) * (1 / vertStep);
	Vector3 rot = CAM::GET_GAMEPLAY_CAM_ROT(2);
	Vector3 coord = CAM::GET_GAMEPLAY_CAM_COORD();
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
				} else if (entityName3 == "GTA.Ped") {
					r = 0; g = 255; b = 0;
				} else if (entityName3 == "GTA.Prop") {
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

void ScriptMain() {
	srand(GetTickCount()); // I do not know why author use something here
	unsigned int count(0); // initialize the number of point cloud data we will have 
	// wait for the command(pressing F6) to start

	Vehicle car;
	while (true) {
		if (IsKeyJustUp(VK_F6))
		{
			notificationOnLeft("It may take a while for auto to find the road");
			Ped playerId = PLAYER::PLAYER_PED_ID();
			Vector3 playerPos = ENTITY::GET_ENTITY_COORDS(playerId, true);
			car = VEHICLE::GET_CLOSEST_VEHICLE(playerPos.x, playerPos.y, playerPos.z, 10, 0, 70);
			VEHICLE::START_VEHICLE_ALARM(car);
			break;
		}
		WAIT(0);
	}
	WAIT(3000);
	notificationOnLeft("Start Recording");
	WAIT(3000);
	// now we start to 
	while (true)
	{
		if (VEHICLE::IS_VEHICLE_STOPPED(car)) continue;
		std::string file_path = "data_set/point_data_" + std::to_string(count) + ".txt";
		lidar(0.0, 360.0, -30.0, 10.0, 0.25, 0.25, 75, file_path);
		++count;
		WAIT(2000);
	}
}