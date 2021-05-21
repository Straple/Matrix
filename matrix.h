#pragma once

// TODO с помощью наследования написать basic_iterator, 
// который будет содержать в себе много вещей из структур итераторов
// чтобы убрать эту копипасту в 300 строк

#include <iostream>
#include <tuple>

#include "eassert.h"
#include "container_traits.h"

// SFINAE verify operator in template
#define verify_operator(T1, oop, T2) typename = decltype(std::declval<T1&>() oop std::declval<const T2&>())

template<typename T>
class matrix {

	size_t row_sz, col_sz;
	T* memory = nullptr;

	// копирует память из source в data
	// матрица должна быть инициализирована!
	void copy_memory(const T* source) {
		size_t n = size();

		if constexpr (std::is_arithmetic_v<T>) {
			std::memcpy(memory, source, n * sizeof(T)); // more faster
		}
		else {
			for (size_t i = 0; i < n; i++) {
				memory[i] = source[i];
			}
		}
	}


	// "корректно" => значит, что можно не волноваться о том, где и как ее вызвать

	// корректно создает матрицу row_size x col_size
	// НЕ инициализирует ее
	void allocate_memory(size_t row_size, size_t col_size) {
		delete[] memory;

		row_sz = row_size;
		col_sz = col_size;

		memory = new T[size()];
	}

	// корректно копирует матрицу
	void copy_matrix(const matrix& source) {
		if (this != &source) {
			allocate_memory(source.row_sz, source.col_sz);
			copy_memory(source.memory);
		}
	}

	// корректно перемещает матрицу
	void move_matrix(matrix& source) {
		if (this != &source) {
			delete[] memory;

			row_sz = source.row_sz;
			col_sz = source.col_sz;
			memory = source.memory;

			source.memory = nullptr;
			source.row_sz = source.col_sz = 0;
		}
	}

public:

	//=========================//
	//=======USING TYPES=======//
	//=========================//

	using value_type = T;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;
	using size_type = size_t;
	using difference_type = ptrdiff_t;


	//=========================//
	//==STANDART CONSTRUCTORS==//
	//=========================//

	matrix() {
		row_sz = col_sz = 0;
	}
	matrix(const matrix& source) {
		copy_matrix(source);
	}
	matrix(matrix&& source) noexcept {
		move_matrix(source);
	}
	~matrix() {
		delete[] memory;
	}

	matrix& operator = (const matrix& source) {
		copy_matrix(source);
		return *this;
	}
	matrix& operator = (matrix&& source) noexcept {
		move_matrix(source);
		return *this;
	}

	//=========================//
	//===OTHERS CONSTRUCTORS===//
	//=========================//

	matrix(size_t row_size, size_t col_size, const T& fill_value = T()) {
		reshape(row_size, col_size, fill_value);
	}
	matrix(const std::tuple<size_t, size_t>& shape, const T& fill_value = T()) {
		reshape(shape, fill_value);
	}

	matrix(size_t row_size, size_t col_size, const T* source) {
		allocate_memory(row_size, col_size);
		copy_memory(source);
	}
	matrix(const std::tuple<size_t, size_t>& shape, const T* source) {
		allocate_memory(std::get<0>(shape), std::get<1>(shape));
		copy_memory(source);
	}

	template<typename container_t,
		typename = decltype(
			is_container<typename container_t>::value,
			is_container<typename container_t::value_type>::value
		)>
	matrix(const container_t& container2d) {

		allocate_memory(container2d.size(), !container2d.empty() ? container2d.begin()->size() : 0);

		size_t i = 0;
		for (const auto& row_container : container2d) {
			EASSERT(col_sz == row_container.size(), "this is not a matrix in matrix(const container_t& container2d)");

			for (const auto& col_container : row_container) {
				memory[i] = static_cast<T>(col_container);
				i++;
			}
		}
	}

	template<typename container_t, typename = decltype(is_container<typename container_t>::value/*, std::is_same_v<T, container_t::value_type>*/)>
	matrix(const container_t& container, size_t row_size, size_t col_size) {
		EASSERT(row_size * col_size == container.size(), "incorrect container size in matrix(col_size, row_size, const container_t& container)");

		allocate_memory(row_size, col_size);

		size_t i = 0;
		for (const auto& it : container) {
			memory[i] = static_cast<T>(it);
			i++;
		}
	}

