#include <iostream>
#include <thread>
#include <chrono>
#include <numeric>

#include "opencv2/opencv.hpp"
#include "MultiVideoCapture.hpp"

#include "boost/filesystem.hpp"


std::vector<int> camera_open(MultiVideoCapture& mvc, std::vector<int> camIds, bool retry = false);
std::vector<int> file_open(MultiVideoCapture& mvc, std::vector<std::string> filenames, bool retry = false);


int main() {
	std::vector<int> videoIds;

	MultiVideoCapture mvc(true);
	videoIds = camera_open(mvc, { 0, 1 }, true);

	std::vector<FrameType> images(videoIds.size());
	cv::Size resolution = { 640, 360 };
	float fps = 30.f;


	mvc.set(cv::CAP_PROP_FRAME_WIDTH, resolution.width);
	mvc.set(cv::CAP_PROP_FRAME_HEIGHT, resolution.height);
	mvc.set(cv::CAP_PROP_FPS, fps);


	std::chrono::milliseconds duration(long(1000.f / fps));
	std::chrono::system_clock::time_point wait_until;
	std::chrono::system_clock::time_point capture_times[2];
	std::chrono::system_clock::time_point cam_times[2];
	char c = ' ';
	while (c != 17) {	// 17 == ctrl + q
		wait_until = std::chrono::system_clock::now() + duration;

		capture_times[0] = std::chrono::system_clock::now();
		//mvc >> images;
		if (mvc.grab())
			mvc.retrieve(images);
		capture_times[1] = std::chrono::system_clock::now();

		for (int i = 0; i < videoIds.size(); i++) {
			cv::flip(images[i].mat(), images[i].mat(), 1);	// horizontal flip

			const int id = videoIds[i];
			cam_times[i] = images[i].timestamp();
			if (!images[i].empty())
				cv::imshow("cam " + std::to_string(id), images[i].mat());
			else
				cv::imshow("cam " + std::to_string(id), cv::Mat::zeros(resolution, CV_8UC3));
		}

		std::chrono::duration<double> capture_sec = capture_times[1] - capture_times[0];
		std::chrono::duration<double> diff_sec = cam_times[0] - cam_times[1];
		diff_sec = (diff_sec >= diff_sec.zero()) ? diff_sec : -diff_sec;
		std::cout << "capture time: " << std::setw(9) << std::left << capture_sec.count() << " sec";
		std::cout << "\ttime difference: " << std::setw(9) << std::left << diff_sec.count() << " sec" << std::endl;

		c = cv::waitKey(1);

		std::this_thread::sleep_until(wait_until);
	}

	mvc.release();

	return 1;
}


std::vector<int> camera_open(MultiVideoCapture& mvc, std::vector<int> camIds, bool retry) {
	mvc.open(camIds, CV_CAP_DSHOW, retry);
	
	return camIds;
}


std::vector<int> file_open(MultiVideoCapture& mvc, std::vector<std::string> filenames, bool retry) {
	std::vector<int> videoIds(filenames.size());
	std::iota(videoIds.begin(), videoIds.end(), 0);

	mvc.open(filenames);

	return videoIds;
}
