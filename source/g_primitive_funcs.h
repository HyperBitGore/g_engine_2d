#pragma once
#include <iostream>
#include <vector>
#include <functional>

//add stack 
//add queue
//add sorting algorithms
//add tree structures

namespace Gore {



	
	template<class T>
	class Vector {
	public:
		using iterator = T*;
		iterator begin() { return stor; }
		iterator end() { return stor + offset; }
	private:
		T* stor;
		size_t offset;
		size_t byte_size;
		size_t allocd;
	public:
		Vector() {
			stor = (T*)std::malloc(sizeof(T));
			offset = 0;
			byte_size = 0;
			allocd = sizeof(T);
		}
		//copy constructor for returns
		Vector(const Vector& in) {
			stor = (T*)std::malloc(sizeof(T));
			allocd = (sizeof(T));
			reserve(in.offset);
			std::memcpy(stor, in.stor, in.byte_size);
			byte_size = in.byte_size;
			offset = in.offset;
		}
		//https://en.cppreference.com/w/cpp/language/move_constructor

		void push_back(T in) {
			if (byte_size + sizeof(T) > allocd) {
				//we realloc double the amount of data to reduce realloc calls
				//allocd = allocd << 1;
				stor = (T*)std::realloc(stor, allocd * 2);
				allocd *= 2;
			}
			stor[offset] = in;
			offset++;
			byte_size += sizeof(T);
		}
		void erase(int n) {
			//move entire chunk back by one and offset is reduced by one 
			std::memcpy(stor + n, stor + n + 1, (offset - n) * sizeof(T));
			offset--;
			byte_size -= sizeof(T);
		}
		size_t size() {
			return offset;
		}
		void reserve(size_t n) {
			T* temp = (T*)std::realloc(stor, allocd + (sizeof(T) * n));
			if (temp != NULL) {
				stor = temp;
			}
			allocd += sizeof(T) * n;
		}
		void pop_back() {
			//don't need to touch data will just be copied over next push_back
			byte_size -= sizeof(T);
			offset--;
		}
		void insert(int n, T in) {
			//checking if enough space to memcpy over, branchless? 
			if (byte_size + sizeof(T) > allocd) {
				T* temp = (T*)std::realloc(stor, allocd + sizeof(T));
				if (temp != NULL) {
					stor = temp;
				}
				allocd += sizeof(T);
			}
			//copy current data over by 1 offset
			std::memcpy(stor + n + 1, stor + n, (offset - n) * sizeof(T));
			*(stor + n) = in;
			byte_size += sizeof(T);
			offset++;
		}
		void clear() {
			//just reset entire block of data, without deallocating it
			std::memset(stor, 0, offset * sizeof(T));
			offset = 0;
			byte_size = 0;
		}
		T& operator[] (int n) {
			return stor[n];
		}

		Gore::Vector<T> operator= (Gore::Vector<T> in) {
			Gore::Vector<T> nc;
			nc.reserve(in.size());
			std::memcpy(nc.stor, in.stor, in.byte_size);
			nc.byte_size = in.byte_size;
			nc.offset = in.size();
			return nc;
		}

		~Vector() {
			if (stor) {
				std::free(stor);
			}
		}
	};


