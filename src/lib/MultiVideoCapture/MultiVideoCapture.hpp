#ifndef MULTI_VIDEO_CAPTURE_H_
#define MULTI_VIDEO_CAPTURE_H_


#ifndef __cplusplus
#  error MultiVideoCapture.hpp header must be compiled as C++
#endif

#ifndef MULTIVIDEOCAPTURE_EXPORTS
#  ifdef DLL_EXPORTS
#    if (defined _WIN32 || defined WINCE || defined __CYGWIN__)
#      define MULTIVIDEOCAPTURE_EXPORTS __declspec(dllexport)
#    elif defined __GNUC__ && __GNUC__ >= 4 || defined(__APPLE__)
#      define MULTIVIDEOCAPTURE_EXPORTS __attribute__ ((visibility ("default")))
#      define FRAMETYPE_TEMPLATE
#    endif
#  else
#    if (defined _WIN32 || defined WINCE || defined __CYGWIN__)
#    define MULTIVIDEOCAPTURE_EXPORTS __declspec(dllimport)
#    elif defined __GNUC__ && __GNUC__ >= 4 || defined(__APPLE__)
#      define MULTIVIDEOCAPTURE_EXPORTS
#    endif
#  endif	// !DLL_EXPORTS
#endif	// !FRAMETYPE_EXPORTS


#include <iostream>
#include <vector>

#include "opencv2/opencv.hpp"
#include "FrameType.hpp"

class MULTIVIDEOCAPTURE_EXPORTS MultiVideoCapture {
public:
	MultiVideoCapture(bool verbose = false);
	virtual ~MultiVideoCapture();

	virtual void open(const std::string& filename);
	virtual void open(int index, bool retry = false);
	virtual void open(int index, int apiPreference, bool retry = false);
	virtual void open(const std::vector<std::string>& filenames);
	virtual void open(std::vector<int> indices, bool retry = false);
	virtual void open(std::vector<int> indices, int apiPreference, bool retry = false);
	virtual void release();

	virtual bool isOpened(int cameraNum) const;
	virtual bool isOpened(bool all = false) const;
	virtual bool isAnyOpened() const;
	virtual bool isAllOpened() const;

	virtual bool read(std::vector<FrameType>& frames);
	virtual MultiVideoCapture& operator >> (std::vector<FrameType>& frames);

	virtual bool set(std::vector<int> cameraIds, cv::Size resolution, float fps = 30.f);

	virtual void verbose(bool verbose = false);

protected:
	virtual void resize(size_t size);
	virtual bool set(int cameraId, cv::Size resolution, float fps = 30.f);

protected:
	std::vector<int> mCameraIds;
	int mApiPreference;
	bool mVerbose;
	bool mRetryOpening;

	std::vector<cv::Size> mResolutions;
	std::vector<float> mFpses;
};


#endif // !MULTI_VIDEO_CAPTURE_H_
