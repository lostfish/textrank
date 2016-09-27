#ifndef TEXT_UTILS_H_
#define TEXT_UTILS_H_

#include <string>
#include <map>
#include <set>
#include <vector>
#include <ctime>

struct size_value_less
{
    template<typename T>
    inline bool operator()(const std::pair<T, size_t> &x, const std::pair<T, size_t> &y) const
    {
        return x.second < y.second;
    }
};

struct size_value_great
{
    template<typename T>
    inline bool operator()(const std::pair<T, size_t> &x, const std::pair<T, size_t> &y) const
    {
        return x.second > y.second;
    }
};

const std::string g_seperator_str("！？。；:】　…，丨—｜┃!?@| ");

const std::string g_gbk_seperator_str("\xa3\xa1\xa3\xbf\xa1\xa3\xa3\xbb:\xa1\xbf\xa1\xa1\xa1\xad\xa3\xac\xd8\xad\xa1\xaa\xa3\xfc\xa9\xa7!?@| ");

class TextUtils
{
public:

    // @brief 抽取出所有字符
    static void ExtractChars(const std::string &text, bool is_utf8, std::vector<std::string> &res);

    // @brief 抽取Ngram特征
    static void ExtractNgram(const std::string &text, size_t n, bool is_utf8, std::vector<std::string> &res);

    // @brief 去除表情符号
    static std::string RemoveExpression(const std::string &text, const size_t max_remove_length = 8);

    // @brief 去除标题中括号前缀 (just for GBK)
    static std::string RemoveBracketPiece(const std::string &text, const size_t max_remove_length = 8);

    // @brief 判断是否是ASCII字符
    static bool IsAscii(const std::string &word);

    // @brief 判断是否是字母数字词
    static bool IsAlphaNum(const std::string &word);

    // @brief 查找GBK文本中的子串 (just for GBK)
    static std::string::size_type FindGbkSubstring(const std::string &src, const std::string &sub);

    // @brief 将文本分割为句子 (just for GBK)
    static void SplitToSentenceGBK(const std::string &text, const std::string &separator_str, std::vector<std::string> &sentence_vec, const size_t least_sentence_length = 8);
    
    // @brief 将文本分割为句子
    static void SplitToSentence(const std::string &text, const std::string &separator_str, std::vector<std::string> &sentence_vec, bool is_utf8, const size_t least_sentence_length = 8);

    // @brief 将时间字符串转为时间戳
    static time_t ConvertTimestr(const std::string &time_str, const std::string format = "%Y-%m-%d");

    // @brief 将时间戳转为时间字符串
    static std::string ConvertTimestamp(time_t tt, const std::string format="%Y-%m-%d %H:%M:%S");

    // @brief 分割字符串
    static void Split(const std::string &s, const std::string &delim, std::vector<std::string> &elems, size_t max_split = 0);

    // @brief 拼接字符串
    static std::string Join(const std::vector<std::string> &vec, char sep);
};

#endif
