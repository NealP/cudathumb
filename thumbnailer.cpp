#include <opencv2/opencv.hpp>
#include <opencv2/gpu/gpu.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <string>
#include <iostream>

using namespace cv;
namespace fs = boost::filesystem;

const double THUMB_WIDTH = 300.0;

bool is_valid_image(fs::path path) {
	std::string extension = path.extension().string();
	boost::algorithm::to_lower(extension);

	if (!fs::is_regular_file(path)) {
		return false;
	} else if (path.string().find(".thumb.") != std::string::npos) {
		return false;
	} else if ((extension != ".jpg") && (extension != ".jpeg") && (extension != ".png")) {
		return false;
	}

	return true;
}

std::string thumbnail_path(fs::path path) {
	fs::path output_path = path;
	output_path.replace_extension(".thumb" + path.extension().string());

	return output_path.string();
}

void thumbnail(std::string path, std::string thumb_path, Mat &src, Mat &dst, gpu::GpuMat &g_src, gpu::GpuMat &g_dst) {
	if (!is_valid_image(path)) {
		return;
	}

	src = imread(path);

	if (src.empty()) {
		std::cerr << "[!] Image could not be read: " << path << std::endl;
		return;
	}

	double thumb_height = (THUMB_WIDTH * src.rows) / src.cols;

	g_src.upload(src);
	gpu::resize(g_src, g_dst, Size(THUMB_WIDTH, thumb_height), 0, 0, INTER_AREA);
	g_dst.download(dst);

	imwrite(thumb_path, dst);
	std::cout << "[*] Thumbnailed " << path << std::endl;
}

int main(int argc, char **argv) {
	Mat src, dst;
	gpu::GpuMat g_src, g_dst;

	if (argc < 2) {
		std::cout << "Usage: thumbnail <images>" << std::endl;
		return EXIT_FAILURE;
	}

	for (int i = 1; i < argc; i++) {
		fs::path path(argv[i]);

		if (fs::is_regular_file(path)) {
			thumbnail(path.string(), thumbnail_path(path), src, dst, g_src, g_dst);
		} else {
			for (fs::recursive_directory_iterator dir_iter(path), end_iter; dir_iter != end_iter; ++dir_iter) {
				thumbnail(path.string(), thumbnail_path(path), src, dst, g_src, g_dst);
			}
		}
	}

	return EXIT_SUCCESS;
}