	//=========================//
	//===CONTAINER FUNCTIONS===//
	//=========================//

	void reshape(size_t row_size, size_t col_size, const T& fill_value = T()) {
		allocate_memory(row_size, col_size);

		size_t n = size();

		if constexpr (std::is_arithmetic_v<T>) {
			std::fill_n(memory, n, fill_value); // more faster
		}
		else {
			for (size_t i = 0; i < n; i++) {
				memory[i] = fill_value;
			}
		}
	}
	void reshape(const std::tuple<size_t, size_t>& shape, const T& fill_value = T()) {
		reshape(std::get<0>(shape), std::get<1>(shape), fill_value);
	}

	void clear() {
		delete[] memory;
		memory = nullptr;
		row_sz = col_sz = 0;
	}

	size_t size() const {
		return row_sz * col_sz;
	}
	size_t row_size() const {
		return row_sz;
	}
	size_t col_size() const {
		return col_sz;
	}
	std::tuple<size_t, size_t> shape() const {
		return std::make_tuple(row_sz, col_sz);
	}
	bool empty() const {
		return row_sz == 0 || col_sz == 0;
	}

	T* operator [](size_t row) {
		EASSERT(row < row_sz, "incorrect row index in operator []");
		return memory + row * col_sz;
	}
	const T* operator [](size_t row) const {
		EASSERT(row < row_sz, "incorrect row index in operator []");
		return memory + row * col_sz;
	}

	T& at(size_t row, size_t col) {
		ERROR_REPORT(row < row_sz && col < col_sz, "incorrect row or col index in at");

		return memory[row * col_sz + col];
	}
	const T& at(size_t row, size_t col) const {
		ERROR_REPORT(row < row_sz && col < col_sz, "incorrect row or col index in at");

		return memory[row * col_sz + col];
	}

	T* data() {
		return memory;
	}
	const T* data() const {
		return memory;
	}

	//=========================//
	//=====MATRIX FUNCTIONS====//
	//=========================//

	// проходит по всем элементам матрицы и применяет fn
	// fn должна возвращать void
	template<typename func_t>
	void aggregate(func_t&& fn) {
		size_t n = size();
		for (size_t i = 0; i < n; i++) {
			fn(memory[i]);
		}
	}
	// проходит по всем элементам матрицы и применяет fn
	// fn должна возвращать void
	template<typename func_t>
	void aggregate(func_t&& fn) const {
		size_t n = size();
		for (size_t i = 0; i < n; i++) {
			fn(memory[i]);
		}
	}

	// применяет fn для данной матрицы. второй переменной будет элемент из m
	template<typename func_t>
	void aggregate(const matrix& m, func_t&& fn) {
		EASSERT(shape() == m.shape(), "incorrect matrix shape in aggregate");

		size_t n = size();
		for (size_t i = 0; i < n; i++) {
			fn(memory[i], m.memory[i]);
		}
	}
	// применяет fn для данной матрицы. второй переменной будет элемент из m
	template<typename func_t>
	void aggregate(const matrix& m, func_t&& fn) const {
		EASSERT(shape() == m.shape(), "incorrect matrix shape in aggregate");

		size_t n = size();
		for (size_t i = 0; i < n; i++) {
			fn(memory[i], m.memory[i]);
		}
	}


	template<verify_operator(T, +=, T)>
	T sum() const {
		T res = T();
		aggregate([&res](const T& value) {
			res += value;
		});
		return res;
	}

	template<verify_operator(T, +=, T), verify_operator(T, /=, T)>
	T mean() const {
		// переполнения size_t произойти не может, потому что мы бы не смогли выделить столько места

		T res = T();

		if (!empty()) {
			res = sum();
			res /= static_cast<T>(size());
		}
		return res;
	}


	//=========================//
	//=====MATRIX TRANSFORM====//
	//=========================//

