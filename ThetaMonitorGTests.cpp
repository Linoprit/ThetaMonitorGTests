#include <stdio.h>
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <chrono>

// build googletest:
// https://forums.linuxmint.com/viewtopic.php?t=267500
// https://www.ics.uci.edu/~pattis/common/modules46/googletestpc.html
// enable -lpthread

int main(int argc, char **argv) {

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();

	return 0;
}
