#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cctype>
#include "text_utils.h"

using namespace std;

void TextUtils::ExtractChars(const string &text, bool is_utf8, vector<string> &res)
{
    res.clear();

    const size_t len = text.size();
    size_t i = 0;
    if (is_utf8)
    {
        while (i < len)
        {
            if (text[i] < 0) // hanzi
            {
                unsigned char value = (unsigned char) (text[i]);
                size_t byte_num = 0;
                while (value & 0x80)
                {
                    value = value << 1;
                    byte_num++;
                }
                res.push_back(text.substr(i, byte_num));
                i += byte_num;
            }
            else
            {
                res.push_back(text.substr(i,1));
                i++;
            }
        }
        return;
    }
    
    // gbk
    i = 0;
    while (i < len)
    {
        if (text[i] < 0) // hanzi
        {
            res.push_back(text.substr(i,2));
            i += 2;
        }
        else
        {
            res.push_back(text.substr(i,1));
            i++;
        }
    }
}

void TextUtils::ExtractNgram(const string &text, size_t n, bool is_utf8, vector<string> &res)
{
    res.clear();

    if (text.size() < n)
    {
        cout << "text is too short" << endl;
        return;
    }

    vector<string> char_vec;
    ExtractChars(text, is_utf8, char_vec);

    if (char_vec.empty())
        return;

    stringstream ss;
    size_t char_num = char_vec.size();
    for (size_t i = 0; i < char_num; ++i)
    {
        for(size_t j = 0; j < n; ++j)
        {
            if (i+j >= char_num)
                return;
            ss << char_vec[i+j];
        }
        res.push_back(ss.str());
        ss.str("");
        ss.clear();
    }
}

string TextUtils::RemoveExpression(const std::string &text, const size_t max_remove_length)
{
    string strx(text);
    string::size_type start = 0;
    size_t length; 
    while (true)
    {   
        start = strx.find('[', start); 
        if (start == string::npos)
            break;
        string::size_type end = strx.find(']', start+1);
        if (end == string::npos)
            break;
        length =  end - start + 1;
        if (length <= max_remove_length)
            strx = strx.replace(start, length, " ");
        start++;
    }   
    return strx;
}

string TextUtils::RemoveBracketPiece(const std::string &text, const size_t max_remove_length)
{
    string tag("\xa1\xbe"); //GBK:【
    string tag2("\xa1\xbf");
    size_t len = 2;
    if (0 == text.compare(0, len, tag))
    {
        size_t i = 0;
        while (i < text.size())
        {
            if (text[i] < 0)
            {
                if (text[i] == tag2[0] && text[i+1] == tag2[1])
                    break;
                i += 2;
            }
            else
                ++i;
        }
        if ( i != text.size() && (i-len) <= max_remove_length)
            return text.substr(i+len);
    }
    return text;
}

bool TextUtils::IsAscii(const std::string &word)
{
    unsigned char mask = 0x80;
    for (size_t i = 0; i < word.size(); ++i)
    {
        if ((unsigned char)word[i] & mask)
            return false;
    }
    return true;
}

bool TextUtils::IsAlphaNum(const std::string &word)
{
    if (word.empty())
        return false;
    for (size_t i = 1; i < word.size(); ++i)
    {
        if (!isalnum(word[i]))
            return false;
    }
    return true;
}

string::size_type TextUtils::FindGbkSubstring(const string &src, const string &sub)
{
    string::size_type i = 0;
    const size_t len = sub.size();
    while (i < src.size())
    {
        if (src[i] < 0) // gbk hanzi
        {
            if (src.compare(i, len, sub) == 0)
                return i;
            i += 2;
        }
        else
        {
            if (src.compare(i, len, sub) == 0)
                return i;
            i++;
        }
    }
    return string::npos;
}

