#include <efsw/efsw.hpp>
#include <mutex>

namespace fs = std::filesystem;

namespace ehj {

class FileWatcher : public efsw::FileWatchListener {
public:
	FileWatcher(std::string targetPath) {
		fs::path path(targetPath);
		m_targetAbsolutePath = fs::absolute(path);

		if (!fs::exists(m_targetAbsolutePath)) {
			std::cerr << "[Watcher Error] Path does not exist: " << m_targetAbsolutePath << std::endl;
			m_watchID = -1;
			return;
		}

		std::string directoryToWatch;

		if (fs::is_regular_file(m_targetAbsolutePath)) {
			m_isWatchingSingleFile = true;
			directoryToWatch = m_targetAbsolutePath.parent_path().string();
		} else {
			m_isWatchingSingleFile = false;
			directoryToWatch = m_targetAbsolutePath.string();
		}

		m_watchID = m_watcher.addWatch(directoryToWatch, this, true);
		m_watcher.watch();
	}

	~FileWatcher() = default;

	// efsw background thread callback
	void handleFileAction(efsw::WatchID watchid, const std::string& dir,
						  const std::string& filename, efsw::Action action,
						  std::string oldFilename) override 
	{
		if (action == efsw::Actions::Modified) {
			fs::path eventPath = fs::absolute(fs::path(dir) / filename);

			if (m_isWatchingSingleFile && eventPath != m_targetAbsolutePath) {
				return; 
			}

			std::lock_guard<std::mutex> lock(m_mutex);
			m_hasChanged = true;
			m_lastChangedPath = eventPath.string();
		}
	}

	// Single-use gate: returns true ONCE per modification, then resets itself
	bool CheckAndReset() {
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_hasChanged) {
			// Buffer to ensure text editor/IDE write lock is released
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
			
			m_hasChanged = false; // Reset the state
			return true;
		}
		return false;
	}

	// Accessors
	std::string GetPath() const { return m_targetAbsolutePath.string(); }
	std::string GetLastChangedPath() const { return m_lastChangedPath; }
	bool IsValid() const { return m_watchID >= 0; }

private:
	efsw::FileWatcher m_watcher;
	efsw::WatchID m_watchID;
	
	std::mutex m_mutex;
	bool m_hasChanged = false;
	std::string m_lastChangedPath;

	fs::path m_targetAbsolutePath;
	bool m_isWatchingSingleFile = false;
};

}//ehj
