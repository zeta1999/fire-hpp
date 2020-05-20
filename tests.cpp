
#include <gtest/gtest.h>
#include "fire.hpp"

#define EXPECT_EXIT_SUCCESS(statement) EXPECT_EXIT(statement, ::testing::ExitedWithCode(0), "")
#define EXPECT_EXIT_FAIL(statement) EXPECT_EXIT(statement, ::testing::ExitedWithCode(fire::_failure_code), "")

using namespace std;
using namespace fire;

void init_args(const vector<string> &args, bool loose_query, int named_calls = 1000000) {
    const char ** argv = new const char *[args.size()];
    for(size_t i = 0; i < args.size(); ++i)
        argv[i] = args[i].c_str();

    _matcher::init_args(args.size(), argv, named_calls, loose_query);

    delete [] argv;
}

void init_args_loose_query(const vector<string> &args) {
    init_args(args, true);
}

void init_args_strict_query(const vector<string> &args, int named_calls) {
    init_args(args, false, named_calls);
}



TEST(optional, value) {
    fire::optional<int> no_value;
    EXPECT_FALSE((bool) no_value);
    EXPECT_FALSE(no_value.has_value());
    EXPECT_EQ(no_value.value_or(3), 3);
    EXPECT_EXIT_FAIL(no_value.value());

    fire::optional<int> value(1);
    EXPECT_TRUE((bool) value);
    EXPECT_TRUE(value.has_value());
    EXPECT_EQ(value.value_or(3), 1);
    EXPECT_EQ(value.value(), 1);
}

TEST(optional, assignment) {
    fire::optional<int> opt1, opt2 = 3, opt3(3);
    EXPECT_FALSE(opt1.has_value());
    EXPECT_TRUE(opt2.has_value());
    EXPECT_TRUE(opt3.has_value());
    opt1 = opt3;
    EXPECT_TRUE(opt1.has_value());
}

TEST(optional, no_value) {
    fire::optional<int> opt;
    EXPECT_FALSE((bool) opt);
    EXPECT_FALSE(opt.has_value());
    EXPECT_EQ(opt.value_or(3), 3);
}


TEST(identifier, constructor) {
    identifier({"l", "long"});
    identifier({"l"});
    identifier({"long"});
    EXPECT_EXIT_FAIL(identifier({"l", "l"}));
    EXPECT_EXIT_FAIL(identifier({"long", "l"}));
    EXPECT_EXIT_FAIL(identifier({"long", "long"}));
    EXPECT_EXIT_FAIL(identifier({"", "long"}));
    EXPECT_EXIT_FAIL(identifier({"l", ""}));

    identifier("l");
    identifier("long");
    EXPECT_EXIT_FAIL(identifier(""));
}

TEST(identifier, overlap) {
    identifier long1("l"), long2({"l", "long"}), long3("long");
    identifier short1("s"), short2({"s", "short"}), short3("short");

    EXPECT_TRUE(long1.overlaps(long2));
    EXPECT_TRUE(long2.overlaps(long3));
    EXPECT_FALSE(long3.overlaps(long1));

    EXPECT_FALSE(long1.overlaps(short1));
    EXPECT_FALSE(long2.overlaps(short2));
    EXPECT_FALSE(long3.overlaps(short3));
}

TEST(identifier, contains) {
    identifier long1("l"), long2({"l", "long"}), long3("long");

    EXPECT_FALSE(long1.contains("long"));
    EXPECT_TRUE(long2.overlaps("long"));
    EXPECT_TRUE(long3.overlaps("long"));

    EXPECT_TRUE(long1.contains("l"));
    EXPECT_TRUE(long2.overlaps("l"));
    EXPECT_FALSE(long3.overlaps("l"));
}

TEST(identifier, help) {
    EXPECT_EQ(identifier("l").help(), "-l");
    EXPECT_EQ(identifier({"l", "long"}).help(), "-l|--long");
    EXPECT_EQ(identifier("long").help(), "--long");
}

TEST(identifier, longer) {
    EXPECT_EQ(identifier("l").longer(), "-l");
    EXPECT_EQ(identifier({"l", "long"}).longer(), "--long");
    EXPECT_EQ(identifier("long").longer(), "--long");
}

TEST(identifier, less) {
    EXPECT_TRUE(identifier("a") < identifier("z"));
    EXPECT_FALSE(identifier("z") < identifier("a"));

    EXPECT_TRUE(identifier("abc") < identifier("zyx"));
    EXPECT_FALSE(identifier("zyx") < identifier("abc"));

    EXPECT_TRUE(identifier({"z", "aa"}) < identifier({"a", "az"}));
    EXPECT_FALSE(identifier({"a", "az"}) < identifier({"z", "aa"}));
}


TEST(matcher, invalid_input) {
    EXPECT_EXIT_FAIL(init_args_loose_query({"./run_tests", "--i"}));
    EXPECT_EXIT_FAIL(init_args_loose_query({"./run_tests", "--i", "0"}));
    EXPECT_EXIT_FAIL(init_args_loose_query({"./run_tests", "-int", "0"}));
}


