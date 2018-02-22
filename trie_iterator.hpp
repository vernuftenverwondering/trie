#ifndef __TRIE_ITERATOR_HPP__
#define __TRIE_ITERATOR_HPP__

#include <deque>

// A depth first pre-order iterator for Trie
//
// Design issues:
// 1. insert() is a somewhat odd function for an iterator, integrate with constructor or remove
// 2. Add move constructor to TrieProxy
// 3. Add const iterators


template<typename Key, typename Data>
class TrieProxy {
private:
	using value_type = std::pair<Key, Data&>;

	value_type _value;
public:
	TrieProxy(const Key& key, Data& data) : _value(value_type(key, data)) {}
	
	value_type* operator->() {
		return &_value;
	}
}; // TrieProxy


template<typename Trie>
class TrieIterator {
private:
	Trie* _root;

	using node_iterator = typename Trie::node_iterator;
	
	class NodeIteratorStack {
	private:
		using Stack = std::deque<node_iterator>;

		Stack _stack;
	public:
		using const_iterator = typename  Stack::const_iterator;

		node_iterator pop() {
			auto it = _stack.back();
			_stack.pop_back();
			return it;
		}

		node_iterator& top() {
			return _stack.back();
		}

		const node_iterator& top() const {
			return _stack.back();
		}

		Trie& top_node() {
			return *top()->second;
		}

		void push(node_iterator it) {
			_stack.push_back(it);
		}

		void clear() { _stack.clear(); }

		bool empty() const { return _stack.empty(); }
		size_t size() const { return _stack.size(); }

		const_iterator begin() const { return _stack.begin(); }
		const_iterator end() const { return _stack.end(); }

		friend bool operator==(const NodeIteratorStack& lhs, const NodeIteratorStack& rhs) { return lhs._stack == rhs._stack; }
		friend bool operator!=(const NodeIteratorStack& lhs, const NodeIteratorStack& rhs) { return lhs._stack != rhs._stack; }
	} _path;

public:
	using key_type = typename Trie::key_type;
	using data_type = typename Trie::data_type;
	using value_type = typename Trie::value_type;

	TrieIterator() : _root(nullptr) {}

	TrieIterator(Trie* trie) : _root(trie) {}

	TrieIterator(Trie* trie, typename Trie::node_iterator end_it) : _root(trie) {
		_path.push(end_it);
	}

	TrieIterator(Trie* trie, const key_type& key) : root(trie) {

		auto it = key.begin();

		auto node_it = _root->nfind(*it);

		if (node_it != _root->nend() && node_it->first == *it) {
			_path.push(node_it);

			for (++it; it != key.end(); ++it) {
				auto node_it = _path.top_node().nfind(*it);

				if (node_it != _path.top_node().nend() && node_it == *it)
					_path.push(node_it);
				else {
					to_end();
					break;
				}

			} // for
		} // if
	}

	template<typename Key, typename Data>
	void insert(const Key& key, const Data& data) {
		auto it = key.begin();

		_path.push(_root->insert_node(*it));

		for (; it != key.end(); ++it) {
			_path.push(top_node().insert_node(*it));
		}

		_path.top_node().data() = data;
	}

	void to_end() {
		_path.clear();
		_path.push(_root->nend());
	}

	size_t depth() const {
		return _path.size();
	}

	bool at_begin() const {
		return _path.empty();
	}

	bool at_end() const {
		return _path.size() == 1 && _path.top() == _root->nend();
	}

	key_type key() const {
		key_type key;

		for (auto it : _path)
			key.push_back(it->first);

		return key;
	}

	data_type& data() {
		if (at_begin())
			return _root->data();

		return _path.top_node().data();
	}

	data_type data() const {
		if (at_begin())
			return _root->data();

		return _path.top_node().data();
	}

	TrieIterator& operator++() {
		if (at_begin())
			_path.push(_root->nbegin());
		else {
			if (!at_end()) {
				if (_path.top_node().is_leaf()) {
					auto it = _path.pop();
					++it;

					while (!_path.empty() && it == _path.top_node().nend()) {
						it = _path.pop();
						++it;
					}

					_path.push(it);
				}
				else {
					_path.push(_path.top_node().nbegin());
				} // else
			} // if
		} // else

		return *this;
	}

	TrieIterator& operator--() {
		if (!at_begin()) {
			auto it = _path.pop();

			if ((_path.empty() && it != _root->nbegin()) || 
				(!_path.empty() && it != _path.top_node().nbegin())) {

				--it;
				_path.push(it);

				while (!it->second->is_leaf()) {
					it = it->second->nend();
					--it;
					_path.push(it);
				}
			}
		}

		return *this;
	}


	value_type operator*() const {
		return std::make_pair(key_sequence(), data());
	}


	std::pair<key_type, data_type&> operator*() {
		return std::pair<key_type, data_type&>(key(), data());
	}

	TrieProxy<key_type, data_type> operator->() {
		return TrieProxy<key_type, data_type>(key(), data());
	}
	
	friend bool operator != (const TrieIterator<Trie>& lhs, const TrieIterator<Trie>& rhs) {
		return lhs._root != rhs._root || lhs._path != rhs._path;
	}

	friend bool operator == (const TrieIterator<Trie>& lhs, const TrieIterator<Trie>& rhs) {
		return lhs._root == rhs._root && lhs._path == rhs._path;
	}
}; // TrieIterator

#endif // __TRIE_ITERATOR_HPP__
