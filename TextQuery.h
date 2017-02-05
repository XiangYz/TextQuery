#pragma once
#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <iterator>
#include <sstream>


class QueryResult
{
public:
	friend std::ostream& print(std::ostream&, const QueryResult);
	using line_no = std::vector<std::string>::size_type;

	QueryResult(std::shared_ptr<std::vector<std::string> > ptr_file
		, std::shared_ptr<std::set<line_no> > ptr_nums
		, std::string word)
		:_file(ptr_file), _nums(ptr_nums), _word(word)
	{

	}

	std::set<line_no>::iterator begin() const
	{
		return _nums->begin();
	}

	std::set<line_no>::iterator end() const
	{
		return _nums->end();
	}

	std::shared_ptr<std::vector<std::string> > get_file() const
	{
		return _file;
	}

private:

	std::shared_ptr<std::vector<std::string> > _file;
	std::shared_ptr<std::set<line_no> > _nums;
	std::string _word;


};

std::ostream& print(std::ostream& os, const QueryResult qr)
{
	int nums = qr._nums->size();
	os << qr._word << " occurs " << nums << " "
		<< (nums == 1 ? "time" : "times") << std::endl;

	for (auto num : *qr._nums)
	{
		os << "\t(line " << num + 1 << ") "
			<< *(qr._file->begin() + num) << std::endl;
	}

	return os;
}

class TextQuery
{
public:
	using line_no = std::vector<std::string>::size_type;

	explicit TextQuery(std::ifstream &ifs)
	{
		_file = std::make_shared<std::vector<std::string> >();
		std::string line;
		while (std::getline(ifs, line))
		{
			_file->push_back(line);
			std::istringstream iss(line);
			std::string word;
			while (iss >> word)
			{
				auto it = _words_map.find(word);
				if (it == _words_map.end())
				{
					_words_map[word] = std::make_shared<std::set<line_no> >();
				}
				_words_map[word]->insert(_file->size() - 1);
			} // while iss >> word
		} // while getline
	} // ctor

	QueryResult query(const std::string &word) const
	{
		static std::shared_ptr<std::set<line_no> > no_data;

		auto it_find = _words_map.find(word);
		if (it_find == _words_map.end())
		{
			return QueryResult(_file, no_data, word);
		}
		else
		{
			return QueryResult(_file, it_find->second, word);
		}
	}

private:
	std::shared_ptr<std::vector<std::string> > _file;
	std::map<std::string, std::shared_ptr<std::set<line_no> > > _words_map;
};



