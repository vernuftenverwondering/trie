#ifndef __SCORE_FUNCTION_HPP__
#define __SCORE_FUNCTION_HPP__

class OverlapScore {
public:
	using type = int;

	type init() const { return 0; }

	template<typename T>
	type operator()(const T& lhs, const T& rhs) const {
		return lhs == rhs ? 1 : 0;
	}

	template<typename T>
	type operator()(const type& val, const T& lhs, const T& rhs) {
		return val + operator()(lhs, rhs);
	}
}; // OverlapScore

#endif // __SCORE_FUNCTION_HPP__
