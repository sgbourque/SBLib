#include <Traits/clifford_traits.h>

#if defined( STATIC_UNIT_TESTS )
#define TEST_CLIFFORD_TRAITS
#endif

#if defined( GENERATE_UNIT_TESTS )
// Nothing to generate
#elif defined( TEST_CLIFFORD_TRAITS )
static_assert(alternating_traits<1,2, true>::sign == +1, "Invalid reversion conjugacy sign");
static_assert(alternating_traits<2,1, true>::sign == -1, "Invalid reversion conjugacy sign");

static_assert(alternating_traits<1,2, false>::sign == -1, "Invalid reversion conjugacy sign");
static_assert(alternating_traits<2,1, false>::sign == +1, "Invalid reversion conjugacy sign");


static_assert(reversion_conjugacy_traits<(1|2)>::sign == -1, "Invalid reversion conjugacy sign");


static_assert(grade_conjugacy_traits<0>::sign == +1, "Invalid grade conjugacy sign");
static_assert(grade_conjugacy_traits<1>::sign == -1, "Invalid grade conjugacy sign");
static_assert(grade_conjugacy_traits<3>::sign == +1, "Invalid grade conjugacy sign");
static_assert(grade_conjugacy_traits<7>::sign == -1, "Invalid grade conjugacy sign");


static_assert(clifford_adjoint_conjugacy_traits<0>::sign == +1, "Invalid adjoint conjugacy sign");
static_assert(clifford_adjoint_conjugacy_traits<1>::sign == -1, "Invalid adjoint conjugacy sign");
static_assert(clifford_adjoint_conjugacy_traits<3>::sign == -1, "Invalid adjoint conjugacy sign");
static_assert(clifford_adjoint_conjugacy_traits<7>::sign == +1, "Invalid adjoint conjugacy sign");



//
// dimension 1
//
// rank 0 (scalar)
static_assert(hodge_conjugacy_traits<0, 1, true>::sign == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<0, 1, false>::sign == +1, "Invalid hodge conjugacy sign");
// rank 1 (pseudoscalar is a vector)
static_assert(hodge_conjugacy_traits<1, 1, true>::sign == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<1, 1, false>::sign == +1, "Invalid hodge conjugacy sign");
// duals
static_assert(hodge_conjugacy_traits<0, 1, true>::bit_set == 1,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<0, 1, false>::bit_set == 1,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<1, 1, true>::bit_set == 0,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<1, 1, false>::bit_set == 0,  "Invalid hodge conjugate");

//
// dimension 2
//
// rank 0 (scalar)
static_assert(hodge_conjugacy_traits<0, 3, true>::sign == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<0, 3, false>::sign == +1, "Invalid hodge conjugacy sign");
// rank 1 (vector = pseudovector)
// e1 ^ (*e1) = +(e1 ^ e2)	 (big endian)
static_assert(hodge_conjugacy_traits<1, 3, true>::sign  == +1, "Invalid hodge conjugacy sign");
// e1 ^ (*e1) = -(e2 ^ e1)	 (little endian)
static_assert(hodge_conjugacy_traits<1, 3, false>::sign == -1, "Invalid hodge conjugacy sign");
// e2 ^ (*e2) = -(e1 ^ e2)	 (big endian)
static_assert(hodge_conjugacy_traits<2, 3, true>::sign  == -1, "Invalid hodge conjugacy sign");
// e2 ^ (*e2) = +(e2 ^ e1)	 (little endian)
static_assert(hodge_conjugacy_traits<2, 3, false>::sign == +1, "Invalid hodge conjugacy sign");
// rank 2 (pseudoscalar)
static_assert(hodge_conjugacy_traits<3, 3, true>::sign == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<3, 3, false>::sign == +1, "Invalid hodge conjugacy sign");
// duals
static_assert(hodge_conjugacy_traits<0, 3, true>::bit_set == 3,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<0, 3, false>::bit_set == 3,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<1, 3, true>::bit_set == 2,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<1, 3, false>::bit_set == 2,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<2, 3, true>::bit_set == 1,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<2, 3, false>::bit_set == 1,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<3, 3, true>::bit_set == 0,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<3, 3, false>::bit_set == 0,  "Invalid hodge conjugate");