void TextUtils::SplitToSentenceGBK(const string &text, const string &separator_str, vector<string> &sentence_vec, const size_t least_sentence_length)
{
    sentence_vec.clear();
    
    size_t prev = 0;
    size_t i = 0;
    size_t len = 0;
    const size_t n = text.size();
    while (i < n)
    {
        if (text[i] < 0) // gbk hanzi
        {
            if (FindGbkSubstring(separator_str, text.substr(i,2)) != string::npos)
            {
                len = i - prev;
                if (len >= least_sentence_length)
                {
                    sentence_vec.push_back(text.substr(prev, len));
                }
                prev = i+2;
            }
            else if ((i+2) == n)
            {
                sentence_vec.push_back(text.substr(prev, n-prev));
            }
            i += 2;
        }
        else //ASCII
        {
            if (FindGbkSubstring(separator_str, text.substr(i,1)) != string::npos)
            {
                len = i - prev;
                if (len >= least_sentence_length)
                {
                    sentence_vec.push_back(text.substr(prev, len));
                }
                prev = i+1;
            }
            else if (text.compare(i, 3, "...") == 0)
            {
                len = i - prev;
                sentence_vec.push_back(text.substr(prev, len));
                prev = i+3;
                i+=2;
            }
            else if ((i+1) == n)
            {
                sentence_vec.push_back(text.substr(prev, n-prev));
            }
            i++;
        }
    }
}

void TextUtils::SplitToSentence(const string &text, const string &separator_str, vector<string> &sentence_vec, bool is_utf8, const size_t least_sentence_length)
{
    if (!is_utf8)
    {
        SplitToSentenceGBK(text, separator_str, sentence_vec, least_sentence_length);
        return;
    }

    sentence_vec.clear();
    
    size_t prev = 0;
    size_t i = 0;
    size_t len = 0;
    const size_t n = text.size();
    while (i < n)
    {
        if (text[i] < 0) // hanzi
        {
            if (separator_str.find(text.substr(i,3)) != string::npos)
            {
                len = i - prev;
                if (len >= least_sentence_length)
                {
                    sentence_vec.push_back(text.substr(prev, len));
                }
                prev = i+3;
            }
            else if ((i+3) == n)
            {
                sentence_vec.push_back(text.substr(prev, n-prev));
            }
            i += 3;
        }
        else //ASCII
        {
            if (separator_str.find(text[i]) != string::npos)
            {
                len = i - prev;
                if (len >= least_sentence_length)
                {
                    sentence_vec.push_back(text.substr(prev, len));
                }
                prev = i+1;
            }
            else if (text.compare(i, 3, "...") == 0)
            {
                len = i - prev;
                sentence_vec.push_back(text.substr(prev, len));
                prev = i+3;
                i+=2;
            }
            else if ((i+1) == n)
            {
                sentence_vec.push_back(text.substr(prev, n-prev));
            }
            i++;
        }
    }
}

time_t TextUtils::ConvertTimestr(const string &time_str, const string format)
{
    struct tm ttime = {0};
    strptime(time_str.c_str(), format.c_str(), &ttime);
    return mktime(&ttime);
}

string TextUtils::ConvertTimestamp(time_t tt, const string format)
{
    struct tm *ttime = localtime(&tt);
    char date_str[64];
    strftime(date_str, 64, format.c_str(),ttime);
    return string(date_str);
}

void TextUtils::Split(const string &s, const string &delim, vector<string> &elems, size_t max_split)
{
    elems.clear();
    if (s.empty())
        return;
    string::size_type next = string::npos;
    string::size_type current = 0;
    size_t len = delim.size();
    while (true) {
        next = s.find(delim, current);
        elems.push_back(s.substr(current, next - current));
        if (next == string::npos)
            break;
        current = next+len;
        if (max_split > 0 && max_split == elems.size()) {
            elems.push_back(s.substr(current));
            break;
        }  
    }  
}

string TextUtils::Join(const vector<string> &vec, char sep)
{
    if (vec.empty())
        return string("");
    stringstream ss;
    vector<string>::const_iterator iter = vec.begin();
    ss << *iter;
    for (iter++; iter != vec.end(); ++iter)
    {
        ss << sep << *iter;
    }
    return ss.str();
}
