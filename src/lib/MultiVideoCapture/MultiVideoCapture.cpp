#include "MultiVideoCapture.hpp"
#include "VideoCaptureType.hpp"

#include <numeric>

#include <atomic>
std::atomic_bool gKeepCamOpening;
std::atomic_bool gCamSetChanged;	//TODO adding the function for online video settings change.

#include "ThreadPool.hpp"
ThreadPool::ThreadPool* pThread_pool = NULL;


std::vector<VideoCaptureType*> gVidCaps;	// to hide from the MultiVideoCapture class


void openVideo(std::vector<int> camIds, int apiPreference) {
	const int nbDevs = (int)camIds.size();
	bool (VideoCaptureType::*openfunc)(int, int) = &VideoCaptureType::open;

	// check the video status whether open or not
	int waitFor = 2000;
	do {
		std::vector<std::future<bool> > futures;
		for (int i = 0; i < nbDevs; i++) {
			if (gVidCaps[i]->status() == VideoStatus::VIDEO_STATUS_CLOSED
				|| gVidCaps[i]->status() == VideoStatus::VIDEO_STATUS_UNKNOWN) {
				int id = camIds[i];
				futures.emplace_back(pThread_pool->EnqueueJob(openfunc, gVidCaps[i], id, apiPreference));
			}
		}

		// wait until all jobs are done.
		bool status = false;
		for (int i = 0; i < futures.size(); i++) {
			futures[i].wait();
			status = status || futures[i].get();
		}

		// keep trying to open each video in every [waitFor] sec.
		std::this_thread::sleep_for(std::chrono::milliseconds(waitFor));
	} while (gKeepCamOpening);
}


void openFile(std::vector<std::string> filenames) {
	const size_t nbFiles = filenames.size();
	bool (VideoCaptureType::*openfunc)(const std::string&) = &VideoCaptureType::open;

	// open the video files
	std::vector<std::future<bool> > futures;
	for (size_t i = 0; i < nbFiles; i++) {
		if (gVidCaps[i]->status() == VideoStatus::VIDEO_STATUS_CLOSED) {;
			futures.emplace_back(pThread_pool->EnqueueJob(openfunc, gVidCaps[i], filenames[i]));
		}
	}
}


MultiVideoCapture::MultiVideoCapture(bool verbose) {
	mVideoIds.clear();
	mVerbose = verbose;
	mRetryOpening = false;
}


MultiVideoCapture::~MultiVideoCapture() {
	release();
}


void MultiVideoCapture::open(const std::string& filename) {
	this->open({ filename });
}


void MultiVideoCapture::open(int index, bool retry) {
	this->open({ index }, -1, retry);
}


void MultiVideoCapture::open(int index, int apiPreference, bool retry) {
	this->open({ index }, apiPreference, retry);
}


