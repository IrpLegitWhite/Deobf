#pragma once
#include <string>
#include <vector>

namespace deobf {
	namespace core {
		namespace utils {

			std::string trim(const std::string& s);
			std::vector<std::string> splitLines(const std::string& s);
			std::string join(const std::vector<std::string>& parts, const std::string& sep);
			bool isPrintableUtf8(const std::string& s);
			std::string escapeCpp(const std::string& s);
			std::string toLower(std::string s);
			std::string toUpper(std::string s);
			bool startsWith(const std::string& s, const std::string& prefix);
			bool endsWith(const std::string& s, const std::string& suffix);
			std::string replaceAll(std::string s, const std::string& from, const std::string& to);

		} // namespace utils
	} // namespace core
} // namespace deobf