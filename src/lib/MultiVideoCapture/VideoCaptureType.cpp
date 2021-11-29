#include "VideoCaptureType.hpp"

#include <chrono>
#include <thread>

#include "boost/filesystem.hpp"
namespace fs = boost::filesystem;


VideoCaptureType::VideoCaptureType() {
	mStatus = VideoStatus::VIDEO_STATUS_CLOSED;
	mIsSet = false;
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

		std::string name = cv::VideoCapture::getBackendName();	//TODO check this line!
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


double VideoCaptureType::get(int propId) const {
	return cv::VideoCapture::get(propId);
}


void VideoCaptureType::verbose(bool verbose) {
	mVerbose = verbose;
}