	//use offset within data instead to avoid the pointer being made useless if realloc moves chunk
	struct StoreElement {
		size_t offset;
		size_t size;
		size_t type;
	};
	//returns char pointer which you convert to the struct you want
	struct ReturnElement {
		char* data;
		size_t type;
	};
	//keep track of your own types, I don't feel like writing a reflection system
	//could probably optimize this pretty hard
	class MultiVector {
	private:
		//stores the location of data and its size
		std::vector<StoreElement> elements;
		//stores gaps made by erase
		std::vector<StoreElement> gaps;
		//where to write in the data
		char* index;
		//the number of bytes written
		size_t size;
		//the number of bytes allocated
		size_t allocd;
		//the actual pointer to the data
		char* stor;
	public:
		MultiVector() {
			stor = (char*)std::malloc(32);
			index = stor;
			size = 0;
			allocd = 32;
		}
		void push_back(char* data, size_t insize, size_t type) {
			size_t off = size;
			//checking if item fits into any of the gaps
			for (int i = 0; i < (int)gaps.size(); i++) {
				if (gaps[i].size <= insize) {
					char* t = stor + gaps[i].offset;
					off = gaps[i].offset;
					for (size_t j = 0; j < gaps[i].size; j++) {
						*t = data[j];
						t++;
					}
					StoreElement st = { off, insize, type };
					elements.push_back(st);
					gaps.erase(gaps.begin() + i);
					return;
				}
			}
			if (size + insize > allocd) {
				char* temp = (char*)std::realloc(stor, allocd + insize);
				if (temp != NULL) {
					stor = temp;
					index = stor;
					//moving index back to where it was before we reallocd the memory
					for (int i = 0; i < (int)size; i++) {
						index++;
					}
				}
				allocd += insize;
			}
			for (size_t i = 0; i < insize; i++) {
				*index = data[i];
				index++;
				size++;
			}
			StoreElement st = { off, insize, type };
			elements.push_back(st);
		}
		void erase(size_t n) {
			gaps.push_back(elements[n]);
			elements.erase(elements.begin() + n);
		}
		void pop_back() {
			size -= elements[elements.size() - 1].size;
			index -= elements[elements.size() - 1].size;
			elements.erase(elements.begin() + (elements.size() - 1));
		}
		void reserve(size_t in) {
			char* temp = (char*)std::realloc(stor, allocd + in);
			if (temp != NULL) {
				stor = temp;
				index = stor;
				for (int i = 0; i < (int)size; i++) {
					index++;
				}
			}
			allocd += in;
		}
		void insert(int n, char* data, size_t insize, size_t type) {
			size_t off = size;
			//checking if item fits into any of the gaps
			for (int i = 0; i < (int)gaps.size(); i++) {
				if (gaps[i].size <= insize) {
					char* t = stor + gaps[i].offset;
					off = gaps[i].offset;
					for (size_t j = 0; j < gaps[i].size; j++) {
						*t = data[j];
						t++;
					}
					StoreElement st = { off, insize, type };
					elements.insert(elements.begin() + n, st);
					gaps.erase(gaps.begin() + i);
					return;
				}
			}
			if (size + insize > allocd) {
				char* temp = (char*)std::realloc(stor, allocd + insize);
				if (temp != NULL) {
					stor = temp;
					index = stor;
					//moving index back to where it was before we reallocd the memory
					for (int i = 0; i < (int)size; i++) {
						index++;
					}
				}
				allocd += insize;
			}
			for (size_t i = 0; i < insize; i++) {
				*index = data[i];
				index++;
				size++;
			}
			elements.insert(elements.begin() + n, {off, insize, type});
			size += insize;
			index += insize;
		}
		void clear() {
			elements.clear();
			size = 0;
			index = stor;
		}

		size_t getSize() {
			return elements.size();
		}

		ReturnElement operator[](int n) {
			return { stor + elements[n].offset, elements[n].type };
		}

		const ReturnElement operator[](int n) const {
			return{ stor + elements[n].offset, elements[n].type };
		}

	};
	
