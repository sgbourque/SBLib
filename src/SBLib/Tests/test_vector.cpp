#define USE_MULTIVECTOR 1
#include <test_common.h>

#include <fstream>

namespace SBLib::Test
{
class test_vector : public RegisteredFunctor
{
	enum
	{
		e0  = (1 << 0),  e1 = (1 << 1),
		e2  = (1 << 2),  e3 = (1 << 3),
		e4  = (1 << 4),  e5 = (1 << 5),
		e6  = (1 << 6),  e7 = (1 << 7),
		e8  = (1 << 8),  e9 = (1 << 9),
		e10 = (1 << 10), e11 = (1 << 11),
		e12 = (1 << 12), e13 = (1 << 13),
		e14 = (1 << 14), e15 = (1 << 15),
	};
	using vector_type1 = vector_t<float, e0 | e2 | e7 | e15>;
	using vector_type2 = vector_t<float, e0 | e2 | e7 | e13 | e15>;
	using vector_type3 = vector_t<long double, e0 | e1 | e2 | e7 | e13 | e15>;
	static_assert(sizeof(vector_type1) == vector_type1::dimension_size * sizeof(vector_type1::scalar_type), "vector size is incorrect...");
	//static_assert(sizeof(vector_type2) == vector_type2::dimension_size * sizeof(vector_type2::scalar_type), "vector size is incorrect...");
	static_assert(sizeof(vector_type3) == vector_type3::dimension_size * sizeof(vector_type3::scalar_type), "vector size is incorrect...");

