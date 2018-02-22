#ifndef __TRIE_HPP__
#define __TRIE_HPP__

#include <memory>
#include <vector>
#include <algorithm>

template<typename T>
class TrieIterator;

struct first_less_than {
	template<typename PairT, typename ValT>
	bool operator()(const PairT& lhs, const ValT& rhs) const {
		return lhs.first < rhs;
	}
}; // first_less_than

// Trie: a key-value storage for keys that are sequences of elements (e.g. a string or a vector).
// The trie is efficient in case of overlapping keys. 
// Keys should provide stl-style forward iterators that give access to the elements in the sequence through dereferencing (operator*())
// and begin() and end() functions to obtain such iterators. 
// Data should be default constructible.
// The trie does not differentiate between stored keys and subsequences of those keys, each subsequence is associated with a data element.
// Insertion may invalidate iterators
//
// Design issues:
// 1. compare() provides rather specific functionality, therefore it is currently not part of trie itself, it is however strongly dependent
//    on the recursive implementation of each_elem.
// 2. Trie provides a minimal iterator, but is not compatible with stl style containers, for example insert() overwrites existing entries
//    as it has no means to differentiate between stored values and empty values that correspond to subsequences of stored keys.
//    One can however construct stl-style tries based on this class, e.g. by adding a data structure that contains a flag to indicate if a path
//    corresponds to a stored key.
// 3. The vector-based implementation is space effcient, but insertion may invalidate iterators, alternatively std::map or a hash can be used. 
//		Could extract functionality for storing, sorting and finding nodes in a linear_map, a wrapper around std::vector that provides the same interface as std::map.
// 4. each_elem() does not use the data of the root. Could provide an option to do so. In that case, the functor should provide a suitable overload of operator().
//
template<typename Key, typename Data>
class Trie {
public:
	using key_type = Key;
	using key_element_type = typename Key::value_type;
	using data_type = Data;
	using value_type = std::pair<key_type, data_type>;

	using iterator = TrieIterator<Trie>;
private:
	using Nodes = std::vector<std::pair<typename key_element_type, std::unique_ptr<Trie>>>;
	
	Data _data;
	Nodes _nodes;
public:
	using node_iterator = typename Nodes::iterator;

	Trie() : _data{} {}

	Trie(const Trie& other) 
		: _data(other._data) 
	{
		// deep copy
		for (node : other._nodes) {
			_nodes.push_back(node.first, new Trie(*node.second));
		}
	}

	Trie(Trie&& other) // TODO: check implementation
		: _data(std::move(_other.data)), _nodes(std::move(_other.nodes))
	{}

	Trie& operator=(const Trie& other) {
		if (&other != this) {
			_data = other._data;

			for (node : other._nodes) {
				_nodes.push_back(node.first, new Trie(*node.second));
			}
		}

		return *this;
	}

	Trie& operator=(Trie&& other) { // TODO: check implementation
		_data = std::move(other._data);
		_nodes = std::move(other._nodes);

		other._data = data_type();
	}

	const data_type& data() const {return _data;}
	data_type& data() { return _data; }

	bool is_leaf() const {
		return _nodes.empty();
	}

	void clear() {
		_data = data_type();
		_nodes.clear();
	}

	node_iterator nbegin() { return _nodes.begin(); }
	node_iterator nend() { return _nodes.end(); }

	node_iterator nlower_bound(const key_element_type& key) {
		return std::lower_bound(_nodes.begin(), _nodes.end(), key, first_less_than());
	}

	node_iterator insert_node(const key_element_type& element) {
		auto it = nlower_bound(element);

		if (it == nend() || it->first != element)
			it = _nodes.emplace(it, element, std::unique_ptr<Trie>(new Trie()));

		return it;
	}

	template<typename Iterator>
	data_type& insert_key(Iterator first, Iterator last) {
		if (first == last)
			return _data;

		auto it = insert_node(*first);

		for (++first; first != last; ++first) {
			it = it->second->insert_node(*first);
		}

		return it->second->_data;
	}

	void insert(const key_type& key, const data_type& data) {
		insert(std::begin(key), std::end(key), data);
	}

	template<typename Iterator>
	void insert(Iterator first, Iterator last, const data_type& data) {
		insert_key(first, last) = data;
	}


