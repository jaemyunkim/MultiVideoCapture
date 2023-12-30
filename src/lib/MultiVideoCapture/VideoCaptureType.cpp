#include "VideoCaptureType.hpp"

#include <chrono>
#include <thread>

#include "boost/filesystem.hpp"
namespace fs = boost::filesystem;


VideoCaptureType::VideoCaptureType() {
	mStatus = VideoStatus::VIDEO_STATUS_CLOSED;
	mIsSet = false;
	mResolution = { 640, 480 };
	mFps = 30.f;
	mVerbose = false;
}


VideoCaptureType::~VideoCaptureType() {
	this->release();
}


bool VideoCaptureType::open(const std::string& filename) {
	return this->open(filename, -1);
}


bool VideoCaptureType::open(const std::string& filename, int apiPreference) {
	fs::path fName = filename;

	// file existance check
	if (!fs::exists(fName)) {
		std::string msg = "The target (" + fName.filename().string() + " cannot be opened";
		if (mVerbose) {
			std::lock_guard<std::mutex> lock(mMtxMsg);
			std::cout << msg << std::endl;
		}
		//throw std::runtime_error(msg);
		return false;
	}

	// check video status
	if (mStatus == VideoStatus::VIDEO_STATUS_OPENED) {
		std::string msg = "The target (" + fName.filename().string() + " cannot be opened";
		if (mVerbose) {
			std::lock_guard<std::mutex> lock(mMtxMsg);
			std::cout << msg << std::endl;
		}
		//throw std::runtime_error(msg);
		return false;
	}
	else if (mStatus == VideoStatus::VIDEO_STATUS_SETTING) {
		std::string msg = "The target (" + fName.filename().string() + " cannot be opened";
		if (mVerbose) {
			std::lock_guard<std::mutex> lock(mMtxMsg);
			std::cout << msg << std::endl;
		}
		//throw std::runtime_error(msg);
		return false;
	}
	else if (mStatus == VideoStatus::VIDEO_STATUS_OPENING) {
		std::string msg = "The target (" + fName.filename().string() + " cannot be opened";
		if (mVerbose) {
			std::lock_guard<std::mutex> lock(mMtxMsg);
			std::cout << msg << std::endl;
		}
		//throw std::runtime_error(msg);
		return false;
	}

	// try to open the video
	VideoStatus prevStatus = mStatus;
	{
		std::lock_guard<std::mutex> lock(mMtxStatus);
		mStatus = VideoStatus::VIDEO_STATUS_OPENING;
	}

	bool video_status = false;
	if (apiPreference == -1) {
		video_status = cv::VideoCapture::open(fName.string());
	}
	else {
		video_status = cv::VideoCapture::open(fName.string(), apiPreference);
	}

	if (video_status == true) {
		std::lock_guard<std::mutex> lock(mMtxStatus);
		mStatus = VideoStatus::VIDEO_STATUS_OPENED;
	}
	else {
		{
			std::lock_guard<std::mutex> lock(mMtxStatus);
			mStatus = prevStatus;
		}
		std::string msg = "The file (" + fName.filename().string() + " cannot be opened";
		if (mVerbose) {
			std::lock_guard<std::mutex> lock(mMtxMsg);
			std::cout << msg << std::endl;
		}
		//throw std::runtime_error(msg);
		return false;
	}

	return video_status;
}


bool VideoCaptureType::open(int index) {
	return this->open(index, -1);
}


bool VideoCaptureType::open(int index, int apiPreference) {
	// check video status
	if (mStatus == VideoStatus::VIDEO_STATUS_OPENED) {
		std::string msg = "camera " + std::to_string(index) + " is already opened";
		if (mVerbose) {
			std::lock_guard<std::mutex> lock(mMtxMsg);
			std::cout << msg << std::endl;
		}
		//throw std::runtime_error(msg);
		return false;
	}
	else if (mStatus == VideoStatus::VIDEO_STATUS_SETTING) {
		std::string msg = "camera " + std::to_string(index) + " is already opened and is on setting";
		if (mVerbose) {
			std::lock_guard<std::mutex> lock(mMtxMsg);
			std::cout << msg << std::endl;
		}
		//throw std::runtime_error(msg);
		return false;
	}
	else if (mStatus == VideoStatus::VIDEO_STATUS_OPENING) {
		std::string msg = "camera " + std::to_string(index) + " is opening";
		if (mVerbose) {
			std::lock_guard<std::mutex> lock(mMtxMsg);
			std::cout << msg << std::endl;
		}
		//throw std::runtime_error(msg);
		return false;
	}

	// try to open the video
	VideoStatus prevStatus = mStatus;
	{
		std::lock_guard<std::mutex> lock(mMtxStatus);
		mStatus = VideoStatus::VIDEO_STATUS_OPENING;
	}
	mVideoId = index;
	bool video_status = false;
	if (apiPreference == -1) {
		video_status = cv::VideoCapture::open(index);
	}
	else {
		video_status = cv::VideoCapture::open(index, apiPreference);
	}

	if (video_status == true && cv::VideoCapture::grab() == true) {
		{
			std::lock_guard<std::mutex> lock(mMtxStatus);
			mStatus = VideoStatus::VIDEO_STATUS_OPENED;
		}
		if (mIsSet)
			this->set(mResolution, mFps);


		std::string name = cv::VideoCapture::getBackendName();
	}
	else {
		{
			std::lock_guard<std::mutex> lock(mMtxStatus);
			mStatus = prevStatus;
		}
		std::string msg = "can't open a camera " + std::to_string(index);
		if (mVerbose) {
			std::lock_guard<std::mutex> lock(mMtxMsg);
			std::cout << msg << std::endl;
		}
		//throw std::runtime_error(msg);
		return false;
	}

	return video_status;
}


