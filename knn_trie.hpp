#ifndef __KNN_TRIE_HPP__
#define __KNN_TRIE_HPP__

#include <set>
#include <map>
#include <iostream>

#include "trie.hpp"
#include "score_function.hpp"

std::ostream& operator<<(std::ostream& out, const std::map<int, int>& m) {
	out << "[";
	for (auto v : m) {
		out << "{ " << v.first << " : " << v.second << " }";
	}
	out << "]";

	return out;
}

template<typename Key, typename Data>
std::ostream& operator<<(std::ostream& out, Trie<Key, Data>& trie) {
	trie.each([&out](const Key& key, const Data& data) {
		out << "{ ";
		for (auto key_elem : key) {
			out << key_elem << ' ';
		}
		out << "} : " << data << std::endl;

		return true;
	});
	return out;
}

// TODO:
// - Add weighted scoring

template <typename Features, typename Label = int>
class KNNTrie {
private:
	using LabelFrequencies = std::map<Label, int>;

	Trie<Features, LabelFrequencies> _trie;

	Label majority_vote(const LabelFrequencies& labels) {
		if (labels.empty()) 
			return Label();

		auto best = labels.begin();

		for (auto it = labels.begin(); it != labels.end(); ++it) {
			if (it->second > best->second) {
				best = it;
			}
		}

		return best->first;
	}
public:
	void learn(const Features& features, const Label& label) {
		++_trie[features][label];
	}

	Label classify(const Features& features) {
		typename OverlapScore::type score{ 0 };
		LabelFrequencies labels;

		compare(_trie, features, OverlapScore(), [&score, &labels](const typename OverlapScore::type& s, const LabelFrequencies& l) {
			if (s > score) {
				score = s;
				labels = l;
			}
		});

		return majority_vote(labels);
	}

	Label classify(const Features& features, int k) {
		class KBest {
		private:
			unsigned _k;
			std::set<std::pair<OverlapScore::type, LabelFrequencies>> _best;
		public:
			KBest(unsigned k) : _k(k) {}

			void store_if_better(const typename OverlapScore::type& s, const LabelFrequencies& l) {
				auto lowest = _best.begin();

				if (lowest == _best.end())
					_best.insert(std::make_pair(s, l));
				else {
					if (s > lowest->first)
						_best.insert(std::make_pair(s, l));

					if (_best.size() > _k)
						_best.erase(lowest);
				}
			}

			
			LabelFrequencies label_frequencies() {
				LabelFrequencies labels;

				for (auto& lf : _best) {
					std::cout << lf.first << " " << lf.second << std::endl;
					for (auto& label : lf.second) {
						labels[label.first] += label.second;
					}

				}

				return labels;
			}

		} k_best(k);

		compare(_trie, features, OverlapScore(), [&k_best](const typename OverlapScore::type& s, const LabelFrequencies& l) {
			k_best.store_if_better(s, l);
		});

		return majority_vote(k_best.label_frequencies());
	}

	friend std::ostream& operator<<(std::ostream& out, KNNTrie& knn) {
		return out << knn._trie;
	}
}; // KNNTrie

#endif // __KNN_TRIE_HPP__
