#include <test_common.h>

RegisteredFunctor::data_t RegisteredFunctor::common_data;
int main()
{
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
			std::cout << "bye";
		else if (help)
			RegisteredFunctor::help(std::cout);
		else if (!RegisteredFunctor::eval(testCase))
			std::cout << "... test not found, please try again!" << std::endl;

		std::cout << std::endl;
	}
	while (std::cin.good() && !quit);

	if (RegisteredFunctor::size() < 2)
		std::cin.get();
}
