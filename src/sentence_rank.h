#ifndef SENTENCE_RANK_H_
#define SENTENCE_RANK_H_

#include <string>
#include <vector>
#include <map>
#include <set>

class SentenceRank
{

    struct value_great
    {
        template <typename T>
        bool operator()(const std::pair<T, double> &x, const std::pair<T, double> &y) const
        {
            return x.second > y.second;
        }
    };
    
    public:
        SentenceRank() {}
        SentenceRank(int window_length, int max_iter_num, double d, double least_delta):
            m_window_length(window_length),
            m_max_iter_num(max_iter_num),
            m_d(d),
            m_least_delta(least_delta) {}

        ~SentenceRank() {}

        // @brief 抽取关键句
        void ExtractKeySentence(const std::map<std::string, std::vector<std::string> > &sentence_token_map, std::vector<std::pair<std::string, double> > &key_sentences, const size_t topN = 5);

    private:

        // @brief 计算两个句子距离
        double CalcDist(const std::vector<std::string> &token_vec1, const std::vector<std::string> &token_vec2);

        // @brief 所有句子两两构建边
        void BuildSentenceRelation(const std::map<std::string, std::vector<std::string> > &sentence_token_map);

        // @brief 计算句子得分 
        void CalcSentenceScore(std::map<size_t, double> &score_map);

        // @brief 获取边的权重
        double GetWeight(size_t i, size_t j) const;

        // @brief clear for extracting from new doc
        void Clear();

    private:
        size_t m_window_length;
        size_t m_max_iter_num;
        double m_d;
        double m_least_delta;

        std::vector<std::string> m_sentence_vec;
        std::map<std::string, size_t> m_sentence_index;
        std::map<std::pair<size_t, size_t>, double> m_weight_map;   // edge weight
        std::map<size_t, double> m_out_sum_map;                     // out edges weight sum of one node  
};

#endif
