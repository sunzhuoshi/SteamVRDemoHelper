#pragma once
#include <string>
#include <sstream>
#include <vector>

namespace l4util {
	typedef std::pair<std::string, std::string> StringPair;

	inline std::string trim(const std::string &str)
	{
		std::string result;
		std::stringstream trimmer;

		trimmer << str;
		trimmer.clear();
		trimmer >> result;
		return result;
	}

	inline std::vector<std::string>& split(const std::string &text, char sep, std::vector<std::string> &tokens) {
		std::size_t start = 0, end = 0;
		while ((end = text.find(sep, start)) != std::string::npos) {
			tokens.push_back(text.substr(start, end - start));
			start = end + 1;
		}
		tokens.push_back(text.substr(start));
		return tokens;
	}

	inline bool parseProperty(const std::string &line, StringPair &keyValue)
	{
		bool result = false;
		std::string trimmedLine = trim(line);
		std::vector<std::string> splited;
		split(line, '=', splited);
		if (2 == splited.size()) {
			keyValue.first = splited[0];
			keyValue.second = splited[1];
			result = true;
		}
		return result;
	}

	std::string getCurrentExePath();
	std::string getFileFullPath(const std::string &relativePath);
	std::string getCurrentProcessName();
	std::string loadString(UINT id);
}