	// транспонирование матрицы
	// вернет матрицу, в которой [i][j] => [j][i]
	// плохой кеш. ускорить невозможно
	matrix transpose() const {
		matrix result(col_sz, row_sz);

		size_t n = size();

		size_t res_i = 0;
		for (size_t i = 0; i < n; i++) {

			result.memory[res_i] = memory[i];

			res_i += row_sz;
			if (res_i >= n) {
				res_i -= n;
				res_i++;
			}
		}

		return result;
	}

	// returns matrix in rect [left, right) and [top, bottom)
	matrix slice(const size_t left, const size_t top, const size_t right, const size_t bottom) const {

		EASSERT(left <= right && top <= bottom, "incorrect rect in slice");

		EASSERT(right <= col_sz && bottom <= row_sz, "incorrect rect position in slice");

		matrix result(bottom - top, right - left);

		for (size_t i = top; i < bottom; i++) {

			if constexpr (std::is_arithmetic_v<T>) {
				std::memcpy(result[i - top], (*this)[i] + left, (right - left) * sizeof(T));
			}
			else {
				for (size_t j = left; j < right; j++) {
					result[i - top][j - left] = (*this)[i][j];
				}
			}
		}

		return result;
	}

	// split matrix on gorizontal
	// returns pair( <= row, > row )
	// very fast
	std::pair<matrix, matrix> gorizontal_split(const size_t row) const {
		EASSERT(row < row_sz, "incorrect row in gorizontal_split");

		return std::make_pair(
			matrix(row + 1, col_sz, memory), // up
			matrix(row_sz - (row + 1), col_sz, memory + (row + 1) * col_sz) // down
		);
	}

	// split matrix on vertical
	// returns pair( <= col, > col )
	std::pair<matrix, matrix> vertical_split(const size_t col) const {
		EASSERT(col < col_sz, "incorrect col in vertical_split");

		// красивое, но очень медленное решение в ~16 раз
		// из-за плохого кеша в транспонировании матриц
		/*
		matrix this_transpose = transpose();

		auto result = this_transpose.gorizontal_split(col);

		return std::make_pair(result.first.transpose(), result.second.transpose());
		*/

		matrix left, right;
		left.allocate_memory(row_sz, col + 1); 
		right.allocate_memory(row_sz, col_sz - (col + 1));

		for (size_t i = 0; i < row_sz; i++) {

			if constexpr (std::is_arithmetic_v<T>) {
				std::memcpy(left[i], (*this)[i], (col + 1) * sizeof(T));

				std::memcpy(right[i], (*this)[i] + col + 1, (col_sz - col) * sizeof(T));
			}
			else {
				size_t j = 0;
				const T* row_ptr = (*this)[i];

				for (; j <= col; j++) {
					left[i][j] = row_ptr[j];
				}

				for (; j < col_sz; j++) {
					right[i][j - col - 1] = row_ptr[j];
				}
			}
		}

		return std::make_pair(left, right);
	}

	//=========================//
	//=====MATRIX OPERATORS====//
	//=========================//

	template<verify_operator(T, +=, T)>
	matrix& operator += (const matrix& add) {
		aggregate(add, [](T& value, const T& add) {
			value += add;
		});
		return *this;
	}
	template<verify_operator(T, +=, T)>
	matrix operator + (const matrix& add) const {
		return matrix(*this) += add;
	}

	template<verify_operator(T, -=, T)>
	matrix& operator -= (const matrix& sub) {
		aggregate(sub, [](T& value, const T& sub) {
			value -= sub;
		});
		return *this;
	}
	template<verify_operator(T, -=, T)>
	matrix operator - (const matrix& sub) const {
		return matrix(*this) -= sub;
	}

	template<verify_operator(T, +=, T), verify_operator(T, *, T)>
	matrix& operator *= (const matrix& mult) {
		return *this = *this * mult;
	}
	template<verify_operator(T, +=, T), verify_operator(T, *, T)>
	matrix operator * (const matrix& mult) const {
		EASSERT(col_sz == mult.row_sz, "incorrect matrix shape in multiple");

		matrix result(row_sz, mult.col_sz);

		for (size_t i = 0; i < row_sz; i++) {
			for (size_t k = 0; k < col_sz; k++) {
				for (size_t j = 0; j < mult.col_sz; j++) {

					result[i][j] += (*this)[i][k] * mult[k][j];
				}
			}
		}

		return result;
	}