bool VideoCaptureType::isOpened() const {
	if (mStatus == VideoStatus::VIDEO_STATUS_OPENED)
		return true;
	else
		return false;
}


VideoStatus VideoCaptureType::status() const {
	return mStatus;
}


void VideoCaptureType::release() {
	cv::VideoCapture::release();
	{
		std::lock_guard<std::mutex> lock(mMtxStatus);
		mStatus = VideoStatus::VIDEO_STATUS_CLOSED;
	}
}


bool VideoCaptureType::grab() {
	bool status = false;
	if (mStatus == VideoStatus::VIDEO_STATUS_OPENED) {
		status = cv::VideoCapture::grab();
		if (status == true) {
			mGrabTimestamp = std::chrono::system_clock::now();
		}
	}

	return status;
}


bool VideoCaptureType::retrieve(FrameType& frame, int flag) {
	bool status = false;
	if (mStatus == VideoStatus::VIDEO_STATUS_OPENED) {
		status = cv::VideoCapture::retrieve(frame.mat(), flag);
		if (status == true) {
			frame.setTimestamp(mGrabTimestamp);
		}
		else
			frame.release();
	}

	return status;
}


VideoCaptureType& VideoCaptureType::operator >> (FrameType& frame) {
	read(frame);

	return *this;
}


bool VideoCaptureType::read(FrameType& frame) {
	bool status = cv::VideoCapture::grab();
	if (status && mStatus == VideoStatus::VIDEO_STATUS_OPENED) {
		this->retrieve(frame);
	}
	else if (mStatus == VideoStatus::VIDEO_STATUS_SETTING) {
		frame.release();
	}
	else {
		frame.release();
		//release();
		{
			std::lock_guard<std::mutex> lock(mMtxStatus);
			mStatus = VideoStatus::VIDEO_STATUS_UNKNOWN;
		}
	}

	return !frame.empty();
}


bool VideoCaptureType::set(int propId, double value) {
	VideoStatus lastStatus = mStatus;
	{
		std::lock_guard<std::mutex> lock(mMtxStatus);
		mStatus = VideoStatus::VIDEO_STATUS_SETTING;
	}

	bool res = cv::VideoCapture::set(propId, value);

	{
		std::lock_guard<std::mutex> lock(mMtxStatus);
		mStatus = lastStatus;
	}

	return res;
}


bool VideoCaptureType::set(cv::Size resolution, float fps) {
	VideoStatus prevStatus = mStatus;
	{
		std::lock_guard<std::mutex> lock(mMtxStatus);
		mStatus = VideoStatus::VIDEO_STATUS_SETTING;
	}
	mIsSet = true;

	// get old settings
	cv::Size oldSize((int)cv::VideoCapture::get(cv::CAP_PROP_FRAME_WIDTH), (int)cv::VideoCapture::get(cv::CAP_PROP_FRAME_HEIGHT));
	double oldFps = cv::VideoCapture::get(cv::CAP_PROP_FPS);
	double oldAutofocus = cv::VideoCapture::get(cv::CAP_PROP_AUTOFOCUS);

	if (resolution == oldSize && fps == oldFps)
		return true;

	if (resolution == cv::Size(-1, -1))	resolution = mResolution;
	else	mResolution = resolution;
	if (mFps == -1.f)	fps = mFps;
	else	mFps = fps;

	// disable autofocus
	if (oldAutofocus != 0) {
		cv::VideoCapture::set(cv::CAP_PROP_AUTOFOCUS, 0);
	}

	// set resolution and fps
	bool statusSize = true, statusFps = true;
	if (resolution != oldSize) {
		statusSize =
			cv::VideoCapture::set(cv::CAP_PROP_FRAME_WIDTH, resolution.width) &&
			cv::VideoCapture::set(cv::CAP_PROP_FRAME_HEIGHT, resolution.height);
	}
	if (fps != oldFps) {
		statusFps = cv::VideoCapture::set(cv::CAP_PROP_FPS, fps);
	}

	if (statusSize && statusFps) {
		{
			std::lock_guard<std::mutex> lock(mMtxStatus);
			mStatus = prevStatus;
		}
		return true;
	}
	else {
		// rollback resolution
		cv::VideoCapture::set(cv::CAP_PROP_FRAME_WIDTH, oldSize.width);
		cv::VideoCapture::set(cv::CAP_PROP_FRAME_HEIGHT, oldSize.height);

		// rollback fps
		cv::VideoCapture::set(cv::CAP_PROP_FPS, oldFps);
		{
			std::lock_guard<std::mutex> lock(mMtxStatus);
			mStatus = prevStatus;
		}
		return false;
	}
}


double VideoCaptureType::get(int propId) const {
	switch (propId)
	{
	case cv::CAP_PROP_FRAME_WIDTH:
		return mResolution.width;
	case cv::CAP_PROP_FRAME_HEIGHT:
		return mResolution.height;
	case cv::CAP_PROP_FPS:
		return mFps;
	default:
		return cv::VideoCapture::get(propId);
	}
}


void VideoCaptureType::verbose(bool verbose) {
	mVerbose = verbose;
}
