////////////////////////////////////////////////////////////////////////////////
// taskd - Task Server
//
// Copyright 2010 - 2012, Göteborg Bit Factory.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <Directory.h>
#include <cmake.h>

////////////////////////////////////////////////////////////////////////////////
Directory::Directory ()
{
}

////////////////////////////////////////////////////////////////////////////////
Directory::Directory (const Directory& other)
: File::File (other)
{
}

////////////////////////////////////////////////////////////////////////////////
Directory::Directory (const File& other)
: File::File (other)
{
}

////////////////////////////////////////////////////////////////////////////////
Directory::Directory (const Path& other)
: File::File (other)
{
}

////////////////////////////////////////////////////////////////////////////////
Directory::Directory (const std::string& in)
: File::File (in)
{
}

////////////////////////////////////////////////////////////////////////////////
Directory::~Directory ()
{
}

////////////////////////////////////////////////////////////////////////////////
Directory& Directory::operator= (const Directory& other)
{
  if (this != &other)
  {
    File::operator= (other);
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Directory::create ()
{
  return mkdir (_data.c_str (), 0755) == 0 ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
bool Directory::remove () const
{
  return remove_directory (_data);
}

////////////////////////////////////////////////////////////////////////////////
bool Directory::remove_directory (const std::string& dir) const
{
  DIR* dp = opendir (dir.c_str ());
  if (dp != NULL)
  {
    struct dirent* de;
    while ((de = readdir (dp)) != NULL)
    {
      if (!strcmp (de->d_name, ".") ||
          !strcmp (de->d_name, ".."))
        continue;

#if defined (SOLARIS) || defined (HAIKU)
      struct stat s;
      stat ((dir + "/" + de->d_name).c_str (), &s);
      if (s.st_mode & S_IFDIR)
        remove_directory (dir + "/" + de->d_name);
      else
        unlink ((dir + "/" + de->d_name).c_str ());
#else
      if (de->d_type == DT_DIR)
        remove_directory (dir + "/" + de->d_name);
      else
        unlink ((dir + "/" + de->d_name).c_str ());
#endif
    }

    closedir (dp);
  }

  return rmdir (dir.c_str ()) ? false : true;
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> Directory::list ()
{
  std::vector <std::string> files;
  list (_data, files, false);
  return files;
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> Directory::listRecursive ()
{
  std::vector <std::string> files;
  list (_data, files, true);
  return files;
}

////////////////////////////////////////////////////////////////////////////////
std::string Directory::cwd ()
{
  char buf[PATH_MAX];
  getcwd (buf, PATH_MAX - 1);
  return std::string (buf);
}

////////////////////////////////////////////////////////////////////////////////
bool Directory::up ()
{
  if (_data == "/")
    return false;

  std::string::size_type slash = _data.rfind ('/');
  if (slash == 0)
  {
    _data = "/";  // Root dir should retain the slash.
    return true;
  }
  else if (slash != std::string::npos)
  {
    _data = _data.substr (0, slash);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Directory::cd () const
{
  return chdir (_data.c_str ()) == 0 ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
void Directory::list (
  const std::string& base,
  std::vector <std::string>& results,
  bool recursive)
{
  DIR* dp = opendir (base.c_str ());
  if (dp != NULL)
  {
    struct dirent* de;
    while ((de = readdir (dp)) != NULL)
    {
      if (!strcmp (de->d_name, ".") ||
          !strcmp (de->d_name, ".."))
        continue;

#if defined (SOLARIS) || defined (HAIKU)
      struct stat s;
      stat ((base + "/" + de->d_name).c_str (), &s);
      if (recursive && s.st_mode & S_IFDIR)
        list (base + "/" + de->d_name, results, recursive);
      else
        results.push_back (base + "/" + de->d_name);
#else
      if (recursive && de->d_type == DT_DIR)
        list (base + "/" + de->d_name, results, recursive);
      else
        results.push_back (base + "/" + de->d_name);
#endif
    }

    closedir (dp);
  }
}

////////////////////////////////////////////////////////////////////////////////