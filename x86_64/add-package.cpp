#include <iostream>
#include <sys/stat.h>
#include <sys/sendfile.h>

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
		FILE* fin = ::fopen(argv[1], "rb");
		std::string& filename = get_filename(argv[1]);
		FILE* fout = ::fopen(filename.c_str(), "wb");
		
		struct stat st{ };
		::fstat(fin->_fileno, &st);
		
		off_t copied;
		::sendfile(fout->_fileno, fin->_fileno, &copied, st.st_size);
		
		if (copied == st.st_size)
		{
			std::cout << "copying file \033[32msuccessful\033[0m.\n";
		}
		else
		{
			std::cout << "copying file \033[31munsuccessful\033[0m.\n";
		}
		
		system("fish -c unconfigure.fish");
		system(("fish -c repo-add xor-crypto-repo.db.tar.gz " + filename).c_str());
		system("fish -c configure.fish");
		system("git add *");
		system(("git commit -m \"added " + filename + " package\"").c_str());
		system("git push");
		
		::fclose(fin);
		::fclose(fout);
	}
	else
	{
		std::cout << argv[0] << " <path_to_package>\n";
	}
}