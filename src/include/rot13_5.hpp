#ifndef INFER_INCLUDE_ROT13_5_HPP_
#define INFER_INCLUDE_ROT13_5_HPP_

class rot13_5 {
  public:
	rot13_5()
		:_table(new unsigned char[256])
	{
		for (size_t i(0); i <= std::numeric_limits<unsigned char>::max(); ++i) {
			_table[i] = i;
		}

		for (size_t i('a'); i <= 'z'; ++i) {
			_table[i] += 13;
			if (_table[i] > 'z') {
				_table[i] -= 'z' - 'a' + 1;
			}
		}

		for (size_t i('A'); i <= 'Z'; ++i) {
			_table[i] += 13;
			if (_table[i] > 'Z') {
				_table[i] -= 'Z' - 'A' + 1;
			}
		}

		for (size_t i('0'); i < '9'; ++i) {
			_table[i] += 5;
			if (_table[i] > '9') {
				_table[i] -= '9' - '0' + 1;
			}
		}
	}

	~rot13_5() {
		delete _table;
	}

	std::string operator() (const std::string &s) {
		std::string ret(s);
		for (std::string::iterator i(ret.begin());
			 i != ret.end();
			 ++i)
		{
			*i = _table[*i];
		}

		return ret;
	}

  private:
	unsigned char *_table;
};

#endif
