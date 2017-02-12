#pragma once

#include "TextQuery.h"
#include <algorithm>
#include <cctype>

// 所有查询的抽象基类
class Query_base
{
	friend class Query;

protected:
	using line_no = TextQuery::line_no;
	virtual ~Query_base() = default;

private:
	virtual QueryResult eval(const TextQuery&) const = 0;
	virtual std::string rep() const = 0;
};

// 统一的接口类，用来隐藏复杂的类继承关系
class Query
{
	friend Query operator~(const Query&);
	friend Query operator|(const Query&, const Query&);
	friend Query operator&(const Query&, const Query&);

public:
	Query(const std::string&);
	QueryResult eval(const TextQuery& t) const
	{
		return q->eval(t);
	}
	std::string rep() const
	{
		return q->rep();
	}

private:
	Query(std::shared_ptr<Query_base> query) :q(query) {}
	std::shared_ptr<Query_base> q;
};

std::ostream& operator<<(std::ostream& os, const Query& query)
{
	return os << query.rep();
}

// 普通单词查询类
class WordQuery : public Query_base
{
	friend class Query;
	WordQuery(const std::string& s) : _query_word(s) {}
	QueryResult eval(const TextQuery& tq) const
	{
		return tq.query(_query_word);
	}
	std::string rep() const
	{
		return _query_word;
	}
	std::string _query_word;
};


// 先有了WordQuery的定义才能定义Query类的这个构造函数
// 难道不应该在这个函数中进行输入字符串的解析么
inline Query::Query(const std::string& s)
	//:q(new WordQuery(s))
{
	std::string::const_iterator it = s.begin();
	std::string word;
	while (it != s.end())
	{
		char ch = *it;
		if (std::isalpha(ch))
		{
			word.insert(word.end(), ch);
		}
		else if (std::isspace(ch))
		{

		}
		else if (ch == '|')
		{

		}
		else if (ch == '&')
		{

		}
		else if (ch == '~')
		{

		}
		else
		{
			exit(-1);
		}

		++it;
	}
}

class NotQuery : public Query_base
{
	friend Query operator~(const Query&);

	NotQuery(const Query& q) : query(q) {}
	std::string rep() const
	{
		return "~(" + query.rep() + ")";
	}
	QueryResult eval(const TextQuery&) const;
	Query query;
};

inline Query operator~(const Query& operand)
{
	// 返回时调用了Query的构造函数
	return std::shared_ptr<Query_base>(new NotQuery(operand));
}

class BinaryQuery : public Query_base
{
protected:
	BinaryQuery(const Query& l, const Query& r, std::string s)
		:lhs(l), rhs(r), opSym(s) {}
	std::string rep() const
	{
		return "(" + lhs.rep() + " " + opSym + " " + rhs.rep() + ")";
	}

	Query lhs, rhs;
	std::string opSym;
};

class AndQuery : public BinaryQuery
{
	friend Query operator&(const Query&, const Query&);

	AndQuery(const Query& left, const Query& right)
		:BinaryQuery(left, right, "&")
	{

	}

	QueryResult eval(const TextQuery&) const;
};

inline Query operator&(const Query& lhs, const Query& rhs)
{
	return std::shared_ptr<Query_base>(new AndQuery(lhs, rhs));
}

class OrQuery : public BinaryQuery
{
	friend Query operator|(const Query&, const Query&);

	OrQuery(const Query& left, const Query& right)
		:BinaryQuery(left, right, "|"){}
	QueryResult eval(const TextQuery&) const;
};

inline Query operator|(const Query& lhs, const Query& rhs)
{
	return std::shared_ptr<Query_base>(new OrQuery(lhs, rhs));
}




QueryResult OrQuery::eval(const TextQuery& text) const
{
	auto right = rhs.eval(text), left = lhs.eval(text);
	auto ret_lines =
		std::make_shared<std::set<line_no> >(left.begin(), left.end());
	ret_lines->insert(right.begin(), right.end());
	return QueryResult(left.get_file(), ret_lines, rep());
}

QueryResult AndQuery::eval(const TextQuery& text) const
{
	auto left = lhs.eval(text), right = rhs.eval(text);
	auto ret_lines = std::make_shared<std::set<line_no> >();
	std::set_intersection(left.begin(), left.end()
		, right.begin(), right.end()
		, std::inserter(*ret_lines, ret_lines->begin()));

	return QueryResult(left.get_file(), ret_lines, rep());
}

QueryResult NotQuery::eval(const TextQuery& text) const
{
	auto result = query.eval(text);
	auto ret_lines = std::make_shared<std::set<line_no> >();
	auto beg = result.begin(), end = result.end();
	auto sz = result.get_file()->size();
	for (size_t n = 0; n != sz; ++n)
	{
		if (beg == end || *beg != n)
			ret_lines->insert(n);
		else if (beg != end)
			++beg;
	}

	return QueryResult(result.get_file(), ret_lines, rep());
}


void runQueries(std::ifstream& ifs)
{
	TextQuery tq(ifs);
	std::string sin;
	while (true)
	{
		std::cout << "Enter a word or 'q' to quit: ";
		if (!(std::cin >> sin) || sin == "q") break;
		QueryResult qr = tq.query(sin);
		print(std::cout, qr) << std::endl;
	} // while true
}