TEST(help, help_invocation) {
    EXPECT_EXIT_SUCCESS(init_args_strict_query({"./run_tests", "-h"}, 0));
    EXPECT_EXIT_SUCCESS(init_args_strict_query({"./run_tests", "--help"}, 0));

    init_args_strict_query({"./run_tests", "-h"}, 1);
    EXPECT_EXIT_SUCCESS(int_t i_undef = arg("i"));

    init_args_strict_query({"./run_tests", "-h"}, 1);
    EXPECT_EXIT_SUCCESS(int_t i_undef = arg("i", "", 0));

    init_args_strict_query({"./run_tests", "-h", "-i", "1"}, 1);
    EXPECT_EXIT_SUCCESS(int_t i_undef = arg("i"));

    init_args_strict_query({"./run_tests", "-h", "-i", "1"}, 1);
    EXPECT_EXIT_SUCCESS(int_t i_undef = arg("i", "", 0));

    init_args_strict_query({"./run_tests", "-h"}, 1);
    EXPECT_EXIT_SUCCESS(fire::optional<int_t> i_undef = arg("i"));

    init_args_strict_query({"./run_tests", "-h", "-i", "1"}, 1);
    EXPECT_EXIT_SUCCESS(fire::optional<int_t> i_undef = arg("i"));

    init_args_strict_query({"./run_tests", "-h"}, 3);
    int_t i1_undef = arg("i1");
    int_t i2_undef = arg("i2");
    EXPECT_EXIT_SUCCESS(int_t i3_undef = arg("i3"));
}


TEST(arg, false_hyphens) {
    init_args_loose_query({"./run_tests"});

    EXPECT_EXIT_FAIL(arg("--undefined"));
    EXPECT_EXIT_FAIL(arg("-i"));
    EXPECT_EXIT_FAIL(arg("-i", "", 0));
    EXPECT_EXIT_FAIL(arg("-f", "", 0.0));
    EXPECT_EXIT_FAIL(arg("-s", "", "test"));
}

TEST(arg, defaults) {
    init_args_loose_query({"./run_tests"});

    EXPECT_EQ((int) arg("i", "", 1), 1);
    EXPECT_NEAR((double) arg("f", "", 2), 2, 1e-5);
    EXPECT_NEAR((double) arg("f", "", 2.0), 2.0, 1e-5);
    EXPECT_EQ((string) arg("s", "", "test"), "test");

    EXPECT_EXIT_FAIL((int) arg("i", "", 1.0));
    EXPECT_EXIT_FAIL((int) arg("i", "", "test"));
    EXPECT_EXIT_FAIL((double) arg("f", "", "test"));
    EXPECT_EXIT_FAIL((string) arg("s", "", 1));
    EXPECT_EXIT_FAIL((string) arg("s", "", 1.0));

    EXPECT_EXIT_FAIL((bool) arg("b", "", 1));
}

TEST(arg, correct_parsing) {
    init_args_loose_query({"./run_tests", "--bool1", "-i", "1", "-f", "2.0", "-s", "test", "--bool2"});

    EXPECT_TRUE((bool) arg("bool1"));
    EXPECT_TRUE((bool) arg("bool2"));
    EXPECT_FALSE((bool) arg("undefined"));

    EXPECT_EQ((int) arg("i", "", 2), 1);

    EXPECT_EQ((int) arg("i"), 1);
    EXPECT_NEAR((double) arg("i"), 1.0, 1e-5);
    EXPECT_NEAR((double) arg("f"), 2.0, 1e-5);
    EXPECT_EQ((string) arg("s"), "test");

    EXPECT_EXIT_FAIL((int) arg("f"));
    EXPECT_EXIT_FAIL((int) arg("s"));
    EXPECT_EXIT_FAIL((double) arg("s"));

    EXPECT_EXIT_FAIL((int) arg("x"));
}

TEST(arg, strict_query) {
    EXPECT_EXIT_FAIL(init_args_strict_query({"./run_tests", "-i", "1"}, 0));

    init_args_strict_query({"./run_tests", "-i", "1"}, 2);
    (int_t) arg("i");
    EXPECT_EXIT_FAIL((int_t) arg("x"));

    init_args_strict_query({"./run_tests", "-i", "1"}, 1);
    EXPECT_EXIT_FAIL((int_t) arg("x"));

    init_args_strict_query({"./run_tests", "-i", "1"}, 1);
    EXPECT_EXIT_FAIL((int_t) arg("x", "", 0));
}

TEST(arg, optional_arguments) {
    init_args_loose_query({"./run_tests", "-i", "1", "-f", "1.0", "-s", "test"});

    fire::optional<int_t> i_undef = arg("undefined");
    fire::optional<int_t> i = arg("i");
    EXPECT_FALSE(i_undef.has_value());
    EXPECT_EQ(i.value(), 1);

    fire::optional<float_t> f_undef = arg("undefined");
    fire::optional<float_t> f = arg("f");
    EXPECT_FALSE(f_undef.has_value());
    EXPECT_NEAR(f.value(), 1.0, 1e-5);

    fire::optional<string_t> s_undef = arg("undefined");
    fire::optional<string_t> s = arg("s");
    EXPECT_FALSE(s_undef.has_value());
    EXPECT_EQ(s.value(), "test");
}

TEST(arg, optional_and_default) {
    init_args_loose_query({"./run_tests", "-i", "0"});

    EXPECT_EXIT_FAIL(fire::optional<int_t> i_undef = arg("undefined", "", 0));
    EXPECT_EXIT_FAIL(fire::optional<int_t> i = arg("i", "", 0));
}

TEST(arg, duplicate_parameter) {
    init_args_strict_query({"./run_tests"}, 2);

    int_t i1 = arg("undefined", "", 0);
    EXPECT_EXIT_FAIL(int_t i2 = arg("undefined", "", 0));
}
