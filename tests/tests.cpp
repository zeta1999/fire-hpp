
#include <gtest/gtest.h>
#include "../fire.hpp"

#define EXPECT_EXIT_SUCCESS(statement) EXPECT_EXIT(statement, ::testing::ExitedWithCode(0), "")
#define EXPECT_EXIT_FAIL(statement) EXPECT_EXIT(statement, ::testing::ExitedWithCode(fire::_failure_code), "")

using namespace std;
using namespace fire;

void init_args(const vector<string> &args, bool positional, bool strict, int named_calls = 1000000) {
    const char ** argv = new const char *[args.size()];
    for(size_t i = 0; i < args.size(); ++i)
        argv[i] = args[i].c_str();

    fire::_::help_logger = fire::_help_logger();
    fire::_::matcher = fire::_matcher(args.size(), argv, named_calls, positional, strict);

    delete [] argv;
}

void init_args(const vector<string> &args) {
    init_args(args, false, false);
}

void init_args_strict(const vector<string> &args, int named_calls) {
    init_args(args, false, true, named_calls);
}

void init_args_positional(const vector<string> &args) {
    init_args(args, true, false);
}

void init_args_positional_strict(const vector<string> &args, int named_calls) {
    init_args(args, true, true, named_calls);
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

    identifier(0);
}

TEST(identifier, overlap) {
    identifier long0("l"), long1({"l", "long"}), long2("long");
    identifier short0("s"), short1({"s", "short"}), short2("short");
    identifier pos0(0), pos1(1);

    EXPECT_TRUE(long0.overlaps(long1));
    EXPECT_TRUE(long1.overlaps(long2));
    EXPECT_FALSE(long2.overlaps(long0));

    EXPECT_FALSE(long0.overlaps(short0));
    EXPECT_FALSE(long1.overlaps(short1));
    EXPECT_FALSE(long2.overlaps(short2));

    EXPECT_FALSE(pos0.overlaps(long0));
    EXPECT_FALSE(pos0.overlaps(long2));
    EXPECT_TRUE(pos0.overlaps(pos0));
    EXPECT_FALSE(pos0.overlaps(pos1));
}

TEST(identifier, contains) {
    identifier long0("l"), long1({"l", "long"}), long2("long");
    identifier pos0(0);

    EXPECT_FALSE(long0.contains("long"));
    EXPECT_TRUE(long1.contains("long"));
    EXPECT_TRUE(long2.contains("long"));

    EXPECT_TRUE(long0.contains("l"));
    EXPECT_TRUE(long1.contains("l"));
    EXPECT_FALSE(long2.contains("l"));

    EXPECT_FALSE(long0.contains(0));
    EXPECT_FALSE(long2.contains(0));

    EXPECT_TRUE(pos0.contains(0));
    EXPECT_FALSE(pos0.contains(1));
}

TEST(identifier, help) {
    EXPECT_EQ(identifier("l").help(), "-l");
    EXPECT_EQ(identifier({"l", "long"}).help(), "-l|--long");
    EXPECT_EQ(identifier("long").help(), "--long");

    EXPECT_EQ(identifier(0).help(), "<0>");
    EXPECT_EQ(identifier().help(), "...");
}

TEST(identifier, longer) {
    EXPECT_EQ(identifier("l").longer(), "-l");
    EXPECT_EQ(identifier({"l", "long"}).longer(), "--long");
    EXPECT_EQ(identifier("long").longer(), "--long");

    EXPECT_EQ(identifier(0).longer(), "<0>");
    EXPECT_EQ(identifier().longer(), "...");
}

TEST(identifier, less) {
    EXPECT_TRUE(identifier("a") < identifier("z"));
    EXPECT_FALSE(identifier("z") < identifier("a"));

    EXPECT_TRUE(identifier("abc") < identifier("zyx"));
    EXPECT_FALSE(identifier("zyx") < identifier("abc"));

    EXPECT_TRUE(identifier({"z", "aa"}) < identifier({"a", "az"}));
    EXPECT_FALSE(identifier({"a", "az"}) < identifier({"z", "aa"}));
}

TEST(identifier, equals) {
    EXPECT_TRUE(identifier() == identifier());
    EXPECT_FALSE(identifier() == identifier("a"));
    EXPECT_FALSE(identifier("a") == identifier("b"));
    EXPECT_FALSE(identifier("bca") == identifier("abc"));
}


TEST(matcher, invalid_input) {
    EXPECT_EXIT_FAIL(init_args({"./run_tests", "--i"}));
    EXPECT_EXIT_FAIL(init_args({"./run_tests", "--i", "0"}));
    EXPECT_EXIT_FAIL(init_args({"./run_tests", "-ab", "0"}));
    EXPECT_EXIT_FAIL(init_args({"./run_tests", "0"}));
}

