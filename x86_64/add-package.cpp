#include <iostream>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <fcntl.h>

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

int main(int argc, char** argv)
{
	if (argc >= 2)
	{
		int input, output;
		if ((input = open(argv[1], O_RDONLY)) == -1)
		{
			return -1;
		}
		std::string& filename = get_filename(argv[1]);
		
		struct stat st{ };
		if (::stat(filename.c_str(), &st) >= 0)
		{
			::remove(filename.c_str());
		}
		
		if ((output = creat(filename.c_str(), 0644)) == -1)
		{
			::close(input);
			return -1;
		}
		
		::fstat(input, &st);
		
		off_t copied;
		::sendfile(output, input, &copied, st.st_size);
		
		if (copied == st.st_size)
		{
			std::cout << "copying file \033[32msuccessful\033[0m.\n";
		}
		else
		{
			std::cout << "copying file \033[31munsuccessful\033[0m.\n";
		}
		
		system("unconfigure.fish");
		system(("repo-add xor-crypto-repo.db.tar.gz \'" + filename + "\'").c_str());
		system("configure.fish");
		system("git add *");
		system(("git commit -m \"added " + filename + " package\"").c_str());
		system("git push");
		
		::close(input);
		::close(output);
	}
	else
	{
		std::cout << argv[0] << " <path_to_package>\n";
	}
}