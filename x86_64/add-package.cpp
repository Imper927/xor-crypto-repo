#include <iostream>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

void error(const std::string& message)
{
	std::cerr << "\033[31m\033[1merror\033[0m : " << message << "\n";
	throw std::runtime_error(message);
}

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
		std::cerr << "opening " << path << " as input failed : " << ::strerror(errno) << "\n";
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
		std::cerr << "opening " << filename << " as output failed : " << ::strerror(errno) << "\n";
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
	
	system(("fish -c \"fish unconfigure.fish; repo-add xor-crypto-repo.db.tar.gz \'" + filename
			+ "\'; fish configure.fish; git add *; git commit -m \'added " + filename + " package\'\"").c_str());
}

void include_file_to_repo(const char* path)
{
	system(("fish -c \"fish unconfigure.fish; repo-add xor-crypto-repo.db.tar.gz \'" + std::string(path)
			+ "\'; fish configure.fish; git add *; git commit -m \'included " + std::string(path) + " package\'\"").c_str());
}

std::string& get_s_in_fmt(const std::string& str, const std::string& format)
{
	auto* result = new std::string;
	int pos_prefix_end = -1;
	for (int i = 0; i < format.size() && i < str.size(); ++i)
	{
		if (format[i] == '%' && format[i + 1] == 's')
		{
			pos_prefix_end = i;
			break;
		}
		else if (str[i] != format[i])
		{
			break;
		}
	}
	
	if (pos_prefix_end < 0)
	{
		error("prefix not found in \'str\'");
	}
	
	int j = pos_prefix_end;
	int i = pos_prefix_end + 2, pos_s = j;
	int delta = 0, delta2 = 0;
	for (; j < str.size() && i < format.size();)
	{
		bool is_eq = true;
		for (int k = i, l = j;; ++k, ++l)
		{
			if ((format.size() - k >= 2 && format[k] == '%' && format[k + 1] == 's') || k >= format.size())
			{
				delta2 += k - i;
				j = l;
				break;
			}
			else if (str[l] != format[k])
			{
				is_eq = false;
				break;
			}
		}
		if (is_eq)
		{
			(*result) += str.substr(pos_s, delta);
			i += delta2 + 2;
			delta = 0;
			delta2 = 0;
			pos_s = j;
		}
		else
		{
			++delta;
			++j;
		}
	}
	if (format[i - 2] == '%' && format[i - 1] == 's')
	{
		(*result) += str.substr(j);
	}
	return *result;
}

void delete_package_from_repo(const char* package_name)
{
	system(("fish -c \"git add *; fish unconfigure.fish; repo-remove xor-crypto-repo.db.tar.gz \'"
			+ std::string(package_name) + "\'; fish configure.fish; rm -f \'" + std::string(package_name)
			+ "\'*.pkg.tar.zst; git commit -m \'removed " + get_filename(package_name) + " package\'\"").c_str());
}

int main(int argc, char** argv)
{
//	std::string& res = get_s_in_fmt("some text which must be splitted", "s%sme%swh%sch%ste");
//	std::cout << res;
	if (argc >= 3)
	{
		if (!::strcmp(argv[1], "add"))
		{
			for (int i = 2; i < argc; ++i)
			{
				add_file_to_repo(argv[i]);
			}
		}
		else if (!strcmp(argv[1], "del"))
		{
			for (int i = 2; i < argc; ++i)
			{
				delete_package_from_repo(argv[i]);
			}
		}
		else if (!strcmp(argv[1], "include"))
		{
			for (int i = 2; i < argc; ++i)
			{
				include_file_to_repo(argv[i]);
			}
		}
		system("git push");
		std::cout << "\033[32mgit \033[31mdiff\033[0m :\n";
		system("git diff");
	}
	else
	{
		std::cout << argv[0] << " add/del/include <package...>\n";
	}
}