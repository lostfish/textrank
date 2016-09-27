#ifndef TEXT_RANK_H_
#define TEXT_RANK_H_

#include <string>
#include <vector>
#include <map>
#include <set>

struct value_great
{
    template <typename T>
    bool operator()(const std::pair<T, double> &x, const std::pair<T, double> &y) const
    {
        return x.second > y.second;
    }
};

class TextRank
{
    public:
        TextRank() {}
        TextRank(int window_length, int max_iter_num, double d, double least_delta):
            m_window_length(window_length),
            m_max_iter_num(max_iter_num),
            m_d(d),
            m_least_delta(least_delta) {}

        ~TextRank() {}

        // @brief 抽取tf最高的词 
        void ExtractHighTfWords(const std::vector<std::string> &token_vec, std::vector<std::pair<std::string, double> > &keywords, const size_t topN);

        // @brief 抽取关键词 
        void ExtractKeyword(const std::vector<std::string> &token_vec, std::vector<std::pair<std::string, double> > &keywords, const size_t topN = 10);

    private:
        // @brief 建立词之间的关系
        int BuildWordRelation(const std::vector<std::string> &token_vec, std::map<size_t, std::set<size_t> > &word_neighbors);

        // @brief 计算单词得分
        void CalcWordScore(const std::map<size_t, std::set<size_t> > &word_neighbors, std::map<size_t, double> &score_map);

        // @brief 更新边的权重
        void UpdateWeightMap(size_t i, size_t j);

        // @brief 获取边的权重
        double GetWeight(size_t i, size_t j) const;

        // @brief clear for extracting from new doc
        void Clear();

    private:
        size_t m_window_length;
        size_t m_max_iter_num;
        double m_d;
        double m_least_delta;

        std::vector<std::string> m_word_vec;
        std::map<std::string, size_t> m_word_index;                 // word -> word index of m_word_vec
        std::map<std::pair<size_t, size_t>, double> m_weight_map;   // edge weight  
        std::map<size_t, double> m_out_sum_map;                     // out edges weight sum of one node  
};

#endif
