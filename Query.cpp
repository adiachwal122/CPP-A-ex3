/*Adi Achwal 311233688
Amit nuni 204359400*/

#include "Query.h"
#include "TextQuery.h"
#include <memory>
#include <set>
#include <algorithm>
#include <iostream>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <regex>
using namespace std;
////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<QueryBase> QueryBase::factory(const string& s)
{
  regex regOneWord("^'?\\s*['\\w]+\\s*$");  // find rowe whith contains only one word
  regex regNot("^\\s*NOT\\s+(['\\w]+)\\s*$");  // find rowe whith contains NOT and only one word
  regex regAnd("^\\s*(['\\w]+)\\s+AND\\s+(['\\w]+)\\s*$");  // find rowe whith contains one word AND one word
  regex regOr("^\\s*(['\\w]+)\\s+OR\\s+(['\\w]+)\\s*$");  // find rowe whith contains one of the 2 words (word1 OR word2)
  regex regN("^\\s*(['\\w]+)\\s+(\\d+)\\s+(['\\w]+)\\s*$");  // find rowe whith contains both words and there is no more than n words between them n (word1 n word2)
  
  smatch result;  
  
  if(regex_match(s, regOneWord)) 
    return std::shared_ptr<QueryBase>(new WordQuery(s));
  else if(regex_search(s, result, regNot))
    return std::shared_ptr<QueryBase>(new NotQuery(result[1].str()));
  else if(regex_search(s, result, regAnd))
    return std::shared_ptr<QueryBase>(new AndQuery(result[1].str(), result[2].str()));
  else if(regex_search(s, result, regOr))
    return std::shared_ptr<QueryBase>(new OrQuery(result[1].str(), result[2].str()));
  else if(regex_search(s, result, regN))
    return std::shared_ptr<QueryBase>(new NQuery(result[1].str(), result[3].str(), stoi(result[2].str())));
  else
    throw invalid_argument("Unrecognized search"); //as asked
  
}
////////////////////////////////////////////////////////////////////////////////
QueryResult NotQuery::eval(const TextQuery &text) const
{
  QueryResult result = text.query(query_word);
  auto ret_lines = std::make_shared<std::set<line_no>>();
  auto beg = result.begin(), end = result.end();
  auto sz = result.get_file()->size();
  
  for (size_t n = 0; n != sz; ++n)
  {
    if (beg==end || *beg != n)
		ret_lines->insert(n);
    else if (beg != end)
		++beg;
  }
  return QueryResult(rep(), ret_lines, result.get_file());
    
}

QueryResult AndQuery::eval (const TextQuery& text) const
{
  QueryResult left_result = text.query(left_query);
  QueryResult right_result = text.query(right_query);
  
  auto ret_lines = std::make_shared<std::set<line_no>>();
  std::set_intersection(left_result.begin(), left_result.end(),
      right_result.begin(), right_result.end(), 
      std::inserter(*ret_lines, ret_lines->begin()));

  return QueryResult(rep(), ret_lines, left_result.get_file());
}

QueryResult OrQuery::eval(const TextQuery &text) const
{
  QueryResult left_result = text.query(left_query);
  QueryResult right_result = text.query(right_query);
  
  auto ret_lines = 
      std::make_shared<std::set<line_no>>(left_result.begin(), left_result.end());

  ret_lines->insert(right_result.begin(), right_result.end());

  return QueryResult(rep(), ret_lines, left_result.get_file());
}
/////////////////////////////////////////////////////////
QueryResult NQuery::eval(const TextQuery &text) const
{
  std::string query = "[\\.\",]?(" + left_query + "|" + right_query + ")[\\.\",]?";
  regex reg(query);  
  smatch res;
      
  QueryResult result = AndQuery::eval(text);
  auto ret_lines = std::make_shared<std::set<line_no>>();
  std::set<unsigned long>::iterator it;
      
  for (it = result.begin(); it != result.end(); ++it) {
    auto match = *(result.get_file()->begin() + *it);
    istringstream line(match);     
    int count = 0;
    int flag = 0;
		std::string word;               
		while (line >> word) {       
      flag += regex_search(word, res, reg);
		  if(flag && flag < 2) 
		    count++; 
		}
		if(count-1 <= dist) 
		ret_lines->insert(*it);
  }
  return QueryResult(rep(), ret_lines, result.get_file());
}
/////////////////////////////////////////////////////////