	//=========================//
	//=====MATRIX ITERATORS====//
	//=========================//

	
	struct iterator {

		using value_type = T;
		using pointer = T*;
		using reference = T&;
		using difference_type = ptrdiff_t;

	private:

		pointer data;
		size_t row_sz, col_sz;

		size_t index;

	public:

		iterator() {
			data = nullptr;
			row_sz = col_sz = 0;
			index = 0;
		}

		iterator(matrix& source, bool is_begin) {
			data = source.data();
			row_sz = source.row_size();
			col_sz = source.col_size();

			index = is_begin ? 0 : source.size();
		}

		difference_type operator - (const iterator& Rhs) const {
			ERROR_REPORT(data == Rhs.data, "different objects in iterators");
			return static_cast<difference_type>(index) - Rhs.index;
		}

		reference operator * () const {
			ERROR_REPORT(data, "iterator is not initialize or matrix is empty");

			ERROR_REPORT(index < row_sz * col_sz, "out of data in matrix::iterator::operator *");
			return data[index];
		}
		pointer operator -> () const {
			return &**this;
		}

		iterator operator ++ (int) {
			auto temp = *this;
			index++;
			return temp;
		}
		iterator& operator ++ () {
			index++;
			return *this;
		}

		iterator operator -- (int) {
			auto temp = *this;
			index--;
			return temp;
		}
		iterator& operator -- () {
			index--;
			return *this;
		}

		iterator operator + (difference_type Off) const {
			return iterator(*this) += Off;
		}
		iterator& operator += (difference_type Off) {
			index += Off;

			return *this;
		}

		iterator operator - (difference_type Off) const {
			return iterator(*this) -= Off;
		}
		iterator& operator -= (difference_type Off) {
			this->operator+=(-Off);
			return *this;
		}

		size_t get_row() const {
			return index / col_sz;
		}
		size_t get_col() const {
			return index % col_sz;
		}

		bool operator == (const iterator& Rhs) const {
			return data == Rhs.data && row_sz == col_sz && index == Rhs.index;
		}

		bool operator != (const iterator& Rhs) const {
			return !(*this == Rhs);
		}
	};

	struct const_iterator {

		using value_type = T;
		using pointer = const T*;
		using reference = const T&;
		using difference_type = ptrdiff_t;

	private:

		pointer data;
		size_t row_sz, col_sz;

		size_t index;

	public:

		const_iterator() {
			data = nullptr;
			row_sz = col_sz = 0;
			index = 0;
		}

		const_iterator(const matrix& source, bool is_begin) {
			data = source.data();
			row_sz = source.row_size();
			col_sz = source.col_size();

			index = is_begin ? 0 : source.size();
		}

		difference_type operator - (const const_iterator& Rhs) const {
			ERROR_REPORT(data == Rhs.data, "different objects in iterators");
			return static_cast<difference_type>(index) - Rhs.index;
		}

		reference operator * () const {
			ERROR_REPORT(data, "iterator is not initialize or matrix is empty");

			ERROR_REPORT(index < row_sz* col_sz, "out of data in matrix::const_iterator::operator *");
			
			return data[index];
		}
		pointer operator -> () const {
			return &**this;
		}

		const_iterator operator ++ (int) {
			auto temp = *this;
			index++;
			return temp;
		}
		const_iterator& operator ++ () {
			index++;
			return *this;
		}

		const_iterator operator -- (int) {
			auto temp = *this;
			index--;
			return temp;
		}
		const_iterator& operator -- () {
			index--;
			return *this;
		}

		const_iterator operator + (difference_type Off) const {
			return const_iterator(*this) += Off;
		}
		const_iterator& operator += (difference_type Off) {
			index += Off;

			return *this;
		}

		const_iterator operator - (difference_type Off) const {
			return const_iterator(*this) -= Off;
		}
		const_iterator& operator -= (difference_type Off) {
			this->operator+=(-Off);
			return *this;
		}

		size_t get_row() const {
			return index / col_sz;
		}
		size_t get_col() const {
			return index % col_sz;
		}

