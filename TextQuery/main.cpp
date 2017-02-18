#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <stack>
#include "TextQuery2.h"


bool GetToken(std::string& expression, int& it, std::string& token)
{
	std::string word;
	bool res = false;
	while (it != expression.length())
	{
		char ch = expression[it++];

		if (ch == '&' || ch == '|' || ch == '~' || ch == '(' || ch == ')')
		{
			if (word.empty())
			{
				char s[5] = { 0 };
				s[0] = ch;
				token = std::string(s);
				return true;
			}
			else
			{
				--it;
				token = word;
				return true;
			}
		}
		else if (ch == ' ')
		{
			if (!word.empty())
			{
				token = word;
				return true;
			}
		}
		else
		{
			word.insert(word.end(), ch);
		}
	} // while it != end

	if (!word.empty())
	{
		token = word;
		return true;
	}

	return false;
}

void err_exit(std::string& str)
{
	std::cerr << str << std::endl;
	exit(-1);
}

std::string ToPostExpression(std::string& expression)
{
	std::string post_expression;
	std::stack<std::string> trans_stack;
	int it = 0;
	
	while (true)
	{
		std::string word;
		if (!GetToken(expression, it, word)) break;

		if (word == "&")
		{
			trans_stack.push(word);

		}
		else if (word == "|")
		{
			trans_stack.push(word);
		}
		else if (word == "~")
		{
			bool has_next = GetToken(expression, it, word);
			if (!has_next) err_exit(std::string("wrong expression"));

			if (word == "(")
			{
				trans_stack.push(word);
			}
			else // ~后面的不是左括号就一定是单词
			{
				post_expression.append(word);
				post_expression.append(" ~ ");
			}

		}
		else if (word == "(")
		{
			trans_stack.push(word);
		}
		else if (word == ")")
		{
			while (!trans_stack.empty())
			{
				std::string top = trans_stack.top();
				trans_stack.pop();
				if (top == "(")
				{
					top = trans_stack.top();
					if (top == "~")
					{
						trans_stack.pop();
						post_expression.append("~ ");
					}

					break;
				}
				
				post_expression.append(top);
				post_expression.append(" ");
			}
		}
		else // 其余情况均作为单词的一部分
		{
			post_expression.append(word);
			post_expression.append(" ");
		}

	} // while true

	// 栈内还剩余的符号添加到外面
	while (!trans_stack.empty())
	{
		std::string top = trans_stack.top();
		trans_stack.pop();
		post_expression.append(top);
		post_expression.append(" ");

	}

	return post_expression;

}

#define DEFAULT_PATH "TextQueryTest.txt"

int main(int argc, char* argv[])
{
	std::string path;
	if (argc == 2) path = argv[1];
	else path = DEFAULT_PATH;

	std::ifstream ifs(path);
	if (!ifs) err_exit(std::string("Cannot open target file"));

	std::string expression;
	std::string post_expression;

	std::stack<Query> calc_stack;

	while (true)  // 查询循环
	{
		std::cout << "Input your expression: ";
		std::getline(std::cin, expression);
		if (expression == "q") break;
		if (expression.empty()) continue;

		post_expression = ToPostExpression(expression);

		std::string word;
		int it = 0;
		while (GetToken(post_expression, it, word))
		{
			if (word == "&")
			{
				if (calc_stack.empty()) err_exit(std::string("wrong expression"));
				Query op1 = calc_stack.top();
				calc_stack.pop();

				if (calc_stack.empty()) err_exit(std::string("wrong expression"));
				Query op2 = calc_stack.top();
				calc_stack.pop();

				Query res = op1 & op2;
				calc_stack.push(res);

			}
			else if (word == "|")
			{
				if (calc_stack.empty()) err_exit(std::string("wrong expression"));
				Query op1 = calc_stack.top();
				calc_stack.pop();

				if (calc_stack.empty()) err_exit(std::string("wrong expression"));
				Query op2 = calc_stack.top();
				calc_stack.pop();

				Query res = op1 | op2;
				calc_stack.push(res);
			}
			else if (word == "~")
			{
				if (calc_stack.empty()) err_exit(std::string("wrong expression"));
				Query op1 = calc_stack.top();
				calc_stack.pop();

				Query res = ~op1;
				calc_stack.push(res);
			}
			else
			{
				Query q(word);
				calc_stack.push(q);
			}
		} // while post_expression

		if (calc_stack.size() != 1) err_exit(std::string("wrong expression"));

		Query res = calc_stack.top();
		calc_stack.pop();

		TextQuery tq(ifs);
		QueryResult qr = res.eval(tq);
		print(std::cout, qr);

	} // while true
	

	return 0;
}