TEST(matcher, boolean_flags) {
    init_args({"./run_tests", "-a", "-bcd"});
    EXPECT_TRUE((bool) arg("a"));
    EXPECT_TRUE((bool) arg("b"));
    EXPECT_TRUE((bool) arg("c"));
    EXPECT_TRUE((bool) arg("d"));
}

TEST(matcher, equations) {
    init_args({"./run_tests", "-a=b", "--abc=xy", "-x=y=z"});
    EXPECT_EQ((string) arg("a"), "b");
    EXPECT_EQ((string) arg("abc"), "xy");
    EXPECT_EQ((string) arg("x"), "y=z"); // quotation marks are omitted from command line

    EXPECT_EXIT_FAIL(init_args({"./run_tests", "-a=b", "123"}));
}

TEST(matcher, positional_mode) {
    init_args_positional({"./run_tests"});
    init_args_positional({"./run_tests", "0"});
    init_args_positional({"./run_tests", "0", "0"});
    init_args_positional({"./run_tests", "-x", "0"}); // there's no equals sign, so "0" is positional
    EXPECT_EXIT_FAIL((int) arg("x"));
}


TEST(help, help_invocation) {
    EXPECT_EXIT_SUCCESS(init_args_strict({"./run_tests", "-h"}, 0));
    EXPECT_EXIT_SUCCESS(init_args_strict({"./run_tests", "--help"}, 0));
    EXPECT_EXIT_SUCCESS(init_args_strict({"./run_tests", "--help", "1"}, 0));

    init_args_strict({"./run_tests", "-h"}, 1);
    EXPECT_EXIT_SUCCESS(int i_undef = arg("i"));

    init_args_strict({"./run_tests", "-h", "-i", "1"}, 1);
    EXPECT_EXIT_SUCCESS(int i_undef = arg("i"));

    init_args_strict({"./run_tests", "-h"}, 3);
    int i1_undef = arg("i1");
    int i2_undef = arg("i2");
    EXPECT_EXIT_SUCCESS(int i3_undef = arg("i3"));

}

TEST(help, positional_help_invocation) {
    EXPECT_EXIT_SUCCESS(init_args_positional_strict({"./run_tests", "-h"}, 0));
    EXPECT_EXIT_SUCCESS(init_args_positional_strict({"./run_tests", "--help"}, 0));
    EXPECT_EXIT_SUCCESS(init_args_positional_strict({"./run_tests", "--help", "1"}, 0));

    init_args_positional_strict({"./run_tests", "-h"}, 1);
    EXPECT_EXIT_SUCCESS(int i_undef = arg(0));

    init_args_positional_strict({"./run_tests", "-h", "1"}, 1);
    EXPECT_EXIT_SUCCESS(int i_undef = arg(0));

    init_args_positional_strict({"./run_tests", "-h"}, 3);
    int i1_undef = arg(0);
    int i2_undef = arg(1);
    EXPECT_EXIT_SUCCESS(int i3_undef = arg(2));

    init_args_positional_strict({"./run_tests", "-h"}, 1);
    EXPECT_EXIT_SUCCESS(vector<string> v_undef = arg::vector());
}


TEST(arg, false_hyphens) {
    init_args({"./run_tests"});

    EXPECT_EXIT_FAIL(arg("--undefined"));
    EXPECT_EXIT_FAIL(arg("-i"));
    EXPECT_EXIT_FAIL(arg("-i", "", 0));
    EXPECT_EXIT_FAIL(arg("-f", "", 0.0));
    EXPECT_EXIT_FAIL(arg("-s", "", "test"));
}

TEST(arg, defaults) {
    init_args({"./run_tests"});

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
    init_args({"./run_tests", "--bool1", "-i", "1", "-f", "2.0", "-s", "test", "--bool2"});

    EXPECT_TRUE((bool) arg("bool1"));
    EXPECT_TRUE((bool) arg("bool2"));
    EXPECT_FALSE((bool) arg("undefined"));

    EXPECT_EQ((int) arg("i", "", 2), 1);

    EXPECT_EQ((int) arg("i"), 1);
    EXPECT_NEAR((double) arg("i"), 1.0, 1e-5);
    EXPECT_NEAR((double) arg("f"), 2.0, 1e-5);
    EXPECT_EQ((string) arg("s"), "test");
}

TEST(arg, incorrect_parsing) {
    init_args({"./run_tests", "-i", "1", "-f", "2.0", "-s", "test"});
    EXPECT_EXIT_FAIL((bool) arg("i"));

    EXPECT_EXIT_FAIL((int) arg("f"));
    EXPECT_EXIT_FAIL((int) arg("s"));
    EXPECT_EXIT_FAIL((double) arg("s"));

    EXPECT_EXIT_FAIL((int) arg("x"));

    init_args({"./run_tests"});
    EXPECT_EXIT_FAIL((int) arg(0, "", -1));
}

