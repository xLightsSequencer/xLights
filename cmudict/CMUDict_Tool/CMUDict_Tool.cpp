#include <list>
#include <string>
#include <fstream>
#include <algorithm>
#include <map>

void Replace(std::string& str,
    const std::string& oldStr,
    const std::string& newStr)
{
    std::string::size_type pos = 0u;
    while ((pos = str.find(oldStr, pos)) != std::string::npos) {
        str.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
}

std::string PrepareLine(const std::string& line, const std::map<std::string, std::string>& map)
{
    std::string res = line;

    // remove alternate pronounciations as xLights does not use them
    size_t firstparen = res.find('(');
    if (firstparen != std::string::npos && firstparen != 0) {
        res = "";
    }

    // convert to upper case
    std::transform(res.begin(), res.end(), res.begin(),
        [](unsigned char c) { return std::toupper(c); });

    // add an extra space after the first word
    size_t firstspace = res.find(' ');
    if (firstspace != std::string::npos) {
        if (res.size() > firstspace + 1 && res[firstspace+1] != ' ') res = res.substr(0, firstspace) + " " + res.substr(firstspace);
    }

    // if a line contains a # then the #and everything after it is removed except if it is the first character
    size_t hash = res.find('#');
    if (hash != std::string::npos && hash != 0) {
        res = res.substr(0, hash);
    }

    // now go through the phonemes replacing any in the map
    for (const auto& it : map) {
        while (res.find(it.first) != std::string::npos) {
            res = res.replace(res.find(it.first), it.first.size(), it.second);
        }
    }

    // leading and trailing spaces on the line are removed
    res.erase(res.begin(), std::find_if(res.begin(), res.end(), [](unsigned char ch) {
        return !std::isspace(ch);
        }));
    res.erase(std::find_if(res.rbegin(), res.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
        }).base(), res.end());

    // Add end of line
    if (res != "") res += "\n";

    return res;
}

int main()
{
    bool ok = true;

    std::ifstream fReadme("README", std::ifstream::in);
    std::ifstream fVP("cmudict.vp", std::ifstream::in);
    std::ifstream fDict("cmudict.dict", std::ifstream::in);
    std::ifstream fMap("map", std::ifstream::in);

    if (!fReadme.is_open()) {
        printf("README file not found.\n");
        ok = false;
    }

    if (!fVP.is_open()) {
        printf("cmudict.vp file not found.\n");
        ok = false;
    }

    if (!fDict.is_open()) {
        printf("cmudict.dict file not found.\n");
        ok = false;
    }

    if (!fMap.is_open()) {
        printf("map file not found.\n");
        ok = false;
    }

    std::string line;
    std::map<std::string, std::string> map;
    while (std::getline(fMap, line)) {
        if (line.find("==>") != std::string::npos) {
            std::string from = " " + line.substr(0, line.find("==>")) + " ";
            std::string to = " " + line.substr(line.find("==>") + 3) + " ";
            if (to == "  ")
                to = " ";
            map[from] = to;
        }
    }
    fMap.close();

    std::ofstream fSD("standard_dictionary", std::ofstream::out);

    if (!fSD.is_open()) {
        printf("standard_dictionary file could not be created.\n");
        ok = false;
    }

    if (!ok) return 1;

    printf("Input files are there and output file can be created.\n");

    while (std::getline(fReadme, line)) {
        line = ";;; # " + line + "\n";
        fSD.write(line.c_str(), line.size());
    }
    fReadme.close();

    printf("Header added.\n");

    std::list<std::string> lines;

    while (std::getline(fVP, line)) {
        line = PrepareLine(line, map);
        if (line != "") lines.push_back(line);
    }
    fVP.close();

    printf("Punctuation file read.\n");

    while (std::getline(fDict, line)) {
        line = PrepareLine(line, map);
        if (line != "") lines.push_back(line);
    }
    fVP.close();

    printf("Dictionary read.\n");
    printf("Sorting ...\n");

    lines.sort();

    printf("Dictionary sorted.\n");

    for (const auto& it : lines) {
        fSD.write(it.c_str(), it.size());
    }
    fSD.close();

    printf("standard_dictionary written.\n");

    return 0;
}