	template<class F>
	struct FObj {
		F* current;
		FObj<F>* next;
		std::string name;
	};
	template<class W> 
	class ForwardList {
	private:
		FObj<W>* object = nullptr;
	public:
		//will have problem with remove function if pointer isn't created with new
		void insert(W* item, std::string name) {
			FObj<W>* nt = new FObj<W>;
			nt->name = name;
			nt->current = item;
			nt->next = object;
			object = nt;
		}
		void insert(W item, std::string name) {
			FObj<W>* nt = new FObj<W>;
			W* it = new W;
			*it = item;
			nt->name = name;
			nt->current = it;
			nt->next = object;
			object = nt;
		}
		W* search(std::string name) {
			FObj<W>* ptr = object;
			while (ptr != nullptr) {
				if (ptr->name.compare(name.c_str()) == 0) {
					return ptr->current;
				}
				ptr = ptr->next;
			}
			return nullptr;
		}
		void removeFObj(std::string name) {
			FObj<W>* ptr = object;
			FObj<W>* prev = ptr;
			int c = 0;
			while (ptr != nullptr) {
				if (ptr->name.compare(name.c_str()) == 0) {
					prev->next = ptr->next;
					ptr->name.clear();
					if (c == 0) {
						object = ptr->next;
					}
					delete ptr;
					return;
				}
				prev = ptr;
				ptr = ptr->next;
				c++;
			}
		}
		void removeBoth(std::string name) {
			FObj<W>* ptr = object;
			FObj<W>* prev = ptr;
			int c = 0;
			while (ptr != nullptr) {
				if (ptr->name.compare(name.c_str()) == 0) {
					prev->next = ptr->next;
					ptr->name.clear();
					if (c == 0) {
						object = ptr->next;
					}
					delete ptr->current;
					delete ptr;
					return;
				}
				prev = ptr;
				ptr = ptr->next;
				c++;
			}
		}
		FObj<W>* getHead() {
			return object;
		}
	};
	template<typename F>
	struct FBObj {
		F* current;
		FBObj<F>* prev;
		FBObj<F>* next;
		std::string name;
	};
	//forwards and backwards linked list
	template<class T>
	class FBList {
	public:
		FBObj<T>* obj;
		FBList() {
			obj = nullptr;
		}
		void insert(T in_obj, std::string name) {
			FBObj<T>* nt = new FBObj<T>;
			nt->current = new T;
			*nt->current = in_obj;
			nt->name = name;
			nt->next = obj;
			nt->prev = nullptr;
			(obj != nullptr) ? obj->prev = nt : nt;
			obj = nt;
		}
		void insert(T* in_obj, std::string name) {
			FBObj<T>* nt = new FBObj<T>;
			nt->current = in_obj;
			nt->name = name;
			nt->next = obj;
			nt->prev = nullptr;
			(obj != nullptr) ? obj->prev = nt : nt;
			obj = nt;
		}
		T* search(std::string name) {
			FBObj<T>* ptr = obj;
			while (ptr != nullptr) {
				if (name.compare(ptr->name) == 0) {
					return ptr->current;
				}
				ptr = ptr->next;
			}
			return nullptr;
		}
		void removeBoth(std::string name) {
			FBObj<T>* ptr = obj;
			while (ptr != nullptr) {
				if (name.compare(ptr->name) == 0) {
					FBObj<T>* t1 = ptr->prev;
					FBObj<T>* t2 = ptr->next;
					(t1 != nullptr) ? t1->next = t2 : obj = t2;
					(t2 != nullptr) ? t2->prev = t1 : t2;
					ptr->name.clear();
					delete ptr->current;
					delete ptr;
					return;
				}
				ptr = ptr->next;
			}
		}
		void removeLinkObj(std::string name) {
			FBObj<T>* ptr = obj;
			while (ptr != nullptr) {
				if (name.compare(ptr->name) == 0) {
					FBObj<T>* t1 = ptr->prev;
					FBObj<T>* t2 = ptr->next;
					(t1 != nullptr) ? t1->next = t2 : obj = t2;
					(t2 != nullptr) ? t2->prev = t1 : t2;
					ptr->name.clear();
					delete ptr;
					return;
				}
				ptr = ptr->next;
			}
		}
		FBObj<T>* getHead() {
			return obj;
		}
	};


