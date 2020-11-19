#include <iostream>
#include <iomanip>
#include <string>
#include "communication.h"

#ifdef WINDOWED_APP

bool rplidar_print_info(rplidar::RPlidarDriver* lidar) {
	rplidar_response_device_info_t lidar_info;
	auto res = lidar->getDeviceInfo(lidar_info);
	if (IS_FAIL(res)) {
		std::cout << "ERROR: Unable to get device info response." << std::endl;
		return false;
	}
	std::cout << "LIDAR info:" << std::endl;
	std::cout << "  Model:         " << lidar_info.model << std::endl;
	std::cout << "  Firmware ver.: " << lidar_info.firmware_version << std::endl;
	std::cout << "  Hardware ver.: " << unsigned(lidar_info.hardware_version) << std::endl;
	std::cout << "  Serial number: ";
	for (int i = 0; i < 16; i++) {
		std::cout << std::setfill('0') << std::setw(2) <<std::hex << int(lidar_info.serialnum[i]);
	}
	std::cout << std::endl;
	return true;
}

bool rplidar_print_health(rplidar::RPlidarDriver* lidar, _u8* status) {
	rplidar_response_device_health_t lidar_health;
	auto res = lidar->getHealth(lidar_health);
	if (IS_FAIL(res)) {
		std::cout << "ERROR: Unable to get device health response." << std::endl;
		return false;
	}
	std::cout << "LIDAR health: ";
	if (lidar_health.status == RPLIDAR_STATUS_OK) std::cout << "Good";
	else if (lidar_health.status == RPLIDAR_STATUS_WARNING) std::cout << "Warning";
	else if (lidar_health.status == RPLIDAR_STATUS_ERROR) std::cout << "Error";
	std::cout << std::endl;
	if (status) {
		*status = lidar_health.status;
	}
	return true;
}

bool rplidar_print_scan_modes(rplidar::RPlidarDriver* lidar, _u16* scan_mode) {
	std::vector<rplidar::RplidarScanMode> scan_modes;
	_u16 current_scan_mode;
	auto res = lidar->getTypicalScanMode(current_scan_mode);
	res = lidar->getAllSupportedScanModes(scan_modes);
	if (IS_FAIL(res)) {
		std::cout << "ERROR: Unable to get device scan modes response." << std::endl;
		return false;
	}
	std::cout << "Supported scan modes:" << std::endl;
	for (const auto& scan_mode : scan_modes) {
		std::cout << "  ";
		std::cout << scan_mode.scan_mode
			<< " (" << scan_mode.id << (scan_mode.id == current_scan_mode ? ", DEFAULT" : "") <<  "), "
			<< "sample time: " << scan_mode.us_per_sample << "us, "
			<< "max distance: " << scan_mode.max_distance << "m" << std::endl;
	}
	if (scan_mode) {
		*scan_mode = current_scan_mode;
	}
	return true;
}

bool rplidar_launch(rplidar::RPlidarDriver* lidar, const char* port, _u32 baudrate) {
	std::cout << "LIDAR connection:" << std::endl;
	std::cout << "  Port: " << port << "" << std::endl;
	std::cout << "  Baudrate: " << baudrate << "" << std::endl;
	auto res = lidar->connect(port, baudrate);
	if (IS_FAIL(res)) {
		std::cout << "ERROR: Unable to establish connection with LIDAR." << std::endl;
		return false;
	}
	std::cout << "Connection established." << std::endl;

	_u16 scan_mode;
	if (!rplidar_print_info(lidar)) return false;
	if (!rplidar_print_health(lidar)) return false;
	if (!rplidar_print_scan_modes(lidar, &scan_mode)) return false;

	std::cout << "Statring motor." << std::endl;
	res = lidar->startMotor();
	if (IS_FAIL(res)) {
		std::cout << "ERROR: Unable to start motor." << std::endl;
		return false;
	}
	lidar->startScanExpress(false, scan_mode);
}

void rplidar_stop(rplidar::RPlidarDriver* lidar) {
	std::cout << "Stopping motor and deallocating LIDAR driver." << std::endl;
	auto res = lidar->stopMotor();
	if (IS_FAIL(res)) {
		std::cout << "ERROR: Unable to start motor." << std::endl;
	}
	lidar->disconnect();
	rplidar::RPlidarDriver::DisposeDriver(lidar);
}

#endif