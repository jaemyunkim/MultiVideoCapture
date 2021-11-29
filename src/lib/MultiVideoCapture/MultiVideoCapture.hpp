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

class MultiVideoCapture {
public:
	MULTIVIDEOCAPTURE_EXPORTS MultiVideoCapture(bool verbose = false);
	MULTIVIDEOCAPTURE_EXPORTS virtual ~MultiVideoCapture();
	
	MULTIVIDEOCAPTURE_EXPORTS virtual void open(const std::string& filename);
	MULTIVIDEOCAPTURE_EXPORTS virtual void open(int index, bool retry = false);
	MULTIVIDEOCAPTURE_EXPORTS virtual void open(int index, int apiPreference, bool retry = false);
	MULTIVIDEOCAPTURE_EXPORTS virtual void open(const std::vector<std::string>& filenames);
	MULTIVIDEOCAPTURE_EXPORTS virtual void open(std::vector<int> indices, bool retry = false);
	MULTIVIDEOCAPTURE_EXPORTS virtual void open(std::vector<int> indices, int apiPreference, bool retry = false);
	MULTIVIDEOCAPTURE_EXPORTS virtual void release();
	
	MULTIVIDEOCAPTURE_EXPORTS virtual bool isOpened(int index) const;
	MULTIVIDEOCAPTURE_EXPORTS virtual bool isOpened(bool all = false) const;
	MULTIVIDEOCAPTURE_EXPORTS virtual bool isAnyOpened() const;
	MULTIVIDEOCAPTURE_EXPORTS virtual bool isAllOpened() const;
	
	MULTIVIDEOCAPTURE_EXPORTS virtual bool grab();
	MULTIVIDEOCAPTURE_EXPORTS virtual bool retrieve(std::vector<FrameType>& frames, int flag = 0);
	MULTIVIDEOCAPTURE_EXPORTS virtual MultiVideoCapture& operator >> (std::vector<FrameType>& frames);
	MULTIVIDEOCAPTURE_EXPORTS virtual bool read(std::vector<FrameType>& frames);
	
	MULTIVIDEOCAPTURE_EXPORTS virtual bool set(int propId, double value);
	MULTIVIDEOCAPTURE_EXPORTS virtual bool set(int propId, std::vector<double> value);
	MULTIVIDEOCAPTURE_EXPORTS virtual bool set(int index, int propId, double value);
	MULTIVIDEOCAPTURE_EXPORTS virtual std::vector<double> get(int propId) const;
	MULTIVIDEOCAPTURE_EXPORTS virtual double get(int index, int propId) const;
	
	MULTIVIDEOCAPTURE_EXPORTS virtual void verbose(bool verbose = false);

protected:
	virtual void resize(size_t size);

protected:
	std::vector<int> mVideoIds;
	bool mVerbose;
	bool mRetryOpening;
};


#endif // !MULTI_VIDEO_CAPTURE_H_
