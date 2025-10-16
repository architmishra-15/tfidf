#include <filesystem>
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <string>
#include <unordered_map>
#include <vector>

std::vector<std::string> readDir(const std::string &filename)
{
  namespace fs = std::filesystem;

  std::vector<std::string> files;
  files.reserve(100);

  if (!fs::exists(filename) || !fs::is_directory(filename))
  {
    std::cout << "Please provide a directory name\n";
    return files;
  }

  for (const auto &i : fs::directory_iterator(filename))
  {
    if (i.is_directory())
    {
      auto subfiles = readDir(i.path().string());
      files.insert(files.end(), std::make_move_iterator(subfiles.begin()),
                   std::make_move_iterator(subfiles.end()));
    } else {
      files.push_back(i.path().string());
    }
  }
  return files;
}


std::unordered_map<std::string, int> readFile(const std::string &filename)
{
  std::ifstream file(filename);

  std::unordered_map<std::string, int> count;
  count.reserve(1000);

  if (!file.is_open())
  {
    std::cout << "Could not read file: " << filename << "\n";
    std::exit(1);
  }

  std::string content;
  file.seekg(0, std::ios::end);
  content.reserve(file.tellg());
  file.seekg(0, std::ios::beg);

  content.assign((std::istreambuf_iterator<char>(file)),
                 std::istreambuf_iterator<char>());

  // Parse words from content
  const char *ptr = content.data();
  const char *end = ptr + content.size();

  std::string word;
  word.reserve(32); // Reserve space for typical word length

  while (ptr < end)
  {
    // Skip non-alphabetic characters
    while (ptr < end && !std::isalnum(static_cast<unsigned char>(*ptr)))
      ++ptr;

    if (ptr >= end)
      break;

    // Build word with lowercase conversion
    word.clear();
    while (ptr < end && std::isalnum(static_cast<unsigned char>(*ptr))) {
      word += std::tolower(static_cast<unsigned char>(*ptr));
      ++ptr;
    }

    if (!word.empty())
      ++count[word];
  }

  return count;
}

int main(void)
{
  std::ofstream file("frequency.json");

  Json::Value root;
  std::unordered_map<std::string, std::string> freq;
  freq.reserve(10000);

  std::vector<std::string> files = readDir("docs.gl/");

  for (const auto &i : files)
  {
    std::unordered_map<std::string, int> wordCount = readFile(i);
    
    // Create a JSON object for this file
    Json::Value fileJson;
    for (const auto& [word, count] : wordCount)
    {
      fileJson[word] = count;
    }
    
    // Add it to the root object with the filename as key
    root[i] = fileJson;
  }

  Json::StreamWriterBuilder builder;
  builder["indentation"] = "  "; // Pretty print, or use "" for compact
  std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  writer->write(root, &file);
  file << "\n";

  std::cout << "Done\n";
  return 0;
}