void MultiVideoCapture::open(const std::vector<std::string>& filenames) {
	this->resize(filenames.size());
	std::iota(mVideoIds.begin(), mVideoIds.end(), 0);

	pThread_pool = new ThreadPool::ThreadPool(gVidCaps.size() * 2 + 4);

	mRetryOpening = false;
	gKeepCamOpening.store(mRetryOpening ? true : false);

	pThread_pool->EnqueueJob(openFile, filenames);

	while (!isAllOpened()) {
		if (mVerbose) {
			std::cout << ".";
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	if (mVerbose) {
		std::cout << "one of the videos is open!" << std::endl;
	}
}


void MultiVideoCapture::open(std::vector<int> indices, bool retry) {
	this->open(indices, -1, retry);
}


void MultiVideoCapture::open(std::vector<int> indices, int apiPreference, bool retry) {
	this->resize(indices.size());
	mVideoIds = indices;

	pThread_pool = new ThreadPool::ThreadPool(gVidCaps.size() * 2 + 4);

	mRetryOpening = retry;
	gKeepCamOpening.store(mRetryOpening ? true : false);

	pThread_pool->EnqueueJob(openVideo, indices, apiPreference);

	while (!isAnyOpened()) {
		if (mVerbose) {
			std::cout << ".";
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	
	if (mVerbose) {
		std::cout << "one of the cameras is open!" << std::endl;
	}
}


void MultiVideoCapture::release() {
	// stop thread flag
	gKeepCamOpening.store(false);
	
	const int nbDevs = (int)gVidCaps.size();
	void (VideoCaptureType::*releasefunc)() = &VideoCaptureType::release;
	std::vector<std::future<void> > futures;

	for (int i = 0; i < nbDevs; i++) {
		if (gVidCaps[i]->status() != VideoStatus::VIDEO_STATUS_CLOSED) {
			futures.emplace_back(pThread_pool->EnqueueJob(releasefunc, gVidCaps[i]));
		}
	}

	// wait until all jobs are done.
	for (int i = 0; i < futures.size(); i++) {
		futures[i].wait();
	}

	if (pThread_pool) {
		delete[] pThread_pool;
		pThread_pool = NULL;
	}

	// release instances of VideoCapture from memory
	for (auto vc : gVidCaps) {
		delete[] vc;
	}
	gVidCaps.clear();
}


bool MultiVideoCapture::isOpened(int index) const {
	if (gVidCaps[index]->isOpened() == true)
		return true;
	else
		return false;
}


bool MultiVideoCapture::isOpened(bool all) const {
	if (all == true) {
		return isAllOpened();
	}
	else {
		return isAnyOpened();
	}
}


bool MultiVideoCapture::isAnyOpened() const {
	for (int i = 0; i < (int)gVidCaps.size(); i++) {
		if (gVidCaps[i]->isOpened() == true) {
			return true;
		}
	}

	return false;
}


bool MultiVideoCapture::isAllOpened() const {
	if (gVidCaps.empty()) {
		return false;
	}
	for (int i = 0; i < (int)gVidCaps.size(); i++) {
		if (gVidCaps[i]->isOpened() == false) {
			return false;
		}
	}

	return true;
}


bool MultiVideoCapture::grab() {
	const size_t nbDevs = gVidCaps.size();

	std::vector<std::future<bool> > futures;
	bool (VideoCaptureType::*grabfunc)() = &VideoCaptureType::grab;

	for (int i = 0; i < nbDevs; i++) {
		if (gVidCaps[i]->status() == VideoStatus::VIDEO_STATUS_OPENED) {
			futures.emplace_back(pThread_pool->EnqueueJob(grabfunc, gVidCaps[i]));
		}
	}

	// wait until all jobs are done.
	bool status = false;
	for (int i = 0; i < futures.size(); i++) {
		futures[i].wait();
		status = status || futures[i].get();
	}

	return status;
}


bool MultiVideoCapture::retrieve(std::vector<FrameType>& frames, int flag) {
	const size_t nbDevs = gVidCaps.size();
	if (nbDevs != frames.size()) {
		frames.resize(nbDevs);
	}

	std::vector<std::future<bool> > futures;
	bool (VideoCaptureType::* retrievefunc)(FrameType&, int) = &VideoCaptureType::retrieve;

	for (int i = 0; i < nbDevs; i++) {
		if (gVidCaps[i]->status() == VideoStatus::VIDEO_STATUS_OPENED) {
			futures.emplace_back(pThread_pool->EnqueueJob(retrievefunc, gVidCaps[i], std::ref(frames[i]), flag));
		}
	}

	// wait until all jobs are done.
	bool status = false;
	for (int i = 0; i < futures.size(); i++) {
		futures[i].wait();
		status = status || futures[i].get();
	}

	return status;
}


MultiVideoCapture& MultiVideoCapture::operator >> (std::vector<FrameType>& frames) {
	read(frames);

	return *this;
}


bool MultiVideoCapture::read(std::vector<FrameType>& frames) {
	const int nbDevs = (int)gVidCaps.size();
	if (nbDevs != frames.size())
		frames.resize(nbDevs);

	std::vector<std::future<bool> > futures;
	bool (VideoCaptureType::*readfunc)(FrameType&) = &VideoCaptureType::read;

	for (int i = 0; i < nbDevs; i++) {
		if (gVidCaps[i]->status() == VideoStatus::VIDEO_STATUS_OPENED) {
			futures.emplace_back(pThread_pool->EnqueueJob(readfunc, gVidCaps[i], std::ref(frames[i])));
		}
		else
			frames[i].release();
	}

	// wait until all jobs are done.
	bool status = false;
	for (int i = 0; i < futures.size(); i++) {
		futures[i].wait();
		status = status || futures[i].get();
	}

	return status;
}


bool MultiVideoCapture::set(int propId, double value) {
	return this->set(propId, std::vector<double>(gVidCaps.size(), value));
}


bool MultiVideoCapture::set(int propId, std::vector<double> values) {
	// get current settings
	std::vector<double> prevValues(mVideoIds.size());
	for (size_t i = 0; i < gVidCaps.size(); i++) {
		prevValues[i] = gVidCaps[i]->get(propId);
	}

	std::vector<bool> results(mVideoIds.size(), false);

	for (size_t i = 0; i < mVideoIds.size(); i++) {
		results[i] = gVidCaps[i]->set(propId, values[i]);
	}

	bool res = false;
	for (const auto& result : results) {
		res = res || result;
	}

	// if setting is failed then restore settings for all devices.
	if (res == false) {
		for (size_t i = 0; i < gVidCaps.size(); i++) {
			if (results[i] == true) {
				gVidCaps[i]->set(propId, prevValues[i]);
			}
		}
	}

	return res;
}


bool MultiVideoCapture::set(int index, int propId, double value) {
	return gVidCaps[index]->set(propId, value);
}


std::vector<double> MultiVideoCapture::get(int propId) const {
	std::vector<double> res(gVidCaps.size(), -1);
	for (size_t i = 0; i < gVidCaps.size(); i++) {
		res[i] = gVidCaps[i]->get(propId);
	}

	return res;
}


double MultiVideoCapture::get(int index, int propId) const {
	return gVidCaps[index]->get(propId);
}


void MultiVideoCapture::verbose(bool verbose) {
	mVerbose = verbose;

	for (auto vc : gVidCaps) {
		vc->verbose(mVerbose);
	}
}


void MultiVideoCapture::resize(size_t size) {
	if (gVidCaps.size() != size) {
		release();

		gVidCaps.resize(size);
		mVideoIds.resize(size, -1);
		for (int i = 0; i < size; i++) {
			gVidCaps[i] = new VideoCaptureType;
		}
	}
}