	template<typename T, typename G>
	struct MapItem {
		G key;
		T item;
		MapItem<T, G>* next;
	};
	//gonna be more inefficent than a specially designed version
	template<typename T, typename G>
	class HashMap {
	private:
		std::vector<MapItem<T, G>*> buckets;
		std::function<int(G)> hash_func;
	public:
		HashMap() {

		}
		~HashMap() {
			//actually need to clean up news here tf
			for (size_t i = 0; i < buckets.size(); i++) {
				if (buckets[i] != nullptr) {
					MapItem<T, G>* cur = buckets[i];
					while (cur != nullptr) {
						MapItem<T, G>* next = cur->next;
						delete cur;
						cur = next;
					}
				}
			}
			buckets.clear();
		}
		void insert(G f, T item) {
			int n = hash_func(f);
			if (n > int(buckets.size()) - 1) {
				int dif = n - (buckets.size() - 1);
				for (dif; dif > 0; dif--) {
					buckets.push_back(nullptr);
				}
			}
			MapItem<T, G>* temp = buckets[n];
			buckets[n] = new MapItem<T, G>;
			buckets[n]->key = f;
			buckets[n]->item = item;
			buckets[n]->next = temp;

		}
		T* get(G f) {
			int n = hash_func(f);
			if (n > int(buckets.size()) - 1 || buckets[n] == nullptr) {
				return nullptr;
			}
			if (buckets[n]->key != f) {
				MapItem<T, G>* ptr = buckets[n];
				while (ptr != nullptr) {
					if (ptr->key == f) {
						return &ptr->item;
					}
					ptr = ptr->next;
				}
				return nullptr;
			}
			return &buckets[n]->item;
		}
		bool remove(G f) {
			int n = hash_func(f);
			if (n > int(buckets.size()) - 1 || buckets[n] == nullptr) {
				return false;
			}
			if (buckets[n]->key != f) {
				MapItem<T, G>* ptr = buckets[n]->next;
				MapItem<T, G>* last = buckets[n];
				while (ptr != nullptr) {
					if (ptr->key == f) {
						last->next = ptr->next;
						delete ptr;
						return true;
					}
					last = ptr;
					ptr = ptr->next;
				}
			}
			MapItem<T, G>* next = buckets[n]->next;
			delete buckets[n];
			buckets[n] = next;
			return false;
		}
		bool remove(G f, T* t) {
			int n = hash_func(f);
			if (n > int(buckets.size()) - 1 || buckets[n] == nullptr) {
				return false;
			}
			if (&buckets[n]->item != t) {
				MapItem<T, G>* ptr = buckets[n]->next;
				MapItem<T, G>* last = buckets[n];
				while (ptr != nullptr) {
					if (&ptr->item == t) {
						last->next = ptr->next;
						delete ptr;
						return true;
					}
					last = ptr;
					ptr = ptr->next;
				}
			}
			MapItem<T, G>* next = buckets[n]->next;
			delete buckets[n];
			buckets[n] = next;
			return false;
		}

		void setHashFunction(std::function<int(G)> in) {
			buckets.clear();
			hash_func = in;
		}
		void reserve(size_t n) {
			buckets.reserve(n);
		}
		std::vector<MapItem<T, G>*>& getBuckets(){
			return buckets;
		}

	};

	//throw whatever class you want in there
	template<class T>
	class GStack {
		public:
		using iterator = T*;
		iterator begin() { return stack; }
		iterator end() { return stack + offset; }
		private:
			T* stack;
			size_t allocd;
			size_t offset;
		public:
			GStack() {
				stack = (T*)std::malloc(256);
				allocd = 256;
				offset = 0;
			}
			~GStack() {
				std::free(stack);
			}

			void push(T in) {
				if ((offset * sizeof(T)) + sizeof(T) > allocd) {
					stack = (T*)std::realloc(stack, allocd * 2);
					allocd *= 2;
				}
				std::memcpy(stack + 1, stack, (sizeof(T) * offset));
				*stack = in;
				offset++;
			}

			T pop() {
				T out = *stack;
				std::memcpy(stack, stack + 1, (sizeof(T) * offset));
				offset--;
				return out;
			}
			size_t size() {
				return offset;
			}
			void clear() {
				std::memset(stack, 0, allocd);
				offset = 0;
			}

	};
	template<class T>
	class Queue {
	public:
		using iterator = T*;
		iterator begin() { return queue; }
		iterator end() { return queue + offset; }
	private:
		T* queue;
		size_t offset;
		size_t allocd;
	public:
		Queue() {
			queue = (T*)std::malloc(256);
			allocd = 256;
			offset = 0;
		}
		~Queue() {
			std::free(queue);
		}
		void enqueue(T in) {
			if ((offset * sizeof(T)) + sizeof(T) > allocd) {
				queue = (T*)std::realloc(queue, allocd * 2);
				allocd *= 2;
			}
			*(queue + offset) = in;
			offset++;
		}
		T dequeue() {
			T out = *queue;
			offset--;
			std::memcpy(queue, queue + 1, (sizeof(T) * offset));
			return out;
		}
		size_t size() {
			return offset;
		}
		void clear() {
			std::memset(queue, 0, (sizeof(T) * offset));
			offset = 0;
		}
	};
	//only really works for built in number data types, add your own exceptions if you want
	template<class T>
	class Sort {
	private:
		static Gore::Vector<T> partition(Gore::Vector<T>& arr) {
			if (arr.size() < 2) {
				return arr;
			}
			Gore::Vector<T> left;
			Gore::Vector<T> right;
			size_t pivot = arr.size() - 1;
			for (int i = 0; i < pivot; i++) {
				if (arr[i] < arr[pivot]) {
					left.push_back(arr[i]);
				}
				else {
					right.push_back(arr[i]);
				}
			}
			Gore::Vector<T> ol = partition(left);
			Gore::Vector<T> orl = partition(right);
			ol.push_back(arr[pivot]);
			for (int i = 0; i < orl.size(); i++) {
				ol.push_back(orl[i]);
			}

			return ol;
		}

