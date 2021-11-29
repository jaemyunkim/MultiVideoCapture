#include <iostream>
#include <thread>
#include <chrono>

#include "opencv2/opencv.hpp"
#include "MultiVideoCapture.hpp"


int main() {
	std::vector<int> camIds = { 0, 1 };
	std::vector<FrameType> images(camIds.size());
	//cv::Size resolution = { 1920, 1080 };
	//cv::Size resolution = { 1280, 720 };
	//cv::Size resolution = { 800, 600 };
	//cv::Size resolution = { 640, 480 };
	cv::Size resolution = { 640, 360 };
	//cv::Size resolution = { 320, 240 };
	float fps = 30.f;

	MultiVideoCapture mvc(true);
	mvc.open(camIds, CV_CAP_DSHOW, true);
	mvc.set(camIds, resolution, fps);

	std::chrono::milliseconds duration(long(1000.f / fps));
	std::chrono::system_clock::time_point wait_until;
	std::chrono::system_clock::time_point capture_times[2];
	std::chrono::system_clock::time_point cam_times[2];
	char c = ' ';
	while (c != 17) {	// 17 == ctrl + q
		wait_until = std::chrono::system_clock::now() + duration;

		capture_times[0] = std::chrono::system_clock::now();
		mvc >> images;
		capture_times[1] = std::chrono::system_clock::now();

		for (int i = 0; i < camIds.size(); i++) {
			cv::flip(images[i].mat(), images[i].mat(), 1);	// horizontal flip

			const int id = camIds[i];
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
