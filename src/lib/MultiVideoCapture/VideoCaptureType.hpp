#ifndef VIDEO_CAPTURE_TYPE_H_
#define VIDEO_CAPTURE_TYPE_H_


#ifndef __cplusplus
#  error MultiVideoCapture.hpp header must be compiled as C++
#endif

#include <mutex>

#include "opencv2/opencv.hpp"
#include "FrameType.hpp"


enum class VideoStatus {
	VIDEO_STATUS_CLOSED = 0,
	VIDEO_STATUS_OPENING,
	VIDEO_STATUS_OPENED,
	VIDEO_STATUS_SETTING,
	VIDEO_STATUS_UNKNOWN,
};


class VideoCaptureType : protected cv::VideoCapture {
public:
	VideoCaptureType();
	virtual ~VideoCaptureType();

	virtual bool open(const std::string& filename);
	virtual bool open(const std::string& filename, int apiPreference);
	virtual bool open(int index);
	virtual bool open(int index, int apiPreference);
	virtual bool isOpened() const;
	virtual VideoStatus status() const;

	virtual void release();

	virtual bool grab();
	virtual bool retrieve(FrameType& frame, int flag = 0);
	virtual VideoCaptureType& operator >> (FrameType& frame);
	virtual bool read(FrameType& frame);

	virtual bool set(int propId, double value);
	virtual bool set(cv::Size resolution = { -1, -1 }, float fps = -1.f);
	virtual double get(int propId) const;

	virtual void verbose(bool verbose = false);

protected:
	int mVideoId;
	VideoStatus mStatus;
	bool mIsSet;

	std::chrono::system_clock::time_point mGrabTimestamp;
	int mCloseCount;
	int mCloseLimit;

	cv::Size mResolution;
	float mFps;

	bool mVerbose;

	std::mutex mMtxStatus;
	std::mutex mMtxMsg;
};


#endif // !VIDEO_CAPTURE_TYPE_H_