		static std::vector<T> partition(std::vector<T>& arr) {
			if (arr.size() < 2) { 
				return arr; }
			std::vector<T> left;
			std::vector<T> right;
			size_t pivot = arr.size() - 1;
			for (int i = 0; i < pivot; i++) {
				if (arr[i] < arr[pivot]) {
					left.push_back(arr[i]);
				}
				else {
					right.push_back(arr[i]);
				}
			}
			std::vector<T> ol = partition(left);
			std::vector<T> orl = partition(right);
			ol.push_back(arr[pivot]);
			for (int i = 0; i < orl.size(); i++) {
				ol.push_back(orl[i]);
			}
			
			return ol;
		}
		static std::vector<T> merge(std::vector<T>& arr) {
			if (arr.size() < 2) {
				return arr;
			}
			size_t middle = arr.size() / 2;
			std::vector<T> left;
			std::vector<T> right;
			for (int i = 0; i < middle; i++) { left.push_back(arr[i]); }
			for (int i = arr.size() - 1; i >= middle; i--) { right.push_back(arr[i]); }
			left = merge(left);
			right = merge(right);
			std::vector<T> out;
			size_t small = 0;
			size_t smallr = 0;
			while (true) {
				if (left[small] < right[smallr]) {
					int i;
					for (i = 0; i < out.size() && out[i] < left[small]; i++);
					if (i == out.size()) {
						out.push_back(left[small]);
					}
					else {
						out.insert(out.begin() + i, left[small]);
					}
					small++;
				}
				else if(right[smallr] < left[small]) {
					int i;
					for (i = 0; i < out.size() && out[i] < right[smallr]; i++);
					if (i == out.size()) {
						out.push_back(right[smallr]);
					}
					else {
						out.insert(out.begin() + i, right[smallr]);
					}
					smallr++;
				}
				//hit final element
				if (smallr == right.size()) {
					size_t start = out.size() - 1;
					for (int i = small; i < left.size(); i++) {
						if (left[i] > out[start]) {
							out.push_back(left[i]);
						}
						else {
							out.insert(out.begin() + start + 1, left[i]);
							start++;
						}
					}
					break;
				}
				else if (small == left.size()) {
					//have to add in the order of the smallest
					size_t start = out.size() - 1;
					for (int i = smallr; i < right.size(); i++) {
						if (right[i] > out[start]) {
							out.push_back(right[i]);
						}
						else {
							out.insert(out.begin() + start + 1, right[i]);
							start++;
						}
					}
					break;
				}
			}
			return out;
		}

		static Gore::Vector<T> merge(Gore::Vector<T>& arr) {
			if (arr.size() < 2) {
				return arr;
			}
			size_t middle = arr.size() / 2;
			Gore::Vector<T> left;
			Gore::Vector<T> right;
			for (int i = 0; i < middle; i++) { left.push_back(arr[i]); }
			for (int i = arr.size() - 1; i >= middle; i--) { right.push_back(arr[i]); }
			left = merge(left);
			right = merge(right);
			Gore::Vector<T> out;
			size_t small = 0;
			size_t smallr = 0;
			while (true) {
				if (left[small] < right[smallr]) {
					int i;
					for (i = 0; i < out.size() && out[i] < left[small]; i++);
					if (i == out.size()) {
						out.push_back(left[small]);
					}
					else {
						out.insert(i, left[small]);
					}
					small++;
				}
				else if (right[smallr] < left[small]) {
					int i;
					for (i = 0; i < out.size() && out[i] < right[smallr]; i++);
					if (i == out.size()) {
						out.push_back(right[smallr]);
					}
					else {
						out.insert(i, right[smallr]);
					}
					smallr++;
				}
				//hit final element
				if (smallr == right.size()) {
					size_t start = out.size() - 1;
					for (int i = small; i < left.size(); i++) {
						if (left[i] > out[start]) {
							out.push_back(left[i]);
						}
						else {
							out.insert(start + 1, left[i]);
							start++;
						}
					}
					break;
				}
				else if (small == left.size()) {
					//have to add in the order of the smallest
					size_t start = out.size() - 1;
					for (int i = smallr; i < right.size(); i++) {
						if (right[i] > out[start]) {
							out.push_back(right[i]);
						}
						else {
							out.insert(start + 1, right[i]);
							start++;
						}
					}
					break;
				}
			}
			return out;
		}

