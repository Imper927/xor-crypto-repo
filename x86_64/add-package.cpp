#include <iostream>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

std::string& get_filename(const std::string& path)
{
	for (int i = path.size() - 1; i >= 0; --i)
	{
		if (path[i] == '/')
		{
			return *new std::string(path.substr(i + 1));
		}
	}
	return *new std::string;
}

void add_file_to_repo(const char* path)
{
	int input, output;
	if ((input = open(path, O_RDONLY)) == -1)
	{
		std::cerr << "opening " << path << " failed : " << ::strerror(errno) << "\n";
		exit(-1);
	}
	std::string& filename = get_filename(path);
	
	struct stat st{ };
	if (::stat(filename.c_str(), &st) >= 0)
	{
		::remove(filename.c_str());
	}
	
	if ((output = creat(filename.c_str(), 0644)) == -1)
	{
		std::cerr << "opening " << filename << " failed : " << ::strerror(errno) << "\n";
		::close(input);
		exit(-1);
	}
	
	::fstat(input, &st);
	
	off_t offset = 0;
	
	if (::sendfile(output, input, &offset, st.st_size) == st.st_size)
	{
		std::cout << "copying file \033[32msuccessful\033[0m.\n";
	}
	else
	{
		std::cout << "copying file \033[31munsuccessful\033[0m.\n";
	}
	
	::close(input);
	::close(output);
	
	system("fish unconfigure.fish");
	system(("repo-add xor-crypto-repo.db.tar.gz \'" + filename + "\'").c_str());
	system("fish configure.fish");
	system("git add *");
	system(("git commit -m \"added " + filename + " package\"").c_str());
}

int main(int argc, char** argv)
{
	if (argc >= 2)
	{
		for (int i = 1; i < argc; ++i)
		{
			add_file_to_repo(argv[i]);
		}
		system("git push");
	}
	else
	{
		std::cout << argv[0] << " <package...>\n";
	}
}