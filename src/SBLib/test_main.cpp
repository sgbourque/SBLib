#include <test_common.h>
#include <intrin.h>

int main()
{
	using namespace SBLib::Test;

	bool quit = false, help = false;
	size_t testCase = ~0uLL;
	do
	{
		if (RegisteredFunctor::size() > 1)
		{
			quit = help = false;
			testCase = ~0uLL;
			std::cout << "Enter test case [0 - " << RegisteredFunctor::size() - 1 << "] or 'q' to quit, 'h' for help : ";
			std::cin >> testCase;
			while (!std::cin.good())
			{
				std::cin.clear();
				int data = std::cin.get();
				quit = (data == 'q');
				help = (data == 'h');
			}
		}
		else
		{
			testCase = ~testCase;
			quit = (testCase != 0);
		}

		if (quit)
		{
			std::cout << "bye";
		}
		else if (!help)
		{
			std::string testName = RegisteredFunctor::get_name(testCase);
			std::cout << std::setw(testName.length()) << std::setfill('-') << "" << std::endl << std::setw(0) << std::setfill(' ');
			std::cout << testName << std::endl;
			std::cout << std::setw(testName.length()) << std::setfill('-') << "" << std::endl << std::setw(0) << std::setfill(' ') << std::endl;

			if (!RegisteredFunctor::eval(testCase))
			{
				std::cout << "... test not found, please try again!" << std::endl;
				help = true;
			}
		}

		if (help)
			RegisteredFunctor::help(std::cout);

		std::cout << std::endl;
	}
	while (std::cin.good() && !quit);

	if (RegisteredFunctor::size() < 2)
		std::cin.get();
}