TEST(arg, positional_parsing) {
    init_args_positional({"./run_tests", "0", "1"});
    EXPECT_EQ((int) arg(0), 0);
    EXPECT_EQ((int) arg(1), 1);
    EXPECT_EXIT_FAIL((int) arg(2));

    init_args_positional({"./run_tests", "0", "-x=3", "1"});
    EXPECT_EQ((int) arg("x"), 3);
    EXPECT_EQ((int) arg(0), 0);
    EXPECT_EQ((int) arg(1), 1);
    EXPECT_EQ((int) arg(2, "", -1), -1);
    optional<int> opt = arg(2);
    EXPECT_FALSE(opt.has_value());

    vector<int> all1 = arg::vector();
    EXPECT_EQ(all1, vector<int>({0, 1}));
}

TEST(arg, all_positional_parsing) {
    init_args_positional({"./run_tests"});
    vector<int> all0 = arg::vector();
    EXPECT_EQ(all0, vector<int>({}));

    init_args_positional({"./run_tests", "0", "1"});
    vector<int> all1 = arg::vector();
    EXPECT_EQ(all1, vector<int>({0, 1}));

    init_args_positional({"./run_tests", "text"});
    vector<std::string> all2 = arg::vector();
    EXPECT_EQ(all2, vector<std::string>({"text"}));
}

TEST(arg, precision) {
    init_args({"./run_tests", "--65535", "65535", "--65536", "65536",
               "--permitted", "1000000000000", "--overflow", "100000000000000000000000000000000000000"});

    EXPECT_EXIT_FAIL((unsigned) arg("a", "", "-1"));

    (uint16_t) arg("65535");
    EXPECT_EXIT_FAIL((uint16_t) arg("65536"));

    (int32_t) arg("a", "", (1LL << 31) - 1);
    EXPECT_EXIT_FAIL((int32_t) arg("a", "", 1LL << 31));

    (uint32_t) arg("a", "", (1LL << 32) - 1);
    EXPECT_EXIT_FAIL((uint32_t) arg("a", "", 1LL << 32));

    (int64_t) arg("a", "", 1LL << 62);
    (int64_t) arg("permitted");
    EXPECT_EXIT_FAIL((int64_t) arg("overflow"));

    (double) arg("a", "", 1e100);
    EXPECT_EXIT_FAIL((float) arg("a", "", 1e100));
}

TEST(arg, strict_query) {
    init_args_strict({"./run_tests"}, 0);

    init_args_strict({"./run_tests", "-x", "0"}, 1);
    (int) arg("x");

    EXPECT_EXIT_FAIL(init_args_strict({"./run_tests", "-i", "1"}, 0));

    init_args_strict({"./run_tests", "-i", "1"}, 2);
    (int) arg("i");
    EXPECT_EXIT_FAIL((int) arg("x"));

    init_args_strict({"./run_tests", "-i", "1"}, 1);
    EXPECT_EXIT_FAIL((int) arg("x"));

    init_args_strict({"./run_tests", "-i", "1"}, 1);
    EXPECT_EXIT_FAIL((int) arg("x", "", 0));
}

TEST(arg, strict_query_positional) {
    init_args_positional_strict({"./run_tests", "0", "1"}, 1);
    EXPECT_EXIT_FAIL((int) arg(0)); // Invalid 2-nd argument

    init_args_positional_strict({"./run_tests", "1"}, 2);
    fire::optional<int> x0 = arg(0), x1 = arg(1);
    EXPECT_EQ(x0.value(), 1);
    EXPECT_FALSE(x1.has_value());

    init_args_positional_strict({"./run_tests", "0", "1"}, 1);
    EXPECT_EXIT_FAIL((int) arg(0));

    init_args_positional_strict({"./run_tests", "0", "1"}, 1);
    vector<int> all0 = arg::vector();
}

TEST(arg, strict_query_all_positional) {
    init_args_positional_strict({"./run_tests", "0", "1"}, 2);
    vector<int> all1 = arg::vector();
    EXPECT_EXIT_FAIL((int) arg(0));

    init_args_positional_strict({"./run_tests", "0", "1"}, 2);
    (int) arg(0);
    EXPECT_EXIT_FAIL(vector<int> all2 = arg::vector());
}

TEST(arg, optional_arguments) {
    init_args({"./run_tests", "-i", "1", "-f", "1.0", "-s", "test"});

    fire::optional<int> i_undef = arg("undefined");
    fire::optional<int> i = arg("i");
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
    init_args({"./run_tests", "-i", "0"});

    EXPECT_EXIT_FAIL(fire::optional<int> i_undef = arg("undefined", "", 0));
    EXPECT_EXIT_FAIL(fire::optional<int> i = arg("i", "", 0));
}

TEST(arg, duplicate_parameter) {
    init_args_strict({"./run_tests"}, 2);

    int i1 = arg("undefined", "", 0);
    EXPECT_EXIT_FAIL(int i2 = arg("undefined", "", 0));
}
