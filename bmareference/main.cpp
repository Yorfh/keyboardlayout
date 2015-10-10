#include "BMAReference.hpp"

int main(int argc, char* argv[])
{
	BMAReference r;
	int target = std::atoi(argv[1]);
	string input_file_name = argv[2];
	return r.run(target, input_file_name, "output.txt");
}
