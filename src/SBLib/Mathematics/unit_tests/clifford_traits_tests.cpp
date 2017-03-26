#include <Traits/clifford_traits.h>
using namespace SBLib;

#if defined( STATIC_UNIT_TESTS )
#define TEST_CLIFFORD_TRAITS
#endif

#if defined( GENERATE_UNIT_TESTS )
// Nothing to generate
#elif defined( TEST_CLIFFORD_TRAITS )
enum
{
	e     = 0,
	e0    = (1 << 0),  	e1   = (1 << 1),  	e2   = (1 << 2),  	e3   = (1 << 3),
	e01   = (e0^e1),   	e02  = (e0^e2),   	e12  = (e1^e2),  	e03  = (e0^e3),  	e13 = (e1^e3),  	e23 = (e2^e3),
	e10   = (e1^e0),   	e20  = (e2^e0),   	e21  = (e2^e1),  	e30  = (e3^e0),  	e31 = (e3^e1),  	e32 = (e3^e2),
	e012  = (e0^e1^e2),	e013 = (e0^e1^e3),	e023 = (e0^e2^e3),	e123 = (e1^e2^e3),
	e210  = (e2^e1^e0),	e310 = (e3^e1^e0),	e320 = (e3^e2^e0),	e321 = (e3^e2^e1),
	e0123 = (e0^e1^e2^e3),
	e3210 = (e3^e2^e1^e0),
};

// wedge product alternating property check
static_assert(alternating_traits<e0, e0, true>::sign     == 0,   "Invalid wedge product sign");
static_assert(alternating_traits<e0, e0, true>::bit_set  == e,   "Invalid wedge product");
static_assert(alternating_traits<e0, e1, true>::sign     == +1,  "Invalid wedge product sign");
static_assert(alternating_traits<e0, e1, true>::bit_set  == e01, "Invalid wedge product");
static_assert(alternating_traits<e1, e0, true>::sign     == -1,  "Invalid wedge product sign");
static_assert(alternating_traits<e1, e0, true>::bit_set  == e10, "Invalid wedge product");
static_assert(alternating_traits<e0, e0, false>::sign    == 0,   "Invalid wedge product sign");
static_assert(alternating_traits<e0, e0, false>::bit_set == e,   "Invalid wedge product");
static_assert(alternating_traits<e0, e1, false>::sign    == -1,  "Invalid wedge product sign");
static_assert(alternating_traits<e0, e1, false>::bit_set == e10, "Invalid wedge product");
static_assert(alternating_traits<e1, e0, false>::sign    == +1,  "Invalid wedge product sign");
static_assert(alternating_traits<e1, e0, false>::bit_set == e10, "Invalid wedge product");

// reversion parity check (ordering independant)
static_assert(reversion_conjugacy_traits<e   >::sign == +1, "Invalid reversion conjugacy sign");
static_assert(reversion_conjugacy_traits<e0  >::sign == +1, "Invalid reversion conjugacy sign");
static_assert(reversion_conjugacy_traits<e01 >::sign == -1, "Invalid reversion conjugacy sign");
static_assert(reversion_conjugacy_traits<e012>::sign == -1, "Invalid reversion conjugacy sign");

// grade check (ordering independant)
static_assert(grade_conjugacy_traits<e   >::grade == 0, "Invalid grade");
static_assert(grade_conjugacy_traits<e   >::sign  == +1, "Invalid grade conjugacy sign");
static_assert(grade_conjugacy_traits<e0  >::grade == 1, "Invalid grade");
static_assert(grade_conjugacy_traits<e0  >::sign  == -1, "Invalid grade conjugacy sign");
static_assert(grade_conjugacy_traits<e01 >::grade == 2, "Invalid grade");
static_assert(grade_conjugacy_traits<e01 >::sign  == +1, "Invalid grade conjugacy sign");
static_assert(grade_conjugacy_traits<e012>::grade == 3, "Invalid grade");
static_assert(grade_conjugacy_traits<e012>::sign  == -1, "Invalid grade conjugacy sign");

// clifford adjoint parity check (ordering independant)
static_assert(clifford_adjoint_conjugacy_traits<e   >::sign == +1, "Invalid adjoint conjugacy sign");
static_assert(clifford_adjoint_conjugacy_traits<e0  >::sign == -1, "Invalid adjoint conjugacy sign");
static_assert(clifford_adjoint_conjugacy_traits<e01 >::sign == -1, "Invalid adjoint conjugacy sign");
static_assert(clifford_adjoint_conjugacy_traits<e012>::sign == +1, "Invalid adjoint conjugacy sign");


