/// =========================================================================
/// Written by pfederl@ucalgary.ca in 2023, for CPSC457.
/// =========================================================================
/// You need to edit this file.
///
/// You can delete all contents of this file and start from scratch if
/// you wish, as long as you implement the analyzeDir() function as
/// defined in "analyzeDir.h".

#include "analyzeDir.h"
#include <iostream>
#include <dirent.h>
#include <unordered_map>
#include <stdio.h>
#include <algorithm>
#include <errno.h>
#include <cassert>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <string.h>

/// check if path refers to a directory
static bool is_dir(const std::string & path)
{
  struct stat buff;
  if (0 != stat(path.c_str(), &buff)) return false;
  return S_ISDIR(buff.st_mode);
}

/// check if path refers to a file
static bool is_file(const std::string & path)
{
  struct stat buff;
  if (0 != stat(path.c_str(), &buff)) return false;
  return S_ISREG(buff.st_mode);
}

/// check if string ends with another string
static bool ends_with(const std::string & str, const std::string & suffix)
{
  if (str.size() < suffix.size()) return false;
  else
    return 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

bool compareSize(ImageInfo i1, ImageInfo i2){
  return(i1.height*i1.width < i2.height*i2.width);
}

constexpr int MAX_WORD_SIZE = 1024; // This shouldn't be necessary given the instructions of the assignment, but doesn't hurt to have.

//
// returns the next word from stdin
//   - word is a sequence of characters [a-zA-Z]
//   - the word is converted into lower case
//   - words are separated by any non alphabetic characters
//   - return empty string on EOF
//  example:
//   "   Hello..  World:) !" --> ["hello", "world", ""]
//
// Next word was taken from https://gitlab.com/cpsc457/public/word-histogram by pfederl
//
std::string
next_word(FILE* file)
{
  std::string result;
  while(1) {
    char c = fgetc(file);
    if(c == EOF) break;
    c = tolower(c);
    if(! isalpha(c)) {
      if(result.size() == 0)
        continue;
      else
        break;
    }
    else {
      if(result.size() >= MAX_WORD_SIZE) {
        printf("input exceeded %d word size, aborting...\n", MAX_WORD_SIZE);
        exit(-1);
      }
      result.push_back(c);
    }
  }
  return result;
}


Results inDirectory(std::string dir_name)
{ // Traverse the directories inside directories (note, need pathname)
  
  Results res;
  res.largest_file_path = "";
  res.largest_file_size = 0;
  res.n_files = 0;
  res.n_dirs = 0;
  res.all_files_size = 0;

  
  auto dir = opendir(dir_name.c_str());
  assert(dir != nullptr);
  for (auto de = readdir(dir); de != nullptr; de = readdir(dir)) {
    std::string name = de->d_name;
    // skip . and .. entries
    if (name == "." or name == "..") continue;
    name = dir_name + "/" + name;

    // check if this is directory
    if (is_dir(name)) 
    {
      res.n_dirs++; //Since it is a directory, we add it to total number of directories
      Results inner = inDirectory(name); //If it is, we want to 
      // sum the results of our sub directories with the larger one once it returns
    
      // Give largest between the two results
      if (inner.largest_file_size > res.largest_file_size)
      {
        res.largest_file_size = inner.largest_file_size;
        res.largest_file_path = inner.largest_file_path;
      }
      res.n_dirs = res.n_dirs + inner.n_dirs;
      res.n_files = res.n_files + inner.n_files;
      res.all_files_size = res.all_files_size + inner.all_files_size;
    
      res.vacant_dirs.insert(res.vacant_dirs.end(), inner.vacant_dirs.begin(),inner.vacant_dirs.end()); //Combine vacant_dirs
      res.most_common_words.insert(res.most_common_words.end(), inner.most_common_words.begin(),inner.most_common_words.end()); //Combine common words
      res.largest_images.insert(res.largest_images.end(), inner.largest_images.begin(),inner.largest_images.end()); //Combine largest images
      continue;
    }
    // make sure this is a file
    if( not is_file(name)) {continue;}


    res.n_files++; //Since it is a file and not a directory, we add it to total number of files

    // Add file to total file size and if largest file make path the largest file path
      struct stat buffchar;
      if (0 != stat(name.c_str(), &buffchar)) {}
      else{
        res.all_files_size = res.all_files_size + buffchar.st_size;
        if (long(buffchar.st_size) > res.largest_file_size){
          res.largest_file_size = long(buffchar.st_size);
          res.largest_file_path = name;
        }
      }

  //check if filename ends with .txt and if so file the words into a bank
  // This section was based on https://gitlab.com/cpsc457/public/word-histogram by pfederl
    if (ends_with(name, ".txt")) {
    std::unordered_map<std::string,int> hist;
    FILE *file;
    file = fopen(name.c_str(),"r");
    while(1) {
    auto w = next_word(file);
    if(w.size() == 0) break;
    if(w.size() >= 5) hist[w] ++;
    }
    fclose(file);
  // Approach 1
  // ===========
  // we put the contents of hist[] into std::vector and then sort it
  // Approach 1 was taken from https://gitlab.com/cpsc457/public/word-histogram by pfederl
  // first we place the words and counts into array (with count
  // negative to reverse the sort)
  for(auto & h : hist) {res.most_common_words.emplace_back(h.first, h.second);}
  }

    // let's see if this is an image and whether we can manage to get image info
    // by calling an external utility 'identify'
    std::string cmd = "identify -format '%wx%h' " + name + " 2> /dev/null";
    std::string img_size;
    auto fp = popen(cmd.c_str(), "r");
    assert(fp);
    char buff[PATH_MAX];
    if( fgets(buff, PATH_MAX, fp) != NULL) {
      img_size = buff;
    }
    int status = pclose(fp);
    if( status != 0 or img_size[0] == '0')
      img_size = "";
    if( !img_size.empty())
    {
        // Imageinfo was by pfederl
        ImageInfo images = ImageInfo();
        std::string imagepath = name;
        images.path = imagepath.substr(2,imagepath.length());
        std::string delimiter = "x";
        size_t pos = 0;
        std::string img = img_size.c_str();
        while((pos = img.find(delimiter)) != std::string::npos){
            images.width = stol(img.substr(0, pos),nullptr,10);
            img.erase(0, pos + 1);
        }
        images.height = stol(img, nullptr, 10);
        res.largest_images.push_back(images);
    }
  }

  // If no files exist from our completes search through the directory, 
  // that tells us that it is a vacant directory
  // Because we are running this recursively, each directory handles saying whether it's vacant or not
  // clear it then make our current directory inside it
  if(res.n_files == 0){
    res.vacant_dirs.clear();
    std::string vacantpath = dir_name.c_str();
    if(strcmp(dir_name.c_str(), ".") != 0){
        res.vacant_dirs.push_back(vacantpath.substr(2,vacantpath.length())); 
    }
  }
  return res;
}


// analyzeDir(n) computes stats about current directory
//   n = how many words and images to report in results
Results analyzeDir(int n)
{ // Traverse the directories inside directories (note, need pathname)
  Results res = inDirectory(".");
  if(res.n_files == 0)
  {    
    res.vacant_dirs.clear();
    res.vacant_dirs.push_back("."); 
    return res;
  }
  res.largest_file_path = res.largest_file_path.substr(2, res.largest_file_path.length());
  // Note that we have all the images and words, we now need to get the n largest and n most common
  
  // Partial sort was taken from https://gitlab.com/cpsc457/public/word-histogram by pfederl
  // if we have more than N entries, we'll sort partially, since
  // we only need the first N to be sorted
  if(res.most_common_words.size() > size_t(n)) {
    std::partial_sort(res.most_common_words.begin(), res.most_common_words.begin() + n, res.most_common_words.end());
    // drop all entries after the first n
    res.most_common_words.resize(n);
  }
  else {
    std::sort(res.most_common_words.begin(), res.most_common_words.end());
  }

  // Partial sort was taken from https://gitlab.com/cpsc457/public/word-histogram by pfederl
  // if we have more than N entries, we'll sort partially, since
  // we only need the first N to be sorted
  if(res.largest_images.size() > size_t(n)) {
    std::partial_sort(res.largest_images.begin(), res.largest_images.begin() + n, res.largest_images.end(), compareSize);
    // drop all entries after the first n
    res.largest_images.resize(n);
  } else {
    std::sort(res.largest_images.begin(), res.largest_images.end(), compareSize);
  }

  return res;
}


