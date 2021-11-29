#include "VideoCaptureType.hpp"

#include <chrono>
#include <thread>

#include "boost/filesystem.hpp"
namespace fs = boost::filesystem;


VideoCaptureType::VideoCaptureType() {
	this->release();
	mIsSet = false;
	mResolution = { 640, 480 };
	mFps = 30.f;
	mVerbose = false;
}


VideoCaptureType::~VideoCaptureType() {
	this->release();
}


bool VideoCaptureType::open(const std::string& filename) {
	fs::path fName = filename;

	// file existance check
	if (!fs::exists(fName)) {
		std::lock_guard<std::mutex> lock(mMtxMsg);
		std::string msg = "The file (" + fName.filename().string() + " cannot be opened";
		if (mVerbose) {
			std::cout << msg << std::endl;
		}
		//throw std::runtime_error(msg);
		return false;
	}

	// check camera status
	if (mStatus == CamStatus::CAM_STATUS_OPENED) {
		std::lock_guard<std::mutex> lock(mMtxMsg);
		std::string msg = "The file (" + fName.filename().string() + " cannot be opened";
		if (mVerbose) {
			std::cout << msg << std::endl;
		}
		//throw std::runtime_error(msg);
		return false;
	}
	else if (mStatus == CamStatus::CAM_STATUS_SETTING) {
		std::lock_guard<std::mutex> lock(mMtxMsg);
		std::string msg = "The file (" + fName.filename().string() + " cannot be opened";
		if (mVerbose) {
			std::cout << msg << std::endl;
		}
		//throw std::runtime_error(msg);
		return false;
	}
	else if (mStatus == CamStatus::CAM_STATUS_OPENING) {
		std::lock_guard<std::mutex> lock(mMtxMsg);
		std::string msg = "The file (" + fName.filename().string() + " cannot be opened";
		if (mVerbose) {
			std::cout << msg << std::endl;
		}
		//throw std::runtime_error(msg);
		return false;
	}

	// get previous status
	CamStatus prevStatus = mStatus;

	// try to open the camera
	release();	// handling the camera disconnected previously
	{
		std::lock_guard<std::mutex> lock(mMtxStatus);
		mStatus = CamStatus::CAM_STATUS_OPENING;
	}

	bool cam_status = false;
	cam_status = cv::VideoCapture::open(fName.string());

	if (cam_status == true) {
		std::lock_guard<std::mutex> lock(mMtxStatus);
		mStatus = CamStatus::CAM_STATUS_OPENED;
	}
	else {
		release();
		std::lock_guard<std::mutex> lock(mMtxMsg);
		std::string msg = "The file (" + fName.filename().string() + " cannot be opened";
		if (mVerbose) {
			std::cout << msg << std::endl;
		}
		mStatus = prevStatus;
		throw std::runtime_error(msg);
	}

	return cam_status;
}


bool VideoCaptureType::open(int index) {
	return this->open(index, -1);
}


bool VideoCaptureType::open(int index, int apiPreference) {
	// check camera status
	if (mStatus == CamStatus::CAM_STATUS_OPENED) {
		std::lock_guard<std::mutex> lock(mMtxMsg);
		std::string msg = "camera " + std::to_string(index) + " is already opened";
		if (mVerbose) {
			std::cout << msg << std::endl;
		}
		//throw std::runtime_error(msg);
		return false;
	}
	else if (mStatus == CamStatus::CAM_STATUS_SETTING) {
		std::lock_guard<std::mutex> lock(mMtxMsg);
		std::string msg = "camera " + std::to_string(index) + " is already opened and is on setting";
		if (mVerbose) {
			std::cout << msg << std::endl;
		}
		//throw std::runtime_error(msg);
		return false;
	}
	else if (mStatus == CamStatus::CAM_STATUS_OPENING) {
		std::lock_guard<std::mutex> lock(mMtxMsg);
		std::string msg = "camera " + std::to_string(index) + " is opening";
		if (mVerbose) {
			std::cout << msg << std::endl;
		}
		//throw std::runtime_error(msg);
		return false;
	}

	// try to open the camera
	release();	// handling the camera disconnected previously
	{
		std::lock_guard<std::mutex> lock(mMtxStatus);
		mStatus = CamStatus::CAM_STATUS_OPENING;
	}
	mCamId = index;
	bool cam_status = false;
	if (apiPreference == -1)
		cam_status = cv::VideoCapture::open(index);
	else
		cam_status = cv::VideoCapture::open(index, apiPreference);

	if (cam_status == true && cv::VideoCapture::grab() == true) {
		{
			std::lock_guard<std::mutex> lock(mMtxStatus);
			mStatus = CamStatus::CAM_STATUS_OPENED;
		}
		if (mIsSet)
			this->set(mResolution, mFps);
	}
	else {
		release();
		std::lock_guard<std::mutex> lock(mMtxMsg);
		std::string msg = "can't open a camera " + std::to_string(index);
		if (mVerbose) {
			std::cout << msg << std::endl;
		}
		throw std::runtime_error(msg);
	}

	return cam_status;
}


bool VideoCaptureType::isOpened() const {
	if (mStatus == CamStatus::CAM_STATUS_OPENED || mStatus == CamStatus::CAM_STATUS_SETTING)
		return true;
	else
		return false;
}


CamStatus VideoCaptureType::status() const {
	return mStatus;
}


void VideoCaptureType::release() {
	{
		std::lock_guard<std::mutex> lock(mMtxStatus);
		mStatus = CamStatus::CAM_STATUS_CLOSED;
	}

	cv::VideoCapture::release();
}


bool VideoCaptureType::grab() {
	bool res = cv::VideoCapture::grab();
	mGrabTimestamp = std::chrono::system_clock::now();

	return res;
}


bool VideoCaptureType::retrieve(FrameType& frame, int flag) {
	bool status = cv::VideoCapture::retrieve(frame.mat(), flag);
	frame.setTimestamp(mGrabTimestamp);

	return status;
}


VideoCaptureType& VideoCaptureType::operator >> (FrameType& frame) {
	read(frame);

	return *this;
}


bool VideoCaptureType::read(FrameType& frame) {
	if (cv::VideoCapture::grab() && mStatus == CamStatus::CAM_STATUS_OPENED) {
		this->retrieve(frame);
	}
	else if (mStatus == CamStatus::CAM_STATUS_SETTING) {
		frame.release();
	}
	else {
		//release();
		{
			std::lock_guard<std::mutex> lock(mMtxStatus);
			mStatus = CamStatus::CAM_STATUS_CLOSED;
		}
		frame.release();
	}

	return !frame.empty();
}


bool VideoCaptureType::set(int propId, double value) {
	CamStatus lastStatus = mStatus;
	{
		std::lock_guard<std::mutex> lock(mMtxStatus);
		mStatus = CamStatus::CAM_STATUS_SETTING;
	}

	bool res = cv::VideoCapture::set(propId, value);

	{
		std::lock_guard<std::mutex> lock(mMtxStatus);
		mStatus = lastStatus;
	}

	return res;
}


bool VideoCaptureType::set(cv::Size resolution, float fps) {
	{
		std::lock_guard<std::mutex> lock(mMtxStatus);
		mStatus = CamStatus::CAM_STATUS_SETTING;
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
			mStatus = CamStatus::CAM_STATUS_OPENED;
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
			mStatus = CamStatus::CAM_STATUS_OPENED;
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
