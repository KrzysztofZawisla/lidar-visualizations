#include "app.h"


App::App(std::vector<std::string>& args) {
	running_ = true;
	if (!parse_args(args)) {
		running_ = false;
	}
}

App::~App() {
}

int App::run() {
	Cloud cloud;

	while (running_) {
		// Grab cloud
		if (running_ && cloud_grabber_ptr_) {
			if (!cloud_grabber_ptr_->read(cloud)) {
				running_ = false;
			}
		}

		// Do some calculations, writing to file, depending on the selected scenario
		if (running_ && scenario_ptr_) {
			if (!scenario_ptr_->update(cloud)) {
				running_ = false;
			}
		}

		// Update the GUI with new data
		if (running_ && gui_ptr_) {
			if (!gui_ptr_->update(cloud)) {
				running_ = false;
			}
		}
	}

	return 0;
}

void App::print_help() {
	std::cout << "-----------------------------------------------------------" << std::endl;
	std::cout << "LIDAR Visualizations" << std::endl;
	std::cout << "-----------------------------------------------------------" << std::endl;
	std::cout << "Authors: Bartek Dudek, Szymon Bednorz" << std::endl;
	std::cout << "Source: https://github.com/knei-knurow/lidar-visualizations" << std::endl;
	std::cout << std::endl;
	std::cout << "Usage:" << std::endl;
	std::cout << "\tlidar [options]" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "\t-f <arg>  file with lines containing angle [deg] and distance [mm] separated by whitespaces" << std::endl;
	std::cout << "\t-h        Show this message" << std::endl;
	std::cout << "\t-o <arg>  Output directory" << std::endl;
	std::cout << "\t-p <arg>  RPLidar port" << std::endl;
	std::cout << "\t-r        Disable mouse ray" << std::endl;
	std::cout << "\t-s <arg>  Select display scale (1mm -> 1px for scale = 1.0; set 0 to autoscale)" << std::endl;
	std::cout << "\t-S <arg>  Select scenario" << std::endl;
	std::cout << std::endl;
	std::cout << "Scenarios:" << std::endl;
	std::cout << "\t0\tsave point clouds from each frame as batched TXT file" << std::endl;
	std::cout << std::endl;
	std::cout << "GUI Mode Keyboard Shortcuts:" << std::endl;
	std::cout << "\tT           save point cloud as TXT" << std::endl;
	std::cout << "\tS           save screenshot" << std::endl;
	std::cout << "\tUp/Down     scale displayed cloud (faster with shift, slower with ctrl)" << std::endl;
	std::cout << "\tLeft/Right  rotate cloud (faster with shift, slower with ctrl; only with files)" << std::endl;
	std::cout << "\tP           rotation on/off (only with files)" << std::endl;
	std::cout << "\tC           switch color maps" << std::endl;
	std::cout << "\tM           switch point cloud display modes" << std::endl;
	std::cout << "\tR           mouse ray display on/off" << std::endl;
}