//
// hodge conjugation check for dimension 1 to 4 follows
// *u is such that u ^ *u = *1 for all unitary blade u, with *1 satisfying choosen basis order.
//
//
// dimension 1
//
// rank 0 (scalar)
// *1 = +e1
static_assert(hodge_conjugacy_traits<e, e0, true>::sign      == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e, e0, true>::bit_set   == e0, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e, e0, false>::sign     == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e, e0, false>::bit_set  == e0, "Invalid hodge conjugate");
// rank 1 (pseudoscalar is a vector)
// *e1 = +1
static_assert(hodge_conjugacy_traits<e0, e0, true>::sign     == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e0, e0, true>::bit_set  == e, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e0, e0, false>::sign    == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e0, e0, false>::bit_set == e, "Invalid hodge conjugate");
//
//
// dimension 2
//
// rank 0 (scalar)
// *1 = +(e0 ^ e1)	 (big endian)
// *1 = +(e1 ^ e0)	 (little endian)
static_assert(hodge_conjugacy_traits<e, e01, true>::sign       == +1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e, e01, true>::bit_set    == e01, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e, e10, false>::sign      == +1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e, e10, false>::bit_set   == e10, "Invalid hodge conjugate");
// rank 1 (vector = pseudovector)
// *e0 = +e1         (big endian)
// *e1 = -e0         (big endian)
static_assert(hodge_conjugacy_traits<e0, e01, true>::sign      == +1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e0, e01, true>::bit_set   == e1,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e1, e01, true>::sign      == -1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e1, e01, true>::bit_set   == e0,  "Invalid hodge conjugate");
// *e0 = -e1         (little endian)
// *e1 = +e0         (little endian)
static_assert(hodge_conjugacy_traits<e0, e10, false>::sign     == -1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e0, e10, false>::bit_set  == e1,  "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e1, e10, false>::sign     == +1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e1, e10, false>::bit_set  == e0,  "Invalid hodge conjugate");
// rank 2 (pseudoscalar)
// **1 = +1
static_assert(hodge_conjugacy_traits<e01, e01, true>::sign     == +1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e01, e01, true>::bit_set  == e,   "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e10, e10, false>::sign    == +1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e10, e10, false>::bit_set == e,   "Invalid hodge conjugate");
//
//
// dimension 3
//
// rank 0 (scalar)
// *1 = +(e0 ^ e1 ^ e2)  (big endian)
// *1 = +(e2 ^ e1 ^ e0)  (little endian)
static_assert(hodge_conjugacy_traits<e, e012, true>::sign     == +1,   "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e, e012, true>::bit_set  == e012, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e, e210, false>::sign    == +1,   "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e, e210, false>::bit_set == e210, "Invalid hodge conjugate");
// rank 1 (vector)
// *e0 = +(e1 ^ e2)      (big endian)
// *e1 = -(e0 ^ e2)      (big endian)
// *e2 = +(e0 ^ e1)      (big endian)
static_assert(hodge_conjugacy_traits<e0, e012, true>::sign     == +1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e0, e012, true>::bit_set  == e12, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e1, e012, true>::sign     == -1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e1, e012, true>::bit_set  == e02, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e2, e012, true>::sign     == +1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e2, e012, true>::bit_set  == e01, "Invalid hodge conjugate");
// *e0 = +(e2 ^ e1)      (little endian)
// *e1 = -(e2 ^ e0)      (little endian)
// *e2 = +(e1 ^ e0)      (little endian)
static_assert(hodge_conjugacy_traits<e0, e210, false>::sign    == +1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e0, e210, false>::bit_set == e21, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e1, e210, false>::sign    == -1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e1, e210, false>::bit_set == e20, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e2, e210, false>::sign    == +1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e2, e210, false>::bit_set == e10, "Invalid hodge conjugate");
// rank 2 (pseudovector)
// *(e0 ^ e1) = +e2      (big endian)
// *(e0 ^ e2) = -e1      (big endian)
// *(e1 ^ e2) = +e0      (big endian)
static_assert(hodge_conjugacy_traits<e01, e012, true>::sign     == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e01, e012, true>::bit_set  == e2, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e02, e012, true>::sign     == -1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e02, e012, true>::bit_set  == e1, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e12, e012, true>::sign     == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e12, e012, true>::bit_set  == e0, "Invalid hodge conjugate");
// *(e1 ^ e0) = +e2      (little endian)
// *(e2 ^ e0) = -e1      (little endian)
// *(e2 ^ e1) = +e0      (little endian)
static_assert(hodge_conjugacy_traits<e10, e210, false>::sign    == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e10, e210, false>::bit_set == e2, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e20, e210, false>::sign    == -1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e20, e210, false>::bit_set == e1, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e21, e210, false>::sign    == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e21, e210, false>::bit_set == e0, "Invalid hodge conjugate");
// rank 3 (pseudoscalar)
// **1 = +1
static_assert(hodge_conjugacy_traits<e012, e012, true>::sign     == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e012, e012, true>::bit_set  == e, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e210, e210, false>::sign    == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e210, e210, false>::bit_set == e, "Invalid hodge conjugate");
//
//
// dimension 4
//
// rank 0 (scalar)
// *1 = +(e0 ^ e1 ^ e2 ^ e3)  (big endian)
// *1 = +(e3 ^ e2 ^ e1 ^ e0)  (little endian)
static_assert(hodge_conjugacy_traits<e, e0123, true>::sign     == +1,   "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e, e0123, true>::bit_set  == e0123, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e, e3210, false>::sign    == +1,   "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e, e3210, false>::bit_set == e3210, "Invalid hodge conjugate");
// rank 1 (vector)
// *e0 = +(e1 ^ e2 ^ e3)      (big endian)
// *e1 = -(e0 ^ e2 ^ e3)      (big endian)
// *e2 = +(e0 ^ e1 ^ e3)      (big endian)
// *e3 = -(e0 ^ e1 ^ e2)      (big endian)
static_assert(hodge_conjugacy_traits<e0, e0123, true>::sign     == +1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e0, e0123, true>::bit_set  == e123, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e1, e0123, true>::sign     == -1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e1, e0123, true>::bit_set  == e023, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e2, e0123, true>::sign     == +1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e2, e0123, true>::bit_set  == e013, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e3, e0123, true>::sign     == -1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e3, e0123, true>::bit_set  == e012, "Invalid hodge conjugate");
// *e0 = -(e3 ^ e2 ^ e1)      (little endian)
// *e1 = +(e3 ^ e2 ^ e0)      (little endian)
// *e2 = -(e3 ^ e1 ^ e0)      (little endian)
// *e3 = +(e2 ^ e1 ^ e0)      (little endian)
static_assert(hodge_conjugacy_traits<e0, e3210, false>::sign    == -1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e0, e3210, false>::bit_set == e321, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e1, e3210, false>::sign    == +1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e1, e3210, false>::bit_set == e320, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e2, e3210, false>::sign    == -1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e2, e3210, false>::bit_set == e310, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e3, e0123, false>::sign    == +1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e3, e0123, false>::bit_set == e210, "Invalid hodge conjugate");
// rank 2 (bivector = pseudobivector)
// *(e0 ^ e1) = +(e2 ^ e3)      (big endian)
// *(e0 ^ e2) = -(e1 ^ e3)      (big endian)
// *(e0 ^ e3) = +(e1 ^ e2)      (big endian)
// *(e1 ^ e2) = +(e0 ^ e3)      (big endian)
// *(e1 ^ e3) = -(e0 ^ e2)      (big endian)
// *(e2 ^ e3) = +(e0 ^ e1)      (big endian)
static_assert(hodge_conjugacy_traits<e01, e0123, true>::sign     == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e01, e0123, true>::bit_set  == e23, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e02, e0123, true>::sign     == -1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e02, e0123, true>::bit_set  == e13, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e03, e0123, true>::sign     == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e03, e0123, true>::bit_set  == e12, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e12, e0123, true>::sign     == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e12, e0123, true>::bit_set  == e03, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e13, e0123, true>::sign     == -1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e13, e0123, true>::bit_set  == e02, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e23, e0123, true>::sign     == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e23, e0123, true>::bit_set  == e01, "Invalid hodge conjugate");
// *(e1 ^ e0) = +(e3 ^ e2)      (little endian)
// *(e2 ^ e0) = -(e3 ^ e1)      (little endian)
// *(e3 ^ e0) = +(e2 ^ e1)      (little endian)
// *(e2 ^ e1) = +(e3 ^ e0)      (little endian)
// *(e3 ^ e1) = -(e2 ^ e0)      (little endian)
// *(e3 ^ e2) = +(e1 ^ e0)      (little endian)
static_assert(hodge_conjugacy_traits<e10, e0123, false>::sign     == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e10, e0123, false>::bit_set  == e32, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e20, e0123, false>::sign     == -1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e20, e0123, false>::bit_set  == e31, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e30, e0123, false>::sign     == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e30, e0123, false>::bit_set  == e21, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e21, e0123, false>::sign     == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e21, e0123, false>::bit_set  == e30, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e31, e0123, false>::sign     == -1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e31, e0123, false>::bit_set  == e20, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e32, e0123, false>::sign     == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e32, e0123, false>::bit_set  == e10, "Invalid hodge conjugate");
// rank 3 (pseudovector)
// *(e0 ^ e1 ^ e2) = +e3      (big endian)
// *(e0 ^ e1 ^ e3) = -e2      (big endian)
// *(e0 ^ e2 ^ e3) = +e1      (big endian)
// *(e1 ^ e2 ^ e3) = -e0      (big endian)
static_assert(hodge_conjugacy_traits<e012, e0123, true>::sign     == +1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e012, e0123, true>::bit_set  == e3, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e013, e0123, true>::sign     == -1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e013, e0123, true>::bit_set  == e2, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e023, e0123, true>::sign     == +1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e023, e0123, true>::bit_set  == e1, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e123, e0123, true>::sign     == -1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e123, e0123, true>::bit_set  == e0, "Invalid hodge conjugate");
// *(e2 ^ e1 ^ e0) = -e3      (little endian)
// *(e3 ^ e1 ^ e0) = +e2      (little endian)
// *(e3 ^ e2 ^ e0) = -e1      (little endian)
// *(e3 ^ e2 ^ e1) = +e0      (little endian)
static_assert(hodge_conjugacy_traits<e210, e3210, false>::sign     == -1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e210, e3210, false>::bit_set  == e3, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e310, e3210, false>::sign     == +1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e310, e3210, false>::bit_set  == e2, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e320, e3210, false>::sign     == -1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e320, e3210, false>::bit_set  == e1, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e321, e3210, false>::sign     == +1,  "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e321, e3210, false>::bit_set  == e0, "Invalid hodge conjugate");
// rank 4 (pseudoscalar)
// **1 = +1
static_assert(hodge_conjugacy_traits<e0123, e0123, true>::sign     == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e0123, e0123, true>::bit_set  == e, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e3210, e3210, false>::sign    == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e3210, e3210, false>::bit_set == e, "Invalid hodge conjugate");

//
// some random outer check in dimension 3 + 1 generated by {e0, e2, e3} + {e1}...
//
static_assert(hodge_conjugacy_traits<e0, e023, true>::sign    == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e0, e023, true>::bit_set == e23, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e1, e023, true>::sign    == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e1, e023, true>::bit_set == e023, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e2, e023, true>::sign    == -1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e2, e023, true>::bit_set == e03, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e3, e023, true>::sign    == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e3, e023, true>::bit_set == e02, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e01, e023, true>::sign    == -1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e01, e023, true>::bit_set == e23, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e12, e023, true>::sign    == -1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e12, e023, true>::bit_set == e03, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e13, e023, true>::sign    == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e13, e023, true>::bit_set == e02, "Invalid hodge conjugate");
//
// some random outer check in dimension 2 + 2 generated by {e0, e2} + {e1, e3}...
//
static_assert(hodge_conjugacy_traits<e0, e02, true>::sign    == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e0, e02, true>::bit_set == e2, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e1, e02, true>::sign    == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e1, e02, true>::bit_set == e02, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e2, e02, true>::sign    == -1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e2, e02, true>::bit_set == e0, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e3, e02, true>::sign    == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e3, e02, true>::bit_set == e02, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e01, e02, true>::sign    == -1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e01, e02, true>::bit_set == e2, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e12, e02, true>::sign    == -1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e12, e02, true>::bit_set == e0, "Invalid hodge conjugate");
static_assert(hodge_conjugacy_traits<e13, e02, true>::sign    == +1, "Invalid hodge conjugacy sign");
static_assert(hodge_conjugacy_traits<e13, e02, true>::bit_set == e02, "Invalid hodge conjugate");
#endif