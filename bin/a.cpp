#include <windows.h>
#include <filesystem>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>

int main() {
	std::vector<std::pair<std::string,std::filesystem::file_time_type>> a;

	std::vector<std::string> names;
	names.push_back("einherjarEngine.exe");
	names.push_back("einherjarEngineDbg.exe");
	names.push_back("einherjarEngineMY.exe");
	names.push_back("einherjarEngineMYDbg.exe");

	for (auto name : names) {
		if (std::filesystem::exists(name))
			a.push_back({name,std::filesystem::last_write_time(name.c_str())});
	}

	std::sort(a.begin(), a.end(), 
		[](const auto& a, const	auto& b) {
			return a.second	> b.second;	// Descending order	(most recent first)
		});

	system(a[0].first.c_str());

	//// check timestamp of debug and release and use last build
	//auto ftime = std::filesystem::last_write_time("einherjarEngine.exe");
	//auto ftimeDbg = std::filesystem::last_write_time("einherjarEngineDbg.exe");

	//if (ftime > ftimeDbg)
	//	system("einherjarEngine.exe");
	//else
	//	system("einherjarEngineDbg.exe");
	
	return 0;
}