	public:
		static Gore::Vector<T> Quicksort(Gore::Vector<T>& arr) {
			return partition(arr);
		}
		static std::vector<T> Quicksort(std::vector<T>& arr) {
			return partition(arr);
		}

		static Gore::Vector<T> Mergesort(Gore::Vector<T>& arr) {
			return merge(arr);
		}
		static std::vector<T> Mergesort(std::vector<T>& arr) {
			return merge(arr);
		}

		static void Insertionsort(Gore::Vector<T>& arr) {
			for (size_t i = 1; i < arr.size(); i++) {
				size_t j = 0; //place to insert new element
				for (j; j < i && arr[j] < arr[i]; j++);
				T temp2 = arr[i];
				//shift the array over to the right by one
				for (int p = i - 1; p >= j && p >= 0; p--) {
					arr[p + 1] = arr[p];
				}

				arr[j] = temp2;
			}
		}
		static void Insertionsort(std::vector<T>& arr) {
			for (size_t i = 1; i < arr.size(); i++) {
				size_t j = 0; //place to insert new element
				for (j; j < i && arr[j] < arr[i]; j++);
				T temp2 = arr[i];
				//shift the array over to the right by one
				for (int p = i - 1; p >= j && p >= 0; p--) {
					arr[p + 1] = arr[p];
				}

				arr[j] = temp2;
			}

		}

		static void Bubblesort(Gore::Vector<T>& arr) {
			while (true) {
				bool swap = false;
				for (int i = 0; i < arr.size() - 1; i++) {
					if (arr[i + 1] < arr[i]) {
						T temp = arr[i];
						arr[i] = arr[i + 1];
						arr[i + 1] = temp;
						swap = true;
					}
				}
				if (!swap) {
					return;
				}
			}
		}
		static void Bubblesort(std::vector<T>& arr) {
			while (true) {
				bool swap = false;
				for (int i = 0; i < arr.size() - 1; i++) {
					if (arr[i + 1] < arr[i]) {
						T temp = arr[i];
						arr[i] = arr[i + 1];
						arr[i + 1] = temp;
						swap = true;
					}
				}
				if (!swap) {
					return;
				}
			}
		}

		static void Selectionsort(Gore::Vector<T>& arr) {
			for (int i = 0; i < arr.size(); i++) {
				int small = i;
				for (int j = i; j < arr.size(); j++) {
					if (arr[j] < arr[small]) {
						small = j;
					}
				}
				T temp = arr[small];
				arr[small] = arr[i];
				arr[i] = temp;
			}
		}
		static void Selectionsort(std::vector<T>& arr) {
			for (int i = 0; i < arr.size(); i++) {
				int small = i;
				for (int j = i; j < arr.size(); j++) {
					if (arr[j] < arr[small]) {
						small = j;
					}
				}
				T temp = arr[small];
				arr[small] = arr[i];
				arr[i] = temp;
			}
		}

		static void Bucketsort(Gore::Vector<T>& arr) {

		}
		static void Bucketsort(std::vector<T>& arr) {

		}
		static void Radixsort(Gore::Vector<T>& arr) {

		}

		static void Radixsort(std::vector<T>& arr) {

		}

		static void Countingsort(Gore::Vector<T>& arr) {

		}

		static void Countingsort(std::vector<T>& arr) {

		}

	};
}