	test_vector() : RegisteredFunctor("test_vector", fct) {}
	static void fct()
	{
		vector_type1 test1{ -1.0f,-1.0f,-1.0f,-1.0f }; // sets all 4 components
		vector_type1 test2;
		vector_type2 test3{ -1.0f,-1.0f,0.0f,0.0f, 1.0f }; // only sets components for e0 and e2, all other being 0.
		vector_type2 test4;
		vector_type3 test5{ -1.0f,-1.0f }; // only sets components for e0 and e1, all other being 0.
		vector_type3 test6 = test1;
		vector_type1 test7 = test5.project<vector_type1>(); // only e0 component is set to a non-zero value as e1 is not is vector_type1
		vector_type1::scalar_type coeff1, coeff2;

		const std::string input_filename = "../../tmp/test_vector.in";
		const bool onlytest = (std::cin.gcount() == 0);
		if (onlytest)
		{
			std::cout << "Press 'd' to delete file, press anything else to continue..." << std::endl;
		}
		int data = std::cin.get();
		if (data == 'd')
			std::remove(input_filename.c_str());
		else if (!onlytest)
			std::cin.unget();

		{
			auto file = std::fstream(input_filename, std::ios_base::in | std::ios_base::_Nocreate);

			static const std::string version_string = "version";
			static const size_t version = 0;
			if (file.is_open())
			{
				std::string file_version_string;
				size_t file_version;
				file >> file_version_string >> file_version;
				if (file_version_string != version_string || file_version != version)
					file.close();
			}
			else
			{
				std::cout << "Please enter 10 numbers : ";
			}

			std::istream& in = file.is_open() ? file : std::cin;

			in >> coeff1 >> coeff2;
			in >> test1.get<e0>();
			in >> test1.get<e2>();
			in >> test1.get<e7>();
			in >> test1.get<e15>();

			in >> test2.get<e0>();
			in >> test2.get<e2>();
			in >> test2.get<e7>();
			in >> test2.get<e15>();

			if (!file.is_open())
			{
				file.open(input_filename, std::ios_base::out);
				if (file.is_open())
				{
					file << version_string << " " << version << std::endl;
					file << coeff1 << " " << coeff2;
					file << " " << test1.cget<e0>();
					file << " " << test1.cget<e2>();
					file << " " << test1.cget<e7>();
					file << " " << test1.cget<e15>();
					file << " " << test2.cget<e0>();
					file << " " << test2.cget<e2>();
					file << " " << test2.cget<e7>();
					file << " " << test2.cget<e15>();
				}
			}
			test3.get<e0>() = test1.cget<e0>();
			//test3.get<e1>()  = test1.cget<e1>(); // this would be fine and would not have any effect! not doing it just to check explicitly data set in constructor
			test3.get<e2>() = test1.cget<e2>();
			test3.get<e7>() = test1.cget<e7>();
			//test3.get<e13>() = test1.get<e13>(); // this would be fine and would not have any effect! not doing it just to check for unitialized data...
			test3.get<e15>() = test1.cget<e15>();
			test4.get<e0>() = test2.cget<e0>();
			//test4.get<e1>() = test2.cget<e1>(); // this would be fine and would not have any effect! not doing it just to check for unitialized data...
			test4.get<e2>() = test2.cget<e2>();
			test4.get<e7>() = test2.cget<e7>();
			test4.get<e13>() = test2.cget<e13>(); // this is fine even if test1 does not have any e13 component!
			test4.get<e15>() = test2.cget<e15>();

			test5.get<e0>() = test1.cget<e0>();
			//test5.get<e1>()  = test1.cget<e1>(); // this would be fine and would not have any effect! not doing it just to check explicitly data set in constructor
			test5.get<e2>() = test1.cget<e2>();
			test5.get<e7>() = test1.cget<e7>();
			//test3.get<e13>() = test1.get<e13>(); // this would be fine and would not have any effect! not doing it just to check for unitialized data...
			test5.get<e15>() = test1.cget<e15>();
			test6.get<e0>() = test2.cget<e0>();
			//test4.get<e1>() = test2.cget<e1>(); // this would be fine and would not have any effect! not doing it just to check for unitialized data...
			test6.get<e2>() = test2.cget<e2>();
			test6.get<e7>() = test2.cget<e7>();
			test6.get<e13>() = test2.cget<e13>(); // this is fine even if test1 does not have any e1 component!
			test6.get<e15>() = test2.cget<e15>();
		}

		vector_type3::scalar_type coeff1d = coeff1, coeff2d = coeff2;
		auto test_result1 = coeff1 * test1 * coeff2 + test2;
		// checking both const and non-const accessors
		std::cout << "("
			<< test_result1.cget<e0>() << ", "
			<< test_result1.get<e2>() << ", "
			<< test_result1.get<e7>() << ", "
			<< test_result1.get<e15>()
			<< ")" << std::endl;

		std::cout << "0 = "
			<< test_result1.cget<0>() << " = "
			<< test_result1.get<e1 | e2>() << " = "
			<< test_result1.get<e3>()
			<< std::endl;

		std::cout << "test_result1: " << test_result1 << std::endl;
		auto test_result2 = coeff1 * test3 * coeff2 + test4;
		std::cout << "test_result2: " << test_result2 << std::endl;
		auto test_result3 = coeff1d * test5 * coeff2d + test6;
		std::cout << "test_result3: " << test_result3 << std::endl;
		//auto test_result4 = coeff1 * test1 * coeff2 + test4; // this will fail compilation (vector types are incompatible) ... eventually this should be fixed as it all fits into destination
		auto test_result4 = test_result1 + test_result1;
		std::cout << "test_result4: " << test_result4 << std::endl;
		test_result4 = test_result1;
		test_result4 += test_result4;
		test_result4 *= coeff1;
		test_result4 /= coeff2;
		std::cout << "test_result1 (copied on 4): " << test_result1 << std::endl;
		std::cout << "test_result4 (modified 1): " << test_result4 << std::endl;
		std::cout << test7 << std::endl;

		std::cout << "... run test '" << instance.get_id() << "d' to delete input file..." << std::endl;
	}

	static test_vector instance;
};
test_vector test_vector::instance;
} // namespace SBLib::Test