	// sets the data of every subsequence of key to func(data).
	template<typename Functor>
	void insert(const key_type& key, Functor func) {
		insert(std::begin(key), std::end(key), func);
	}

	template<typename Iterator, typename Functor>
	void insert(Iterator first, Iterator last, Functor func) {
		_data = func(_data);

		if (first != last) {
			auto it = insert_node(*first);
			it->second->_data = func(it->second->_data);

			for (++first; first != last; ++first) {
				it = it->second->insert_node(*first);
				it->second->_data = func(it->second->_data);
			}
		} // if
	}

	// returns a reference to the data corresponding to the provided key_sequence.
	// missing keys are inserted, in that case the data item is default constructed.
	data_type& operator[](const key_type& key) {
		return insert_key(std::begin(key), std::end(key));
	}

	// stl-style find() returns an iterator to the data that matches the provided key_sequence.
	// The iterator points to end() if the key_sequence is not found.
	iterator find(const key_type& key) const {
		return iterator(this, key);
	}

	// find out if sequence is in the Trie, match returns true if the entire sequence can be matched exactly
	// and false if only a prefix of the sequence can be matched. In that case it will always match the longest
	// prefix (which might be the empty sequence). 
	// In either case it will return the value corresponding to the matched (sub)sequence.
	std::pair<bool, Data> match(const key_type& key) {
		return match(std::begin(key), std::end(key));
	}

	template<typename Iterator>
	std::pair<bool, Data> match(Iterator first, Iterator last) {
		if (first == last)
			return std::make_pair(true, _data);

		auto it = nlower_bound(*first);

		if (it != _nodes.end() && it->first == *first)
			return it->second->match(++first, last);
		
		return std::make_pair(false, _data);
	}

	// Iterates depth-first over all key elements (i.e. nodes) in the trie.
	// The functor should take a key_element_type argument and a data_type argument.
	// It should return a bool indicating if keys of which the current key is a prefix should be traversed.
	// If the return value is false, the algorithm continues with the next value with the same or a lower depth.
	template<typename Functor>
	void each_elem(Functor func) {
		// if (func(_data))
		for (const auto& node : _nodes)
			node.second->each_elem(node.first, func);
	}

	template<typename Functor>
	void each_elem(key_element_type element, Functor func) {
		if (func(element, _data))
			for (const auto& node : _nodes)
				node.second->each_elem(node.first, func);
	}

	// Iterates over all (key, data) pairs in sorted order (i.e. traverses the trie depth-first in pre-order).
	// The functor should take a key_type argument (the subsequence up to the node that is visited) and a data_type argument.
	// It should return a bool indicating if keys of which the current key is a prefix should be traversed.
	// If the return value is false, the algorithm continues with the next value with the same or a lower depth.
	template<typename Functor>
	void each(Functor visitor) {
		key_type key;
		each(key, visitor);
	}

	template<typename Functor>
	void each(key_type& key, Functor visitor) {
		if (visitor(key, _data)) {

			for (const auto& node : _nodes) {
				key.push_back(node.first);
				node.second->each(key, visitor);
				key.pop_back();
			} // for
		} // if
	}

	iterator begin() { return iterator(this); }
	iterator end() { return iterator(this, _nodes.end()); }
}; // Trie

// compares the pattern with each key in the tree that is of equal length.
// comparison is done by calling the score_function on each element of the pattern.
// ScoreFunction should provide an init() function that gives the initial score and an operator() that calculates 
// a score based on previous value of a score, the current pattern element and a corresponding element in the trie.
// For each key in the trie that has the same length as pattern, The functor result is called with the calculated
// score and the data item corresponding to the key.
template<typename TrieT, typename Key, typename ScoreFunction, typename Result>
inline void compare(TrieT& trie, const Key& pattern, ScoreFunction score_function, Result result) {
	return compare(trie, std::begin(pattern), std::end(pattern), score_function, result);
}

template<typename TrieT, typename Iterator, typename ScoreFunction, typename Result>
inline void compare(TrieT& trie, Iterator first, Iterator last, ScoreFunction score_function, Result result) {
	auto score = score_function.init();

	trie.each_elem([=](const typename TrieT::key_element_type& element, const typename TrieT::data_type& data) mutable {

		score = score_function(score, element, *first++);

		if (first == last) {
			result(score, data);
			return false;
		}

		return true;
	});
}

#include "trie_iterator.hpp"

#endif