//
// dimension 3
//
// rank 0 (scalar)
static_assert(hodge_conjugacy_traits<0, 7, true>::sign == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<0, 7, false>::sign == +1, "Invalid hodge conjugacy sign");
// rank 1 (vector)
// e1 ^ (*e1) = +(e1 ^ (e2 ^ e3))	 (big endian)
// e1 ^ (*e1) = +(e1 ^ (e3 ^ e2))	 (little endian)
static_assert(hodge_conjugacy_traits<1, 7, true>::sign == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<1, 7, false>::sign == +1, "Invalid hodge conjugacy sign");
// e2 ^ (*e2) = -(e2 ^ (e1 ^ e3))	 (big endian)
// e2 ^ (*e2) = -(e2 ^ (e3 ^ e1))	 (little endian)
static_assert(hodge_conjugacy_traits<2, 7, true>::sign == -1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<2, 7, false>::sign == -1, "Invalid hodge conjugacy sign");
// e3 ^ (*e3) = +(e3 ^ (e1 ^ e2))	 (big endian)
// e3 ^ (*e3) = +(e3 ^ (e2 ^ e1))	 (little endian)
static_assert(hodge_conjugacy_traits<4, 7, true>::sign == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<4, 7, false>::sign == +1, "Invalid hodge conjugacy sign");
// rank 2 (pseudovector)
// (e2 ^ e3) ^ (*(e2 ^ e3)) = +<(e2 ^ e3), (e2 ^ e3)> ((e2 ^ e3) ^ e1) = -(e1 ^ e2 ^ e3)	 (big endian)
// (e3 ^ e2) ^ (*(e3 ^ e2)) = +<(e3 ^ e2), (e3 ^ e2)> ((e3 ^ e2) ^ e1) = -(e3 ^ e2 ^ e1)	 (little endian)
static_assert(hodge_conjugacy_traits<6, 7, true>::sign == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<6, 7, false>::sign == +1, "Invalid hodge conjugacy sign");
// (e1 ^ e3) ^ (*(e1 ^ e3)) = -<(e1 ^ e3), (e1 ^ e3)> ((e1 ^ e3) ^ e2) = +(e1 ^ e2 ^ e3)	 (big endian)
// (e3 ^ e1) ^ (*(e3 ^ e1)) = -<(e3 ^ e1), (e3 ^ e1)> ((e3 ^ e1) ^ e2) = +(e3 ^ e2 ^ e1)	 (little endian)
static_assert(hodge_conjugacy_traits<5, 7, true>::sign == -1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<5, 7, false>::sign == -1, "Invalid hodge conjugacy sign");
// (e1 ^ e2) ^ (*(e1 ^ e2)) = +<(e1 ^ e2), (e1 ^ e2)> ((e1 ^ e2) ^ e3) = -(e1 ^ e2 ^ e3)	 (big endian)
// (e2 ^ e1) ^ (*(e2 ^ e1)) = +<(e2 ^ e1), (e2 ^ e1)> ((e2 ^ e1) ^ e3) = -(e3 ^ e2 ^ e1)	 (little endian)
static_assert(hodge_conjugacy_traits<3, 7, true>::sign == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<3, 7, false>::sign == +1, "Invalid hodge conjugacy sign");
// rank 3 (pseudoscalar)
static_assert(hodge_conjugacy_traits<7, 7, true>::sign == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<7, 7, false>::sign == +1, "Invalid hodge conjugacy sign");
// duals
static_assert(hodge_conjugacy_traits<0, 7, true>::bit_set == 7,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<0, 7, false>::bit_set == 7,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<1, 7, true>::bit_set == 6,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<1, 7, false>::bit_set == 6,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<2, 7, true>::bit_set == 5,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<2, 7, false>::bit_set == 5,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<4, 7, true>::bit_set == 3,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<4, 7, false>::bit_set == 3,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<6, 7, true>::bit_set == 1,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<6, 7, false>::bit_set == 1,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<5, 7, true>::bit_set == 2,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<5, 7, false>::bit_set == 2,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<3, 7, true>::bit_set == 4,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<3, 7, false>::bit_set == 4,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<7, 7, true>::bit_set == 0,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<7, 7, false>::bit_set == 0,  "Invalid hodge conjugate");

//
// outer check in dimension 2 + 1 generated by {e0, e1} + {e2}
//
// (e0 ^ e2) ^ *_{e0^e1}(e0 ^ e2) = (e0 ^ e2) ^ (-e_1) = (e0 ^ e1 ^ e2) = <(e0 ^ e2), (e0 ^ e2)> (e0 ^ e1 ^ e2)
// (e2 ^ e0) ^ *_{e1^e0}(e2 ^ e0) = (e2 ^ e0) ^ (-e_1) = (e2 ^ e1 ^ e0) = <(e2 ^ e0), (e2 ^ e0)> (e2 ^ e1 ^ e0)
//
static_assert(hodge_conjugacy_traits<(1|4), 3, true>::sign    == -1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<(1|4), 3, false>::sign    == -1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<(1|4), 3, true>::bit_set ==  2, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<(1|4), 3, false>::bit_set ==  2, "Invalid hodge conjugate");
#endif