bool App::parse_args(std::vector<std::string>& args) {
	std::vector<std::string>::iterator it;

	// Print help
	it = std::find_if(args.begin(), args.end(), [](const std::string& s) { 
		return s == "-h" || s == "--help";}
	);
	if (it != args.end()) {
		print_help();
		return false;
	}

	// Input RPLIDAR port name
	std::string rplidar_port;
	it = std::find_if(args.begin(), args.end(), [](const std::string& s) {
		return s == "-p" || s == "--port"; }
	);
	if (it != args.end() && ++it != args.end()) {
		rplidar_port = *it;
	}

	// Input cloud filename
	std::string cloud_filename;
	it = std::find_if(args.begin(), args.end(), [](const std::string& s) {
		return s == "-f" || s == "--file"; }
	);
	if (it != args.end() && ++it != args.end()) {
		cloud_filename = *it;
	}

	// Input cloud series filename
	std::string cloud_series_filename;
	it = std::find_if(args.begin(), args.end(), [](const std::string& s) {
		return s == "-fs" || s == "--file-series"; }
	);
	if (it != args.end() && ++it != args.end()) {
		cloud_series_filename = *it;
	}

	// Output directory
	std::string output_dir;
	it = std::find_if(args.begin(), args.end(), [](const std::string& s) {
		return s == "-o" || s == "--outpur-dir"; }
	);
	if (it != args.end() && ++it != args.end()) {
		output_dir = *it;
	}

	// RPLIDAR mode
	RPLIDARScanModes rplidar_mode = RPLIDARScanModes::SENSITIVITY;
	it = std::find_if(args.begin(), args.end(), [](const std::string& s) {
		return s == "-m" || s == "--rplidar-mode"; }
	);
	if (it != args.end() && ++it != args.end()) {
		if (*it == std::to_string(int(RPLIDARScanModes::STANDARD))) {
			rplidar_mode = RPLIDARScanModes::STANDARD;
		}
		else if (*it == std::to_string(int(RPLIDARScanModes::EXPRESS))) {
			rplidar_mode = RPLIDARScanModes::EXPRESS;
		}
		else if (*it == std::to_string(int(RPLIDARScanModes::BOOST))) {
			rplidar_mode = RPLIDARScanModes::BOOST;
		}
		else if (*it == std::to_string(int(RPLIDARScanModes::SENSITIVITY))) {
			rplidar_mode = RPLIDARScanModes::SENSITIVITY;
		}
		else if (*it == std::to_string(int(RPLIDARScanModes::STABILITY))) {
			rplidar_mode = RPLIDARScanModes::STABILITY;
		}
		else {
			std::cerr << "ERROR: Invalid RPLIDAR mode id." << std::endl;
		}
	}

	// GUI Type
	GUIType gui_type = GUIType::SFML;
	it = std::find_if(args.begin(), args.end(), [](const std::string& s) {
		return s == "-g" || s == "--gui"; }
	);
	if (it != args.end() && ++it != args.end()) {
		if (*it == std::to_string(int(GUIType::TERMINAL))) {
			gui_type = GUIType::TERMINAL;
		}
		else if (*it == std::to_string(int(GUIType::SFML))) {
			gui_type = GUIType::SFML;
		}
		else {
			std::cerr << "ERROR: Invalid GUI id." << std::endl;
		}
	}

	// Scenario
	ScenarioType scenario_type = ScenarioType::IDLE;
	it = std::find_if(args.begin(), args.end(), [](const std::string& s) {
		return s == "-s" || s == "--scenario"; }
	);
	if (it != args.end() && ++it != args.end()) {
		if (*it == std::to_string(int(ScenarioType::IDLE))) {
			scenario_type = ScenarioType::IDLE;
		}
		else {
			std::cerr << "ERROR: Invalid scenario id." << std::endl;
		}
	}

	if (!rplidar_port.empty()) {
		cloud_grabber_ptr_ = std::make_unique<CloudRPLIDARPortGrabber>(rplidar_port, 256000, rplidar_mode);
		if (!cloud_grabber_ptr_->get_status()) cloud_grabber_ptr_.reset(nullptr);
	}
	else if (!cloud_series_filename.empty()) {
		cloud_grabber_ptr_ = std::make_unique<CloudFileSeriesGrabber>(cloud_series_filename);
		if (!cloud_grabber_ptr_->get_status()) cloud_grabber_ptr_.reset(nullptr);
	}
	else if (!cloud_filename.empty()) {
		cloud_grabber_ptr_ = std::make_unique<CloudFileGrabber>(cloud_filename, 0.2);
		if (!cloud_grabber_ptr_->get_status()) cloud_grabber_ptr_.reset(nullptr);
	}

	if (!cloud_grabber_ptr_ && !gui_ptr_ && !scenario_ptr_) {
		return false;
	}

	if (gui_type == GUIType::TERMINAL) {
		gui_ptr_ = std::make_unique<TerminalGUI>();
	}
	else if (gui_type == GUIType::SFML) {
		SFMLGUISettings sfml_settings;
		gui_ptr_ = std::make_unique<SFMLGUI>(sfml_settings);
	}

	return true;
}