		bool operator == (const const_iterator& Rhs) const {
			return data == Rhs.data && row_sz == col_sz && index == Rhs.index;
		}

		bool operator != (const const_iterator& Rhs) const {
			return !(*this == Rhs);
		}
	};

	struct reverse_iterator {

		using value_type = T;
		using pointer = T*;
		using reference = T&;
		using difference_type = ptrdiff_t;

	private:

		pointer data;
		size_t row_sz, col_sz;

		size_t index;

	public:

		reverse_iterator() {
			data = nullptr;
			row_sz = col_sz = 0;
			index = 0;
		}

		reverse_iterator(matrix& source, bool is_begin) {
			data = source.data() + source.size() - 1;
			row_sz = source.row_size();
			col_sz = source.col_size();

			index = is_begin ? 0 : source.size();
		}

		difference_type operator - (const reverse_iterator& Rhs) const {
			ERROR_REPORT(data == Rhs.data, "different objects in iterators");
			return static_cast<difference_type>(index) - Rhs.index;
		}

		reference operator * () {
			ERROR_REPORT(data, "iterator is not initialize or matrix is empty");

			ERROR_REPORT(index < row_sz* col_sz, "out of data in matrix::reverse_iterator::operator *");
			return *(data - index);
		}
		pointer operator -> () {
			return &**this;
		}

		reverse_iterator operator ++ (int) {
			auto temp = *this;
			index++;
			return temp;
		}
		reverse_iterator& operator ++ () {
			index++;
			return *this;
		}

		reverse_iterator operator -- (int) {
			auto temp = *this;
			index--;
			return temp;
		}
		reverse_iterator& operator -- () {
			index--;
			return *this;
		}

		reverse_iterator operator + (difference_type Off) const {
			return reverse_iterator(*this) += Off;
		}
		reverse_iterator& operator += (difference_type Off) {
			index += Off;

			return *this;
		}

		reverse_iterator operator - (difference_type Off) const {
			return reverse_iterator(*this) -= Off;
		}
		reverse_iterator& operator -= (difference_type Off) {
			this->operator+=(-Off);
			return *this;
		}

		size_t get_row() const {
			return index / col_sz;
		}
		size_t get_col() const {
			return index % col_sz;
		}

		bool operator == (const reverse_iterator& Rhs) const {
			return data == Rhs.data && row_sz == col_sz && index == Rhs.index;
		}

		bool operator != (const reverse_iterator& Rhs) const {
			return !(*this == Rhs);
		}
	};

	iterator begin() {
		return iterator(*this, true);
	}
	iterator end() {
		return iterator(*this, false);
	}

	const_iterator cbegin() const {
		return const_iterator(*this, true);
	}
	const_iterator cend() const {
		return const_iterator(*this, false);
	}

	reverse_iterator rbegin() {
		return reverse_iterator(*this, true);
	}
	reverse_iterator rend() {
		return reverse_iterator(*this, false);
	}
};

template<typename T, verify_operator(std::ostream, << , T)>
std::ostream& operator << (std::ostream& output, const matrix<T>& matrix) {
	output << matrix.row_size() << " " << matrix.col_size() << "\n";

	for (size_t i = 0; i < matrix.row_size(); i++) {
		for (size_t j = 0; j < matrix.col_size(); j++) {
			output << matrix[i][j] << " ";
		}
		output << "\n";
	}
	return output;
}

// result_ij = fn(A_ij)
template<typename T, typename func_t>
matrix<T> aggregate(const matrix<T>& A, func_t&& fn) {
	matrix<T> result(A.shape());

	size_t n = A.size();
	for (size_t i = 0; i < n; i++) {
		result.data()[i] = fn(A.data()[i]);
	}
	return result;
}

// result_ij = fn(A_ij, B_ij)
template<typename T, typename func_t>
matrix<T> aggregate(const matrix<T>& A, const matrix<T>& B, func_t&& fn) {
	EASSERT(A.shape() == B.shape(), "incorrect matrix shape in aggregate");

	matrix<T> result(A.shape());

	size_t n = A.size();
	for (size_t i = 0; i < n; i++) {
		result.data()[i] = fn(A.data()[i], B.data()[i]);
	}
	return result;
}

#undef verify_operator
