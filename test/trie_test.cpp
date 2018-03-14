#include <iostream>
#include <string>
#include <chrono>

#include "trie.hpp"

#define BOOST_TEST_MODULE trie_test
#include "boost/test/unit_test.hpp"

template<typename Out, typename T>
Out& operator<<(Out& out, const std::vector<T>& v) {
	out << "{ ";
	for (auto i : v) {
		out << i << ' ';
	}
	out << '}';

	return out;
}

BOOST_AUTO_TEST_SUITE(trie_test_suite)

BOOST_AUTO_TEST_CASE(trie_test) {
	Trie<std::string, int> trie;

	BOOST_CHECK(trie.is_leaf());

	trie.insert(std::string("test"), 42);
	trie.insert(std::string("trie"), 1);
	trie.insert(std::string("abc"), 7);

	BOOST_CHECK(!trie.is_leaf());

	BOOST_CHECK_EQUAL(trie["test"], 42);
	BOOST_CHECK_EQUAL(trie["trie"], 1);
	BOOST_CHECK_EQUAL(trie["t"], 0);
	BOOST_CHECK_EQUAL(trie["abd"], 0);

	trie["abd"] = 3;

	BOOST_CHECK_EQUAL(trie["abd"], 3);

	struct {
		int operator()(int data) const {
			return ++data;
		}
	} increase_with_one;

	trie.insert(std::string("tree"), increase_with_one);

	BOOST_CHECK_EQUAL(trie["t"], 1);
	BOOST_CHECK_EQUAL(trie["tr"], 1);
	BOOST_CHECK_EQUAL(trie["tre"], 1);
	BOOST_CHECK_EQUAL(trie["tree"], 1);

	BOOST_CHECK(trie.match("trie").first);
	BOOST_CHECK(trie.match("tree").first);
	BOOST_CHECK(trie.match("tr").first);
	BOOST_CHECK(!trie.match("true").first);

	trie["tr"] = 29;
	BOOST_CHECK_EQUAL(trie.match("tr").second, 29);
	BOOST_CHECK_EQUAL(trie.match("true").second, 29);

	std::string accu;
	trie.each_elem([&accu](char ch, int& data){
		accu += ch;
		data = 1;

		return true;
	});

	BOOST_CHECK_EQUAL(accu, std::string("abcdtestreeie"));
	BOOST_CHECK_EQUAL(trie["tr"], 1);
	BOOST_CHECK_EQUAL(trie["test"], 1);

	accu.clear();
	trie.each([&accu](std::string key, int& data) {
		if (!key.empty()) {
			accu += key;
			data = 2;
		}
		return true;
	});

	BOOST_CHECK_EQUAL(accu, std::string("aababcabdttetestesttrtretreetritrie"));
	BOOST_CHECK_EQUAL(trie["tr"], 2);
	BOOST_CHECK_EQUAL(trie["test"], 2);
}

BOOST_AUTO_TEST_CASE (trie_iterator_test) {
	Trie<std::vector<int>, int> trie;

	trie.insert({ 1, 2, 3, 4}, 1);
	trie.insert({ 5, 6, 7, 8, 9 }, 2);
	trie.insert({ 1, 2, 3, 5, 8, 13, 21 }, 3);

	auto it = trie.begin();
	BOOST_CHECK_EQUAL((*it).second, 0);

	BOOST_CHECK_EQUAL(trie[{1}], 0);

	++it;
	(*it).second = 1;
	BOOST_CHECK_EQUAL((*it).second, 1);
	BOOST_CHECK_EQUAL(trie[{1}], 1);
    BOOST_CHECK_EQUAL(it->second, 1);
	
	for (int i = 0; i < 7; ++i)
		++it;

	BOOST_CHECK_EQUAL(it->second, 3);
	it->second = 42;
	BOOST_CHECK_EQUAL(it->second, 42);
	BOOST_CHECK_EQUAL(trie[std::vector<int>({ 1, 2, 3, 5, 8, 13, 21 })], 42);

	for (int i = 0; i < 5; ++i)
		++it;

	BOOST_CHECK_EQUAL(it->second, 2);

	++it;

	BOOST_CHECK(it.at_end());
	BOOST_CHECK(it == trie.end());

	--it;

	BOOST_CHECK_EQUAL(it->second, 2);

	for (int i = 0; i < 5; ++i)
		--it;

	BOOST_CHECK_EQUAL(it->second, 42);

	for (int i = 0; i < 7; ++i)
		--it;

	BOOST_CHECK_EQUAL(it->second, 1);

	--it;

	BOOST_CHECK(it.at_begin());
	BOOST_CHECK(it == trie.begin());
}

BOOST_AUTO_TEST_SUITE_END()