/*
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <vector>
#include <cmath>
#include <utility>
#include <SFML\System.hpp>
#include <SFML\Window.hpp>
#include <SFML\Graphics.hpp>

#include "characters.h"
#include <rplidar.h>


//
// Command line arguments parsing
//
bool check_arg_exist(int argc, char** argv, const std::string & arg) {
	auto it = std::find(argv, argv + argc, arg);
	if (it != argv + argc) {
		(*it)[0] = '\0';
		return true;
	}
	return false;
}

std::string get_arg(int argc, char** argv, const std::string & arg) {
	auto it = std::find(argv, argv + argc, arg);
	if (it != argv + argc) {
		(*it)[0] = '\0';
		if (it + 1 != argv + argc) {
			std::string s(*(it + 1));
			(*(it + 1))[0] = '\0';
			return s;
		}
	}
	return "";
}

void check_invalid_args(int argc, char** argv) {
	std::vector<int> v;
	for (int i = 1; i < argc; i++) {
		if (std::strlen(*(argv + i)) > 0) {
			v.push_back(i);
		}
	}
	for (auto i : v) {
		std::cerr << "ERROR: Invalid argument: " << argv[i] << std::endl;
	}
}

void print_help() {
	std::cout << "-----------------------------------------------------------" << std::endl;
	std::cout << "LIDAR Visualizations" << std::endl;
	std::cout << "-----------------------------------------------------------" << std::endl;
	std::cout << "Authors: Bartek Dudek, Szymon Bednorz" << std::endl;
	std::cout << "Source: https://github.com/knei-knurow/lidar-visualizations" << std::endl;
	std::cout << std::endl;
	std::cout << "Usage:" << std::endl;
	std::cout << "\tlidar [options]" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "\t-f <arg>  file with lines containing angle [deg] and distance [mm] separated by whitespaces" << std::endl;
	std::cout << "\t-h        Show this message" << std::endl;
	std::cout << "\t-o <arg>  Output directory" << std::endl;
	std::cout << "\t-p <arg>  RPLidar port" << std::endl;
	std::cout << "\t-r        Disable mouse ray" << std::endl;
	std::cout << "\t-s <arg>  Select display scale (1mm -> 1px for scale = 1.0; set 0 to autoscale)" << std::endl;
	std::cout << "\t-S <arg>  Select scenario" << std::endl;
	std::cout << std::endl;
	std::cout << "Scenarios:" << std::endl;
	std::cout << "\t0\tsave point clouds from each frame as batched TXT file" << std::endl;
	std::cout << std::endl;
	std::cout << "GUI Mode Keyboard Shortcuts:" << std::endl;
	std::cout << "\tT           save point cloud as TXT" << std::endl;
	std::cout << "\tS           save screenshot" << std::endl;
	std::cout << "\tUp/Down     scale displayed cloud (faster with shift, slower with ctrl)" << std::endl;
	std::cout << "\tLeft/Right  rotate cloud (faster with shift, slower with ctrl; only with files)" << std::endl;
	std::cout << "\tP           rotation on/off (only with files)" << std::endl;
	std::cout << "\tC           switch color maps" << std::endl;
	std::cout << "\tM           switch point cloud display modes" << std::endl;
	std::cout << "\tR           mouse ray display on/off" << std::endl;
}

//
// GUI
//
size_t mouse_ray_to_point(const Cloud& cloud, int x, int y) {
	x -= ORIGIN_X;
	y -= ORIGIN_Y;
	if (y == 0) {
		return -1;
	}
	auto angle = 90.0f - atan2(y, x) * 180.0f / acos(-1.0f);
	if (angle < 0) {
		angle += 360.0f;
	}
	for (int i = 1; i < cloud.size; i++) {
		if (cloud.pts[i - 1].first <= angle && cloud.pts[i].first >= angle) {
			return i;
		}
	}
	if (cloud.pts.back().first <= angle && 360.0f >= angle) {
		return cloud.size - 1;
	}
	return 0;
}

//
// IO
//
bool load_cloud(const std::string& filename, Cloud& cloud) {
	std::ifstream file(filename);
	while (file) {
		std::string line;
		std::getline(file, line);
		if (line.empty() || line[0] == '#')
			continue;
		float angle, dist;
		std::stringstream sline(line);
		sline >> angle >> dist;

		if (dist > cloud.max) {
			cloud.max = dist;
			cloud.max_idx = cloud.size;
		}
		if (dist < cloud.min && dist > 0) {
			cloud.min = dist;
			cloud.min_idx = cloud.size;
		}
		cloud.size++;
		cloud.avg += dist;
		cloud.pts.push_back(std::make_pair(angle, dist));
	}
	cloud.avg /= cloud.size;
	for (auto& pt : cloud.pts) {
		cloud.std += (cloud.avg - pt.second) * (cloud.avg - pt.second);
	}
	cloud.std = std::sqrt(cloud.std);

	if (cloud.size == 0) {
		std::cerr << "Error: File does not contain a valid cloud." << std::endl;
		return false;
	}
	return true;
}

void load_cloud_from_buffer(rplidar_response_measurement_node_hq_t* buffer, size_t count, Cloud& cloud, bool skip_bad) {
	cloud = Cloud();
	for (int i = 0; i < count; i++) {
		float angle = buffer[i].angle_z_q14 / 65536.0f * 360;
		float dist = buffer[i].dist_mm_q2 / 4.0f;

		if (skip_bad && dist == 0) continue;
		if (dist > cloud.max) cloud.max = dist;
		if (dist < cloud.min && dist > 0) cloud.min = dist;
		cloud.size++;
		cloud.avg += dist;
		cloud.pts.push_back(std::make_pair(angle, dist));
	}
	cloud.avg /= cloud.size;
	for (auto& pt : cloud.pts) {
		cloud.std += (cloud.avg - pt.second) * (cloud.avg - pt.second);
	}
	cloud.std = std::sqrt(cloud.std);
}

std::string create_filename(const std::string& dir, const std::string& dot_ext, size_t cnt) {
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	std::ostringstream oss;
	oss << std::put_time(&tm, "%d.%m.%Y-%H.%M.%S");
	return dir + "/" + std::to_string(cnt) + "-" + oss.str() + dot_ext;
}

bool save_screenshot(uint8_t* mat, const std::string& dir) {
	static size_t cnt = 0;
	sf::Texture texture;
	texture.create(WIDTH, HEIGHT);
	sf::Sprite sprite(texture);
	texture.update(mat);
	return texture.copyToImage().saveToFile(create_filename(dir, ".png", cnt++));
}

bool save_txt(const Cloud& cloud, const std::string& dir) {
	static size_t cnt = 0;
	std::ofstream file(create_filename(dir, ".txt", cnt++));
	if (!file) {
		return false;
	}
	file << "# RPLIDAR SCAN DATA" << std::endl;
	file << "# Software: https://github.com/knei-knurow/lidar-visualizations" << std::endl;
	file << "# Authors: Szymon Bednorz, Bartek Dudek" << std::endl;
	file << "# Angle Distance" << std::endl;
	for (const auto& pt : cloud.pts) {
		file << pt.first << " " << pt.second << std::endl;
	}
	return true;
}

//
// Drawing
//
void draw_pixel(uint8_t* mat, int x, int y, color c) {
	if (x < 0 || x > WIDTH - 1 || y < 0 || y > HEIGHT - 1)
		return;
	mat[(WIDTH * y + x) * CHANNELS + 0] = c.r;
	mat[(WIDTH * y + x) * CHANNELS + 1] = c.g;
	mat[(WIDTH * y + x) * CHANNELS + 2] = c.b;
	mat[(WIDTH * y + x) * CHANNELS + 3] = c.a;
}

void draw_point(uint8_t* mat, int x, int y, color c, float lightness) {
	c.r *= lightness;
	c.g *= lightness;
	c.b *= lightness;

	for (auto cx : { -1, 0, 1 }) {
		for (auto cy : { -1, 0, 1 }) {
			draw_pixel(mat, x + cx, y + cy, c);
		}
	}
}

void draw_line(uint8_t* mat, float x0, float y0, float x1, float y1, color c) {
	float x = x1 - x0, y = y1 - y0;
	const float max = std::max(std::fabs(x), std::fabs(y));
	x /= max; y /= max;
	for (float n = 0; n < max; n++) {
		draw_point(mat, int(x0), int(y0), c);
		x0 += x; y0 += y;
	}
}

void draw_line_color_dist(uint8_t* mat, float x0, float y0, float x1, float y1, float max_dist, float lightness) {
	float x = x1 - x0, y = y1 - y0;
	const float max = std::max(std::fabs(x), std::fabs(y));
	x /= max; y /= max;
	for (float n = 0; n < max; n++) {
		float dist = std::hypot(x0 - ORIGIN_X, y0 - ORIGIN_Y);
		draw_point(mat, int(x0), int(y0), calc_color_dist(dist, max_dist, lightness));
		x0 += x; y0 += y;
	}
}

void draw_ray(uint8_t* mat, float x0, float y0, float x1, float y1, color c) {
	float x = x1 - x0, y = y1 - y0;
	const float max = std::max(std::fabs(x), std::fabs(y));
	x /= max; y /= max;
	while (x0 < WIDTH && x0 >= 0 && y0 < HEIGHT && y0 >= 0) {
		draw_point(mat, x0, y0, c);
		x0 += x; y0 += y;
	}
}

void draw_background(uint8_t* mat, color c) {
	for (int i = 0; i < WIDTH * HEIGHT * CHANNELS; i += CHANNELS) {
		mat[i + 0] = c.r;
		mat[i + 1] = c.g;
		mat[i + 2] = c.b;
		mat[i + 3] = c.a;
	}
}

void draw_grid(uint8_t* mat, color c) {
	for (int x = 0; x < WIDTH; x += WIDTH / 8) {
		for (int y = 0; y < HEIGHT; y++) {
			draw_pixel(mat, x, y, c);
		}
	}
	for (int y = 0; y < HEIGHT; y += HEIGHT / 8) {
		for (int x = 0; x < WIDTH; x++) {
			draw_pixel(mat, x, y, c);
		}
	}
}

void draw_cloud_bars(uint8_t* mat, const Cloud& cloud, unsigned coloring) {
	if (cloud.size == 0) return;
	unsigned max_width = 80;
	for (int j = 0; j < HEIGHT; j++) {
		float dist = cloud.pts[size_t(j * cloud.size / HEIGHT)].second;

		int width = int(std::round(dist / cloud.max * max_width));

		color c;
		if (coloring == 0) c = calc_color_angle(float(j * cloud.size / HEIGHT) / float(cloud.size));
		else if (coloring == 1) c = calc_color_dist(cloud.pts[size_t(j * cloud.size / HEIGHT)].second, cloud.max, 1.0);
		for (int i = 0; i < width; i++) {
			draw_pixel(mat, i, j, c);
		}
	}
}

void draw_connected_cloud(uint8_t* mat, const Cloud& cloud, float scale, int y_offset, float lightness, bool marks, unsigned coloring) {
	if (scale == 0)
		scale = calc_scale(cloud);

	if (cloud.size == 0)
		return;

	int cnt = 1;
	auto first_pt = cyl_to_cart(cloud.pts[0], scale);
	auto last_pt = first_pt;
	for (int i = 1; i < cloud.size; i++) {
		auto pt = cyl_to_cart(cloud.pts[i], scale);
		color c;
		if (coloring == 0) c = calc_color_angle(float(cnt) / float(cloud.size), lightness);
		else if (coloring == 1) c = calc_color_dist(cloud.pts[i].second, cloud.max, lightness);

		if (cloud.pts[i].second > 0 && cloud.pts[i - 1].second > 0) {
			if (coloring == 0) draw_line(mat, float(last_pt.first), float(last_pt.second + y_offset), float(pt.first), float(pt.second) + y_offset, c);
			else if (coloring == 1) draw_line_color_dist(mat, float(last_pt.first), float(last_pt.second + y_offset), float(pt.first), float(pt.second) + y_offset, cloud.max * scale, lightness);
		}

		last_pt = pt;
		cnt++;
	}
	color c;
	if (coloring == 0) {
		c = calc_color_angle(float(cnt) / float(cloud.size), lightness);
		draw_line(mat, float(last_pt.first), float(last_pt.second + y_offset), float(first_pt.first), float(first_pt.second + y_offset), c);
	}
	else if (coloring == 1) {
		c = calc_color_dist(cloud.pts.back().second, cloud.max, lightness);
		draw_line_color_dist(mat, float(last_pt.first), float(last_pt.second + y_offset), float(first_pt.first), float(first_pt.second + y_offset), cloud.max * scale, lightness);
	}

	if (marks) {
		auto pt_min = cyl_to_cart(cloud.pts[cloud.min_idx], scale);
		auto pt_max = cyl_to_cart(cloud.pts[cloud.max_idx], scale);
		draw_mark(mat, pt_min.first, pt_min.second, cloud.min);
		draw_mark(mat, pt_max.first, pt_max.second, cloud.max);
	}
}

void draw_cloud(uint8_t* mat, const Cloud& cloud, float scale, int y_offset, float lightness, bool marks, unsigned coloring) {
	if (scale == 0)
		scale = calc_scale(cloud);

	if (cloud.size == 0)
		return;

	int cnt = 1;
	for (int i = 0; i < cloud.size; i++) {
		auto pt = cyl_to_cart(cloud.pts[i], scale);
		color c;
		if (coloring == 0) c = calc_color_angle(float(cnt) / float(cloud.size), lightness);
		else if (coloring == 1) c = calc_color_dist(cloud.pts[i].second, cloud.max, lightness);
		draw_point(mat, pt.first, pt.second, c, lightness);
		cnt++;
	}

	if (marks) {
		auto pt_min = cyl_to_cart(cloud.pts[cloud.min_idx], scale);
		auto pt_max = cyl_to_cart(cloud.pts[cloud.max_idx], scale);
		draw_mark(mat, pt_min.first, pt_min.second, cloud.min);
		draw_mark(mat, pt_max.first, pt_max.second, cloud.max);
	}
}

color calc_color_angle(float v, float lightness) {
	color c0, c1;
	if (v >= 0 && v <= 0.33f) {
		c0 = COLOR_CLOUD0;
		c1 = COLOR_CLOUD1;
	}
	else if (v <= 0.66f) {
		v -= 0.33f;
		c0 = COLOR_CLOUD2;
		c1 = COLOR_CLOUD0;
	}
	else if (v <= 1.0f) {
		v -= 0.66f;
		c0 = COLOR_CLOUD1;
		c1 = COLOR_CLOUD2;
	}
	else {
		return color(255, 255, 255);
	}

	c0.r *= float(v) / 0.34f;
	c0.g *= float(v) / 0.34f;
	c0.b *= float(v) / 0.34f;
	c1.r *= 1.0f - float(v) / 0.34f;
	c1.g *= 1.0f - float(v) / 0.34f;
	c1.b *= 1.0f - float(v) / 0.34f;
	return color(float(c0.r + c1.r) * lightness, float(c0.g + c1.g) * lightness, float(c0.b + c1.b) * lightness);
}

color calc_color_dist(float dist, float max, float lightness) {
	float v = dist / max;
	color c0, c1;
	if (v >= 0 && v <= 0.33f) {
		c0 = color(255, 255, 0);
		c1 = color(255, 0, 0);
	}
	else if (v <= 0.66f) {
		v -= 0.33f;
		c0 = color(0, 255, 0);
		c1 = color(255, 255, 0);
	}
	else if (v <= 1.0f) {
		v -= 0.66f;
		c0 = color(0, 0, 255);
		c1 = color(0, 255, 0);
	}
	else {
		return color(255, 255, 255);
	}

	c0.r *= float(v) / 0.34f;
	c0.g *= float(v) / 0.34f;
	c0.b *= float(v) / 0.34f;
	c1.r *= 1.0f - float(v) / 0.34f;
	c1.g *= 1.0f - float(v) / 0.34f;
	c1.b *= 1.0f - float(v) / 0.34f;
	return color(float(c0.r + c1.r) * lightness, float(c0.g + c1.g) * lightness, float(c0.b + c1.b) * lightness);
}

void draw_mark(uint8_t* mat, int x, int y, float val, color c) {
	int a = val / 1000;
	int b = int(val) % 1000;

	draw_point(mat, x, y, c);
	auto str = std::to_string(a) + "." + std::to_string(b);
	x += -12;
	y += 5;
	for (int ch : str) {
		if (ch == '.') ch = 10;
		else ch -= '0';

		for (int cy = 0; cy < CHAR_HEIGHT; cy++) {
			for (int cx = 0; cx < CHAR_MAT[ch][cy].size(); cx++) {
				if (CHAR_MAT[ch][cy][cx] == '#')
					draw_pixel(mat, x + cx, y + cy, c);
			}
		}
		x += CHAR_MAT[ch][0].size() + 1;
	}
}

//
// Point cloud calculations
//
std::pair<int, int> cyl_to_cart(std::pair<float, float> pt, float scale) {
	float phi = pt.first;
	float dist = pt.second;
	int x = int(std::round(dist * std::sin(phi * (acos(-1.0f) / 180.0f)) * scale)) + ORIGIN_X;
	int y = int(std::round(dist * std::cos(phi * (acos(-1.0f) / 180.0f)) * scale)) + ORIGIN_Y;
	return std::make_pair(x, y);
}

float calc_scale(const Cloud& cloud) {
	return float(HEIGHT) * 0.7f / cloud.max;
}

void rotate_cloud(Cloud& cloud, float angle) {
	for (auto& i : cloud.pts) {
		i.first += angle;
		if (i.first >= 360) i.first -= 360;
	}
}
*/